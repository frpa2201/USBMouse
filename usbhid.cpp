/* usbhid.cpp */
/* USB HID class device */
/* Copyright (c) Phil Wright 2008 */

#include "mbed.h"
#include "usbhid.h"
#include "asciihid.h"

/* Endpoint packet sizes */
#define MAX_PACKET_SIZE_EP1 (64)

/* HID Class */
#define HID_CLASS         (3)
#define HID_SUBCLASS_NONE (0)
#define HID_PROTOCOL_NONE (0)
#define HID_DESCRIPTOR    (33)
#define REPORT_DESCRIPTOR (34)

/* Class requests */
#define GET_REPORT (0x1)
#define GET_IDLE   (0x2)
#define SET_REPORT (0x9)
#define SET_IDLE   (0xa)
    
/* Descriptors */
unsigned char deviceDescriptor[] = {
    0x12,                    /* bLength */
    DEVICE_DESCRIPTOR,       /* bDescriptorType */
    0x00,                    /* bcdUSB (LSB) */
    0x02,                    /* bcdUSB (MSB) */
    0x00,                    /* bDeviceClass */
    0x00,                    /* bDeviceSubClass */
    0x00,                    /* bDeviceprotocol */
    MAX_PACKET_SIZE_EP0,     /* bMaxPacketSize0 */
    0x28,                    /* idVendor (LSB) */
    0x0d,                    /* idVendor (MSB) */
    0x05,                    /* idProduct (LSB) */
    0x02,                    /* idProduct (MSB) */
    0x00,                    /* bcdDevice (LSB) */
    0x00,                    /* bcdDevice (MSB) */
    0x00,                    /* iManufacturer */
    0x00,                    /* iProduct */
    0x00,                    /* iSerialNumber */
    0x01                     /* bNumConfigurations */
    };
    
unsigned char configurationDescriptor[] = {
    0x09,                        /* bLength */
    CONFIGURATION_DESCRIPTOR,    /* bDescriptorType */
    0x09+0x09+0x09+0x07,         /* wTotalLength (LSB) */
    0x00,                        /* wTotalLength (MSB) */
    0x01,                        /* bNumInterfaces */
    0x01,                        /* bConfigurationValue */
    0x00,                        /* iConfiguration */
    0xc0,                        /* bmAttributes */
    0x00,                        /* bMaxPower */
    
    0x09,                        /* bLength */
    INTERFACE_DESCRIPTOR,        /* bDescriptorType */    
    0x00,                        /* bInterfaceNumber */
    0x00,                        /* bAlternateSetting */
    0x01,                        /* bNumEndpoints */
    HID_CLASS,                   /* bInterfaceClass */
    HID_SUBCLASS_NONE,           /* bInterfaceSubClass */
    HID_PROTOCOL_NONE,           /* bInterfaceProtocol */
    0x00,                        /* iInterface */
    
    0x09,                        /* bLength */
    HID_DESCRIPTOR,              /* bDescriptorType */
    0x11,                        /* bcdHID (LSB) */
    0x01,                        /* bcdHID (MSB) */
    0x00,                        /* bCountryCode */
    0x01,                        /* bNumDescriptors */
    REPORT_DESCRIPTOR,           /* bDescriptorType */
    0x79,                        /* wDescriptorLength (LSB) */
    0x00,                        /* wDescriptorLength (MSB) */
        
    0x07,                        /* bLength */
    ENDPOINT_DESCRIPTOR,         /* bDescriptorType */
    0x81,                        /* bEndpointAddress */
    0x03,                        /* bmAttributes */
    MAX_PACKET_SIZE_EP1,         /* wMaxPacketSize (LSB) */
    0x00,                        /* wMaxPacketSize (MSB) */
    0x0a,                        /* bInterval */
    };
    
/* HID Class Report Descriptor */
/* Short items: size is 0, 1, 2 or 3 specifying 0, 1, 2 or 4 (four) bytes of data as per HID Class standard */

/* Main items */
#define INPUT(size)             (0x80 | size)
#define OUTPUT(size)            (0x90 | size)
#define FEATURE(size)           (0xb0 | size)
#define COLLECTION(size)        (0xa0 | size)
#define END_COLLECTION(size)    (0xc0 | size)

