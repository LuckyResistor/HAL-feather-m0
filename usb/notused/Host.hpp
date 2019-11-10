#pragma once
//
//  Copyright (c) 2014 Arduino LLC.  All right reserved.
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//


#include "hal-core/Chip.hpp"


namespace lr::usb {


extern __attribute__((__aligned__(4))) volatile UsbHostDescriptor usb_pipe_table[USB_EPT_NUM];

const auto USB_EP_DIR_IN = 0x80; // USB_SETUP_DEVICE_TO_HOST
const auto USB_EP_DIR_OUT = 0x00; // USB_SETUP_HOST_TO_DEVICE;

const auto USB_HOST_PTYPE_DIS = USB_HOST_PCFG_PTYPE(0x0); // Pipe is disabled
const auto USB_HOST_PTYPE_CTRL = USB_HOST_PCFG_PTYPE(0x1); // Pipe is enabled and configured as CONTROL
const auto USB_HOST_PTYPE_ISO = USB_HOST_PCFG_PTYPE(0x2); // Pipe is enabled and configured as ISO
const auto USB_HOST_PTYPE_BULK = USB_HOST_PCFG_PTYPE(0x3); // Pipe is enabled and configured as BULK
const auto USB_HOST_PTYPE_INT = USB_HOST_PCFG_PTYPE(0x4); // Pipe is enabled and configured as INTERRUPT
const auto USB_HOST_PTYPE_EXT = USB_HOST_PCFG_PTYPE(0x5); // Pipe is enabled and configured as EXTENDED

const auto USB_HOST_NB_BK_1 = 1;

const auto USB_HOST_PCFG_PTOKEN_SETUP = USB_HOST_PCFG_PTOKEN(0x0);
const auto USB_HOST_PCFG_PTOKEN_IN = USB_HOST_PCFG_PTOKEN(0x1);
const auto USB_HOST_PCFG_PTOKEN_OUT = USB_HOST_PCFG_PTOKEN(0x2);

const auto USB_ERRORFLOW = USB_HOST_STATUS_BK_ERRORFLOW;
const auto USB_ERRORTIMEOUT = USB_HOST_STATUS_PIPE_TOUTER;
const auto USB_ERROR_DATATOGGLE = USB_HOST_STATUS_PIPE_DTGLER;

const auto USB_PCKSIZE_SIZE_8_BYTES = 0;
const auto USB_PCKSIZE_SIZE_16_BYTES = 1;
const auto USB_PCKSIZE_SIZE_32_BYTES = 2;
const auto USB_PCKSIZE_SIZE_64_BYTES = 3 ;
const auto USB_PCKSIZE_SIZE_128_BYTES = 4;
const auto USB_PCKSIZE_SIZE_256_BYTES = 5;
const auto USB_PCKSIZE_SIZE_512_BYTES = 6;
const auto USB_PCKSIZE_SIZE_1023_BYTES_FS = 7;
const auto USB_PCKSIZE_SIZE_1024_BYTES_HS = 7;

inline uint8_t USB_HOST_DTGL(const uint8_t p) {
    return (USB->HOST.HostPipe[p].PSTATUS.reg & USB_HOST_PSTATUS_DTGL) >> USB_HOST_PSTATUS_DTGL_Pos;
}

// USB host connection/disconnection monitoring
inline void uhd_enable_connection_int() { USB->HOST.INTENSET.reg = USB_HOST_INTENSET_DCONN; }
inline void uhd_disable_connection_int() { USB->HOST.INTENCLR.reg = USB_HOST_INTENCLR_DCONN; }
inline void uhd_ack_connection() { USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_DCONN; }

inline void uhd_enable_disconnection_int() { USB->HOST.INTENSET.reg = USB_HOST_INTENSET_DDISC; }
inline void uhd_disable_disconnection_int() { USB->HOST.INTENCLR.reg = USB_HOST_INTENCLR_DDISC; }
inline void uhd_ack_disconnection() { USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_DDISC; }

// Initiates a USB register reset
inline void uhd_start_USB_reg_reset() { USB->HOST.CTRLA.bit.SWRST = 1; }

// Bus Reset
inline void Is_uhd_starting_reset() { (USB->HOST.CTRLB.bit.BUSRESET == 1); }
inline void UHD_BusReset() { USB->HOST.CTRLB.bit.BUSRESET = 1; }
inline void uhd_stop_reset() {} // nothing to do

inline void uhd_ack_reset_sent() { USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_RST; }
inline void Is_uhd_reset_sent() { (USB->HOST.INTFLAG.reg & USB_HOST_INTFLAG_RST); }

// Initiates a SOF events
inline void uhd_enable_sof() { USB->HOST.CTRLB.bit.SOFE = 1; }
inline void uhd_disable_sof() { USB->HOST.CTRLB.bit.SOFE = 0; }
inline bool Is_uhd_sof_enabled() {
    return (USB->HOST.CTRLB.reg & USB_HOST_CTRLB_SOFE);
}
inline bool Is_uhd_sof() {
    return (USB->HOST.INTFLAG.reg & USB_HOST_INTFLAG_HSOF);
}

// USB address of pipes
inline void uhd_configure_address(const uint8_t pipe_num, const uint16_t addr) {
    usb_pipe_table[pipe_num].HostDescBank[0].CTRL_PIPE.bit.PDADDR = addr; }
inline void uhd_get_configured_address(const uint8_t pipe_num) {
    usb_pipe_table[pipe_num].HostDescBank[0].CTRL_PIPE.bit.PDADDR; }

// Pipes
inline void uhd_freeze_pipe(const uint8_t p) { USB->HOST.HostPipe[p].PSTATUSSET.reg = USB_HOST_PSTATUSSET_PFREEZE; }
inline void uhd_unfreeze_pipe(const uint8_t p) { USB->HOST.HostPipe[p].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_PFREEZE; }
inline bool Is_uhd_pipe_frozen(const uint8_t p) {
    return ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_PFREEZE)==USB_HOST_PSTATUS_PFREEZE); }

