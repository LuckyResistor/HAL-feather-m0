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


namespace lr::usb {


#define USB_CONFIG_POWERED_MASK                0x40
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

// bMaxPower in Configuration Descriptor
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)
#ifndef USB_CONFIG_POWER
#define USB_CONFIG_POWER                      (100)
#endif


/// Configuration descriptor
///
struct ConfigDescriptor
{
    uint8_t len; // 9
    uint8_t dtype; // 2
    uint16_t totalLength; // total length
    uint8_t numInterfaces;
    uint8_t config;
    uint8_t iconfig;
    uint8_t attributes;
    uint8_t maxPower;

    static constexpr ConfigDescriptor create(
        uint16_t totalLength,
        uint8_t numInterfaces)
    {
        return {
            9,
            2,
            totalLength,
            numInterfaces,
            1,
            0,
            USB_CONFIG_BUS_POWERED|USB_CONFIG_REMOTE_WAKEUP,
            USB_CONFIG_POWER_MA(USB_CONFIG_POWER)
        };
    }
} __attribute__((packed));


}

