#ifndef HVSCU_SDC_HPP
#define HVSCU_SDC_HPP

#include "ST-LIB_LOW.hpp"

namespace HVSCU {
class SDC {
    SensorInterrupt sdc_good;
    PinState sdc_good_value{OFF};
    uint32_t last_interrupt_time{0};

    void sdc_callback(void) {
        sdc_good.read();
        uint32_t now = HAL_GetTick();
        if (now - last_interrupt_time > 30) {  // Debouncing
            last_interrupt_time = now;

            if (sdc_good_value == PinState::ON) return;

            triggered = true;
        }
    }

   public:
    bool triggered{false};

    SDC(Pin& pin)
        : sdc_good{pin, [&]() { sdc_callback(); }, &sdc_good_value,
                   TRIGGER::BOTH_EDGES} {};
};
}  // namespace HVSCU

#endif