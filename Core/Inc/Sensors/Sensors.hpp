#ifndef SENSORS_HPP
#define SENSORS_HPP

#include "ADCLinearSensor.hpp"
#include "BMS.hpp"

#define BATTERIES_CONNECTED 1
#define N_BATTERIES 3
#define READING_PERIOD_US 10000
#define FAKE_TOTAL_VOLTAGE 250.0

namespace HVSCU {
struct BMSConfig {
    static inline uint8_t spi_id{};

    static constexpr size_t n_LTC6810{3};
    static void SPI_transmit(const std::span<uint8_t> data);
    static void SPI_receive(std::span<uint8_t> buffer);
    static void SPI_CS_turn_on(void);
    static void SPI_CS_turn_off(void);
    static int32_t get_tick(void);
    static constexpr int32_t tick_resolution_us{500};
    static constexpr int32_t period_us{READING_PERIOD_US};
};

class Sensors {
    // Voltage sensor
    static constexpr Pin &VOLTAGE_PIN{PF13};
    static constexpr float VOLTAGE_SLOPE{124.299234299011000};
    static constexpr float VOLTAGE_OFFSET{-5.409069953014590};

    // Current sensor
    static constexpr Pin &CURRENT_PIN{PA0};
    static constexpr float CURRENT_SLOPE{92.27959442138672};
    static constexpr float CURRENT_OFFSET{-152.49656677246094};

    static inline bool reading_sensors_flag{false};

#if BATTERIES_CONNECTED
    static inline bool read_total_voltage_flag{false};
#endif

   public:
    static ADCLinearSensor<10> &voltage_sensor() {
        static ADCLinearSensor<10> voltage_sensor{
            VOLTAGE_PIN, static_cast<uint16_t>(Comms::IDPacket::VOLTAGE),
            VOLTAGE_SLOPE, VOLTAGE_OFFSET};
        return voltage_sensor;
    }

    static ADCLinearSensor<10> &current_sensor() {
        static ADCLinearSensor<10> current_sensor{
            CURRENT_PIN, static_cast<uint16_t>(Comms::IDPacket::CURRENT),
            CURRENT_SLOPE, CURRENT_OFFSET};
        return current_sensor;
    }
#if BATTERIES_CONNECTED
    static constexpr BMS<BMSConfig> bms{};

    static inline auto &batteries = bms.get_data();
    static inline auto &driver_diag = bms.get_diag();
    static inline float dummy{0.0};
    static inline bool dummy_bool{false};
    static inline float total_voltage{};
    static inline int32_t us_counter{};
#else
    static inline float total_voltage{FAKE_TOTAL_VOLTAGE};
#endif

    static void init();
    static void start();
    static void update();
#if BATTERIES_CONNECTED
    static void read_total_voltage();
#endif
};
}  // namespace HVSCU

#endif