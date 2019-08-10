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
#pragma once

// Workaround for Atmel header include to avoid unnecessary warnings
#undef _U
#undef _L
#undef LITTLE_ENDIAN

// Include the header for the chip you use here.
#include "sam.h"

// Define actual code from the macros to make namespaces work.
namespace chip {
Port* const gPort = PORT;
Sercom* const gSercom0 = SERCOM0;
Sercom* const gSercom1 = SERCOM1;
Sercom* const gSercom2 = SERCOM2;
Sercom* const gSercom3 = SERCOM3;
Sercom* const gSercom4 = SERCOM4;
Sercom* const gSercom5 = SERCOM5;
}

