#include "Sensors/Sensors.hpp"

float Sensors::offset = -153.898815693102;
BMSH *Sensors::bmsh;
std::array<float, BMS::EXTERNAL_ADCS> Sensors::converted_temps;
std::array<float, BMS::EXTERNAL_ADCS> Sensors::offset_batteries_temps = {
    1273.9, 1273.9, 1273.9, 1273.9, 1273.9,
    1273.9, 1273.9, 1273.9, 1273.9, 1273.9,
};
Sensors::TURNO Sensors::turno = CELLS;

LinearSensor<float> *Sensors::current_sensor = nullptr;
uint8_t Sensors::voltage_adc_id;
float Sensors::current_reading;
float Sensors::voltage_reading;

#if BATTERIES_CONNECTED
float Sensors::total_voltage = 0.0;
#else
float Sensors::total_voltage = 251.8;
#endif

float Sensors::PPU1_voltage = 0.0;
float Sensors::PPU2_voltage = 0.0;

#if BATTERIES_CONNECTED
bool Sensors::cell_conversion_flag = false;
#endif
bool Sensors::current_reading_flag = false;

#if BATTERIES_CONNECTED
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
    total_voltage = 0.0;
    for (auto &adc : bmsh->external_adcs) {
        adc.battery.update_data();
        total_voltage += adc.battery.total_voltage;
    }
}
#endif

void Sensors::read_current() {
    voltage_reading = ADC::get_value(voltage_adc_id);
    current_sensor->read();
}

void Sensors::zeroing() {
    float average_voltage = 0.0;
    for (int i = 0; i < ZEROING_MEASURE; ++i) {
        read_current();
        average_voltage += voltage_reading;
    }
    average_voltage /= ZEROING_MEASURE;
    offset = -slope * average_voltage;
    current_sensor->set_offset(offset);
}

void Sensors::start() {
    bmsh = new BMSH(SPI::spi3);

    current_sensor =
        new LinearSensor<float>(PA0, slope, offset, &current_reading);
    voltage_adc_id = current_sensor->get_id();

    zeroing();

#if BATTERIES_CONNECTED
    Time::register_low_precision_alarm(11,
                                       [&]() { cell_conversion_flag = true; });
#endif
    Time::register_low_precision_alarm(10,
                                       [&]() { current_reading_flag = true; });
}

void Sensors::update() {
#if BATTERIES_CONNECTED
    if (cell_conversion_flag) {
        cell_conversion();
        cell_conversion_flag = false;
    }
#endif
    if (current_reading_flag) {
        read_current();
        current_reading_flag = false;
    }
}