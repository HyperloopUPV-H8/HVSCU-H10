#include "ADCLinearSensor.hpp"
#include "BMS-LIB.hpp"
#include "HVSCUConfig.hpp"

namespace HVSCU {
class Sensors {
    // Voltage sensor
    static constexpr Pin &VOLTAGE_PIN{PF13};
    static constexpr float VOLTAGE_SLOPE{124.299234299011000};
    static constexpr float VOLTAGE_OFFSET{-5.409069953014590};

    // Current sensor
    static constexpr Pin &CURRENT_PIN{PA0};
    static constexpr float CURRENT_SLOPE{1.0};
    static constexpr float CURRENT_OFFSET{};

    static ADCLinearSensor *voltage_sensor;
    static ADCLinearSensor *current_sensor;
    static bool reading_sensors_flag;

    // Old BMS-LIB stuff
    constexpr static float gain_batteries_temperatures = -795.45;
    static std::array<float, BMS::EXTERNAL_ADCS> offset_batteries_temps;
    static bool cell_conversion_flag;
    enum TURNO { CELLS, TEMPS };
    static TURNO turno;

   public:
    static BMSH *bmsh;
    static std::array<float, BMS::EXTERNAL_ADCS> converted_temps;

    static void start();
    static void update();
    static void cell_conversion();
};
}  // namespace HVSCU