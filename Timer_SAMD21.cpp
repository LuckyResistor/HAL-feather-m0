//
// (c)2018 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "hal-common/Timer.hpp"


#include "Reset_SAMD21.hpp"

#include "hal-core/Chip.hpp"


namespace lr {
namespace Timer {


volatile uint32_t gTickCounter = 0;


Milliseconds tickMilliseconds()
{
    return Milliseconds(gTickCounter);
}


void waitForNextTick()
{
    const auto currentValue = tickMilliseconds();
    while (currentValue == tickMilliseconds()) {}
}


void delayMilliseconds(uint32_t milliseconds)
{
    while (milliseconds > 0) {
        waitForNextTick();
        --milliseconds;
    }
}


void delayMicroseconds(const uint32_t microseconds)
{
//    ::delayMicroseconds(microseconds);
// FIXME! Implement...
}


}
}


void SysTick_Handler(void)
{
    ++lr::Timer::gTickCounter;
    lr::Reset::eraseTick();
}
