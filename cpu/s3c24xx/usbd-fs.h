/*
 * cpu/s3c24xx/usbd-fs.h
 *
 *  $Id: $
 *
 * (C) Copyright 2007
 * Byungjae Lee, Samsung Erectronics, bjlee@samsung.com.
 *	- only support for S3C2412, S3C2443
 *
 *	Copyright (C)  Compaq Computer Corporation, 1998, 1999
 *	Copyright (C)  Extenex Corporation 2001
 *	Copyright (C)  MIZI Research Inc. 2002
 *
 *  PRIVATE interface used to share info among components of the S3C2400 USB
 *  core: usb_ctl, usb_ep0, usb_recv and usb_send. Clients of the USB core
 *  should use s3c2400_usb.h.
 *
 *  bushi <bushi@mizi.com>
 */

/*
 * Public interface to the s3c24x0 USB core. For use by client modules
 * like usb-eth and usb-char.
 *  2004 (C) Samsung Elecronics
 *              SW.LEE <hitchcar@samsung.com>
                For S3C2440A and S3C24A0A
 *
 */

#ifndef _S3C_USBD_FS_H
#define _S3C_USBD_FS_H

#include <asm/byteorder.h>
#include <asm/arch/hardware.h>

#undef USE_USBD_DMA

#define TX_PACKET_SIZE		64
#define RX_PACKET_SIZE		64
#define EP0_STATE_IDLE		0
#define EP0_STATE_TRANSFER	1
#define EP0_STATE_RECEIVER	2

/* in usbd-ctrl-fs.c */
extern unsigned int s3c_usbd_dn_addr;
extern unsigned int s3c_usbd_dn_cnt;
extern int DNW;
extern struct usb_info_t s3c_usbd_info;

int s3c_usbctl_init(void);
void s3c_udc_int_hndlr(void);
void s3c_twiddle_descriptors( void );
int s3c_usbc_activate(void);
int s3c_usb_stop( void );

/* in usbd-ep0-fs.c */
extern unsigned int s3c_ep0_state;
extern u8 s3c_set_configuration;
extern u8 s3c_set_interface;
extern u8 s3c_device_status;
extern u8 s3c_ep0_status;
extern u8 s3c_ep_bulk_in_status;
extern u8 s3c_ep_bulk_out_status;

void s3c_ep0_reset(void);
void s3c_ep0_int_hndlr(void);

/* in usbd-recv-fs.c */
extern int s3c_got_header;
extern int s3c_receive_done;

void s3c_ep1_int_hndlr(int status);
void s3c_ep1_reset(void);
void s3c_ep2_reset(void);
void s3c_ep2_int_hndlr(int status);


//////////////////////////////////////////////////////////////////////////////
// Descriptor Management
//////////////////////////////////////////////////////////////////////////////

#define DescriptorHeader \
	__u8 bLength;        \
	__u8 bDescriptorType


// --- Device Descriptor -------------------

typedef struct {
	 DescriptorHeader;
	 __u16 bcdUSB;		   	/* USB specification revision number in BCD */
	 __u8  bDeviceClass;	/* USB class for entire device */
	 __u8  bDeviceSubClass; /* USB subclass information for entire device */
	 __u8  bDeviceProtocol; /* USB protocol information for entire device */
	 __u8  bMaxPacketSize0; /* Max packet size for endpoint zero */
	 __u16 idVendor;        /* USB vendor ID */
	 __u16 idProduct;       /* USB product ID */
	 __u16 bcdDevice;       /* vendor assigned device release number */
	 __u8  iManufacturer;	/* index of manufacturer string */
	 __u8  iProduct;        /* index of string that describes product */
	 __u8  iSerialNumber;	/* index of string containing device serial number */
	 __u8  bNumConfigurations; /* number fo configurations */
} __attribute__ ((packed)) device_desc_t;

// --- Configuration Descriptor ------------

typedef struct {
	 DescriptorHeader;
	 __u16 wTotalLength;	    /* total # of bytes returned in the cfg buf 4 this cfg */
	 __u8  bNumInterfaces;      /* number of interfaces in this cfg */
	 __u8  bConfigurationValue; /* used to uniquely ID this cfg */
	 __u8  iConfiguration;      /* index of string describing configuration */
	 __u8  bmAttributes;        /* bitmap of attributes for ths cfg */
	 __u8  MaxPower;		    /* power draw in 2ma units */
} __attribute__ ((packed)) config_desc_t;

// bmAttributes:
enum { USB_CONFIG_REMOTEWAKE=0x20, USB_CONFIG_SELFPOWERED=0x40,
	   USB_CONFIG_BUSPOWERED=0x80 };
// MaxPower:
#define USB_POWER( x)  ((x)>>1) /* convert mA to descriptor units of A for MaxPower */

