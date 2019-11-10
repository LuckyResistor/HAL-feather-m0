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


#include "EndPointHandler.hpp"
#include "DeviceWrapper.hpp"
#include "InterruptGuard.hpp"


namespace lr::usb {


/// A double buffered end point handler.
///
class DoubleBufferedEPOutHandler : public EPHandler
{
public:
    /// Create a new double buffered end point handler.
    ///
    /// @param endPoint The end point index.
    /// @param bufferSize The buffer size.
    ///
    DoubleBufferedEPOutHandler(DeviceWrapper &deviceWrapper, uint32_t endPoint, uint32_t bufferSize);

    /// dtor
    ///
    virtual ~DoubleBufferedEPOutHandler();

public: // Implement EPHandler
    void init() override;
    uint32_t recv(void *_data, uint32_t len) override;
    void handleEndpoint() override;
    uint32_t available() const override;
    void release() override;

private:
    DeviceWrapper &_deviceWrapper; ///< Access to the device.

    const uint32_t ep;
    const uint32_t size;
    uint32_t current, incoming;

    volatile uint8_t *data0;
    uint32_t first0;
    volatile uint32_t last0;
    volatile bool ready0;

    volatile uint8_t *data1;
    uint32_t first1;
    volatile uint32_t last1;
    volatile bool ready1;

    volatile bool notify;
};


}

