/* usbdc.cpp */
/* USB device controller */
/* Copyright (c) Phil Wright 2008 */

#include "mbed.h"  
#include "usbdc.h"
#include "cmsis.h"

#ifdef  TARGET_LPC2368
#undef  LPC_USB_BASE
#define LPC_USB_BASE (0xFFE0C000) /* TODO */
#endif         

/* Power Control for Peripherals register */
#define PCUSB      ((unsigned long)1<<31)

/* USB Clock Control register */
#define DEV_CLK_EN ((unsigned long)1<<1)
#define AHB_CLK_EN ((unsigned long)1<<4)

/* USB Clock Status register */
#define DEV_CLK_ON ((unsigned long)1<<1)
#define AHB_CLK_ON ((unsigned long)1<<4)

/* USB Device Interupt registers */
#define FRAME      ((unsigned long)1<<0)
#define EP_FAST    ((unsigned long)1<<1)
#define EP_SLOW    ((unsigned long)1<<2)
#define DEV_STAT   ((unsigned long)1<<3)
#define CCEMPTY    ((unsigned long)1<<4)
#define CDFULL     ((unsigned long)1<<5)
#define RxENDPKT   ((unsigned long)1<<6)
#define TxENDPKT   ((unsigned long)1<<7)
#define EP_RLZED   ((unsigned long)1<<8)
#define ERR_INT    ((unsigned long)1<<9)

/* Endpoint Interrupt Registers */
#define EP(endpoint) (1<<endpoint)

/* USB Control register */
#define RD_EN (1<<0)
#define WR_EN (1<<1)
#define LOG_ENDPOINT(endpoint) ((endpoint>>1)<<2)

/* USB Receive Packet Length register */
#define DV      ((unsigned long)1<<10)
#define PKT_RDY ((unsigned long)1<<11)
#define PKT_LNGTH_MASK (0x3ff)

/* Serial Interface Engine (SIE) */
#define SIE_WRITE   (0x01)
#define SIE_READ    (0x02)
#define SIE_COMMAND (0x05)
#define SIE_CMD_CODE(phase, data) ((phase<<8)|(data<<16))

/* SIE Command codes */
#define SIE_CMD_SET_ADDRESS        (0xD0)
#define SIE_CMD_CONFIGURE_DEVICE   (0xD8)
#define SIE_CMD_SET_MODE           (0xF3)
#define SIE_CMD_READ_FRAME_NUMBER  (0xF5)
#define SIE_CMD_READ_TEST_REGISTER (0xFD)
#define SIE_CMD_SET_DEVICE_STATUS  (0xFE)
#define SIE_CMD_GET_DEVICE_STATUS  (0xFE)
#define SIE_CMD_GET_ERROR_CODE     (0xFF)
#define SIE_CMD_READ_ERROR_STATUS  (0xFB)

#define SIE_CMD_SELECT_ENDPOINT(endpoint)                 (0x00+endpoint)
#define SIE_CMD_SELECT_ENDPOINT_CLEAR_INTERRUPT(endpoint) (0x40+endpoint)
#define SIE_CMD_SET_ENDPOINT_STATUS(endpoint)             (0x40+endpoint)

#define SIE_CMD_CLEAR_BUFFER    (0xF2)
#define SIE_CMD_VALIDATE_BUFFER (0xFA)

/* SIE Device Status register */
#define SIE_DS_CON    (1<<0)
#define SIE_DS_CON_CH (1<<1)
#define SIE_DS_SUS    (1<<2)
#define SIE_DS_SUS_CH (1<<3)
#define SIE_DS_RST    (1<<4)

/* SIE Device Set Address register */
#define SIE_DSA_DEV_EN  (1<<7)

/* SIE Configue Device register */
#define SIE_CONF_DEVICE (1<<0)

/* Select Endpoint register */
#define SIE_SE_FE       (1<<0)
#define SIE_SE_ST       (1<<1)
#define SIE_SE_STP      (1<<2)
#define SIE_SE_PO       (1<<3)
#define SIE_SE_EPN      (1<<4)
#define SIE_SE_B_1_FULL (1<<5)
#define SIE_SE_B_2_FULL (1<<6)

/* Set Endpoint Status command */
#define SIE_SES_ST      (1<<0)
#define SIE_SES_DA      (1<<5)
#define SIE_SES_RF_MO   (1<<6)
#define SIE_SES_CND_ST  (1<<7)

usbdc *usbdc::instance;

