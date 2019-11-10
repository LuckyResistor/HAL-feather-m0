//
// Copyright (c) 2015 Arduino LLC.  All right reserved.
// SAMD51 support added by Adafruit - Copyright (c)2018 Dean Miller for Adafruit Industries
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
#include "DeviceWrapper.hpp"


#include "InterruptGuard.hpp"

#include <cstring>


namespace lr::usb {


DeviceWrapper &getDeviceWrapper0()
{
    static DeviceWrapper device0(&USB->DEVICE);
    return device0;
}


namespace {


/// Get the packet size for an endpoint.
///
inline constexpr uint8_t getEndPointPacketSize(uint16_t size)
{
    switch (size) {
    case 8: return 0;
    case 16: return 1;
    case 32: return 2;
    case 64: return 3;
    case 128: return 4;
    case 256: return 5;
    case 512: return 6;
    case 1023: return 7; // ??? not 1024 ?
    default: return 0;
    }
}


}


DeviceWrapper::DeviceWrapper(UsbDevice *usbDevice)
    : _usb(usbDevice), _endPointDescriptor()
{
}


void DeviceWrapper::init()
{
    // Enable USB clock
    PM->APBBMASK.reg |= PM_APBBMASK_USB;

    // Set up the USB DP/DN pins
    PORT->Group[0].PINCFG[PIN_PA24G_USB_DM].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[PIN_PA24G_USB_DM / 2].reg &= ~(0xF << (4 * (PIN_PA24G_USB_DM & 0x01u)));
    PORT->Group[0].PMUX[PIN_PA24G_USB_DM / 2].reg |= MUX_PA24G_USB_DM << (4 * (PIN_PA24G_USB_DM & 0x01u));
    PORT->Group[0].PINCFG[PIN_PA25G_USB_DP].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[PIN_PA25G_USB_DP / 2].reg &= ~(0xF << (4 * (PIN_PA25G_USB_DP & 0x01u)));
    PORT->Group[0].PMUX[PIN_PA25G_USB_DP / 2].reg |= MUX_PA25G_USB_DP << (4 * (PIN_PA25G_USB_DP & 0x01u));

    // Put Generic Clock Generator 0 as source for Generic Clock Multiplexer 6 (USB reference)
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(6) | // Generic Clock Multiplexer 6
                        GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is source
                        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY);
}


void DeviceWrapper::reset()
{
    _usb->CTRLA.bit.SWRST = 1;
    memset(_endPointDescriptor, 0, sizeof(_endPointDescriptor));
    while (_usb->SYNCBUSY.bit.SWRST || _usb->SYNCBUSY.bit.ENABLE) {}
    _usb->DESCADD.reg = (uint32_t) (&_endPointDescriptor);
}


void DeviceWrapper::calibrate()
{
    // Load Pad Calibration data from non-volatile memory
    uint32_t *pad_transn_p = (uint32_t *)USB_FUSES_TRANSN_ADDR;
    uint32_t *pad_transp_p = (uint32_t *)USB_FUSES_TRANSP_ADDR;
    uint32_t *pad_trim_p = (uint32_t *)USB_FUSES_TRIM_ADDR;

    uint32_t pad_transn = (*pad_transn_p & USB_FUSES_TRANSN_Msk) >> USB_FUSES_TRANSN_Pos;
    uint32_t pad_transp = (*pad_transp_p & USB_FUSES_TRANSP_Msk) >> USB_FUSES_TRANSP_Pos;
    uint32_t pad_trim = (*pad_trim_p & USB_FUSES_TRIM_Msk) >> USB_FUSES_TRIM_Pos;

    if (pad_transn == 0x1F)  // maximum value (31)
        pad_transn = 5;
    if (pad_transp == 0x1F)  // maximum value (31)
        pad_transp = 29;
    if (pad_trim == 0x7)     // maximum value (7)
        pad_trim = 3;

    _usb->PADCAL.bit.TRANSN = pad_transn;
    _usb->PADCAL.bit.TRANSP = pad_transp;
    _usb->PADCAL.bit.TRIM = pad_trim;
}


void DeviceWrapper::enable()
{
    // Configure interrupts
    NVIC_SetPriority((IRQn_Type) USB_IRQn, 0UL);
    NVIC_EnableIRQ((IRQn_Type) USB_IRQn);

    _usb->CTRLA.bit.ENABLE = 1;
}


void DeviceWrapper::disable()
{
    _usb->CTRLA.bit.ENABLE = 0;

    NVIC_DisableIRQ((IRQn_Type) USB_IRQn);
}


void DeviceWrapper::setUSBDeviceMode()
{
    _usb->CTRLA.bit.MODE = USB_CTRLA_MODE_DEVICE_Val;
}


void DeviceWrapper::setUSBHostMode()
{
    _usb->CTRLA.bit.MODE = USB_CTRLA_MODE_HOST_Val;
}


