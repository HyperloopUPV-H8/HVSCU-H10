#pragma once

#include "ST-LIB.hpp"

namespace HVSCU {
typedef struct {
    DigitalOutput pin;
    bool normally_opened;
} Contactor;

Contactor low = {DigitalOutput(PG14), true};
Contactor high = {DigitalOutput(PG12), true};
Contactor precharge = {DigitalOutput(PD4), true};
Contactor discharge = {DigitalOutput(PF4), false};

enum ContactorAction { OPEN, CLOSE };

void action_contactor(ContactorAction action, Contactor contactor) {
    if (action == ContactorAction::OPEN) {
        if (contactor.normally_opened)
            contactor.pin.turn_off();
        else {
            contactor.pin.turn_on();
        }
    } else {
        if (contactor.normally_opened)
            contactor.pin.turn_on();
        else {
            contactor.pin.turn_off();
        }
    }
}
};  // namespace HVSCU