usbdc::usbdc()
{
#ifdef TARGET_LPC1768
    LPC_SC->USBCLKCFG=5; /* TODO */
#endif

    /* Enable power to USB device controller */
    LPC_SC->PCONP |= PCUSB;

    /* Enable USB clocks */
    LPC_USB->USBClkCtrl |= DEV_CLK_EN | AHB_CLK_EN;
    while (LPC_USB->USBClkSt != (DEV_CLK_ON | AHB_CLK_ON));

    /* Configure pins P0.29 and P0.30 to be USB D+ and USB D- */
    LPC_PINCON->PINSEL1 &= 0xc3ffffff;
    LPC_PINCON->PINSEL1 |= 0x14000000;

#ifdef ENABLE_VBUS    
    /* Configure pin P1.30 to be VBUS */
    LPC_PINCON->PINSEL3 &= 0xcfffffff;
    LPC_PINCON->PINSEL3 |= 0x20000000;
    
    /* Configure pin P1.30 to have pull-down */
    LPC_PINCON->PINMODE3 |= 0x30000000;
#endif
        
    /* Configure pin P2.9 to be Connect */
    LPC_PINCON->PINSEL4 &= 0xfffcffff;
    LPC_PINCON->PINSEL4 |= 0x00040000;
    
    /* Connect must be low for at least 2.5uS */
    wait_ms(1);
    
    /* Attach IRQ */
    instance = this;
    NVIC_SetVector(USB_IRQn, (uint32_t)&_usbisr);
    NVIC_EnableIRQ(USB_IRQn); 

    /* Enable device interrupts */
    enableEvents();
}

void usbdc::connect(void)
{
    /* Connect USB device */
    unsigned char status;
    
    status = getDeviceStatus();
    setDeviceStatus(status | SIE_DS_CON);
}

void usbdc::disconnect(void)
{
    /* Disconnect USB device */
    unsigned char status;
    
    status = getDeviceStatus();
    setDeviceStatus(status & ~SIE_DS_CON);
}

void usbdc::SIECommand(unsigned long command)
{
    /* The command phase of a SIE transaction */
    LPC_USB->USBDevIntClr = CCEMPTY;
    LPC_USB->USBCmdCode = SIE_CMD_CODE(SIE_COMMAND, command);
    while (!(LPC_USB->USBDevIntSt & CCEMPTY)); 
}

void usbdc::SIEWriteData(unsigned char data)
{
    /* The data write phase of a SIE transaction */
    LPC_USB->USBDevIntClr = CCEMPTY;
    LPC_USB->USBCmdCode = SIE_CMD_CODE(SIE_WRITE, data);
    while (!(LPC_USB->USBDevIntSt & CCEMPTY)); 
}

unsigned char usbdc::SIEReadData(unsigned long command)
{
    /* The data read phase of a SIE transaction */
    LPC_USB->USBDevIntClr = CDFULL;
    LPC_USB->USBCmdCode = SIE_CMD_CODE(SIE_READ, command);
    while (!(LPC_USB->USBDevIntSt & CDFULL));
    return (unsigned char)LPC_USB->USBCmdData;
}

void usbdc::setDeviceStatus(unsigned char status)
{
    /* Write SIE device status register */
    SIECommand(SIE_CMD_SET_DEVICE_STATUS);
    SIEWriteData(status);
}

unsigned char usbdc::getDeviceStatus(void)
{
    /* Read SIE device status register */
    SIECommand(SIE_CMD_GET_DEVICE_STATUS);
    return SIEReadData(SIE_CMD_GET_DEVICE_STATUS);
}

void usbdc::setAddress(unsigned char address)
{
    /* Write SIE device address register */
    SIECommand(SIE_CMD_SET_ADDRESS);
    SIEWriteData((address & 0x7f) | SIE_DSA_DEV_EN);
}

unsigned char usbdc::selectEndpoint(unsigned char endpoint)
{
    /* SIE select endpoint command */
    SIECommand(SIE_CMD_SELECT_ENDPOINT(endpoint));
    return SIEReadData(SIE_CMD_SELECT_ENDPOINT(endpoint));
}

