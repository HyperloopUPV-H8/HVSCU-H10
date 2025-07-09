#ifndef HVSCU_VOLTAGE_SENSOR_HPP
#define HVSCU_VOLTAGE_SENSOR_HPP

#include "Comms/Comms.hpp"
#include "ST-LIB_LOW.hpp"

namespace HVSCU {
template <std::size_t FilterSize>
class ADCLinearSensor {
    FilteredLinearSensor<float, FilterSize> sensor;
    HeapPacket packet;
    MovingAverage<FilterSize> filter{};

   public:
    float reading{};

    ADCLinearSensor(Pin& pin, uint16_t id, float slope, float offset,
                    std::vector<Comms::Target> targets)
        : sensor{pin, slope, offset, reading, filter}, packet{id, &reading} {
        for (auto& t : targets) {
            Comms::add_packet(t, &packet);
        }
    }

    void read() { sensor.read(); };
};
}  // namespace HVSCU

#endif