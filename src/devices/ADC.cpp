#include "devices/ADC.hpp"
#include "MAX22530.h"
#include "constants.hpp"
#include "logging/log.hpp"
#include "rtos/mutex.hpp"
#include <SPI.h>

namespace wrvcu {

void ADC::init(uint8_t cspin, SPIClass* theSPI, bool crc_enable) {
    MAX22530 adc_o(cspin, theSPI);
    adc = &adc_o;
    // adc->SPI_CRC(crc_enable); // prob
    assert(crc_enable == crc_enable);
    mutex.init();
    if (!adc->begin(ADC_FREQUENCY)) {
        ERROR("ADC: Could not start ADC.");
    }
}

int ADC::read(int channel) {
    uint16_t adc_ou = 0;
    mutex.take();
    adc_ou = adc->readADC(channel);
    mutex.give();
    return adc_ou;
}

}