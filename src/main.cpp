#include "./mqtt/mqtt.h"
#include "./teslaopener/teslaopener.h"
#include "./wifi/wifi.h"
#include "./pm25sensor/pm25sensor.h"
#include "secrets.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

elektronvolt::TeslaOpener *teslaOpener;
elektronvolt::MQTT *mqtt;
elektronvolt::WiFi *wifi;
elektronvolt::PM25Sensor *pm25;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    // init
    teslaOpener = new elektronvolt::TeslaOpener();
    wifi = new elektronvolt::WiFi();
    mqtt = new elektronvolt::MQTT();
    pm25 = new elektronvolt::PM25Sensor();

    wifi->setup();
    teslaOpener->setup();
    mqtt->setup();
    pm25->setup();

    // Subscribe to mqtt event to open the tesla charge port.
    mqtt->subscribeTo("openChargePort", [](char * _1,uint8_t * _2, int _3) {
      Serial.println("Asking to open the charge port");
      teslaOpener->openChargePort();
    });
}

void loop() {
  wifi->loop();
  mqtt->loop();
  pm25->loop();

    // put your main code here, to run repeatedly:
}