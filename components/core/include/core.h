#ifndef CORE_H
#define CORE_H

#include <stdint.h>

#define MAX_PACKET_SIZE 88
#define MAX_SINGLE_READING_SIZE 8

typedef struct
{
    uint8_t length;
    char payload[MAX_PACKET_SIZE];
} Packet_t;

typedef enum
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS, 
    NO_AXIS
} Axis_t;

typedef enum
{
    ACCEL,
    GYRO,
    MAG,
    NO_SENSOR
} Sensor_t;

typedef enum
{
    POSITIVE,
    NEGATIVE,
    NO_DIRECTION
} Direction_t;

typedef int16_t Vector16_t[NO_AXIS];
typedef int32_t Vector32_t[NO_AXIS];


#endif