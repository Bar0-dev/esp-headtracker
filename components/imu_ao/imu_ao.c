#include "imu_ao.h"

// static void clearCalibrationOffsets(Imu * const me)
// {
//     for(uint8_t sensor = ACCEL; sensor<NO_SENSOR; sensor++){
//         for(uint8_t axis = X_AXIS; axis<NO_AXIS; axis++){
//             me->calibration.offsets[sensor][axis] = 0;
//         }
//     } 
// }

// TODO: Move this to hal
typedef char axisString_t[20];

axisString_t accelAxisNames[] = {
    "X_POS",
    "X_NEG",
    "Y_POS",
    "Y_NEG",
    "Z_POS",
    "Z_NEG",
    "ACCEL_NO_AXIS",
};

//Forward declarations
State Imu_init(Imu * const me, Event const * const e);
State Imu_top(Imu * const me, Event const * const e);
State Imu_idle(Imu * const me, Event const * const e);
State Imu_read(Imu * const me, Event const * const e);
State Imu_calibration(Imu * const me, Event const * const e);
State Imu_cal_accel(Imu * const me, Event const * const e);
State Imu_cal_axis_accel(Imu * const me, Event const * const e);
// State Imu_cal_gyro(Imu * const me, Event const * const e);
// State Imu_cal_mag(Imu * const me, Event const * const e);

State Imu_init(Imu * const me, Event const * const e)
{
    return transition(&me->super.super, (StateHandler)&Imu_idle);
}

State Imu_top(Imu * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        //load offsets from nvs if they exist
        get_accel_offsets(me->calibration.accel.scale, me->calibration.accel.bias);
        status = HANDLED_STATUS;
        break;

    case EV_CONTROLLER_START_READING_IMU:
        status = transition(&me->super.super, (StateHandler)&Imu_read);
        break;

    case EV_CONTROLLER_START_CALIBRATION_IMU:
        status = transition(&me->super.super, (StateHandler)&Imu_calibration);
        break;

    case EV_CONTROLLER_STOP_READING_IMU:
        status = transition(&me->super.super, (StateHandler)&Imu_idle);
        break;

    case EV_CONTROLLER_CALIBRATE_ACCEL:
        status = transition(&me->super.super, (StateHandler)&Imu_cal_accel);
        break;

    case EV_CONTROLLER_CALIBRATE_GYRO:
        // status = transition(&me->super.super, (StateHandler)&Imu_cal_gyro);
        status = HANDLED_STATUS;
        break;

    case EV_CONTROLLER_CALIBRATE_MAG:
        // status = transition(&me->super.super, (StateHandler)&Imu_cal_mag);
        status = HANDLED_STATUS;
        break;

    case EV_CONTROLLER_STOP_CALIBRATION_IMU:
        status = transition(&me->super.super, (StateHandler)&Imu_idle);
        break;

    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Hsm_top);
        break;
    }
    return status;
}

State Imu_idle(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_IDLE;
        Active_post(AO_Broker, &evt);
        status = HANDLED_STATUS;
        break;

    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_top);
        break;
    }
    return status;
}

State Imu_read(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    char *payload = "TESTDATA1";
    ImuData_t data;
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_READING;
        Active_post(AO_Broker, &evt);
        TimeEvent_arm(&me->readTimer);
        status = HANDLED_STATUS;
        break;

    case IMU_READ_TIMEOUT_SIG:
        imu_read(data);
        imu_apply_accel_offsets(data, me->calibration.accel.scale, me->calibration.accel.bias);
        imu_log_data(data, ACCEL, true);
        evt.sig = EV_IMU_SEND_DATA;
        evt.payload = payload;
        Active_post(AO_Broker, &evt);
        status = HANDLED_STATUS;
        break;

    case EXIT_SIG:
        TimeEvent_disarm(&me->readTimer);
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_top);
        break;
    }
    return status;
}

State Imu_calibration(Imu * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        ESP_LOGI("CALIBRATION", "Entered");
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        if(me->calibration.accel.completed){
            store_accel_offsets(me->calibration.accel.scale, me->calibration.accel.bias);
            me->calibration.accel.completed = false;
        }
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_top);
        break;
    }
    return status;
}

