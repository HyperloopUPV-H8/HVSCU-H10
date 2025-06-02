#ifndef HVSCU_SDC_HPP
#define HVSCU_SDC_HPP

#include "ST-LIB_LOW.hpp"

namespace HVSCU {
class SDC {
    SensorInterrupt sdc_good;
    bool first_time{false};
    PinState sdc_good_value{OFF};

    void sdc_callback(void) {
        if (!first_time) {
            first_time = true;
        } else {
            ErrorHandler("SDC triggered");
        }
    }

   public:
    SDC(Pin& pin)
        : sdc_good{pin, [&]() { sdc_callback(); }, sdc_good_value,
                   TRIGGER::FAILING_EDGE} {};
};
}  // namespace HVSCU

#endif