#ifndef HVSCU_SDC_HPP
#define HVSCU_SDC_HPP

#include "ST-LIB_LOW.hpp"

namespace HVSCU {
class SDC {
    SensorInterrupt sdc_good;
    bool first_time{false};
    PinState sdc_good_value{OFF};
    uint32_t last_interrupt_time{0};

    void sdc_callback(void) {
        uint32_t now = HAL_GetTick();
        if (now - last_interrupt_time > 30) {  // Debouncing
            last_interrupt_time = now;
            if (!first_time) {
                first_time = true;
            } else {
                ErrorHandler("SDC triggered");
            }
        }
    }

   public:
    SDC(Pin& pin)
        : sdc_good{pin, [&]() { sdc_callback(); }, sdc_good_value,
                   TRIGGER::FAILING_EDGE} {};
};
}  // namespace HVSCU

#endif