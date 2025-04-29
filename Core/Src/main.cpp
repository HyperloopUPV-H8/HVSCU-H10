#ifndef SIM_ON
#include "main.h"

#include "lwip.h"
#endif

#include "ST-LIB.hpp"

// LEDS
#define PIN_LED_OPERATIONAL PG8
#define PIN_LED_FAULT PG7

// PFM
#define PIN_PWM_P PE9
#define PIN_PWM_N PE8
#define PIN_PWM_BUFFER_EN PB2
#define PIN_OBCPU_RESET PD11

// SENSORS
#define PIN_INPUT_VOLTAGE_OBCPU PB0
#define PIN_INPUT_CURRENT_OBCPU PB1
#define PIN_OUTPUT_VOLTAGE_OBCPU PF11
#define PIN_OUTPUT_CURRENT_OBCPU PF12

#define HVSCU_IP "192.168.1.7"
#define CONTROL_SATION_IP "192.168.0.9"

struct PFM {
    enum class PFM_STATUS : uint8_t { STOPPED, STARTED };
    enum class SWITCHING_STATUS : uint8_t { NOT_SWITCHING, SWITCHING };
    enum class BUFFER_STATUS : uint8_t { ENABLED, DISABLED };
    static DualPWM *pfm;
    static HeapPacket *packet;

    static DigitalOutput *buffer_en;
    static BUFFER_STATUS buffer_status;
    static DigitalOutput *OBCPU_reset;
    static bool reset_status;

    static uint dead_time;
    static uint frequency;
    static PFM_STATUS pfm_status;
    static SWITCHING_STATUS switching_status;

    static void update_data() {
        pfm->set_dead_time(std::chrono::nanoseconds(dead_time));
        pfm->set_frequency(frequency);
    };

    static void toggle_buffer() {
        buffer_en->toggle();
        if (buffer_status == BUFFER_STATUS::DISABLED) {
            buffer_status = BUFFER_STATUS::ENABLED;
        } else {
            buffer_status = BUFFER_STATUS::DISABLED;
        }
        check_pfm();
    }

    static void turn_on() {
        pfm->turn_on();
        pfm_status = PFM_STATUS::STARTED;
        check_pfm();
    }

    static void turn_off() {
        pfm->turn_off();
        pfm_status = PFM_STATUS::STOPPED;
        check_pfm();
    }

    static void toggle_reset() {
        OBCPU_reset->toggle();
        reset_status = !reset_status;
        check_pfm();
    }

    static void init(Pin positive, Pin negative, Pin buffer, Pin reset) {
        pfm = new DualPWM(positive, negative);
        packet = new HeapPacket(600, &buffer_status, &reset_status, &dead_time,
                                &frequency, &pfm_status, &switching_status);
        buffer_en = new DigitalOutput(buffer);
        OBCPU_reset = new DigitalOutput(reset);

        pfm->set_duty_cycle(50.0);
        update_data();

        buffer_en->turn_on();  // Disable buffer (is low_active)
        OBCPU_reset->turn_off();
    }

    static void check_pfm() {
        if (buffer_status == BUFFER_STATUS::ENABLED && reset_status &&
            pfm_status == PFM_STATUS::STARTED) {
            switching_status = SWITCHING_STATUS::SWITCHING;
        } else {
            switching_status = SWITCHING_STATUS::NOT_SWITCHING;
        }
    }
};

DualPWM *PFM::pfm = nullptr;
HeapPacket *PFM::packet = nullptr;
DigitalOutput *PFM::buffer_en = nullptr;
PFM::BUFFER_STATUS PFM::buffer_status = PFM::BUFFER_STATUS::DISABLED;
DigitalOutput *PFM::OBCPU_reset = nullptr;
bool PFM::reset_status = false;
uint PFM::dead_time = 100;
uint PFM::frequency = 200000;
PFM::PFM_STATUS PFM::pfm_status = PFM::PFM_STATUS::STOPPED;
PFM::SWITCHING_STATUS PFM::switching_status =
    PFM::SWITCHING_STATUS::NOT_SWITCHING;

struct OBCPUSensor {
    float reading;
    float raw;
    HeapPacket packet;

    OBCPUSensor(Pin pin, uint16_t id, float slope, float offset)
        : packet(id, &raw, &reading), sensor(pin, slope, offset, &reading) {
        sensor_id = sensor.get_id();
    }

    void read() {
        raw = ADC::get_value(sensor_id);
        sensor.read();
    }

   private:
    LinearSensor<float> sensor;
    uint8_t sensor_id;
};

uint dead_time = 300;  // In ns
uint freq = 30000;     // In Hz

DigitalOutput *led_fault;
DigitalOutput *led_operational;
DigitalOutput *buffer_en;
DigitalOutput *obcpu_reset;

ServerSocket *backend;
DatagramSocket *packets_endpoint;

OBCPUSensor *input_current_sensor;
OBCPUSensor *output_current_sensor;
OBCPUSensor *input_voltage_sensor;
OBCPUSensor *output_voltage_sensor;

int main(void) {
#ifdef SIM_ON
    SharedMemory::start();
#endif

    led_fault = new DigitalOutput(PIN_LED_FAULT);
    led_operational = new DigitalOutput(PIN_LED_OPERATIONAL);
    Time::register_low_precision_alarm(500,
                                       [&]() { led_operational->toggle(); });

    PFM::init(PIN_PWM_P, PIN_PWM_N, PIN_PWM_BUFFER_EN, PIN_OBCPU_RESET);

    input_current_sensor = new OBCPUSensor(PIN_INPUT_CURRENT_OBCPU, 601, 1, 0);
    output_current_sensor =
        new OBCPUSensor(PIN_OUTPUT_CURRENT_OBCPU, 602, 1, 0);
    input_voltage_sensor = new OBCPUSensor(PIN_INPUT_VOLTAGE_OBCPU, 603, 1, 0);
    output_voltage_sensor = new OBCPUSensor(
        PIN_OUTPUT_VOLTAGE_OBCPU, 604, 163.498533508976, -12.0197125310396);

    Time::register_low_precision_alarm(17, [&]() {
        input_current_sensor->read();
        input_voltage_sensor->read();
        output_current_sensor->read();
        output_voltage_sensor->read();
    });

    STLIB::start(HVSCU_IP);

    backend = new ServerSocket(IPV4(HVSCU_IP), 50500);

    // ORDERS
    new HeapOrder(500, PFM::update_data, &PFM::dead_time, &PFM::frequency);
    new HeapOrder(501, PFM::toggle_buffer);
    new HeapOrder(502, PFM::turn_off);
    new HeapOrder(503, PFM::turn_on);
    new HeapOrder(504, PFM::toggle_reset);

    packets_endpoint = new DatagramSocket(IPV4(HVSCU_IP), 50400,
                                          IPV4(CONTROL_SATION_IP), 50400);

    Time::register_low_precision_alarm(100, [&]() {
        packets_endpoint->send_packet(*PFM::packet);
        packets_endpoint->send_packet(input_current_sensor->packet);
        packets_endpoint->send_packet(input_voltage_sensor->packet);
        packets_endpoint->send_packet(output_current_sensor->packet);
        packets_endpoint->send_packet(output_voltage_sensor->packet);
    });

    while (1) {
        STLIB::update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
