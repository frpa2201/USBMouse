/* usbhid.h */
/* USB HID class device */
/* Copyright (c) Phil Wright 2008 */

#ifndef USBHID_H
#define USBHID_H

#include "usbdevice.h"

/* Mouse buttons */
#define MOUSE_L (1<<0)
#define MOUSE_M (1<<1)
#define MOUSE_R (1<<2)

class usbhid : public usbdevice
{
public:
    usbhid();
    bool keyboard(char c);
    bool keyboard(char *string);
    bool mouse(signed char x, signed char y, unsigned char buttons=0, signed char wheel=0);
protected:
    virtual bool requestSetConfiguration();
    virtual void endpointEventEP1In(void);
    virtual void deviceEventReset(void);
    virtual bool requestGetDescriptor(void);
    virtual bool requestSetup(void);
private:
    bool sendInputReport(unsigned char id, unsigned char *data, unsigned char size);
};

#endif