#ifndef HVSCU_VOLTAGE_SENSOR_HPP
#define HVSCU_VOLTAGE_SENSOR_HPP

#include "ST-LIB_LOW.hpp"

#include "Comms/Comms.hpp"

namespace HVSCU {
class ADCLinearSensor {
    LinearSensor<float> sensor;
    HeapPacket packet;

   public:
    float reading{};

    ADCLinearSensor(Pin& pin, uint16_t id, float slope, float offset)
        : sensor{pin, slope, offset, reading}, packet{id, &reading} {
        Comms::add_packet(&packet);
    }

    void read() { sensor.read(); };
};
}  // namespace HVSCU

#endif