#include "weatherstation.h"
#include <RadioLib.h>
#include "../mqtt/mqtt.h"
#include <WiFi.h>
#include "wifi.h"
#include <time.h>

int timezone = 1;
int dst = 0;

// CONFIGURATION SECTION
#define MQTT_TOPIC_IPADDRESS "weather_station/ipaddress"
#define MQTT_TOPIC_TIME "weather_station/time"
#define MQTT_TOPIC_BATTERY "weather_station/battery"
#define MQTT_TOPIC_WIND_AVG "weather_station/wind_avg"
#define MQTT_TOPIC_WIND_GUST "weather_station/wind_gust"
#define MQTT_TOPIC_WIND_DIR "weather_station/wind_dir"
#define MQTT_TOPIC_HUMIDITY "weather_station/humidity"
#define MQTT_TOPIC_TEMP "weather_station/temp"
#define MQTT_TOPIC_RAIN "weather_station/rain"
#define GDO0 35
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5
// END OF CONFIGURATION SECTION



#define RADIOSTATEASSERTOK(STATEVAR)              \
    {                                             \
        if ((STATEVAR) != ERR_NONE) {             \
            Serial.print("ERROR RADIO CC1101: "); \
            Serial.println(STATEVAR);             \
            return;                               \
        }                                         \
    }


extern elektronvolt::MQTT *mqtt;