// --- Interface Descriptor ---------------

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

// --- Endpoint  Descriptor ---------------

typedef struct {
	 DescriptorHeader;
	 __u8  bEndpointAddress;  /* 0..3 ep num, bit 7: 0 = 0ut 1= in */
	 __u8  bmAttributes;      /* 0..1 = 0: ctrl, 1: isoc, 2: bulk 3: intr */
	 __u16 wMaxPacketSize;    /* data payload size for this ep in this cfg */
	 __u8  bInterval;         /* polling interval for this ep in this cfg */
} __attribute__ ((packed)) ep_desc_t;

// bEndpointAddress:
/*
 *  bEndpointAddress[0:7]
 *  	[0:3] - The endpoint number
 *	[4:6] - Reserved, reset to zero
 *	[7]   - Direction, ignored for contol endpoint
 *		0 = OUT endpoint
 *		1 = IN endpoint
 *   shawn
 */
enum { USB_OUT=0, USB_IN=1 };



#define USB_EP_ADDRESS(a,d) (((a)&0xf) | ((d) << 7))
// bmAttributes:
enum { USB_EP_CNTRL=0, USB_EP_BULK=2, USB_EP_INT=3 };

// --- String Descriptor -------------------

typedef struct {
	 DescriptorHeader;
	 __u16 bString[1];		  /* unicode string .. actaully 'n' __u16s */
} __attribute__ ((packed)) string_desc_t;

/*=======================================================
 * Handy helpers when working with above
 *
 */
// these are x86-style 16 bit "words" ...
#define make_word_c( w ) __constant_cpu_to_le16(w)
#define make_word( w )   __cpu_to_le16(w)

// descriptor types
enum {
	USB_DESC_DEVICE=1,
	USB_DESC_CONFIG=2,
	USB_DESC_STRING=3,
	USB_DESC_INTERFACE=4,
	USB_DESC_ENDPOINT=5
};


/*=======================================================
 * Default descriptor layout for S3C UDC
 */

/* "config descriptor buffer" - that is, one config,
   ..one interface and 2 endpoints */
struct cdb {
	 config_desc_t cfg;
	 intf_desc_t   intf;
	 ep_desc_t     ep1; // DOWN stream(OUT)
	 ep_desc_t     ep2; // UP stream(IN)
} __attribute__ ((packed));


/* all S3C device descriptors */
typedef struct {
	 device_desc_t dev;   /* device descriptor */
	 struct cdb b;        /* bundle of descriptors for this cfg */
} __attribute__ ((packed)) desc_t;


/*=======================================================
 * Descriptor API
 */

/* Get the address of the statically allocated desc_t structure
   in the usb core driver. Clients can modify this between
   the time they call s3c_usb_open() and s3c_usb_start()
*/
desc_t *
s3c_usb_get_descriptor_ptr( void );


/* Set a pointer to the string descriptor at "index". The driver
 ..has room for 8 string indicies internally. Index zero holds
 ..a LANGID code and is set to US English by default. Inidices
 ..1-7 are available for use in the config descriptors as client's
 ..see fit. This pointer is assumed to be good as long as the
 ..S3C usb core is open (so statically allocate them). Returnes -EINVAL
 ..if index out of range */
int s3c_usb_set_string_descriptor( int index, string_desc_t * p );

/* reverse of above */
string_desc_t *
s3c_usb_get_string_descriptor( int index );

/* kmalloc() a string descriptor and convert "p" to unicode in it */
string_desc_t *
s3c_usb_kmalloc_string_descriptor( const char * p );

#define usbctl_next_state_on_event(var)	1

/*
 * These states correspond to those in the USB specification v1.0
 * in chapter 8, Device Framework.
 */
enum {
	USB_STATE_NOTATTACHED = 0,
	USB_STATE_ATTACHED = 1,
	USB_STATE_POWERED = 2,
	USB_STATE_DEFAULT = 3,
	USB_STATE_ADDRESS = 4,
	USB_STATE_CONFIGURED = 5,
	USB_STATE_SUSPENDED = 6
};

struct usb_stats_t {
	unsigned long ep0_fifo_write_failures;
	unsigned long ep0_bytes_written;
	unsigned long ep0_fifo_read_failures;
	unsigned long ep0_bytes_read;
};

struct usb_info_t {
	char *client_name;
	int state;
	unsigned char address;
	struct usb_stats_t stats;
};

/*
 * Function Prototypes
 */
enum {
	kError = -1,
	kEvSuspend = 0,
	kEvReset = 1,
	kEvResume = 2,
	kEvAddress = 3,
	kEvConfig = 4,
	kEvDeConfig = 5
};

#endif	/* _S3C_USBD_FS_H */
