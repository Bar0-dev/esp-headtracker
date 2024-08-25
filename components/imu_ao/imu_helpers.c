#include "imu_helpers.h"
#include <string.h>

//SLOW!!
static float map_int16_to_range(int16_t value, int16_t range)
{
    return ((float)value-INT16_MIN)*2*range/(INT16_MAX-INT16_MIN)-range;
}
//SLOW!!

//SLOW
static void convert_raw(ImuData_t raw, float data[NO_SENSOR][NO_AXIS])
{
    uint16_t range;
    for(SensorType_t sensor = ACCEL; sensor < NO_SENSOR; sensor++){
        switch (sensor)
        {
        case ACCEL:
            range = (int16_t)imu_get_accel_range();
            break;
        
        case GYRO:
            range = imu_get_gyro_range();
            break;
        
        case MAG:
            range = imu_get_mag_range();
            break;
        
        default:
            assert(0);
            break;
        }

        for(uint8_t axis=X_AXIS; axis<NO_AXIS; axis++){
            data[sensor][axis] = map_int16_to_range(raw[sensor][axis], range);
        }
    }
    return;
}
//SLOW

// void imu_log_data(ImuData_t data, SensorType_t sensor, bool convert)
// {
//     float data_to_show[NO_AXIS];
//     if(convert){
//         convert_raw(data, data_to_show, sensor);
//         ESP_LOGI("IMU", "X: %.2f, Y: %.2f, Z: %.2f", data_to_show[0], data_to_show[1], data_to_show[2]);
//     } else {
//         ESP_LOGI("IMU", "X: %d, Y: %d, Z: %d", data[sensor][0], data[sensor][1], data[sensor][2]);
//     }
// }

//TODO: move string and packet preping functionality to COMS AO and leave only conversion to ranged floats
void imu_process_data(ImuData_t data, packet_t * packet)
{
    float conveted_data[NO_SENSOR][NO_AXIS];
    char buffer[MAX_SINGLE_READING_SIZE];
    convert_raw(data, conveted_data);
    for(SensorType_t sensor = GYRO; sensor < MAG; sensor++){
        for(Axis_t axis=X_AXIS; axis<NO_AXIS; axis++){
            sprintf(buffer, "%.2f,", conveted_data[sensor][axis]);
            packet->length += strlen(buffer);
            strcat(packet->payload, buffer);
        }
    }
    strcat(packet->payload, "\n");
    packet->length++;
}

void imu_calc_scale_and_bias(int16_t scale[], int16_t bias[], int16_t accel_offsets[]){
    uint8_t range = imu_get_accel_range();
    int16_t ymax = INT16_MAX/range;
    int16_t pos_offset;
    int16_t neg_offset;

    for (int8_t axis = X_AXIS; axis<NO_AXIS; axis++){
        int16_t a = accel_offsets[axis<<1];
        int16_t b = accel_offsets[(axis<<1)+1];
        pos_offset = (a > b) ? a : b;
        neg_offset = (a < b) ? a : b;
        if((pos_offset-neg_offset)!=0){
            scale[axis] = (int16_t)(ACCEL_SCALE_FACTOR*(2.0*ymax/(pos_offset-neg_offset)));
            bias[axis] = -ymax*(pos_offset+neg_offset)/(pos_offset-neg_offset);
        }
        ESP_LOGI("IMU-HAL", "axis: %d, scale:%d, bias:%d", axis, scale[axis], bias[axis]);
    }
}

void imu_apply_accel_offsets(ImuData_t data, int16_t scales[], int16_t bias[]){
    SensorType_t sensor = ACCEL;
    for(uint8_t axis = X_AXIS; axis<NO_AXIS; axis++){
        data[sensor][axis] = (int16_t)(scales[axis]*data[sensor][axis]/ACCEL_SCALE_FACTOR) + bias[axis];
    }
}