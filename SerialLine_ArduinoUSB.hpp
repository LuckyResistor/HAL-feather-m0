#pragma once
//
// Arduino USB Serial Line
// ---------------------------------------------------------------------------
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


#include "SerialLine.hpp"
#include "Timer.hpp"


namespace lr {


/// This is a wrapper around the Arduino USB serial line implementation.
///
/// This wrapper allows using the `SerialLine` with the Arduino USB
/// serial line, implemented as `SerialUSB`. The serial line is initialized
/// in a non blocking way. Writing and reading from the line is available,
/// as soon the line is ready. Therefore, it is advised to use a buffer
/// around this implementation to bridge the initialization time.
///
class SerialLine_ArduinoUSB : public SerialLine
{
public:
    enum class State : uint8_t {
        Initialized, ///< The USB serial may be initializing.
        Open, ///< The USB serial is up and running.
        NotWorking ///< Seems the USB serial isn't working.
    };
    
public:
    /// Create a new instance of the serial line wrapper.
    ///
    SerialLine_ArduinoUSB();
    
public:
    /// Initialize the serial line with the given speed.
    ///
    /// @param speedBaud The speed of the serial line in baud.
    ///
    Status initialize(uint32_t speedBaud);
    
public: // Implement SerialLine
    DataSize sendBytesAvailable() noexcept override;
    Status send(uint8_t value) noexcept override;
    Status send(const uint8_t *data, DataSize dataSize, DataSize *dataSent = nullptr) noexcept override;
    Status sendReset() noexcept override;
    DataSize receiveBytesAvailable() noexcept override;
    Status receive(uint8_t &value) noexcept override;
    Status receive(uint8_t *data, DataSize dataSize, DataSize *dataReceived = nullptr) noexcept override;
    Status receiveBlock(uint8_t *data, DataSize dataSize, uint8_t blockEndMark, DataSize *dataReceived) noexcept override;
    Status receiveReset() noexcept override;
    
private:
    /// Check if the USB connection is ready.
    ///
    bool isReady();
    
private:
    State _state; ///< The current state of the USB serial.
    Timer::Deadline _initDeadline; ///< The timeout waiting for the serial to initalize.
};


}

