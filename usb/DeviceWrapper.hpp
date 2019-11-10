#pragma once
//
// Copyright (c) 2015 Arduino LLC.  All right reserved.
// SAMD51 support added by Adafruit - Copyright (c)2018 Dean Miller for Adafruit Industries
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


#include "hal-core/Chip.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstdint>


namespace lr::usb {


/// A thin wrapper around the hardware USB device.
///
class DeviceWrapper
{
public:
    /// The end point index.
    ///
    using EndPointIndex = uint8_t;

public:
    /// Create a new USB device wrapper.
    ///
    explicit DeviceWrapper(UsbDevice *usbDevice);

public:
    /// Initialize the USB peripheral
    ///
    /// This will enable the clock for the USB peripheral and setup the pin to be used for USB communication.
    ///
    void init();

    // Reset USB Device
    void reset();

    // Enable
    void enable();
    void disable();

    // USB mode (device/host)
    void setUSBDeviceMode();
    void setUSBHostMode();

    void runInStandby();
    void noRunInStandby();
    void wakeupHost();

    // USB speed
    void setFullSpeed();
    void setLowSpeed();
    void setHiSpeed();
    void setHiSpeedTestMode();

    // Authorize attach if Vbus is present
    void attach();
    void detach();

    // USB Interrupts
    bool isEndOfResetInterrupt();
    void ackEndOfResetInterrupt();
    void enableEndOfResetInterrupt();
    void disableEndOfResetInterrupt();

    bool isStartOfFrameInterrupt();
    void ackStartOfFrameInterrupt();
    void enableStartOfFrameInterrupt();
    void disableStartOfFrameInterrupt();

    // USB Address
    void setAddress(uint32_t addr);
    void unsetAddress();

    // Frame number
    uint16_t frameNumber();

    // Load calibration values
    void calibrate();

    // USB Device Endpoints function mapping
    // -------------------------------------

    // Config
    void epBank0SetType(EndPointIndex ep, uint8_t type);
    void epBank1SetType(EndPointIndex ep, uint8_t type);

    // Interrupts
    uint16_t epInterruptSummary();

    bool epBank0IsSetupReceived(EndPointIndex ep);
    bool epBank0IsStalled(EndPointIndex ep);
    bool epBank1IsStalled(EndPointIndex ep);
    bool epBank0IsTransferComplete(EndPointIndex ep);
    bool epBank1IsTransferComplete(EndPointIndex ep);

    void epBank0AckSetupReceived(EndPointIndex ep);
    void epBank0AckStalled(EndPointIndex ep);
    void epBank1AckStalled(EndPointIndex ep);
    void epBank0AckTransferComplete(EndPointIndex ep);
    void epBank1AckTransferComplete(EndPointIndex ep);

    void epBank0EnableSetupReceived(EndPointIndex ep);
    void epBank0EnableStalled(EndPointIndex ep);
    void epBank1EnableStalled(EndPointIndex ep);
    void epBank0EnableTransferComplete(EndPointIndex ep);
    void epBank1EnableTransferComplete(EndPointIndex ep);

    void epBank0DisableSetupReceived(EndPointIndex ep);
    void epBank0DisableStalled(EndPointIndex ep);
    void epBank1DisableStalled(EndPointIndex ep);
    void epBank0DisableTransferComplete(EndPointIndex ep);
    void epBank1DisableTransferComplete(EndPointIndex ep);

    // Status
    bool epBank0IsReady(EndPointIndex ep);
    bool epBank1IsReady(EndPointIndex ep);
    void epBank0SetReady(EndPointIndex ep);
    void epBank1SetReady(EndPointIndex ep);
    void epBank0ResetReady(EndPointIndex ep);
    void epBank1ResetReady(EndPointIndex ep);

    void epBank0SetStallReq(EndPointIndex ep);
    void epBank1SetStallReq(EndPointIndex ep);
    void epBank0ResetStallReq(EndPointIndex ep);
    void epBank1ResetStallReq(EndPointIndex ep);

    // Packet
    uint16_t epBank0ByteCount(EndPointIndex ep);
    uint16_t epBank1ByteCount(EndPointIndex ep);
    void epBank0SetByteCount(EndPointIndex ep, uint16_t bc);
    void epBank1SetByteCount(EndPointIndex ep, uint16_t bc);
    void epBank0SetMultiPacketSize(EndPointIndex ep, uint16_t s);
    void epBank1SetMultiPacketSize(EndPointIndex ep, uint16_t s);

    void epBank0SetAddress(EndPointIndex ep, void *addr);
    void epBank1SetAddress(EndPointIndex ep, void *addr);
    void epBank0SetSize(EndPointIndex ep, uint16_t size);
    void epBank1SetSize(EndPointIndex ep, uint16_t size);

    void epBank0DisableAutoZLP(EndPointIndex ep);
    void epBank1DisableAutoZLP(EndPointIndex ep);
    void epBank0EnableAutoZLP(EndPointIndex ep);
    void epBank1EnableAutoZLP(EndPointIndex ep);

private:
    UsbDevice* const _usb; ///< Alias for the used USB device.
    __attribute__((__aligned__(4)))
    UsbDeviceDescriptor _endPointDescriptor[USB_EPT_NUM]; ///< The end point descriptors table
};


/// Access the global instance of USB device 0.
///
DeviceWrapper& getDeviceWrapper0();


}


