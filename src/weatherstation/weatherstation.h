#ifndef WEATHERSTATION_H
#define WEATHERSTATION_H
#endif

// #define EVWEATHERASSERT(STATEVAR) { if((STATEVAR) != ERR_NONE) { Serial.println("ERROR"); Serial.println(STATEVAR); return; } }

namespace elektronvolt {
  class WeatherStation {
    public: 
      void setup();
      void loop();

    private:
      void tryReadAndDecode();
  };
}
