#pragma once

#include "Actuators/Contactor.hpp"

#define CONTACTOR_LOW PG14
#define CONTACTOR_HIGH PG12
#define CONTACTOR_PRECHARGE PD4
#define CONTACTOR_DISCHARGE PF4
#define LED_OPERATIONAL PG8
#define LED_FAULT PG7
#define IMD_BYPASS PF5
#define SDC_OBCCU PA11

namespace HVSCU {
class Actuators {
   private:
    static Contactor& contactor_low() {
        static Contactor contactor_low{CONTACTOR_LOW, true};
        return contactor_low;
    }
    static Contactor& contactor_high() {
        static Contactor contactor_high{CONTACTOR_HIGH, true};
        return contactor_high;
    }
    static Contactor& contactor_precharge() {
        static Contactor contactor_precharge{CONTACTOR_PRECHARGE, true};
        return contactor_precharge;
    }
    static Contactor& contactor_discharge() {
        static Contactor contactor_discharge{CONTACTOR_DISCHARGE, true};
        return contactor_discharge;
    }

    static uint8_t contactors_timeout_id;

   public:
    static DigitalOutput& led_operational() {
        static DigitalOutput led_operational{LED_OPERATIONAL};
        return led_operational;
    }
    static DigitalOutput& led_fault() {
        static DigitalOutput led_fault{LED_FAULT};
        return led_fault;
    }
    static DigitalOutput& sdc_obccu() {
        static DigitalOutput sdc_obccu{SDC_OBCCU};
        return sdc_obccu;
    }
    static DigitalOutput& imd_bypass() {
        static DigitalOutput imd_bypass{IMD_BYPASS};
        return imd_bypass;
    }

    static void init();

    static void open_HV();
    static bool is_HV_open();

    static void close_HV();
    static bool is_HV_closed();

    static void start_precharge();
    static bool is_precharging();
};
}  // namespace HVSCU