#pragma once

#include "Actuators/Contactor.hpp"

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

    static void start();
    static void open_contactors();
    static void close_contactors();
};