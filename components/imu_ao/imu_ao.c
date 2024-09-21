#include "imu_ao.h"
#include "core.h"
#include "events_broker.h"
#include "imu_hal.h"
#include "imu_helpers.h"
#include <stdint.h>

// Forward declarations
State Imu_init(Imu *const me, Event const *const e);
State Imu_top(Imu *const me, Event const *const e);
State Imu_idle(Imu *const me, Event const *const e);
State Imu_read(Imu *const me, Event const *const e);
State Imu_calibration(Imu *const me, Event const *const e);
State Imu_cal_accel(Imu *const me, Event const *const e);
State Imu_cal_gyro(Imu *const me, Event const *const e);
// State Imu_cal_mag(Imu * const me, Event const * const e);

State Imu_init(Imu *const me, Event const *const e) {
  return transition(&me->super.super, (StateHandler)&Imu_idle);
}

State Imu_top(Imu *const me, Event const *const e) {
  State status;
  switch (e->sig) {
  case ENTRY_SIG:
    // load offsets from nvs if they exist
    get_accel_scale_and_bias(me->calibration.accel.scale,
                             me->calibration.accel.bias);
    get_gyro_bias(me->calibration.gyro.bias);
    loadMagTransformationMatrix(&me->calibration.mag);
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

  case EV_IMU_HAL_DATA_READY:
    imu_hal_update_dbuffer();
    status = HANDLED_STATUS;
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

State Imu_idle(Imu *const me, Event const *const e) {
  State status;
  Event evt = {LAST_EVENT_FLAG, (void *)0};
  switch (e->sig) {
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

State Imu_read(Imu *const me, Event const *const e) {
  State status;
  Event evt = {LAST_EVENT_FLAG, (void *)0};
  Packet_t packet = {.length = 0};
  Sensor_t sensor = MAG;
  switch (e->sig) {
  case ENTRY_SIG:
    imu_hal_init_dbuffer();
    imu_hal_enable_interrupt();
    evt.sig = EV_IMU_READING;
    Active_post(AO_Broker, &evt);
    status = HANDLED_STATUS;
    break;

  case EV_IMU_HAL_PROCESS_BUFFER:

    // accelApplyBiasAndScale(read, &me->calibration.accel);
    // gyroApplyBias(read, &me->calibration.gyro);
    // magApplyTransformMatrix(read, &me->calibration.mag);
    // prepareRawPacket(read, &packet);
    // evt.sig = EV_IMU_SEND_DATA;
    // evt.payload = &packet;
    // Active_post(AO_Broker, &evt);
    // TODO: add madgwick filter here
    Buffer_t *readBuffer = imu_hal_read_buffer();
    ESP_LOGI("DUBUG", "%lld", readBuffer->data[1].timeDelta);
    status = HANDLED_STATUS;
    break;

  case EXIT_SIG:
    imu_hal_disable_interrupt();
    status = HANDLED_STATUS;
    break;

  default:
    status = super(&me->super.super, (StateHandler)&Imu_top);
    break;
  }
  return status;
}

State Imu_calibration(Imu *const me, Event const *const e) {
  State status;
  switch (e->sig) {
  case ENTRY_SIG:
    status = HANDLED_STATUS;
    break;

  case EXIT_SIG:
    if (me->calibration.accel.completed) {
      set_accel_scale_and_bias(me->calibration.accel.scale,
                               me->calibration.accel.bias);
      me->calibration.accel.completed = false;
    }
    if (me->calibration.gyro.completed) {
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

State Imu_cal_accel(Imu *const me, Event const *const e) {
  State status;
  Event evt = {LAST_EVENT_FLAG, (void *)0};

  static AccelCalibrationBuffer_t buffer;
  static Axis_t axis = NO_AXIS;
  static char axisName[6];
  static Direction_t direction = NO_DIRECTION;

  switch (e->sig) {
  case ENTRY_SIG:
    imu_hal_init_dbuffer();
    imu_hal_enable_interrupt();
    accelBufferClear(&buffer);
    TimeEvent_arm(&me->preCalibrationTimer);
    axis = X_AXIS;
    direction = POSITIVE;
    getAxisName(axis, axisName);
    ESP_LOGI("IMU_CALIBRATION",
             "Accelerometer calibration on axis: %s starts in: %ds", axisName,
             (uint8_t)(PRE_CALIBRATION_PERIOD / 1000));
    status = HANDLED_STATUS;
    break;

  case IMU_PRE_CALIBRATION_TIMEOUT_SIG:
    evt.sig = EV_IMU_CALIBRATION_IN_PROGRESS;
    Active_post(AO_Broker, &evt);
    TimeEvent_arm(&me->calibrationTimer);
    status = HANDLED_STATUS;
    break;

  case IMU_CALIBRATION_TIMEOUT_SIG:
    direction++;
    if (direction >= NO_DIRECTION) {
      direction = POSITIVE;
      axis++;
    }
    if (axis >= NO_AXIS) {
      ESP_LOGI("IMU_CALIBRATION", "Accelerometer calibration finished");
      evt.sig = EV_IMU_CALIBRATION_DONE;
      Active_post(AO_Broker, &evt);
      status = transition(&me->super.super, (StateHandler)&Imu_calibration);
    } else {
      getAxisName(axis, axisName);
      ESP_LOGI("IMU_CALIBRATION",
               "Accelerometer calibration on axis: %s starts in: %ds", axisName,
               (uint8_t)(PRE_CALIBRATION_PERIOD / 1000));
      TimeEvent_arm(&me->preCalibrationTimer);
      status = HANDLED_STATUS;
    }
    break;

  case EV_IMU_HAL_PROCESS_BUFFER:
    Buffer_t *sensorBuffer = imu_hal_read_buffer();
    accelUpdateBuffer(sensorBuffer, &buffer, axis, direction);
    status = HANDLED_STATUS;
    break;

  case EXIT_SIG:
    imu_hal_disable_interrupt();
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

State Imu_cal_gyro(Imu *const me, Event const *const e) {
  State status;
  Event evt = {LAST_EVENT_FLAG, (void *)0};
  static GyroCalibrationBuffer_t buffer;

  switch (e->sig) {
  case ENTRY_SIG:
    imu_hal_init_dbuffer();
    imu_hal_enable_interrupt();
    TimeEvent_arm(&me->preCalibrationTimer);
    gyroBufferClear(&buffer);
    ESP_LOGI("IMU_CALIBRATION", "Gyro calibration starts in: %ds",
             (uint8_t)(PRE_CALIBRATION_PERIOD / 1000));
    status = HANDLED_STATUS;
    break;

  case IMU_PRE_CALIBRATION_TIMEOUT_SIG:
    evt.sig = EV_IMU_CALIBRATION_IN_PROGRESS;
    Active_post(AO_Broker, &evt);
    TimeEvent_arm(&me->calibrationTimer);
    status = HANDLED_STATUS;
    break;

  case IMU_CALIBRATION_TIMEOUT_SIG:
    ESP_LOGI("IMU_CALIBRATION", "Gyro calibration finished");
    evt.sig = EV_IMU_CALIBRATION_DONE;
    Active_post(AO_Broker, &evt);
    status = transition(&me->super.super, (StateHandler)&Imu_calibration);
    break;

  case EV_IMU_HAL_PROCESS_BUFFER:
    Buffer_t *sensorBuffer = imu_hal_read_buffer();
    gyroUpdateBuffer(sensorBuffer, &buffer);
    status = HANDLED_STATUS;
    break;

  case EXIT_SIG:
    imu_hal_disable_interrupt();
    gyroCalculateBias(&buffer, &me->calibration.gyro);
    status = HANDLED_STATUS;
    break;

  default:
    status = super(&me->super.super, (StateHandler)&Imu_calibration);
    break;
  }
  return status;
}

void Imu_ctor(Imu *const me) {
  Active_ctor(&me->super, (StateHandler)&Imu_init);
  TimeEvent_ctor(
      &me->calibrationTimer, "IMU calibration timer",
      (TickType_t)(ACCEL_GYRO_CALIBRATION_PERIOD / portTICK_PERIOD_MS), pdFALSE,
      IMU_CALIBRATION_TIMEOUT_SIG, &me->super);
  TimeEvent_ctor(&me->preCalibrationTimer, "IMU pre calibration timer",
                 (TickType_t)(PRE_CALIBRATION_PERIOD / portTICK_PERIOD_MS),
                 pdFALSE, IMU_PRE_CALIBRATION_TIMEOUT_SIG, &me->super);
  imu_hal_init();
  calibrationSetNotCompleted(&me->calibration);
}
