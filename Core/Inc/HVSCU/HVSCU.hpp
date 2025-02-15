#include "HVSCU/Contactor.hpp"
#include "ST-LIB.hpp"

class HVSCU {
   private:
    static Contactor* low;
    static Contactor* high;
    static Contactor* precharge;
    static Contactor* discharge;

    static uint8_t contactors_timeout_id;

   public:
    static DigitalOutput* led_operational;
    static DigitalOutput* led_fault;
    static DigitalOutput* sdc_obccu;

    void start();
    static void open_contactors();
    static void close_contactors();
};