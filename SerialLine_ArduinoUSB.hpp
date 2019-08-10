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


#include "hal-common/SerialLine.hpp"
#include "hal-common/Timer.hpp"


namespace lr {


/// This is a wrapper around the Arduino USB serial line implementation.
///
/// This wrapper allows using the `SerialLine` with the Arduino USB serial line, implemented as `SerialUSB`. The
/// serial line is initialized in a non blocking way. Writing and reading from the line is available, as soon the line
/// is ready. Therefore, it is advised to use a buffer around this implementation to bridge the initialization time.
///
/// This wrapper does not solve the main problem of the Arduino USB implementation. If the USB cable is removed
/// after a successful connection, the implementation will block forever.
///
class SerialLine_ArduinoUSB : public SerialLine
{
public:
    enum class State : uint8_t {
        Initialized, ///< The USB serial may be initializing.
        Open, ///< The USB serial is up and running.
        NoConnection ///< Seems there is no client connected on the host side.
    };
    
public:
    /// Create a new instance of the serial line wrapper.
    ///
    SerialLine_ArduinoUSB();
    
public:
    /// Initialize the USB serial line.
    ///
    /// Speed is controlled by the host, not by the device.
    ///
    Status initialize();

    /// Check if the USB connection is ready.
    ///
    bool isReady();

    /// Wait until this serial line is ready or not available.
    ///
    /// This will wait, until the USB serial line is ready and open, or if this process runs into a timeout
    /// because there is no USB cable attached. This process can take up to three seconds. A better alternative
    /// is to use a buffer, which will capture any messages at the start and send it to the serial line as soon
    /// it is ready. If the MCU hangs while testing, it may still makes sense to call this method to make sure you
    /// get all messages as fast as possible.
    ///
    void waitUntilReady();

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
    State _state; ///< The current state of the USB serial.
    Timer::Deadline _connectDeadline; ///< The timeout waiting for a USB connection.
};


}

