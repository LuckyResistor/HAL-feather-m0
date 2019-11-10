//
// (c)2018 by Lucky Resistor. See LICENSE for details.
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
#include "SerialLine_USB.hpp"


#include "usb/DeviceClass.hpp"
#include "usb/CDC.hpp"

#include "hal-common/Timer.hpp"


namespace lr {

    
namespace {
    

/// Initial delay waiting for a connection.
///
const auto cInitialWait = 2000_ms;

/// Retry time after the the initial delay.
///
const auto cRetryWait = 5000_ms;


}

    
SerialLine_USB::SerialLine_USB(usb::USBDeviceClass *usbDevice)
:
    _usbDevice(usbDevice),
    _state(State::Initialized),
    _connectDeadline(2000_ms)
{
}

    
SerialLine_USB::Status SerialLine_USB::initialize()
{
    // Initialize the USB system as device.
    _usbDevice->init();
    _state = State::Initialized;
    _connectDeadline.restart(cInitialWait); // Maximum wat 3 seconds to initialization.
    return Status::Success;
}

    
SerialLine_USB::DataSize SerialLine_USB::sendBytesAvailable() noexcept
{
    if (!isReady()) {
        return 0;
    }
    return usb::USBDeviceClass::EPX_SIZE - 1;
}
    
    
SerialLine_USB::Status SerialLine_USB::send(uint8_t value) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }

    return send(&value, 1);
}
    
    
SerialLine_USB::Status SerialLine_USB::send(const uint8_t *data, DataSize dataSize, DataSize *dataSent) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }

    DataSize sent = 0;
    if (_usbDevice->communication().lineState() > 0) {
        sent = _usbDevice->send(usb::CommunicationDeviceClass::CDC_ENDPOINT_IN, data, dataSize);
        if (dataSent != nullptr) {
            *dataSent = sent;
        }
    }

    if (sent != dataSize) {
        return Status::Partial;
    } else {
        return Status::Success;
    }
}
    
    
SerialLine_USB::Status SerialLine_USB::sendReset() noexcept
{
    if (!isReady()) {
        return Status::Error;
    }

    // Just flush the buffer?
    //_usbDevice.

    return Status::Success;
}
    
    
SerialLine_USB::DataSize SerialLine_USB::receiveBytesAvailable() noexcept
{
    if (!isReady()) {
        return 0;
    }

    return _usbDevice->available(usb::CommunicationDeviceClass::CDC_ENDPOINT_OUT);
}
    
    
SerialLine_USB::Status SerialLine_USB::receive(uint8_t &value) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }

    auto receivedValue = _usbDevice->recv(usb::CommunicationDeviceClass::CDC_ENDPOINT_OUT);
    if (receivedValue >= 0) {
        value = static_cast<uint8_t>(receivedValue);
        return Status::Success;
    } else {
        return Status::Partial;
    }
}
    
    
SerialLine_USB::Status SerialLine_USB::receive(uint8_t *data, DataSize dataSize, DataSize *dataReceived) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }

    DataSize received = _usbDevice->recv(usb::CommunicationDeviceClass::CDC_ENDPOINT_OUT, data, dataSize);
    if (dataReceived != nullptr) {
        *dataReceived = received;
    }
    if (received != dataSize) {
        return Status::Partial;
    } else {
        return Status::Success;
    }
}
    
    
SerialLine_USB::Status SerialLine_USB::receiveBlock(uint8_t *data, DataSize dataSize, uint8_t blockEndMark, DataSize *dataReceived) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }

    // Slow implementation
    uint8_t dataElement;
    DataSize readCount = 0;
    while (readCount < dataSize) {
        const auto status = receive(dataElement);
        if (status == Status::Error) {
            return status;
        } else if (status == Status::Partial) {
            break;
        }
        *data = dataElement;
        data += 1;
        readCount += 1;
        if (dataElement == blockEndMark) {
            break;
        }
    }
    if (dataReceived != nullptr) {
        *dataReceived = readCount;
    }
    if (readCount != dataSize) {
        return Status::Partial;
    } else {
        return Status::Success;
    }
}
    
    
SerialLine_USB::Status SerialLine_USB::receiveReset() noexcept
{
    if (!isReady()) {
        return Status::Error;
    }
//    serialAccess.flush(); // Best we ca do.
//    return Status::Success;
    return Status::Error;
}

                       
bool SerialLine_USB::isReady()
{
//    if (_state == State::Open) {
//        return true;
//    } else if (_state == State::NoConnection) {
//        if (_connectDeadline.hasTimeout()) {
//            if (serialAccess) {
//                _state = State::Open;
//                return true;
//            }
//            _connectDeadline.restart(cRetryWait);
//        }
//        return false;
//    } else if (_state == State::Initialized) {
//        if (serialAccess) {
//            _state = State::Open;
//            return true;
//        }
//        if (_connectDeadline.hasTimeout()) {
//            // Give up for now.
//            _state = State::NoConnection;
//            _connectDeadline.restart(cRetryWait);
//        }
//    }
    return false;
}


void SerialLine_USB::waitUntilReady()
{
    while (!isReady() && _state == State::Initialized) {
        Timer::delay(100_ms);
    }
}


}

