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


#include "DescriptorType.hpp"

#include <cstdint>


namespace lr::usb {


/// Endpoint descriptor
///
struct EndpointDescriptor
{
    uint8_t len; // 7
    DescriptorType dtype;
    uint8_t addr;
    uint8_t attr;
    uint16_t packetSize;
    uint8_t interval;

    static constexpr EndpointDescriptor create(
        uint8_t addr,
        uint8_t attr,
        uint16_t packetSize,
        uint8_t interval)
    {
        return {
            7,
            DescriptorType::Endpoint,
            addr,
            attr,
            packetSize,
            interval
        };
    }
} __attribute__((packed));


}
