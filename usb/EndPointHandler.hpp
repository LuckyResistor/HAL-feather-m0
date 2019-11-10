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


#include <cstdlib>
#include <cstdio>
#include <cstdint>


namespace lr::usb {


/// USB end point generic handler.
///
class EPHandler
{
public:
    virtual void init() = 0;
    virtual void handleEndpoint() = 0;
    virtual uint32_t recv(void *_data, uint32_t len) = 0;
    virtual uint32_t available() const = 0;
    virtual void release() = 0;
};


}


