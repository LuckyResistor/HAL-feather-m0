#pragma once
//
// The GPIO abstraction layer / static pins for Feather M0
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


#include "GPIO_Pin_SAMD21.hpp"


namespace lr {
namespace GPIO {


using PinA0     = PinPA02;
using PinA1     = PinPB08;
using PinA2     = PinPB09;
using PinA3     = PinPA04;
using PinA4     = PinPA05;
using PinA5     = PinPB02;
using PinSCK    = PinPB11;
using PinMOSI   = PinPB10;
using PinMISO   = PinPA12;
using PinRX     = PinPA11;
using PinTX     = PinPA10;
using PinSDA    = PinPA22;
using PinSCL    = PinPA23;
using Pin5      = PinPA15;
using Pin6      = PinPA20;
using Pin9      = PinPA07;
using Pin10     = PinPA18;
using Pin11     = PinPA16;
using Pin12     = PinPA19;
using Pin13     = PinPA17;


}
}

