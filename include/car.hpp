#pragma once

#include "TractiveSystem.hpp"
#include "devices/ADC.hpp"
#include "devices/battery.hpp"
#include "devices/inverter.hpp"
#include "devices/throttleManager.hpp"

namespace wrvcu {

extern TractiveSystem ts;
extern ThrottleManager throttle;
extern Inverter inverter;
extern Battery battery;
extern ADC adc;
}