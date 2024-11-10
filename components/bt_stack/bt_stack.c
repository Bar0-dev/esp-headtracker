#include "bt_stack.h"
#include "esp_spp_api.h"

static const char local_device_name[] = LOCAL_DEVICE_NAME;
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

esp_spp_cfg_t bt_spp_cfg = {
    .mode = esp_spp_mode,
    .enable_l2cap_ertm = true,
    .tx_buffer_size = 0, /* Only used for ESP_SPP_MODE_VFS mode */
};

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  char bda_str[18] = {0};

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
#if (SPP_SHOW_MODE == SPP_SHOW_DATA)
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
#else
    gettimeofday(&time_new, NULL);
    data_num += param->data_ind.len;
    if (time_new.tv_sec - time_old.tv_sec >= 3) {
      print_speed();
    }
#endif
    break;
  case ESP_SPP_CONG_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
    break;
  case ESP_SPP_WRITE_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT");
    break;
  case ESP_SPP_SRV_OPEN_EVT:
    ESP_LOGI(SPP_TAG,
             "ESP_SPP_SRV_OPEN_EVT status:%d handle:%" PRIu32 ", rem_bda:[%s]",
             param->srv_open.status, param->srv_open.handle,
             bda2str(param->srv_open.rem_bda, bda_str, sizeof(bda_str)));
    gettimeofday(&time_old, NULL);
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
