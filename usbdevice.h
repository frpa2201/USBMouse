/* usbdevice.h */
/* Generic USB device */
/* Copyright (c) Phil Wright 2008 */

#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "usbdc.h"

/* Endpoint packet sizes */
#define MAX_PACKET_SIZE_EP0 (64)

/* bmRequestType.dataTransferDirection */
#define HOST_TO_DEVICE (0)
#define DEVICE_TO_HOST (1)

/* bmRequestType.Type*/
#define STANDARD_TYPE  (0)
#define CLASS_TYPE     (1)
#define VENDOR_TYPE    (2)
#define RESERVED_TYPE  (3)

/* bmRequestType.Recipient */
#define DEVICE_RECIPIENT    (0)
#define INTERFACE_RECIPIENT (1)
#define ENDPOINT_RECIPIENT  (2)
#define OTHER_RECIPIENT     (3)

/* Descriptors */
#define DESCRIPTOR_TYPE(wValue)  (wValue >> 8)
#define DESCRIPTOR_INDEX(wValue) (wValue & 0xf)

/* Descriptor type */
#define DEVICE_DESCRIPTOR        (1)
#define CONFIGURATION_DESCRIPTOR (2)
#define STRING_DESCRIPTOR        (3)
#define INTERFACE_DESCRIPTOR     (4)
#define ENDPOINT_DESCRIPTOR      (5)

typedef struct {
    struct { 
        unsigned char dataTransferDirection;
        unsigned char Type;
        unsigned char Recipient;
    } bmRequestType;
    unsigned char  bRequest;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
} SETUP_PACKET;

typedef struct {
    SETUP_PACKET  setup;
    unsigned char *ptr;
    unsigned long remaining;
    unsigned char direction;
    bool          zlp;
} CONTROL_TRANSFER;

typedef enum {ATTACHED, POWERED, DEFAULT, ADDRESS, CONFIGURED} DEVICE_STATE;

typedef struct {
    DEVICE_STATE  state;
    unsigned char configuration;
    bool          suspended;
} USB_DEVICE;

class usbdevice : public usbdc
{
public:
    usbdevice(); 
protected:
    virtual void endpointEventEP0Setup(void);
    virtual void endpointEventEP0In(void);
    virtual void endpointEventEP0Out(void);
    virtual bool requestSetup(void);
    virtual bool requestOut(void);
    virtual void deviceEventReset(void);
    virtual bool requestGetDescriptor(void);
            bool requestSetAddress(void);
    virtual bool requestSetConfiguration(void);
    virtual bool requestGetConfiguration(void);
            bool requestGetStatus(void);
    virtual bool requestSetInterface(void);
    virtual bool requestGetInterface(void);
            bool requestSetFeature(void);
            bool requestClearFeature(void);    
    CONTROL_TRANSFER transfer;
    USB_DEVICE device;
private:
    bool controlIn(void);
    bool controlOut(void);
    bool controlSetup(void);
    void decodeSetupPacket(unsigned char *data, SETUP_PACKET *packet);
};

#endif