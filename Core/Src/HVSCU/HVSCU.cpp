#include "HVSCU/HVSCU.hpp"

HVSCU::HVSCU()
    : low(DigitalOutput(PG14), true),
      high(DigitalOutput(PG12), true),
      precharge(DigitalOutput(PD4), true),
      discharge(DigitalOutput(PF4), false),
      led_operational(PG8),
      led_fault(PG7) {}

void HVSCU::open_contactors() {
    if (timeout_id) Time::cancel_timeout(timeout_id);
    discharge.close();
    low.open();
    high.open();
    precharge.open();
}

void HVSCU::close_contactors() {
    if (timeout_id) Time::cancel_timeout(timeout_id);
    discharge.open();
    low.close();
    precharge.close();
    high.open();
    timeout_id = Time::set_timeout(3000, [this]() {
        precharge.open();
        high.close();
    });
}