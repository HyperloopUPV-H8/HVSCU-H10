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
#if BATTERIES_CONNECTED
    static bool cell_conversion_flag;
#endif
    static bool current_reading_flag;

    enum TURNO { CELLS, TEMPS };
    static TURNO turno;

#if BATTERIES_CONNECTED
    static void cell_conversion();
#endif
    static void read_current();

   public:
    static BMSH *bmsh;
    static std::array<float, BMS::EXTERNAL_ADCS> converted_temps;

    static float current_reading;
    static float voltage_reading;
    static float total_voltage;

    static float PPU1_voltage;
    static float PPU2_voltage;

    static void start();
    static void update();
    static void zeroing();
};
