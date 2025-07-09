#ifndef HVSCU_SDC_HPP
#define HVSCU_SDC_HPP

#include "ST-LIB_LOW.hpp"

namespace HVSCU {
class SDC {
    enum class STATUS : uint8_t { ENGAGED, DISENGAGED };
    SensorInterrupt sdc_good;
    PinState sdc_good_value{ON};

    HeapPacket packet;

    void sdc_callback(void) {
        sdc_good.read();
        if (sdc_good_value == PinState::OFF) {
            status = STATUS::DISENGAGED;
            triggered = true;
        } else {
            status = STATUS::ENGAGED;
        }
    }

   public:
    bool triggered{false};
    STATUS status{STATUS::DISENGAGED};

    SDC(Pin& pin, uint16_t id)
        : sdc_good{pin, [&]() { sdc_callback(); }, &sdc_good_value,
                   TRIGGER::BOTH_EDGES},
          packet{id, &status} {
        Comms::add_packet(Comms::Target::CONTROL_STATION, &packet);
    };
};
}  // namespace HVSCU

#endif