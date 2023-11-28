/* usbdc.h */
/* USB device controller */
/* Copyright (c) Phil Wright 2008 */

#ifndef USBDC_H
#define USBDC_H

/* Endpoints */
#define EP0OUT  (0) /* Control */
#define EP0IN   (1) /* Control */
#define EP1OUT  (2) /* Interrupt */
#define EP1IN   (3) /* Interrupt */
#define EP2OUT  (4) /* Bulk */
#define EP2IN   (5) /* Bulk */

#include "mbed.h"

class usbdc : public Base 
{
public:
    usbdc();
    void connect(void);
    void disconnect(void);
protected:
    void setAddress(unsigned char address);
    void realiseEndpoint(unsigned char endpoint, unsigned long maxPacket);
    void enableEndpointEvent(unsigned char endpoint);
    void disableEndpointEvent(unsigned char endpoint);
    void stallEndpoint(unsigned char endpoint);
    void unstallEndpoint(unsigned char endpoint);
    bool getEndpointStallState(unsigned char endpoint);
    void configureDevice(void);
    void unconfigureDevice(void);
    unsigned long endpointRead(unsigned char endpoint, unsigned char *buffer);
    void endpointWrite(unsigned char endpoint, unsigned char *buffer, unsigned long size);
    void enableEvents(void);
    void disableEvents(void);    
    virtual void deviceEventReset(void);
    virtual void deviceEventFrame(void); 
    virtual void endpointEventEP0Setup(void);
    virtual void endpointEventEP0In(void);
    virtual void endpointEventEP0Out(void);
    virtual void endpointEventEP1In(void);
    virtual void endpointEventEP1Out(void);    
    virtual void endpointEventEP2In(void);
    virtual void endpointEventEP2Out(void);        
private:
    void SIECommand(unsigned long command);
    void SIEWriteData(unsigned char data);
    unsigned char SIEReadData(unsigned long command);
    void setDeviceStatus(unsigned char status);
    void setEndpointStatus(unsigned char endpoint, unsigned char status);    
    unsigned char getDeviceStatus(void);
    unsigned char selectEndpoint(unsigned char endpoint);
    unsigned char selectEndpointClearInterrupt(unsigned char endpoint);
    unsigned char clearBuffer(void);
    void validateBuffer(void);
    void usbisr(void);
    unsigned long endpointStallState;
    static void _usbisr(void);
    static usbdc *instance;
};


#endif