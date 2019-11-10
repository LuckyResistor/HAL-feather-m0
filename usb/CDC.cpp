//
//  Copyright (c) 2015 Arduino LLC.  All right reserved.
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
#include "CDC.hpp"


#include "Description.hpp"
#include "CDCDescriptor.hpp"
#include "DeviceClass.hpp"

#include "hal-common/Reset.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstdint>


namespace lr::usb {


// For information purpose only since RTS is not always handled by the terminal application
const auto cDcLineStateDtr = 0x01; ///< Data Terminal Ready
const auto cDcLineStateRts = 0x02; ///< Ready to Send


// not used?
const auto CDC_SERIAL_BUFFER_SIZE = 256;
const auto cDcLineStateReady = (cDcLineStateRts | cDcLineStateDtr);
static volatile int32_t breakValue = -1;


volatile CommunicationDeviceClass::LineInfo CommunicationDeviceClass::_lineInfo = {
    115200, // dWDTERate
    0x00,   // bCharFormat
    0x00,   // bParityType
    0x08,   // bDataBits
    0x00    // lineState
};


const CDCDescriptor CommunicationDeviceClass::_cdcInterface = {
    IADDescriptor::create(0, 2, CDC_COMMUNICATION_INTERFACE_CLASS, CDC_ABSTRACT_CONTROL_MODEL, 0),

    // CDC communication interface
    InterfaceDescriptor::create(CDC_ACM_INTERFACE, 1, CDC_COMMUNICATION_INTERFACE_CLASS, CDC_ABSTRACT_CONTROL_MODEL, 0),
    CDCCSInterfaceDescriptor::create(CDC_HEADER, CDC_V1_10 & 0xFF, (CDC_V1_10 >> 8) & 0x0FF), // Header (1.10 bcd)
    ACMFunctionalDescriptor::create(CDC_ABSTRACT_CONTROL_MANAGEMENT, 6), // SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE supported
    CDCCSInterfaceDescriptor::create(CDC_UNION, CDC_ACM_INTERFACE, CDC_DATA_INTERFACE), // Communication interface is master, data interface is slave 0
    CMFunctionalDescriptor::create(CDC_CALL_MANAGEMENT, 1, 1), // Device handles call management (not)
    EndpointDescriptor::create(USBDeviceClass::USB_ENDPOINT_IN(CDC_ENDPOINT_ACM), USBDeviceClass::USB_ENDPOINT_TYPE_INTERRUPT, 0x10, 0x10),

    // CDC data interface
    InterfaceDescriptor::create(CDC_DATA_INTERFACE, 2, CDC_DATA_INTERFACE_CLASS, 0, 0),
    EndpointDescriptor::create(USBDeviceClass::USB_ENDPOINT_OUT(CDC_ENDPOINT_OUT), USBDeviceClass::USB_ENDPOINT_TYPE_BULK, USBDeviceClass::EPX_SIZE, 0),
    EndpointDescriptor::create(USBDeviceClass::USB_ENDPOINT_IN(CDC_ENDPOINT_IN), USBDeviceClass::USB_ENDPOINT_TYPE_BULK, USBDeviceClass::EPX_SIZE, 0)
};


CommunicationDeviceClass::CommunicationDeviceClass(USBDeviceClass *device)
    : _device(device)
{
}


const void* CommunicationDeviceClass::_CDC_GetInterface()
{
    return &_cdcInterface;
}


uint32_t CommunicationDeviceClass::_CDC_GetInterfaceLength()
{
    return sizeof(_cdcInterface);
}


int CommunicationDeviceClass::CDC_GetInterface(uint8_t *interfaceNum)
{
    interfaceNum[0] += 2;    // uses 2
    return _device->sendControl(&_cdcInterface, sizeof(_cdcInterface));
}


bool CommunicationDeviceClass::CDC_Setup(Setup &setup)
{
    const auto requestType = setup.bmRequestType;
    const auto r = setup.bRequest;

    if (requestType == RequestType::DeviceToHostClassInterface) {
        if (r == Request::CdcGetLineCoding) {
            _device->sendControl((void *) &_lineInfo, 7);
            return true;
        }
    }

    if (requestType == RequestType::HostToDeviceClassInterface) {
        if (r == Request::CdcSetLineCoding) {
            _device->recvControl((void *) &_lineInfo, 7);
        }

        if (r == Request::CdcSetControlLineState) {
            _lineInfo.lineState = setup.wValueL;
        }

        if (r == Request::CdcSetLineCoding || r == Request::CdcSetControlLineState) {
            // auto-reset into the bootloader is triggered when the port, already
            // open at 1200 bps, is closed. We check DTR state to determine if host
            // port is open (bit 0 of lineState).
            if (_lineInfo.dwDTERate == 1200 && (_lineInfo.lineState & 0x01) == 0) {
                Reset::eraseDelayed(250_ms);
            } else {
                Reset::cancelErase();
            }
            return false;
        }

        if (r == Request::CdcSendBreak) {
            breakValue = ((uint16_t)setup.wValueH << 8) | setup.wValueL;
            return false;
        }
    }
    return false;
}


}
