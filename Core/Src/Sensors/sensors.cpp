#include "Sensors/Sensors.hpp"

#include "Comms/Comms.hpp"

namespace HVSCU {
void BMSConfig::SPI_transmit(const span<uint8_t> data) {
    SPI::Instance *spi = SPI::registered_spi[spi_id];
    HAL_SPI_Transmit(spi->hspi, data.data(), data.size(), 10);
}
void BMSConfig::SPI_receive(span<uint8_t> buffer) {
    SPI::Instance *spi = SPI::registered_spi[spi_id];
    HAL_SPI_Receive(spi->hspi, buffer.data(), buffer.size(), 10);
}
void BMSConfig::SPI_CS_turn_off() {
    SPI::Instance *spi = SPI::registered_spi[spi_id];
    SPI::turn_off_chip_select(spi);
}
void BMSConfig::SPI_CS_turn_on() {
    SPI::Instance *spi = SPI::registered_spi[spi_id];
    SPI::turn_on_chip_select(spi);
}

#if BATTERIES_CONNECTED
int32_t BMSConfig::get_tick() { return Sensors::us_counter; }
#endif

void Sensors::init() {
    voltage_sensor();
    current_sensor();
    if constexpr (BATTERIES_CONNECTED) {
        BMSConfig::spi_id = SPI::inscribe(SPI::spi3);
    }
}

void Sensors::start() {
#if BATTERIES_CONNECTED
    Time::register_low_precision_alarm(READING_PERIOD_US / 1000, [&]() {
        process_batteries_data_flag = true;
    });
    Time::register_high_precision_alarm(500, +[]() { ++Sensors::us_counter; });
    for (int i{0}; i < N_BATTERIES; ++i) {
        auto battery_packet = new HeapPacket(
            static_cast<uint16_t>(Comms::IDPacket::BATTERY_1) + i, &dummy,
            &batteries[i].cells[0], &batteries[i].cells[1],
            &batteries[i].cells[2], &batteries[i].cells[3],
            &batteries[i].cells[4], &batteries[i].cells[5], &dummy, &dummy,
            &batteries_temp[i][0], &batteries_temp[i][1], &dummy_bool,
            &batteries[i].total_voltage);

        Comms::add_packet(battery_packet);
    }

    auto total_voltage_packet = new HeapPacket(
        static_cast<uint16_t>(Comms::IDPacket::TOTAL_VOLTAGE), &total_voltage);
    Comms::add_packet(total_voltage_packet);

    auto driver_diag_packet = new HeapPacket(
        static_cast<uint16_t>(Comms::IDPacket::DRIVER_DIAG),
        &driver_diag.reading_period, &driver_diag.success_conv_rate);
    Comms::add_packet(driver_diag_packet);
#endif
    Time::register_low_precision_alarm(10,
                                       [&]() { reading_sensors_flag = true; });
}

void Sensors::update() {
#if BATTERIES_CONNECTED
    if (process_batteries_data_flag) {
        process_batteries_data();
        process_batteries_data_flag = false;
    }
    bms.update();
#endif
    if (reading_sensors_flag) {
        voltage_sensor().read();
        current_sensor().read();
        reading_sensors_flag = false;
    }
}

#if BATTERIES_CONNECTED
void Sensors::process_batteries_data() {
    // Total batteries voltage
    float voltage = 0.0;
    for (auto &battery : batteries) {
        voltage += battery.total_voltage;
    }
    total_voltage = voltage;

    // Batteries temperatures
    for (auto i{0}; i < N_BATTERIES; ++i) {
        for (auto j{0}; j < 2; ++j) {
            auto voltage = batteries[i].GPIOs[j];
            auto resistance = (-VOLTAGE_REFERENCE * RESISTANCE_REFERENCE) /
                              (voltage - VOLTAGE_REFERENCE);
            batteries_temp[i][j] = (resistance - R0) / (TCR * R0);
        }
    }
}
#endif
}  // namespace HVSCU