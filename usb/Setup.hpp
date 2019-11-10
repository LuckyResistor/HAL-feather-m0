#pragma once
//
// Copyright (c)2014 Arduino LLC.  All right reserved.
// Copyright (c)2019 Lucky Resistor. Conversion into C++ code, clean-up.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//


#include <cstdint>


namespace lr::usb {


/// The request.
///
enum class Request : uint8_t {
    GetStatus = 0x00,
    ClearFeature = 0x01,
    SetFeature = 0x03,
    SetAddress = 0x05,
    GetDescriptor = 0x06,
    SetDescriptor = 0x07,
    GetConfiguration = 0x08,
    SetConfiguration = 0x09,
    GetInterface = 0x0a,
    SetInterface = 0x0b,
    CdcSetLineCoding = 0x20,
    CdcGetLineCoding = 0x21,
    CdcSetControlLineState = 0x22,
    CdcSendBreak = 0x23,
};


/// The combined request type with its masks.
///
enum class RequestType : uint8_t {
    // The direction.
    HostToDevice = 0x00u,
    DeviceToHost = 0x80u,
    DirectionMask = 0x80u,
    // The type.
    Standard = 0x00u,
    Class = 0x20u,
    Vendor = 0x40u,
    TypeMask = 0x60u,
    // The recipient.
    Device = 0x00u,
    Interface = 0x01u,
    Endpoint = 0x02u,
    Other = 0x03u,
    RecipientMask = 0x1Fu,
    // Combined values.
    DeviceToHostClassInterface = (DeviceToHost | Class | Interface),
    HostToDeviceClassInterface = (HostToDevice|Class|Interface),
    DeviceToHostStandardInterface = (DeviceToHost | Standard | Interface),
};


/// Allow and operation on the request type for masking.
///
constexpr RequestType operator&(RequestType a, RequestType b) {
    return static_cast<RequestType>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}


/// A low level setup structure.
///
struct Setup
{
    union {
        RequestType bmRequestType;
        struct {
            uint8_t direction : 5;
            uint8_t type : 2;
            uint8_t transferDirection : 1;
        };
    };
    Request bRequest;
    uint8_t wValueL;
    uint8_t wValueH;
    uint16_t wIndex;
    uint16_t wLength;
};


}



