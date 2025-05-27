#ifndef SENSORS_HPP
#define SENSORS_HPP

#include "ADCLinearSensor.hpp"
#include "BMS.hpp"
#include "BatteryPack.hpp"

#define BATTERIES_CONNECTED 1
#define N_BATTERIES 10

namespace HVSCU {
class Sensors {
    // Voltage sensor
    static constexpr Pin &VOLTAGE_PIN{PF13};
    static constexpr float VOLTAGE_SLOPE{124.299234299011000};
    static constexpr float VOLTAGE_OFFSET{-5.409069953014590};

    // Current sensor
    static constexpr Pin &CURRENT_PIN{PA0};
    static constexpr float CURRENT_SLOPE{92.27959442138672};
    static constexpr float CURRENT_OFFSET{-152.49656677246094};

    static inline bool reading_sensors_flag{false};

#if BATTERIES_CONNECTED
    static inline bool reading_batteries_flag{false};
#endif

   public:
    static ADCLinearSensor<10> &voltage_sensor() {
        static ADCLinearSensor<10> voltage_sensor{
            VOLTAGE_PIN, static_cast<uint16_t>(Comms::IDPacket::VOLTAGE),
            VOLTAGE_SLOPE, VOLTAGE_OFFSET};
        return voltage_sensor;
    }

    static ADCLinearSensor<10> &current_sensor() {
        static ADCLinearSensor<10> current_sensor{
            CURRENT_PIN, static_cast<uint16_t>(Comms::IDPacket::CURRENT),
            CURRENT_SLOPE, CURRENT_OFFSET};
        return current_sensor;
    }
#if BATTERIES_CONNECTED
    static BatteryPack<10> &batteries() {
        static BatteryPack<10> batteries{
            static_cast<uint16_t>(Comms::IDPacket::BATTERY_1),
            static_cast<uint16_t>(Comms::IDPacket::TOTAL_VOLTAGE),
            static_cast<uint16_t>(Comms::IDPacket::DRIVER_DIAG)};
        return batteries;
    }
#endif

    static void init();
    static void start();
    static void update();
};
}  // namespace HVSCU

#endif