State Imu_cal_accel(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };

    int16_t buffer = 0;
    static int32_t sum = 0;
    static uint16_t samples = 0;

    switch (e->sig)
    {
    case ENTRY_SIG:
        TimeEvent_arm(&me->preCalibrationTimer);
        me->calibration.accel.axis = X_POS;
        ESP_LOGI("IMU_CALIBRATION", "Accelerometer calibration on axis: %s starts in: %dms", accelAxisNames[me->calibration.accel.axis], PRE_CALIBRATION_PERIOD);
        status = HANDLED_STATUS;
        break;

    case IMU_PRE_CALIBRATION_TIMEOUT_SIG:
        evt.sig = EV_IMU_CALIBRATION_IN_PROGRESS;
        Active_post(AO_Broker, &evt);
        sum = 0;
        samples = 0;
        me->calibration.accel.offsets[me->calibration.accel.axis] = 0;
        TimeEvent_arm(&me->readTimer);
        TimeEvent_arm(&me->calibrationTimer);
        status = HANDLED_STATUS;
        break;

    case IMU_READ_TIMEOUT_SIG:
        imu_read_accel_axis(&buffer, me->calibration.accel.axis);
        sum += buffer;
        samples++;
        status = HANDLED_STATUS;
        break;

    case IMU_CALIBRATION_TIMEOUT_SIG:
        TimeEvent_disarm(&me->readTimer);
        me->calibration.accel.offsets[me->calibration.accel.axis] = sum/samples;
        me->calibration.accel.axis++;
        if(me->calibration.accel.axis >= ACCEL_NO_AXIS){
            ESP_LOGI("IMU_CALIBRATION", "Accelerometer calibration finished");
            evt.sig = EV_IMU_CALIBRATION_DONE;
            Active_post(AO_Broker, &evt);
            status = transition(&me->super.super, (StateHandler)&Imu_calibration);
        } else {
            ESP_LOGI("IMU_CALIBRATION", "Accelerometer calibration on axis: %s starts in: %dms", accelAxisNames[me->calibration.accel.axis], PRE_CALIBRATION_PERIOD);
            TimeEvent_arm(&me->preCalibrationTimer);
            status = HANDLED_STATUS;
        }
        break;
    
    case EXIT_SIG:
        TimeEvent_disarm(&me->readTimer);
        TimeEvent_disarm(&me->calibrationTimer);
        TimeEvent_disarm(&me->preCalibrationTimer);
        //calculate the final bias from POS and NEG offsets
        if(me->calibration.accel.axis >= ACCEL_NO_AXIS){
            imu_calc_scale_and_bias(me->calibration.accel.scale, me->calibration.accel.bias, me->calibration.accel.offsets);
            me->calibration.accel.completed = true;
        }
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_calibration);
        break;
    }
    return status;
}

void Imu_ctor(Imu * const me)
{
    Active_ctor(&me->super, (StateHandler)&Imu_init);
    TimeEvent_ctor(&me->readTimer, "IMU read timer", (TickType_t)(READ_PERIOD/portTICK_PERIOD_MS), pdTRUE, IMU_READ_TIMEOUT_SIG, &me->super);
    TimeEvent_ctor(&me->calibrationTimer, "IMU calibration timer", (TickType_t)(ACCEL_GYRO_CALIBRATION_PERIOD/portTICK_PERIOD_MS), pdFALSE, IMU_CALIBRATION_TIMEOUT_SIG, &me->super);
    TimeEvent_ctor(&me->preCalibrationTimer, "IMU pre calibration timer", (TickType_t)(PRE_CALIBRATION_PERIOD/portTICK_PERIOD_MS), pdFALSE, IMU_PRE_CALIBRATION_TIMEOUT_SIG, &me->super);
    imu_hal_init();

    me->calibration.accel.axis = ACCEL_NO_AXIS;
    me->calibration.accel.completed = false;
    // clearCalibrationOffsets(me);
}