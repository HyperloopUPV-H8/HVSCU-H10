#include "Sensors/Sensors.hpp"

#include "Comms/Comms.hpp"

namespace HVSCU {

ADCLinearSensor *Sensors::voltage_sensor{nullptr};
ADCLinearSensor *Sensors::current_sensor{nullptr};
bool Sensors::reading_sensors_flag{false};

// Old BMS-LIB stuff
BMSH *Sensors::bmsh;
std::array<float, BMS::EXTERNAL_ADCS> Sensors::converted_temps;
std::array<float, BMS::EXTERNAL_ADCS> Sensors::offset_batteries_temps = {
    1273.9, 1273.9, 1273.9, 1273.9, 1273.9,
    1273.9, 1273.9, 1273.9, 1273.9, 1273.9,
};
bool Sensors::cell_conversion_flag{false};
Sensors::TURNO Sensors::turno = CELLS;
float Sensors::total_voltage{};

void Sensors::start() {
    voltage_sensor = new ADCLinearSensor(
        VOLTAGE_PIN, static_cast<uint16_t>(Comms::IDPacket::VOLTAGE),
        VOLTAGE_SLOPE, VOLTAGE_OFFSET);
    current_sensor = new ADCLinearSensor(
        CURRENT_PIN, static_cast<uint16_t>(Comms::IDPacket::CURRENT),
        CURRENT_SLOPE, CURRENT_OFFSET);

    bmsh = new BMSH(SPI::spi3);

    Time::register_low_precision_alarm(11,
                                       [&]() { cell_conversion_flag = true; });
    Time::register_low_precision_alarm(10,
                                       [&]() { reading_sensors_flag = true; });
}

void Sensors::update() {
    if (cell_conversion_flag) {
        cell_conversion();
        cell_conversion_flag = false;
    }
    if (reading_sensors_flag) {
        voltage_sensor->read();
        current_sensor->read();
        reading_sensors_flag = false;
    }
}

void Sensors::cell_conversion() {
    bmsh->wake_up();
    if (turno == CELLS) {
        bmsh->update_cell_voltages();
#if READING_BATTERY_TEMPERATURES
        turno = TEMPS;
    } else {
        bmsh->measure_internal_device_parameters();
        bmsh->start_adc_conversion_gpio();
        bmsh->update_temperatures();
        bmsh->read_internal_temperature();
        turno = CELLS;
    }

    for (int i = 0; i < BMS::EXTERNAL_ADCS; i++) {
        auto val = bmsh->external_adcs[i].battery.filtered_temp *
                       gain_batteries_temperatures +
                   offset_batteries_temps[i];
        converted_temps[i] = val;
    }
#else
    }
#endif
    for (auto &adc : bmsh->external_adcs) {
        adc.battery.update_data();
    }
}
}  // namespace HVSCU