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


/// Interface Association Descriptor
///
/// Used to bind 2 interfaces together in CDC composite device
///
struct IADDescriptor
{
    uint8_t len; // 8
    DescriptorType dtype; // 11
    uint8_t firstInterface;
    uint8_t interfaceCount;
    uint8_t functionClass;
    uint8_t funtionSubClass;
    uint8_t functionProtocol;
    uint8_t iInterface;

    static constexpr IADDescriptor create(
        uint8_t firstInterface,
        uint8_t interfaceCount,
        uint8_t functionClass,
        uint8_t functionSubClass,
        uint8_t functionProtocol)
    {
        return {
            8,
            DescriptorType::InterfaceAssociation,
            firstInterface,
            interfaceCount,
            functionClass,
            functionSubClass,
            functionProtocol,
            0
        };
    }
} __attribute__((packed));


}

