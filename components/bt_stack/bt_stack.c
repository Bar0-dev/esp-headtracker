#include "bt_stack.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_log.h"
#include "esp_spp_api.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define SPP_TAG "SPP"
#define SPP_SERVER_NAME "SPP_SERVER"

static const char local_device_name[] = LOCAL_DEVICE_NAME;
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

static uint32_t handle;
static bool congested;

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

  switch (event) {
  case ESP_SPP_INIT_EVT:
    if (param->init.status == ESP_SPP_SUCCESS) {
      ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
      esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME);
    } else {
      ESP_LOGE(SPP_TAG, "ESP_SPP_INIT_EVT status:%d", param->init.status);
    }
    break;
  case ESP_SPP_DISCOVERY_COMP_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_DISCOVERY_COMP_EVT");
    break;
  case ESP_SPP_OPEN_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_OPEN_EVT");
    break;
  case ESP_SPP_CLOSE_EVT:
    ESP_LOGI(SPP_TAG,
             "ESP_SPP_CLOSE_EVT status:%d handle:%" PRIu32
             " close_by_remote:%d",
             param->close.status, param->close.handle, param->close.async);
    break;
  case ESP_SPP_START_EVT:
    if (param->start.status == ESP_SPP_SUCCESS) {
      ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT handle:%" PRIu32 " sec_id:%d scn:%d",
               param->start.handle, param->start.sec_id, param->start.scn);
      esp_bt_gap_set_device_name(local_device_name);
      esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    } else {
      ESP_LOGE(SPP_TAG, "ESP_SPP_START_EVT status:%d", param->start.status);
    }
    break;
  case ESP_SPP_CL_INIT_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_CL_INIT_EVT");
    break;
  case ESP_SPP_DATA_IND_EVT:
    /*
     * We only show the data in which the data length is less than 128 here. If
     * you want to print the data and the data rate is high, it is strongly
     * recommended to process them in other lower priority application task
     * rather than in this callback directly. Since the printing takes too much
     * time, it may stuck the Bluetooth stack and also have a effect on the
     * throughput!
     */
    ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len:%d handle:%" PRIu32,
             param->data_ind.len, param->data_ind.handle);
    if (param->data_ind.len < 128) {
      esp_log_buffer_hex("", param->data_ind.data, param->data_ind.len);
    }
    break;
  case ESP_SPP_CONG_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
    break;
  case ESP_SPP_WRITE_EVT:
    congested = param->write.cong;
    ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT, congested: %d", congested);
    break;
  case ESP_SPP_SRV_OPEN_EVT:
    handle = param->srv_open.handle;
    ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT status:%d", param->srv_open.status);
    break;
  case ESP_SPP_SRV_STOP_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_STOP_EVT");
    break;
  case ESP_SPP_UNINIT_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_UNINIT_EVT");
    break;
  default:
    break;
  }
}

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {

  switch (event) {
  case ESP_BT_GAP_AUTH_CMPL_EVT: {
    if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
      ESP_LOGI(SPP_TAG, "authentication success: %s",
               param->auth_cmpl.device_name);
    } else {
      ESP_LOGE(SPP_TAG, "authentication failed, status:%d",
               param->auth_cmpl.stat);
    }
    break;
  }
  case ESP_BT_GAP_PIN_REQ_EVT: {
    ESP_LOGI(SPP_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d",
             param->pin_req.min_16_digit);
    if (param->pin_req.min_16_digit) {
      ESP_LOGI(SPP_TAG, "Input pin code: 0000 0000 0000 0000");
      esp_bt_pin_code_t pin_code = {0};
      esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
    } else {
      ESP_LOGI(SPP_TAG, "Input pin code: 1234");
      esp_bt_pin_code_t pin_code;
      pin_code[0] = '1';
      pin_code[1] = '2';
      pin_code[2] = '3';
      pin_code[3] = '4';
      esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
    }
    break;
  }
  case ESP_BT_GAP_CFM_REQ_EVT:
    ESP_LOGI(
        SPP_TAG,
        "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %" PRIu32,
        param->cfm_req.num_val);
    esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
    break;
  case ESP_BT_GAP_KEY_NOTIF_EVT:
    ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%" PRIu32,
             param->key_notif.passkey);
    break;
  case ESP_BT_GAP_KEY_REQ_EVT:
    ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
    break;

  case ESP_BT_GAP_MODE_CHG_EVT:
    ESP_LOGI(SPP_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode:%d", param->mode_chg.mode);
    break;

  default: {
    ESP_LOGI(SPP_TAG, "event: %d", event);
    break;
  }
  }
  return;
}

void bt_stack_init(void) {
  congested = true;
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
    ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
    ESP_LOGE(SPP_TAG, "%s enable controller failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
  if ((ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
    ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bluedroid_enable()) != ESP_OK) {
    ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
    ESP_LOGE(SPP_TAG, "%s gap register failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
    ESP_LOGE(SPP_TAG, "%s spp register failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  esp_spp_cfg_t bt_spp_cfg = {
      .mode = esp_spp_mode,
      .enable_l2cap_ertm = true,
      .tx_buffer_size = 0, /* Only used for ESP_SPP_MODE_VFS mode */
  };
  if ((ret = esp_spp_enhanced_init(&bt_spp_cfg)) != ESP_OK) {
    ESP_LOGE(SPP_TAG, "%s spp init failed: %s", __func__, esp_err_to_name(ret));
    return;
  }

  /* Set default parameters for Secure Simple Pairing */
  esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
  esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
  esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

  /*
   * Set default parameters for Legacy Pairing
   * Use variable pin, input pin code when pairing
   */
  esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
  esp_bt_pin_code_t pin_code;
  esp_bt_gap_set_pin(pin_type, 0, pin_code);
}

void bt_stack_write(uint8_t *msg, size_t size) {
  if (congested) {
    esp_spp_write(handle, size, msg);
  }
}
