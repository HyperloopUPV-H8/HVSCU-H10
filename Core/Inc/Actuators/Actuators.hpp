#pragma once

#include "Actuators/Contactor.hpp"

namespace HVSCU {
class Actuators {
   private:
    static Contactor* contactor_low;
    static Contactor* contactor_high;
    static Contactor* contactor_precharge;
    static Contactor* contactor_discharge;

    static uint8_t contactors_timeout_id;

   public:
    static DigitalOutput* led_operational;
    static DigitalOutput* led_fault;
    static DigitalOutput* sdc_obccu;
    static DigitalOutput* imd_bypass;
    static DigitalOutput* led_nucleo;

    static void init();

    static void open_HV();
    static bool is_HV_open();

    static void close_HV();
    static bool is_HV_closed();

    static void start_precharge();
    static bool is_precharging();
};
}  // namespace HVSCU