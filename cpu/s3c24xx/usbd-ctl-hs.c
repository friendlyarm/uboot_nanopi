/*
 * cpu/s3c24xx/usbd-ctl-hs.c
 *
 *  $Id: usbd-ctl-hs.c,v 1.4 2008/03/31 23:23:32 boyko Exp $
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

#include <common.h>

#if defined(CONFIG_S3C2443) || defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
#include <command.h>
#include <asm/errno.h>
#include <regs.h>
#include "usbd-hs.h"

#define USB_CHECKSUM_EN
#undef	USB_DEBUG

#ifdef USB_DEBUG
uint sb_debug_cnt = 0;
#define LOG(arg...) printf(arg)
#else
#define LOG(arg...)
#endif

/* Select Data transfer mode */
#if 1
#define USB_OP_MODE USB_CPU_MODE
#else
#define USB_OP_MODE USB_DMA_MODE
#endif

#define EINVAL          22	/* Invalid argument */
#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})
#define USB_CPU_MODE 0
#define USB_DMA_MODE 1

__u32 s3c_ep0_state;
__u32 s3c_ep0_sub_state;
__u32 s3c_usbd_dn_addr;
__u32 ep0_maxpktsize;
__u32 ep1_maxpktsize;
__u32 ep3_maxpktsize;
__u32 udc_speed;
__u32 download_filesize;
__u32 download_addr;
__u16 *download_ptr;
__u32 upload_size;
__u16 *upload_ptr;
__u32 remote_wakeup;
__u16 config_value;

int DNW;
int s3c_receive_done = 0;
int s3c_got_header = 0;
static int op_mode;

/* Device Qualifier */
const __u8 device_qualifier[] =
{
	0x0a,	/* 0 desc size */
	0x06,	/* 1 desc type (DEVICE) */
	0x00,	/* 2 USB release */
	0x02,	/* 3 => 2.00 */
	0x00,	/* 4 class */
	0x00,	/* 5 subclass */
	0x00,	/* 6 protocol */
	64,	/* 7 max pack size */
	0x01,	/* 8 number of other-speed configuration */
	0x00,	/* 9 reserved */
};

 /* codes representing languages */
const __u8 aDescStr0[]=
{
	4, USB_DESC_STRING, USB_LANGID_US_L, USB_LANGID_US_H,
};

const __u8 desc_string1[]= /* Manufacturer */
{
	(0x24+2), USB_DESC_STRING,
	'S', 0x0, 'a', 0x0, 'm', 0x0, 's', 0x0, 'u', 0x0,
	'n', 0x0, 'g', 0x0, ' ', 0x0, 'S', 0x0, 'y', 0x0,
	's', 0x0, 't', 0x0, 'e', 0x0, 'm', 0x0, ' ', 0x0,
	'L', 0x0, 'S', 0x0, 'I', 0x0
};

const __u8 desc_string2[]= /* Product */
{
	(0x2a+2), USB_DESC_STRING,
	'S', 0x0, 'E', 0x0, 'C', 0x0, ' ', 0x0, 'S', 0x0,
	'3', 0x0, 'C', 0x0, ' ', 0x0, 'S', 0x0, 'M', 0x0,
	'D', 0x0, 'K', 0x0, ' ', 0x0, 'E', 0x0, 'v', 0x0,
	'a', 0x0, 'l', 0x0, ' ', 0x0, 'B', 0x0, '/', 0x0,
	'D', 0x0
};

/* device descriptors */
static desc_t desc;
static device_req_t	*dev_req;
static get_status_t	*get_status;
static get_intf_t	*get_intf;


void int_hndlr(void);
static void initialize_descriptors(void);
void transfer_ep0(void);

int s3c_usbc_activate (void)
{
	/* dont used in usb high speed, but used in common file cmd_usbd.c  */
	return 0;
}

int s3c_usb_stop (void)
{
	/*usb power disable */
#if defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
	GPHPU_REG = (GPHPU_REG&~(3<<28))|(1<<28); /* pull-dwon enable */
	GPFPU_REG = (GPFPU_REG&~(3<<4))|(1<<4); /* vbus detect pull-down enable */
#else
	GPHPU_REG = (GPHPU_REG&~(3<<28))|(2<<28); /* pull-dwon enable */
#endif
	GPHCON_REG = (GPHCON_REG&~(3<<28))|(1<<28); /* output */
	GPHDAT_REG &= ~(1<<14); /* output */

	USB_CLKCON_REG = 0;	/* usb clock disbale */
	MISCCR_REG != (1<<12);	/* USB Port is suspend mode */
	PWRCFG_REG &= ~(1<<4);	/* phy power disable */
	return 0;
}

void set_endpoint(void)
{
	/* EP0: control */
	UD_INDEX_REG		= UD_INDEX_EP0;
	UD_EP_DIR_REG		= 0x02; /* EP1=> TX, EP3=>RX */

	/* EP0~3 Interrupt enable, why set 12b~19b at F/W? */
	UD_EP_INT_EN_REG	= 0x4d0f;

	LOG("EP_DIR_REG : 0x%x, EP_INT_EN_REG : 0x%x \n",
		UD_EP_DIR_REG, UD_EP_INT_EN_REG);

	UD_TEST_REG = 0x0000;

	/* error interrupt enable, 16bit bus, Little format,
	 * suspend&reset enable
	 */
	UD_SYS_CON_REG = UD_RRD_EN | UD_SUS_EN | UD_RST_EN;
	UD_EP0_CON_REG = 0x0000;


	/* EP1 dual FIFO mode enable */
	UD_INDEX_REG = UD_INDEX_EP1;
	UD_EP_CON_REG = 0x0080;

	/* EP3 dual FIFO mode enable */
	UD_INDEX_REG = UD_INDEX_EP3;
	UD_EP_CON_REG = 0x0080;

	UD_INDEX_REG = UD_INDEX_EP0;


}

