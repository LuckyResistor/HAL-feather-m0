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
#include "InterruptHandler.hpp"


namespace lr::usb {


static InterruptCallback gInterruptCallback0 = nullptr;


void setInterruptCallback(const uint8_t deviceIndex, const InterruptCallback callback)
{
    if (deviceIndex == 0) {
        gInterruptCallback0 = callback;
    }
}


}


/// The USB interrupt handler.
///
void USB_Handler()
{
    if (lr::usb::gInterruptCallback0 != nullptr) {
        lr::usb::gInterruptCallback0();
    }
}

