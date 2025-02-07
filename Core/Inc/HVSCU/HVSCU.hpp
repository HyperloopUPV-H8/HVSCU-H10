#include "HVSCU/Contactor.hpp"

class HVSCU {
   private:
    Contactor low;
    Contactor high;
    Contactor precharge;
    Contactor discharge;

    uint8_t timeout_id;

   public:
    HVSCU();
    void open_contactors();
    void close_contactors();
};