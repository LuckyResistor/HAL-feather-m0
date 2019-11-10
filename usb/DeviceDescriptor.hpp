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


namespace lr::usb {


/// The device descriptor
///
struct DeviceDescriptor
{
    uint8_t len; // 18
    DescriptorType dtype;
    uint16_t usbVersion; // 0x200
    uint8_t deviceClass;
    uint8_t deviceSubClass;
    uint8_t deviceProtocol;
    uint8_t packetSize0; // Packet 0
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t deviceVersion; // 0x100
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;

    /// Create a new device descriptor
    ///
    static constexpr DeviceDescriptor create(
        uint8_t deviceClass,
        uint8_t deviceSubClass,
        uint8_t deviceProtocol,
        uint8_t packetSize0,
        uint16_t idVendor,
        uint16_t idProduct,
        uint16_t deviceVersion,
        uint8_t iManufacturer,
        uint8_t iProduct,
        uint8_t iSerialNumber,
        uint8_t bNumConfigurations)
    {
        return {
            18,
            DescriptorType::Device,
            0x200,
            deviceClass,
            deviceSubClass,
            deviceProtocol,
            packetSize0,
            idVendor,
            idProduct,
            deviceVersion,
            iManufacturer,
            iProduct,
            iSerialNumber,
            bNumConfigurations
        };
    }
} __attribute__((packed));


}

