#include "BMS-LIB.hpp"
#include "CurrentSensor.hpp"
#include "VoltageSensor.hpp"
#include "HVSCUConfig.hpp"

class Sensors {
    constexpr static float gain_batteries_temperatures = -795.45;

    // Old BMS-LIB relative stuff
    static std::array<float, BMS::EXTERNAL_ADCS> offset_batteries_temps;
    enum TURNO { CELLS, TEMPS };
    static TURNO turno;
    static void cell_conversion();

    // Flags
    static bool cell_conversion_flag;
    static bool current_reading_flag;
    static bool voltage_reading_flag;

   public:
    // Sensors
    static std::unique_ptr<CurrentSensor> current_sensor;
    static std::unique_ptr<VoltageSensor> voltage_sensor;

    // Old BMS-LIB relative stuff
    static BMSH *bmsh;
    static std::array<float, BMS::EXTERNAL_ADCS> converted_temps;

    static void init();
    static void start();
    static void update();
    static void zeroing();
};