// Pipe configuration
inline void uhd_configure_pipe_token(const uint8_t p, const uint8_t token) {
    USB->HOST.HostPipe[p].PCFG.bit.PTOKEN = token; }

// Pipe data management
inline void uhd_byte_count(const uint8_t p) {
    usb_pipe_table[p].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT; }
inline void uhd_ack_setup_ready(const uint8_t p) {
    USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TXSTP; }
inline bool Is_uhd_setup_ready(const uint8_t p) {
    return ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TXSTP) == USB_HOST_PINTFLAG_TXSTP); }
inline void uhd_ack_in_received(const uint8_t p) {
    USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT(1); }
inline bool Is_uhd_in_received(const uint8_t p) {
    return ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TRCPT(1)) == USB_HOST_PINTFLAG_TRCPT(1)); }
inline void uhd_ack_out_ready(const uint8_t p) {
    USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT(1); }
inline bool Is_uhd_out_ready(const uint8_t p) {
    return ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TRCPT(1)) == USB_HOST_PINTFLAG_TRCPT(1)); }
inline void uhd_ack_nak_received(const uint8_t p) {
    usb_pipe_table[p].HostDescBank[1].STATUS_BK.reg &= ~USB_HOST_STATUS_BK_ERRORFLOW; }
inline bool Is_uhd_nak_received(const uint8_t p) {
    return (usb_pipe_table[p].HostDescBank[1].STATUS_BK.reg & USB_HOST_STATUS_BK_ERRORFLOW); }

// Endpoint Interrupt Summary
inline uint16_t uhd_endpoint_interrupt() {
    return USB->HOST.PINTSMRY.reg;
}

// Run in Standby
inline void uhd_run_in_standby() { USB->HOST.CTRLA.reg |= USB_CTRLA_RUNSTDBY; }
// Force host mode
inline void uhd_force_host_mode() { USB->HOST.CTRLA.reg |= USB_CTRLA_MODE; }

// Enable USB macro
inline void uhd_enable() { USB->HOST.CTRLA.reg |= USB_CTRLA_ENABLE; }
// Disable USB macro
inline void uhd_disable() { USB->HOST.CTRLA.reg &= ~USB_CTRLA_ENABLE; }

// Force full speed mode
inline void uhd_force_full_speed() { USB->HOST.CTRLB.reg &= ~USB_HOST_CTRLB_SPDCONF_Msk; }

const auto tokSETUP	= USB_HOST_PCFG_PTOKEN_SETUP;
const auto tokIN = USB_HOST_PCFG_PTOKEN_IN;
const auto tokOUT =	USB_HOST_PCFG_PTOKEN_OUT;
const auto tokINHS = USB_HOST_PCFG_PTOKEN_IN;
const auto tokOUTHS	= USB_HOST_PCFG_PTOKEN_OUT;

/// States of USBB interface
///
enum uhd_vbus_state_t {
    UHD_STATE_NO_VBUS = 0,
    UHD_STATE_DISCONNECTED = 1,
    UHD_STATE_CONNECTED = 2,
    UHD_STATE_ERROR = 3,
};

void UHD_Init();
void UHD_Handler();
uhd_vbus_state_t UHD_GetVBUSState();
uint32_t UHD_Pipe0_Alloc(uint32_t ul_add, uint32_t ul_ep_size);
uint32_t UHD_Pipe_Alloc(uint32_t ul_dev_addr, uint32_t ul_dev_ep, uint32_t ul_type, uint32_t ul_dir, uint32_t ul_maxsize, uint32_t ul_interval, uint32_t ul_nb_bank);
void UHD_Pipe_CountZero(uint32_t ul_pipe);
void UHD_Pipe_Free(uint32_t ul_pipe);
uint32_t UHD_Pipe_Read(uint32_t ul_pipe, uint32_t ul_size, uint8_t* data);
void UHD_Pipe_Write(uint32_t ul_pipe, uint32_t ul_size, uint8_t* data);
void UHD_Pipe_Send(uint32_t ul_pipe, uint32_t ul_token_type);
uint32_t UHD_Pipe_Is_Transfer_Complete(uint32_t ul_pipe, uint32_t ul_token_type);


}
