#ifndef SIM_ON
#include "main.h"

#include "lwip.h"
#endif

#include "HVSCU/Comms.hpp"
#include "HVSCU/HVSCUStateMachine.hpp"
#include "ST-LIB.hpp"

int main(void) {
#ifdef SIM_ON
    SharedMemory::start("shm_gpio_HVSCU", "shm_sm_HVSCU");
#endif
    DigitalOutput imd_bypass(PF5);
    DigitalOutput obccu_relay(PA11);
    HVSCUStateMachine state_machine = HVSCUStateMachine();
    STLIB::start(Comms::HVSCU_IP);
    Comms::start();

    imd_bypass.turn_on();
    obccu_relay.turn_off();
    while (1) {
        STLIB::update();
        state_machine.update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
