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
#include "DoubleBufferedEndPointHandler.hpp"


#include "DeviceWrapper.hpp"


namespace lr::usb {


DoubleBufferedEPOutHandler::DoubleBufferedEPOutHandler(
    DeviceWrapper &deviceWrapper,
    uint32_t endPoint,
    uint32_t bufferSize)
:
    _deviceWrapper(deviceWrapper),
    ep(endPoint),
    size(bufferSize),
    current(0),
    incoming(0),
    first0(0),
    last0(0),
    ready0(false),
    first1(0),
    last1(0),
    ready1(false),
    notify(false)
{
    data0 = reinterpret_cast<uint8_t *>(malloc(size));
    data1 = reinterpret_cast<uint8_t *>(malloc(size));

    _deviceWrapper.epBank0SetSize(ep, 64);
    _deviceWrapper.epBank0SetType(ep, 3); // BULK OUT

    _deviceWrapper.epBank0SetAddress(ep, const_cast<uint8_t *>(data0));

    release();
}


DoubleBufferedEPOutHandler::~DoubleBufferedEPOutHandler()
{
    free((void *) data0);
    free((void *) data1);
}


void DoubleBufferedEPOutHandler::init()
{
    // nothing to initialize.
}


uint32_t DoubleBufferedEPOutHandler::recv(void *_data, uint32_t len)
{
    uint8_t *data = reinterpret_cast<uint8_t *>(_data);

    // R/W: current, first0/1, ready0/1, notify
    // R  : last0/1, data0/1
    if (current == 0) {
        synchronized {
            if (!ready0) {
                return 0;
            }
        }
        // when ready0==true the buffer is not being filled and last0 is constant
        uint32_t i;
        for (i = 0; i < len && first0 < last0; i++) {
            data[i] = data0[first0++];
        }
        if (first0 == last0) {
            first0 = 0;
            current = 1;
            synchronized {
                ready0 = false;
                if (notify) {
                    notify = false;
                    release();
                }
            }
        }
        return i;
    } else {
        synchronized {
            if (!ready1) {
                return 0;
            }
        }
        // when ready1==true the buffer is not being filled and last1 is constant
        uint32_t i;
        for (i = 0; i < len && first1 < last1; i++) {
            data[i] = data1[first1++];
        }
        if (first1 == last1) {
            first1 = 0;
            current = 0;
            synchronized {
                ready1 = false;
                if (notify) {
                    notify = false;
                    release();
                }
            }
        }
        return i;
    }
}


void DoubleBufferedEPOutHandler::handleEndpoint()
{
    // R/W : incoming, ready0/1
    //   W : last0/1, notify
    if (_deviceWrapper.epBank0IsTransferComplete(ep)) {
        // Ack Transfer complete
        _deviceWrapper.epBank0AckTransferComplete(ep);
        //_deviceWrapper.epBank0AckTransferFailed(ep); // XXX

        // Update counters and swap banks for non-ZLP's
        if (incoming == 0) {
            last0 = _deviceWrapper.epBank0ByteCount(ep);
            if (last0 != 0) {
                incoming = 1;
                _deviceWrapper.epBank0SetAddress(ep, const_cast<uint8_t *>(data1));
                synchronized {
                    ready0 = true;
                    if (ready1) {
                        notify = true;
                        return;
                    }
                    notify = false;
                }
            }
        } else {
            last1 = _deviceWrapper.epBank0ByteCount(ep);
            if (last1 != 0) {
                incoming = 0;
                _deviceWrapper.epBank0SetAddress(ep, const_cast<uint8_t *>(data0));
                synchronized {
                    ready1 = true;
                    if (ready0) {
                        notify = true;
                        return;
                    }
                    notify = false;
                }
            }
        }
        release();
    }
}


uint32_t DoubleBufferedEPOutHandler::available() const
{
    if (current == 0) {
        bool ready = false;
        synchronized {
            ready = ready0;
        }
        return ready ? (last0 - first0) : 0;
    } else {
        bool ready = false;
        synchronized {
            ready = ready1;
        }
        return ready ? (last1 - first1) : 0;
    }
}


void DoubleBufferedEPOutHandler::release()
{
    // Release OUT gEndPointDescriptor
    _deviceWrapper.epBank0EnableTransferComplete(ep);
    _deviceWrapper.epBank0SetMultiPacketSize(ep, size);
    _deviceWrapper.epBank0SetByteCount(ep, 0);
    _deviceWrapper.epBank0ResetReady(ep);
}
}