#ifdef USB_CHECKSUM_EN
int verify_checksum(void)
{
	__u8 *checksum_ptr;
	__u16 checksum, down_checksum;

	checksum_ptr = (__u8 *)download_addr;
	checksum = 0;

	/* checksum calculation,
	 * download_filesize : addr(4)+size(4)+data(n)+cs(2)
	 */
	printf("Now, Checksum calculation");
	while((__u32)checksum_ptr < (download_addr+(download_filesize-8-2))){
		checksum += *checksum_ptr++;
		if(((__u32)checksum_ptr&0xfffff)==0) printf(".");
	}
	down_checksum = *((__u16 *)(download_addr+download_filesize-8-2));

	if (checksum!=down_checksum) {
		printf("\nChecksum Error !!! MEM:0x%x DN:0x%x\n",
			checksum, down_checksum);
		return FALSE;
	} else {
		printf("\nChecksum O.K !!!\n");
		return TRUE;
	}

}
#else /* don't use verify_checksum() */
int verify_checksum(void)
{
	return 0;
}
#endif

void write_ep0buf(__u16 *buf, int num)
{
	int i;
	__u16 data = 0;

	for(i=0;i<num;i+=2) {
		data = *buf;
		UD_EP0_BUF_REG = data;
		buf++;
	}
}

void write_ep0buf_1byte(__u8 *buf)
{
	__u8 data = 0;

	data = *buf;
	UD_EP0_BUF_REG = data;
}

void write_epbuf(__u16 *buf, int num)
{
	int i;
	__u16 write_data=0;

	for(i=0;i<num;i+=2) {
		write_data = *buf;
		UD_EP1_BUF_REG = write_data;
		buf++;
	}
	upload_ptr += (num>>1);
}

void read_epbuf(__u16 *buf, int num)
{
	int i;
	__u16 read_data;

	for(i=0;i<num;i+=2) {
		read_data = (__u16)UD_EP3_BUF_REG;
		*buf = read_data;
		buf++;
	}
	download_ptr += (num>>1);
}

void upload_start(void)
{

	LOG("upload_size:0x%x, upload_ptr:0x%x\n", upload_size, upload_ptr);
	if (upload_size > ep1_maxpktsize) {
		UD_INDEX_REG = UD_INDEX_EP1;
		UD_BYTE_WRITE_CNT_REG = ep1_maxpktsize;
		write_epbuf(upload_ptr, ep1_maxpktsize);
		upload_size -= ep1_maxpktsize;
	} else {
		UD_INDEX_REG = UD_INDEX_EP1;
		UD_BYTE_WRITE_CNT_REG = upload_size;
		write_epbuf(upload_ptr, upload_size);
		upload_size = 0;
	}


}

void set_max_pktsize(__u32 speed)
{
	if (speed == USB_SPEED_HIGH) {
		udc_speed = USB_SPEED_HIGH;
		ep0_maxpktsize = 64;
		ep1_maxpktsize = 512;
		ep3_maxpktsize = 512;
	} else {
		udc_speed = USB_SPEED_FULL;
		ep0_maxpktsize = 64;
		ep1_maxpktsize = 64;
		ep3_maxpktsize = 64;
	}

	/* EP0 Max Packet size settings */
	UD_INDEX_REG = UD_INDEX_EP0;
	UD_MAX_PKT_REG = ep0_maxpktsize;

	/* EP1 Max Packet size settings */
	UD_INDEX_REG = UD_INDEX_EP1;
	UD_MAX_PKT_REG = ep1_maxpktsize;

	/* EP3 Max Packet size settings */
	UD_INDEX_REG = UD_INDEX_EP3;
	UD_MAX_PKT_REG = ep3_maxpktsize;
}

void get_desc(void)
{
	__u16 device_req_length;
	__u8  desc_type, desc_index;

	desc_type  = (__u8)(dev_req->wValue >> 8);
	desc_index = (__u8)(dev_req->wValue & 0xff);

	switch (desc_type) {
	case USB_DESC_DEVICE:
		LOG("MCU >> Get Device Descriptor \n");
		s3c_ep0_state = USB_GET_DESC_DEVICE;
		break;

	case USB_DESC_CONFIG:
		LOG("MCU >> Get Configuration Descriptor \n");

		device_req_length = dev_req->wLength;

		if (device_req_length > USB_DESC_CONFIG_SIZE){
			s3c_ep0_state = USB_GET_DESC_CFG;
		} else
			s3c_ep0_state = USB_GET_DESC_CFG_ONLY;
		break;

	case USB_DESC_STRING:
		switch(desc_index) {
		case 0:
			s3c_ep0_state = USB_GET_DESC_STR0;
			break;
		case 1:
			s3c_ep0_state = USB_GET_DESC_STR1;
			break;
		case 2:
			s3c_ep0_state = USB_GET_DESC_STR2;
			break;
		default:
			break;
		}
		break;
	case USB_DESC_ENDPOINT:
		switch(desc_index & 0xf) {
		case 0:
			s3c_ep0_state = USB_GET_DESC_EP1;
			break;
		case 1:
			s3c_ep0_state = USB_GET_DESC_EP3;
			break;
		default:
			break;
		}
		break;

	case USB_DESC_DEVICE_QUALIFIER:
		LOG("MCU >> Get Device Qualifier Descriptor \n");
		s3c_ep0_state = USB_GET_DESC_QUALIFIER;
		break;
	}
}

