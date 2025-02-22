#include "Sensors/Sensors.hpp"

BMSH *Sensors::bmsh;
std::array<float, BMS::EXTERNAL_ADCS> Sensors::converted_temps;
std::array<float, BMS::EXTERNAL_ADCS> Sensors::offset_batteries_temps = {
    1273.9, 1273.9, 1273.9, 1273.9, 1273.9,
    1273.9, 1273.9, 1273.9, 1273.9, 1273.9,
};

void Sensors::start() {
    bmsh = new BMSH(SPI::spi3);
    bmsh->initialize();

    for (int i = 0; i < BMS::EXTERNAL_ADCS; i++) {
        auto val = bmsh->external_adcs[i].battery.filtered_temp *
                       gain_batteries_temperatures +
                   offset_batteries_temps[i];
        converted_temps[i] = val;
    }
}
