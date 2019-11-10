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


#include "Setup.hpp"
#include "CDCDescriptor.hpp"


namespace lr::usb {




class USBDeviceClass;


class CommunicationDeviceClass
{
public:
    static constexpr uint16_t CDC_V1_10 = 0x0110;
    static constexpr uint8_t CDC_COMMUNICATION_INTERFACE_CLASS = 0x02;
    static constexpr uint8_t CDC_CALL_MANAGEMENT = 0x01;
    static constexpr uint8_t CDC_ABSTRACT_CONTROL_MODEL = 0x02;
    static constexpr uint8_t CDC_HEADER = 0x00;
    static constexpr uint8_t CDC_ABSTRACT_CONTROL_MANAGEMENT = 0x02;
    static constexpr uint8_t CDC_UNION = 0x06;
    static constexpr uint8_t CDC_CS_INTERFACE = 0x24;
    static constexpr uint8_t CDC_CS_ENDPOINT = 0x25;
    static constexpr uint8_t CDC_DATA_INTERFACE_CLASS = 0x0A;
    static constexpr uint8_t CDC_INTERFACE_COUNT = 2;
    static constexpr uint8_t CDC_ENPOINT_COUNT = 3;
    static constexpr uint8_t CDC_ACM_INTERFACE = 0;  // CDC ACM
    static constexpr uint8_t CDC_DATA_INTERFACE = 1;   // CDC Data
    static constexpr uint8_t CDC_FIRST_ENDPOINT = 1;
    static constexpr uint8_t CDC_ENDPOINT_ACM = 1;
    static constexpr uint8_t CDC_ENDPOINT_OUT = 2;
    static constexpr uint8_t CDC_ENDPOINT_IN = 3;
    static constexpr uint8_t CDC_RX = CDC_ENDPOINT_OUT;
    static constexpr uint8_t CDC_TX = CDC_ENDPOINT_IN;

private:
    struct LineInfo {
        uint32_t dwDTERate;
        uint8_t bCharFormat;
        uint8_t bParityType;
        uint8_t bDataBits;
        uint8_t lineState;
    } __attribute__((packed));

public:
    CommunicationDeviceClass(USBDeviceClass *device);

public:
    int CDC_GetInterface(uint8_t* interfaceNum);
    const void* _CDC_GetInterface(void);
    uint32_t _CDC_GetInterfaceLength(void);
    uint32_t CDC_GetOtherInterface(uint8_t* interfaceNum);
    uint32_t CDC_GetDescriptor(uint32_t i);
    bool CDC_Setup(Setup& setup);

    /// Access the line state from line info.
    ///
    inline uint8_t lineState() const {
        return _lineInfo.lineState;
    }

private:
    USBDeviceClass *_device; ///< The used device.
    static volatile LineInfo _lineInfo; ///< Information about the COM line.
    static const CDCDescriptor _cdcInterface; ///< The interface descriptor.
};


}


