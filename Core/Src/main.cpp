#ifndef SIM_ON
#include "main.h"

#include "lwip.h"
#endif

#include "ST-LIB.hpp"
#include "HVSCUComms.hpp"
#include "StateMachineHVSCU.hpp"

int main(void) {
#ifdef SIM_ON
    SharedMemory::start("shm_gpio_HVSCU", "shm_sm_HVSCU");
#endif
    STLIB::start(HVSCUComms::HVSCU_IP);
    StateMachineHVSCU* sm = new StateMachineHVSCU();

    while (1) {
        STLIB::update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
