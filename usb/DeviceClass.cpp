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
#include "DeviceClass.hpp"

#include "PluggableUsb.hpp"
#include "DeviceWrapper.hpp"
#include "Description.hpp"
#include "EndPointHandler.hpp"
#include "DoubleBufferedEndPointHandler.hpp"
#include "DeviceInfo.hpp"
#include "InterruptHandler.hpp"
#include "ConfigDescriptor.hpp"

#include "../ClockCycles.hpp"

#include <limits>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstdlib>


namespace lr::usb {


USBDeviceClass* getDevice0()
{
    static USBDeviceClass __aligned(4) device0(getDeviceWrapper0());
    // Multiple calls of this function do no harm.
    setInterruptCallback(0, []{
        device0.ISRHandler();
    });
    return &device0;
}


const uint16_t USBDeviceClass::STRING_LANGUAGE[2] = {
    (3 << 8) | (2 + 2),
    0x0409    // English
};


// The device descriptors.
const DeviceDescriptor USBDeviceClass::USB_DeviceDescriptorB =
    DeviceDescriptor::create(0xEF, 0x02, 0x01, 64,
                             cDeviceVID, cDevicePID, 0x100, IMANUFACTURER, IPRODUCT, ISERIAL, 1);
const DeviceDescriptor USBDeviceClass::USB_DeviceDescriptor =
    DeviceDescriptor::create(0x00, 0x00, 0x00, 64,
                             cDeviceVID, cDevicePID, 0x100, IMANUFACTURER, IPRODUCT, ISERIAL, 1);


void USBDeviceClass::utox8(uint32_t val, char *s)
{
    for (int i = 0; i < 8; i++) {
        int d = val & 0XF;
        val = (val >> 4);

        s[7 - i] = d > 9 ? 'A' + d - 10 : '0' + d;
    }
}


USBDeviceClass::USBDeviceClass(DeviceWrapper &deviceWrapper)
:
    _deviceWrapper(deviceWrapper),
    _communicationDeviceClass(this),
    initialized(false)
{
    memset(epHandlers, 0, sizeof(epHandlers));
    memset(EndPoints, 0, sizeof(EndPoints));
    EndPoints[0] = USB_ENDPOINT_TYPE_CONTROL;
    EndPoints[1] = USB_ENDPOINT_TYPE_INTERRUPT | USB_ENDPOINT_IN(0); // CDC_ENDPOINT_ACM
    EndPoints[2] = USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_OUT(0); // CDC_ENDPOINT_OUT
    EndPoints[3] = USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_IN(0); // CDC_ENDPOINT_IN
    for (auto &flag : LastTransmitTimedOut) {
        flag = false;
    }
}


// Send a USB descriptor string. The string is stored as a
// plain ASCII string but is sent out as UTF-16 with the
// correct 2-byte prefix
bool USBDeviceClass::sendStringDescriptor(const uint8_t *string, uint8_t maxlen)
{
    if (maxlen < 2)
        return false;

    uint8_t buffer[maxlen];
    buffer[0] = strlen((const char *) string) * 2 + 2;
    buffer[1] = 0x03;

    uint8_t i;
    for (i = 2; i < maxlen && *string; i++) {
        buffer[i++] = *string++;
        if (i == maxlen) break;
        buffer[i] = 0;
    }

    return sendControl(buffer, i);
}


void USBDeviceClass::packMessages(bool val)
{
    if (val) {
        _pack_message = true;
        _pack_size = 0;
    } else {
        _pack_message = false;
        sendControl(_pack_buffer, _pack_size);
    }
}


uint8_t USBDeviceClass::SendInterfaces(uint32_t *total)
{
    uint8_t interfaces = 0;

    total[0] += _communicationDeviceClass.CDC_GetInterface(&interfaces);
    total[0] += PluggableUSB().getInterface(&interfaces);

    return interfaces;
}


// Construct a dynamic configuration descriptor
// This really needs dynamic endpoint allocation etc
uint32_t USBDeviceClass::sendConfiguration(uint32_t maxlen)
{
    uint32_t total = 0;
    // Count and measure interfaces
    _dry_run = true;
    uint8_t interfaces = SendInterfaces(&total);

    ConfigDescriptor config = ConfigDescriptor::create(
        (uint16_t) (total + sizeof(ConfigDescriptor)), interfaces);

    //	Now send them
    _dry_run = false;

    if (maxlen == sizeof(ConfigDescriptor)) {
        sendControl(&config, sizeof(ConfigDescriptor));
        return true;
    }

    total = 0;

    packMessages(true);
    sendControl(&config, sizeof(ConfigDescriptor));
    SendInterfaces(&total);
    packMessages(false);

    return true;
}


bool USBDeviceClass::sendDescriptor(Setup &setup)
{
    const auto descriptorType = static_cast<DescriptorType>(setup.wValueH);
    uint8_t desc_length = 0;
    bool _cdcComposite = false;
    const uint8_t *desc_addr = nullptr;

    if (descriptorType == DescriptorType::Configuration) {
        return sendConfiguration(setup.wLength);
    }

    const auto ret = PluggableUSB().getDescriptor(setup);
    if (ret != 0) {
        return ret > 0;
    }

    if (descriptorType == DescriptorType::Device) {
        if (setup.wLength == 8) {
            _cdcComposite = true;
        }
        desc_addr = _cdcComposite ? (const uint8_t *) &USB_DeviceDescriptorB : (const uint8_t *) &USB_DeviceDescriptor;
        if (*desc_addr > setup.wLength) {
            desc_length = setup.wLength;
        }
    } else if (descriptorType == DescriptorType::String) {
        if (setup.wValueL == 0) {
            desc_addr = (const uint8_t *) &STRING_LANGUAGE;
        } else if (setup.wValueL == IPRODUCT) {
            return sendStringDescriptor(cDeviceProduct, setup.wLength);
        } else if (setup.wValueL == IMANUFACTURER) {
            return sendStringDescriptor(cDeviceManufacturer, setup.wLength);
        } else if (setup.wValueL == ISERIAL) {
            // from section 9.3.3 of the data sheet
            char name[ISERIAL_MAX_LEN];
            utox8(*(volatile uint32_t *) (0x0080A00C), &name[0]);
            utox8(*(volatile uint32_t *) (0x0080A040), &name[8]);
            utox8(*(volatile uint32_t *) (0x0080A044), &name[16]);
            utox8(*(volatile uint32_t *) (0x0080A048), &name[24]);
            name[32] = '\0';
            return sendStringDescriptor((uint8_t *) name, setup.wLength);
        } else {
            return false;
        }
        if (*desc_addr > setup.wLength) {
            desc_length = setup.wLength;
        }
    } else {
    }

    if (desc_addr == nullptr) {
        return false;
    }

    if (desc_length == 0) {
        desc_length = *desc_addr;
    }

    sendControl(desc_addr, desc_length);

    return true;
}


void USBDeviceClass::standby()
{
    _deviceWrapper.noRunInStandby();
}


void USBDeviceClass::handleEndpoint(uint8_t ep)
{
    if (ep == CommunicationDeviceClass::CDC_ENDPOINT_IN) {
        // NAK on endpoint IN, the bank is not yet filled in.
        _deviceWrapper.epBank1ResetReady(CommunicationDeviceClass::CDC_ENDPOINT_IN);
        _deviceWrapper.epBank1AckTransferComplete(CommunicationDeviceClass::CDC_ENDPOINT_IN);
    }
    if (ep == CommunicationDeviceClass::CDC_ENDPOINT_ACM) {
        // NAK on endpoint IN, the bank is not yet filled in.
        _deviceWrapper.epBank1ResetReady(CommunicationDeviceClass::CDC_ENDPOINT_ACM);
        _deviceWrapper.epBank1AckTransferComplete(CommunicationDeviceClass::CDC_ENDPOINT_ACM);
    }
}


void USBDeviceClass::init()
{
    _deviceWrapper.init();
    _deviceWrapper.reset();
    _deviceWrapper.calibrate();
    _deviceWrapper.setUSBDeviceMode();
    _deviceWrapper.runInStandby();
    _deviceWrapper.setFullSpeed();
    _deviceWrapper.enable();
    initialized = true;
}


bool USBDeviceClass::attach()
{
    if (!initialized)
        return false;

    _deviceWrapper.attach();

    _deviceWrapper.enableEndOfResetInterrupt();
    _deviceWrapper.enableStartOfFrameInterrupt();

    _usbConfiguration = 0;
    return true;
}


void USBDeviceClass::setAddress(uint32_t addr)
{
    _deviceWrapper.epBank1SetByteCount(0, 0);
    _deviceWrapper.epBank1AckTransferComplete(0);

    // RAM buffer is full, we can send data (IN)
    _deviceWrapper.epBank1SetReady(0);

    // Wait for transfer to complete
    while (!_deviceWrapper.epBank1IsTransferComplete(0)) {}

    // Set USB address to addr
    _deviceWrapper.setAddress(addr);
}


bool USBDeviceClass::detach()
{
    if (!initialized)
        return false;
    _deviceWrapper.detach();
    return true;
}


bool USBDeviceClass::end()
{
    if (!initialized)
        return false;
    _deviceWrapper.disable();
    return true;
}


bool USBDeviceClass::configured()
{
    return _usbConfiguration != 0;
}


bool USBDeviceClass::handleClassInterfaceSetup(Setup &setup)
{
    uint8_t i = setup.wIndex;

    if (CommunicationDeviceClass::CDC_ACM_INTERFACE == i) {
        if (!_communicationDeviceClass.CDC_Setup(setup)) {
            sendZlp(0);
        }
        return true;
    }

    const bool ret = PluggableUSB().setup(setup);
    if (!ret) {
        sendZlp(0);
    }
    return ret;
}


void USBDeviceClass::initEndpoints()
{
    for (uint8_t i = 1; i < sizeof(EndPoints) && EndPoints[i] != 0; i++) {
        initEP(i, EndPoints[i]);
    }
}


void USBDeviceClass::initEP(uint32_t ep, uint32_t config)
{
    if (config == (USB_ENDPOINT_TYPE_INTERRUPT | USB_ENDPOINT_IN(0))) {
        _deviceWrapper.epBank1SetSize(ep, 64);
        _deviceWrapper.epBank1SetAddress(ep, &udd_ep_in_cache_buffer[ep]);
        _deviceWrapper.epBank1SetType(ep, 4); // INTERRUPT IN
    } else if (config == (USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_OUT(0))) {
        if (epHandlers[ep] != NULL) {
            delete (DoubleBufferedEPOutHandler *) epHandlers[ep];
        }
        epHandlers[ep] = new DoubleBufferedEPOutHandler(_deviceWrapper, ep, 256);
    } else if (config == (USB_ENDPOINT_TYPE_INTERRUPT | USB_ENDPOINT_OUT(0))) {
        if (epHandlers[ep]) {
            epHandlers[ep]->init();
        }
    } else if (config == (USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_IN(0))) {
        _deviceWrapper.epBank1SetSize(ep, 64);
        _deviceWrapper.epBank1SetAddress(ep, &udd_ep_in_cache_buffer[ep]);

        // NAK on endpoint IN, the bank is not yet filled in.
        _deviceWrapper.epBank1ResetReady(ep);

        _deviceWrapper.epBank1SetType(ep, 3); // BULK IN
    } else if (config == USB_ENDPOINT_TYPE_CONTROL) {
        // Setup Control OUT
        _deviceWrapper.epBank0SetSize(ep, 64);
        _deviceWrapper.epBank0SetAddress(ep, &udd_ep_out_cache_buffer[ep]);
        _deviceWrapper.epBank0SetType(ep, 1); // CONTROL OUT / SETUP

        // Setup Control IN
        _deviceWrapper.epBank1SetSize(ep, 64);
        _deviceWrapper.epBank1SetAddress(ep, &udd_ep_in_cache_buffer[0]);
        _deviceWrapper.epBank1SetType(ep, 1); // CONTROL IN

        // Release OUT gEndPointDescriptor
        _deviceWrapper.epBank0SetMultiPacketSize(ep, 64);
        _deviceWrapper.epBank0SetByteCount(ep, 0);

        // NAK on endpoint OUT, the bank is full.
        _deviceWrapper.epBank0SetReady(ep);
    }
}


void USBDeviceClass::setHandler(uint32_t ep, EPHandler *handler)
{
    epHandlers[ep] = handler;
}


void USBDeviceClass::flush(uint32_t ep)
{
    if (available(ep)) {
        // RAM buffer is full, we can send data (IN)
        _deviceWrapper.epBank1SetReady(ep);

        // Clear the transfer complete flag
        _deviceWrapper.epBank1AckTransferComplete(ep);
    }
}


void USBDeviceClass::stall(uint32_t ep)
{
    _deviceWrapper.epBank1SetStallReq(ep);
}


bool USBDeviceClass::connected()
{
    // If the frame number changes, we are connected.
    const auto frameNumber = _deviceWrapper.frameNumber();
    return frameNumber != _deviceWrapper.frameNumber();
}


uint32_t USBDeviceClass::recvControl(void *_data, uint32_t len)
{
    auto data = reinterpret_cast<uint8_t *>(_data);

    // The RAM Buffer is empty: we can receive data
    _deviceWrapper.epBank0ResetReady(0);

    uint32_t read = armRecvCtrlOUT(0);
    if (read > len)
        read = len;
    uint8_t *buffer = udd_ep_out_cache_buffer[0];
    for (uint32_t i = 0; i < len; i++) {
        data[i] = buffer[i];
    }

    return read;
}


// Number of bytes, assumes a rx endpoint
uint32_t USBDeviceClass::available(uint32_t ep)
{
    if (epHandlers[ep]) {
        return epHandlers[ep]->available();
    } else {
        return _deviceWrapper.epBank0ByteCount(ep);
    }
}


// Non Blocking receive
// Return number of bytes read
uint32_t USBDeviceClass::recv(uint32_t ep, void *_data, uint32_t len)
{
    if (!_usbConfiguration)
        return -1;

    if (epHandlers[ep]) {
        return epHandlers[ep]->recv(_data, len);
    }

    if (available(ep) < len)
        len = available(ep);

    armRecv(ep);

    _deviceWrapper.epBank0DisableTransferComplete(ep);

    memcpy(_data, udd_ep_out_cache_buffer[ep], len);

    // release empty buffer
    if (len && !available(ep)) {
        // The RAM Buffer is empty: we can receive data
        _deviceWrapper.epBank0ResetReady(ep);

        // Clear Transfer complete 0 flag
        _deviceWrapper.epBank0AckTransferComplete(ep);

        // Enable Transfer complete 0 interrupt
        _deviceWrapper.epBank0EnableTransferComplete(ep);
    }

    return len;
}


// Recv 1 byte if ready
int USBDeviceClass::recv(uint32_t ep)
{
    uint8_t c;
    if (recv(ep, &c, 1) != 1) {
        return -1;
    } else {
        return c;
    }
}


uint8_t USBDeviceClass::armRecvCtrlOUT(uint32_t ep)
{
    // Get endpoint configuration from setting register
    _deviceWrapper.epBank0SetAddress(ep, &udd_ep_out_cache_buffer[ep]);
    _deviceWrapper.epBank0SetMultiPacketSize(ep, 8);
    _deviceWrapper.epBank0SetByteCount(ep, 0);

    _deviceWrapper.epBank0ResetReady(ep);

    // Wait OUT
    while (!_deviceWrapper.epBank0IsReady(ep)) {}
    while (!_deviceWrapper.epBank0IsTransferComplete(ep)) {}
    return _deviceWrapper.epBank0ByteCount(ep);
}


uint8_t USBDeviceClass::armRecv(uint32_t ep)
{
    uint16_t count = _deviceWrapper.epBank0ByteCount(ep);
    if (count >= 64) {
        _deviceWrapper.epBank0SetByteCount(ep, count - 64);
    } else {
        _deviceWrapper.epBank0SetByteCount(ep, 0);
    }
    return _deviceWrapper.epBank0ByteCount(ep);
}


// Blocking Send of data to an endpoint
uint32_t USBDeviceClass::send(uint32_t ep, const void *data, uint32_t len)
{
    uint32_t written = 0;
    uint32_t length = 0;

    if (!_usbConfiguration)
        return -1;
    if (len > 16384)
        return -1;

    // Flash area
    while (len != 0) {
        if (_deviceWrapper.epBank1IsReady(ep)) {
            // previous transfer is still not complete

            // convert the timeout from microseconds to a number of times through
            // the wait loop; it takes (roughly) 23 clock cycles per iteration.
            uint32_t timeout = lr::ClockCycles::fromMicroseconds(TX_TIMEOUT_MS * 1000) / 23;

            // Wait for (previous) transfer to complete
            // inspired by Paul Stoffregen's work on Teensy
            while (!_deviceWrapper.epBank1IsTransferComplete(ep)) {
                if (LastTransmitTimedOut[ep] || timeout-- == 0) {
                    LastTransmitTimedOut[ep] = 1;

                    // set byte count to zero, so that ZLP is sent
                    // instead of stale data
                    _deviceWrapper.epBank1SetByteCount(ep, 0);
                    return -1;
                }
            }
        }

        LastTransmitTimedOut[ep] = 0;

        if (len > EPX_SIZE) {
            length = EPX_SIZE;
        } else {
            length = len;
        }

        /* memcopy could be safer in multi threaded environment */
        memcpy(&udd_ep_in_cache_buffer[ep], data, length);

        _deviceWrapper.epBank1SetAddress(ep, &udd_ep_in_cache_buffer[ep]);
        _deviceWrapper.epBank1SetByteCount(ep, length);

        // Clear the transfer complete flag
        _deviceWrapper.epBank1AckTransferComplete(ep);

        // RAM buffer is full, we can send data (IN)
        _deviceWrapper.epBank1SetReady(ep);

        written += length;
        len -= length;
        data = (char *) data + length;
    }
    return written;
}


uint32_t USBDeviceClass::armSend(uint32_t ep, const void *data, uint32_t len)
{
    memcpy(&udd_ep_in_cache_buffer[ep], data, len);

    // Get endpoint configuration from setting register
    _deviceWrapper.epBank1SetAddress(ep, &udd_ep_in_cache_buffer[ep]);
    _deviceWrapper.epBank1SetMultiPacketSize(ep, 0);
    _deviceWrapper.epBank1SetByteCount(ep, len);

    return len;
}


uint32_t USBDeviceClass::sendControl(const void *_data, uint32_t len)
{
    auto data = reinterpret_cast<const uint8_t *>(_data);
    uint32_t length = len;
    uint32_t sent = 0;
    uint32_t pos = 0;

    if (_dry_run) {
        return length;
    }

    if (_pack_message) {
        memcpy(&_pack_buffer[_pack_size], data, len);
        _pack_size += len;
        return length;
    }

    while (len > 0) {
        sent = armSend(EP0, data + pos, len);
        pos += sent;
        len -= sent;
    }

    return length;
}


void USBDeviceClass::sendZlp(uint32_t ep)
{
    // Set the byte count as zero
    _deviceWrapper.epBank1SetByteCount(ep, 0);
}


bool USBDeviceClass::handleStandardSetup(Setup &setup)
{
    switch (setup.bRequest) {
    case Request::GetStatus:
        if (setup.bmRequestType == RequestType::Device) {// device
            // Send the device status
            // TODO: Check current configuration for power mode (if device is configured)
            // TODO: Check if remote wake-up is enabled
            uint8_t buff[] = {0, 0};
            armSend(0, buff, 2);
            return true;
        } else {
            // Send the endpoint status.
            // Check if the endpoint if currently halted.
            uint8_t buff[] = {0, 0};
            if (isEndpointHalt == 1)
                buff[0] = 1;
            armSend(0, buff, 2);
            return true;
        }

    case Request::ClearFeature:
        // Check which is the selected feature
        if (setup.wValueL == 1) {
            // Enable remote wake-up and send a ZLP
            uint8_t buff[] = {0, 0};
            if (isRemoteWakeUpEnabled == 1)
                buff[0] = 1;
            armSend(0, buff, 2);
            return true;
        } else {
            isEndpointHalt = 0;
            sendZlp(0);
            return true;
        }

    case Request::SetFeature:
        // Check which is the selected feature
        if (setup.wValueL == 1) {
            // Enable remote wake-up and send a ZLP
            isRemoteWakeUpEnabled = 1;
            uint8_t buff[] = {0};
            armSend(0, buff, 1);
            return true;
        }
        if (setup.wValueL == 0) {
            // Halt endpoint
            isEndpointHalt = 1;
            sendZlp(0);
            return true;
        }

    case Request::SetAddress:
        setAddress(setup.wValueL);
        return true;

    case Request::GetDescriptor:
        return sendDescriptor(setup);

    case Request::SetDescriptor:
        return false;

    case Request::GetConfiguration:
        armSend(0, (void *) &_usbConfiguration, 1);
        return true;

    case Request::SetConfiguration:
        if (RequestType::Device == (setup.bmRequestType & RequestType::RecipientMask)) {
            initEndpoints();
            _usbConfiguration = setup.wValueL;
            // Enable interrupt for CDC reception from host (OUT packet)
            _deviceWrapper.epBank1EnableTransferComplete(CommunicationDeviceClass::CDC_ENDPOINT_ACM);
            _deviceWrapper.epBank0EnableTransferComplete(CommunicationDeviceClass::CDC_ENDPOINT_OUT);
            sendZlp(0);
            return true;
        } else {
            return false;
        }

    case Request::GetInterface:
        armSend(0, (void *) &_usbSetInterface, 1);
        return true;

    case Request::SetInterface:
        _usbSetInterface = setup.wValueL;
        sendZlp(0);
        return true;

    default:
        return true;
    }
}


void USBDeviceClass::ISRHandler()
{
    if (_pack_message) {
        return;
    }
    // End-Of-Reset
    if (_deviceWrapper.isEndOfResetInterrupt()) {
        // Configure gEndPointDescriptor 0
        initEP(0, USB_ENDPOINT_TYPE_CONTROL);
        // Enable Setup-Received interrupt
        _deviceWrapper.epBank0EnableSetupReceived(0);
        _usbConfiguration = 0;
        _deviceWrapper.ackEndOfResetInterrupt();
    }

    // Start-Of-Frame
    if (_deviceWrapper.isStartOfFrameInterrupt()) {
        _deviceWrapper.ackStartOfFrameInterrupt();
    }

    // Endpoint 0 Received Setup interrupt
    if (_deviceWrapper.epBank0IsSetupReceived(0)) {
        _deviceWrapper.epBank0AckSetupReceived(0);
        auto setup = reinterpret_cast<Setup *>(udd_ep_out_cache_buffer[0]);

        // Clear the Bank 0 ready flag on Control OUT
        // The RAM Buffer is empty: we can receive data
        _deviceWrapper.epBank0ResetReady(0);

        bool ok;
        if (RequestType::Standard == (setup->bmRequestType & RequestType::TypeMask)) {
            // Standard Requests
            ok = handleStandardSetup(*setup);
        } else {
            // Class Interface Requests
            ok = handleClassInterfaceSetup(*setup);
        }

        if (ok) {
            _deviceWrapper.epBank1SetReady(0);
        } else {
            stall(0);
        }

        if (_deviceWrapper.epBank1IsStalled(0)) {
            _deviceWrapper.epBank1AckStalled(0);
            // Remove stall request
            _deviceWrapper.epBank1DisableStalled(0);
        }

    } // end Received Setup handler

    uint8_t i = 0;
    uint8_t ept_int = _deviceWrapper.epInterruptSummary() & 0xFE; // Remove endpoint number 0 (setup)
    while (ept_int != 0) {
        // Check if endpoint has a pending interrupt
        if ((ept_int & (1 << i)) != 0) {
            // Endpoint Transfer Complete (0/1) Interrupt
            if (_deviceWrapper.epBank0IsTransferComplete(i) ||
                _deviceWrapper.epBank1IsTransferComplete(i)) {
                if (epHandlers[i]) {
                    epHandlers[i]->handleEndpoint();
                } else {
                    handleEndpoint(i);
                }
            }
            ept_int &= ~(1 << i);
        }
        i++;
        if (i > USB_EPT_NUM) {
            break;  // fire exit
        }
    }
}


CommunicationDeviceClass &USBDeviceClass::communication()
{
    return _communicationDeviceClass;
}


}

