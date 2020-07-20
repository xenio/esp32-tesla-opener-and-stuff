#include "pm25sensor.h"
#include "../mqtt/mqtt.h"
#include "PMS.h"
#include <Arduino.h>
#include <HardwareSerial.h>


extern elektronvolt::MQTT *mqtt;

// PMS_READ_INTERVAL (4:30 min) and PMS_READ_DELAY (30 sec) CAN'T BE EQUAL! Values are also used to detect sensor state.
static const uint32_t PMS_READ_INTERVAL = 60000;
static const uint32_t PMS_READ_DELAY = 30000;

uint32_t timerInterval = PMS_READ_DELAY;

namespace elektronvolt {
PMS pms(Serial2);
PMS::DATA data;

char mqttMessageBuffer[30];


void PM25Sensor::setup() {
    Serial2.begin(PMS::BAUD_RATE); //, SERIAL_8N1, RX2, TX2);
    // Put it in passive mode to preserve sensor readings accuracy over time.
    pms.passiveMode();
    sleep(1);
    pms.wakeUp();    
}

void PM25Sensor::loop() {
    static uint32_t timerLast = 0;
    uint32_t timerNow = millis();

    if (timerNow - timerLast >= timerInterval) {
        if (timerInterval == PMS_READ_DELAY) {
            this->read();
        } else {
            Serial.println("Waking up sensor");
            pms.wakeUp();
        }
        timerLast = timerNow;
        timerInterval = timerInterval == PMS_READ_DELAY ? PMS_READ_INTERVAL : PMS_READ_DELAY;
    }
}

void PM25Sensor::read() {
    // CLear buffer;
    while (Serial2.available()) {
        Serial2.read();
    }
    pms.requestRead();

    if (pms.readUntil(data)) {
      // messageString = String(data.PM_AE_UG_1_0);
      // messageString.toCharArray(mqttMessageBuffer, messageString.length() + 1);
      // Serial.print(mqttMessageBuffer);
      // Serial.println
      // mqtt->writeToTopic("pmsensor/pm_1_0", "ciao");


      sprintf(mqttMessageBuffer, "%d", data.PM_AE_UG_1_0);
      mqtt->writeToTopic("pmsensor/pm_1_0", mqttMessageBuffer);

      sprintf(mqttMessageBuffer, "%d", data.PM_AE_UG_2_5);
      mqtt->writeToTopic("pmsensor/pm_2_5", mqttMessageBuffer);
      
      sprintf(mqttMessageBuffer, "%d", data.PM_AE_UG_10_0);
      mqtt->writeToTopic("pmsensor/pm_10_0", mqttMessageBuffer);

      Serial.print("PM 1.0 (ug/m3): ");
      Serial.println(data.PM_AE_UG_1_0);

      Serial.print("PM 2.5 (ug/m3): ");
      Serial.println(data.PM_AE_UG_2_5);

      Serial.print("PM 10.0 (ug/m3): ");
      Serial.println(data.PM_AE_UG_10_0);

    } else {
        Serial.println("NO data");
    }
    // Put pms to sleep.
    pms.sleep();
}
} // namespace elektronvolt