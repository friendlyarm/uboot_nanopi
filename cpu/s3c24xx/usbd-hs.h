/*
 * cpu/s3c24xx/usbd-hs.h
 *
 *  $Id: usbd-hs.h,v 1.1 2007/02/01 07:03:19 dasan Exp $
 *
 * (C) Copyright 2007
 * Byungjae Lee, Samsung Erectronics, bjlee@samsung.com.
 *	- only support for S3C2412, S3C2443
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef _S3C_USBD_HS_H
#define _S3C_USBD_HS_H

#include <asm/byteorder.h>
#include <asm/arch/hardware.h>

#define TRUE	1
#define FALSE	0
#define USB_SPEED_FULL	0
#define USB_SPEED_HIGH	1
#define EP0_STATE_IDLE	0


/* in usbd-ctl-hs.c */
extern unsigned int s3c_usbd_dn_addr;
extern unsigned int s3c_usbd_dn_cnt;
extern int DNW;
extern int s3c_got_header;
extern int s3c_receive_done;

int s3c_usbctl_init(void);
void s3c_udc_int_hndlr(void);
int s3c_usbc_activate(void);
int s3c_usb_stop( void );

/*
 * Descriptor types
 * Refer to Table 9-5 Descriptor Types in the USB specification v2.0
 */
#define	USB_DESC_DEVICE			1
#define	USB_DESC_CONFIG			2
#define	USB_DESC_STRING			3
#define	USB_DESC_INTERFACE		4
#define	USB_DESC_ENDPOINT		5
#define	USB_DESC_DEVICE_QUALIFIER	6
#define	USB_DESC_OTHER_SPEED_CONFIG	7
#define	USB_DESC_INTERFACE_POWER	8

#define DescriptorHeader \
	__u8 bLength;        \
	__u8 bDescriptorType


/* Device Descriptor */
typedef struct {
	 DescriptorHeader;
	 __u16 bcdUSB;		/* USB specification revision number in BCD */
	 __u8 bDeviceClass;	/* USB class for entire device */
	 __u8 bDeviceSubClass;	/* USB subclass information for entire device */
	 __u8 bDeviceProtocol;	/* USB protocol information for entire device */
	 __u8 bMaxPacketSize0;	/* Max packet size for endpoint zero */
	 __u16 idVendor;	/* USB vendor ID */
	 __u16 idProduct;	/* USB product ID */
	 __u16 bcdDevice;	/* vendor assigned device release number */
	 __u8 iManufacturer;	/* index of manufacturer string */
	 __u8 iProduct;		/* index of string that describes product */
	 __u8 iSerialNumber;	/* index of string containing device serial number */
	 __u8 bNumConfigurations; /* number fo configurations */
} __attribute__ ((packed)) device_desc_t;

/* Configuration Descriptor */
typedef struct {
	 DescriptorHeader;
	 __u16 wTotalLength;	/* total # of bytes returned in the cfg buf 4 this cfg */
	 __u8 bNumInterfaces;	/* number of interfaces in this cfg */
	 __u8 bConfigurationValue;/* used to uniquely ID this cfg */
	 __u8 iConfiguration;	/* index of string describing cfg*/
	 __u8 bmAttributes;	/* bitmap of attributes for ths cfg */
	 __u8 MaxPower;		/* power draw in 2ma units */
} __attribute__ ((packed)) config_desc_t;

/* bmAttributes */
#define	USB_CONFIG_REMOTEWAKE	0x20
#define	USB_CONFIG_SELFPOWERED	0x40
#define	USB_CONFIG_BUSPOWERED	0x80

/*
 * MaxPower
 * convert mA to descriptor units of A for MaxPower
 */
 #define USB_POWER( x)  ((x)>>1)

/* Interface Descriptor */

typedef struct {
	 DescriptorHeader;
	 __u8  bInterfaceNumber;   /* Index uniquely identfying this interface */
	 __u8  bAlternateSetting;  /* ids an alternate setting for this interface */
	 __u8  bNumEndpoints;      /* number of endpoints in this interface */
	 __u8  bInterfaceClass;    /* USB class info applying to this interface */
	 __u8  bInterfaceSubClass; /* USB subclass info applying to this interface */
	 __u8  bInterfaceProtocol; /* USB protocol info applying to this interface */
	 __u8  iInterface;         /* index of string describing interface */
} __attribute__ ((packed)) intf_desc_t;

/* String Descriptor */
#define USB_LANGID_US_L 0x09
#define USB_LANGID_US_H 0x04

typedef struct {
	 DescriptorHeader;
	 __u16 bString[1];	/* unicode string .. actaully 'n' __u16s */
} __attribute__ ((packed)) string_desc_t;

