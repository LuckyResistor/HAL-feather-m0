//
// The GPIO abstraction layer
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
#include "GPIO.hpp"


#include <Arduino.h>


namespace lr {
namespace GPIO {


Status initialize()
{
    // Nothing to initialize.
    return Status::Success;
}


Status setMode(PinNumber pin, Mode mode, Pull pull)
{
    switch (mode) {
    case Mode::Input:
    case Mode::HighImpendance:
        switch (pull) {
        case Pull::Up:
            ::pinMode(pin, INPUT_PULLUP);
            break;
        case Pull::Down:
            ::pinMode(pin, INPUT_PULLDOWN);
            break;
        default:
            ::pinMode(pin, INPUT);
            break;
        }
        break;
    case Mode::High:
        ::pinMode(pin, OUTPUT);
        ::digitalWrite(pin, HIGH);
        break;
    case Mode::Low:
        ::pinMode(pin, OUTPUT);
        ::digitalWrite(pin, LOW);
        break;
    default:
        break;
    }
    return Status::Success;
}


bool getState(PinNumber pin)
{
    return ::digitalRead(pin) == HIGH;
}


}
}
