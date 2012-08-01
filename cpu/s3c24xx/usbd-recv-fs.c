/*
 * cpu/s3c24xx/usbd-recv-fs.c
 *
 *  $Id: usbd-recv-fs.c,v 1.1 2007/02/01 07:03:19 dasan Exp $
 *
 * (C) Copyright 2007
 * Byungjae Lee, Samsung Erectronics, bjlee@samsung.com.
 *	- only support for S3C2412, S3C2443
 *
 * 2004 (C) SAMSUNG ELECTRONICS
 *       SW.LEE <hitchcar@samsung.com>
 *       ported to Uboot for S3C24A0A, S3C2440A
 *
 *
 * S3C2410 USB receive function
 * endpoint 1
 *
 * bushi <bushi@mizi.com>
 */

#include <common.h>

#if defined(CONFIG_S3C2412) || defined(CONFIG_S3C2442)
#include <command.h>

#include <regs.h>

#include "usbd-fs.h"

/* error number from linux */
#define EINTR            4	/* Interrupted system call */
#define EAGAIN          11	/* Try again */
#define EBUSY           16	/* Device or resource busy */
#define ENODEV          19	/* No such device */


#undef	USB_RECV_DEBUG

#ifdef USB_RECV_DEBUG
#define LOG(arg...) printf(arg)
#else
#define LOG(arg...)
#endif

static unsigned int ep1_len;
static unsigned int ep1_curdmalen;
static unsigned int rx_pktsize;
static unsigned int last_packet;

int s3c_got_header = 0;
int s3c_receive_done = 0;

union {
	unsigned char data[9];
	unsigned int val[2];
} hdr;

static void ep1_done(int flag)
{
	LOG(" start\n");

	if (!ep1_len)
		return;
	ep1_len = ep1_curdmalen = 0;

	LOG(" end\n");
}

void s3c_ep1_reset(void)
{
	desc_t *pd = s3c_usb_get_descriptor_ptr();
	rx_pktsize = __le16_to_cpu(pd->b.ep1.wMaxPacketSize);

	LOG("rx_pktsize = %d\n", rx_pktsize);

	UD_INDEX = UD_INDEX_EP1;
	UD_OCSR1 &= ~(UD_OCSR1_SENDSTL);

	ep1_done(-EINTR);
}


static void check_eof(int len)
{
	int unit;
	int index;
	unsigned short cal = 0, rec;
	unit = UD_MAXP * 8;
	
	if (DNW) {
		if ((hdr.val[1] == (s3c_usbd_dn_cnt + 8))) {
			rec = *(unsigned short *) (s3c_usbd_dn_addr + s3c_usbd_dn_cnt - 2);
			printf("Recevie CRC: Received 0x%x \n", rec);
			for (index = s3c_usbd_dn_addr + 8; index < hdr.val[1]; index++)
				cal += *((unsigned char *) (index));
			printf("Calculated CRC: 0x%x bytes \n", cal);
		}
	}

	if (0 == (s3c_usbd_dn_cnt % SZ_64K))
		printf("*");

	if (len != unit) {
		printf("\nDownload done: Received %d(0x%x) bytes \n", s3c_usbd_dn_cnt, s3c_usbd_dn_cnt);
		printf("Received data is located to 0x%08X \n", s3c_usbd_dn_addr);
		s3c_got_header = 0;
		s3c_usbd_dn_cnt = 0;
	}
}

#ifdef OREIAS
# define adjust_bank1(x) {\
	if(x>=0x11e00000) \
		x=x+0x6200000;\
}
#endif

#ifdef CONFIG_DISCONTIG
# define adjust_bank1(x) {\
	if(x>=0x11e00000) \
		x=x+0xe200000;\
}
#endif

