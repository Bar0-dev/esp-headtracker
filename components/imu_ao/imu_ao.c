#include "imu_ao.h"
#include "core.h"

//Forward declarations
State Imu_init(Imu * const me, Event const * const e);
State Imu_top(Imu * const me, Event const * const e);
State Imu_idle(Imu * const me, Event const * const e);
State Imu_read(Imu * const me, Event const * const e);
State Imu_calibration(Imu * const me, Event const * const e);
State Imu_cal_accel(Imu * const me, Event const * const e);
State Imu_cal_gyro(Imu * const me, Event const * const e);
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
        get_accel_scale_and_bias(me->calibration.accel.scale, me->calibration.accel.bias);
        get_gyro_bias(me->calibration.gyro.bias);
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
        status = transition(&me->super.super, (StateHandler)&Imu_cal_gyro);
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
    Packet_t packet = {.length = 0};
    ImuData_t read;
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_READING;
        Active_post(AO_Broker, &evt);
        TimeEvent_arm(&me->readTimer);
        status = HANDLED_STATUS;
        break;

    case IMU_READ_TIMEOUT_SIG:
        imu_read(read);
        accelApplyBiasAndScale(read, &me->calibration.accel);
        gyroApplyBias(read, &me->calibration.gyro);
        preparePacket(read, &packet);
        evt.sig = EV_IMU_SEND_DATA;
        evt.payload = &packet;
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
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        if(me->calibration.accel.completed){
            set_accel_scale_and_bias(me->calibration.accel.scale, me->calibration.accel.bias);
            me->calibration.accel.completed = false;
        }
        if(me->calibration.gyro.completed){
            set_gyro_bias(me->calibration.gyro.bias);
            me->calibration.gyro.completed = false;
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

    ImuData_t read;
    static AccelCalibrationBuffer_t buffer;
    static Axis_t axis = NO_AXIS;
    static char axisName[6];
    static Direction_t direction = NO_DIRECTION;

    switch (e->sig)
    {
    case ENTRY_SIG:
        accelBufferClear(&buffer);
        TimeEvent_arm(&me->preCalibrationTimer);
        axis = X_AXIS;
        direction = POSITIVE;
        getAxisName(axis, axisName);
        ESP_LOGI("IMU_CALIBRATION", "Accelerometer calibration on axis: %s starts in: %ds", axisName, (uint8_t)(PRE_CALIBRATION_PERIOD/1000));
        status = HANDLED_STATUS;
        break;

    case IMU_PRE_CALIBRATION_TIMEOUT_SIG:
        evt.sig = EV_IMU_CALIBRATION_IN_PROGRESS;
        Active_post(AO_Broker, &evt);
        TimeEvent_arm(&me->readTimer);
        TimeEvent_arm(&me->calibrationTimer);
        status = HANDLED_STATUS;
        break;

    case IMU_READ_TIMEOUT_SIG:
        imu_read(read);
        accelUpdateBuffer(read, &buffer, axis, direction);
        status = HANDLED_STATUS;
        break;

    case IMU_CALIBRATION_TIMEOUT_SIG:
        TimeEvent_disarm(&me->readTimer);
        direction++;
        if(direction >= NO_DIRECTION){
            direction = POSITIVE;
            axis++;
        }
        if(axis >= NO_AXIS){
            ESP_LOGI("IMU_CALIBRATION", "Accelerometer calibration finished");
            evt.sig = EV_IMU_CALIBRATION_DONE;
            Active_post(AO_Broker, &evt);
            status = transition(&me->super.super, (StateHandler)&Imu_calibration);
        } else {
            getAxisName(axis, axisName);
            ESP_LOGI("IMU_CALIBRATION", "Accelerometer calibration on axis: %s starts in: %ds", axisName, (uint8_t)(PRE_CALIBRATION_PERIOD/1000));
            TimeEvent_arm(&me->preCalibrationTimer);
            status = HANDLED_STATUS;
        }
        break;
    
    case EXIT_SIG:
        TimeEvent_disarm(&me->readTimer);
        TimeEvent_disarm(&me->calibrationTimer);
        TimeEvent_disarm(&me->preCalibrationTimer);
        accelCalculateBiasAndScale(&buffer, &me->calibration.accel);
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_calibration);
        break;
    }
    return status;
}

State Imu_cal_gyro(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    ImuData_t read;
    static GyroCalibrationBuffer_t buffer;

    switch (e->sig)
    {
    case ENTRY_SIG:
        TimeEvent_arm(&me->preCalibrationTimer);
        gyroBufferClear(&buffer);
        ESP_LOGI("IMU_CALIBRATION", "Gyro calibration starts in: %ds", (uint8_t)(PRE_CALIBRATION_PERIOD/1000));
        status = HANDLED_STATUS;
        break;
    
    case IMU_PRE_CALIBRATION_TIMEOUT_SIG:
        evt.sig = EV_IMU_CALIBRATION_IN_PROGRESS;
        Active_post(AO_Broker, &evt);
        TimeEvent_arm(&me->readTimer);
        TimeEvent_arm(&me->calibrationTimer);
        status = HANDLED_STATUS;
        break;

    case IMU_READ_TIMEOUT_SIG:
        imu_read(read);
        gyroUpdateBuffer(read, &buffer);
        status = HANDLED_STATUS;
        break;

    case IMU_CALIBRATION_TIMEOUT_SIG:
        TimeEvent_disarm(&me->readTimer);
        ESP_LOGI("IMU_CALIBRATION", "Gyro calibration finished");
        evt.sig = EV_IMU_CALIBRATION_DONE;
        Active_post(AO_Broker, &evt);
        status = transition(&me->super.super, (StateHandler)&Imu_calibration);
        break;

    case EXIT_SIG:
        gyroCalculateBias(&buffer, &me->calibration.gyro);
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
    calibrationSetNotCompleted(&me->calibration);
}