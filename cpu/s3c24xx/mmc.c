/*
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <part.h>

#include <regs.h>
#include <mmc.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/protocol.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include "mmc.h"

//#define DEBUG_MMC
#ifdef DEBUG_MMC
#define DBG(x...)       printf(x)
#else
#define DBG(x...)       do { } while (0)
#endif

#ifdef CONFIG_MMC

#define SDI_Tx_buffer_HSMMC (0x31000000)
#define SDI_Rx_buffer_HSMMC (0x31300000)

extern int
fat_register_device(block_dev_desc_t *dev_desc, int part_no);

static block_dev_desc_t mmc_dev;

block_dev_desc_t * mmc_get_dev(int dev)
{
	return ((block_dev_desc_t *)&mmc_dev);
}

/*
 * FIXME needs to read cid and csd info to determine block size
 * and other parameters
 */
static uchar mmc_buf[MMC_BLOCK_SIZE];
static int mmc_ready = 0;

CardType card;
static uint RCA = 0;
static uint mmc_spec, sd_spec;
static uint width = 1;

static void mmc_set_gpio(void)
{
	u32 regs;
	regs = readl(ELFIN_GPIO_BASE+GPECON_OFFSET) & ~(0xfff << 10);
	writel(regs | 0xaaa << 10, ELFIN_GPIO_BASE+GPECON_OFFSET);
}

static int check_command_end(ushort cmd, uint resp)
{
	u32 sdi_csta;

	sdi_csta = readl(SDICSTA);
	DBG("sdi_csta: 0x%x \n", sdi_csta);
	
	if(!(resp&WAIT_RESP)) {
		while(!(sdi_csta & CMD_END))
			sdi_csta = readl(SDICSTA);

		writel(sdi_csta, SDICSTA);
		return 0;
	}
	else {
		while(!((sdi_csta&RESP_END) || (sdi_csta&CMD_TOUT)))
			sdi_csta = readl(SDICSTA);

		writel(sdi_csta, SDICSTA);
		return 0;
	}
	return 1;
}

static int mmc_cmd(ushort cmd, uint arg, uint flags)
{
	uint ret = -1;
	u32 sdi_carg, sdi_ccon;

	DBG("cmd 0x%x(%d), arg 0x%x, wait %d\n", cmd, cmd, arg, (flags >> 9) & 0x1);
	
	sdi_carg = arg;
	writel(sdi_carg, SDICARG);
	
	sdi_ccon = flags | (0x40 | cmd);
	writel(sdi_ccon | CMD_START, SDICCON);

	ret = check_command_end(cmd, flags&WAIT_RESP);
		
	return ret;
}

void get_sd_scr(void)
{
	uint scr1;

	mmc_cmd(MMC_APP_CMD, RCA<<16, SHORT_RESP | WITH_DATA | WAIT_RESP);	
	mmc_cmd(SD_APP_SEND_SCR, 0, SHORT_RESP | WITHOUT_DATA | WAIT_RESP);

	scr1 = readl(SDIRSP0);
	DBG("scr1: 0x%08x \n", scr1);

	if (scr1 & 0x1<<24)
		sd_spec = 1;	/* Version 1.10, support CMD6 */
	else
		sd_spec = 0;	/* Version 1.0 ~ 1.01 */

	if (sd_spec)
		printf("sd_spec: Version 1.%d0, support CMD6 \n", sd_spec);
	else
		printf("sd_spec: Version 1.%d ~ 1.01 \n", sd_spec);

	DBG("sd_spec = 1.%d(0x%08x)\n", sd_spec, scr1);

	width = 4;
	
	printf("SD bus width is %dbit. \n", width);
}