void DeviceWrapper::runInStandby()
{
    _usb->CTRLA.bit.RUNSTDBY = 1;
}


void DeviceWrapper::noRunInStandby()
{
    _usb->CTRLA.bit.RUNSTDBY = 0;
}


void DeviceWrapper::wakeupHost()
{
    _usb->CTRLB.bit.UPRSM = 1;
}


void DeviceWrapper::setFullSpeed()
{
    _usb->CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_FS_Val;
}


void DeviceWrapper::setLowSpeed()
{
    _usb->CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_LS_Val;
}


void DeviceWrapper::setHiSpeed()
{
    _usb->CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_HS_Val;
}


void DeviceWrapper::setHiSpeedTestMode()
{
    _usb->CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_HSTM_Val;
}


void DeviceWrapper::attach()
{
    _usb->CTRLB.bit.DETACH = 0;
}


void DeviceWrapper::detach()
{
    _usb->CTRLB.bit.DETACH = 1;
}


bool DeviceWrapper::isEndOfResetInterrupt()
{
    return _usb->INTFLAG.bit.EORST;
}


void DeviceWrapper::ackEndOfResetInterrupt()
{
    _usb->INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;
}


void DeviceWrapper::enableEndOfResetInterrupt()
{
    _usb->INTENSET.bit.EORST = 1;
}


void DeviceWrapper::disableEndOfResetInterrupt()
{
    _usb->INTENCLR.bit.EORST = 1;
}


bool DeviceWrapper::isStartOfFrameInterrupt()
{
    return _usb->INTFLAG.bit.SOF;
}


void DeviceWrapper::ackStartOfFrameInterrupt()
{
    _usb->INTFLAG.reg = USB_DEVICE_INTFLAG_SOF;
}


void DeviceWrapper::enableStartOfFrameInterrupt()
{
    _usb->INTENSET.bit.SOF = 1;
}


void DeviceWrapper::disableStartOfFrameInterrupt()
{
    _usb->INTENCLR.bit.SOF = 1;
}


void DeviceWrapper::setAddress(uint32_t addr)
{
    _usb->DADD.bit.DADD = addr;
    _usb->DADD.bit.ADDEN = 1;
}


void DeviceWrapper::unsetAddress()
{
    _usb->DADD.bit.DADD = 0;
    _usb->DADD.bit.ADDEN = 0;
}


uint16_t DeviceWrapper::frameNumber()
{
    return _usb->FNUM.bit.FNUM;
}


void DeviceWrapper::epBank0SetType(EndPointIndex ep, uint8_t type)
{
    _usb->DeviceEndpoint[ep].EPCFG.bit.EPTYPE0 = type;
}


void DeviceWrapper::epBank1SetType(EndPointIndex ep, uint8_t type)
{
    _usb->DeviceEndpoint[ep].EPCFG.bit.EPTYPE1 = type;
}


uint16_t DeviceWrapper::epInterruptSummary()
{
    return _usb->EPINTSMRY.reg;
}


bool DeviceWrapper::epBank0IsSetupReceived(EndPointIndex ep)
{
    return _usb->DeviceEndpoint[ep].EPINTFLAG.bit.RXSTP;
}


bool DeviceWrapper::epBank0IsStalled(EndPointIndex ep)
{
    return _usb->DeviceEndpoint[ep].EPINTFLAG.bit.STALL0;
}


bool DeviceWrapper::epBank1IsStalled(EndPointIndex ep)
{
    return _usb->DeviceEndpoint[ep].EPINTFLAG.bit.STALL1;
}


bool DeviceWrapper::epBank0IsTransferComplete(EndPointIndex ep)
{
    return _usb->DeviceEndpoint[ep].EPINTFLAG.bit.TRCPT0;
}


bool DeviceWrapper::epBank1IsTransferComplete(EndPointIndex ep)
{
    return _usb->DeviceEndpoint[ep].EPINTFLAG.bit.TRCPT1;
}


void DeviceWrapper::epBank0AckSetupReceived(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP;
}


void DeviceWrapper::epBank0AckStalled(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_STALL(1);
}


void DeviceWrapper::epBank1AckStalled(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_STALL(2);
}


void DeviceWrapper::epBank0AckTransferComplete(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT(1);
}


void DeviceWrapper::epBank1AckTransferComplete(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT(2);
}


void DeviceWrapper::epBank0EnableSetupReceived(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENSET.bit.RXSTP = 1;
}


void DeviceWrapper::epBank0EnableStalled(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENSET.bit.STALL0 = 1;
}


void DeviceWrapper::epBank1EnableStalled(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENSET.bit.STALL1 = 1;
}


void DeviceWrapper::epBank0EnableTransferComplete(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENSET.bit.TRCPT0 = 1;
}


void DeviceWrapper::epBank1EnableTransferComplete(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENSET.bit.TRCPT1 = 1;
}


