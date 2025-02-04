#pragma once

#include "ST-LIB.hpp"
namespace HVSCU {
namespace Comms {
namespace Orders {
static constexpr uint16_t CLOSE_CONTACTORS = 900;
static constexpr uint16_t OPEN_CONTACTORS = 901;
};  // namespace Orders

const uint16_t CS_PORT = 50500;
const std::string HVSCU_IP = "192.168.1.9";

extern ServerSocket* control_station;
extern bool open_contactors_order_received;
extern bool close_contactors_order_received;
extern HeapOrder* open_contactors_order;
extern HeapOrder* close_contactors_order;

void start();

};  // namespace Comms
};  // namespace HVSCU