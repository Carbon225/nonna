#include "nonna_esp/ota.h"

#include <string.h>
#include <inttypes.h>

#include "sys/param.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"


#define PORT                   8888
#define KEEPALIVE_IDLE         5
#define KEEPALIVE_INTERVAL     5
#define KEEPALIVE_COUNT        3


static const char TAG[] = "ota";


typedef struct
{
    char message[64];
} tcp_message_t;


static QueueHandle_t g_sock;
static QueueHandle_t g_send_queue;


static void tcp_queue_send(const tcp_message_t *msg)
{
    xQueueSend(g_send_queue, msg, 0);
}

#define TCP_LOGI(TAG, fmt, ...) \
    do { \
        ESP_LOGI(TAG, fmt, ##__VA_ARGS__); \
        tcp_message_t msg; \
        snprintf(msg.message, sizeof(msg.message), fmt "\n", ##__VA_ARGS__); \
        tcp_queue_send(&msg); \
    } while (0)

#define TCP_LOGE(TAG, fmt, ...) \
    do { \
        ESP_LOGE(TAG, fmt, ##__VA_ARGS__); \
        tcp_message_t msg; \
        snprintf(msg.message, sizeof(msg.message), fmt "\n", ##__VA_ARGS__); \
        tcp_queue_send(&msg); \
    } while (0)

static void ota_recv_main(int sock)
{
    int n_recv = 0;
    char rx_buffer[256] = {0};

    esp_err_t err = ESP_FAIL;

    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    int binary_file_length = 0;

    uint32_t expected_file_size = 0;

    bool image_header_was_checked = false;
    char image_header_buf[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)] = {0};
    int image_header_written = 0;
    esp_app_desc_t *new_app_info = (esp_app_desc_t*) &image_header_buf[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)];

    const esp_partition_t *running = esp_ota_get_running_partition();

    TCP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08"PRIx32")",
             running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    assert(update_partition != NULL);
    TCP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%"PRIx32,
             update_partition->subtype, update_partition->address);

    do
    {
        if (expected_file_size == 0)
        {
            n_recv = recv(sock, &expected_file_size, sizeof(expected_file_size), 0);
            if (n_recv != sizeof(expected_file_size))
            {
                TCP_LOGE(TAG, "Error in receiving file size");
                return;
            }
            TCP_LOGI(TAG, "Expecting file size: %lu", expected_file_size);
        }
        else if (image_header_written < sizeof(image_header_buf))
        {
            n_recv = recv(sock, image_header_buf + image_header_written, sizeof(image_header_buf) - image_header_written, 0);
            if (n_recv > 0)
            {
                image_header_written += n_recv;
            }
        }
        else
        {
            n_recv = recv(sock, rx_buffer, sizeof(rx_buffer), 0);
            if (n_recv > 0)
            {
                err = esp_ota_write(update_handle, (const void *) rx_buffer, n_recv);
                if (err != ESP_OK)
                {
                    TCP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
                    esp_ota_abort(update_handle);
                    return;
                }
                binary_file_length += n_recv;
                ESP_LOGD(TAG, "Written image length %d", binary_file_length);
                if (binary_file_length >= expected_file_size)
                {
                    break;
                }
            }
        }

        if (n_recv < 0)
        {
            TCP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        }
        else if (n_recv == 0)
        {
            TCP_LOGI(TAG, "Connection closed");
        }
        else
        {   
            if (!image_header_was_checked && image_header_written == sizeof(image_header_buf))
            {
                TCP_LOGI(TAG, "New firmware version: %s", new_app_info->version);

                esp_app_desc_t running_app_info;
                if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
                {
                    TCP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
                }

                const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                esp_app_desc_t invalid_app_info;
                if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK)
                {
                    TCP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
                }

                if (last_invalid_app != NULL)
                {
                    if (memcmp(invalid_app_info.version, new_app_info->version, sizeof(new_app_info->version)) == 0)
                    {
                        TCP_LOGI(TAG, "New version is the same as invalid version");
                        TCP_LOGI(TAG, "Old invalid version: %s", invalid_app_info.version);
                        TCP_LOGI(TAG, "The firmware has been rolled back to the previous version");
                        return;
                    }
                }

                image_header_was_checked = true;

                err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
                if (err != ESP_OK)
                {
                    TCP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                    esp_ota_abort(update_handle);
                    return;
                }
                TCP_LOGI(TAG, "esp_ota_begin succeeded");

                err = esp_ota_write(update_handle, (const void *) image_header_buf, sizeof(image_header_buf));
                if (err != ESP_OK)
                {
                    TCP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
                    esp_ota_abort(update_handle);
                    return;
                }
                binary_file_length += n_recv;
                ESP_LOGD(TAG, "Written image length %d", binary_file_length);
            }
        }
    } while (n_recv > 0);

    TCP_LOGI(TAG, "Total binary data length: %d", binary_file_length);
    if (expected_file_size != binary_file_length)
    {
        TCP_LOGE(TAG, "Error: received %d bytes, expected %lu", binary_file_length, expected_file_size);
        esp_ota_abort(update_handle);
        return;
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED)
        {
            TCP_LOGE(TAG, "Image validation failed, image is corrupted");
        }
        else
        {
            TCP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        return;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK)
    {
        TCP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)", esp_err_to_name(err));
        return;
    }

    TCP_LOGI(TAG, "OTA update success, rebooting...");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    shutdown(sock, 0);
    close(sock);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    esp_restart();
}

static void tcp_send(int sock, const char buf[])
{
    int to_write = strlen(buf);
    int written = 0;
    while (to_write > 0)
    {
        int ret = send(sock, buf + written, to_write, 0);
        if (ret < 0)
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            break;
        }
        to_write -= ret;
        written += ret;
    }
}

static void tcp_send_task(void *pv)
{
    tcp_message_t msg;
    int sock;

    for (;;)
    {
        if (xQueueReceive(g_send_queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            if (xQueuePeek(g_sock, &sock, 0) != pdTRUE) continue;
            tcp_send(sock, msg.message);
        }
    }
}

static void tcp_server_task(void *pv)
{
    int addr_family = AF_INET;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *) &dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        ESP_ERROR_CHECK(ESP_FAIL);
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0)
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    for (;;)
    {
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr;
        socklen_t addr_len = sizeof(source_addr);

        int sock = accept(listen_sock, (struct sockaddr *) &source_addr, &addr_len);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            continue;
        }

        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        ESP_LOGI(TAG, "Socket accepted");

        xQueueOverwrite(g_sock, &sock);

        ota_recv_main(sock);

        while (xQueueReceive(g_sock, &sock, portMAX_DELAY) != pdTRUE);

        shutdown(sock, 0);
        close(sock);
    }
}

static void tcp_server_init(void)
{
    g_send_queue = xQueueCreate(32, sizeof(tcp_message_t));
    g_sock = xQueueCreate(1, sizeof(int));
}

static void tcp_server_start(void)
{
    xTaskCreatePinnedToCore(tcp_server_task, "tcp_recv", 4096, NULL, 6, NULL, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(tcp_send_task, "tcp_send", 4096, NULL, 5, NULL, PRO_CPU_NUM);
}

void ota_init(void)
{
    tcp_server_init();
}

void ota_start(void)
{
    tcp_server_start();
}
