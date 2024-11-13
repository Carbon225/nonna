#include "nonna_esp/http_params.h"
#include "nonna_esp/logo.h"

#include <stdint.h>

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "mdns.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const char TAG[] = "params_store";
static const char PART_NAME[] = "storage";

bool param_enabled = false;
double param_speed = 0.600f;
double param_kp = 0.580f;
double param_ki = 0.000f;
double param_kd = 0.220f;
int32_t param_mode = 1;

static nvs_handle_t my_handle;

static void read_double_from_nvs(const char *key, double *value)
{
    size_t required_size = sizeof(double);
    esp_err_t err = nvs_get_blob(my_handle, key, value, &required_size);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Read %s from NVS: %f", key, *value);
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(TAG, "The value %s is not initialized yet!", key);
    }
    else
    {
        ESP_LOGE(TAG, "Error (%s) reading %s from NVS!", esp_err_to_name(err), key);
    }
}

static void read_int_from_nvs(const char *key, int32_t *value)
{
    esp_err_t err = nvs_get_i32(my_handle, key, value);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Read %s from NVS: %ld", key, *value);
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(TAG, "The value %s is not initialized yet!", key);
    }
    else
    {
        ESP_LOGE(TAG, "Error (%s) reading %s from NVS!", esp_err_to_name(err), key);
    }
}

static void start_mdns_service(void)
{
    esp_err_t err = mdns_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "MDNS Init failed: %d", err);
        return;
    }

    mdns_hostname_set("nonna");
    mdns_instance_name_set("Nonna configuration portal");
    mdns_service_add("ESP32-Web", "_http", "_tcp", 80, NULL, 0);
}

static void read_params_from_nvs(void)
{
    read_double_from_nvs("param_speed", &param_speed);
    read_double_from_nvs("param_kp", &param_kp);
    read_double_from_nvs("param_ki", &param_ki);
    read_double_from_nvs("param_kd", &param_kd);
    read_int_from_nvs("param_mode", &param_mode);
}

static char response[16384];

