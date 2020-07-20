#include <Arduino.h>
#include "teslaopener.h"

// Original code: https://github.com/fredilarsen/TeslaChargeDoorOpener/blob/master/TeslaChargeDoorOpener.ino
namespace elektronvolt
{
    
    const uint8_t signalPin = 19; // The number of the pin with the output signal

    // The signal to send
    const uint16_t pulseWidth = 400;     // Microseconds
    const uint16_t messageDistance = 23; // Millis
    const uint8_t transmissions = 5;     // Number of repeated transmissions
    const uint8_t messageLength = 43;
    const uint8_t sequence[messageLength] = {
        0x02, 0xAA, 0xAA, 0xAA, // Preamble of 26 bits by repeating 1010
        0x2B,                   // Sync byte
        0x2C, 0xCB, 0x33, 0x33, 0x2D, 0x34, 0xB5, 0x2B, 0x4D, 0x32, 0xAD, 0x2C, 0x56, 0x59, 0x96, 0x66,
        0x66, 0x5A, 0x69, 0x6A, 0x56, 0x9A, 0x65, 0x5A, 0x58, 0xAC, 0xB3, 0x2C, 0xCC, 0xCC, 0xB4, 0xD2,
        0xD4, 0xAD, 0x34, 0xCA, 0xB4, 0xA0};

    void sendByte(uint8_t dataByte)
    {
      for (int8_t bit = 7; bit >= 0; bit--)
      { // MSB
        digitalWrite(signalPin, (dataByte & (1 << bit)) != 0 ? HIGH : LOW);
        delayMicroseconds(pulseWidth);
      }
    }

    void TeslaOpener::openChargePort()
    {
      for (uint8_t t = 0; t < transmissions; t++)
      {
        for (uint8_t i = 0; i < messageLength; i++)
          sendByte(sequence[i]);
        digitalWrite(signalPin, LOW);
        delay(messageDistance);
      }
    }

    void TeslaOpener::setup()
    {
      pinMode(signalPin, OUTPUT);
    }

} // namespace elektronvolt
