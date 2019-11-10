
void Serial_::begin(uint32_t /* baud_count */)
{
    // uart config is ignored in USB-CDC
}


void Serial_::begin(uint32_t /* baud_count */, uint8_t /* config */)
{
    // uart config is ignored in USB-CDC
}


void Serial_::end(void)
{
    memset((void *) &_usbLineInfo, 0, sizeof(_usbLineInfo));
}


int Serial_::available(void)
{
    return usb.available(CDC_ENDPOINT_OUT);
}


int Serial_::availableForWrite(void)
{
    // return the number of bytes left in the current bank,
    // always gEndPointDescriptor size - 1, because bank is flushed on every write
    return (EPX_SIZE - 1);
}


int _serialPeek = -1;


int Serial_::peek(void)
{
    if (_serialPeek != -1)
        return _serialPeek;
    _serialPeek = read();
    return _serialPeek;
}


int Serial_::read(void)
{
    if (_serialPeek != -1) {
        int res = _serialPeek;
        _serialPeek = -1;
        return res;
    }
    return usb.recv(CDC_ENDPOINT_OUT);
}


size_t Serial_::readBytes(char *buffer, size_t length)
{
    size_t count = 0;
    _startMillis = millis();
    while (count < length) {
        uint32_t n = usb.recv(CDC_ENDPOINT_OUT, buffer + count, length - count);
        if (n == 0 && (millis() - _startMillis) >= _timeout)
            break;
        count += n;
    }
    return count;
}


void Serial_::flush(void)
{
    usb.flush(CDC_ENDPOINT_IN);
}


size_t Serial_::write(const uint8_t *buffer, size_t size)
{
    /* only try to send bytes if the high-level CDC connection itself
     is open (not just the pipe) - the OS should set lineState when the port
     is opened and clear lineState when the port is closed.
     bytes sent before the user opens the connection or after
     the connection is closed are lost - just like with a UART. */

    // TODO - ZE - check behavior on different OSes and test what happens if an
    // open connection isn't broken cleanly (cable is yanked out, host dies
    // or locks up, or host virtual serial port hangs)
    uint32_t r = 0;
    if (_usbLineInfo.lineState > 0)  // Problem with Windows(R)
    {
        r = usb.send(CDC_ENDPOINT_IN, buffer, size);
    }

    if (r > 0) {
        return r;
    } else {
        setWriteError();
        return 0;
    }
}


size_t Serial_::write(uint8_t c)
{
    return write(&c, 1);
}


// This operator is a convenient way for a sketch to check whether the
// port has actually been configured and opened by the host (as opposed
// to just being connected to the host).  It can be used, for example, in
// setup() before printing to ensure that an application on the host is
// actually ready to receive and display the data.
// We add a short delay before returning to fix a bug observed by Federico
// where the port is configured (lineState != 0) but not quite opened.
Serial_::operator bool()
{
    // this is here to avoid spurious opening after upload
    if (millis() < 500)
        return false;

    bool result = false;

    if (_usbLineInfo.lineState > 0) {
        result = true;
    }

    delay(10);
    return result;
}


Serial_ Serial(USBDevice);
