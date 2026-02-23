#include "mqtt_client.h"

typedef struct {
    bool on;
} relay_data;

static relay_data last_state;
esp_err_t mqtt_init(const char* broker_url, esp_mqtt_client_handle_t* client);
void mqtt_disconnect(esp_mqtt_client_handle_t client);
void mqtt_send_temperature(esp_mqtt_client_handle_t client, int8_t temperature);