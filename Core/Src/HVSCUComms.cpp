#include "HVSCUComms.hpp"

namespace HVSCUComms {
    void start() {
        control_station = new ServerSocket(IPV4(HVSCU_IP), CS_PORT);
    }
};