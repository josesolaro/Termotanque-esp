#include "wifi.h"
#include "string.h"
static const char* WIGI_TAG = "WIFI_INIT";

int8_t s_retry_num = 0;
EventGroupHandle_t s_wifi_event_group;

void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    
    ESP_LOGI(WIGI_TAG, "Got eventID: %ld", event_id);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(WIGI_TAG, "Connect..");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WIGI_TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(WIGI_TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIGI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_init_sta(const char* ssid, const char* password)
{
    if(esp_netif_init() == ESP_FAIL){
        return ESP_FAIL;
    }

    if(esp_event_loop_create_default() == ESP_FAIL){
        return ESP_FAIL;
    }

    s_wifi_event_group = xEventGroupCreate();

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if(esp_wifi_init(&cfg) == ESP_FAIL){
        return ESP_FAIL;
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    if(esp_event_handler_instance_register(
        WIFI_EVENT, 
        ESP_EVENT_ANY_ID, 
        &event_handler, 
        NULL, 
        &instance_any_id) == ESP_FAIL){
            return ESP_FAIL;
        }

    if(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_handler,
        NULL,
        &instance_got_ip
    ) == ESP_FAIL){
        return ESP_FAIL;
    }

    wifi_config_t config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        }
    };

    strncpy((char*)config.sta.ssid, ssid, sizeof(config.sta.ssid));
    strncpy((char*)config.sta.password, password, sizeof(config.sta.password));

    if(esp_wifi_set_mode(WIFI_MODE_STA) == ESP_FAIL){
        return ESP_FAIL;
    }
    if(esp_wifi_set_config(WIFI_IF_STA, &config) == ESP_FAIL){
        return ESP_FAIL;
    }
    if(esp_wifi_start() == ESP_FAIL){
        return ESP_FAIL;
    }

    ESP_LOGI(WIGI_TAG, "Wifi_init_sta finished");

    //wait for connection to happen
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
        WIFI_FAIL_BIT | WIFI_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        10000);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(WIGI_TAG, "connected to ap SSID:%s",
                 ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(WIGI_TAG, "Failed to connect to SSID:%s",
                 ssid);
        return ESP_FAIL;
    } else {
        ESP_LOGE(WIGI_TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}

void m_wifi_deinit(){
    esp_event_loop_delete_default();
    esp_wifi_disconnect();
    esp_wifi_deinit();
}