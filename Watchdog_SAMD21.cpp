//
// (c)2019 by Lucky Resistor. See LICENSE for details.
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
#include "hal-common/Watchdog.hpp"


#include "hal-core/Chip.hpp"


namespace lr {
namespace Watchdog {


void initialize()
{
    // Setup generic clock generator 2 with divisor = 32
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2)|GCLK_GENDIV_DIV(4);
    // Enable the generator sing low-power 32KHz oscillator.
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2)|GCLK_GENCTRL_GENEN|GCLK_GENCTRL_SRC_OSCULP32K|GCLK_GENCTRL_DIVSEL;
    while(GCLK->STATUS.bit.SYNCBUSY);
    // Assign this clock to the watchdog
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT|GCLK_CLKCTRL_CLKEN|GCLK_CLKCTRL_GEN_GCLK2;

    // Disable watchdog for config
    WDT->CTRL.reg = 0;
    while(WDT->STATUS.bit.SYNCBUSY);

    // Disable the early warning interrupt.
    WDT->INTENCLR.bit.EW = 1;
    // Set the period for chip reset.
    // The low power oscillator of the WDT runs at 32kHz with a 1:32 prescale = 1kHz
    // Use setting 0x9 with 4096 clock cycles => ~4s.
    WDT->CONFIG.bit.PER = 0x9;
    // Disable the window mode.
    WDT->CTRL.bit.WEN = 0;
    // Synchronize the registers.
    while(WDT->STATUS.bit.SYNCBUSY);

    // Clear the watchdog before enable it.
    clearWatchdog();

    // Enable the watchdog.
    WDT->CTRL.bit.ENABLE = 1;
    while(WDT->STATUS.bit.SYNCBUSY);
}


void clearWatchdog()
{
    // Clear the watchdog.
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
    while(WDT->STATUS.bit.SYNCBUSY);
}


}
}