void clear_feature(void)
{
	LOG("MCU >> Clear Feature \n");
	switch (dev_req->bmRequestType) {
	case USB_RECIPIENT_DEVICE:
		if (dev_req->wValue == USB_DEV_REMOTE_WAKEUP)
			remote_wakeup = FALSE;
		break;

	case USB_RECIPIENT_ENDPOINT:
		if (dev_req->wValue == USB_ENDPOINT_HALT) {
			if ((dev_req->wIndex & 0x7f) == 0x00)
				get_status->Endpoint0 = 0;

			if ((dev_req->wIndex & 0x8f) == 0x01)
				get_status->Endpoint1 = 0;

			if ((dev_req->wIndex & 0x8f) == 0x03)
				get_status->Endpoint3 = 0;
		}
		break;

	default:
		break;
	}
	s3c_ep0_state = EP0_STATE_IDLE;
}

void set_feature(void)
{
	LOG("MCU >> Set Feature \n");
	switch (dev_req->bmRequestType) {
	case USB_RECIPIENT_DEVICE:
		if (dev_req->wValue == USB_DEV_REMOTE_WAKEUP)
			remote_wakeup = TRUE;
			break;

	case USB_RECIPIENT_ENDPOINT:
		if (dev_req->wValue == USB_ENDPOINT_HALT) {
			if ((dev_req->wIndex & 0x7f) == 0x00)
				get_status->Endpoint0 = 1;

			if ((dev_req->wIndex & 0x8f) == 0x01)
				get_status->Endpoint1 = 1;

			if ((dev_req->wIndex & 0x8f) == 0x03)
				get_status->Endpoint3 = 1;
		}
		break;

	default:
		break;
	}
	s3c_ep0_state = EP0_STATE_IDLE;
}

void usb_get_status(void)
{
	LOG("MCU >> Get Status \n");
	switch(dev_req->bmRequestType) {
	case  (0x80):
		/* SelfPowered */
		get_status->Device=((__u8)remote_wakeup<<1)|0x1;
		s3c_ep0_state = USB_GET_STATUS_DEVICE;
		break;

	case  (0x81):
		get_status->Interface=0;
		s3c_ep0_state = USB_GET_STATUS_INTF;
		break;

	case  (0x82):
		if ((dev_req->wIndex & 0x7f) == 0x00)
			s3c_ep0_state = USB_GET_STATUS_EP0;

		if ((dev_req->wIndex & 0x8f) == 0x01)
			s3c_ep0_state = USB_GET_STATUS_EP1;

		if ((dev_req->wIndex & 0x8f) == 0x03)
			s3c_ep0_state = USB_GET_STATUS_EP3;
		break;

	default:
		break;
	}
}

/*
 * This is function for Control Transfer.
 * If it occurs endpoint0 interrupt, this function is called.
 * This function check Device Request for Control Transfer type and
 * call each other functions.
 */
void s3c_ep0_int_hndlr(void)
{
	__u32 ep0sr;
	__u16 read_cnt, i;
	__u16 read_buf[4]={0x0000, };
	__u8 set_addr;

	UD_INDEX_REG = UD_INDEX_EP0;
	ep0sr = UD_EP0_STATUS_REG;

	LOG(" EP0 Status Register = %x \n", s3c_ep0_state);

	/* EP0 status register check */
	if (ep0sr & UD_EP0_SENT_STALL) {
		LOG(" Sent Stall \n");
		UD_EP0_STATUS_REG = UD_EP0_SENT_STALL;

		if (ep0sr & UD_EP0_RX_SUCCESS)
			UD_EP0_STATUS_REG = UD_EP0_RX_SUCCESS;

		s3c_ep0_state= EP0_STATE_IDLE;
		return;
	}

	if (ep0sr & UD_EP0_TX_SUCCESS) {
		LOG(" EP0_TX_SUCCESS \n");
		UD_EP0_STATUS_REG = UD_EP0_TX_SUCCESS;
	}

	LOG(" s3c_ep0_state = %x \n", s3c_ep0_state);

	if (s3c_ep0_state == EP0_STATE_IDLE) {
		read_cnt = UD_BYTE_READ_CNT_REG;

		for(i=0;i<4;i++)
			read_buf[i] = (__u16)UD_EP0_BUF_REG;

		UD_EP0_STATUS_REG = UD_EP0_RX_SUCCESS;

		dev_req->bmRequestType	= (__u8)read_buf[0];
		dev_req->bRequest	= (__u8)(read_buf[0]>>8);
		dev_req->wValue		= read_buf[1];
		dev_req->wIndex		= read_buf[2];
		dev_req->wLength	= read_buf[3];

		switch (dev_req->bRequest) {
		case USB_REQ_SET_ADDRESS:
			/* Set Address Update bit */
			set_addr = (__u8)(dev_req->wValue & 0xff);
			LOG("Func_ADDR_Setaddr : %d \n", set_addr);
			s3c_ep0_state = EP0_STATE_IDLE;
			break;

		case USB_REQ_SET_DESCRIPTOR:
			LOG("Set Descriptor \n");
			break;

		case USB_REQ_SET_CONFIGURATION:
			LOG("Set Configuration \n");
			config_value = dev_req->wValue;
			break;

		case USB_REQ_GET_CONFIGURATION:
			LOG("Get Configruation \n");
			UD_BYTE_WRITE_CNT_REG = 1;
			UD_EP0_BUF_REG = config_value;
			break;

		case USB_REQ_GET_DESCRIPTOR:
			LOG("Get Descriptor \n");
			get_desc();
			break;

		case USB_REQ_CLEAR_FEATURE:
			LOG("Clear Feature \n");
			clear_feature();
			break;

		case USB_REQ_SET_FEATURE:
			LOG("Set Feature \n");
			set_feature();
			break;

		case USB_REQ_GET_STATUS:
			LOG("Get Status \n");
			usb_get_status();
			break;

		case USB_REQ_GET_INTERFACE:
			LOG("Get Interface \n");
			s3c_ep0_state = USB_GET_INTERFACE;
			break;

		case USB_REQ_SET_INTERFACE:
			LOG("Set Interface \n");
			get_intf->AlternateSetting = (__u8)dev_req->wValue;
			s3c_ep0_state = EP0_STATE_IDLE;
			break;

		case USB_REQ_SYNCH_FRAME:
			LOG("Synch Frame \n");
			s3c_ep0_state = EP0_STATE_IDLE;
			break;

		default:
			break;
		}
	}

	transfer_ep0();

}

