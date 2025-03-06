#include "BMS-LIB.hpp"
#include "HVSCUConfig.hpp"

#define ZEROING_MEASURE 100000

class Sensors {
   private:
#if SENSOR_CHARACTERIZATION
    static constexpr float slope = 1.0;
#else
    static constexpr float slope = 92.9342343462816;
#endif
    static float offset;
    constexpr static float gain_batteries_temperatures = -795.45;
    static std::array<float, BMS::EXTERNAL_ADCS> offset_batteries_temps;
    static LinearSensor<float> *current_sensor;
    static uint8_t voltage_adc_id;
    static bool cell_conversion_flag;
    static bool current_reading_flag;

    enum TURNO { CELLS, TEMPS };
    static TURNO turno;

    static void cell_conversion();
    static void read_current();

   public:
    static BMSH *bmsh;
    static std::array<float, BMS::EXTERNAL_ADCS> converted_temps;

    static float current_reading;
    static float voltage_reading;

    static void start();
    static void update();
    static void zeroing();
};
