#include "time.h"

typedef void (*DelayFunction)();

void init_time(DelayFunction delay);
int32_t get_time();