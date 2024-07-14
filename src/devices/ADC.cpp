#include "devices/ADC.hpp"
#include "MAX22530.h"
#include "constants.hpp"
#include "logging/log.hpp"
#include "rtos/mutex.hpp"
#include <SPI.h>

namespace wrvcu {

ADC::ADC(uint8_t cspin, SPIClass* theSPI) :
    adc(cspin, theSPI){};

void ADC::init(bool crc_enable) {
    // adc->SPI_CRC(crc_enable); // prob
    // assert(crc_enable == crc_enable);
    mutex.init();
    if (!adc.begin(ADC_FREQUENCY)) {
        ERROR("ADC: Could not start ADC.");
    }
}

int ADC::read(int channel) {
    uint16_t adc_ou = 0;
    mutex.take();
    adc_ou = adc.readFiltered(channel);
    mutex.give();
    return adc_ou;
}

}