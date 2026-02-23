#include "mtime.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"

static const char* TAG = "MTime";

void init_time(DelayFunction delay){
    ESP_LOGI(TAG, "Init Time");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");

    esp_netif_sntp_init(&config);

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    setenv("TZ", "ART3" ,1);
    tzset();

    //tm_year => current_Year = 1900 + timeinfo.tm.year
    while (timeinfo.tm_year < (2025 - 1900) && ++retry < 20){
        ESP_LOGI(TAG, "Waiting for system time to bet set ...(%d)", retry);
        delay();
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
}

int32_t get_time(){
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
}