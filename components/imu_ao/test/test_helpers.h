#ifndef TEST_HELPERS
#define TEST_HELPERS
#include "../imu_hal.h"
#include <stdint.h>

typedef void (*function_ptr_t)();
typedef void (*function1_ptr_t)(void *);
typedef void (*function2_ptr_t)(void *, void *);
uint32_t measure_execution_time(function_ptr_t func, char *funcName);
uint32_t measure_execution_time1(function1_ptr_t func, void *args,
                                 char *funcName);
uint32_t measure_execution_time2(function2_ptr_t func, void *arg1, void *arg2,
                                 char *funcName);
void set_buffer_instance(Buffer_t *buffer, ImuData_t read);

#endif // !TEST_HELPERS
