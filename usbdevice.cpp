/* usbdevice.cpp */
/* Generic USB device */
/* Copyright (c) Phil Wright 2008 */

#include "mbed.h"
#include "usbdevice.h"

/* Standard requests */
#define GET_STATUS        (0)
#define CLEAR_FEATURE     (1)
#define SET_FEATURE       (3)
#define SET_ADDRESS       (5)
#define GET_DESCRIPTOR    (6)
#define SET_DESCRIPTOR    (7)
#define GET_CONFIGURATION (8)
#define SET_CONFIGURATION (9)
#define GET_INTERFACE     (10)
#define SET_INTERFACE     (11)

/* Device status */
#define DEVICE_STATUS_SELF_POWERED  (1<<0)
#define DEVICE_STATUS_REMOTE_WAKEUP (1<<1)

/* Endpoint status */
#define ENDPOINT_STATUS_HALT        (1<<0)

/* Standard feature selectors */
#define DEVICE_REMOTE_WAKEUP        (1)
#define ENDPOINT_HALT               (0)

/* Macro to convert wIndex endpoint number to physical endpoint number */
#define WINDEX_TO_PHYSICAL(endpoint) (((endpoint & 0x0f) << 1) + ((endpoint & 0x80) ? 1 : 0))

CONTROL_TRANSFER transfer;
USB_DEVICE device;
    
usbdevice::usbdevice()
{
    /* Set initial device state */
    device.state = POWERED;
    device.configuration = 0;
    device.suspended = false;
    
    /* Set the maximum packet size for the control endpoints */
    realiseEndpoint(EP0IN, MAX_PACKET_SIZE_EP0);
    realiseEndpoint(EP0OUT, MAX_PACKET_SIZE_EP0);
    
    /* Enable endpoint events for EP0 */
    enableEndpointEvent(EP0IN);
    enableEndpointEvent(EP0OUT);
}

void usbdevice::endpointEventEP0Setup(void)
{
    /* Endpoint 0 setup event */
    if (!controlSetup())
    {    
        /* Protocol stall; this will stall both endpoints */
        stallEndpoint(EP0OUT);
    }
}

void usbdevice::endpointEventEP0Out(void)
{
    /* Endpoint 0 OUT data event */
    if (!controlOut())
    {    
        /* Protocol stall; this will stall both endpoints */
        stallEndpoint(EP0OUT);
    }    
}

void usbdevice::endpointEventEP0In(void)
{
    /* Endpoint 0 IN data event */
    if (!controlIn())
    {    
        /* Protocol stall; this will stall both endpoints */
        stallEndpoint(EP0OUT);
    }
}

void usbdevice::deviceEventReset(void)
{
    device.state = DEFAULT;
    device.configuration = 0;
    device.suspended = false;
}

void usbdevice::decodeSetupPacket(unsigned char *data, SETUP_PACKET *packet)
{
    /* Fill in the elements of a SETUP_PACKET structure from raw data */
    packet->bmRequestType.dataTransferDirection = (data[0] & 0x80) >> 7;
    packet->bmRequestType.Type = (data[0] & 0x60) >> 5;
    packet->bmRequestType.Recipient = data[0] & 0x1f;
    packet->bRequest = data[1];
    packet->wValue = (data[2] | (unsigned short)data[3] << 8);
    packet->wIndex = (data[4] | (unsigned short)data[5] << 8);
    packet->wLength = (data[6] | (unsigned short)data[7] << 8);
}

