# ElektronVolt Arduino/ESP32 Project

Heyyo! This repository contains some stuff:

 1. A Tesla Charge Port Door Opener using a RF433 OOK TX Module
 2. A PM sensor reader and reporter through MQTT
 3. A Weather Station (Bresser 5-in-1) Data sniffer and reporter to MQTT (not yet)


## Products used:

Of course an [ESP32](https://amzn.to/2WCQVBx)

### for Tesla Door Opener
 - [433MHz transmitter](https://www.amazon.it/gp/product/B00R2U8OEU) for Tesla Charge Door Opener 
 - Tesla Model 3 :) -> Referral for [Some Free SuperCharging](https://ts.la/andrea74473)

### for Weather Station
 - Bresser 5 in 1 [Weather Station](https://amzn.to/2E2WLWl)
 - [868MHz CC1101 transceiver](https://amzn.to/3eHddbf)

### for PM Air quality Sensor
 - [PMX5003](https://amzn.to/2WCMRkL)

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

## Wiring 

I've used an ESP32 WROOM dev so wiring might differ from board to board.

### for Tesla Door Opener

The 433MHz needs to be connected to ground, +3.3v and data to pin 19.

### for PM air quality sensor

Refer to datasheet for its pinout. 

 - RX pin -> ESP32[17]
 - TX PIN -> ESP32[16]
 - 5v
 - gnd