#ifndef SIM_ON
#include "main.h"

#include "lwip.h"
#endif

#include "ST-LIB.hpp"

uint dead_time = 300;  // In ns
uint freq = 30000;     // In Hz

DigitalOutput *led_low_charge;
DigitalOutput *led_full_charge;
DigitalOutput *led_sleep;
DigitalOutput *led_flash;
DigitalOutput *led_can;
DigitalOutput *led_fault;
DigitalOutput *led_operational;
DigitalOutput *buffer_en;
DigitalOutput *obcpu_reset;

DualPWM *test_pwm;
ServerSocket *backend;

int main(void) {
#ifdef SIM_ON
    SharedMemory::start();
#endif

    led_low_charge = new DigitalOutput(PG2);
    led_full_charge = new DigitalOutput(PG3);
    led_sleep = new DigitalOutput(PG4);
    led_flash = new DigitalOutput(PG5);
    led_can = new DigitalOutput(PG6);
    led_fault = new DigitalOutput(PG7);
    led_operational = new DigitalOutput(PG8);
    Time::register_low_precision_alarm(500, [&]() {
        led_low_charge->toggle();
        led_full_charge->toggle();
        led_sleep->toggle();
        led_flash->toggle();
        led_can->toggle();
        led_fault->toggle();
        led_operational->toggle();
    });

    buffer_en = new DigitalOutput(PB2);
    obcpu_reset = new DigitalOutput(PD11);

    test_pwm = new DualPWM(PE9, PE8);
    test_pwm->set_duty_cycle(50.0);

    STLIB::start("192.168.1.7");

    buffer_en->turn_on(); // Disable buffer (is low_active)

    backend = new ServerSocket(IPV4("192.168.1.7"), 50500);
    new HeapOrder(
        500,
        []() {
            test_pwm->set_dead_time(std::chrono::nanoseconds(dead_time));
            test_pwm->set_frequency(freq);
        },
        &dead_time, &freq);
    new HeapOrder(501, []() { buffer_en->toggle(); });
    new HeapOrder(502, []() { test_pwm->turn_off(); });
    new HeapOrder(503, []() { test_pwm->turn_on(); });
    new HeapOrder(504, []() { obcpu_reset->turn_on(); });
    new HeapOrder(505, []() { obcpu_reset->turn_off(); });

    while (1) {
        STLIB::update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
