#ifndef BATTERY_PACK_HPP
#define BATTERY_PACK_HPP

#include "BMS.hpp"
#include "Sensors/Sensors.hpp"

#define READING_PERIOD_US 10000
#define FAKE_TOTAL_VOLTAGE 250.0  // V
#define NOMINAL_CAPACITY 6        // Ah
#define MIN_VOLTAGE 22.0          // V
#define MAX_VOLTAGE 25.0          // V
#define OCV_POINTS 256

namespace HVSCU {
template <size_t N_BATTERIES>
class BatteryPack {
    struct BMSConfig {
        static inline uint8_t spi_id{};
        static inline int32_t us_counter{};

        static constexpr size_t n_LTC6810{N_BATTERIES};
        static void SPI_transmit(const std::span<uint8_t> data) {
            SPI::Instance *spi = SPI::registered_spi[spi_id];
            HAL_SPI_Transmit(spi->hspi, data.data(), data.size(), 10);
        }
        static void SPI_receive(std::span<uint8_t> buffer) {
            SPI::Instance *spi = SPI::registered_spi[spi_id];
            HAL_SPI_Receive(spi->hspi, buffer.data(), buffer.size(), 10);
        }
        static void SPI_CS_turn_on(void) {
            SPI::Instance *spi = SPI::registered_spi[spi_id];
            SPI::turn_on_chip_select(spi);
        }
        static void SPI_CS_turn_off(void) {
            SPI::Instance *spi = SPI::registered_spi[spi_id];
            SPI::turn_off_chip_select(spi);
        }
        static int32_t get_tick(void) { return us_counter; }
        static constexpr int32_t tick_resolution_us{500};
        static constexpr int32_t period_us{READING_PERIOD_US};
    };

    template <size_t points>
    constexpr array<std::pair<float, float>, points> calculate_OCV() {
        float A{-0.8697};
        float B{52.1307};
        float C{-973.6016};
        float D{5441.4283};

        auto delta = (MAX_VOLTAGE - MIN_VOLTAGE) / (points - 1);
        array<std::pair<float, float>, points> result;
        for (size_t i{0}; i < points; ++i) {
            auto x = i * delta;
            auto soc = A * x * x * x + B * x * x + C * x + D;
            result[i] = std::make_pair(x, soc);
        }

        return result;
    }

    array<std::pair<float, float>, OCV_POINTS> ocv{calculate_OCV<OCV_POINTS>()};

    float lookup_OCV(float voltage) {
        constexpr float delta = (MAX_VOLTAGE - MIN_VOLTAGE) / (OCV_POINTS - 1);
        size_t index = static_cast<size_t>((voltage - MIN_VOLTAGE) / delta);

        if (index >= OCV_POINTS - 1) return ocv.back().second;

        float x0 = ocv[index].first;
        float y0 = ocv[index].second;
        float x1 = ocv[index + 1].first;
        float y1 = ocv[index + 1].second;

        float t = (voltage - x0) / (x1 - x0);
        return y0 + t * (y1 - y0);
    }

    const BMS<BMSConfig> bms{};
    array<Battery<6>, N_BATTERIES> &batteries = bms.get_data();
    array<std::pair<uint, float>, N_BATTERIES> SoCs{};
    BMSDiag &driver_diag = bms.get_diag();
    float dummy{0.0};
    bool dummy_bool{false};

    HeapPacket total_voltage_packet;
    HeapPacket driver_diag_packet;
    array<std::unique_ptr<HeapPacket>, N_BATTERIES> battery_packets;

   public:
    float total_voltage{FAKE_TOTAL_VOLTAGE};

    BatteryPack(uint16_t total_voltage_id, uint16_t driver_diag_id,
                uint16_t battery_id)
        : total_voltage_packet{total_voltage_id, &total_voltage},
          driver_diag_packet{driver_diag_id, &driver_diag.reading_period,
                             &driver_diag.success_conv_rate} {
        Comms::add_packet(&total_voltage_packet);
        Comms::add_packet(&driver_diag_packet);
        for (uint16_t i{0}; i < N_BATTERIES; ++i) {
            battery_packets[i] = std::make_unique<HeapPacket>(
                battery_id + i, &SoCs[i].second, &batteries[i].cells[0],
                &batteries[i].cells[1], &batteries[i].cells[2],
                &batteries[i].cells[3], &batteries[i].cells[4],
                &batteries[i].cells[5], &dummy, &dummy, &dummy, &dummy,
                &dummy_bool, &batteries[i].total_voltage);

            Comms::add_packet(battery_packets[i].get());
        }

        Time::register_high_precision_alarm(
            500, +[]() { ++BMSConfig::us_counter; });
    }

    void update() { bms.update(); }

    void read(float current) {
        float voltage = 0.0;
        for (auto &battery : batteries) {
            voltage += battery.total_voltage;
        }
        total_voltage = voltage;

        if (std::abs(current) < 0.01) {
            // Coulomb counting
            for (auto &soc : SoCs) {
                auto now = HAL_GetTick();
                float delta_time = (now - soc.first) / 1000.0f;
                float delta_soc = (current * delta_time) / NOMINAL_CAPACITY;
                float new_soc = soc.second - delta_soc;

                soc = std::make_pair(now, new_soc);
            }
        } else {
            // OCV
            for (uint i{0}; i < N_BATTERIES; ++i) {
                auto now = HAL_GetTick();
                float new_soc = lookup_OCV(batteries[i].total_voltage);
                SoCs[i] = std::make_pair(now, new_soc);
            }
        }
    }
};
}  // namespace HVSCU

#endif
