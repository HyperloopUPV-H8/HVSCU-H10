#ifndef BATTERY_PACK_HPP
#define BATTERY_PACK_HPP

#include "BMS.hpp"

#define READING_PERIOD_US 20000   // us
#define CONV_RATE_TIME_MS 1000    // ms
#define FAKE_TOTAL_VOLTAGE 250.0  // V
#define NOMINAL_CAPACITY 6        // Ah
#define MIN_VOLTAGE 22.0          // V
#define MAX_VOLTAGE 25.0          // V
#define OCV_POINTS 256

#define RESISTANCE_REFERENCE 1000.0  // Ohmios
#define VOLTAGE_REFERENCE 3.0        // V
#define R0 100.0                     // Ohmios
#define TCR 0.00385

#define TEMP_CHEAT 1  // Ñapa para temperaturas, esto no debería existir

namespace HVSCU {
template <size_t N_BATTERIES>
class BatteryPack {
    using Battery =
        LTC6810Driver::LTC6810<6, READING_PERIOD_US, CONV_RATE_TIME_MS>;
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
        static constexpr int32_t conv_rate_time_ms{CONV_RATE_TIME_MS};
    };

    template <size_t points>
    constexpr array<std::pair<float, float>, points> calculate_OCV() {
        float A{-0.008697};
        float B{0.521307};
        float C{-9.736016};
        float D{54.414283};

        auto delta = (MAX_VOLTAGE - MIN_VOLTAGE) / (points - 1);
        array<std::pair<float, float>, points> result;
        for (size_t i{0}; i < points; ++i) {
            auto x = MIN_VOLTAGE + i * delta;
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

    HeapPacket total_voltage_packet;
    HeapPacket reading_period_packet;
    HeapPacket minimum_soc_packet;
    HeapPacket batteries_data_packet;
    array<std::unique_ptr<HeapPacket>, N_BATTERIES> battery_packets;
    float minimum_soc{1.0};
    float minimum_temp{100.0};
    float maximum_temp{-100.0};
    float minimum_cell_voltage{5.0};
    float maximum_cell_voltage{0.0};

#if TEMP_CHEAT
    FloatMovingAverage<100> temp_average{};
#endif

    void get_SoC(uint i, float current, float &temp_minimum_soc) {
        auto now = HAL_GetTick();
        float new_soc;
        if (std::abs(current) < 0.1) {  // Coulomb counting
            float delta_time = (now - SoCs[i].first) / 1000.0f;
            float delta_soc =
                (current * delta_time) / (NOMINAL_CAPACITY * 3600.0);
            new_soc = SoCs[i].second - delta_soc;

            SoCs[i] = std::make_pair(now, new_soc);
        } else {  // OCV
            new_soc = lookup_OCV(batteries[i].total_voltage);
            SoCs[i] = std::make_pair(now, new_soc);
        }

        temp_minimum_soc = std::min(temp_minimum_soc, new_soc);
    }

    void read_temps(uint i) {
        for (auto j{0}; j < 2; ++j) {
            auto GPIO_voltage = batteries[i].GPIOs[j];
            auto resistance = (GPIO_voltage * RESISTANCE_REFERENCE) /
                              (VOLTAGE_REFERENCE - GPIO_voltage);

            if constexpr (TEMP_CHEAT) {
                auto temp = (resistance - R0) / (TCR * R0);
                auto val_average = temp_average.compute(temp);
                if (std::abs(temp - val_average) > 5) {
                    temp = val_average;
                }
                batteries_temp[i][j] = temp;
            } else {
                batteries_temp[i][j] = (resistance - R0) / (TCR * R0);
            }
        }
    }

   public:
    array<Battery, N_BATTERIES> &batteries = bms.get_data();
    float total_voltage{FAKE_TOTAL_VOLTAGE};
    array<std::pair<uint, float>, N_BATTERIES> SoCs{};  // ms -> soc[0,1]
    array<array<float, 2>, N_BATTERIES> batteries_temp{};

    BatteryPack(uint16_t total_voltage_id, uint16_t reading_period_id,
                uint16_t battery_id, uint16_t minimum_soc_id,
                uint16_t batteries_data_id)
        : total_voltage_packet{total_voltage_id, &total_voltage},
          reading_period_packet{reading_period_id, &bms.get_period()},
          minimum_soc_packet{minimum_soc_id, &minimum_soc},
          batteries_data_packet{batteries_data_id, &minimum_cell_voltage,
                                &maximum_cell_voltage, &minimum_temp,
                                &maximum_temp} {
        Comms::add_packet(Comms::Target::CONTROL_STATION,
                          &total_voltage_packet);
        Comms::add_packet(Comms::Target::CONTROL_STATION,
                          &reading_period_packet);
        Comms::add_packet(Comms::Target::CONTROL_STATION, &minimum_soc_packet);
        Comms::add_packet(Comms::Target::CONTROL_STATION,
                          &batteries_data_packet);
        for (uint16_t i{0}; i < N_BATTERIES; ++i) {
            battery_packets[i] = std::make_unique<HeapPacket>(
                battery_id + i, &SoCs[i].second, &batteries[i].cells[0],
                &batteries[i].cells[1], &batteries[i].cells[2],
                &batteries[i].cells[3], &batteries[i].cells[4],
                &batteries[i].cells[5], &batteries_temp[i][0],
                &batteries_temp[i][1], &batteries[i].total_voltage,
                &batteries[i].conv_rate);

            Comms::add_packet(Comms::Target::CONTROL_STATION,
                              battery_packets[i].get());
        }

        SoCs.fill({0, 1.0});
        BMSConfig::spi_id = SPI::inscribe(SPI::spi3);
    }

    void start() {
        SoCs.fill(std::pair<uint, float>{0, 1.0});
        Time::register_high_precision_alarm(
            500, +[]() { ++BMSConfig::us_counter; });
    }

    void update() { bms.update(); }

    void read(float current) {
        float voltage{};
        float temp_minimum_soc{1.0};
        float min_v_cell_temp{std::numeric_limits<float>::max()};
        float max_v_cell_temp{std::numeric_limits<float>::min()};
        float min_temperature_temp{std::numeric_limits<float>::max()};
        float max_temperature_temp{std::numeric_limits<float>::min()};

        for (uint i = 0; i < N_BATTERIES; ++i) {
            voltage += batteries[i].total_voltage;

            get_SoC(i, current, temp_minimum_soc);

            read_temps(i);

            for (const auto &v : batteries[i].cells) {
                min_v_cell_temp = std::min(min_v_cell_temp, v);
                max_v_cell_temp = std::max(max_v_cell_temp, v);
            }

            for (const auto &t : batteries_temp[i]) {
                min_temperature_temp = std::min(min_temperature_temp, t);
                max_temperature_temp = std::max(max_temperature_temp, t);
            }
        }

        total_voltage = voltage;
        minimum_soc = temp_minimum_soc;
        minimum_cell_voltage = min_v_cell_temp;
        maximum_cell_voltage = max_v_cell_temp;
        minimum_temp = min_temperature_temp;
        maximum_temp = max_temperature_temp;
    }
};
}  // namespace HVSCU

#endif
