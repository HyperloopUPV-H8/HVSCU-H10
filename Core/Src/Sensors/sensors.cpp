#include "Sensors/Sensors.hpp"

BMSH *Sensors::bmsh;

void Sensors::start() {
    bmsh = new BMSH(SPI::spi3);
    bmsh->initialize();
}