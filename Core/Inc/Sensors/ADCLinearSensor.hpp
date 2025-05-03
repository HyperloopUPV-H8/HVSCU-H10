#ifndef HVSCU_VOLTAGE_SENSOR_HPP
#define HVSCU_VOLTAGE_SENSOR_HPP

#include "ST-LIB_LOW.hpp"

namespace HVSCU {
class ADCLinearSensor {
    LinearSensor<float> sensor;
    HeapPacket packet{};

   public:
    float reading{};

    ADCLinearSensor(Pin& pin, uint16_t id, float slope, float offset)
        : sensor{pin, slope, offset, reading}, packet{id, &reading} {}

    void read() { sensor.read(); };
};
}  // namespace HVSCU

#endif