bool usbdevice::controlSetup(void)
{
    /* Control transfer setup stage */
    unsigned char buffer[MAX_PACKET_SIZE_EP0];
    unsigned long count;

    count = endpointRead(EP0OUT, buffer);
    
    /* Must be 8 bytes of data */
    if (count != 8)
    {    
        return false;
    }
    
    /* Initialise control transfer state */
    decodeSetupPacket(buffer, &transfer.setup);        
    transfer.ptr = NULL;
    transfer.remaining = 0;
    transfer.direction = 0;
    transfer.zlp = false;
    
    /* Process request */
    if (!requestSetup())
    {
        return false;
    }

    /* Check transfer size and direction  */
    if (transfer.setup.wLength>0)
    {
        if (transfer.setup.bmRequestType.dataTransferDirection==DEVICE_TO_HOST)
        {
            /* IN data stage is required */
            if (transfer.direction != DEVICE_TO_HOST)
            {
                return false;
            }
            
            /* Transfer must be less than or equal to the size requested by the host */
            if (transfer.remaining > transfer.setup.wLength)
            {
                transfer.remaining = transfer.setup.wLength;
            }
        }
        else
        {
            /* OUT data stage is required */
            if (transfer.direction != HOST_TO_DEVICE)
            {
                return false;
            }
            
            /* Transfer must be equal to the size requested by the host */
            if (transfer.remaining != transfer.setup.wLength)
            {
                return false;
            }
        }
    }
    else
    {
        /* No data stage; transfer size must be zero */
        if (transfer.remaining != 0)
        {
            return false;
        }
    }                
    
    /* Data or status stage if applicable */
    if (transfer.setup.wLength>0)
    {
        if (transfer.setup.bmRequestType.dataTransferDirection==DEVICE_TO_HOST)
        {            
            /* Check if we'll need to send a zero length packet at the end of this transfer */
            if (transfer.setup.wLength > transfer.remaining)
            {
                /* Device wishes to transfer less than host requested */
                if ((transfer.remaining % MAX_PACKET_SIZE_EP0) == 0)
                {
                    /* Transfer is a multiple of EP0 max packet size */
                    transfer.zlp = true;
                }            
            }
            
            /* IN stage */
            controlIn();
        }
    }
    else
    {
        /* Status stage */
        endpointWrite(EP0IN, NULL, 0);
    }
    
    return true;
}

bool usbdevice::controlOut(void)
{    
    /* Control transfer data OUT stage */
    unsigned char buffer[MAX_PACKET_SIZE_EP0];
    unsigned long packetSize;

    /* Check we should be transferring data OUT */
    if (transfer.direction != HOST_TO_DEVICE)
    {
        return false;
    }
    
    /* Read from endpoint */
    packetSize = endpointRead(EP0OUT, buffer);
    
    /* Check if transfer size is valid */
    if (packetSize > transfer.remaining)
    {
        /* Too big */
        return false;
    }
    
    /* Update transfer */
    transfer.ptr += packetSize;
    transfer.remaining -= packetSize;
    
    /* Check if transfer has completed */
    if (transfer.remaining == 0)
    {
        /* Process request */        
        if (!requestOut())
        {
            return false;
        }
        
        /* Status stage */
        endpointWrite(EP0IN, NULL, 0);
    }
    
    return true;
}

bool usbdevice::controlIn(void)
{
    /* Control transfer data IN stage */    
    unsigned packetSize;
    
    /* Check if transfer has completed (status stage transactions also have transfer.remaining == 0) */
    if (transfer.remaining == 0)
    {
        if (transfer.zlp)
        {
            /* Send zero length packet */
            endpointWrite(EP0IN, NULL, 0);
            transfer.zlp = false;
        }
        
        /* Completed */
        return true;
    }
    
    /* Check we should be transferring data IN */
    if (transfer.direction != DEVICE_TO_HOST)
    {
        return false;
    }
    
    packetSize = transfer.remaining;
    
    if (packetSize > MAX_PACKET_SIZE_EP0)
    {
        packetSize = MAX_PACKET_SIZE_EP0;
    }
    
    /* Write to endpoint */
    endpointWrite(EP0IN, transfer.ptr, packetSize);
    
    /* Update transfer */
    transfer.ptr += packetSize;
    transfer.remaining -= packetSize;
    
    return true;
}

bool usbdevice::requestSetup(void)
{
    bool success = false;

    /* Process standard requests */
    if ((transfer.setup.bmRequestType.Type == STANDARD_TYPE))
    {
        switch (transfer.setup.bRequest)
        {
             case GET_STATUS:
                 success = requestGetStatus();
                 break;
             case CLEAR_FEATURE:
                 success = requestClearFeature();
                 break;
             case SET_FEATURE:
                 success = requestSetFeature();
                 break;
             case SET_ADDRESS:
                success = requestSetAddress();
                 break;    
             case GET_DESCRIPTOR:
                 success = requestGetDescriptor();
                 break;
             case SET_DESCRIPTOR:
                 /* TODO: Support is optional, not implemented here */
                 success = false;
                 break;
             case GET_CONFIGURATION:
                 success = requestGetConfiguration();
                 break;
             case SET_CONFIGURATION:
                 success = requestSetConfiguration();
                 break;
             case GET_INTERFACE:
                 success = requestGetInterface();
                 break;
             case SET_INTERFACE:
                 success = requestSetInterface();
                 break;
             default:
                 break;
        }
    }
    
    return success;
}

