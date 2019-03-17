//
// Arduino USB Serial Line
// ---------------------------------------------------------------------------
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
#include "SerialLine_ArduinoUSB.hpp"


#include <Arduino.h>


namespace lr {

    
namespace {
    
    
/// Use a local reference to access the serial line.
/// This allows a simple change of the used variable.
///
auto &serialAccess = Serial;
    

}

    
SerialLine_ArduinoUSB::SerialLine_ArduinoUSB()
    : _state(State::Initialized), _initDeadline(2000_ms)
{
}

    
SerialLine_ArduinoUSB::Status SerialLine_ArduinoUSB::initialize(uint32_t speedBaud)
{
    serialAccess.begin(speedBaud); // The baud rate is ignored.
    serialAccess.setTimeout(100); // Maximum wait 100ms for data.
    _state = State::Initialized;
    _initDeadline.restart(2000_ms); // Maximum wat 2 seconds to initialization.
    return Status::Success;
}

    
SerialLine_ArduinoUSB::DataSize SerialLine_ArduinoUSB::sendBytesAvailable() noexcept
{
    if (!isReady()) {
        return 0;
    }
    return serialAccess.availableForWrite();
}
    
    
SerialLine_ArduinoUSB::Status SerialLine_ArduinoUSB::send(uint8_t value) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }
    auto writtenBytes = serialAccess.write(value);
    return ((writtenBytes == 1) ? Status::Success : Status::Partial);
}
    
    
SerialLine_ArduinoUSB::Status SerialLine_ArduinoUSB::send(const uint8_t *data, DataSize dataSize, DataSize *dataSent) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }
    auto writtenBytes = serialAccess.write(data, dataSize);
    if (dataSent != nullptr) {
        *dataSent = writtenBytes;
    }
    return ((writtenBytes == dataSize) ? Status::Success : Status::Partial);
}
    
    
SerialLine_ArduinoUSB::Status SerialLine_ArduinoUSB::sendReset() noexcept
{
    if (!isReady()) {
        return Status::Error;
    }
    serialAccess.flush(); // Best we ca do.
    return Status::Success;
}
    
    
SerialLine_ArduinoUSB::DataSize SerialLine_ArduinoUSB::receiveBytesAvailable() noexcept
{
    if (!isReady()) {
        return 0;
    }
    return serialAccess.available();
}
    
    
SerialLine_ArduinoUSB::Status SerialLine_ArduinoUSB::receive(uint8_t &value) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }
    const auto received = serialAccess.read();
    if (received >= 0) {
        value = static_cast<uint8_t>(received);
        return Status::Success;
    } else {
        return Status::Partial;
    }
}
    
    
SerialLine_ArduinoUSB::Status SerialLine_ArduinoUSB::receive(uint8_t *data, DataSize dataSize, DataSize *dataReceived) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }
    const auto received = serialAccess.readBytes(reinterpret_cast<char*>(data), dataSize);
    if (dataReceived != nullptr) {
        *dataReceived = received;
    }
    return ((received == dataSize) ? Status::Success : Status::Partial);
}
    
    
SerialLine_ArduinoUSB::Status SerialLine_ArduinoUSB::receiveBlock(uint8_t *data, DataSize dataSize, uint8_t blockEndMark, DataSize *dataReceived) noexcept
{
    if (!isReady()) {
        return Status::Error;
    }
    const auto received = serialAccess.readBytesUntil(blockEndMark, data, dataSize);
    if (dataReceived != nullptr) {
        *dataReceived = received;
    }
    return ((received == dataSize) ? Status::Success : Status::Partial);
}
    
    
SerialLine_ArduinoUSB::Status SerialLine_ArduinoUSB::receiveReset() noexcept
{
    if (!isReady()) {
        return Status::Error;
    }
    serialAccess.flush(); // Best we ca do.
    return Status::Success;
}

                       
bool SerialLine_ArduinoUSB::isReady()
{
    if (_state == State::Open) {
        return true;
    } else if (_state == State::NotWorking) {
        return false;
    } else if (_state == State::Initialized) {
        if (serialAccess) {
            _state = State::Open;
            return true;
        }
        if (_initDeadline.hasTimeout()) {
            // Give up.
            _state = State::NotWorking;
        }
    }
    return false;
}


}

