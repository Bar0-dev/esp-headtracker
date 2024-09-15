#ifndef TEST_HELPERS
#define TEST_HELPERS

typedef void (*function_ptr_t)();
typedef void (*function1_ptr_t)(void *);
void measure_execution_time(function_ptr_t func, char *funcName);
void measure_execution_time1(function1_ptr_t func, void *args, char *funcName);

#endif // !TEST_HELPERS
