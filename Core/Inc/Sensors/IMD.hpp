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
    DigitalOutput pow;
    DigitalSensor ok;
    float freq{};
    float duty{};

    uint8_t status{static_cast<uint8_t>(Status::FAST_EVAL)};
    float resistance{};

    HeapPacket packet;

   public:
    PinState ok_status{ON};
    bool is_ok{true};

    IMD(Pin& m_ls_pin, Pin& pow_pin, Pin& ok_pin, uint16_t id)
        : m_ls{m_ls_pin, freq, duty},
          pow{pow_pin},
          ok{ok_pin, ok_status},
          packet{id, &status, &resistance, &is_ok} {
        Comms::add_packet(Comms::Target::CONTROL_STATION, &packet);
    }

    void power_on() { pow.turn_on(); }

    void calculate_resistance() {
        resistance = ((90 * 1.2e6) / (duty - 5)) - 1.2e6;
    };

    void read() {
        ok.read();
        if (ok_status == PinState::OFF) {
            is_ok = false;
        } else {
            is_ok = true;
        }

        m_ls.read();
        if (lessError(freq, 0, 0.1)) {
            status = static_cast<uint8_t>(Status::SHORTCIRCUIT);
            return;
        }
        if (lessError(freq, 10, 0.1)) {
            status = static_cast<uint8_t>(Status::NORMAL);
            calculate_resistance();
            return;
        }
        if (lessError(freq, 20, 0.1)) {
            status = static_cast<uint8_t>(Status::UNDERVOLTAGE);
            calculate_resistance();
            return;
        }
        if (lessError(freq, 30, 0.1)) {
            status = static_cast<uint8_t>(Status::FAST_EVAL);
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