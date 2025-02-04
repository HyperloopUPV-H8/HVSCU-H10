#ifndef SIM_ON
#include "main.h"

#include "lwip.h"
#endif

#include "ST-LIB.hpp"
#include "HVSCUComms.hpp"
#include "StateMachineHVSCU.hpp"
using namespace HVSCU;
int main(void) {
#ifdef SIM_ON
    SharedMemory::start("shm_gpio_HVSCU", "shm_sm_HVSCU");
#endif
    STLIB::start(Comms::HVSCU_IP);
    SM* sm = new SM();

    while (1) {
        STLIB::update();
        sm->update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
