#ifndef HVSCU_SDC_HPP
#define HVSCU_SDC_HPP

#include "ST-LIB_LOW.hpp"

namespace HVSCU {
class SDC {
    SensorInterrupt sdc_good;
    PinState sdc_good_value{ON};

    void sdc_callback(void) {
        triggered = true;
    }

   public:
    bool triggered{false};

    SDC(Pin& pin)
        : sdc_good{pin, [&]() { sdc_callback(); }, &sdc_good_value,
                   TRIGGER::FAILING_EDGE} {};
};
}  // namespace HVSCU

#endif