CardType check_sd_ocr(void)
{
	uint i, ocr = 0, ret = ERROR_CARD;

	DBG("check_sd_ocr \n");

	for(i = 0; i < 0x80; i++) {
		if(mmc_cmd(MMC_APP_CMD, RCA<<16, SHORT_RESP | WAIT_RESP))
			DBG("Command %d is fail \n", MMC_APP_CMD);
		
		mmc_cmd(SD_APP_OP_COND, ocr, SHORT_RESP | WAIT_RESP);
		ocr = readl(SDIRSP0);	
		DBG("ocr1: %08x\n", ocr);
		udelay(1000*10);
		if(ocr & (0x1 << 31)) {
			ret = SD_CARD;
			printf("SD card is detected. \n");
			break;
		}
	}

	if(ret) {
		mmc_cmd(MMC_APP_CMD, RCA<<16, SHORT_RESP | WAIT_RESP);
		mmc_cmd(SD_APP_OP_COND, ocr, SHORT_RESP | WAIT_RESP);
		ocr = readl(SDIRSP0);
		DBG("ocr2: %08x\n", ocr);
		if (ocr & (0x1 << 31)) {
			printf("Voltage range: ");
			if (ocr & (1 << 23))
				printf("2.7V ~ 3.6V");
			else if (ocr & (1 << 22))
				printf("2.7V ~ 3.5V");
			else if (ocr & (1 << 21))
				printf("2.7V ~ 3.4V");
			else if (ocr & (1 << 20))
				printf("2.7V ~ 3.3V");
			else if (ocr & (1 << 19))
				printf("2.7V ~ 3.2V");
			else if (ocr & (1 << 18))
				printf("2.7V ~ 3.1V");

			if (ocr & (1 << 7))
				printf(", 1.90V ~ 2.00V\n");
			else
				putc('\n');

			return ret;
		}
		udelay(1000);
	}

	return ret;
}
	
CardType check_mmc_ocr(void)
{
	uint i, ocr = 0;
	CardType ret;

	DBG("check_mmc_ocr \n");
	
	for(i = 0; i < 0x80; i++) {
		mmc_cmd(MMC_SEND_OP_COND, ocr, SHORT_RESP | WAIT_RESP);
		ocr = readl(SDIRSP0);
		DBG("ocr1: %08x\n", ocr);
		/* wait for 10ms */
		udelay(1000*10);
		if ((ocr>>16) & 0x8000) {
			width = 1;
			printf("MMC card is detected(%d bit width). \n", width);
			ret = MMC_CARD;
			break;
		}
		
		ret = ERROR_CARD;
	}

	mmc_cmd(MMC_SEND_OP_COND, ocr, SHORT_RESP | WAIT_RESP);
	ocr = readl(SDIRSP0);
	DBG("ocr2: %08x\n", ocr);		

	if(ret)	{
		if (ocr & (0x1 << 31)) {
			printf("Voltage range: ");
			if (ocr & (1 << 23))
				printf("2.7V ~ 3.6V");
			else if (ocr & (1 << 22))
				printf("2.7V ~ 3.5V");
			else if (ocr & (1 << 21))
				printf("2.7V ~ 3.4V");
			else if (ocr & (1 << 20))
				printf("2.7V ~ 3.3V");
			else if (ocr & (1 << 19))
				printf("2.7V ~ 3.2V");
			else if (ocr & (1 << 18))
				printf("2.7V ~ 3.1V");

			if (ocr & (1 << 7))
				printf(", 1.70V ~ 1.95V(Dual voltage) \n");
			else
				putc('\n');

			return ret;
		}
	}

	return ret;
}

static void set_bus_width(void)
{
	uint sdi_dcon = readl(SDIDCON);
	uint resp;
	if(width == 1)
		writel(sdi_dcon & ~WIDE_BUS_EN, SDIDCON);
	else
		writel(sdi_dcon | WIDE_BUS_EN, SDIDCON);

	if(card == SD_CARD) {
		while(mmc_cmd(MMC_APP_CMD, RCA<<16, SHORT_RESP | WITHOUT_DATA | WAIT_RESP));
		if(width == 4)
			while(mmc_cmd(SD_APP_SET_BUS_WIDTH, 0x2, SHORT_RESP | WITHOUT_DATA | WAIT_RESP));
		else	/* width = 1 */
			while(mmc_cmd(SD_APP_SET_BUS_WIDTH, 0x0, SHORT_RESP | WITHOUT_DATA | WAIT_RESP));
		
		resp = readl(SDIRSP0);
		DBG("SD_APP_SET_BUS_WIDTH(%d bit) resp: 0x%x \n", width, resp);
	}
}

void clock_config(void)
{
	uint sdi_psc;
	ulong mmc_clk, pclk = get_PCLK();

	if(card == MMC_CARD)
		mmc_clk = MMCCLK;
	else if(card == SD_CARD)		
		mmc_clk = SDCLK;
		
	sdi_psc = pclk / mmc_clk - 1;

	while(pclk / (sdi_psc + 1) > mmc_clk)		
		writel(++sdi_psc, SDIPRE);

	if(card == MMC_CARD) ;
	else if(card == SD_CARD) {
		get_sd_scr();
		if (sd_spec == 1) {
			if (mmc_clk > SDCLK) {
//				set_sd_speed(HIGH);	//Higher than 25MHz, should use high speed mode. Max 50MHz and 25MB/sec
				printf("Set SD High speed mode OK!!\n");
			} else {
//				set_sd_speed(NORMAL);
				printf("Set SD Normal speed mode OK!!\n");
			}
		} else
			printf("Old version SD card can not support working frequency higher than 25MHz\n");
	}
	printf("frequency: %dMHz (pclk: %dMHz) \n", pclk / (sdi_psc + 1) / 1048576, pclk / 1048576);
}

