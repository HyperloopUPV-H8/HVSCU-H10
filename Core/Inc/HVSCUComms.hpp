#pragma once

#include "ST-LIB.hpp"
namespace HVSCU {
namespace Comms {
const uint16_t CS_PORT = 50500;
const std::string HVSCU_IP = "192.168.1.9";

ServerSocket* control_station;
bool open_contactors_order_received, close_contactors_order_received = false;
HeapOrder* open_contactors_order = new HeapOrder(
    Orders::CLOSE_CONTACTORS, []() { open_contactors_order_received = true; });
HeapOrder* close_contactors_order = new HeapOrder(
    Orders::CLOSE_CONTACTORS, []() { close_contactors_order_received = true; });

void start() { control_station = new ServerSocket(IPV4(HVSCU_IP), CS_PORT); };

namespace Orders {
static constexpr uint16_t CLOSE_CONTACTORS = 900;
static constexpr uint16_t OPEN_CONTACTORS = 901;
};  // namespace Orders
};  // namespace Comms
};  // namespace HVSCU