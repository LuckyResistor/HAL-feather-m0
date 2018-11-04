//
// Wire Master - SAM D21
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
#include "WireMaster_SAMD21.hpp"


#include "GPIO_SAMD21.hpp"
#include "Timer.hpp"
#include "StatusTools.hpp"


namespace lr {
  
  
namespace {


/// The default baud rate 100kHz
///
const uint32_t cDefaultSpeed100k = 100000;

// Constants not defined in CMSIS.
const uint8_t cBusStateUnknown = 0x0;
const uint8_t cBusStateIdle = 0x1;
const uint8_t cBusStateOwner = 0x2;
const uint8_t cBusStateBusy = 0x3;


/// The acknowledge bit.
///
enum class Acknowledge : uint8_t {
    Yes = 0,
    No = 1
};

/// The command to send.
///
enum class Command : uint8_t {
    RepeatedStart = 0x1,
    ByteRead = 0x2,
    Stop = 0x3,
};


/// Wait for a system operation to finish.
///
/// @param sercom The SERCOM interface to use.
/// @return `Success` or `Timeout`
///
inline WireMaster::Status waitForSystemOperation(Sercom *sercom)
{
    Timer::Deadline sysopTimer(10_ms);
    while (sercom->I2CM.SYNCBUSY.bit.SYSOP != 0) {
        if (sysopTimer.hasTimeout()) {
            return WireMaster::Status::Timeout;
        }
    }
    return WireMaster::Status::Success;
}


/// Wait for a enable/disable to finish.
///
/// @param sercom The SERCOM interface to use.
/// @return `Success` or `Timeout`
///
inline WireMaster::Status waitForSyncEnable(Sercom *sercom)
{
    Timer::Deadline sysopTimer(10_ms);
    while (sercom->I2CM.SYNCBUSY.bit.ENABLE != 0) {
        if (sysopTimer.hasTimeout()) {
            return WireMaster::Status::Timeout;
        }
    }
    return WireMaster::Status::Success;
}


/// Check if the bus is ready
///
/// @param sercom The SERCOM interface to use.
/// @return `Success` or `Timeout`
///
inline bool isBusReady(Sercom *sercom)
{
    const auto busState = sercom->I2CM.STATUS.bit.BUSSTATE;
    return busState == cBusStateIdle || busState == cBusStateOwner;
}

/// Check if there is an error on the bus.
///
/// @param sercom The SERCOM interface to use.
/// @return `true` if there is an error.
///
inline bool hasBusError(Sercom *sercom)
{
    return sercom->I2CM.STATUS.bit.BUSERR;
}

/// Wait until the bus is ready.
///
/// @param sercom The SERCOM interface to use.
/// @return `Success` or `Timeout`
///
inline WireMaster::Status waitUntilReady(Sercom *sercom)
{
    if (hasBusError(sercom)) {
        return WireMaster::Status::Error;
    }
    Timer::Deadline readyTimer(100_ms);
    while (!isBusReady(sercom)) {
        if (readyTimer.hasTimeout()) {
            return WireMaster::Status::Timeout;
        }
    }
    return WireMaster::Status::Success;
}

/// Set the acknowledge bit.
///
/// @param sercom The SERCOM interface to use.
/// @param acknowledge The acknowledge state.
/// @return `Success` or `Timeout`
///
inline WireMaster::Status setAcknowledge(Sercom *sercom, Acknowledge acknowledge)
{
    sercom->I2CM.CTRLB.bit.ACKACT = static_cast<uint8_t>(acknowledge);
    return waitForSystemOperation(sercom);
}

/// Wait until the bus is idle.
///
/// Called after finishing read/write operation to allow the chip to
/// send any pending ACK/NACK and stop condition.
///
/// @param sercom The SERCOM interface to use.
/// @return `Success` or `Timeout`
///
inline WireMaster::Status waitForBusIdle(Sercom *sercom)
{
    Timer::Deadline dt(10_ms);
    while (sercom->I2CM.STATUS.bit.BUSSTATE != cBusStateIdle) {
        if (dt.hasTimeout()) {
            return WireMaster::Status::Timeout;
        }
    }
    return WireMaster::Status::Success;
}

/// Wait for the master on bus flag (MB).
///
/// @param sercom The SERCOM interface to use.
/// @return `Success` or `Timeout`
///
inline WireMaster::Status waitForMasterOnBus(Sercom *sercom)
{
    Timer::Deadline waitTimer(100_ms);
    while (!sercom->I2CM.INTFLAG.bit.MB) {
        if (waitTimer.hasTimeout()) {
            return WireMaster::Status::Timeout;
        }
    }
    return WireMaster::Status::Success;
}


/// Wait for the slave on bus flag (SB).
///
/// @param sercom The SERCOM interface to use.
/// @return `Success` on success,
///         `Timeout` on timeout,
///         `NoAcknowledge` if no acknowledge was received.
///
inline WireMaster::Status waitForSlaveOnBus(Sercom *sercom)
{
    Timer::Deadline waitTimer(100_ms);
    while (!sercom->I2CM.INTFLAG.bit.SB) {
        if (waitTimer.hasTimeout()) {
            return WireMaster::Status::Timeout;
        }
        if (sercom->I2CM.INTFLAG.bit.MB) {
            return WireMaster::Status::NoAcknowledge;
        }
    }
    return WireMaster::Status::Success;
}

/// Send a command.
///
/// @param sercom The SERCOM interface to use.
/// @param command The command to send.
/// @return `Success` or `Timeout`
///
inline WireMaster::Status sendCommand(Sercom *sercom, Command command)
{
    sercom->I2CM.CTRLB.bit.CMD = static_cast<uint8_t>(command);
    return waitForSystemOperation(sercom);
}


}


WireMaster_SAMD21::WireMaster_SAMD21(Interface interface, GPIO::PinNumber pinSDA, GPIO::PinNumber pinSCL)
:
    WireMaster(),
    _interface(interface),
    _pinSDA(pinSDA),
    _pinSCL(pinSCL),
    _speed(cDefaultSpeed100k)
{
    // Assign the correct interface structure.
    switch (interface) {
    default:
    case Interface::SerCom0:
    case Interface::SerCom0Alt:
        _sercom = chip::gSercom0;
        break;
    case Interface::SerCom1:
    case Interface::SerCom1Alt:
        _sercom = chip::gSercom1;
        break;
    case Interface::SerCom2:
    case Interface::SerCom2Alt:
        _sercom = chip::gSercom2;
        break;
    case Interface::SerCom3:
    case Interface::SerCom3Alt:
        _sercom = chip::gSercom3;
        break;
    case Interface::SerCom4:
    case Interface::SerCom4Alt:
        _sercom = chip::gSercom4;
        break;
    case Interface::SerCom5:
    case Interface::SerCom5Alt:
        _sercom = chip::gSercom5;
        break;
    }
}


WireMaster_SAMD21::Status WireMaster_SAMD21::initialize()
{
    // Enable the clock for the SERCOM interface.
    uint8_t clockId;
    switch (_interface) {
    default:
    case Interface::SerCom0:
    case Interface::SerCom0Alt:
        clockId = GCLK_CLKCTRL_ID_SERCOM0_CORE_Val;
        break;
    case Interface::SerCom1:
    case Interface::SerCom1Alt:
        clockId = GCLK_CLKCTRL_ID_SERCOM1_CORE_Val;
        break;
    case Interface::SerCom2:
    case Interface::SerCom2Alt:
        clockId = GCLK_CLKCTRL_ID_SERCOM2_CORE_Val;
        break;
    case Interface::SerCom3:
    case Interface::SerCom3Alt:
        clockId = GCLK_CLKCTRL_ID_SERCOM3_CORE_Val;
        break;
    case Interface::SerCom4:
    case Interface::SerCom4Alt:
        clockId = GCLK_CLKCTRL_ID_SERCOM4_CORE_Val;
        break;
    case Interface::SerCom5:
    case Interface::SerCom5Alt:
        clockId = GCLK_CLKCTRL_ID_SERCOM5_CORE_Val;
        break;
    }
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID(clockId) |
        GCLK_CLKCTRL_GEN_GCLK0 | // Source is clock generator 0
        GCLK_CLKCTRL_CLKEN; // Enable it.
    Timer::Deadline gclkTimer(10_ms);
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {
        if (gclkTimer.hasTimeout()) {
            return Status::Timeout;
        }
    }

    // Reset and configure the interface
    auto status = reset();
    if (status != Status::Success) {
        return status;
    }
    
    // Set the pin peripheral mode for the Arduino environment.
    switch (_interface) {
    default:
    case Interface::SerCom0:
    case Interface::SerCom1:
    case Interface::SerCom2:
    case Interface::SerCom3:
    case Interface::SerCom4:
    case Interface::SerCom5:
        GPIO::setFunction(_pinSDA, GPIO::Function::Sercom);
        GPIO::setFunction(_pinSCL, GPIO::Function::Sercom);
        break;
    case Interface::SerCom0Alt:
    case Interface::SerCom1Alt:
    case Interface::SerCom2Alt:
    case Interface::SerCom3Alt:
    case Interface::SerCom4Alt:
    case Interface::SerCom5Alt:
        GPIO::setFunction(_pinSDA, GPIO::Function::SercomAlt);
        GPIO::setFunction(_pinSCL, GPIO::Function::SercomAlt);
        break;
    }
    return Status::Success;
}


WireMaster_SAMD21::Status WireMaster_SAMD21::reset()
{
    // Start the software reset.
    _sercom->I2CM.CTRLA.bit.SWRST = 1;
    // Wait for the software reset to finish.
    Timer::Deadline opTimer(100_ms);
    while (_sercom->I2CM.CTRLA.bit.SWRST||_sercom->I2CM.SYNCBUSY.bit.SWRST) {
        if (opTimer.hasTimeout()) {
            return Status::Error;
        }
    }

    // Enable master mode and SCL clock stretch mode.
    _sercom->I2CM.CTRLA.reg =
        SERCOM_I2CM_CTRLA_MODE(SERCOM_I2CM_CTRLA_MODE_I2C_MASTER_Val)| // Act as master on the bus.
        SERCOM_I2CM_CTRLA_SDAHOLD(0x2)| // SDA hold time 300-600ns
        SERCOM_I2CM_CTRLA_SEXTTOEN| // Enable time-out for slave.
        SERCOM_I2CM_CTRLA_MEXTTOEN| // Enable time-out for master.
        SERCOM_I2CM_CTRLA_LOWTOUTEN; // Enable time-out if the clock held low.
    // Enable smart mode.
    _sercom->I2CM.CTRLB.bit.SMEN = 1;
    if (hasError(waitForSystemOperation(_sercom))) {
        return Status::Error;
    }
    // Set the baudrate.
    _sercom->I2CM.BAUD.bit.BAUD = SystemCoreClock/(2*cDefaultSpeed100k)-1;
    // Enable the SERCOM interface.
    _sercom->I2CM.CTRLA.bit.ENABLE = 1;
    // Wait for synchronization.
    if (hasError(waitForSyncEnable(_sercom))) {
        return Status::Error;
    }
    // Setting the bus into idle mode.
    _sercom->I2CM.STATUS.bit.BUSSTATE = cBusStateIdle;
    // Wait for the operation.
    if (hasError(waitForSystemOperation(_sercom))) {
        return Status::Error;
    }
    // Disable SB and MB interrupts.
    _sercom->I2CM.INTENCLR.reg = SERCOM_I2CM_INTENCLR_MB|SERCOM_I2CM_INTENCLR_SB;
    // Success
    return Status::Success;
}


WireMaster_SAMD21::Status WireMaster_SAMD21::setSpeed(Speed speed)
{
    uint32_t frequencyHz;
    switch (speed) {
    case Speed::Standard: frequencyHz = 100000; break;
    case Speed::Fast: frequencyHz = 400000; break;
    case Speed::FastPlus: frequencyHz = 1000000; break;
    case Speed::HighSpeed: frequencyHz = 3400000; break;
    default:
        return Status::NotSupported;
    }
    return setSpeed(frequencyHz);
}


WireMaster_SAMD21::Status WireMaster_SAMD21::setSpeed(uint32_t frequencyHz)
{
    // Ignore calls which do not actually change the frequency.
    if (_speed == frequencyHz) {
        return Status::Success;
    }
    // Wait until the bus is idle.
    if (hasError(waitForBusIdle(_sercom))) {
        return Status::Error;
    }
    // Disable the SERCOM interface.
    _sercom->I2CM.CTRLA.bit.ENABLE = 0;
    // Wait until the interface is disabled.
    if (hasError(waitForSyncEnable(_sercom))) {
        return Status::Error;
    }
    // Change the baudrate.
    _sercom->I2CM.BAUD.bit.BAUD = SystemCoreClock/(2*frequencyHz)-1;
    // Change the bus speed bits.
    if (frequencyHz <= 400000) {
        _sercom->I2CM.CTRLA.bit.SPEED = 0;
    } else if (frequencyHz <= 1000000) {
        _sercom->I2CM.CTRLA.bit.SPEED = 1;
    } else {
        _sercom->I2CM.CTRLA.bit.SPEED = 2;
    }
    // Enable the SERCOM interface.
    _sercom->I2CM.CTRLA.bit.ENABLE = 1;
    // Wait for synchronization.
    if (hasError(waitForSyncEnable(_sercom))) {
        return Status::Error;
    }
    // Update the local speed information.
    _speed = frequencyHz;
    // Wait until the bus is idle.
    if (hasError(waitForBusIdle(_sercom))) {
        return Status::Error;
    }
    return Status::Success;
}


WireMaster_SAMD21::Status WireMaster_SAMD21::writeBegin(uint8_t address)
{
    WireMaster::Status status;
    // The data byte is 7bit address + bit 1 = 0 for write.
    const uint8_t addressData = (address<<1)|static_cast<uint8_t>(0x00);
    // Wait for the bus to be ready.
    if (hasError(status = waitUntilReady(_sercom))) return status;
    // Make sure acknowledge is set.
    if (hasError(status = setAcknowledge(_sercom, Acknowledge::Yes))) return status;
    // Send the address and write bit.
    _sercom->I2CM.ADDR.bit.ADDR = addressData;
    if (hasError(status = waitForMasterOnBus(_sercom))) return status;
    // Check if an acknowledge was received.
    if (_sercom->I2CM.STATUS.bit.RXNACK) {
        // Send stop condition, but ignore any timeout.
        sendCommand(_sercom, Command::Stop);
        // Wait until the bus is idle, ignore any timeout.
        waitForBusIdle(_sercom);
        return WireMaster::Status::AddressNotFound;
    }
    return WireMaster::Status::Success;
}


WireMaster_SAMD21::Status WireMaster_SAMD21::writeByte(uint8_t data)
{
    WireMaster::Status status;
    // Prepare the data byte to send.
    _sercom->I2CM.DATA.bit.DATA = data;
    if (hasError(status = waitForMasterOnBus(_sercom))) return status;
    // Check if we received an acknowledge.
    if (_sercom->I2CM.STATUS.bit.RXNACK) {
        // Send stop condition, but ignore any timeout.
        sendCommand(_sercom, Command::Stop);
        // Wait until the bus is idle, ignore any timeout.
        waitForBusIdle(_sercom);
        return Status::NoAcknowledge;
    } else {
        return Status::Success;
    }
}


WireMaster_SAMD21::Status WireMaster_SAMD21::writeEndAndStop()
{
    WireMaster::Status status;
    // Send a stop condition.
    if (hasError(status = sendCommand(_sercom, Command::Stop))) return status;
    // Wait until the bus is idle to make sure everything was sent.
    if (hasError(status = waitForBusIdle(_sercom))) return status;
    return Status::Success;
}


WireMaster_SAMD21::Status WireMaster_SAMD21::writeEndAndStart()
{
    WireMaster::Status status;
    // Send repeated start condition.
    if (hasError(status = sendCommand(_sercom, Command::RepeatedStart))) return status;
    return Status::Success;
}


WireMaster_SAMD21::Status WireMaster_SAMD21::writeBytes(uint8_t address, const uint8_t *data, uint8_t count)
{
    WireMaster::Status status;
    // Check the parameter.
    if (count == 0 || data == nullptr) return Status::Error;
    // Start the write with the address.
    if (hasError(status = writeBegin(address))) return status;
    // Send the data.
    for (uint8_t i = 0; i < count; ++i) {
        if (hasError(status = writeByte(data[i]))) return status;
    }
    // Stop the transaction
    return writeEndAndStop();
}


WireMaster_SAMD21::Status WireMaster_SAMD21::writeRegisterData(uint8_t address, uint8_t registerAddress, uint8_t data)
{
    WireMaster::Status status;
    // Start the write with the address.
    if (hasError(status = writeBegin(address))) return status;
    // Send the register address.
    if (hasError(status = writeByte(registerAddress))) return status;
    // Send the byte.
    if (hasError(status = writeByte(data))) return status;
    // Stop the transaction
    return writeEndAndStop();
}


WireMaster_SAMD21::Status WireMaster_SAMD21::writeRegisterData(uint8_t address, uint8_t registerAddress, const uint8_t *data, uint8_t count)
{
    WireMaster::Status status;
    // Check the parameter.
    if (count == 0 || data == nullptr) return Status::Error;
    // Check if there is an error on the bus.
    if (hasError(status = writeBegin(address))) return status;
    if (hasError(status = writeByte(registerAddress))) return status;
    for (uint8_t i = 0; i < count; ++i) {
        if (hasError(status = writeByte(data[i]))) return status;
    }
    return writeEndAndStop();
}


WireMaster_SAMD21::Status WireMaster_SAMD21::readBytes(uint8_t address, uint8_t *data, uint8_t count)
{
    WireMaster::Status status;
    // Check the parameter.
    if (count == 0 || data == nullptr) return Status::Error;
    // Wait for the bus to be ready.
    if (hasError(status = waitUntilReady(_sercom))) return status;
    // Send the address and wait for acknowledge + bit 1 = 1 for read.
    const uint8_t addressData = (address<<1)|static_cast<uint8_t>(0x01);
    // Make sure acknowledge is set.
    if (hasError(status = setAcknowledge(_sercom, Acknowledge::Yes))) return status;
    // Send the address and write bit.
    _sercom->I2CM.ADDR.bit.ADDR = addressData;
    if (hasError(status = waitForSlaveOnBus(_sercom))) return status;
    // Check if an acknowledge was received.
    if (_sercom->I2CM.STATUS.bit.RXNACK) {
        // No... send a stop.
        sendCommand(_sercom, Command::Stop); // Ignore timeout.
        return WireMaster::Status::AddressNotFound;
    }
    for (uint8_t i = 0; i < count; ++i) {
        // After the first byte, which is read after the address
        // send a read command.
        if (i != 0) {
            // Send the ACK and start a read operation.
            if (hasError(status = sendCommand(_sercom, Command::ByteRead))) return status;
            // Wait for the read operation to finish.
            if (hasError(status = waitForSlaveOnBus(_sercom))) return status;
        }
        // Last byte?
        const bool lastByte = ((i+1)==count);
        if (lastByte) {
            // No acknowledge (NACK) + stop.
            if (hasError(status = setAcknowledge(_sercom, Acknowledge::No))) return status;
            if (hasError(status = sendCommand(_sercom, Command::Stop))) return status;
        } else {
            // Set the acknowledge bit.
            if (hasError(status = setAcknowledge(_sercom, Acknowledge::Yes))) return status;
        }
        // Read the byte from the register and trigger ACK/NACK
        data[i] = _sercom->I2CM.DATA.bit.DATA;
        if (hasError(status = waitForSystemOperation(_sercom))) return status;
    }
    // Wait until the bus is idle to make sure everything was sent.
    if (hasError(status = waitForBusIdle(_sercom))) return status;
    return Status::Success;
}


WireMaster_SAMD21::Status WireMaster_SAMD21::readRegisterData(uint8_t address, uint8_t registerAddress, uint8_t *data, uint8_t count)
{
    WireMaster::Status status;
    // Start writing an address.
    if (hasError(status = writeBegin(address))) return status;
    // Write a byte with the register address.
    if (hasError(status = writeByte(registerAddress))) return status;
    // End the operation and repeat the start condition.
    if (hasError(status = writeEndAndStart())) return status;
    // Read the data from the selected register.
    return readBytes(address, data, count);
}


}

