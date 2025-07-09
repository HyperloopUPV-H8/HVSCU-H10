#ifndef SENSORS_HPP
#define SENSORS_HPP

#include "ADCLinearSensor.hpp"
#include "BMS.hpp"
#include "BatteryPack.hpp"
#include "IMD.hpp"
#include "SDC.hpp"

#define BATTERIES_CONNECTED 1
#define N_BATTERIES 2

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
    static constexpr Pin &IMD_OK{PA12};

    // SDC
    static constexpr Pin &SDC_GOOD_PIN{PB12};

    static inline bool reading_sensors_flag{false};
    static inline bool reading_batteries_flag{false};

   public:
    static ADCLinearSensor<10> &voltage_sensor() {
        static ADCLinearSensor<10> voltage_sensor{
            VOLTAGE_PIN,
            static_cast<uint16_t>(Comms::IDPacket::VOLTAGE),
            VOLTAGE_SLOPE,
            VOLTAGE_OFFSET,
            {Comms::Target::CONTROL_STATION, Comms::Target::LCU}};
        return voltage_sensor;
    }

    static ADCLinearSensor<15> &current_sensor() {
        static ADCLinearSensor<15> current_sensor{
            CURRENT_PIN,
            static_cast<uint16_t>(Comms::IDPacket::CURRENT),
            CURRENT_SLOPE,
            CURRENT_OFFSET,
            {Comms::Target::CONTROL_STATION}};
        return current_sensor;
    }

    static IMD &imd() {
        static IMD imd{M_LS_PIN, IMD_POW, IMD_OK,
                       static_cast<uint16_t>(Comms::IDPacket::IMD)};
        return imd;
    }

    static SDC &sdc() {
        static SDC sdc{SDC_GOOD_PIN,
                       static_cast<uint16_t>(Comms::IDPacket::SDC)};
        return sdc;
    }

    static BatteryPack<N_BATTERIES> &batteries() {
        static BatteryPack<N_BATTERIES> batteries{
            static_cast<uint16_t>(Comms::IDPacket::TOTAL_VOLTAGE),
            static_cast<uint16_t>(Comms::IDPacket::DRIVER_DIAG),
            static_cast<uint16_t>(Comms::IDPacket::BATTERY_1)};
        return batteries;
    }

    static void init();
    static void start();
    static void update();
};
}  // namespace HVSCU

#endif