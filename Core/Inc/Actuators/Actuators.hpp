#pragma once

#include "Actuators/Contactor.hpp"
#include "HVSCUConfig.hpp"

class Actuators {
   private:
    static Contactor* contactor_low;
    static Contactor* contactor_high;
    static Contactor* contactor_precharge;
    static Contactor* contactor_discharge;

   public:
    static DigitalOutput* led_operational;
    static DigitalOutput* led_fault;
    static DigitalOutput* sdc_obccu;
    static DigitalOutput* imd_bypass;
#if SMART_PRECHARGE
#else
    static uint8_t contactors_timeout_id;
#endif
#ifdef NUCLEO
    static DigitalOutput* led_nucleo;
#endif

    static void start();

    static void open_HV();
    static void close_HV();
    static void start_precharge();
};
