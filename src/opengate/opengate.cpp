#include <Arduino.h>
#include "opengate.h"

// Original code: https://github.com/fredilarsen/TeslaChargeDoorOpener/blob/master/TeslaChargeDoorOpener.ino
namespace elektronvolt
{
    
    const uint8_t signalPin = 33; // The number of the pin with the output signal

    void OpenGate::openGateTrigger()
    {
        digitalWrite(signalPin, HIGH);
        delay(1000);
        digitalWrite(signalPin, LOW);
    }

    void OpenGate::setup()
    {
      pinMode(signalPin, OUTPUT);
    }

} // namespace elektronvolt
