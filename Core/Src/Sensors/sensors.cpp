#include "Sensors/Sensors.hpp"

#include "Comms/Comms.hpp"

namespace HVSCU {
void Sensors::init() {
    voltage_sensor();
    current_sensor();
    imd();
    if constexpr (BATTERIES_CONNECTED) {
        batteries();
    }
}

void Sensors::start() {
    if constexpr (BATTERIES_CONNECTED) {
        batteries().start();
        Time::register_low_precision_alarm(
            READING_PERIOD_US / 1000, [&]() { reading_batteries_flag = true; });
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
    }
    if (reading_sensors_flag) {
        voltage_sensor().read();
        current_sensor().read();
        imd().read();
        reading_sensors_flag = false;
    }
}
}  // namespace HVSCU
