#include "HVSCUConfig.hpp"
#include "ST-LIB.hpp"

#define ZEROING_MEASURE 100000

template <float slope>
class CurrentSensor {
    float voltage_reading;
    float current_reading;
    uint8_t adc_id;

    float offset;

    std::unique_ptr<LinearSensor<float>> sensor;

   public:
    CurrentSensor() : offset(-153.898815693102) {
        sensor = std::make_unique<LinearSensor<float>>(PA0, slope, offset,
                                                       &current_reading);
        adc_id = sensor->get_id();
    }

    void zeroing() {
        float average_voltage = 0.0;
        for (int i = 0; i < ZEROING_MEASURE; ++i) {
            read_current();
            average_voltage += voltage_reading;
        }
        average_voltage /= ZEROING_MEASURE;
        offset = -slope * average_voltage;
        sensor->set_offset(offset);
    }

    void read_current() {
        voltage_reading = ADC::get_value(adc_id);
        sensor->read();
    }

    float* get_voltage() { return &voltage_reading; }

    float* get_current() { return &current_reading; }
};