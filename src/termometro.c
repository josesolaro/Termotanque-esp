#include "termometro.h"
#include "esp_log.h"
#include "math.h"

static const char* TAG = "Termometro";

onewire_bus_handle_t init_termometro(int GPIO_pin)
{
    // install new 1-wire bus
    onewire_bus_handle_t bus;
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = GPIO_pin,
        .flags = {
            .en_pull_up = 1, // enable internal pull-up resistor
        }
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10, // 1byte ROM command + 8byte ROM number + 1byte device command
    };
    ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
    ESP_LOGI(TAG, "1-Wire bus installed on GPIO%d", GPIO_pin);
    return bus;
}

void onewire_disconect(onewire_bus_handle_t bus){
    onewire_bus_del(bus);
}

int8_t read_temperature(onewire_bus_handle_t bus, CallbackFunction delay)
{
    //start convertion
    onewire_bus_reset(bus);

    //skip ROM
    uint8_t txData[1] = {0xCC};
    onewire_bus_write_bytes(bus, txData, sizeof(txData));

    //start convertion
    txData[0] = 0x44;
    onewire_bus_write_bytes(bus, txData, sizeof(txData));

    //delay 1 s
    delay();

    //read temperature
    onewire_bus_reset(bus);

    //skip ROM
    txData[0] = 0xCC;
    onewire_bus_write_bytes(bus, txData, sizeof(txData));

    //read scratchpad
    txData[0] = 0xBE;
    onewire_bus_write_bytes(bus, txData, sizeof(txData));

    //read
    uint8_t txRead[9];
    onewire_bus_read_bytes(bus, txRead, sizeof(txRead));

    // LSB MSB
    //02 02 55 05 7f a5 81 66 49
    //03 02 55 05 7f a5 81 66 0a

    uint8_t lsb = txRead[0];
    uint8_t msb = txRead[1];

    int8_t temp = 0;
    temp += msb * 16;
    temp += (lsb >> 4);

    return temp;
}

   