static int check_card_status(void)
{
	uint resp;
	
	// Check status is trans state or not.
	while(mmc_cmd(MMC_SEND_STATUS, RCA<<16, LONG_RESP | WAIT_RESP));
	resp = readl(SDIRSP0);
	if (((resp >> 9) & 0xf) == 4) {
		DBG("Enter to transfer status(%d, 0x%x).\n", (resp >> 9) & 0xf, resp);
		return 0;
	}
	else {
		DBG("STATUS: %d\n", (resp >> 9) & 0xf);
		return 1;
	}
}

void display_card_info(void)
{
	uint i, resp[4];
	uint c_size, c_size_multi, read_bl_len, read_bl_len_partial;
	uint trans_speed, trans_speed_multi;
	uint blk_size, card_size;

	for(i=0; i<4; i++)
		resp[i] = readl(SDIRSP0+i*4);

	for(i=0; i<4; i++)
		DBG("resp[%d] : 0x%x\n ", i, resp[i]);

	if(card == MMC_CARD) {
		mmc_spec = (resp[0] >> 26) & 0xF;
		if (!mmc_spec)
			printf("mmc_spec: ver 1.%d to 1.2 \n", mmc_spec);
		else
			printf("mmc_spec: ver %d.x \n", mmc_spec);
	}

	read_bl_len = (resp[1] >> 16) & 0xf;
	read_bl_len_partial = (resp[1] >> 15) & 0x1;
	c_size = ((resp[1] & 0x3ff) << 2 ) | ((resp[2] >> 30) & 0x3);
	c_size_multi = ((resp[2] >> 15) & 0x7);
	trans_speed = resp[0] & 0xff;
	trans_speed_multi = (trans_speed >> 3) & 0xf;
	trans_speed = trans_speed & 0x7;
	blk_size = (1 << read_bl_len);
	card_size = (1 << read_bl_len) * (c_size + 1) * (1 << (c_size_multi + 2)) / 1048576;
	
	printf(" read_bl_len: 		%d\n", read_bl_len);
	printf(" read_bl_len_partial:	%d\n", read_bl_len_partial);
	printf(" c_size: 		%d\n", c_size);
	printf(" c_size_multi: 		%d\n", c_size_multi);
	printf(" trans_speed: 		%d\n", trans_speed);
	printf(" trans_speed_multi: 	%d\n", trans_speed_multi);
	printf(" One Block Size: 	%dByte\n", blk_size);
	printf(" Total Card Size: 	%dMByte\n", card_size + 1);
}

static int mmc_setting(void)
{
	uint i = 0;
	ulong pclk;
	u32 sdi_psc, regs, resp[4];

	mmc_set_gpio();

	pclk = get_PCLK();

	for(sdi_psc = 0 ; sdi_psc < 254 ; sdi_psc++) {
		if ((pclk / (sdi_psc+1)) <= INITCLK)
			break;
        }
	writel(sdi_psc, SDIPRE);

	regs = readl(SDICON);
	writel(regs | CLOCK_TYPE_MMC | BYTE_ORDER_TYPEA | ENABLE_SDCLK, SDICON);

	regs = readl(SDIFSTA);
	writel(regs | FIFO_RESET, SDIFSTA);

	regs = readl(SDIBSIZE) & ~0xfff;
	writel(regs | MMC_BLOCK_SIZE, SDIBSIZE);

	mmc_cmd(MMC_GO_IDLE_STATE, 0,  SHORT_RESP | WAIT_RESP);
	printf("Enter to the IDLE state.\n");
//	udelay(1000);

	card = check_mmc_ocr();
	if(card == ERROR_CARD) {
		card = check_sd_ocr();		
		if(card == ERROR_CARD) {
			printf("Initialize fail & no card cassertion. \n");
			return 1;
		}
		else {
			regs = readl(SDICON) & ~CLOCK_TYPE_MMC;
			writel(regs | CLOCK_TYPE_SD, SDICON);
		}
	}

	// Get CID(Card Identification)
	while(mmc_cmd(MMC_ALL_SEND_CID, 0x0, LONG_RESP | WAIT_RESP));
	for( i = 0 ; i < 4 ; i++)
		resp[i] = readl(SDIRSP0+i*4);
	
	printf("Product Name : %c%c%c%c%c%c\n", resp[0]&0xff, (resp[1]>>24)&0xff, (resp[1]>>16)&0xff, (resp[1]>>8)&0xff, resp[1]&0xff, (resp[2]>>24)&0xff);

	// Send RCA(Relative Card Address). It places the card in the STBY state
	RCA = (card & MMC_CARD) ? 0x1 : 0x0;
	while(mmc_cmd(MMC_SET_RELATIVE_ADDR, RCA<<16, SHORT_RESP | WAIT_RESP));

	if (!RCA) {
		RCA= (readl(SDIRSP0) >> 16) & 0xFFFF;
		printf("=>  RCA=0x%08x\n", RCA);
	}

	// Get CSD(Card Specific Data)
	while(mmc_cmd(MMC_SEND_CSD, RCA<<16, LONG_RESP | WAIT_RESP));
	display_card_info();

	// Select Card
	while(mmc_cmd(MMC_SELECT_CARD, RCA<<16, SHORT_RESP | WAIT_RESP));

	if(check_card_status())
		DBG("error of status\n");

	clock_config();
	
	set_bus_width();
	
	while(mmc_cmd(MMC_SET_BLOCKLEN, MMC_BLOCK_SIZE, SHORT_RESP | WAIT_RESP));
	
	return 0;
}