/* Endpoint  Descriptor */
typedef struct {
	 DescriptorHeader;
	 __u8 bEndpointAddress;	/* 0..3 ep num, bit 7: 0 = 0ut 1= in */
	 __u8 bmAttributes;	/* 0..1 = 0: ctrl, 1: isoc, 2: bulk 3: intr */
	 __u16 wMaxPacketSize;	/* data payload size for this ep in this cfg */
	 __u8 bInterval;	/* polling interval for this ep in this cfg */
} __attribute__ ((packed)) ep_desc_t;

#define USB_OUT	0
#define USB_IN	1
#define USB_EP_ADDRESS(a,d) (((a)&0xf) | ((d) << 7))

/* bmAttributes: */
#define USB_EP_CNTRL		0
#define USB_EP_ISOCHRONOUS	1
#define USB_EP_BULK		2
#define USB_EP_INT		3

typedef struct {
	__u8 Device;
	__u8 Interface;
	__u8 Endpoint0;
	__u8 Endpoint1;
	__u8 Endpoint3;
} __attribute__ ((packed)) get_status_t;

typedef struct
{
	__u8 AlternateSetting;
} __attribute__ ((packed)) get_intf_t;

/*
 * USB Device Requests
 * Refer to chapter 9.3 in the USB specification v2.0
 */
typedef struct {
	__u8 bmRequestType;
	__u8 bRequest;
	__u16 wValue;
	__u16 wIndex;
	__u16 wLength;
} __attribute__ ((packed)) device_req_t;

/*
 * bmRequestType
 * [7]   : Data transfer direction, 0 = Host to device, 1 = Device to host
 * [6:5] : Type
 * [4:0] : Recipient
 * Refer to Table 9-3 Fromat of Setup Data in the USB specification v2.0
 */
#define USB_RECIPIENT_DEVICE	0
#define USB_RECIPIENT_INTERFACE	1
#define USB_RECIPIENT_ENDPOINT	2
#define USB_RECIPIENT_OTHER	3

/*
 * bRequest
 * Refer to Table 9-4 Standard Device Requests in the USB specification v2.0
 */
#define	USB_REQ_GET_STATUS		0
#define	USB_REQ_CLEAR_FEATURE		1
#define	USB_REQ_RESERVED_1		2
#define	USB_REQ_SET_FEATURE		3
#define	USB_REQ_RESERVED_2		4
#define	USB_REQ_SET_ADDRESS		5
#define	USB_REQ_GET_DESCRIPTOR		6
#define	USB_REQ_SET_DESCRIPTOR		7
#define	USB_REQ_GET_CONFIGURATION	8
#define	USB_REQ_SET_CONFIGURATION	9
#define	USB_REQ_GET_INTERFACE		10
#define	USB_REQ_SET_INTERFACE		11
#define	USB_REQ_SYNCH_FRAME		12


#define	USB_DESC_DEVICE_SIZE	sizeof(device_desc_t)
#define	USB_DESC_CONFIG_SIZE	sizeof(config_desc_t)
#define	USB_DESC_STRING_SIZE	sizeof(string_desc_t)
#define	USB_DESC_INTERFACE_SIZE	sizeof(intf_desc_t)
#define	USB_DESC_ENDPOINT_SIZE	sizeof(ep_desc_t)

/*
 * Feature Selector
 * Refer to Table 9-6 Standard Feature Selectors in the USB specification v2.0
 */
#define USB_ENDPOINT_HALT	0
#define USB_DEV_REMOTE_WAKEUP	1
#define USB_TEST_MODE		2

#define USB_GET_DESC_DEVICE	(20)
#define USB_GET_DESC_CFG	(21)
#define USB_GET_DESC_CFG_ONLY	(22)
#define USB_GET_DESC_INTERFACE	(23)
#define USB_GET_DESC_STR0	(24)
#define USB_GET_DESC_STR1	(25)
#define USB_GET_DESC_STR2	(26)
#define USB_GET_DESC_EP1	(27)
#define USB_GET_DESC_EP3	(28)
#define USB_GET_DESC_QUALIFIER	(29)

#define USB_GET_INTERFACE	(30)

#define USB_GET_STATUS_DEVICE	(40)
#define USB_GET_STATUS_INTF	(41)
#define USB_GET_STATUS_EP0	(42)
#define USB_GET_STATUS_EP1	(43)
#define USB_GET_STATUS_EP3	(44)

/*
 * "config descriptor buffer"
 * that is, one config, one interface and 2 endpoints
 */
struct cdb {
	 config_desc_t cfg;
	 intf_desc_t   intf;
	 ep_desc_t     ep1;
	 ep_desc_t     ep3;
} __attribute__ ((packed));


/* all S3C device descriptors */
typedef struct {
	 device_desc_t dev;   /* device descriptor */
	 struct cdb b;        /* bundle of descriptors for this cfg */
} __attribute__ ((packed)) desc_t;


#endif	/* _S3C_USBD_HS_H */
