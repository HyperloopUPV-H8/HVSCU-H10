#include "Sensors/Sensors.hpp"

#include "Comms/Comms.hpp"

namespace HVSCU {

uint8_t spi_id;

void SPI_transmit(const span<uint8_t> data) {
    SPI::Instance *spi = SPI::registered_spi[spi_id];
    HAL_SPI_Transmit(spi->hspi, data.data(), data.size(), 10);
}
void SPI_receive(span<uint8_t> buffer) {
    SPI::Instance *spi = SPI::registered_spi[spi_id];
    HAL_SPI_Receive(spi->hspi, buffer.data(), buffer.size(), 10);
}
void SPI_CS_turn_on() {
    SPI::Instance *spi = SPI::registered_spi[spi_id];
    SPI::turn_on_chip_select(spi);
}
void SPI_CS_turn_off() {
    SPI::Instance *spi = SPI::registered_spi[spi_id];
    SPI::turn_off_chip_select(spi);
}
uint32_t get_tick() { return HAL_GetTick(); }

ADCLinearSensor<10> *Sensors::voltage_sensor{nullptr};
ADCLinearSensor<10> *Sensors::current_sensor{nullptr};
bool Sensors::reading_sensors_flag{false};

#if BATTERIES_CONNECTED
bool Sensors::read_total_voltage_flag{false};
float Sensors::total_voltage{0.0};

#else
float Sensors::total_voltage{FAKE_TOTAL_VOLTAGE};
#endif

void Sensors::init() {
    voltage_sensor = new ADCLinearSensor<10>(
        VOLTAGE_PIN, static_cast<uint16_t>(Comms::IDPacket::VOLTAGE),
        VOLTAGE_SLOPE, VOLTAGE_OFFSET);
    current_sensor = new ADCLinearSensor<10>(
        CURRENT_PIN, static_cast<uint16_t>(Comms::IDPacket::CURRENT),
        CURRENT_SLOPE, CURRENT_OFFSET);

    spi_id = SPI::inscribe(SPI::spi3);
}

void Sensors::start() {
#if BATTERIES_CONNECTED
    Time::register_low_precision_alarm(
        10, [&]() { read_total_voltage_flag = true; });
    for (int i = 0; i < 10; ++i) {
        auto battery_packet = new HeapPacket(
            static_cast<uint16_t>(Comms::IDPacket::BATTERY_1) + i, &dummy,
            &batteries[i].cells[0], &batteries[i].cells[1],
            &batteries[i].cells[2], &batteries[i].cells[3],
            &batteries[i].cells[4], &batteries[i].cells[5], &dummy, &dummy,
            &dummy, &dummy, &dummy_bool, &dummy);

        Comms::add_packet(battery_packet);
    }

    auto total_voltage_packet = new HeapPacket(
        static_cast<uint16_t>(Comms::IDPacket::TOTAL_VOLTAGE), &total_voltage);
    Comms::add_packet(total_voltage_packet);
#endif
    Time::register_low_precision_alarm(10,
                                       [&]() { reading_sensors_flag = true; });
}

void Sensors::update() {
#if BATTERIES_CONNECTED
    if (read_total_voltage_flag) {
        read_total_voltage();
        read_total_voltage_flag = false;
    }
    bms.update();
#endif
    if (reading_sensors_flag) {
        voltage_sensor->read();
        current_sensor->read();
        reading_sensors_flag = false;
    }
}

#if BATTERIES_CONNECTED
void Sensors::read_total_voltage() {
    float voltage = 0.0;
    for (auto &battery : batteries) {
        voltage += battery.total_voltage;
    }
    total_voltage = voltage;
}
#endif
}  // namespace HVSCU