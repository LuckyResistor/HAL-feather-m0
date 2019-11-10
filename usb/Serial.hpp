//================================================================================
//	Serial over CDC (Serial1 is the physical port)

class Serial_ : public Stream
{
public:
    Serial_(USBDeviceClass &_usb) : usb(_usb), stalled(false) { }
    void begin(uint32_t baud_count);
    void begin(unsigned long, uint8_t);
    void end(void);

    virtual int available(void);
    virtual int availableForWrite(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buffer, size_t size);
    using Print::write; // pull in write(str) from Print
    operator bool();

    size_t readBytes(char *buffer, size_t length);

    // This method allows processing "SEND_BREAK" requests sent by
    // the USB host. Those requests indicate that the host wants to
    // send a BREAK signal and are accompanied by a single uint16_t
    // value, specifying the duration of the break. The value 0
    // means to end any current break, while the value 0xffff means
    // to start an indefinite break.
    // readBreak() will return the value of the most recent break
    // request, but will return it at most once, returning -1 when
    // readBreak() is called again (until another break request is
    // received, which is again returned once).
    // This also mean that if two break requests are received
    // without readBreak() being called in between, the value of the
    // first request is lost.
    // Note that the value returned is a long, so it can return
    // 0-0xffff as well as -1.
    int32_t readBreak();

    // These return the settings specified by the USB host for the
    // serial port. These aren't really used, but are offered here
    // in case a sketch wants to act on these settings.
    uint32_t baud();
    uint8_t stopbits();
    uint8_t paritytype();
    uint8_t numbits();
    bool dtr();
    bool rts();
    enum {
        ONE_STOP_BIT = 0,
        ONE_AND_HALF_STOP_BIT = 1,
        TWO_STOP_BITS = 2,
    };
    enum {
        NO_PARITY = 0,
        ODD_PARITY = 1,
        EVEN_PARITY = 2,
        MARK_PARITY = 3,
        SPACE_PARITY = 4,
    };

private:
    int availableForStore(void);

    USBDeviceClass &usb;
    RingBuffer *_cdc_rx_buffer;
    bool stalled;
};
extern Serial_ Serial;
