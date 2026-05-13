#include "mqtt_client.h"


#ifndef RELAY_H
#define RELAY_H

typedef struct {
    bool on;
} relay_data;

extern relay_data last_state;

#endif

esp_err_t mqtt_init(const char* broker_url, esp_mqtt_client_handle_t* client);
void mqtt_disconnect(esp_mqtt_client_handle_t client);
void mqtt_send_temperature(esp_mqtt_client_handle_t client, int8_t temperature);
void mqtt_send_keep_alive(esp_mqtt_client_handle_t client, int32_t time);