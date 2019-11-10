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


/// Interface descriptor
///
struct InterfaceDescriptor
{
    uint8_t len; // 9
    DescriptorType dtype;
    uint8_t number;
    uint8_t alternate;
    uint8_t numEndpoints;
    uint8_t interfaceClass;
    uint8_t interfaceSubClass;
    uint8_t protocol;
    uint8_t iInterface;

    static constexpr InterfaceDescriptor create(
        uint8_t number,
        uint8_t numEndpoints,
        uint8_t interfaceClass,
        uint8_t interfaceSubClass,
        uint8_t protocol)
    {
        return {
            9,
            DescriptorType::Interface,
            number,
            0,
            numEndpoints,
            interfaceClass,
            interfaceSubClass,
            protocol,
            0
        };
    }
} __attribute__((packed));


}