int
/****************************************************/
mmc_block_read(uchar *dst, ulong src, ulong len)
/****************************************************/
{
	ushort cmd;	
	uint read_count = 0;
	uint sdi_dcon = 0;
	u32* buf32 = (u32*)dst;

	len /= MMC_BLOCK_SIZE;
	
	DBG("mmc_block_read: 0x%p, %d, %d\n", dst, src, len);

	while(check_card_status());

	writel(CRC_STAT_FAIL | RECEIVE_CRC_FAIL | DATA_TOUT | DATA_FINISH | BUSY_FINISH, SDIDSTA);

	writel(readl(SDIFSTA) | FIFO_RESET, SDIFSTA);
	udelay(100);

	sdi_dcon = readl(SDIDCON);
	sdi_dcon |= WORD_TRANSFER | RCV_AFTER_CMD | BLOCK_TRANSFER | (len);
	sdi_dcon |= DATA_TRANSMIT_START | DATA_RECEIVE_MODE;
	writel(sdi_dcon, SDIDCON);
	
	cmd = (len > 2) ? MMC_READ_MULTIPLE_BLOCK : MMC_READ_SINGLE_BLOCK;

	while(mmc_cmd(cmd, src, SHORT_RESP | WAIT_RESP));

	DBG("Data reading... \n");

	while(read_count < (len * MMC_BLOCK_SIZE / 4)) {
		if(readl(SDIDSTA) & DATA_TOUT) {
			DBG("Data status: 0x%x(DATA timeout) \n", readl(SDIDSTA));
			writel(readl(SDIDSTA) | DATA_TOUT, SDIDSTA);
			break;
		}

		if(readl(SDIFSTA) & RX_FIFO_AVAILABLE_DETECT) {
			*buf32++ = readl(SDIDAT);
			read_count++;
			DBG(".");
		}
	}

//	if(len > 2)
//		while(mmc_cmd(MMC_STOP_TRANSMISSION, 0x0, SHORT_RESP | WAIT_RESP));

	DBG("\nread done. \n");
	return 0;
}

int
/****************************************************/
mmc_block_write(ulong dst, uchar *src, int len)
/****************************************************/
{
	ushort cmd;	
	uint write_count = 0;
	uint sdi_dcon = 0;
	u32* buf32 = (u32*)src;
	
	len /= MMC_BLOCK_SIZE;
	
	DBG("mmc_block_write: %d, 0x%p, %d\n", dst, src, len);

	while(check_card_status());

	writel(CRC_STAT_FAIL | RECEIVE_CRC_FAIL | DATA_TOUT | DATA_FINISH | BUSY_FINISH, SDIDSTA);

	writel(readl(SDIFSTA) | FIFO_RESET, SDIFSTA);
	udelay(100);

	sdi_dcon = readl(SDIDCON);
	sdi_dcon |= WORD_TRANSFER | TRANS_AFTER_RESP | BLOCK_TRANSFER;
	sdi_dcon |= DATA_TRANSMIT_START | DATA_TRANSMIT_MODE | (len);
	writel(sdi_dcon, SDIDCON);

	cmd = (len < 2) ? MMC_WRITE_BLOCK :MMC_WRITE_MULTIPLE_BLOCK;

	while(mmc_cmd(cmd, dst, SHORT_RESP | WAIT_RESP));

	while(write_count < (len * MMC_BLOCK_SIZE / 4)) {
		if(readl(SDIFSTA) & TX_FIFO_AVAILABLE_DETECT) {
			if(readl(SDIDSTA) & DATA_TOUT) {
				DBG("Data status: 0x%x(DATA timeout) \n", readl(SDIDSTA));
				writel(readl(SDIDSTA) | DATA_TOUT, SDIDSTA);
				break;
			}
			else {
				writel(*buf32++, SDIDAT);
				write_count++;
				DBG(".");
			}
			udelay(1);
		}
	}

	writel(readl(SDIDCON) & ~(DATA_TRANSMIT_START| DATA_TRANSMIT_MODE), SDIDCON);	

	if(len > 1) {
		while(mmc_cmd(MMC_STOP_TRANSMISSION, 0x0, SHORT_RESP | WAIT_RESP));
	}
	
	writel(readl(SDIDSTA) | DATA_FINISH, SDIDSTA);

	DBG("\nwrite done.\n",);
	return 0;
}


