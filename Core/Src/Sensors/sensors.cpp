#include "Sensors/Sensors.hpp"

#include "Comms/Comms.hpp"

namespace HVSCU {
void Sensors::init() {
    voltage_sensor();
    current_sensor();
    if constexpr (BATTERIES_CONNECTED) {
        batteries();
    }
}

void Sensors::start() {
#if BATTERIES_CONNECTED
    Time::register_low_precision_alarm(
        READING_PERIOD_US / 1000, [&]() { reading_batteries_flag = true; });
#endif
    Time::register_low_precision_alarm(10,
                                       [&]() { reading_sensors_flag = true; });
}

void Sensors::update() {
#if BATTERIES_CONNECTED
    batteries().update();

    if (reading_batteries_flag) {
        batteries().read(current_sensor().reading);
        reading_batteries_flag = false;
    }
#endif
    if (reading_sensors_flag) {
        voltage_sensor().read();
        current_sensor().read();
        reading_sensors_flag = false;
    }
}
}  // namespace HVSCU
