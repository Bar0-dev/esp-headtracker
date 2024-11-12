#ifndef BLT_STACK_H
#define BLT_STACK_H

#include <stddef.h>
#include <stdint.h>

#define LOCAL_DEVICE_NAME "ESP-headtracker"
void bt_stack_init(void);
void bt_stack_write(uint8_t *msg, size_t size);

#endif
