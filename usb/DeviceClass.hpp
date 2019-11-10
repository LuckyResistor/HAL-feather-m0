#pragma once
//
// Copyright (c) 2015 Arduino LLC.  All right reserved.
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


#include "Setup.hpp"
#include "DeviceWrapper.hpp"
#include "EndPointHandler.hpp"
#include "DeviceDescriptor.hpp"
#include "CDC.hpp"


namespace lr::usb {


/// Interface for a USB device.
///
class USBDeviceClass
{
public:
    static constexpr DeviceWrapper::EndPointIndex EP0 = 0;
    static constexpr uint16_t EPX_SIZE = 64;

    // bEndpointAddress in Endpoint Descriptor
    static constexpr uint8_t USB_ENDPOINT_DIRECTION_MASK = 0x80;
    static inline constexpr uint8_t USB_ENDPOINT_OUT(uint8_t addr) { return addr | 0x00u; }
    static inline constexpr uint8_t USB_ENDPOINT_IN(uint8_t addr) { return addr | 0x80u; }

    static constexpr uint8_t USB_ENDPOINTS = 7;
    static constexpr uint8_t USB_ENDPOINT_TYPE_MASK = 0x03;
    static constexpr uint8_t USB_ENDPOINT_TYPE_CONTROL = 0x00;
    static constexpr uint8_t USB_ENDPOINT_TYPE_ISOCHRONOUS = 0x01;
    static constexpr uint8_t USB_ENDPOINT_TYPE_BULK = 0x02;
    static constexpr uint8_t USB_ENDPOINT_TYPE_INTERRUPT = 0x03;

    static constexpr uint32_t TX_TIMEOUT_MS = 70; ///< Timeout for sends.

public:
    USBDeviceClass(DeviceWrapper &deviceWrapper);

public:
    // USB Device API
    void init();
    bool end();
    bool attach();
    bool detach();
    void setAddress(uint32_t addr);
    bool configured();
    bool connected();
    void standby();

    // Setup API
    bool handleClassInterfaceSetup(Setup &setup);
    bool handleStandardSetup(Setup &setup);
    bool sendDescriptor(Setup &setup);

    // Control EndPoint API
    uint32_t sendControl(const void *data, uint32_t len);
    uint32_t sendControl(int /* ep */, const void *data, uint32_t len)
    { return sendControl(data, len); }
    uint32_t recvControl(void *data, uint32_t len);
    uint32_t sendConfiguration(uint32_t maxlen);
    bool sendStringDescriptor(const uint8_t *string, uint8_t maxlen);
    void initControl(int end);
    uint8_t SendInterfaces(uint32_t *total);
    void packMessages(bool val);

    // Generic EndPoint API
    void initEndpoints();
    void initEP(uint32_t ep, uint32_t type);
    void setHandler(uint32_t ep, EPHandler *handler);
    void handleEndpoint(uint8_t ep);
    uint32_t send(uint32_t ep, const void *data, uint32_t len);
    void sendZlp(uint32_t ep);
    uint32_t recv(uint32_t ep, void *data, uint32_t len);
    int recv(uint32_t ep);
    uint32_t available(uint32_t ep);
    void flush(uint32_t ep);
    void stall(uint32_t ep);

    void ISRHandler();

    /// Access the communication device class.
    ///
    CommunicationDeviceClass& communication();

private:
    uint32_t armSend(uint32_t ep, const void *data, uint32_t len);
    uint8_t armRecv(uint32_t ep);
    uint8_t armRecvCtrlOUT(uint32_t ep);
    static void utox8(uint32_t val, char *s);

private:
    uint8_t udd_ep_out_cache_buffer[USB_ENDPOINTS][64] __aligned(4); ///< Endpoint output buffer.
    uint8_t udd_ep_in_cache_buffer[USB_ENDPOINTS][64] __aligned(4); ///< Endpoint input buffer.
    EPHandler *epHandlers[USB_ENDPOINTS]; ///< An array with all endpoint handler.
    uint32_t EndPoints[USB_ENDPOINTS]; ///< The endpoint addresses.
    bool LastTransmitTimedOut[USB_ENDPOINTS]; ///< Flag is the last transmit timed out.

    DeviceWrapper &_deviceWrapper; ///< The used device wrapper.

    CommunicationDeviceClass _communicationDeviceClass; ///< The communication device class.

    bool initialized;

    bool _dry_run = false;
    bool _pack_message = false;
    uint16_t _pack_size = 0;
    uint8_t _pack_buffer[256];
    volatile uint32_t _usbConfiguration = 0;
    volatile uint32_t _usbSetInterface = 0;
    char isRemoteWakeUpEnabled = 0;
    char isEndpointHalt = 0;

    static const uint16_t STRING_LANGUAGE[2];
    static const DeviceDescriptor USB_DeviceDescriptorB;
    static const DeviceDescriptor USB_DeviceDescriptor;
};


/// Get USB device 0.
///
/// Please keep a copy of the returned pointer, it stays valid.
///
USBDeviceClass* getDevice0();


}