void transfer_ep0(void)
{
	switch (s3c_ep0_state) {
	case EP0_STATE_IDLE:
		break;

	case USB_GET_DESC_DEVICE:
		UD_BYTE_WRITE_CNT_REG = 18;
		write_ep0buf((__u16 *)&desc.dev, 18);
		s3c_ep0_state = EP0_STATE_IDLE;
		LOG("EndpointZeroTransfer(EP0_STATE_GD_DEV)\n");
		break;

	/* GET_DESCRIPTOR:CONFIGURATION+INTERFACE+ENDPOINT0+ENDPOINT1 */
	case USB_GET_DESC_CFG:
		UD_BYTE_WRITE_CNT_REG = 32;
		write_ep0buf((__u16 *)&desc.b.cfg, 32);
		s3c_ep0_state = EP0_STATE_IDLE;
		LOG("EndpointZeroTransfer(EP0_STATE_GD_CFG)\n");
		break;

	/* GET_DESCRIPTOR:CONFIGURATION ONLY */
	case USB_GET_DESC_CFG_ONLY:
		LOG("EP0_STATE_GD_CFG_ONLY\n");
		UD_BYTE_WRITE_CNT_REG = 9;
		write_ep0buf((__u16 *)&desc.b.cfg, 9);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_DESC_INTERFACE:
		UD_BYTE_WRITE_CNT_REG = 9;
		write_ep0buf((__u16 *)&desc.b.intf, 9);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_DESC_EP1:
		UD_BYTE_WRITE_CNT_REG = USB_DESC_ENDPOINT_SIZE;
		write_ep0buf((__u16 *)&desc.b.ep1, USB_DESC_ENDPOINT_SIZE);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_DESC_EP3:
		UD_BYTE_WRITE_CNT_REG = USB_DESC_ENDPOINT_SIZE;
		write_ep0buf((__u16 *)&desc.b.ep3, USB_DESC_ENDPOINT_SIZE);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_DESC_STR0:
		UD_BYTE_WRITE_CNT_REG = 4;
		LOG("[GDS0_0]");
		write_ep0buf((__u16 *)aDescStr0, 4);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_DESC_STR1:
		LOG("[GDS1_%d]", s3c_ep0_sub_state);
		if ((s3c_ep0_sub_state*ep0_maxpktsize+ep0_maxpktsize)
		    < sizeof(desc_string1)) {

			UD_BYTE_WRITE_CNT_REG = ep0_maxpktsize;

			write_ep0buf((__u16 *)desc_string1
				+ (s3c_ep0_sub_state*ep0_maxpktsize),
				ep0_maxpktsize);

			s3c_ep0_state = USB_GET_DESC_STR1;
			s3c_ep0_sub_state++;

		} else {
			UD_BYTE_WRITE_CNT_REG =
				sizeof(desc_string1)
				- (s3c_ep0_sub_state*ep0_maxpktsize);

			write_ep0buf((__u16 *)desc_string1
				+ (s3c_ep0_sub_state*ep0_maxpktsize),
				sizeof(desc_string1)
				- (s3c_ep0_sub_state*ep0_maxpktsize));

			s3c_ep0_state = EP0_STATE_IDLE;
			s3c_ep0_sub_state = 0;
		}
		break;

	case USB_GET_DESC_STR2:
		LOG("[GDS2_%d]", s3c_ep0_sub_state);
		if ((s3c_ep0_sub_state*ep0_maxpktsize+ep0_maxpktsize)
		    < sizeof(desc_string2)) {

			UD_BYTE_WRITE_CNT_REG = ep0_maxpktsize;

			write_ep0buf((__u16 *)desc_string2
				+ (s3c_ep0_sub_state*ep0_maxpktsize),
				ep0_maxpktsize);

			s3c_ep0_state = USB_GET_DESC_STR2;
			s3c_ep0_sub_state++;
		} else {
			LOG("[E]");
			UD_BYTE_WRITE_CNT_REG =
				sizeof(desc_string2)
				- (s3c_ep0_sub_state*ep0_maxpktsize);

			write_ep0buf((__u16 *)desc_string2
				+ (s3c_ep0_sub_state*ep0_maxpktsize),
				sizeof(desc_string2)
				- (s3c_ep0_sub_state*ep0_maxpktsize));

			s3c_ep0_state = EP0_STATE_IDLE;
			s3c_ep0_sub_state = 0;
		}
		break;

	case USB_GET_DESC_QUALIFIER:
		UD_BYTE_WRITE_CNT_REG = 10;
		write_ep0buf((__u16 *)device_qualifier, 10);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_INTERFACE:
		UD_BYTE_WRITE_CNT_REG = 1;
		write_ep0buf_1byte((__u8 *)get_intf);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;


	case USB_GET_STATUS_DEVICE:
		UD_BYTE_WRITE_CNT_REG = 1;
		write_ep0buf_1byte((__u8 *)get_status);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_STATUS_INTF:
		UD_BYTE_WRITE_CNT_REG = 1;
		write_ep0buf_1byte((__u8 *)get_status+1);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_STATUS_EP0:
		UD_BYTE_WRITE_CNT_REG = 1;
		write_ep0buf_1byte((__u8 *)get_status+2);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_STATUS_EP1:
		UD_BYTE_WRITE_CNT_REG = 1;
		write_ep0buf_1byte((__u8 *)get_status+3);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	case USB_GET_STATUS_EP3:
		UD_BYTE_WRITE_CNT_REG = 1;
		write_ep0buf_1byte((__u8 *)get_status+4);
		s3c_ep0_state = EP0_STATE_IDLE;
		break;

	default:
		break;

	}
}

