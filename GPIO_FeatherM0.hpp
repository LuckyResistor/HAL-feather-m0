#pragma once
//
// The GPIO abstraction layer / FeatherM0 additions
// ---------------------------------------------------------------------------
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
    A0 = Port::PA02,
    A1 = Port::PB08,
    A2 = Port::PB09,
    A3 = Port::PA04,
    A4 = Port::PA05,
    A5 = Port::PB02,
    SCK = Port::PB11,
    MOSI = Port::PB10,
    MISO = Port::PA12,
    RX = Port::PA11,
    TX = Port::PA10,
    SDA = Port::PA22,
    SCL = Port::PA23,
    P5 = Port::PA15,
    P6 = Port::PA20,
    P9 = Port::PA07,
    P10 = Port::PA18,
    P11 = Port::PA16,
    P12 = Port::PA19,
    P13 = Port::PA17
};

    
/// Wrapper to `setMode` to accept feather names.
///
inline Status setMode(FeatherM0 port, Mode mode, Pull pull = Pull::None) {
    return setMode(static_cast<PinNumber>(port), mode, pull);
}

/// Wrapper to `getState` to accept feather names.
///
inline bool getState(FeatherM0 port) {
    return getState(static_cast<PinNumber>(port));
}

    
}
}


