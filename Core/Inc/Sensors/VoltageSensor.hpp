#ifndef VOLTAGE_SENSOR_HPP
#define VOLTAGE_SENSOR_HPP

#include "HVSCUConfig.hpp"
#include "HVSCUPinout.hpp"
#include "ST-LIB.hpp"

#define ZEROING_MEASURE 100000

class VoltageSensor {
#if SENSOR_CHARACTERIZATION
    static constexpr float slope = 1.0;
#else
    static constexpr float slope = 1.0;
#endif

    float raw_reading;
    float voltage_reading;
    uint8_t adc_id;

    float offset;

    std::unique_ptr<LinearSensor<float>> sensor;

   public:
    VoltageSensor() : offset(0) {
        sensor = std::make_unique<LinearSensor<float>>(
            BATTERY_VOLTAGE_SENSOR, slope, offset, &voltage_reading);
        adc_id = sensor->get_id();
    }

    void zeroing() {
        float average_voltage = 0.0;
        for (int i = 0; i < ZEROING_MEASURE; ++i) {
            read_voltage();
            average_voltage += raw_reading;
        }
        average_voltage /= ZEROING_MEASURE;
        offset = -slope * average_voltage;
        sensor->set_offset(offset);
    }

    void read_voltage() {
        raw_reading = ADC::get_value(adc_id);
        sensor->read();
    }

    float* get_raw() { return &raw_reading; }

    float* get_voltage() { return &voltage_reading; }
};

#endif