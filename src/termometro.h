#include "onewire_bus.h"
#include "onewire_device.h"

typedef void (*CallbackFunction)();

onewire_bus_handle_t init_termometro(int GPIO_pin);
void onewire_disconect(onewire_bus_handle_t bus);
int8_t read_temperature(onewire_bus_handle_t bus, CallbackFunction delay);