namespace elektronvolt {

SPIClass newSPI(VSPI);
byte fullMessage[40];
volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;
CC1101 radio = new Module(5, GDO0, RADIOLIB_NC, RADIOLIB_NC, newSPI, SPISettings(2000000, MSBFIRST, SPI_MODE0));
static void IRAM_ATTR setFlag(void) {
    // check if the interrupt is enabled
    if (!enableInterrupt) {
        return;
    }

    // we got a packet, set the flag
    receivedFlag = true;
}
void WeatherStation::setup() {
    newSPI.begin(SCK, MISO, MOSI, SS);
    sleep(4);
    Serial.print(F("[CC1101] Initializing ... "));

    int state = radio.begin(868.35, 8.22, 60, 270.0, 10, 32);
    RADIOSTATEASSERTOK(state);
    Serial.println(F("success!"));

    // We dont need address filtering
    state = radio.disableAddressFiltering();
    RADIOSTATEASSERTOK(state);
    Serial.println(F("[CC1101] Disabled Address Filtering"));

    // Weather Station does not send CRC so disable filtering
    state = radio.setCrcFiltering(false);
    RADIOSTATEASSERTOK(state);
    Serial.println(F("[CC1101] Disabled CRC Filtering"));

    // Weather data is 27 byte long.
    state = radio.fixedPacketLengthMode(27);
    RADIOSTATEASSERTOK(state);
    Serial.println(F("[CC1101] Set packet Length"));

    // Sync word is actually 0x2d 0xd4 but CC1101 was set to have a 4 byte preamble
    // so sync word is the leftover 0xAA + 0x2d (we'll filter out the 0xD4 later)
    state = radio.setSyncWord(0xAA, 0X2d, 0, false);
    RADIOSTATEASSERTOK(state);
    Serial.println(F("[CC1101] Set Sync Word"));

    // Enable such filtering.
    state = radio.enableSyncWordFiltering();
    RADIOSTATEASSERTOK(state);
    Serial.println(F("[CC1101] Enabled Sync Word Filtering"));

    // Initiate receiving.
    state = radio.startReceive();
    RADIOSTATEASSERTOK(state);
    Serial.println(F("[CC1101] Starting Receive"));

    // Set interrupt function.
    radio.setGdo0Action(setFlag);
    
    configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");

}
void WeatherStation::loop() {
    if (!receivedFlag) {
        return;
    }
    enableInterrupt = false;
    receivedFlag = false;
    tryReadAndDecode();
    radio.startReceive();
    enableInterrupt = true;
}

void WeatherStation::tryReadAndDecode() {
    int state = radio.readData(fullMessage, 27);
    byte *msg = &fullMessage[1];
    char mqttBuff[30];
    // PrintHex8(msg, 27);
    Serial.println(radio.getLQI());
    if (state == ERR_NONE && fullMessage[0] == 0xd4) {
        // packet was successfully received
        // print RSSI (Received Signal Strength Indicator)
        // First 13 bytes need to match inverse of last 13 bytes
        for (unsigned col = 0; col < 13; ++col) {
            if ((msg[col] ^ msg[col + 13]) != 0xff) {
                Serial.println(F("Decoding Wrong"));
                return;
            }
        }
        Serial.println(F("success!"));

        uint16_t sensor_id = msg[14];

        int temp_raw = (msg[20] & 0x0f) + ((msg[20] & 0xf0) >> 4) * 10 + (msg[21] & 0x0f) * 100;
        if (msg[25] & 0x0f)
            temp_raw = -temp_raw;
        float temperature = temp_raw * 0.1f;

        int humidity = (msg[22] & 0x0f) + ((msg[22] & 0xf0) >> 4) * 10;

        float wind_direction_deg = ((msg[17] & 0xf0) >> 4) * 22.5f;

        int gust_raw = ((msg[17] & 0x0f) << 8) + msg[16]; //fix merbanan/rtl_433#1315
        float wind_gust = gust_raw * 0.1f;

        int wind_raw = (msg[18] & 0x0f) + ((msg[18] & 0xf0) >> 4) * 10 + (msg[19] & 0x0f) * 100; //fix merbanan/rtl_433#1315
        float wind_avg = wind_raw * 0.1f;

        int rain_raw = (msg[23] & 0x0f) + ((msg[23] & 0xf0) >> 4) * 10 + (msg[24] & 0x0f) * 100;
        float rain = rain_raw * 0.1f;

        int battery_ok = ((msg[25] & 0x80) == 0);

        Serial.println("\tWeather Results: ");
        Serial.print("\tBattery: ");
        Serial.println(battery_ok);
        Serial.print("\tWind Speed: ");
        Serial.println(wind_avg);
        Serial.print("\tWind Gust: ");
        Serial.println(wind_gust);
        Serial.print("\tTemperature: ");
        Serial.println(temperature);
        Serial.print("\tRain: ");
        Serial.println(rain);
        Serial.print("\tHumidity: ");
        Serial.println(humidity);
        time_t now = time(nullptr);
        Serial.print("\tTime: ");
        Serial.println(ctime(&now));

        // MQTT
        sprintf(mqttBuff, "%s", WiFi.localIP().toString());
        mqtt->writeToTopic(MQTT_TOPIC_IPADDRESS, mqttBuff);

        sprintf(mqttBuff, "%s", ctime(&now));
        mqtt->writeToTopic(MQTT_TOPIC_TIME, mqttBuff);

        sprintf(mqttBuff, "%d", battery_ok);
        mqtt->writeToTopic(MQTT_TOPIC_BATTERY, mqttBuff);

        sprintf(mqttBuff, "%.2f", wind_avg);
        mqtt->writeToTopic(MQTT_TOPIC_WIND_AVG, mqttBuff);

        sprintf(mqttBuff, "%.2f", wind_gust);
        mqtt->writeToTopic(MQTT_TOPIC_WIND_GUST, mqttBuff);

        sprintf(mqttBuff, "%.2f", wind_direction_deg);
        mqtt->writeToTopic(MQTT_TOPIC_WIND_DIR, mqttBuff);

        sprintf(mqttBuff, "%d", humidity);
        mqtt->writeToTopic(MQTT_TOPIC_HUMIDITY, mqttBuff);

        sprintf(mqttBuff, "%.2f", temperature);
        mqtt->writeToTopic(MQTT_TOPIC_TEMP, mqttBuff);

        sprintf(mqttBuff, "%.2f", rain);
        mqtt->writeToTopic(MQTT_TOPIC_RAIN, mqttBuff);

    }
}
} // namespace elektronvolt
