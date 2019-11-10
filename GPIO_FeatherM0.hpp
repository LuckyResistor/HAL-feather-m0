#pragma once
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


#include "GPIO_SAMD21.hpp"


namespace lr {
namespace GPIO {

    
/// The pin names of the feather M0 board.
///
enum class FeatherM0 : PinNumber {
    A0     = static_cast<PinNumber>(Port::PA02),
    A1     = static_cast<PinNumber>(Port::PB08),
    A2     = static_cast<PinNumber>(Port::PB09),
    A3     = static_cast<PinNumber>(Port::PA04),
    A4     = static_cast<PinNumber>(Port::PA05),
    A5     = static_cast<PinNumber>(Port::PB02),
    SCK    = static_cast<PinNumber>(Port::PB11),
    MOSI   = static_cast<PinNumber>(Port::PB10),
    MISO   = static_cast<PinNumber>(Port::PA12),
    RX     = static_cast<PinNumber>(Port::PA11),
    TX     = static_cast<PinNumber>(Port::PA10),
    SDA    = static_cast<PinNumber>(Port::PA22),
    SCL    = static_cast<PinNumber>(Port::PA23),
    P5     = static_cast<PinNumber>(Port::PA15),
    P6     = static_cast<PinNumber>(Port::PA20),
    P9     = static_cast<PinNumber>(Port::PA07),
    P10    = static_cast<PinNumber>(Port::PA18),
    P11    = static_cast<PinNumber>(Port::PA16),
    P12    = static_cast<PinNumber>(Port::PA19),
    P13    = static_cast<PinNumber>(Port::PA17),
    RXLED  = static_cast<PinNumber>(Port::PB03),
    TXLED  = static_cast<PinNumber>(Port::PA27),
    UsbEn  = static_cast<PinNumber>(Port::PA28),
    UsbDm  = static_cast<PinNumber>(Port::PA24),
    UsbDp  = static_cast<PinNumber>(Port::PA25),
};

    
/// Wrapper to `setMode` to accept feather names.
///
inline Status setMode(FeatherM0 port, Mode mode, Pull pull = Pull::None) {
    return setMode(static_cast<PinNumber>(port), mode, pull);
}

/// Wrapper for `setFunction` to accept feather names.
///
inline void setFunction(FeatherM0 port, Function function) {
    setFunction(static_cast<PinNumber>(port), function);
}

/// Wrapper to `getState` to accept feather names.
///
inline bool getState(FeatherM0 port) {
    return getState(static_cast<PinNumber>(port));
}

    
}
}


