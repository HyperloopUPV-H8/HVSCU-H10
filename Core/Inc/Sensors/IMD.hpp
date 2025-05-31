#ifndef IMD_HPP
#define IMD_HPP

#include "ST-LIB_LOW.hpp"

namespace HVSCU {

template <typename A, typename B>
concept Substractable = requires(A a, B b) {
    { a - b } -> std::totally_ordered;
};

template <typename A, typename B, typename E>
    requires Substractable<A, B> && std::totally_ordered<E>
inline bool lessError(const A& a, const B& b, const E& error) {
    return std::abs(a - b) < error;
}
class IMD {
    enum class Status {
        SHORTCIRCUIT,
        NORMAL,
        UNDERVOLTAGE,
        EQUIPMENT_FAULT,
        GROUNDING_FAULT
    };

    bool is_ok;

    PWMSensor<float> m_ls;
    float freq{};
    float duty{};

    DigitalOutput pow;

    Status status;

   public:
    IMD(Pin& m_ls_pin, Pin& pow_pin)
        : m_ls{m_ls_pin, freq, duty}, pow{pow_pin} {}

    void power_on() { pow.turn_on(); }

    void read() {
        m_ls.read();
        if (lessError(freq, 0, 0.1)) {
            status = Status::SHORTCIRCUIT;
            is_ok = false;
            return;
        }
        if (lessError(freq, 10, 0.1)) {
            status = Status::NORMAL;
            is_ok = false;
            return;
        }
        if (lessError(freq, 20, 0.1)) {
            status = Status::UNDERVOLTAGE;
            is_ok = false;
            return;
        }
        if (lessError(freq, 30, 0.1)) {
            if (duty < 20) {
                status = Status::NORMAL;
                is_ok = true;
            } else {
                status = Status::SHORTCIRCUIT;
                is_ok = false;
            }
            return;
        }
        if (lessError(freq, 40, 0.1)) {
            status = Status::EQUIPMENT_FAULT;
            is_ok = false;
            return;
        }
        if (lessError(freq, 50, 0.1)) {
            status = Status::GROUNDING_FAULT;
            is_ok = false;
            return;
        }
    }
};
}  // namespace HVSCU

#endif