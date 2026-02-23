#include "mq.h"
#include "esp_log.h"
#include "math.h"

#define PUBLISH_TOPIC "termotanque/temperature"
#define SUBSCRIBE_TOPIC "termotanque/relay"

static char last_state_char[32];
static const char* TAG = "MQTT";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            esp_mqtt_client_subscribe_single(event->client, SUBSCRIBE_TOPIC, 1);
        break;
        case MQTT_EVENT_DATA:
        //latest message
            if (strcmp(event->topic, SUBSCRIBE_TOPIC) == 0){
                int len = fmin(event->data_len, sizeof(last_state) - 1);
                memcpy(last_state_char, event->data, len);
                last_state_char[len] = 0;
                if (strcmp(last_state_char, "ON") == 0){
                    last_state.on = true;
                } else {
                    last_state.on = false;
                }
            }
        break;
        default:
            ESP_LOGI(TAG, "Other event id:%ld", event_id);
        break;
    }
}

esp_err_t mqtt_init(const char* broker_url, esp_mqtt_client_handle_t* client){
    esp_mqtt_client_config_t config = {
        .broker.address.uri = broker_url,
        .broker.address.port = 1883
    };
    *client = esp_mqtt_client_init(&config);

    esp_mqtt_client_register_event(*client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    return esp_mqtt_client_start(*client);
}

void mqtt_disconnect(esp_mqtt_client_handle_t client){
    esp_mqtt_client_disconnect(client);
    esp_mqtt_client_destroy(client);
}

void mqtt_send_temperature(esp_mqtt_client_handle_t client, int8_t temperature){
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", temperature);
    esp_mqtt_client_publish(client, PUBLISH_TOPIC, buffer, 0, 1, 0);
}