int
/****************************************************/
mmc_read(ulong src, uchar *dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0) {
		return 0;
	}
	DBG("mmc_read : 0x%08x, 0x%p, 0x%08x\n", src, dst, size);

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	src -= CFG_MMC_BASE;
	end = src + size;
	part_start = ~mmc_block_address & src;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & src;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf+part_start, part_len);
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; src < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read((uchar *)(dst), src, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && src < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf, part_end);
	}
	return 0;
}

int
/****************************************************/
mmc_write(uchar *src, ulong dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0) {
		return 0;
	}
	DBG("mmc_write : 0x%p, 0x%08x, 0x%08x\n", src, dst, size);
	
	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	dst -= CFG_MMC_BASE;
	end = dst + size;
	part_start = ~mmc_block_address & dst;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & dst;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		(ulong)src, dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf+part_start, src, part_len);
		if ((mmc_block_write(aligned_start, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; dst < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_write(dst, (uchar *)src, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && dst < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf, src, part_end);
		if ((mmc_block_write(aligned_end, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
	}
	return 0;
}

ulong
/****************************************************/
mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst)
/****************************************************/
{
	int mmc_block_size = MMC_BLOCK_SIZE;
	ulong src = blknr * mmc_block_size + CFG_MMC_BASE;

	mmc_read(src, (uchar *)dst, blkcnt*mmc_block_size);
	return blkcnt;
}

int mmc_init(int verbose)
{
 	int rc = -ENODEV;

	if(mmc_setting())
		return rc;
	else
		rc = 0;
	
	mmc_ready = 1;

	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	/* FIXME fill in the correct size (is set to 32MByte) */
	mmc_dev.blksz = MMC_BLOCK_SIZE;
	mmc_dev.lba = 0x10000;
	sprintf(mmc_dev.vendor,"Man %02x%02x%02x Snr %02x%02x%02x",
			0x00, 0x1, 0x2,
			0x1, 0x2, 0x3);
	sprintf(mmc_dev.product,"%s","SD/MMC");
	sprintf(mmc_dev.revision,"%x %x",0x1, 0x1);
	mmc_dev.removable = 0;
	mmc_dev.block_read = mmc_bread;

	printf("before fat_register_device() \n");
	fat_register_device(&mmc_dev,1);			/* partitions start counting with 1 */
	
	return rc;
}

int
mmc_ident(block_dev_desc_t *dev)
{
	return 0;
}

int
mmc2info(ulong addr)
{
	/* FIXME hard codes to 32 MB device */
	if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + 0x02000000) {
		return 1;
	}
	return 0;
}

#if 0	/* test of R/W function */
int test_mmc (uint start_blk, uint blknum, uint tag)
{
	int i;
	uint *txbuf = (uint*)SDI_Tx_buffer_HSMMC;

	DBG("test_mmc: start_blk=0x%x, blknum=%d ", start_blk, blknum, tag);
	if(tag)
		DBG("writing to MMC \n");
	else
		DBG("reading from MMC \n");

	for (i = 0; i < (512 * blknum) / 4; i++) {
		*(txbuf + i) = i + 2;
	}

	if(tag) {
		DBG("Test of writing to MMC from 0x31000000 \n");
		mmc_block_write((ulong)start_blk, SDI_Tx_buffer_HSMMC, blknum*512);
	}
	else {
		DBG("Test of reading from MMC to 0x31300000 \n");
		mmc_block_read(SDI_Rx_buffer_HSMMC, (ulong)start_blk, (ulong)blknum*512);
	}
	return 0;
}
#endif
#endif	/* CONFIG_MMC */
