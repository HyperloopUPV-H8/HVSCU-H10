#include "Sensors/Sensors.hpp"

#include "Comms/Comms.hpp"

namespace HVSCU {
void Sensors::init() {
    voltage_sensor();
    current_sensor();
    imd();
    batteries();
}

void Sensors::start() {
    if constexpr (BATTERIES_CONNECTED) {
        batteries().start();
        Time::register_low_precision_alarm(READING_PERIOD_US / 1000, [&]() {
            reading_batteries_flag = true;
            process_batteries_data_flag = true;
        });
    }
    Time::register_low_precision_alarm(17,
                                       [&]() { reading_sensors_flag = true; });

    imd().power_on();
}

void Sensors::update() {
    if constexpr (BATTERIES_CONNECTED) {
        batteries().update();

        if (reading_batteries_flag) {
            batteries().read(current_sensor().reading);
            reading_batteries_flag = false;
        }
        if (process_batteries_data_flag) {
            process_batteries_data();
            process_batteries_data_flag = false;
        }
    }

    if (reading_sensors_flag) {
        voltage_sensor().read();
        current_sensor().read();
        imd().read();
        reading_sensors_flag = false;
    }
}

#if BATTERIES_CONNECTED
void Sensors::process_batteries_data() {
    // Total batteries voltage
    float voltage = 0.0;
    for (auto &battery : batteries) {
        voltage += battery.total_voltage;
    }
    total_voltage = voltage;

    // Batteries temperatures
    for (auto i{0}; i < N_BATTERIES; ++i) {
        for (auto j{0}; j < 2; ++j) {
            auto voltage = batteries[i].GPIOs[j];
            auto resistance = (-VOLTAGE_REFERENCE * RESISTANCE_REFERENCE) /
                              (voltage - VOLTAGE_REFERENCE);
            batteries_temp[i][j] = (resistance - R0) / (TCR * R0);
        }
    }
}
#endif
}  // namespace HVSCU