#if 1
unsigned char usbdc::selectEndpointClearInterrupt(unsigned char endpoint)
{
    /* SIE select endpoint and clear interrupt command */
    /* Using the Select Endpoint / Clear Interrupt SIE command does not seem   */
    /* to clear the appropriate bit in EP_INT_STAT? - using EP_INT_CLR instead */
    LPC_USB->USBEpIntClr = EP(endpoint);
    while (!(LPC_USB->USBDevIntSt & CDFULL));
    return (unsigned char)LPC_USB->USBCmdData;
}
#else
unsigned char usbdc::selectEndpointClearInterrupt(unsigned char endpoint)
{
    /* SIE select endpoint and clear interrupt command */
    SIECommand(SIE_CMD_SELECT_ENDPOINT_CLEAR_INTERRUPT(endpoint));
    return SIEReadData(SIE_CMD_SELECT_ENDPOINT_CLEAR_INTERRUPT(endpoint));
}
#endif

unsigned char usbdc::clearBuffer(void)
{
    /* SIE clear buffer command */
    SIECommand(SIE_CMD_CLEAR_BUFFER);
    return SIEReadData(SIE_CMD_CLEAR_BUFFER);
}

void usbdc::validateBuffer(void)
{
    /* SIE validate buffer command */
    SIECommand(SIE_CMD_VALIDATE_BUFFER);
}

void usbdc::setEndpointStatus(unsigned char endpoint, unsigned char status)
{
    /* SIE set endpoint status command */
    SIECommand(SIE_CMD_SET_ENDPOINT_STATUS(endpoint));
    SIEWriteData(status);
}

void usbdc::realiseEndpoint(unsigned char endpoint, unsigned long maxPacket)
{
    /* Realise an endpoint */
    LPC_USB->USBDevIntClr = EP_RLZED;
    LPC_USB->USBReEp |= EP(endpoint);
    LPC_USB->USBEpInd = endpoint;
    LPC_USB->USBMaxPSize = maxPacket;
    
    while (!(LPC_USB->USBDevIntSt & EP_RLZED));
    LPC_USB->USBDevIntClr = EP_RLZED;
    
    /* Clear stall state */
    endpointStallState &= ~EP(endpoint);
}

void usbdc::enableEndpointEvent(unsigned char endpoint)
{
    /* Enable an endpoint interrupt */
    LPC_USB->USBEpIntEn |= EP(endpoint);
}

void usbdc::disableEndpointEvent(unsigned char endpoint)
{
    /* Disable an endpoint interrupt */
    LPC_USB->USBEpIntEn &= ~EP(endpoint);
}

void usbdc::stallEndpoint(unsigned char endpoint)
{
    /* Stall an endpoint */
    if ( (endpoint==EP0IN) || (endpoint==EP0OUT) )
    {
        /* Conditionally stall both control endpoints */
        setEndpointStatus(EP0OUT, SIE_SES_CND_ST);
    }
    else
    {
        setEndpointStatus(endpoint, SIE_SES_ST);
        
        /* Update stall state */
        endpointStallState |= EP(endpoint);
    }
}

void usbdc::unstallEndpoint(unsigned char endpoint)
{
    /* Unstall an endpoint. The endpoint will also be reinitialised */
    setEndpointStatus(endpoint, 0);
    
    /* Update stall state */
    endpointStallState &= ~EP(endpoint);
}

bool usbdc::getEndpointStallState(unsigned char endpoint)
{ 
    /* Returns true if endpoint stalled */
    return endpointStallState & EP(endpoint);
}

void usbdc::configureDevice(void)
{
    /* SIE Configure device command */
    SIECommand(SIE_CMD_CONFIGURE_DEVICE);
    SIEWriteData(SIE_CONF_DEVICE);
}

void usbdc::unconfigureDevice(void)
{
    /* SIE Configure device command */
    SIECommand(SIE_CMD_CONFIGURE_DEVICE);
    SIEWriteData(0);
}    

unsigned long usbdc::endpointRead(unsigned char endpoint, unsigned char *buffer)
{
    /* Read from an OUT endpoint */
    unsigned long size;
    unsigned long i;
    unsigned long data;
    unsigned char offset;
    
    LPC_USB->USBCtrl = LOG_ENDPOINT(endpoint) | RD_EN;
    while (!(LPC_USB->USBRxPLen & PKT_RDY));
        
    size = LPC_USB->USBRxPLen & PKT_LNGTH_MASK;
        
    offset = 0;
    
    for (i=0; i<size; i++)
    {    
        if (offset==0)
        {    
            /* Fetch up to four bytes of data as a word */
            data = LPC_USB->USBRxData;
        }
    
        /* extract a byte */
        *buffer++ = data>>offset;
    
        /* move on to the next byte */
        offset = (offset + 8) % 32;
    }    
    
    /* Clear RD_EN to cover zero length packet case */
    LPC_USB->USBCtrl=0;
    
    selectEndpoint(endpoint);
    clearBuffer();
    
    return size;
}

