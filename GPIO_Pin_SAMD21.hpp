#pragma once
//
// The GPIO abstraction layer for SAM D21
// ---------------------------------------------------------------------------
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


#include "Chip.hpp"

#include <hal-common/GPIO.hpp>


namespace lr {
namespace GPIO {


/// The base class for static pin access.
///
/// Do not use this class directly, but the predefined names `PinXXX`. This is a pure static
/// implementation for access the GPIO functions in the fastest possible way. For a dynamic access,
/// or if you need special functionality, use the `setMode` etc. methods.
///
/// Using this classes will result in non portable code which may have to be adapted for a new
/// platform.
///
/// @tparam index The index of the used pin.
///
template<uint8_t pinIndex>
class PinBase
{
private:
    /// Access the port for this pin.
    ///
    constexpr static PortGroup& getPort() {
        return chip::gPort->Group[pinIndex >> 5u];
    }

    /// The pin index in the selected port.
    ///
    constexpr static const uint8_t _index = static_cast<uint8_t>(pinIndex & 0b11111u);

    /// The pin mask in the selected port.
    ///
    constexpr static const uint32_t _mask = static_cast<uint32_t>(1) << _index;

public:
    /// Configure this pin as input.
    ///
    /// @param pull If the port shall be pulled up or down.
    ///
    inline static void configureAsInput(Pull pull = Pull::None) {
        auto &port = getPort();
        port.DIRCLR.reg = _mask;
        switch (pull) {
            case Pull::Up:
                port.PINCFG[_index].reg |= (PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
                port.OUTSET.reg = _mask;
                break;
            case Pull::Down:
                port.PINCFG[_index].reg |= (PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
                port.OUTCLR.reg = _mask;
                break;
            default:
                port.PINCFG[_index].bit.INEN = 1;
                port.PINCFG[_index].bit.PULLEN = 0;
                break;
        }
    }

    /// Configure this pin as output.
    ///
    inline static void configureAsOutput() {
        auto &port = getPort();
        port.DIRSET.reg = _mask;
        port.PINCFG[_index].reg &= ~(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
    }

    /// Configure as high-impendance.
    ///
    inline static void configureAsHighImpendance() {
        auto &port = getPort();
        port.OUTCLR.reg = _mask;
        port.DIRCLR.reg = _mask;
        port.PINCFG[_index].reg &= ~(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
    }

    /// Set the output to low.
    ///
    inline static void setOutputLow() {
        auto &port = getPort();
        port.OUTCLR.reg = _mask;
    }

    /// Set the output to high.
    ///
    inline static void setOutputHigh() {
        auto &port = getPort();
        port.OUTSET.reg = _mask;
    }

    /// Toggle the output.
    ///
    inline static void toggleOutput() {
        auto &port = getPort();
        port.OUTTGL.reg = _mask;
    }

    /// Read the input.
    ///
    inline static bool getInput() {
        auto &port = getPort();
        return (port.IN.reg & _mask) != 0;
    }
};


using PinPA00 = PinBase<0x00>;
using PinPA01 = PinBase<0x01>;
using PinPA02 = PinBase<0x02>;
using PinPA03 = PinBase<0x03>;
using PinPA04 = PinBase<0x04>;
using PinPA05 = PinBase<0x05>;
using PinPA06 = PinBase<0x06>;
using PinPA07 = PinBase<0x07>;
using PinPA08 = PinBase<0x08>;
using PinPA09 = PinBase<0x09>;
using PinPA10 = PinBase<0x0a>;
using PinPA11 = PinBase<0x0b>;
using PinPA12 = PinBase<0x0c>;
using PinPA13 = PinBase<0x0d>;
using PinPA14 = PinBase<0x0e>;
using PinPA15 = PinBase<0x0f>;
using PinPA16 = PinBase<0x10>;
using PinPA17 = PinBase<0x11>;
using PinPA18 = PinBase<0x12>;
using PinPA19 = PinBase<0x13>;
using PinPA20 = PinBase<0x14>;
using PinPA21 = PinBase<0x15>;
using PinPA22 = PinBase<0x16>;
using PinPA23 = PinBase<0x17>;
using PinPA24 = PinBase<0x18>;
using PinPA25 = PinBase<0x19>;
using PinPA26 = PinBase<0x1a>;
using PinPA27 = PinBase<0x1b>;
using PinPA28 = PinBase<0x1c>;
using PinPA29 = PinBase<0x1d>;
using PinPA30 = PinBase<0x1e>;
using PinPA31 = PinBase<0x1f>;

using PinPB00 = PinBase<0x20>;
using PinPB01 = PinBase<0x21>;
using PinPB02 = PinBase<0x22>;
using PinPB03 = PinBase<0x23>;
using PinPB04 = PinBase<0x24>;
using PinPB05 = PinBase<0x25>;
using PinPB06 = PinBase<0x26>;
using PinPB07 = PinBase<0x27>;
using PinPB08 = PinBase<0x28>;
using PinPB09 = PinBase<0x29>;
using PinPB10 = PinBase<0x2a>;
using PinPB11 = PinBase<0x2b>;
using PinPB12 = PinBase<0x2c>;
using PinPB13 = PinBase<0x2d>;
using PinPB14 = PinBase<0x2e>;
using PinPB15 = PinBase<0x2f>;
using PinPB16 = PinBase<0x30>;
using PinPB17 = PinBase<0x31>;
using PinPB18 = PinBase<0x32>;
using PinPB19 = PinBase<0x33>;
using PinPB20 = PinBase<0x34>;
using PinPB21 = PinBase<0x35>;
using PinPB22 = PinBase<0x36>;
using PinPB23 = PinBase<0x37>;
using PinPB24 = PinBase<0x38>;
using PinPB25 = PinBase<0x39>;
using PinPB26 = PinBase<0x3a>;
using PinPB27 = PinBase<0x3b>;
using PinPB28 = PinBase<0x3c>;
using PinPB29 = PinBase<0x3d>;
using PinPB30 = PinBase<0x3e>;
using PinPB31 = PinBase<0x3f>;


}
}

