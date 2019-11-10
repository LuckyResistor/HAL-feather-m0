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


#include "hal-common/GPIO.hpp"


namespace lr::GPIO {


/// The function for a pin.
///
/// See the multiplexing table in the specification for possible
/// functions for the pins of the chip.
///
enum class Function : uint8_t {
    Disabled = 0xff, ///< Disable multiplexing for the pin.
    A = 0x0, ///< Function A
    B = 0x1, ///< Function B
    C = 0x2, ///< Function C
    D = 0x3, ///< Function D
    E = 0x4, ///< Function E
    F = 0x5, ///< Function F
    G = 0x6, ///< Function G
    H = 0x7, ///< Function H
    I = 0x8, ///< Function I
    Eic = A, ///< Select the pin interrupt function (A)
    Ref = B, ///< Select as ADC reference (B)
    Adc = B, ///< Select as ADC input (B)
    Ain = B, ///< Select as AC input (B)
    Ptc = B, ///< Select as PTC output (B)
    Dac = B, ///< Select as DAC output (B)
    Sercom = C, ///< Select as SERCOM pad (C)
    SercomAlt = D, ///< Select as alternative SERCOM pad (D)
    TccA = E, ///< Select as timer/pulse input/output (E)
    TccB = F, ///< Select as alternative timer/pulse input/output (F)
    Com = G, ///< Select as COM function (G)
    AcCmp = H, ///< Select as AC comparison (H),
    Gclk = H, ///< Select as clock output (H)
};
    
    
/// Set the function for a pin.
///
/// @param pin The pin to set the function.
/// @param function The function to assign to the pin, or `Function::Disable` to
///     disable multiplexing for the pin.
///
void setFunction(PinNumber pin, Function function);

/// Port names to pin number mappings.
///
/// These are the official I/O port names from the specification.
/// Not all of this ports are mapped to a pin, it depends on the chip.
///
enum class Port : PinNumber {
    PA00 = 0x00,
    PA01 = 0x01,
    PA02 = 0x02,
    PA03 = 0x03,
    PA04 = 0x04,
    PA05 = 0x05,
    PA06 = 0x06,
    PA07 = 0x07,
    PA08 = 0x08,
    PA09 = 0x09,
    PA10 = 0x0a,
    PA11 = 0x0b,
    PA12 = 0x0c,
    PA13 = 0x0d,
    PA14 = 0x0e,
    PA15 = 0x0f,
    PA16 = 0x10,
    PA17 = 0x11,
    PA18 = 0x12,
    PA19 = 0x13,
    PA20 = 0x14,
    PA21 = 0x15,
    PA22 = 0x16,
    PA23 = 0x17,
    PA24 = 0x18,
    PA25 = 0x19,
    PA26 = 0x1a,
    PA27 = 0x1b,
    PA28 = 0x1c,
    PA29 = 0x1d,
    PA30 = 0x1e,
    PA31 = 0x1f,
    PB00 = 0x20,
    PB01 = 0x21,
    PB02 = 0x22,
    PB03 = 0x23,
    PB04 = 0x24,
    PB05 = 0x25,
    PB06 = 0x26,
    PB07 = 0x27,
    PB08 = 0x28,
    PB09 = 0x29,
    PB10 = 0x2a,
    PB11 = 0x2b,
    PB12 = 0x2c,
    PB13 = 0x2d,
    PB14 = 0x2e,
    PB15 = 0x2f,
    PB16 = 0x30,
    PB17 = 0x31,
    PB18 = 0x32,
    PB19 = 0x33,
    PB20 = 0x34,
    PB21 = 0x35,
    PB22 = 0x36,
    PB23 = 0x37,
    PB24 = 0x38,
    PB25 = 0x39,
    PB26 = 0x3a,
    PB27 = 0x3b,
    PB28 = 0x3c,
    PB29 = 0x3d,
    PB30 = 0x3e,
    PB31 = 0x3f
};


/// Wrapper to `setMode` to accept ports.
///
inline Status setMode(Port port, Mode mode, Pull pull = Pull::None) {
    return setMode(static_cast<PinNumber>(port), mode, pull);
}

/// Wrapper to `getState` to accept ports.
///
inline bool getState(Port port) {
    return getState(static_cast<PinNumber>(port));
}


}

