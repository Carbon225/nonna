#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "pb_encode.h"
#include "proto_framing.h"
#include "nonna.pb.h"

#include "nonna.h"
#include "nonna_esp/wifi_man.h"
#include "nonna_esp/ota.h"
#include "nonna_esp/http_params.h"

static const char TAG[] = "main";

static SemaphoreHandle_t g_core1_init_done;

void nonna_enable(void)
{
    ESP_LOGW(TAG, "Nonna enabled");
}

void nonna_disable(void)
{
    ESP_LOGW(TAG, "Nonna disabled");
}

static void core1_main(void *pv)
{
    xSemaphoreGive(g_core1_init_done);
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGW(TAG, "Nonna starting");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ota_init();
    param_store_init();

    g_core1_init_done = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(core1_main, "core1_main", 4096, 0, 7, 0, APP_CPU_NUM);

    while (xSemaphoreTake(g_core1_init_done, portMAX_DELAY) != pdTRUE);

    wifi_man_start();
    ota_start();
    param_store_start();
}
