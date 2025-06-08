#ifndef IMD_HPP
#define IMD_HPP

#include "Comms/Comms.hpp"
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
    enum class Status : uint8_t {
        SHORTCIRCUIT,
        NORMAL,
        UNDERVOLTAGE,
        FAST_EVAL,
        EQUIPMENT_FAULT,
        GROUNDING_FAULT
    };

    PWMSensor<float> m_ls;
    float freq{};
    float duty{};

    DigitalOutput pow;

    uint8_t status{static_cast<uint8_t>(Status::FAST_EVAL)};
    float resistance{};

    HeapPacket packet;

   public:
    bool is_ok{true};

    IMD(Pin& m_ls_pin, Pin& pow_pin, uint16_t id)
        : m_ls{m_ls_pin, freq, duty},
          pow{pow_pin},
          packet{id, &status, &resistance, &is_ok} {
        Comms::add_packet(&packet);
    }

    void power_on() { pow.turn_on(); }

    void check_ok() {
        resistance = ((90 * 1.2e6) / (duty - 5)) - 1.2e6;
        if (resistance < 100000) {
            is_ok = false;
        } else {
            is_ok = true;
        }
    };

    void read() {
        m_ls.read();
        if (lessError(freq, 0, 0.1)) {
            status = static_cast<uint8_t>(Status::SHORTCIRCUIT);
            return;
        }
        if (lessError(freq, 10, 0.1)) {
            status = static_cast<uint8_t>(Status::NORMAL);
            check_ok();
            return;
        }
        if (lessError(freq, 20, 0.1)) {
            status = static_cast<uint8_t>(Status::UNDERVOLTAGE);
            check_ok();
            return;
        }
        if (lessError(freq, 30, 0.1)) {
            status = static_cast<uint8_t>(Status::FAST_EVAL);
            if (duty < 20) {
                is_ok = true;
            } else {
                is_ok = false;
            }

            return;
        }
        if (lessError(freq, 40, 0.1)) {
            status = static_cast<uint8_t>(Status::EQUIPMENT_FAULT);
            return;
        }
        if (lessError(freq, 50, 0.1)) {
            status = static_cast<uint8_t>(Status::GROUNDING_FAULT);
            return;
        }
    }
};
}  // namespace HVSCU

#endif