static esp_err_t get_index(httpd_req_t *req)
{
    memset(response, 0, sizeof(response));
    // Conditionally display either the "START NONNA" or "STOP NONNA" button based on param_enabled
    const char *start_stop_button = param_enabled ? "<a href=\"/stop\" style=\"background-color: #A82828; color: white; padding: 10px 20px; text-decoration: none; border-radius: 4px; display: inline-block;\">STOP NONNA</a>" : "<a href=\"/start\" style=\"background-color: #4CAF50; color: white; padding: 10px 20px; text-decoration: none; border-radius: 4px; display: inline-block;\">START NONNA</a>";
    sprintf(response,
            "<!DOCTYPE html>"
            "<html>"
            "<head>"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />"
            "<meta charset=\"UTF-8\">"
            "<style>"
            "table {"
            "  font-family: Arial, sans-serif;"
            "  border-collapse: collapse;"
            "  width: 100%%;"
            "}"
            "td, th {"
            "  border: 1px solid #dddddd;"
            "  text-align: left;"
            "  padding: 8px;"
            "}"
            "tr:nth-child(even) {"
            "  background-color: #f2f2f2;"
            "}"
            "input[type=submit], a{"
            "  font-size: 15px;"
            "}"
            "img {"
            "  display: block;"
            "  position: absolute;"
            "  right: 0;"
            "  bottom: 0;"
            "}"
            "a, input[type=submit] {"
            "font-size: 20px;"
            "}"
            "</style>"
            "</head>"
            "<body>"
            "<img src=\"%s\" alt=\"Nonna\"/>"
            "<form action=\"/submit\" method=\"get\">"
            "<table>"
            "<tr><th>Parameter</th><th>Value</th></tr>"
            "<tr><td>Speed</td><td><input type=\"text\" name=\"param_speed\" value=\"%f\"></td></tr>"
            "<tr><td>KP</td><td><input type=\"text\" name=\"param_kp\" value=\"%f\"></td></tr>"
            "<tr><td>KI</td><td><input type=\"text\" name=\"param_ki\" value=\"%f\"></td></tr>"
            "<tr><td>KD</td><td><input type=\"text\" name=\"param_kd\" value=\"%f\"></td></tr>"
            "<tr><td>Control Mode</td><td>"
            "<select name=\"param_mode\">"
            "<option value=\"1\" %s>Neural Network</option>"
            "<option value=\"0\" %s>Classic PID</option>"
            "</select>"
            "</td></tr>"
            "</table><br/><br/>"
            "<input type=\"submit\" value=\"Change params\" style=\"background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer;\"><br/><br/>"
            "</form><br/>"
            "<a href=\"/reset\">Restart board</a><br/><br/><br/>"
            "%s" // Insert the start/stop button dynamically
            "</body>"
            "</html>",
            NONNA_LOGO, param_speed, param_kp, param_ki, param_kd,
            param_mode == 1 ? "selected" : "", // Selected attribute for Neural Network
            param_mode == 0 ? "selected" : "", // Selected attribute for Classic PID
            start_stop_button);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t submit_handler(httpd_req_t *req)
{
    char buf[256];
    int ret, remaining = req->content_len;
    while (remaining > 0)
    {
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                httpd_resp_send_408(req);
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }

    char param_speed_str[10];
    char param_kp_str[10];
    char param_ki_str[10];
    char param_kd_str[10];
    char param_mode_str[10];

    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK)
    {
        if (httpd_query_key_value(buf, "param_speed", param_speed_str, sizeof(param_speed_str)) == ESP_OK)
        {
            param_speed = atof(param_speed_str);
            // control_loop_set_forward(param_speed);
            ESP_LOGI(TAG, "param_speed: %f", param_speed);
            nvs_set_blob(my_handle, "param_speed", &param_speed, sizeof(double));
        }

        if (httpd_query_key_value(buf, "param_kp", param_kp_str, sizeof(param_kp_str)) == ESP_OK)
        {
            param_kp = atof(param_kp_str);
            // turn_pid_set_p(param_kp);
            ESP_LOGI(TAG, "param_kp: %f", param_kp);
            nvs_set_blob(my_handle, "param_kp", &param_kp, sizeof(double));
        }

        if (httpd_query_key_value(buf, "param_ki", param_ki_str, sizeof(param_ki_str)) == ESP_OK)
        {
            param_ki = atof(param_ki_str);
            ESP_LOGI(TAG, "param_ki: %f", param_ki);
            nvs_set_blob(my_handle, "param_ki", &param_ki, sizeof(double));
        }

        if (httpd_query_key_value(buf, "param_kd", param_kd_str, sizeof(param_kd_str)) == ESP_OK)
        {
            param_kd = atof(param_kd_str);
            // turn_pid_set_d(param_kd);
            ESP_LOGI(TAG, "param_kd: %f", param_kd);
            nvs_set_blob(my_handle, "param_kd", &param_kd, sizeof(double));
        }

        if (httpd_query_key_value(buf, "param_mode", param_mode_str, sizeof(param_mode_str)) == ESP_OK)
        {
            param_mode = atoi(param_mode_str);
            ESP_LOGI(TAG, "param_mode: %ld", param_mode);
            nvs_set_i32(my_handle, "param_mode", param_mode);
        }
    }
    else
    {
        ESP_LOGE(TAG, "Error getting the query string");
    }

    nvs_commit(my_handle);

    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t reset_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    esp_restart();
    return ESP_OK;
}

static esp_err_t start_handler(httpd_req_t *req)
{
    param_enabled = true;
    // control_loop_set_enable(1);
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t stop_handler(httpd_req_t *req)
{
    param_enabled = false;
    // control_loop_set_enable(0);
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

void param_store_init(void)
{
    ESP_ERROR_CHECK(nvs_open(PART_NAME, NVS_READWRITE, &my_handle));
    read_params_from_nvs();
}

void param_store_start(void)
{
    start_mdns_service();

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    config.core_id = PRO_CPU_NUM;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_index,
        .user_ctx = NULL};

    httpd_uri_t submit_uri = {
        .uri = "/submit",
        .method = HTTP_GET,
        .handler = submit_handler,
        .user_ctx = NULL};
    httpd_uri_t reset_uri = {
        .uri = "/reset",
        .method = HTTP_GET,
        .handler = reset_handler,
        .user_ctx = NULL};
    httpd_uri_t start_uri = {
        .uri = "/start",
        .method = HTTP_GET,
        .handler = start_handler,
        .user_ctx = NULL};
    httpd_uri_t stop_uri = {
        .uri = "/stop",
        .method = HTTP_GET,
        .handler = stop_handler,
        .user_ctx = NULL};
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &submit_uri);
        httpd_register_uri_handler(server, &reset_uri);
        httpd_register_uri_handler(server, &start_uri);
        httpd_register_uri_handler(server, &stop_uri);
    }
}
