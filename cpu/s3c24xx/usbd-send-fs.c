/*
 * cpu/s3c24xx/usbd-send-fs.c
 *
 *  $Id: usbd-send-fs.c,v 1.1 2007/02/01 07:03:19 dasan Exp $
 *
 * (C) Copyright 2007
 * Byungjae Lee, Samsung Erectronics, bjlee@samsung.com.
 *	- only support for S3C2412, S3C2443
 *
 * S3C2410 USB send function
 * endpoint 2
 *
 * bushi<bushi@mizi.com>
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

#if defined(CONFIG_S3C2412) || defined(CONFIG_S3C2442)
#include <command.h>

#include <regs.h>
#include "usbd-fs.h"


/* error number from linux */
#define EINTR            4	/* Interrupted system call */
#define EAGAIN          11	/* Try again */
#define EBUSY           16	/* Device or resource bus */
#define ENODEV          19	/* No such device */


#undef	USB_SEND_DEBUG
#ifdef USB_SEND_DEBUG
#define LOG(arg...) printf(__FILE__":"__FUNCTION__"(): " arg)
#else
#define LOG(arg...)
#endif

static int ep2_len;

/* Enable DMA on the endpoint 2. */
#ifdef USE_USBD_DMA
static dma_addr_t ep2_dma;
static dma_addr_t ep2_curdmapos;
#else
static char *ep2_curbufpos;
#endif

static int ep2_curdmalen;
static int ep2_remain;
static int tx_pktsize;

static void ep2_start(void)
{
#ifndef USE_USBD_DMA
	int i;
#endif

	LOG("\n");

	if (!ep2_len) {
		LOG("!ep2_len\n");
		return;
	}

	ep2_curdmalen = tx_pktsize;	/* 64 */

	if (ep2_curdmalen > ep2_remain)
		ep2_curdmalen = ep2_remain;

	LOG("ep2_curdmalen = %d\n", ep2_curdmalen);

	for (i = 0; i < ep2_curdmalen; i++) {
		LOG("ep2_curbufpos[i] = 0x%02x\n", ep2_curbufpos[i]);
		UD_FIFO2 = (u_char) ep2_curbufpos[i] & UD_FIFO2_DATA;
	}

	if (DNW)
		UD_INDEX = UD_INDEX_EP3;
	else
		UD_INDEX = UD_INDEX_EP2;
}

static void ep2_done(int flag)
{
	LOG("ep2_len = %d, ep2_remain = %d\n", ep2_len, ep2_remain);

	if (ep2_len)
		ep2_len = 0;
}

void s3c_ep2_reset(void)
{
	desc_t *pd = s3c_usb_get_descriptor_ptr();
	tx_pktsize = __le16_to_cpu(pd->b.ep2.wMaxPacketSize);	/* 64 */

	LOG("tx_pktsize = %d\n", tx_pktsize);

	if (DNW)
		UD_INDEX = UD_INDEX_EP3;
	else
		UD_INDEX = UD_INDEX_EP2;

	UD_ICSR1 &= ~(UD_ICSR1_SENDSTL);	// write 0 to clear

	ep2_done(-EINTR);
}

void s3c_ep2_int_hndlr(int udcsr)
{
	int status;

	if (DNW)
		UD_INDEX = UD_INDEX_EP3;
	else
		UD_INDEX = UD_INDEX_EP2;

	status = UD_ICSR1;
	LOG("status = 0x%08x\n", status);
	if (status & UD_ICSR1_SENTSTL) {
		if (DNW)
			UD_INDEX = UD_INDEX_EP3;
		else
			UD_INDEX = UD_INDEX_EP2;

		UD_ICSR1 &= ~UD_ICSR1_SENTSTL;	// clear_ep1_sent_stall;
		return;
	}
	LOG("[%d]\n", __LINE__);

	if (!(status & UD_ICSR1_PKTRDY)) {
		LOG("[%d]\n", __LINE__);
		ep2_curbufpos += ep2_curdmalen;
		ep2_remain -= ep2_curdmalen;

		if (ep2_remain != 0) {
			LOG("[%d]\n", __LINE__);
			ep2_start();
		}
		else {
			LOG("[%d]\n", __LINE__);
			ep2_done(0);
		}
	}
}

#endif /* #ifdef CONFIG_S3C2412 */
