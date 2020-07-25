#include "weatherstation.h"
#include <RadioLib.h>

#define GDO0 35
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5
#define RADIOSTATEASSERTOK(STATEVAR)              \
    {                                             \
        if ((STATEVAR) != ERR_NONE) {             \
            Serial.print("ERROR RADIO CC1101: "); \
            Serial.println(STATEVAR);             \
            return;                               \
        }                                         \
    }

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
}
void WeatherStation::loop() {
    if (!receivedFlag) {
        return;
    }
    enableInterrupt = false;
    receivedFlag = false;
    // Serial.println(F("[CC1101] REading incoming transmission ... "));
    // while(true);
    tryReadAndDecode();
    radio.startReceive();
    enableInterrupt = true;
    // Serial.println("FINISH");
}

void WeatherStation::tryReadAndDecode() {
    int state = radio.readData(fullMessage, 27);
    byte *msg = &fullMessage[1];
    // PrintHex8(msg, 27);

    if (state == ERR_NONE && radio.getLQI() < 50 && fullMessage[0] == 0xd4) {
        // packet was successfully received
        Serial.println(F("success!"));
        // print RSSI (Received Signal Strength Indicator)
        // First 13 bytes need to match inverse of last 13 bytes
        for (unsigned col = 0; col < 13; ++col) {
            if ((msg[col] ^ msg[col + 13]) != 0xff) {
                Serial.println(F("Decoding Wrong"));
            }
        }

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

        Serial.println("Results: ");
        Serial.print("Battery: ");
        Serial.println(battery_ok);
        Serial.print("Wind Speed: ");
        Serial.println(wind_avg);
        Serial.print("Wind Gust: ");
        Serial.println(wind_gust);
        Serial.print("Temperature: ");
        Serial.println(temperature);
        Serial.print("Rain: ");
        Serial.println(rain);
        Serial.print("Humidity: ");
        Serial.println(humidity);
    }
    for (int i = 0; i < 27; i++) {
        msg[i] = 0;
    }
}
} // namespace elektronvolt
