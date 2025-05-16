#ifndef SENSORS_HPP
#define SENSORS_HPP

#include "ADCLinearSensor.hpp"
#include "BMS.hpp"

#define BATTERIES_CONNECTED 1
#define READING_BATTERIES_TEMPERATURE 0
#define FAKE_TOTAL_VOLTAGE 250.0

namespace HVSCU {

void SPI_transmit(const span<uint8_t> data);
void SPI_receive(span<uint8_t> buffer);
void SPI_CS_turn_on();
void SPI_CS_turn_off();
uint32_t get_tick();

class Sensors {
    // Voltage sensor
    static constexpr Pin &VOLTAGE_PIN{PF13};
    static constexpr float VOLTAGE_SLOPE{124.299234299011000};
    static constexpr float VOLTAGE_OFFSET{-5.409069953014590};

    // Current sensor
    static constexpr Pin &CURRENT_PIN{PA0};
    static constexpr float CURRENT_SLOPE{92.27959442138672};
    static constexpr float CURRENT_OFFSET{-152.49656677246094};

    static bool reading_sensors_flag;

#if BATTERIES_CONNECTED
    static bool read_total_voltage_flag;
#endif

   public:
    static ADCLinearSensor<10> *voltage_sensor;
    static ADCLinearSensor<10> *current_sensor;
#if BATTERIES_CONNECTED
    static constexpr BMS<10, SPI_transmit, SPI_receive, SPI_CS_turn_on,
                         SPI_CS_turn_off, get_tick, 1, 10>
        bms{};

    static inline auto &batteries = bms.get_data();
    static inline float dummy{0.0};
    static inline bool dummy_bool{false};
#endif
    static float total_voltage;

    static void init();
    static void start();
    static void update();
#if BATTERIES_CONNECTED
    static void read_total_voltage();
#endif
};
}  // namespace HVSCU

#endif