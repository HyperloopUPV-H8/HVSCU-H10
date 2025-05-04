#include "ADCLinearSensor.hpp"
#include "BMS-LIB.hpp"

#define BATTERIES_CONNECTED 0
#define READING_BATTERIES_TEMPERATURE 0
#define FAKE_TOTAL_VOLTAGE 50.0

namespace HVSCU {
class Sensors {
    // Voltage sensor
    static constexpr Pin &VOLTAGE_PIN{PF13};
    static constexpr float VOLTAGE_SLOPE{294.6192626953125};
    static constexpr float VOLTAGE_OFFSET{-15.624292373657227};

    // Current sensor
    static constexpr Pin &CURRENT_PIN{PA0};
    static constexpr float CURRENT_SLOPE{92.27959442138672};
    static constexpr float CURRENT_OFFSET{-152.49656677246094};

    static bool reading_sensors_flag;

#if BATTERIES_CONNECTED
    // Old BMS-LIB stuff
    constexpr static float gain_batteries_temperatures = -795.45;
    static std::array<float, BMS::EXTERNAL_ADCS> offset_batteries_temps;
    static bool cell_conversion_flag;
    enum TURNO { CELLS, TEMPS };
    static TURNO turno;
#endif

   public:
    static ADCLinearSensor *voltage_sensor;
    static ADCLinearSensor *current_sensor;
#if BATTERIES_CONNECTED
    static BMSH *bmsh;
    static std::array<float, BMS::EXTERNAL_ADCS> converted_temps;
#endif
    static float total_voltage;

    static void start();
    static void update();
#if BATTERIES_CONNECTED
    static void cell_conversion();
#endif
};
}  // namespace HVSCU