/*
 * udc_int_bulkin()
 * This is function for Endpoint One ( Bulk In)function routine.
 */
void udc_int_bulkin(void)
{
	__u32 ep1csr;

	/* EP1 CSR register status check */
	UD_INDEX_REG = UD_INDEX_EP1;
	ep1csr = UD_EP_STATUS_REG;
	LOG("EP1: DMA_TOTAL_CNT1_REG : %x\n", UD_DMA_TOTAL_CNT1_REG);

	if (ep1csr & UD_EP_SENT_STALL) {
		LOG("Sent Stall \n");
		UD_EP_STATUS_REG = UD_EP_SENT_STALL;
	}

	if (ep1csr & UD_EP_TX_SUCCESS) {
		LOG("EP1_TX_SUCCESS\n");
		UD_EP_STATUS_REG = UD_EP_TX_SUCCESS; /* Register Clear */
		if (op_mode == USB_CPU_MODE) {
			if (upload_size != 0)
				upload_start();
		}
	}

	if (UD_DMA_TOTAL_COUNT_ZERO & ep1csr) {
		LOG("USB_DMA_MODE, DMA TX Done(DMA_TOTAL_COUNT_ZERO) !!\n");
		UD_DMA_CON_REG = UD_DMA_TX_STOP|UD_USB_INT_MODE;
		UD_FIFO_CON_REG = UD_DMA_DISABLE;
	}
}

void download_start(__u32 ep_status)
{
	__u16 fifo_data[5];
	__u32 fifo_count;
	__u32 fifo_byte_cnt;
	__u32 upload_addr;
	__u16 temp;
	__u32 dn_filesize;

	fifo_count = UD_BYTE_READ_CNT_REG;
	LOG("fifocnt: %x\n", fifo_count);

	if (fifo_count == 5) {
		read_epbuf(fifo_data, 10);
		temp = fifo_data[4];
		LOG("temp: 0x%x\n", temp);

		if (temp==0x1) {
			upload_addr = fifo_data[0] + (fifo_data[1]<<16);
			upload_size = fifo_data[2] + (fifo_data[3]<<16);
			upload_ptr = (__u16 *)upload_addr;

			printf("UploadAddress : 0x%x, UploadSize: 0x%x\n",
				upload_ptr, upload_size);

			if (op_mode == USB_CPU_MODE) {
				LOG("CPU_MODE Bulk In Function\n");
				upload_start();
			} else {
				LOG("DMA_MODE Bulk In Function\n");
				UD_FIFO_CON_REG		= UD_DMA_ENABLE;
				UD_INDEX_REG		= UD_INDEX_EP1;
				UD_DMA_IF_CON_REG	= UD_MAX_BURST_INCR16;
				UD_BYTE_WRITE_CNT_REG	= ep1_maxpktsize;
				UD_MAX_PKT_REG		= ep1_maxpktsize;
				UD_DMA_FIFO_CNT_REG	= ep1_maxpktsize;
				UD_DMA_CNT_REG		= ep1_maxpktsize;
				UD_DMA_MEM_BASE_ADDR_REG = upload_addr;
				UD_DMA_TOTAL_CNT1_REG	= upload_size;
				UD_DMA_TOTAL_CNT2_REG	= upload_size>>16;
				UD_DMA_CON_REG		= UD_DMA_FLY_ENABLE|UD_DMA_TX_START|USB_DMA_MODE;
			}
		}
		download_filesize=0;
		return;
	} else {
		download_addr = s3c_usbd_dn_addr;

		read_epbuf(fifo_data, 8);
		if (ep_status&(0x1<<4))
			fifo_byte_cnt = fifo_count * 2 -1;
		else
			fifo_byte_cnt = fifo_count * 2;

		LOG("downloadFileSize==0, 1'st fifo_byte_cnt : 0x%x\n",
			fifo_byte_cnt);

		download_filesize = fifo_data[2] + (fifo_data[3]<<16);
		download_ptr = (__u16 *)download_addr;

		/*
		 * The first 8-bytes are deleted.
		 * USB format : addr(4)+size(4)+data(n)+cs(2)
		 */
		read_epbuf(download_ptr, fifo_byte_cnt-8);

		if (op_mode == USB_CPU_MODE) {
			if (ep_status & (0x2<<2)) {
				fifo_count = UD_BYTE_READ_CNT_REG;

				if (ep_status&(0x1<<4))
					fifo_byte_cnt = fifo_count * 2 -1;
				else
					fifo_byte_cnt = fifo_count * 2;

				LOG("2'd fifo_byte_cnt : %x\n", fifo_byte_cnt);
				read_epbuf(download_ptr, fifo_byte_cnt);
			}
		} else {
			dn_filesize = download_filesize-fifo_byte_cnt;
			if (dn_filesize > ep3_maxpktsize) {
				UD_FIFO_CON_REG		= UD_DMA_ENABLE;
				UD_INDEX_REG		= UD_INDEX_EP3;
				UD_DMA_IF_CON_REG	= UD_MAX_BURST_INCR16;
				UD_MAX_PKT_REG		= ep3_maxpktsize;
				UD_DMA_FIFO_CNT_REG	= ep3_maxpktsize;
				UD_DMA_CNT_REG		= ep3_maxpktsize;
				UD_DMA_MEM_BASE_ADDR_REG= download_addr+fifo_byte_cnt-8;
				dn_filesize		= (dn_filesize/ep3_maxpktsize)*ep3_maxpktsize;
				UD_DMA_TOTAL_CNT1_REG	= (__u16)(dn_filesize);
				UD_DMA_TOTAL_CNT2_REG	= (__u16)(dn_filesize>>16);
				UD_DMA_CON_REG		= UD_DMA_FLY_ENABLE|UD_DMA_RX_START|UD_USB_DMA_MODE;

				LOG("Out Direction DMA RX Start\n");
				LOG("download_ptr : 0x%x, UD_DMA_MEM_BASE_ADDR_REG : 0x%x, dn_filesize : 0x%x\n",
					download_ptr, UD_DMA_MEM_BASE_ADDR_REG, dn_filesize);
			}
			else
				op_mode = USB_CPU_MODE;
		}
	}
}

