/*
 * cpu/s3c24xx/usbd-ctl-fs.c
 *
 *  $Id: usbd-ctl-fs.c,v 1.8 2008/01/25 02:05:16 jsgood Exp $
 *
 * (C) Copyright 2007
 * Byungjae Lee, Samsung Erectronics, bjlee@samsung.com.
 *	- only support for S3C2412, S3C2443
 *
 *  S3C24x0 USB controller core driver.
 *  2004 (C) Samsung Electronics
 *          SW.LEE : Added "ud" interface to Linux (S3C2440,S3C24A0)
 *          Sree  : Added DNW interface
 *
 *  This program is derived from Extenex Corporation's SA-1100 usb
 *  controller core driver by MIZI.  Seungbum Lim <shawn@mizi.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

/*
 * ep0 - register
 * ep2~4 - dual port async. RAM (interrupt or DMA)
 *
 * config:
 *  ep0.
 *  ep2 : input - DMA_CH0 ?
 *  ep1 : output - DMA_CH3 ?
 */

#include <common.h>

#if defined(CONFIG_S3C2412) || defined(CONFIG_S3C2442)
#include <command.h>
#include <asm/errno.h>
#include "usbd-fs.h"

#include <regs.h>

#undef	USB_DEBUG
#ifdef USB_DEBUG
uint sb_debug_cnt = 0;
# define LOG(arg...) printf(arg)
#else
# define LOG(arg...)
#endif

#define EINVAL          22	/* Invalid argument */

static void initialize_descriptors(void);
static void reconfig_usbd(void);
static int s3c_usb_open( const char * client_name );
static int s3c_usb_start( void );

/*
 * set notify callback for when core reaches configured state
 * return previous pointer (if any)
 */
typedef void (*usb_notify_t)(void);
usb_notify_t s3c_set_configured_callback( usb_notify_t callback );

unsigned int s3c_usbd_dn_cnt;
unsigned int s3c_usbd_dn_addr;
int DNW;

static const char pszMe[] = "usbctl: ";
struct usb_info_t s3c_usbd_info;	/* global to ep0, usb_recv, usb_send */

static desc_t desc;		/* device descriptors */

#define MAX_STRING_DESC 8
static string_desc_t *string_desc_array[MAX_STRING_DESC];
static string_desc_t sd_zero;	/* special sd_zero holds language codes */

/* called when configured */
static usb_notify_t configured_callback = NULL;

enum {
	kStateZombie = 0,
	kStateZombieSuspend = 1,
	kStateDefault = 2,
	kStateDefaultSuspend = 3,
	kStateAddr = 4,
	kStateAddrSuspend = 5,
	kStateConfig = 6,
	kStateConfigSuspend = 7
};

//////////////////////////////////////////////////////////////////////////////
// Reset Fucntions
//////////////////////////////////////////////////////////////////////////////

