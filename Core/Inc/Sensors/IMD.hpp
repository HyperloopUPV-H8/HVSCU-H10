#ifndef IMD_HPP
#define IMD_HPP

#include "ST-LIB_LOW.hpp"

namespace HVSCU {
class IMD {
    enum class Status {
        SHORTCIRCUIT,
        NORMAL,
        UNDERVOLTAGE,
        EQUIPMENT_FAULT,
        GROUNDING_FAULT
    };

    DigitalOutput ok_hs;

    PWMSensor<float> m_ls;
    float freq{};
    float duty{};

    Status status;

    IMD(Pin& ok_hs_pin, Pin& m_ls_pin)
        : ok_hs{ok_hs_pin}, m_ls{m_ls_pin, freq, duty} {}

    void update() {}
};
}  // namespace HVSCU

#endif