void download_continue(__u32 ep_status)
{
	__u32 fifo_count;
	__u32 fifo_byte_cnt;

	fifo_count = UD_BYTE_READ_CNT_REG;
	if (ep_status&(0x1<<4))
		fifo_byte_cnt = fifo_count * 2 -1;
	else
		fifo_byte_cnt = fifo_count * 2;

	LOG("downloadFileSize!=0, 1'st fifo_byte_cnt : %x\n", fifo_byte_cnt);
	read_epbuf(download_ptr, fifo_byte_cnt);

	if (ep_status & (0x2<<2)) {
		fifo_count = UD_BYTE_READ_CNT_REG;

		if (ep_status&(0x1<<4))
			fifo_byte_cnt = fifo_count * 2 -1;
		else
			fifo_byte_cnt = fifo_count * 2;

		LOG("2'd fifo_byte_cnt : %x\n", fifo_byte_cnt);
		read_epbuf(download_ptr, fifo_byte_cnt);

	}
	/* USB format : addr(4)+size(4)+data(n)+cs(2) */
	if (((__u32)download_ptr - download_addr) >= (download_filesize - 8)) {
		printf("Download Done!! Download Address: 0x%x, Download Filesize:0x%x\n",
			download_addr, (download_filesize-10));

		verify_checksum();
		s3c_receive_done = 1;
	}
}
/*
 * udc_int_bulkout()
 * This is function for Endpoint Three ( Bulk Out)function routine.
 */
void udc_int_bulkout(void)
{
	__u32 ep3_status;
	__u32 fifo_count;
	__u32 fifo_byte_cnt;
	__u32 DmaCurrAddr;

	UD_INDEX_REG = UD_INDEX_EP3;
	ep3_status = UD_EP_STATUS_REG;

	LOG("ep3_status: 0x%x, EP3: DMA_TOTAL_CNT1_REG : 0x%x, DMA_TOTAL_CNT2_REG : 0x%x\n",
		ep3_status, UD_DMA_TOTAL_CNT1_REG, UD_DMA_TOTAL_CNT2_REG);


	if (ep3_status & UD_EP_SENT_STALL) {
		UD_EP_STATUS_REG = UD_EP_SENT_STALL;
		return;
	}

	if (ep3_status & UD_EP_FIFO_FLUSH) {
		UD_EP_CON_REG = UD_EP_FIFO_FLUSH;
		return;
	}

	if (ep3_status & UD_EP_RX_SUCCESS) {
		LOG("EP3_RX_PKT_SUCCESS : download_filesize : 0x%x\n",
			download_filesize);
		if (download_filesize==0)
			download_start(ep3_status);
		else {
			if (op_mode == USB_CPU_MODE)
				download_continue(ep3_status);
		}
	}

	if (ep3_status & UD_DMA_TOTAL_COUNT_ZERO) {
		mdelay(10);
		UD_DMA_CON_REG = UD_DMA_RX_STOP|UD_USB_INT_MODE;
		UD_FIFO_CON_REG = UD_DMA_DISABLE;
		ep3_status = UD_EP_STATUS_REG;
		LOG("DMA Total Count Zero set!! 2'nd ep3_status : 0x%x\n",
			ep3_status);

		if (ep3_status & UD_EP_RX_SUCCESS) {	/* short packet handled */
			fifo_count = UD_BYTE_READ_CNT_REG;
			LOG("fifo_count : 0x%x\n", fifo_count);

			 /* RX packet success but remain Odd word */
			if (ep3_status&(0x1<<4))
				fifo_byte_cnt = fifo_count * 2 -1;
			else
				fifo_byte_cnt = fifo_count * 2;

			DmaCurrAddr = UD_DMA_MEM_CURRENT_ADDR_REG;
			LOG("DmaCurrAddr : 0x%x, fifo_byte_cnt : 0x%x\n",
				DmaCurrAddr, fifo_byte_cnt);
			download_ptr = (__u16 *)DmaCurrAddr;
			read_epbuf(download_ptr, fifo_byte_cnt);

			if (ep3_status & (0x2<<2)) {
				fifo_count = UD_BYTE_READ_CNT_REG;

				if (ep3_status&(0x1<<4))
					fifo_byte_cnt = fifo_count * 2 -1;
				else
					fifo_byte_cnt = fifo_count * 2;

				LOG("2'd BYTE_READ_CNT_REG : %x\n",
					fifo_byte_cnt);
				read_epbuf(download_ptr, fifo_byte_cnt);
			}
			printf("download_ptr: 0x%x, Download Address: 0x%x, Download Filesize:0x%x\n",
				download_ptr, download_addr, (download_filesize-10));

			verify_checksum();
			s3c_receive_done =1;
		} else {
			printf("download_ptr: 0x%x, Download Address: 0x%x, Download Filesize:0x%x\n",
				download_ptr, download_addr, (download_filesize-10));

			verify_checksum();
			s3c_receive_done = 1;
		}
	}
}

