#ifndef SENSORS_HPP
#define SENSORS_HPP

#include "ADCLinearSensor.hpp"
#include "BMS.hpp"
#include "BatteryPack.hpp"
#include "IMD.hpp"

#define BATTERIES_CONNECTED 1
#define N_BATTERIES 9
#define READING_PERIOD_US 20000
#define FAKE_TOTAL_VOLTAGE 250.0

#define RESISTANCE_REFERENCE 10000.0  // Ohmios
#define VOLTAGE_REFERENCE 3.0         // V
#define R0 100.0                      // Ohmios
#define TCR 0.00385

namespace HVSCU {
class Sensors {
    // Voltage sensor for HVSCU with ID 1
    static constexpr Pin &VOLTAGE_PIN{PF13};
    static constexpr float VOLTAGE_SLOPE{145.879241943359};
    static constexpr float VOLTAGE_OFFSET{-5.83882474899292};

    // Current sensor for HVSCU with ID 1
    static constexpr Pin &CURRENT_PIN{PA0};
    static constexpr float CURRENT_SLOPE{92.27959442138672};
    static constexpr float CURRENT_OFFSET{-152.49656677246094};

    // IMD
    static constexpr Pin &M_LS_PIN{PF7};
    static constexpr Pin &IMD_POW{PE2};

    static inline bool reading_sensors_flag{false};
    static inline bool reading_batteries_flag{false};
#if BATTERIES_CONNECTED
    static inline bool process_batteries_data_flag{false};
#endif

   public:
    static ADCLinearSensor<10> &voltage_sensor() {
        static ADCLinearSensor<10> voltage_sensor{
            VOLTAGE_PIN, static_cast<uint16_t>(Comms::IDPacket::VOLTAGE),
            VOLTAGE_SLOPE, VOLTAGE_OFFSET};
        return voltage_sensor;
    }

    static ADCLinearSensor<15> &current_sensor() {
        static ADCLinearSensor<15> current_sensor{
            CURRENT_PIN, static_cast<uint16_t>(Comms::IDPacket::CURRENT),
            CURRENT_SLOPE, CURRENT_OFFSET};
        return current_sensor;
    }

    static IMD &imd() {
        static IMD imd{M_LS_PIN, IMD_POW,
                       static_cast<uint16_t>(Comms::IDPacket::IMD)};
        return imd;
    }

    static BatteryPack<N_BATTERIES> &batteries() {
        static BatteryPack<N_BATTERIES> batteries{
            static_cast<uint16_t>(Comms::IDPacket::TOTAL_VOLTAGE),
            static_cast<uint16_t>(Comms::IDPacket::DRIVER_DIAG),
            static_cast<uint16_t>(Comms::IDPacket::BATTERY_1)};
        return batteries;
    }
    static inline array<float, N_BATTERIES> batteries_temp{};

    static void init();
    static void start();
    static void update();
#if BATTERIES_CONNECTED
    static void process_batteries_data();
#endif
};
}  // namespace HVSCU

#endif