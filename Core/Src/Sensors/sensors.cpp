#include "Sensors/Sensors.hpp"

#include "Comms/Comms.hpp"

namespace HVSCU {

ADCLinearSensor *Sensors::voltage_sensor{nullptr};
ADCLinearSensor *Sensors::current_sensor{nullptr};
bool Sensors::reading_sensors_flag{false};

#if BATTERIES_CONNECTED
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
#else
float Sensors::total_voltage{FAKE_TOTAL_VOLTAGE};
#endif

void Sensors::init() {
    voltage_sensor = new ADCLinearSensor(
        VOLTAGE_PIN, static_cast<uint16_t>(Comms::IDPacket::VOLTAGE),
        VOLTAGE_SLOPE, VOLTAGE_OFFSET);
    current_sensor = new ADCLinearSensor(
        CURRENT_PIN, static_cast<uint16_t>(Comms::IDPacket::CURRENT),
        CURRENT_SLOPE, CURRENT_OFFSET);

#if BATTERIES_CONNECTED
    bmsh = new BMSH(SPI::spi3);
#endif
}

void Sensors::start() {
#if BATTERIES_CONNECTED
    Sensors::bmsh->initialize();

    Time::register_low_precision_alarm(11,
                                       [&]() { cell_conversion_flag = true; });
    for (int i = 0; i < 10; ++i) {
        auto battery_packet = new HeapPacket(
            static_cast<uint16_t>(Comms::IDPacket::BATTERY_1) + i,
            &Sensors::bmsh->external_adcs[i].battery.SOC,
            Sensors::bmsh->external_adcs[i].battery.cells[0],
            Sensors::bmsh->external_adcs[i].battery.cells[1],
            Sensors::bmsh->external_adcs[i].battery.cells[2],
            Sensors::bmsh->external_adcs[i].battery.cells[3],
            Sensors::bmsh->external_adcs[i].battery.cells[4],
            Sensors::bmsh->external_adcs[i].battery.cells[5],
            &Sensors::bmsh->external_adcs[i].battery.minimum_cell,
            &Sensors::bmsh->external_adcs[i].battery.maximum_cell,
            &Sensors::converted_temps[i],
            Sensors::bmsh->external_adcs[i].battery.temperature2,
            &Sensors::bmsh->external_adcs[i].battery.is_balancing,
            &Sensors::bmsh->external_adcs[i].battery.total_voltage);

        Comms::add_packet(battery_packet);
    }
#endif
    Time::register_low_precision_alarm(10,
                                       [&]() { reading_sensors_flag = true; });
}

void Sensors::update() {
#if BATTERIES_CONNECTED
    if (cell_conversion_flag) {
        cell_conversion();
        cell_conversion_flag = false;
    }
#endif
    if (reading_sensors_flag) {
        voltage_sensor->read();
        current_sensor->read();
        reading_sensors_flag = false;
    }
}

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
    }
#else
    }
#endif
    for (auto &adc : bmsh->external_adcs) {
        adc.battery.update_data();
    }
}
#endif
}  // namespace HVSCU