/* Global items */
#define USAGE_PAGE(size)        (0x04 | size)
#define LOGICAL_MIN(size)       (0x14 | size)
#define LOGICAL_MAX(size)       (0x24 | size)
#define PHYSICAL_MIN(size)      (0x34 | size)
#define PHYSICAL_MAX(size)      (0x44 | size)
#define UNIT_EXPONENT(size)     (0x54 | size)
#define UNIT(size)              (0x64 | size)
#define REPORT_SIZE(size)       (0x74 | size)
#define REPORT_ID(size)         (0x84 | size)
#define REPORT_COUNT(size)      (0x94 | size)
#define PUSH(size)              (0xa4 | size)
#define POP(size)               (0xb4 | size)

/* Local items */
#define USAGE(size)             (0x08 | size)
#define USAGE_MIN(size)         (0x18 | size)
#define USAGE_MAX(size)         (0x28 | size)
#define DESIGNATOR_INDEX(size)  (0x38 | size)
#define DESIGNATOR_MIN(size)    (0x48 | size)
#define DESIGNATOR_MAX(size)    (0x58 | size)
#define STRING_INDEX(size)      (0x78 | size)
#define STRING_MIN(size)        (0x88 | size)
#define STRING_MAX(size)        (0x98 | size)
#define DELIMITER(size)         (0xa8 | size)

#define REPORT_ID_KEYBOARD      (1)
#define REPORT_ID_MOUSE         (2)

#define MAX_REPORT_SIZE         (8)

unsigned char reportDescriptor[] = {
/* Keyboard */
USAGE_PAGE(1),      0x01,
USAGE(1),           0x06,
COLLECTION(1),      0x01,
REPORT_ID(1),       REPORT_ID_KEYBOARD,
USAGE_PAGE(1),      0x07,
USAGE_MIN(1),       0xE0,
USAGE_MAX(1),       0xE7,
LOGICAL_MIN(1),     0x00,
LOGICAL_MAX(1),     0x01,
REPORT_SIZE(1),     0x01,
REPORT_COUNT(1),    0x08,
INPUT(1),           0x02,
REPORT_COUNT(1),    0x01,
REPORT_SIZE(1),     0x08,
INPUT(1),           0x01,
REPORT_COUNT(1),    0x05,
REPORT_SIZE(1),     0x01,
USAGE_PAGE(1),      0x08,
USAGE_MIN(1),       0x01,
USAGE_MAX(1),       0x05,
OUTPUT(1),          0x02,
REPORT_COUNT(1),    0x01,
REPORT_SIZE(1),     0x03,
OUTPUT(1),          0x01,
REPORT_COUNT(1),    0x06,
REPORT_SIZE(1),     0x08,
LOGICAL_MIN(1),     0x00,
LOGICAL_MAX(2),     0xff, 0x00,
USAGE_PAGE(1),      0x07,
USAGE_MIN(1),       0x00,
USAGE_MAX(2),       0xff, 0x00,
INPUT(1),           0x00,
END_COLLECTION(0),

/* Mouse */
USAGE_PAGE(1),      0x01, 
USAGE(1),           0x02, 
COLLECTION(1),      0x01, 
USAGE(1),           0x01, 
COLLECTION(1),      0x00, 
REPORT_ID(1),       REPORT_ID_MOUSE,
REPORT_COUNT(1),    0x03,
REPORT_SIZE(1),     0x01,
USAGE_PAGE(1),      0x09,
USAGE_MIN(1),       0x1,
USAGE_MAX(1),       0x3,
LOGICAL_MIN(1),     0x00,
LOGICAL_MAX(1),     0x01,
INPUT(1),           0x02,
REPORT_COUNT(1),    0x01,
REPORT_SIZE(1),     0x05,
INPUT(1),           0x01,
REPORT_COUNT(1),    0x03,
REPORT_SIZE(1),     0x08,
USAGE_PAGE(1),      0x01,
USAGE(1),           0x30,
USAGE(1),           0x31,
USAGE(1),           0x38,
LOGICAL_MIN(1),     0x81,
LOGICAL_MAX(1),     0x7f,
INPUT(1),           0x06,
END_COLLECTION(0),
END_COLLECTION(0),
};
    
volatile bool complete;
volatile bool configured;
unsigned char outputReport[MAX_REPORT_SIZE];

usbhid::usbhid()
{
    configured = false;
    connect();
}

void usbhid::deviceEventReset()
{
    configured = false;
    
    /* Must call base class */ 
    usbdevice::deviceEventReset();
}

bool usbhid::requestSetConfiguration(void)
{
    bool result;
    
    /* Configure IN interrupt endpoint */
    realiseEndpoint(EP1IN, MAX_PACKET_SIZE_EP1);
    enableEndpointEvent(EP1IN);
    
    /* Must call base class */
    result = usbdevice::requestSetConfiguration();
    
    if (result)
    {
        /* Now configured */
        configured = true;
    }
    
    return result;
}