void DeviceWrapper::epBank0DisableSetupReceived(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENCLR.bit.RXSTP = 1;
}


void DeviceWrapper::epBank0DisableStalled(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENCLR.bit.STALL0 = 1;
}


void DeviceWrapper::epBank1DisableStalled(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENCLR.bit.STALL1 = 1;
}


void DeviceWrapper::epBank0DisableTransferComplete(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENCLR.bit.TRCPT0 = 1;
}


void DeviceWrapper::epBank1DisableTransferComplete(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPINTENCLR.bit.TRCPT1 = 1;
}


bool DeviceWrapper::epBank0IsReady(EndPointIndex ep)
{
    return _usb->DeviceEndpoint[ep].EPSTATUS.bit.BK0RDY;
}


bool DeviceWrapper::epBank1IsReady(EndPointIndex ep)
{
    return _usb->DeviceEndpoint[ep].EPSTATUS.bit.BK1RDY;
}


void DeviceWrapper::epBank0SetReady(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPSTATUSSET.bit.BK0RDY = 1;
}


void DeviceWrapper::epBank1SetReady(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPSTATUSSET.bit.BK1RDY = 1;
}


void DeviceWrapper::epBank0ResetReady(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPSTATUSCLR.bit.BK0RDY = 1;
}


void DeviceWrapper::epBank1ResetReady(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPSTATUSCLR.bit.BK1RDY = 1;
}


void DeviceWrapper::epBank0SetStallReq(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPSTATUSSET.bit.STALLRQ0 = 1;
}


void DeviceWrapper::epBank1SetStallReq(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPSTATUSSET.bit.STALLRQ1 = 1;
}


void DeviceWrapper::epBank0ResetStallReq(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPSTATUSCLR.bit.STALLRQ0 = 1;
}


void DeviceWrapper::epBank1ResetStallReq(EndPointIndex ep)
{
    _usb->DeviceEndpoint[ep].EPSTATUSCLR.bit.STALLRQ1 = 1;
}


uint16_t DeviceWrapper::epBank0ByteCount(EndPointIndex ep)
{
    return _endPointDescriptor[ep].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT;
}


uint16_t DeviceWrapper::epBank1ByteCount(EndPointIndex ep)
{
    return _endPointDescriptor[ep].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT;
}


void DeviceWrapper::epBank0SetByteCount(EndPointIndex ep, uint16_t bc)
{
    _endPointDescriptor[ep].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = bc;
}


void DeviceWrapper::epBank1SetByteCount(EndPointIndex ep, uint16_t bc)
{
    _endPointDescriptor[ep].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = bc;
}


void DeviceWrapper::epBank0SetMultiPacketSize(EndPointIndex ep, uint16_t s)
{
    _endPointDescriptor[ep].DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = s;
}


void DeviceWrapper::epBank1SetMultiPacketSize(EndPointIndex ep, uint16_t s)
{
    _endPointDescriptor[ep].DeviceDescBank[1].PCKSIZE.bit.MULTI_PACKET_SIZE = s;
}


void DeviceWrapper::epBank0SetAddress(EndPointIndex ep, void *addr)
{
    _endPointDescriptor[ep].DeviceDescBank[0].ADDR.reg = (uint32_t)addr;
}


void DeviceWrapper::epBank1SetAddress(EndPointIndex ep, void *addr)
{
    _endPointDescriptor[ep].DeviceDescBank[1].ADDR.reg = (uint32_t)addr;
}


void DeviceWrapper::epBank0SetSize(EndPointIndex ep, uint16_t size)
{
    _endPointDescriptor[ep].DeviceDescBank[0].PCKSIZE.bit.SIZE = getEndPointPacketSize(size);
}


void DeviceWrapper::epBank1SetSize(EndPointIndex ep, uint16_t size)
{
    _endPointDescriptor[ep].DeviceDescBank[1].PCKSIZE.bit.SIZE = getEndPointPacketSize(size);
}


void DeviceWrapper::epBank0DisableAutoZLP(EndPointIndex ep)
{
    _endPointDescriptor[ep].DeviceDescBank[0].PCKSIZE.bit.AUTO_ZLP = 0;
}


void DeviceWrapper::epBank1DisableAutoZLP(EndPointIndex ep)
{
    _endPointDescriptor[ep].DeviceDescBank[1].PCKSIZE.bit.AUTO_ZLP = 0;
}


void DeviceWrapper::epBank0EnableAutoZLP(EndPointIndex ep)
{
    _endPointDescriptor[ep].DeviceDescBank[0].PCKSIZE.bit.AUTO_ZLP = 1;
}


void DeviceWrapper::epBank1EnableAutoZLP(EndPointIndex ep)
{
    _endPointDescriptor[ep].DeviceDescBank[1].PCKSIZE.bit.AUTO_ZLP = 1;
}


}

