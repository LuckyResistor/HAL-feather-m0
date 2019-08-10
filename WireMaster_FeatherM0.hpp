#pragma once
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


#include "GPIO_FeatherM0.hpp"
#include "WireMaster_SAMD21.hpp"


namespace lr {
    
    
/// A rock solid I2C implementation for SAM D21 chips.
///
/// This provides all valid configurations for the Adafruit Feather M0 platform.
///
/// @note Each SERCOM can only be used once!
///
class WireMaster_FeatherM0 : public WireMaster_SAMD21
{
public:
    /// The setup for the bus.
    ///
    enum class Setup {
        A1_A2, ///< A1 = SDA, A2 = SCL, SERCOM0.
        A3_A4, ///< A3 = SDA, A4 = SCL, SERCOM4.
        P11_P13_1, ///< 11 = SDA, 13 = SCL, SERCOM1.
        P11_P13_3, ///< 11 = SDA, 13 = SCL, SERCOM3.
        SDA_SCL_3, ///< SDA = SDA, SCL = SCL, SERCOM3.
        SDA_SCL_5, ///< SDA = SDA, SCL = SCL, SERCOM5.
        Default = SDA_SCL_3, ///< The default bus.
    };
    
private:
    static inline Interface getInterfaceForSetup(const Setup setup)
    {
        switch (setup) {
        case Setup::A1_A2: return Interface::SerCom4Alt;
        case Setup::A3_A4: return Interface::SerCom0Alt;
        case Setup::P11_P13_1: return Interface::SerCom1;
        case Setup::P11_P13_3: return Interface::SerCom3Alt;
        case Setup::SDA_SCL_5: return Interface::SerCom5Alt;
        default:
            break;
        }
        return Interface::SerCom3;
    }

    static inline GPIO::PinNumber getSdaPinForSetup(const Setup setup)
    {
        switch (setup) {
        case Setup::A1_A2: return static_cast<GPIO::PinNumber>(GPIO::Port::PB08);
        case Setup::A3_A4: return static_cast<GPIO::PinNumber>(GPIO::Port::PA04);
        case Setup::P11_P13_1: return static_cast<GPIO::PinNumber>(GPIO::Port::PA16);
        case Setup::P11_P13_3: return static_cast<GPIO::PinNumber>(GPIO::Port::PA16);
        case Setup::SDA_SCL_5: return static_cast<GPIO::PinNumber>(GPIO::Port::PA22);
        default:
            break;
        }
        return static_cast<GPIO::PinNumber>(GPIO::FeatherM0::SDA);
    }

    static inline GPIO::PinNumber getSclPinForSetup(const Setup setup)
    {
        switch (setup) {
        case Setup::A1_A2: return static_cast<GPIO::PinNumber>(GPIO::Port::PB09);
        case Setup::A3_A4: return static_cast<GPIO::PinNumber>(GPIO::Port::PA05);
        case Setup::P11_P13_1: return static_cast<GPIO::PinNumber>(GPIO::Port::PA17);
        case Setup::P11_P13_3: return static_cast<GPIO::PinNumber>(GPIO::Port::PA17);
        case Setup::SDA_SCL_5: return static_cast<GPIO::PinNumber>(GPIO::Port::PA23);
        default:
            break;
        }
        return static_cast<GPIO::PinNumber>(GPIO::FeatherM0::SCL);
    }

public:
    inline explicit WireMaster_FeatherM0(const Setup setup = Setup::Default)
        : WireMaster_SAMD21(getInterfaceForSetup(setup), getSdaPinForSetup(setup), getSclPinForSetup(setup))
    {
    }
};

    
}

