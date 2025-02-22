#include "BMS-LIB.hpp"

class Sensors {
   private:
    constexpr static float gain_batteries_temperatures = -795.45;
    static std::array<float, BMS::EXTERNAL_ADCS> offset_batteries_temps;

   public:
    static BMSH *bmsh;
    static std::array<float, BMS::EXTERNAL_ADCS> converted_temps;

    void start();
};