void s3c_udc_int_hndlr(void)
{
	__u32 usb_status	= UD_SYS_STATUS_REG;
	__u32 ep_int		= UD_EP_INT_REG;

	LOG("[%d] UD_SYS_STATUS_REG = 0x%x, UD_EP_STATUS_REG = 0x%x, UD_EP_INT_REG = 0x%x\n",
		sb_debug_cnt++, usb_status, UD_EP_STATUS_REG, ep_int);

	if (usb_status & UD_INT_VBUSON) {
		LOG("[%d] VBUSON interrupt \n", sb_debug_cnt++);
		UD_SYS_STATUS_REG = UD_INT_VBUSON;	/* Interrupt clear */
	}

	/* Error interrupt check */
	if (usb_status & UD_INT_ERR) {
		LOG("[%d] Error interrupt \n", sb_debug_cnt++);
		UD_SYS_STATUS_REG = UD_INT_ERR;		/* Interrupt clear */
	}

	/* Which USB interrupts happen */
	if (usb_status & UD_INT_SUSPEND) {
		LOG("[%d] Suspend Mode \n", sb_debug_cnt++);
		UD_SYS_STATUS_REG = UD_INT_SUSPEND;	/* Interrupt clear */
	}

	if (usb_status & UD_INT_RESUME) {
		LOG("[%d] Resume Mode \n", sb_debug_cnt++);
		UD_SYS_STATUS_REG = UD_INT_RESUME;	/* Interrupt clear */
	}

	if (usb_status & UD_INT_RESET) {
		LOG("[%d] Reset Mode \n", sb_debug_cnt++);
		UD_SYS_STATUS_REG = UD_INT_RESET;	/* Interrupt clear */
		set_endpoint();
		s3c_ep0_state = EP0_STATE_IDLE;
	}

	if (usb_status & UD_INT_SDE) {
		LOG("[%d] Speed Detection interrupt \n", sb_debug_cnt++);
		UD_SYS_STATUS_REG = UD_INT_SDE;		/* Interrupt clear */

		/* Set if Device is High speed or Full speed */
		if (usb_status & UD_INT_HSP) {
			LOG("[%d] Host High Speed Detection\n", sb_debug_cnt++);
			UD_SYS_STATUS_REG = UD_INT_HSP; /* interrupt clear */
			set_max_pktsize(USB_SPEED_HIGH);
			initialize_descriptors();
		} else {
			LOG("[%d] Host Full Speed Detection\n", sb_debug_cnt++);
			set_max_pktsize(USB_SPEED_FULL);
			initialize_descriptors();
		}
	}

	if (ep_int & UD_INT_EP0) {
		LOG("[%d] Control Transfer Interrupt \n", sb_debug_cnt++);
		UD_EP_INT_REG = UD_INT_EP0; /* Interrupt Clear */
		s3c_ep0_int_hndlr();
	}

	/* Endpoint1 bulkIn */
	else if (ep_int & UD_INT_EP1) {
		UD_EP_INT_REG = UD_INT_EP1; /* Interrupt Clear */
		LOG("[%d] Bulk In Transfer Interrupt  \n", sb_debug_cnt++);
		udc_int_bulkin();
	}

	/* Endpoint3 bulkOut */
	else if (ep_int & UD_INT_EP3) {
		LOG("[%d] Bulk Out Transfer Interrupt  \n", sb_debug_cnt++);
		UD_EP_INT_REG = UD_INT_EP3; /* Interrupt Clear */
		udc_int_bulkout();
	}
}