void s3c_ep1_int_hndlr(int udcsr)
{
	unsigned int len;
	int status;
	int recv_cnt;

	if (DNW) {
		UD_INDEX = UD_INDEX_EP3;
	}
	else {
		UD_INDEX = UD_INDEX_EP1;
	}
	status = UD_OCSR1;

	LOG("0x%02x\n", UD_OCSR1);
	LOG("0x%02x\n", UD_OCSR2);
	LOG("0x%02x\n", UD_ICSR1);
	LOG("0x%02x\n", UD_ICSR2);
	if (status & UD_OCSR1_SENTSTL) {
		if (DNW) {
			UD_INDEX = UD_INDEX_EP3;
		}
		else {
			UD_INDEX = UD_INDEX_EP1;
		}
		UD_OCSR1 &= ~UD_OCSR1_SENTSTL;
		LOG("UD_OCSR1_SENTSTL\n");
		return;
	}

	if (status & UD_OCSR1_PKTRDY) {
		LOG("ep1_len=%x\n", ep1_len);

		if (DNW) {
			UD_INDEX = UD_INDEX_EP3;
		}
		else {
			UD_INDEX = UD_INDEX_EP1;
		}

		recv_cnt = ((UD_OFCNTH << 8) | UD_OFCNTL) & 0xffff;

		if (DNW) {
			if (!s3c_got_header) {
				for (len = 0; len < 8; len++)
					hdr.data[len] = (u_char) UD_FIFO3;
				hdr.data[len] = '\0';
				s3c_usbd_dn_addr = hdr.val[0];
				last_packet = hdr.val[1] / 64;
				printf("\nDownload Done!! Download address: 0x%x, Download Filesize: 0x%x\n",
					s3c_usbd_dn_addr, hdr.val[1] - 10);
			}

			for (len = 0; len < recv_cnt - 8 * (1 - s3c_got_header); len++) {
#ifdef CONFIG_DISCONTIG
				u8 *tptr;
				tptr = (s3c_usbd_dn_addr + s3c_usbd_dn_cnt);
				adjust_bank1(tptr);
				*(tptr) = (u_char) (UD_FIFO3);
				s3c_usbd_dn_cnt++;
#else
				*(volatile unsigned char *) (s3c_usbd_dn_addr +
							     s3c_usbd_dn_cnt) = (u_char) UD_FIFO3;
				s3c_usbd_dn_cnt++;
#endif
				LOG("ep1_buf[%d]=%x\n", len, ep1_buf[len]);
			}
			last_packet--;
			s3c_got_header = 1;
		}
		else {
			LOG("recv_count = %d\n", recv_cnt);
			for (len = 0; len < recv_cnt; len++) {
#if defined(OREIAS) || defined(CONFIG_DISCONTIG)
				u8 *tptr;
				tptr = (s3c_usbd_dn_addr + s3c_usbd_dn_cnt);
				adjust_bank1(tptr);
				*(tptr) = (u_char) (UD_FIFO1);
				s3c_usbd_dn_cnt++;
#else
				//ep1_buf[len] = (u_char) UD_FIFO1;
				*(volatile unsigned char *) (s3c_usbd_dn_addr +
							     s3c_usbd_dn_cnt) = (u_char) UD_FIFO1;
				s3c_usbd_dn_cnt++;
#endif
				LOG("ep1_buf[%d]=%x\n", len, ep1_buf[len]);
			}

			UD_INDEX = UD_INDEX_EP1;
			UD_OCSR1 &= ~(0xFF);	//shaju to clear
			if (((UD_OCSR1 & 0x01) == 1) && ((UD_INT & 0x8) == 0)) {

				UD_INDEX = UD_INDEX_EP1;
				recv_cnt = ((UD_OFCNTH << 8) | UD_OFCNTL) & 0xffff;
				for (len = 0; len < recv_cnt; len++) {
					//ep1_buf[len] = (u_char) UD_FIFO1;
#if defined(OREIAS) || defined(CONFIG_DISCONTIG)
					u8 *tptr;
					tptr = (s3c_usbd_dn_addr + s3c_usbd_dn_cnt);
					adjust_bank1(tptr);
					*(tptr) = (u_char) (UD_FIFO1);
					s3c_usbd_dn_cnt++;
#else
					*(vu_char *) (s3c_usbd_dn_addr + s3c_usbd_dn_cnt) = (u_char) UD_FIFO1;
					s3c_usbd_dn_cnt++;
#endif
					LOG("ep1_buf[%d]=%x\n", len, ep1_buf[len]);
				}

				UD_INDEX = UD_INDEX_EP1;
				UD_OCSR1 &= ~(0xFF);	//shaju to clear

			}

			check_eof(recv_cnt);

		}

		if (DNW) {
			UD_INDEX = UD_INDEX_EP3;
			UD_OCSR1 &= ~(0xFF);	//shaju to clear
		}

		if (DNW) {
			/* H/w bug workaround */
			if (!last_packet) {
				if (hdr.val[1] % 64) {
					UD_INDEX = UD_INDEX_EP3;
					while (!(UD_OCSR1 & 0x1));
					len = ((UD_OFCNTH << 8) | UD_OFCNTL) & 0xffff;
					if (len < hdr.val[1] % 64)
						printf("File Download Should Be Aborted !!!\n");
				}

				for (len = hdr.val[1] % 64; len; len--) {	//We read only what we need
					*(vu_char *) (s3c_usbd_dn_addr + s3c_usbd_dn_cnt) = (u_char) UD_FIFO3;
					s3c_usbd_dn_cnt++;
				}

				UD_INDEX = UD_INDEX_EP3;
				UD_OCSR1 |= UD_OCSR1_FFLUSH;

				if (s3c_usbd_dn_cnt != (hdr.val[1] - 8)) {
					printf("Size=%d s3c_usbd_dn_cnt=%d ", hdr.val[1], s3c_usbd_dn_cnt);
				}
				else {
					hdr.val[0] = 0;
					hdr.val[1] = 0;
					hdr.data[0] = *(vu_char *) (s3c_usbd_dn_addr + s3c_usbd_dn_cnt - 1);
					hdr.data[1] = *(vu_char *) (s3c_usbd_dn_addr + s3c_usbd_dn_cnt - 2);
					printf("Checksum of received file = 0x%X\n", hdr.val[0]);
					s3c_receive_done = 1;
				}
				s3c_usbd_dn_cnt = s3c_got_header = 0;
			}
		}
	}
}

#endif /* #ifdef CONFIG_S3C2412 */
