#ifndef SIM_ON
#include "main.h"
#include "lwip.h"
#endif

// #include "Comms/Comms.hpp"
#include "Control/Control.hpp"
#include "ST-LIB.hpp"

int main(void) {
#ifdef SIM_ON
    SharedMemory::start("shm_gpio_HVSCU", "shm_sm_HVSCU");
#endif
    Control control;

    while (1) {
        control.update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