void reconfig_usbd(void)
{
	LOG("\n");

	/* sec like, shawn */
	s3c_ep0_state = EP0_STATE_IDLE;
	s3c_set_configuration = 1;
	s3c_set_interface = 1;
	s3c_device_status = 0;
	s3c_ep0_status = 0;
	s3c_ep_bulk_in_status = 0;
	s3c_ep_bulk_out_status = 0;

	UD_PWR = UD_PWR_DEFAULT;
	/* EP0 */
	UD_INDEX = UD_INDEX_EP0;
	UD_MAXP = UD_MAXP_8;	// 8 byte
	UD_INDEX = UD_INDEX_EP0;
	UD_ICSR1 = EP0_CSR_SOPKTRDY | EP0_CSR_SSE;

	if (DNW) {
		UD_INDEX = UD_INDEX_EP3;
	}
	else {			/* EP2 */
		UD_INDEX = UD_INDEX_EP2;
	}

	UD_MAXP = UD_MAXP_64;	// 64 byte
	if (DNW) {
		UD_INDEX = UD_INDEX_EP3;
	}
	else {
		UD_INDEX = UD_INDEX_EP2;
	}
	UD_ICSR1 = UD_ICSR1_FFLUSH | UD_ICSR1_CLRDT;	// fifo flush, data toggle

	if (DNW) {
		UD_INDEX = UD_INDEX_EP3;
	}
	else {
		UD_INDEX = UD_INDEX_EP2;
	}

	if (DNW) {
		UD_ICSR2 = UD_ICSR2_DMAIEN;
	}
	else {
		UD_ICSR2 = UD_ICSR2_MODEIN | UD_ICSR2_DMAIEN;	/* input mode, IN_PKT_RDY dis */
	}
#ifdef USE_USBD_DMA
	UD_ICSR2 &= ~UD_ICSR2_DMAIEN;
#endif

	/* EP1 */
	UD_INDEX = UD_INDEX_EP1;
	UD_MAXP = UD_MAXP_64;	// 64 byte
	UD_INDEX = UD_INDEX_EP1;
	UD_ICSR1 = UD_ICSR1_FFLUSH | UD_ICSR1_CLRDT;	// fifo flush, data toggle
	UD_INDEX = UD_INDEX_EP1;
	UD_ICSR2 = 0x0;		// output mode
	UD_INDEX = UD_INDEX_EP1;
	UD_OCSR1 = UD_OCSR1_FFLUSH | UD_OCSR1_CLRDT;	// fifo flush
	UD_INDEX = UD_INDEX_EP1;
	UD_OCSR2 = UD_OCSR2_DMAIEN;	// OUT_PKT_RDY interrupt disable
#ifdef USE_USBD_DMA
	UD_OCSR2 &= ~UD_OCSR2_DMAIEN;	// OUT_PKT_RDY interrupt disable
#endif

	if (DNW) {
		UD_INTE = UD_INTE_EP0 | UD_INTE_EP3 | UD_INTE_EP1;
	}
	else {
		UD_INTE = UD_INTE_EP0 | UD_INTE_EP2 | UD_INTE_EP1;
	}
	UD_USBINTE = UD_USBINTE_RESET | UD_USBINTE_SUSPND;

	initialize_descriptors();
}


void s3c_udc_int_hndlr (void)
{
	u8 saveIdx = UD_INDEX;
	u8 usb_status = UD_USBINT;
	u8 usbd_status = UD_INT;

	LOG("usb_status = 0x%02x, usbd_status = 0x%02x\n", usb_status, usbd_status);

	if (usb_status & UD_USBINT_RESET) {
		LOG("[%d]RESET interrupt\n", sb_debug_cnt++);
		LOG("usbd reset\n");

		s3c_ep0_reset();
		s3c_ep1_reset();	/* output */
		s3c_ep2_reset();	/* input */

		reconfig_usbd();
		UD_USBINT = UD_USBINT_RESET;	//RESET_INT should be cleared after reconfig_usbd().- by samsung src
		s3c_ep0_state = EP0_STATE_IDLE;

	}


	/* RESume Interrupt Request */
	if (usb_status & UD_USBINT_RESUM) {
		LOG("[%d]RESUME interrupt\n", sb_debug_cnt++);
		UD_USBINT = UD_USBINT_RESUM;	/* clear */
	}


	/* SUSpend Interrupt Request */
	if (usb_status & UD_USBINT_SUSPND) {
		LOG("[%d]SUSPEND interrupt\n", sb_debug_cnt++);
		UD_USBINT = UD_USBINT_SUSPND;	/* clear */
	}

	if (usbd_status & UD_INT_EP0) {
		LOG("[%d]EP0 interrupt\n", sb_debug_cnt++);
		UD_INT = UD_INT_EP0;	/* clear */
		s3c_ep0_int_hndlr();

	}

	/* output */
	if (usbd_status & UD_INT_EP1) {
		LOG("[%d]EP1 interrupt\n", sb_debug_cnt++);
		UD_INT = UD_INT_EP1;	/* clear */
		if (DNW) {
			/* tell me reason ? */
			s3c_ep2_int_hndlr(usbd_status);
		}
		else {
			s3c_ep1_int_hndlr(usbd_status);
		}

	}

	if (DNW) {
		/* input */
		if (usbd_status & UD_INT_EP3) {
			LOG("[%d]EP2 interrupt\n", sb_debug_cnt++);
			UD_INT = UD_INT_EP3;	/* clear */
			s3c_ep1_int_hndlr(usbd_status);

		}
	}
	else {
		/* input */
		if (usbd_status & UD_INT_EP2) {
			LOG("[%d]EP2 interrupt\n", sb_debug_cnt++);
			UD_INT = UD_INT_EP2;	/* clear */
			s3c_ep2_int_hndlr(usbd_status);
		}
	}

	if (usbd_status & UD_INT_EP3)
		UD_INT = UD_INT_EP3;
	if (usbd_status & UD_INT_EP4)
		UD_INT = UD_INT_EP4;

	UD_INDEX = saveIdx;
	LOG("end of usb irq\n\n\n");
}

