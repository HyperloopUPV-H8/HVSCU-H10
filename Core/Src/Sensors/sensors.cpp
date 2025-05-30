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
    float total_voltage_temp{0.0};
    for (auto i{0}; i < N_BATTERIES; ++i) {
        total_voltage_temp += batteries[i].total_voltage;

        auto GPIO_voltage = batteries[i].GPIOs[0];
        auto resistance = (GPIO_voltage * RESISTANCE_REFERENCE) /
                          (VOLTAGE_REFERENCE - GPIO_voltage);
        batteries_temp[i] = (resistance - R0) / (TCR * R0);
    }
    total_voltage = total_voltage_temp;
}
#endif
}  // namespace HVSCU
