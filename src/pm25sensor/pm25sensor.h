#ifndef PM25SENSOR_H
#define PM25SENSOR_H

namespace elektronvolt {
  class PM25Sensor {
    public: 
      PM25Sensor() = default;
      void setup();
      void loop();
    private:
      void read();
  };
}


#endif