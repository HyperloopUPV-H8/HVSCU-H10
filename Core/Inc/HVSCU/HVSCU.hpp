#include "HVSCU/Contactor.hpp"
#include "ST-LIB.hpp"

class HVSCU {
   private:
    Contactor low;
    Contactor high;
    Contactor precharge;
    Contactor discharge;

    uint8_t timeout_id;

   public:
    DigitalOutput led_operational;
    DigitalOutput led_fault;
    DigitalOutput sdc_obccu;
    HVSCU();
    void open_contactors();
    void close_contactors();
};