//////////////////////////////////////////////////////////////////////////////
// Public Interface
//////////////////////////////////////////////////////////////////////////////

/* Open S3C24x0 usb core on behalf of a client, but don't start running */

int s3c_usb_open(const char *client)
{

	LOG("\n");

	if (s3c_usbd_info.client_name != NULL)
		return -EBUSY;

	s3c_usbd_info.client_name = (char *) client;
	memset(&s3c_usbd_info.stats, 0, sizeof(struct usb_stats_t));
	memset(string_desc_array, 0, sizeof(string_desc_array));

	/* create descriptors for enumeration */
	initialize_descriptors();

	LOG("%sOpened for %s\n", pszMe, client);
	return 0;
}

/* Start running. Must have called usb_open (above) first */
int s3c_usb_start(void)
{
	unsigned long tmp;

	LOG("\n");

	if (s3c_usbd_info.client_name == NULL) {
		printf("%s%s - no client registered\n", pszMe, __FUNCTION__);
		return -EPERM;
	}

	/* start UDC internal machinery running */
	udelay(100);

	/* clear stall - receiver seems to start stalled? */
	if (DNW) {
		UD_INDEX = UD_INDEX_EP3;
	}
	else {
		UD_INDEX = UD_INDEX_EP2;	// EP2 input
	}
	tmp = UD_ICSR1;
	tmp &= ~(UD_ICSR1_SENTSTL | UD_ICSR1_FFLUSH | UD_ICSR1_UNDRUN);
	tmp &= ~(UD_ICSR1_PKTRDY | UD_ICSR1_SENDSTL);
	UD_ICSR1 = tmp;

	UD_INDEX = UD_INDEX_EP1;	// EP1 output
	tmp = UD_OCSR1;
	tmp &= ~(UD_OCSR1_SENTSTL | UD_OCSR1_FFLUSH | UD_OCSR1_OVRRUN);
	tmp &= ~(UD_OCSR1_PKTRDY | UD_OCSR1_SENDSTL);
	UD_OCSR1 = tmp;

	/* clear all top-level sources */
	if (DNW) {
		UD_INT = UD_INT_EP0 | UD_INT_EP1 | UD_INT_EP3;
	}
	else {
		UD_INT = UD_INT_EP0 | UD_INT_EP1 | UD_INT_EP2;
	}
	UD_USBINT = UD_USBINT_RESET | UD_USBINT_RESUM | UD_USBINT_SUSPND;

	LOG("%sStarted for %s\n", pszMe, s3c_usbd_info.client_name);
	s3c_usbd_dn_cnt = 0;
	printf("USB Device started for download, counter=%d.\n", s3c_usbd_dn_cnt);
	return 0;
}

/* Stop USB core from running */
int s3c_usb_stop (void)
{
	LOG("name=%s\n", s3c_usbd_info.client_name ? s3c_usbd_info.client_name : "NULL");
	if (s3c_usbd_info.client_name == NULL) {
		printf("%s%s - no client registered\n", pszMe, __FUNCTION__);
		return -EPERM;
	}

	s3c_ep1_reset();
	s3c_ep2_reset();
	
#if defined(CONFIG_SMDK2442)
	GPGDAT_REG &= ~(1<<12); /* D+ pull-down */
#elif defined(CONFIG_SP3000)
	GPBDAT_REG |= (1<<4);
#endif	
	return 0;
}

/* set a proc to be called when device is configured */
usb_notify_t s3c_set_configured_callback(usb_notify_t func)
{
	usb_notify_t retval = configured_callback;

	LOG("\n");

	configured_callback = func;
	return retval;
}

/*====================================================
 * Descriptor Manipulation.
 * Use these between open() and start() above to setup
 * the descriptors for your device.
 *
 */

/* get pointer to static default descriptor */
desc_t *s3c_usb_get_descriptor_ptr(void)
{
	return &desc;
}

/* optional: set a string descriptor */
int s3c_usb_set_string_descriptor(int i, string_desc_t * p)
{
	int retval;

	LOG("\n");

	if (i < MAX_STRING_DESC) {
		string_desc_array[i] = p;
		retval = 0;
	}
	else {
		retval = -EINVAL;
	}
	return retval;
}

/* optional: get a previously set string descriptor */
string_desc_t *s3c_usb_get_string_descriptor(int i)
{
	LOG("\n");
	return (i < MAX_STRING_DESC) ? string_desc_array[i] : NULL;
}


