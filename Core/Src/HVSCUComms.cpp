#include "HVSCUComms.hpp"

namespace HVSCU {
namespace Comms {

ServerSocket* control_station = nullptr;
bool open_contactors_order_received = false;
bool close_contactors_order_received = false;

HeapOrder* open_contactors_order = new HeapOrder(
    Orders::OPEN_CONTACTORS, []() { open_contactors_order_received = true; });

HeapOrder* close_contactors_order = new HeapOrder(
    Orders::CLOSE_CONTACTORS, []() { close_contactors_order_received = true; });

void start() { control_station = new ServerSocket(IPV4(HVSCU_IP), CS_PORT); };

};  // namespace Comms
};  // namespace HVSCU