void usbdc::endpointWrite(unsigned char endpoint, unsigned char *buffer, unsigned long size)
{
    /* Write to an IN endpoint */
    unsigned long temp, data;
    unsigned char offset;
    
    LPC_USB->USBCtrl = LOG_ENDPOINT(endpoint) | WR_EN;
    
    LPC_USB->USBTxPLen = size;    
    offset = 0;
    data = 0;
    
    if (size>0)
    {
        do {
            /* Fetch next data byte into a word-sized temporary variable */
            temp = *buffer++;
        
            /* Add to current data word */
            temp = temp << offset;
            data = data | temp;
        
            /* move on to the next byte */
            offset = (offset + 8) % 32;
            size--;
        
            if ((offset==0) || (size==0))
            {
                /* Write the word to the endpoint */
                LPC_USB->USBTxData = data;
                data = 0;
            }    
        } while (size>0);
    }

    /* Clear WR_EN to cover zero length packet case */
    LPC_USB->USBCtrl=0;
    
    selectEndpoint(endpoint);
    validateBuffer();
}

void usbdc::enableEvents(void)
{
    /* Enable interrupt sources */
    LPC_USB->USBDevIntEn = EP_SLOW | DEV_STAT;
}

void usbdc::disableEvents(void)
{
    /* Disable interrupt sources */
    LPC_USB->USBDevIntClr = EP_SLOW | DEV_STAT;
}

void usbdc::usbisr(void)
{ 
    unsigned char devStat;
    
    if (LPC_USB->USBDevIntSt & FRAME)
    {
        /* Frame event */
        deviceEventFrame();
        /* Clear interrupt status flag */
        LPC_USB->USBDevIntClr = FRAME;
    }
    
    if (LPC_USB->USBDevIntSt & DEV_STAT)
    {
        /* Device Status interrupt */        
        /* Must clear the interrupt status flag before reading the device status from the SIE */
        LPC_USB->USBDevIntClr = DEV_STAT;    
            
        /* Read device status from SIE */
        devStat = getDeviceStatus();
        
        if (devStat & SIE_DS_RST)
        {
            /* Bus reset */
            deviceEventReset();
        }
    }
    
    if (LPC_USB->USBDevIntSt & EP_SLOW)
    {
        /* (Slow) Endpoint Interrupt */
        
        /* Process each endpoint interrupt */
        if (LPC_USB->USBEpIntSt & EP(EP0OUT))
        {
            if (selectEndpointClearInterrupt(EP0OUT) & SIE_SE_STP)
            {
                /* this is a setup packet */
                endpointEventEP0Setup();
            }
            else
            {
                endpointEventEP0Out();
            }
        }

        if (LPC_USB->USBEpIntSt & EP(EP0IN))
        {
            selectEndpointClearInterrupt(EP0IN);
            endpointEventEP0In();
        }
        
        if (LPC_USB->USBEpIntSt & EP(EP1OUT))
        {
            selectEndpointClearInterrupt(EP1OUT);
            endpointEventEP1Out();
        }    
        
        if (LPC_USB->USBEpIntSt & EP(EP1IN))
        {
            selectEndpointClearInterrupt(EP1IN);
            endpointEventEP1In();
        }    
        
        if (LPC_USB->USBEpIntSt & EP(EP2OUT))
        {
            selectEndpointClearInterrupt(EP2OUT);
            endpointEventEP2Out();
        }    
        
        if (LPC_USB->USBEpIntSt & EP(EP2IN))
        {
            selectEndpointClearInterrupt(EP2IN);
            endpointEventEP2In();
        }    
        
        /* Clear interrupt status flag */
        /* EP_SLOW and EP_FAST interrupt bits should be cleared after the corresponding endpoint interrupts are cleared. */
        LPC_USB->USBDevIntClr = EP_SLOW;
    }
}


void usbdc::_usbisr(void)
{
    instance->usbisr();
}

void usbdc::deviceEventReset(void)
{
}

void usbdc::deviceEventFrame(void)
{
}

void usbdc::endpointEventEP0Setup(void)
{
}

void usbdc::endpointEventEP0In(void)
{
}

void usbdc::endpointEventEP0Out(void)
{
}

void usbdc::endpointEventEP1In(void)
{
}

void usbdc::endpointEventEP1Out(void)
{
}

void usbdc::endpointEventEP2In(void)
{
}

void usbdc::endpointEventEP2Out(void)
{
}
