#pragma once
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


#include "GPIO_SAMD21.hpp"

#include "hal-core/Chip.hpp"
#include "hal-common/WireMaster.hpp"


namespace lr {


/// A rock solid I2C implementation for SAM D21 chips.
///
/// This should be a more reliable replacement for the Wire library.
/// The main difference is the correct error handling and the possibility
/// to reset the I2C interface in case of any problems.
///
/// You must not use the original Wire library if you use this version
/// of the I2C interface.
///
class WireMaster_SAMD21 : public WireMaster
{
public:
    /// The SERCOM interface to use.
    ///
    enum class Interface : uint8_t {
        SerCom0     = 0x00, ///< Use SERCOM0 default pin configuration
        SerCom0Alt  = 0x10, ///< Use SERCOM0 alternative pin configuration
        SerCom1     = 0x01, ///< Use SERCOM1 default pin configuration
        SerCom1Alt  = 0x11, ///< Use SERCOM1 alternative pin configuration
        SerCom2     = 0x02, ///< Use SERCOM2 default pin configuration
        SerCom2Alt  = 0x12, ///< Use SERCOM2 alternative pin configuration
        SerCom3     = 0x03, ///< Use SERCOM3 default pin configuration
        SerCom3Alt  = 0x13, ///< Use SERCOM3 alternative pin configuration
        SerCom4     = 0x04, ///< Use SERCOM4 default pin configuration
        SerCom4Alt  = 0x14, ///< Use SERCOM4 alternative pin configuration
        SerCom5     = 0x05, ///< Use SERCOM5 default pin configuration
        SerCom5Alt  = 0x15, ///< Use SERCOM5 alternative pin configuration
    };

public:
    /// Create a new I2C interface instance.
    ///
    /// Not all configurations of pin numbers make sense. Check the specification
    /// of the SAM D21 chip. The multiplexing matrix shows valid combinations of
    /// communication interfaces and ports.
    ///
    /// Example for the default interface on Adafruit Feather M0:
    /// `WireMaster_FeatherM0 gWire(WireMaster_SAMD21::Interface::Com3, GPIO::FeatherM0::SDA, GPIO::FeatherM0::SCL);`
    ///
    /// @param interface The SERCOM inerface to use.
    /// @param pinSDA The arduino pin number for the used SDA pin.
    /// @param pinSCL The arduino pin number for the used SCL pin.
    ///
    WireMaster_SAMD21(Interface interface, GPIO::PinNumber pinSDA, GPIO::PinNumber pinSCL);

    /// Create a new I2C interface instance.
    ///
    template<typename PinType>
    inline WireMaster_SAMD21(Interface interface, PinType pinSDA, PinType pinSCL)
        : WireMaster_SAMD21(interface, static_cast<GPIO::PinNumber>(pinSDA), static_cast<GPIO::PinNumber>(pinSCL))
    {
    }

public: // Implement the WireMaster interface.
    Status initialize() override;
    Status reset() override;
    Status setSpeed(Speed speed, Nanoseconds riseTime) override;
    Status setSpeed(uint32_t frequencyHz, Nanoseconds riseTime) override;
    Status writeBegin(uint8_t address) override;
    Status writeByte(uint8_t data) override;
    Status writeEndAndStop() override;
    Status writeEndAndStart() override;
    Status writeBytes(uint8_t address, const uint8_t *data, uint8_t count) override;
    Status writeRegisterData(uint8_t address, uint8_t registerAddress, uint8_t data) override;
    Status writeRegisterData(uint8_t address, uint8_t registerAddress, const uint8_t *data, uint8_t count) override;
    Status readBytes(uint8_t address, uint8_t *data, uint8_t count) override;
    Status readRegisterData(uint8_t address, uint8_t registerAddress, uint8_t *data, uint8_t count) override;

private:
    /// Set the baud registers based on the frequency and rise time.
    ///
    void setBaudRegister(uint32_t frequencyHz, Nanoseconds riseTime);

    /// Read bytes after.
    ///
    Status readBytesAfterAcknowledge(uint8_t *data, uint8_t count);

private:
    const Interface _interface; ///< The interface to use.
    Sercom *_sercom; ///< The link to the low level SERCOM interface.
    const GPIO::PinNumber _pinSDA; ///< The arduino pin number for the SDA pin.
    const GPIO::PinNumber _pinSCL; ///< The arduino pin number for the SCL pin.
    uint32_t _frequencyHz; ///< The current speed.
    Nanoseconds _riseTime; ///< The rise time.
};


}



