#include "ST-LIB.hpp"
#include "HVSCU/HVSCUStateMachine.hpp"

class HVSCUOrder {
   private:

    void (*callback)(void);
    HVSCUStateMachine::State state;
    bool received;
    HeapOrder *order;

   public:
    HVSCUOrder(uint16_t id, HVSCUStateMachine::State state, void (*callback)(void));
    void check_order();
};
