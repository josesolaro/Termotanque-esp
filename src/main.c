#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "termometro.h"
#include "mtime.h"
#include "mq.h"

#define LED_GPIO GPIO_NUM_2
#define RELAY_GPIO GPIO_NUM_3
#define ONEWIRE_GPIO_PIN  GPIO_NUM_0

#define TAG "MAIN_APP"
#define BROKER_URL "mqtt://192.168.0.25"

#define TIME_TO_SLEEP 60000000ULL  // seconds

#define STATIC_ON_TIME 57600 //16hs
#define STATIC_OFF_TIME 68400 //19hs

const char* WIFI_SSID = "HITRON-4A50_EXT";
const char* WIFI_PASSWORD =  "N8DM8CGH12M1";

RTC_DATA_ATTR int32_t current_time = 0;
RTC_DATA_ATTR int64_t sleep_enter_time =0;

void callback_delay();

void app_main()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_FAIL) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP);

    gpio_reset_pin(LED_GPIO);
    gpio_reset_pin(RELAY_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);

    //gpio_set_level(LED_GPIO, 1); // LED ON
    
    onewire_bus_handle_t termometro = init_termometro(ONEWIRE_GPIO_PIN);
    int temperature = read_temperature(termometro, callback_delay);
    onewire_disconect(termometro);

    if (wifi_init_sta(WIFI_SSID, WIFI_PASSWORD) == ESP_OK){
        init_time(callback_delay);

        esp_mqtt_client_handle_t mqtt;
        if(mqtt_init(BROKER_URL, &mqtt) == ESP_OK){
            mqtt_send_temperature(mqtt, temperature);
            if(last_state.on)
            {
                gpio_set_level(LED_GPIO, 1); // LED ON
                gpio_set_level(RELAY_GPIO, 1);
            }
            else
            {
                gpio_set_level(LED_GPIO, 0); // LED OFF
                gpio_set_level(RELAY_GPIO, 0);
            }
            mqtt_disconnect(mqtt);
        }
        current_time = get_time();
        m_wifi_deinit();
    }
    else 
    {
        int64_t sleep_exit_time = esp_timer_get_time();
        current_time += (sleep_exit_time - sleep_enter_time);
        ESP_LOGI(TAG, "Using local time: %ld", current_time);
        if(current_time > STATIC_ON_TIME && current_time < STATIC_OFF_TIME){
            gpio_set_level(RELAY_GPIO, 1);
        }else{
            gpio_set_level(RELAY_GPIO, 0);
        }
    }

    gpio_set_level(LED_GPIO, 0); // LED OFF
    sleep_enter_time = esp_timer_get_time();
    esp_deep_sleep_start();
    // esp_restart();
}

void callback_delay(){
    vTaskDelay(pdMS_TO_TICKS(1000));
}