# EleketronVolt Arduino/ESP32 Project

Heyyo! This repository contains some stuff:

 1. A Tesla Charge Port Door Opener using a RF433 OOK TX Module
 2. A PM sensor reader and reporter through MQTT
 3. A Weather Station (Bresser 5-in-1) Data sniffer and reporter to MQTT (not yet)


## Products used:

 - 


## Configuration


You need to create a file called `secrets.h` within `src` containing the following constants

```
  #define MQTT_HOST "1.1.1.1"
  #define MQTT_PORT 1883
  #define MQTT_USER "username"
  #define MQTT_PASS "password"

  #define WIFI_SSID "WIFI NAME"
  #define WIFI_PASS "WIFI PASSWORD"
```