bool usbhid::requestGetDescriptor(void)
{
    bool success = false;
        
    switch (DESCRIPTOR_TYPE(transfer.setup.wValue))
    {
        case DEVICE_DESCRIPTOR:
            transfer.remaining = sizeof(deviceDescriptor);
            transfer.ptr = deviceDescriptor;
            transfer.direction = DEVICE_TO_HOST;
            success = true;
            break;
        case CONFIGURATION_DESCRIPTOR:
            transfer.remaining =  sizeof(configurationDescriptor);
            transfer.ptr = configurationDescriptor;
            transfer.direction = DEVICE_TO_HOST;
            success = true;
            break;
        case STRING_DESCRIPTOR:
        case INTERFACE_DESCRIPTOR:
        case ENDPOINT_DESCRIPTOR:
            /* TODO: Support is optional, not implemented here */
            break;
        case HID_DESCRIPTOR:
            transfer.remaining = 0x09; /* TODO: Fix hard coded size/offset */
            transfer.ptr = &configurationDescriptor[18];
            transfer.direction = DEVICE_TO_HOST;
            success = true;            
            break;
        case REPORT_DESCRIPTOR:
            transfer.remaining = sizeof(reportDescriptor);
            transfer.ptr = reportDescriptor;
            transfer.direction = DEVICE_TO_HOST;
            success = true;            
            break;
        default:
            break;    
    }
    
    return success;
}

bool usbhid::requestSetup(void)
{
    /* Process class requests */
    bool success = false;

    if (transfer.setup.bmRequestType.Type == CLASS_TYPE)
    {
        switch (transfer.setup.bRequest)
        {
             case SET_REPORT:
                 switch (transfer.setup.wValue & 0xff)
                 {
                    case REPORT_ID_KEYBOARD:                     
                        /* TODO: LED state */
                        transfer.remaining = sizeof(outputReport);
                        transfer.ptr = outputReport;
                        transfer.direction = HOST_TO_DEVICE;
                        success = true;    
                        break;
                    default:
                        break;
                 }
                 break;
             default:
                 break;
        }
    }
    
    if (success)
    {
        /* We've handled this request */
        return true;
    }
    
    return usbdevice::requestSetup();
}

bool usbhid::sendInputReport(unsigned char id, unsigned char *data, unsigned char size)
{
    /* Send an Input Report */
    /* If data is NULL an all zero report is sent */
    
    static unsigned char report[MAX_REPORT_SIZE+1]; /* +1 for report ID */
    unsigned char i;

    if (size > MAX_REPORT_SIZE)
    {
        return false;
    }
    
    /* Add report ID */
    report[0]=id;

    /* Add report data */
    if (data != NULL)
    {    
        for (i=0; i<size; i++)
        {
            report[i+1] = *data++;
        }
    }
    else
    {    
        for (i=0; i<size; i++)
        {
            report[i+1] = 0;
        }
    }    
    
    /* Block if not configured */
    while (!configured);
    
    /* Send report */
    complete = false;
    disableEvents();
    endpointWrite(EP1IN, report, size+1); /* +1 for report ID */
    enableEvents();
    
    /* Wait for completion */
    while(!complete && configured);    
    return true;
}
    
void usbhid::endpointEventEP1In(void)
{
    complete = true;
}

bool usbhid::keyboard(char c)
{
    /* Send a simulated keyboard keypress. Returns true if successful. */    
    unsigned char report[8]={0,0,0,0,0,0,0,0};

    report[0] = keymap[c].modifier;
    report[2] = keymap[c].usage;

    /* Key down */
    if (!sendInputReport(REPORT_ID_KEYBOARD, report, 8))
    {
        return false;
    }

    /* Key up */
    if (!sendInputReport(REPORT_ID_KEYBOARD, NULL, 8))
    {
        return false;
    }    

    return true;
}

bool usbhid::keyboard(char *string)
{
    /* Send a string of characters. Returns true if successful. */
    do {
        if (!keyboard(*string++))
        {
            return false;
        }
    } while (*string != '\0');

    return true;
}

bool usbhid::mouse(signed char x, signed char y, unsigned char buttons, signed char wheel)
{
    /* Send a simulated mouse event. Returns true if successful. */    
    unsigned char report[4]={0,0,0,0};

    report[0] = buttons;
    report[1] = x;
    report[2] = y;
    report[3] = wheel;
    
    if (!sendInputReport(REPORT_ID_MOUSE, report, 4))
    {
        return false;
    }    

    return true;
}