/* optional: kmalloc and unicode up a string descriptor */
string_desc_t *s3c_usb_kmalloc_string_descriptor(const char *p)
{
	string_desc_t *pResult = NULL;

	LOG("\n");

	if (p) {
		int len = strlen(p);
		int uni_len = len * sizeof(__u16);
		// pResult = (string_desc_t*) kmalloc( uni_len + 2, GFP_KERNEL ); /* ugh! */
		if (pResult != NULL) {
			int i;
			pResult->bLength = uni_len + 2;
			pResult->bDescriptorType = USB_DESC_STRING;
			for (i = 0; i < len; i++) {
				pResult->bString[i] = make_word((__u16) p[i]);
			}
		}
	}
	return pResult;
}

//////////////////////////////////////////////////////////////////////////////
// Private Helpers
//////////////////////////////////////////////////////////////////////////////

//* setup default descriptors */

static void initialize_descriptors(void)
{
	LOG("initialize_descriptors\n");

	desc.dev.bLength = sizeof(device_desc_t);
	desc.dev.bDescriptorType = USB_DESC_DEVICE;
	desc.dev.bcdUSB = 0x110;	/* 1.1 */
	desc.dev.bDeviceClass = 0xFF;	/* vendor specific */
	desc.dev.bDeviceSubClass = 0x0;
	desc.dev.bDeviceProtocol = 0x0;
	desc.dev.bMaxPacketSize0 = 0x8;	/* ep0 max fifo size in s3c24x0 */

#if defined(CONFIG_S3C2442)
	desc.dev.idVendor = 0x49f;	/* vendor ID undefined */
	desc.dev.idProduct = 0x505a;	/* product */
	desc.dev.bcdDevice = 0x01;	/* vendor assigned device release num */
#else
	desc.dev.idVendor = 0x4e8;	/* vendor ID undefined */
	desc.dev.idProduct = 0x1234;	/* product */
	desc.dev.bcdDevice = 0x0100;	/* vendor assigned device release num */
#endif
	desc.dev.iManufacturer = 1;	/* index of manufacturer string */
	desc.dev.iProduct = 2;	/* index of product description string */
	desc.dev.iSerialNumber = 0;	/* index of string holding product s/n */
	desc.dev.bNumConfigurations = 0x1;

	desc.b.cfg.bLength = sizeof(config_desc_t);
	desc.b.cfg.bDescriptorType = USB_DESC_CONFIG;
	desc.b.cfg.wTotalLength = make_word_c(sizeof(struct cdb));
	desc.b.cfg.bNumInterfaces = 1;
	desc.b.cfg.bConfigurationValue = 1;
	desc.b.cfg.iConfiguration = 0;
	desc.b.cfg.bmAttributes = USB_CONFIG_BUSPOWERED;
	desc.b.cfg.MaxPower = USB_POWER(500);

	desc.b.intf.bLength = sizeof(intf_desc_t);
	desc.b.intf.bDescriptorType = USB_DESC_INTERFACE;
	desc.b.intf.bInterfaceNumber = 0x0;	/* unique intf index */
	desc.b.intf.bAlternateSetting = 0x0;
	desc.b.intf.bNumEndpoints = 2;	/* endpoint number excluding ep0 */
	desc.b.intf.bInterfaceClass = 0xFF;	/* vendor specific */
	desc.b.intf.bInterfaceSubClass = 0x0;
	desc.b.intf.bInterfaceProtocol = 0x0;
	desc.b.intf.iInterface = 0x0;

	desc.b.ep1.bLength = sizeof(ep_desc_t);
	desc.b.ep1.bDescriptorType = USB_DESC_ENDPOINT;
	if (DNW) {
		desc.b.ep1.bEndpointAddress = USB_EP_ADDRESS(1, USB_IN);
	}
	else {
		desc.b.ep1.bEndpointAddress = USB_EP_ADDRESS(1, USB_OUT);
	}

	desc.b.ep1.bmAttributes = USB_EP_BULK;
	desc.b.ep1.wMaxPacketSize = make_word_c(64);
	desc.b.ep1.bInterval = 0x0;

	desc.b.ep2.bLength = sizeof(ep_desc_t);
	desc.b.ep2.bDescriptorType = USB_DESC_ENDPOINT;
	if (DNW) {
		desc.b.ep2.bEndpointAddress = USB_EP_ADDRESS(3, USB_OUT);
	}
	else {
		desc.b.ep2.bEndpointAddress = USB_EP_ADDRESS(2, USB_IN);
	}
	desc.b.ep2.bmAttributes = USB_EP_BULK;
	desc.b.ep2.wMaxPacketSize = make_word_c(64);
	desc.b.ep2.bInterval = 0x0;

	/* set language */
	/* See: http://www.usb.org/developers/data/USB_LANGIDs.pdf */
	sd_zero.bDescriptorType = USB_DESC_STRING;
	sd_zero.bLength = sizeof(string_desc_t);
	sd_zero.bString[0] = make_word_c(0x409);	/* American English */
	s3c_usb_set_string_descriptor(0, &sd_zero);
}

