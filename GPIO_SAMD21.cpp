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


#include "hal-core/Chip.hpp"


namespace lr::GPIO {


Status initialize()
{
    // Nothing to initialize.
    return Status::Success;
}


Status setMode(PinNumber pin, Mode mode, Pull pull)
{
    auto &port = chip::gPort->Group[static_cast<uint16_t>(pin)>>5];
    const auto pinPortIndex = static_cast<uint8_t>(pin&0b11111);
    const auto setMask = static_cast<uint32_t>(1)<<(pinPortIndex);
    switch (mode) {
    case Mode::Input:
        port.DIRCLR.reg = setMask;
        switch (pull) {
        case Pull::Up:
            port.PINCFG[pinPortIndex].reg |= (PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
            port.OUTSET.reg = setMask;
            break;
        case Pull::Down:
            port.PINCFG[pinPortIndex].reg |= (PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
            port.OUTCLR.reg = setMask;
            break;
        default:
            port.PINCFG[pinPortIndex].bit.INEN = 1;
            port.PINCFG[pinPortIndex].bit.PULLEN = 0;
            break;
        }
        break;
    case Mode::HighImpendance:
        port.DIRCLR.reg = setMask;
        port.OUTCLR.reg = setMask;
        port.PINCFG[pinPortIndex].reg &= ~(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
        break;
    case Mode::High:
        port.DIRSET.reg = setMask;
        port.OUTSET.reg = setMask;
        port.PINCFG[pinPortIndex].reg &= ~(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
        break;
    case Mode::Low:
        port.DIRSET.reg = setMask;
        port.OUTCLR.reg = setMask;
        port.PINCFG[pinPortIndex].reg &= ~(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);
        break;
    default:
        break;
    }
    return Status::Success;
}


bool getState(PinNumber pin)
{
    auto &port = chip::gPort->Group[static_cast<uint16_t>(pin)>>5];
    const auto pinPortIndex = static_cast<uint8_t>(pin&0b11111);
    const auto mask = static_cast<uint32_t>(1)<<(pinPortIndex);
    return (port.IN.reg&mask) != 0;
}

    
void setFunction(PinNumber pin, Function function)
{
    auto &port = chip::gPort->Group[static_cast<uint16_t>(pin)>>5];
    const auto pinPortIndex = static_cast<uint8_t>(pin&0b11111);
    if (function == Function::Disabled) {
        port.PINCFG[pinPortIndex].bit.PMUXEN = 0;
    } else {
        if ((pinPortIndex&1) == 0) {
            port.PMUX[pin>>1].reg &= 0xf0;
            port.PMUX[pin>>1].reg |= static_cast<uint8_t>(function);
        } else {
            port.PMUX[pin>>1].reg &= 0x0f;
            port.PMUX[pin>>1].reg |= (static_cast<uint8_t>(function)<<4);
        }
        port.PINCFG[pinPortIndex].bit.PMUXEN = 1;
    }
}


}