/* setup default descriptors */
static void initialize_descriptors(void)
{
	LOG("\n");

	/* Device descriptor */
	desc.dev.bLength = USB_DESC_DEVICE_SIZE;	/* size of this descriptor in bytes */
	desc.dev.bDescriptorType = USB_DESC_DEVICE;	/* device descriptor type */
	if (udc_speed == USB_SPEED_FULL) {
		desc.dev.bcdUSB = 0x0110;	/* Ver 1.1 */
	} else {
		desc.dev.bcdUSB = 0x0200;	/* Ver 2.0 */
	}
	desc.dev.bDeviceClass	= 0xFF;	/* Class code (0xff vendor specific) */
	desc.dev.bDeviceSubClass= 0x0;	/* Subclass code */
	desc.dev.bDeviceProtocol= 0x0;	/* Protocol code */
	desc.dev.bMaxPacketSize0= (__u8)ep0_maxpktsize; /* Maximum packet size */
	desc.dev.idVendor	= 0x04e8;/* Vendor ID undefined(assigned by tht USB-IF) */
	desc.dev.idProduct	= 0x1234;/* Product ID(assigned by the nanufacturer) */
	desc.dev.bcdDevice	= 0x0100;/* Vendor assigned device release num */
	desc.dev.iManufacturer	= 0x1;	/* Index of string descriptor describing manufacturer */
	desc.dev.iProduct	= 0x2;	/* Index of string descriptor describing product */
	desc.dev.iSerialNumber	= 0x0;	/* Index of string descriptor describing the device's s/n */
	desc.dev.bNumConfigurations = 0x1;/*Number of possible configurations */

	/* Configuration descriptor */
	desc.b.cfg.bLength = USB_DESC_CONFIG_SIZE;
	desc.b.cfg.bDescriptorType = USB_DESC_CONFIG;
	desc.b.cfg.wTotalLength = 0x20;
	desc.b.cfg.bNumInterfaces = 1;
	desc.b.cfg.bConfigurationValue = 1;
	desc.b.cfg.iConfiguration = 0;
	desc.b.cfg.bmAttributes = USB_CONFIG_BUSPOWERED;
	desc.b.cfg.MaxPower = USB_POWER(50); /* draws 50mA current from the USB bus */

	/* Interface descriptor */
	desc.b.intf.bLength = USB_DESC_INTERFACE_SIZE;
	desc.b.intf.bDescriptorType = USB_DESC_INTERFACE;
	desc.b.intf.bInterfaceNumber = 0x0;	/* unique intf index */
	desc.b.intf.bAlternateSetting = 0x0;
	desc.b.intf.bNumEndpoints = 2;	/* endpoint number excluding ep0 */
	desc.b.intf.bInterfaceClass = 0xFF;	/* vendor specific */
	desc.b.intf.bInterfaceSubClass = 0x0;
	desc.b.intf.bInterfaceProtocol = 0x0;
	desc.b.intf.iInterface = 0x0;

	/* EP1 descriptor */
	desc.b.ep1.bLength = USB_DESC_ENDPOINT_SIZE;
	desc.b.ep1.bDescriptorType = USB_DESC_ENDPOINT;
	desc.b.ep1.bEndpointAddress = USB_EP_ADDRESS(1, USB_IN);
	desc.b.ep1.bmAttributes = USB_EP_BULK;
	desc.b.ep1.wMaxPacketSize = (__u16)ep1_maxpktsize;
	desc.b.ep1.bInterval = 0x0;

	/* EP3 descriptor */
	desc.b.ep3.bLength = USB_DESC_ENDPOINT_SIZE;
	desc.b.ep3.bDescriptorType = USB_DESC_ENDPOINT;
	desc.b.ep3.bEndpointAddress = USB_EP_ADDRESS(3, USB_OUT);
	desc.b.ep3.bmAttributes = USB_EP_BULK;
	desc.b.ep3.wMaxPacketSize = (__u16)ep3_maxpktsize;
	desc.b.ep3.bInterval = 0x0;
}

/*
 * USB PHY Initialization
 */
int s3c_usbctl_init(void)
{
	/* usb power enable, vbus detect pull-up/down disable */
#if defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
	GPHPU_REG = (GPHPU_REG&~(3<<28))|(2<<28); /* usb power pull-up enable */
	GPFPU_REG &= ~(0x3<<4); /* vbus detect pull-up/down disable */
#else
	GPHPU_REG &= ~(0x3<<28); /* usb power pull-up enable */
#endif
	GPHCON_REG = (GPHCON_REG&~(3<<28))|(1<<28); /* output */
	GPHDAT_REG |= (1<<14); /* output */

	mdelay(1);
	/* if reset by sleep wakeup, control the retention I/O cell */
	if (RSTSTAT_REG & 0x8) RSTCON_REG |= (1<<16);


	MISCCR_REG = MISCCR_REG&~(1<<12);	/* USB Port is Normal mode */
	PWRCFG_REG |= (0x1<<4);	/* phy power enable */

	/* USB device 2.0 must reset like bellow,
	 * 1st phy reset and after at least 10us, func_reset & host reset
	 * phy reset can reset bellow registers.
	 */
	/* PHY 2.0 S/W reset */
	USB_RSTCON_REG = (0x0<<2)|(0x0<<1)|(0x1<<0);
	mdelay(10); /* phy reset must be asserted for at 10us */

	/*Function 2.0, Host 1.1 S/W reset*/
	USB_RSTCON_REG = (0x1<<2)|(0x1<<1)|(0x0<<0);
	USB_RSTCON_REG = (0x0<<2)|(0x0<<1)|(0x0<<0);

	/* 48Mhz,Oscillator,External X-tal,device */
	USB_PHYCTRL_REG = (0x0<<3)|(0x1<<2)|(0x1<<1)|(0x0<<0);

	/* 48Mhz clock on ,PHY2.0 analog block power on
	 * XO block power on,XO block power in suspend mode,
	 * PHY 2.0 Pll power on ,suspend signal for save mode disable
	 */
	USB_PHYPWR_REG = (0x1<<31)|(0x0<<4)|(0x0<<3)|(0x0<<2)|(0x0<<1)|(0x0<<0);

	/* D+ pull up disable(VBUS detect), USB2.0 Function clock Enable,
	 * USB1.1 HOST disable, USB2.0 PHY test enable
	 */
	USB_CLKCON_REG = (0x0<<31)|(0x1<<2)|(0x0<<1)|(0x1<<0);

	download_filesize = 0;
	op_mode = USB_OP_MODE;
	set_endpoint();
	s3c_ep0_state = EP0_STATE_IDLE;
	s3c_ep0_sub_state = 0;

	/* D+ pull up , USB2.0 Function clock Enable,
	 * USB1.1 HOST disable,USB2.0 PHY test enable
	 */
	USB_CLKCON_REG = (0x1<<31)|(0x1<<2)|(0x0<<1)|(0x1<<0);

	LOG("S3C2443 USB Controller Core Initialized\n");

	return 0;
}

#endif /* CONFIG_S3C2443 */
