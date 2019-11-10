#pragma once
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


#include <cstdint>


namespace lr::ClockCycles {


/// The system core clock for the platform.
///
constexpr uint32_t cSystemCoreClock = 48'000'000ul;


/// Get the number of clocks per microsecond.
///
constexpr uint32_t getPerMicrosecond() {
    return (cSystemCoreClock / 1'000'000lu);
}


/// Get the number of microseconds for a given number of clock cycles.
///
constexpr uint32_t toMicroseconds(uint32_t clockCycles) {
    return ((clockCycles * 1000ul) / (cSystemCoreClock / 1000ul));
}


/// Get the number of clock cycles for the given number of microseconds.
///
constexpr uint32_t fromMicroseconds(uint32_t microSeconds) {
    return (microSeconds * (cSystemCoreClock / 1000000ul));
}


}

