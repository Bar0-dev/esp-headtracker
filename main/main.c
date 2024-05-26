#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "led_ao.h"
#include "button_ao.h"
#include "events_broker.h"
#include "polling_ao.h"
#include "imu_ao.h"

static Broker broker;
Active *AO_Broker = &broker.super;
static Polling polling;
Active *AO_Polling = &polling.super;

static Led led;
Active *AO_Led = &led.super;
static Button button;
Active *AO_Button = &button.super;
static Imu imu;
Active *AO_Imu = &imu.super;

void app_main(void)
{
    Broker_ctor(&broker);
    Active_start(AO_Broker, "Broker thread", 4096, 20, tskNO_AFFINITY, 20);
    
    Polling_ctor(&polling);
    Active_start(AO_Polling, "Polling thread", 2048, 11, tskNO_AFFINITY, 10);

    Led_ctor(&led);
    Active_start(AO_Led, "LED thread", 2048, 10, tskNO_AFFINITY, 10);

    Button_ctor(&button);
    Active_start(AO_Button, "Button thread", 2048, 1, tskNO_AFFINITY, 10);

    Imu_ctor(&imu);
    Active_start(AO_Imu, "Imu thread", 2048, 2, tskNO_AFFINITY, 10);

    /**
     * Subscriptions
    */
    Broker_subscribe(&broker, &(Event){ EV_POLLING_BUTTON_STATE_CHANGED , (void*)0 }, AO_Button);
    Broker_subscribe(&broker, &(Event){ EV_BUTTON_PRESSED , (void*)0 }, AO_Led);
    Broker_subscribe(&broker, &(Event){ EV_BUTTON_RELEASED , (void*)0 }, AO_Led);
    Broker_subscribe(&broker, &(Event){ EV_BUTTON_HOLD , (void*)0 }, AO_Led);
    Broker_subscribe(&broker, &(Event){ EV_BUTTON_DOUBLE_PRESS , (void*)0 }, AO_Led);
    Broker_subscribe(&broker, &(Event){ EV_BUTTON_PRESSED , (void*)0 }, AO_Imu);
}