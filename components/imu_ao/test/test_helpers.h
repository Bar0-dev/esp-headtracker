#ifndef TEST_HELPERS
#define TEST_HELPERS
#include "../imu_hal.h"
#include <stdint.h>

typedef void (*function_ptr_t)();
typedef void (*function1_ptr_t)(void *);
uint32_t measure_execution_time(function_ptr_t func, char *funcName);
uint32_t measure_execution_time1(function1_ptr_t func, void *args,
                                 char *funcName);
void set_buffer_instance(Buffer_t *buffer, ImuData_t read);

#endif // !TEST_HELPERS
