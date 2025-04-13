#include "Sensors/Sensors.hpp"

// Olf BMS-LIB relative stuff
BMSH *Sensors::bmsh;
std::array<float, BMS::EXTERNAL_ADCS> Sensors::converted_temps;
std::array<float, BMS::EXTERNAL_ADCS> Sensors::offset_batteries_temps = {
    1273.9, 1273.9, 1273.9, 1273.9, 1273.9,
    1273.9, 1273.9, 1273.9, 1273.9, 1273.9,
};
Sensors::TURNO Sensors::turno = CELLS;

// Sensors
std::unique_ptr<CurrentSensor> Sensors::current_sensor = nullptr;
std::unique_ptr<VoltageSensor> Sensors::voltage_sensor = nullptr;

// Flags
bool Sensors::cell_conversion_flag = false;
bool Sensors::current_reading_flag = false;
bool Sensors::voltage_reading_flag = false;

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
#endif
    }
    for (auto &adc : bmsh->external_adcs) {
        adc.battery.update_data();
    }
}

void Sensors::init() {
    bmsh = new BMSH(SPI::spi3);
    current_sensor = std::make_unique<CurrentSensor>();
    voltage_sensor = std::make_unique<VoltageSensor>();
}

void Sensors::start() {
    bmsh->initialize();

    current_sensor->zeroing();

    Time::register_low_precision_alarm(11,
                                       [&]() { cell_conversion_flag = true; });
    Time::register_low_precision_alarm(10,
                                       [&]() { current_reading_flag = true; });
    Time::register_low_precision_alarm(10,
                                       [&]() { voltage_reading_flag = true; });
}

void Sensors::update() {
    if (cell_conversion_flag) {
        cell_conversion();
        cell_conversion_flag = false;
    }
    if (current_reading_flag) {
        current_sensor->read_current();
        current_reading_flag = false;
    }
    if (voltage_reading_flag) {
        voltage_sensor->read_voltage();
        voltage_reading_flag = false;
    }
}