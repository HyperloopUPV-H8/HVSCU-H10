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
    static inline Contactor contactor_low{DigitalOutput(CONTACTOR_LOW), true};
    static inline Contactor contactor_high{DigitalOutput(CONTACTOR_HIGH), true};
    static inline Contactor contactor_precharge{
        DigitalOutput(CONTACTOR_PRECHARGE), true};
    static inline Contactor contactor_discharge{
        DigitalOutput(CONTACTOR_DISCHARGE), true};

    static uint8_t contactors_timeout_id;

   public:
    static inline DigitalOutput led_operational{LED_OPERATIONAL};
    static inline DigitalOutput led_fault{LED_FAULT};
    static inline DigitalOutput sdc_obccu{SDC_OBCCU};
    static inline DigitalOutput imd_bypass{IMD_BYPASS};

    static void open_HV();
    static bool is_HV_open();

    static void close_HV();
    static bool is_HV_closed();

    static void start_precharge();
    static bool is_precharging();
};
}  // namespace HVSCU