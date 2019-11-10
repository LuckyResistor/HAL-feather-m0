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
#include "Reset_SAMD21.hpp"


#include "hal-core/Chip.hpp"
#include "hal-core/Segments.h"


namespace lr::Reset {


/// The delay counter for an program erase.
///
volatile uint32_t gEraseDelay = -1; // -1 = disabled.


/// The start of the application.
///
const auto cAppStart = (volatile uint32_t)(&__text_start__) + 4;

/// The start of the NVM.
///
const auto cNvmMemory = ((volatile uint16_t*)0x00000000);

/// Check if the NVM controller is ready.
///
inline bool nvmReady() {
    return NVMCTRL->INTFLAG.reg & NVMCTRL_INTFLAG_READY;
}

/// Erase the current application if a boot loader is present.
///
__attribute__((section(".ramfunc"), noreturn))
void erase() {
    __disable_irq();
    // Only erase the application, if a boot loader is present.
    if (cAppStart >= 0x204) {
        while (!nvmReady()) {}
        // Erase the first block of the application.
        NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
        NVMCTRL->ADDR.reg  = (uintptr_t)&cNvmMemory[cAppStart/4];
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_ER|NVMCTRL_CTRLA_CMDEX_KEY;
        while (!nvmReady()) {}
    }
    // Reset the board.
    NVIC_SystemReset();
    while (true) {};
}


void reset()
{
    // Reset the board.
    NVIC_SystemReset();
    while (true) {};
}


void eraseDelayed(Milliseconds delay)
{
    __disable_irq();
    gEraseDelay = delay.ticks();
    __enable_irq();
}


void cancelErase()
{
    __disable_irq();
    gEraseDelay = -1;
    __enable_irq();
}


void eraseTick()
{
    if (gEraseDelay < 0) {
        return;
    }
    if (--gEraseDelay == 0) {
        erase();
    }
}


}
