#include "imu_ao.h"

// static void clearCalibrationOffsets(Imu * const me)
// {
//     for(uint8_t sensor = ACCEL; sensor<NO_SENSOR; sensor++){
//         for(uint8_t axis = X_AXIS; axis<NO_AXIS; axis++){
//             me->calibration.offsets[sensor][axis] = 0;
//         }
//     } 
// }

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
        get_accel_offsets(me->calibration.accelScale, me->calibration.accelBias);
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        status = transition(&me->super.super, (StateHandler)&Imu_read);
        break;

    case EV_BUTTON_HOLD:
        status = transition(&me->super.super, (StateHandler)&Imu_calibration);
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
    ImuData_t data;
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_READING;
        Active_post(AO_Broker, &evt);
        TimeEvent_arm(&me->readTimer);
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        status = transition(&me->super.super, (StateHandler)&Imu_idle);
        break;

    case IMU_READ_TIMEOUT_SIG:
        imu_read(data);
        imu_apply_accel_offsets(data, me->calibration.accelScale, me->calibration.accelBias);
        imu_log_data(data, ACCEL, true);
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
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_CALIBRATION_READY;
        Active_post(AO_Broker, &evt);
        //controll the calibrated sensor
        if(me->calibration.sensor < NO_SENSOR){
            me->calibration.sensor++;
        } else {
            me->calibration.sensor = ACCEL;
        }
        //reset accel axis
        me->calibration.accelCalAxis = X_POS;
        //clear calibration offsets
        // clearCalibrationOffsets(me);
        ESP_LOGI("IMU_CALIBRATION", "Sensor: %d", me->calibration.sensor);
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        switch (me->calibration.sensor)
        {
        case ACCEL:
            status = transition(&me->super.super, (StateHandler)&Imu_cal_accel);
            break;

        case GYRO:
            // status = transition(&me->super.super, (StateHandler)&Imu_cal_gyro);
            status = HANDLED_STATUS;
            break;

        case MAG:
            // status = transition(&me->super.super, (StateHandler)&Imu_cal_mag);
            status = HANDLED_STATUS;
            break;

        default:
            status = HANDLED_STATUS;
            break;
        }
        break;

    case EV_BUTTON_DOUBLE_PRESS:
        status = transition(&me->super.super, (StateHandler)&Imu_idle);
        break;

    case EXIT_SIG:
        if(me->calibration.completed){
            store_accel_offsets(me->calibration.accelScale, me->calibration.accelBias);
            me->calibration.completed = false;
        }
        me->calibration.sensor = NO_SENSOR;
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
    switch (e->sig)
    {
    case ENTRY_SIG:
        ESP_LOGI("IMU_CALIBRATION", "Accelerometer calibration accelCalAxis: %d", me->calibration.accelCalAxis);
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        if(me->calibration.accelCalAxis < ACCEL_NO_AXIS){
            TimeEvent_arm(&me->preCalibrationTimer);
        }
        status = HANDLED_STATUS;
        break;

    case IMU_PRE_CALIBRATION_TIMEOUT_SIG:
        status = transition(&me->super.super, (StateHandler)&Imu_cal_axis_accel);
        break;

    case EXIT_SIG:
        //calculate the final bias from POS and NEG offsets
        if(me->calibration.accelCalAxis >= ACCEL_NO_AXIS){
            imu_calc_scale_and_bias(me->calibration.accelScale, me->calibration.accelBias, me->calibration.accelOffsets);
            me->calibration.completed = true;
        }
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_calibration);
        break;
    }
    return status;
}

State Imu_cal_axis_accel(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    int16_t buffer = 0;
    static int32_t sum = 0;
    static uint16_t samples = 0;
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_CALIBRATION_IN_PROGRESS;
        Active_post(AO_Broker, &evt);
        sum = 0;
        samples = 0;
        me->calibration.accelOffsets[me->calibration.accelCalAxis] = 0;
        TimeEvent_arm(&me->readTimer);
        TimeEvent_arm(&me->calibrationTimer);
        status = HANDLED_STATUS;
        break;

    case IMU_READ_TIMEOUT_SIG:
        imu_read_accel_axis(&buffer, me->calibration.accelCalAxis);
        sum += buffer;
        samples++;
        status = HANDLED_STATUS;
        break;

    case IMU_CALIBRATION_TIMEOUT_SIG:
        me->calibration.accelOffsets[me->calibration.accelCalAxis] = sum/samples;
        status = transition(&me->super.super, (StateHandler)&Imu_cal_accel);
        break;
    
    case EV_BUTTON_PRESSED:
        //handle this event to block bubbling while sampling
        status = HANDLED_STATUS;
        break;

    case EXIT_SIG:
        TimeEvent_disarm(&me->readTimer);
        TimeEvent_disarm(&me->calibrationTimer); //only disarm if calibration was aborted by holding the button
        evt.sig = EV_IMU_CALIBRATION_DONE;
        Active_post(AO_Broker, &evt);
        ESP_LOGI("IMU_CALIBRATION", "Accelerometer offset: %d, for sequence: %d", me->calibration.accelOffsets[me->calibration.accelCalAxis], me->calibration.accelCalAxis);
        if(me->calibration.accelCalAxis < ACCEL_NO_AXIS){
            me->calibration.accelCalAxis++;
        }
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_cal_accel);
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

    me->calibration.sensor = NO_SENSOR;
    me->calibration.accelCalAxis = ACCEL_NO_AXIS;
    me->calibration.completed = false;
    // clearCalibrationOffsets(me);
}