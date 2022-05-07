
#include "esp_wifi.h"
#include <string.h>
#include <stdlib.h>
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "wifi.h"

static const char *TAG = "WIFI";

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        _Connection_sts_wifi = 1;
        ESP_LOGI(TAG, "internet listo\n");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        _Connection_sts_wifi = 0;
        break;
    default:
        break;
    }
    return ESP_OK;
}

esp_err_t initialize_wifi(const char *wifi_name, const char *wifi_password)
{
    _Connection_sts_wifi = 0;
    int wifi_name_lenght = strlen(wifi_name);
    int wifi_password_lenght = strlen(wifi_password);
    if (wifi_name_lenght > 32)
        return -1;
    if (wifi_password_lenght > 64)
        return -2;
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    char wifi[32] = {0};
    memcpy(wifi, wifi_name, wifi_name_lenght * 4);
    char pass[64] = {0};
    memcpy(pass, wifi_password, wifi_password_lenght * 4);
    wifi_config_t wifi_config = {
        .sta = {
            .bssid_set = 0},
    };
    memcpy(wifi_config.sta.ssid, wifi, 32);
    memcpy(wifi_config.sta.password, pass, 64);
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    return esp_wifi_start();
}

uint8_t is_wifi_connected()
{
    return _Connection_sts_wifi;
}

void wait_wifi_Connection()
{
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

void disconnect_wifi()
{
    if (esp_wifi_disconnect() == ESP_OK)
    {
        esp_wifi_stop();
    }
}