bool usbdevice::requestOut(void)
{
    return true;
}

bool usbdevice::requestSetAddress(void)
{
    /* Set the device address */
    setAddress(transfer.setup.wValue);    
    
    if (transfer.setup.wValue == 0)
    {
        device.state = DEFAULT;
    }
    else
    {
        device.state = ADDRESS;
    }
        
    return true;
}

bool usbdevice::requestSetConfiguration(void)
{
    /* Set the device configuration */    
    if (transfer.setup.wValue == 0)
    {
        /* Not configured */
        unconfigureDevice();
        device.state = ADDRESS;
    }
    else
    {
        configureDevice();
        device.state = CONFIGURED;
    }
    
    /* TODO: We do not currently support multiple configurations, just keep a record of the configuration value */
    device.configuration = transfer.setup.wValue;
    
    return true;
}

bool usbdevice::requestGetConfiguration(void)
{
    /* Send the device configuration */
    transfer.ptr = &device.configuration;
    transfer.remaining = sizeof(device.configuration);
    transfer.direction = DEVICE_TO_HOST;
    return true;
}

bool usbdevice::requestGetInterface(void)
{
    static unsigned char alternateSetting;
    
    /* Return the selected alternate setting for an interface */
    
    if (device.state != CONFIGURED)
    {
        return false;
    }
    
    /* TODO: We currently do not support alternate settings so always return zero */
    /* TODO: Should check that the interface number is valid */    
    alternateSetting = 0;
    
    /* Send the alternate setting */
    transfer.ptr = &alternateSetting;
    transfer.remaining = sizeof(alternateSetting);
    transfer.direction = DEVICE_TO_HOST;
    return true;    
}

bool usbdevice::requestSetInterface(void)
{
    /* TODO: We currently do not support alternate settings, return false */
    return false;
}    

bool usbdevice::requestSetFeature()
{
    bool success = false;
    
    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer.setup.wIndex != 0)
        {
            return false;
        }
    }
    
    switch (transfer.setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Remote wakeup feature not supported */
            break;
        case ENDPOINT_RECIPIENT:    
            if (transfer.setup.wValue == ENDPOINT_HALT)
            {
                /* TODO: We should check that the endpoint number is valid */
                stallEndpoint(WINDEX_TO_PHYSICAL(transfer.setup.wIndex));
                success = true;
            }            
            break;
        default:
            break;
    }
    
    return success;
}

bool usbdevice::requestClearFeature()
{
    bool success = false;
    
    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer.setup.wIndex != 0)
        {
            return false;
        }
    }
    
    switch (transfer.setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Remote wakeup feature not supported */
            break;
        case ENDPOINT_RECIPIENT:
            /* TODO: We should check that the endpoint number is valid */    
            if (transfer.setup.wValue == ENDPOINT_HALT)
            {
                unstallEndpoint(WINDEX_TO_PHYSICAL(transfer.setup.wIndex));
                success = true;
            }            
            break;
        default:
            break;
    }
    
    return success;
}

bool usbdevice::requestGetStatus(void)
{
    static unsigned short status;
    bool success = false;
    
    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer.setup.wIndex != 0)
        {
            return false;
        }
    }
    
    switch (transfer.setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Currently only supports self powered devices */
            status = DEVICE_STATUS_SELF_POWERED;
            success = true;
            break;
        case INTERFACE_RECIPIENT:
            status = 0;
            success = true;
            break;
        case ENDPOINT_RECIPIENT:
            /* TODO: We should check that the endpoint number is valid */
            if (getEndpointStallState(WINDEX_TO_PHYSICAL(transfer.setup.wIndex)))
            {
                status = ENDPOINT_STATUS_HALT;
            }
            else
            {
                status = 0;
            } 
            success = true;
            break;
        default:
            break;
    }
    
    if (success)
    {
        /* Send the status */ 
        transfer.ptr = (unsigned char *)&status; /* Assumes little endian */
        transfer.remaining = sizeof(status); 
        transfer.direction = DEVICE_TO_HOST;
    }
    
    return success;
}

bool usbdevice::requestGetDescriptor(void)
{
    return false;
}