//////////////////////////////////////////////////////////////////////////////
// Module Initialization and Shutdown
//////////////////////////////////////////////////////////////////////////////
/*
 * s3c_usbctl_init()
 * Module load time. Allocate dma and interrupt resources. Setup /proc fs
 * entry. Leave UDC disabled.
 */
#if defined(CONFIG_S3C2412)
int s3c_usbctl_init(void)
{
	memset(&s3c_usbd_info, 0, sizeof(s3c_usbd_info));

	/* USB Device detection Control */
	GPFCON_REG &= ~(3<<4);
	GPFCON_REG |= (1<<4);	/* Output */
	GPFPU_REG  |= (1<<2);	/* Pull down disable */
	GPFDAT_REG |= (1<<2);	/* D+ Pull up */

	/* USB port1 setting */
	MISCCR_REG &= ~(1<<3);	/* select USBD */
	MISCCR_REG &= ~(1<<13);	/* enable USB port */

	/* USB clock setting */
	CLKCON_REG |= CLKCON_USBD;	/*enable pclk into USBD i/f block */
	UPLLCON_REG = (0<<20)|(64<<12)|(7<<4)|0;/* 96MHz UPLL */
	CLKDIVN_REG|= (1<<6);			/* USB 48MHz Div on */
	CLKSRC_REG |= SelUPLL;			/* USYSCLK = FOUTupll */
	
	LOG("S3C24x0 USB Controller Core Initialized");
	s3c_usbd_dn_cnt = 0;
	return 0;
}
#elif defined(CONFIG_S3C2442)
int s3c_usbctl_init(void)
{
#if defined(CONFIG_SMDK2442)
	/* USB Device Part*/
	/* GPGCON is reset for USB device */
	GPGCON_REG = (GPGCON_REG & ~(3<<24)) | (1<<24); /* Output Mode */
	GPGPU_REG = GPGPU_REG | (1<<12); /* Pull up disable */
	GPGDAT_REG |= (1<<12); /* D+ pull-up */
#elif defined(CONFIG_SP3000)
	GPBDAT_REG &= ~(1<<4);	/* D+ control */
#endif
	memset(&s3c_usbd_info, 0, sizeof(s3c_usbd_info));
	/* USB port1 setting */
	MISCCR_REG &= ~(1<<3);	/* select USBD */
	MISCCR_REG &= ~(1<<13);	/* enable USB port */

	/* USB clock setting */
	CLKCON_REG |= CLKCON_USBD;	/*enable pclk into USBD i/f block */
	s3c_usbd_dn_cnt = 0;

	return 0;	
}
#else
#error "* CFG_ERROR : you have to setup right Samsung CPU configuration"
#endif

/* s3c_twiddle_descriptors()
 * It is between open() and start(). Setup descriptors.
 */
void s3c_twiddle_descriptors( void )
{
         desc_t * pDesc = s3c_usb_get_descriptor_ptr();

         pDesc->b.ep1.wMaxPacketSize = make_word_c( RX_PACKET_SIZE );
         pDesc->b.ep1.bmAttributes   = USB_EP_BULK;
         pDesc->b.ep2.wMaxPacketSize = make_word_c( TX_PACKET_SIZE );
         pDesc->b.ep2.bmAttributes   = USB_EP_BULK;
}


int s3c_usbc_activate(void)
{
        int retval = 0;

        /* start usb core */
        retval = s3c_usb_open( "usb-char" );

        s3c_twiddle_descriptors();

        retval = s3c_usb_start();
        if ( retval ) {
                LOG( "%sAGHH! Could not start USB core\n", pszMe );
                //free_txrx_buffers();
                return retval;
        }

        return 0;
}

#endif /* #ifdef CONFIG_S3C2412 */
