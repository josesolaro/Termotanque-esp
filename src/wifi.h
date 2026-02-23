#include "esp_wifi.h"
#include "esp_log.h"

#define MAXIMUM_RETRY 10

#define WIFI_FAIL_BIT BIT1
#define WIFI_CONNECTED_BIT BIT0

esp_err_t wifi_init_sta(const char* ssid, const char* password);

void m_wifi_deinit();

void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);
