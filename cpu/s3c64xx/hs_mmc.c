#include <common.h>

#if defined(CONFIG_SMDK6400) || defined(CONFIG_SMDK6410) || defined(CONFIG_SMDK6430)

#define CONFIG_SUPPORT_MMC_PLUS
#define HCLK_OPERATION
#undef DEBUG_HSMMC
#ifdef DEBUG_HSMMC
#define dbg(x...)       printf(x)
#else
#define dbg(x...)       do { } while (0)
#endif

#include <regs.h>

#include <linux/mmc/mmc.h>
#include <linux/mmc/protocol.h>
#include <asm/io.h>
#include <movi.h>

#include "hs_mmc.h"

static ulong mmc_bread (int dev_num, ulong blknr, ulong blkcnt, ulong* dst);

#if defined(CONFIG_S3C6400)
extern ulong virt_to_phy_smdk6400(ulong addr);
#elif defined(CONFIG_S3C6410)
extern ulong virt_to_phy_smdk6410(ulong addr);
#elif defined(CONFIG_S3C6430)
extern ulong virt_to_phy_smdk6430(ulong addr);
#endif

#define SDI_Tx_buffer_HSMMC		(0x51000000)
#define SDI_Rx_buffer_HSMMC		(0x51000000+(0x300000))
#define SDI_Compare_buffer_HSMMC	(0x51000000+(0x600000))

#define Card_OneBlockSize_ver1		512
#define MMC_BLOCK_SIZE Card_OneBlockSize_ver1

/*  Global variables */
int movi_ch = 1;
static vu_long regs;
static uint *Tx_buffer_HSMMC;
static uint *Rx_buffer_HSMMC;
static uint *Compare_buffer_HSMMC;

static vu_long rd_cnt_HSMMC;
static vu_long wt_cnt_HSMMC;
static vu_long BlockNum_HSMMC = 0;

static vu_long WriteBlockCnt_INT = 0;
static vu_long ReadBlockCnt_INT = 0;
static vu_long WRITEINT_DONE = 0;
static vu_long READINT_DONE = 0;
static vu_long COMPARE_INT_DONE = 0;
static vu_long CompareCnt_INT = 0;
static vu_long BufferBoundary_INT_Cnt = 0;

static vu_long HS_DMA_END = 0;
static vu_long HS_CARD_DETECT = 0;

static uint ocr_check = 0;
static uint mmc_card = 0;
static uint rca = 0;
static uint mmc_spec, sd_spec;

static ulong HCLK;
static uint card_mid = 0;

/* extern functions */
extern ulong get_HCLK(void);

static block_dev_desc_t mmc_dev;

block_dev_desc_t* mmc_get_dev(int dev)
{
	return( (block_dev_desc_t*) &mmc_dev);
}

#if 0
static int wait_for_w_buf_ready (void)
{
	uint uLoop = 0;

	while (!(rHM_NORINTSTS & 0x10)) {
		if (uLoop % 500000 == 0 && uLoop > 0) {
			return 0;
		}
		uLoop++;
	}
	return 1;
}
#endif

static int wait_for_r_buf_ready (void)
{
	uint uLoop = 0;

	while (!(readw(regs + HM_NORINTSTS) & 0x20)) {
		if (uLoop % 500000 == 0 && uLoop > 0) {
			return 0;
		}
		uLoop++;
	}
	return 1;
}

static int wait_for_cmd_done (void)
{
	uint i;
	ushort n_int, e_int;

	udelay(5000);

	dbg("wait_for_cmd_done\n");
	for (i = 0; i < 0x20000000; i++) {
		n_int = readw(regs + HM_NORINTSTS);
		dbg("  HM_NORINTSTS: %04x\n", n_int);
		if (n_int & 0x8000) break;
		if (n_int & 0x0001) return 0;
	}

	e_int = readw(regs + HM_ERRINTSTS);
	writew(e_int, regs + HM_ERRINTSTS);
	writew(n_int, regs + HM_NORINTSTS);
	dbg("cmd error1: 0x%04x, HM_NORINTSTS: 0x%04x\n", e_int, n_int);
	return 1;
}

/* XXX: must modify algorithm. it has bugs. by scsuh */
static int wait_for_data_done (void)
{
	while (!(readw(regs + HM_NORINTSTS) & 0x2))
		return 1;

	return 0;
}

static void ClearCommandCompleteStatus(void)
{
	writew(1 << 0, regs + HM_NORINTSTS);
	while (readw(regs + HM_NORINTSTS) & 0x1) {
		writew(1 << 0, regs + HM_NORINTSTS);
	}
}

static void ClearTransferCompleteStatus(void)
{
	writew(readw(regs + HM_NORINTSTS) | (1 << 1), regs + HM_NORINTSTS);
	while (readw(regs + HM_NORINTSTS) & 0x2) {
		writew(readw(regs + HM_NORINTSTS) | (1 << 1), regs + HM_NORINTSTS);
	}
}

#if 0
static void ClearBufferWriteReadyStatus(void)
{
	rHM_NORINTSTS |= (1 << 4);
	while (rHM_NORINTSTS & 0x10)
		rHM_NORINTSTS |= (1 << 4);
}
#endif

static void ClearBufferReadReadyStatus(void)
{
	writew(readw(regs + HM_NORINTSTS) | (1 << 5), regs + HM_NORINTSTS);
	while (readw(regs + HM_NORINTSTS) & 0x20)
		writew(readw(regs + HM_NORINTSTS) | (1 << 5), regs + HM_NORINTSTS);
}

#if 0
static void HS_MMC_CardDetect(void)
{
	rGPJCON = (rGPJCON & ~(0x3 << 28)) | (1 << 29);
	//rHM_CONTROL2 |= (1<<13)|(1<<12);/* Card Detect using a DAT[3] signal */

	rHM_NORINTSTSEN |= (1 << 7) | (1 << 6);
	rHM_NORINTSIGEN |= (1 << 7) | (1 << 6);

	if (rHM_PRNSTS & (1 << 16)) {
		HS_CARD_DETECT = 1;
		printf("\nCard is Inserted\n");
	}

	while (!HS_CARD_DETECT) {
		printf("\nInsert a Card\n");
		HS_CARD_DETECT = 0;
		do {
			printf(".");
			udelay(500000);
		} while (HS_CARD_DETECT == 0);
	}

	HS_CARD_DETECT = 0;
	rHM_CONTROL2 &= ~(1 << 13);
}
#endif

static void card_irq_enable(ushort temp)
{
	writew((readw(regs + HM_NORINTSTSEN) & 0xFEFF) | (temp << 8), regs + HM_NORINTSTSEN);
}

void hsmmc_set_base (void)
{
	regs = ELFIN_HSMMC_BASE + (movi_ch * 0x100000);
}

void hsmmc_reset (void)
{
	writeb(0x3, regs + HM_SWRST);
}

void hsmmc_set_gpio (void)
{
	u32 reg;

	if (movi_ch == 0) {
		reg = readl(GPGCON) & 0xf0000000;
		writel(reg | 0x02222222, GPGCON);

		reg = readl(GPGPUD) & 0xfffff000;
		writel(reg, GPGPUD);
	} else if (movi_ch == 1) {
		writel(0x00222222, GPHCON0);
		writel(0x00000000, GPHCON1);

		reg = readl(GPHPUD) & 0xfffff000;
		writel(reg, GPHPUD);
	} else
		printf("### HS-MMC channel is not defined!\n");
}

static void set_transfer_mode_register (uint MultiBlk, uint DataDirection, uint AutoCmd12En, uint BlockCntEn, uint DmaEn)
{
	writew((readw(regs + HM_TRNMOD) & ~(0xffff)) | (MultiBlk << 5)
		| (DataDirection << 4) | (AutoCmd12En << 2)
		| (BlockCntEn << 1) | (DmaEn << 0), regs + HM_TRNMOD);
	dbg("\nHM_TRNMOD = 0x%04x\n", HM_TRNMOD);
}

static void set_arg_register (uint arg)
{
	writel(arg, regs + HM_ARGUMENT);
}

static void set_blkcnt_register(ushort uBlkCnt)
{
	writew(uBlkCnt, regs + HM_BLKCNT);
}

static void SetSystemAddressReg(uint SysAddr)
{
	writel(SysAddr, regs + HM_SYSAD);
}

static void set_blksize_register(ushort uDmaBufBoundary, ushort uBlkSize)
{
	writew((uDmaBufBoundary << 12) | (uBlkSize), regs + HM_BLKSIZE);
}

static void ClearErrInterruptStatus(void)
{
	while (readw(regs + HM_NORINTSTS) & (0x1 << 15)) {
		writew(readw(regs + HM_NORINTSTS), regs + HM_NORINTSTS);
		writew(readw(regs + HM_ERRINTSTS), regs + HM_ERRINTSTS);
	}
}

static void InterruptEnable(ushort NormalIntEn, ushort ErrorIntEn)
{
	ClearErrInterruptStatus();
	writew(NormalIntEn, regs + HM_NORINTSTSEN);
	writew(ErrorIntEn, regs + HM_ERRINTSTSEN);
}

static void hsmmc_clock_onoff (int on)
{
	u16 reg16;

	if (on == 0) {
		reg16 = readw(regs + HM_CLKCON) & ~(0x1<<2);
		writew(reg16, regs + HM_CLKCON);
	} else {
		reg16 = readw(regs + HM_CLKCON);
		writew(reg16 | (0x1<<2), regs + HM_CLKCON);

		while (1) {
			reg16 = readw(regs + HM_CLKCON);
			if (reg16 & (0x1<<3))	/*  SD_CLKSRC is Stable */
				break;
		}
	}
}

static void set_clock (uint clksrc, uint div)
{
	u16 reg16;
	uint i;

#if defined(CONFIG_S3C6400)
	writel(0xC0000100 | (clksrc << 4), regs + HM_CONTROL2);	// feedback control off
	writel(0x00000000, regs + HM_CONTROL3);
#else
	writel(0xC0004100 | (clksrc << 4), regs + HM_CONTROL2);	// rx feedback control
	writel(0x00008080, regs + HM_CONTROL3); 			// Low clock: 00008080
	writel(0x3 << 16, regs + HM_CONTROL4);
#endif

	writew(readw(regs + HM_CLKCON) & ~(0xff << 8), regs + HM_CLKCON);

	/* SDCLK Value Setting + Internal Clock Enable */
	writew(((div<<8) | 0x1), regs + HM_CLKCON);

	/* CheckInternalClockStable */
	for (i=0; i<0x10000; i++) {
		reg16 = readw(regs + HM_CLKCON);
		if (reg16 & 0x2)
			break;
	}
	if (i == 0x10000)
		printf("internal clock stabilization failed\n");

	dbg("HM_CONTROL2(0x80) = 0x%08x\n", readl(regs + HM_CONTROL2));
	dbg("HM_CONTROL3(0x84) = 0x%08x\n", readl(regs + HM_CONTROL3));
	dbg("HM_CLKCON  (0x2c) = 0x%04x\n", readw(regs + HM_CLKCON));

	hsmmc_clock_onoff(1);
}

static void set_cmd_register (ushort cmd, uint data, uint flags)
{
	ushort val = (cmd << 8);

	if (cmd == 12)
		val |= (3 << 6);

	if (flags & MMC_RSP_136)	/* Long RSP */
		val |= 0x01;
	else if (flags & MMC_RSP_BUSY)	/* R1B */
		val |= 0x03;
	else if (flags & MMC_RSP_PRESENT)	/* Normal RSP */
		val |= 0x02;

	if (flags & MMC_RSP_OPCODE)
		val |= (1<<4);

	if (flags & MMC_RSP_CRC)
		val |= (1<<3);

	if (data)
		val |= (1<<5);

	dbg("cmdreg =  0x%04x\n", val);
	writew(val, regs + HM_CMDREG);
}

static int issue_command (ushort cmd, uint arg, uint data, uint flags)
{
	int i;

	dbg("### issue_command: %d, %08x, %d, %08x\n", cmd, arg, data, flags);
	/* Check CommandInhibit_CMD */
	for (i=0; i<0x1000000; i++) {
		if (!(readl(regs + HM_PRNSTS) & 0x1))
			break;
	}
	if (i == 0x1000000) {
		printf("@@@@@@1 rHM_PRNSTS: %08x\n", readl(regs + HM_PRNSTS));
	}

	/* Check CommandInhibit_DAT */
	if (flags & MMC_RSP_BUSY) {
		for (i=0; i<0x1000000; i++) {
			if (!(readl(regs + HM_PRNSTS) & 0x2))
				break;
		}
		if (i == 0x1000000) {
			printf("@@@@@@2 rHM_PRNSTS: %08x\n", readl(regs + HM_PRNSTS));
		}
	}

	writel(arg, regs + HM_ARGUMENT);

	set_cmd_register(cmd, data, flags);

	if (wait_for_cmd_done())
		return 0;

	ClearCommandCompleteStatus();

	if (!(readw(regs + HM_NORINTSTS) & 0x8000)) {
		return 1;
	} else {
		if (ocr_check == 1)
			return 0;
		else {
			printf("Command = %d, Error Stat = 0x%04x\n", (readw(regs + HM_CMDREG) >> 8), readw(regs + HM_ERRINTSTS));
			return 0;
		}
	}

}

static void set_mmc_speed (uint eSDSpeedMode)
{
	uint ucSpeedMode, arg;

	ucSpeedMode = (eSDSpeedMode == HIGH) ? 1 : 0;
	arg = (3 << 24) | (185 << 16) | (ucSpeedMode << 8);	/*  Change to the high-speed mode */
	while (!issue_command(MMC_SWITCH, arg, 0, MMC_RSP_R1B));
}

static void set_sd_speed (uint eSDSpeedMode)
{
	uint temp;
	uint arg = 0;
	uchar ucSpeedMode;
	int i;

	ucSpeedMode = (eSDSpeedMode == HIGH) ? 1 : 0;

	if (!issue_command(MMC_SET_BLOCKLEN, 64, 0, MMC_RSP_R1)) {
		printf("CMD16 fail\n");
	} else {
		set_blksize_register(7, 64);
		set_blkcnt_register(1);
		set_arg_register(0 * 64);

		set_transfer_mode_register(0, 1, 0, 0, 0);

		arg = (0x1 << 31) | (0xffff << 8) | (ucSpeedMode << 0);

		if (!issue_command(MMC_SWITCH, arg, 0, MMC_RSP_R1B))
			printf("CMD6 fail\n");
		else {
			wait_for_r_buf_ready();
			ClearBufferReadReadyStatus();

			for (i = 0; i < 16; i++) {
				temp = readl(regs + HM_BDATA);
			}

			if (!wait_for_data_done())
				printf(("Transfer NOT Complete\n"));

			ClearTransferCompleteStatus();
		}
	}
}

static int get_sd_scr (void)
{
	uint uSCR1, uSCR2;

	if (!issue_command(MMC_SET_BLOCKLEN, 8, 0, MMC_RSP_R1))
		return 0;
	else {
		set_blksize_register(7, 8);
		set_blkcnt_register(1);
		set_arg_register(0 * 8);

		set_transfer_mode_register(0, 1, 0, 0, 0);
		if (!issue_command(MMC_APP_CMD, rca<<16, 0, MMC_RSP_R1))
			return 0;
		else {
			if (!issue_command(SD_APP_SEND_SCR, 0, 1, MMC_RSP_R1))
				return 0;
			else {
				wait_for_r_buf_ready();
				ClearBufferReadReadyStatus();

				uSCR1 = readl(regs + HM_BDATA);
				uSCR2 = readl(regs + HM_BDATA);

				if (!wait_for_data_done())
					printf(("Transfer NOT Complete\n"));

				ClearTransferCompleteStatus();

				if (uSCR1 & 0x1)
					sd_spec = 1;	/*  Version 1.10, support cmd6 */
				else
					sd_spec = 0;	/*  Version 1.0 ~ 1.01 */

				dbg("sd_spec = %d(0x%08x)\n", sd_spec, uSCR1);
				return 1;
			}
		}
	}
}

static int check_card_status(void)
{
	if (!issue_command(MMC_SEND_STATUS, rca<<16, 0, MMC_RSP_R1)) {
		return 0;
	} else {
		if (((readl(regs + HM_RSPREG0) >> 9) & 0xf) == 4) {
			dbg("Card is transfer status\n");
			return 1;
		}
	}

	return 1;
}

static void set_hostctl_speed (uchar mode)
{
	u8 reg8;

	reg8 = readb(regs + HM_HOSTCTL) & ~(0x1<<2);
	writeb(reg8 | (mode<<2), regs + HM_HOSTCTL);
}

/* return 0: OK
 * return -1: error
 */
static int set_bus_width (uint width)
{
	uint arg = 0;
	uchar reg = readb(regs + HM_HOSTCTL);
	uchar bitmode = 0;

	dbg("bus width: %d\n", width);

	switch (width) {
	case 8:
		width = mmc_card ? 8 : 4;
		break;
	case 4:
	case 1:
		break;
	default:
		return -1;
	}

	card_irq_enable(0);	// Disable sd card interrupt

	if (mmc_card) { /* MMC Card */
		/* MMC Spec 4.x or above */
		if (mmc_spec == 4) {
			if (width == 1) bitmode = 0;
			else if (width == 4) bitmode = 1;
			else if (width == 8) bitmode = 2;
			else {
				printf("#### unknown mode\n");
				return -1;
			}

			arg = ((3 << 24) | (183 << 16) | (bitmode << 8));
			while (!issue_command(MMC_SWITCH, arg, 0, MMC_RSP_R1B));
		} else
			bitmode = 0;
	} else { /* SD Card */
		if (!issue_command(MMC_APP_CMD, rca<<16, 0, MMC_RSP_R1))
			return -1;
		else {
			if (width == 1) {	// 1-bits
				bitmode = 0;
				if (!issue_command(MMC_SWITCH, 0, 0, MMC_RSP_R1B))
					return -1;
			} else {	// 4-bits
				bitmode = 1;
				if (!issue_command(MMC_SWITCH, 2, 0, MMC_RSP_R1B))
					return -1;
			}
		}
	}

	if (bitmode == 2)
		reg |= 1 << 5;
	else
		reg |= bitmode << 1;

	writeb(reg, regs + HM_HOSTCTL);
	card_irq_enable(1);
	dbg(" transfer rHM_HOSTCTL(0x28) = 0x%02x\n", readb(regs + HM_HOSTCTL));

	return 0;
}

static int set_sd_ocr (void)
{
	uint i, ocr;

	issue_command(MMC_APP_CMD, 0x0, 0, MMC_RSP_R1);
	issue_command(SD_APP_OP_COND, 0x0, 0, MMC_RSP_R3);
	ocr = readl(regs + HM_RSPREG0);
	dbg("ocr1: %08x\n", ocr);

	for (i = 0; i < 100; i++) {
		issue_command(MMC_APP_CMD, 0x0, 0, MMC_RSP_R1);
		issue_command(SD_APP_OP_COND, ocr, 0, MMC_RSP_R3);

		ocr = readl(regs + HM_RSPREG0);
		dbg("ocr2: %08x\n", ocr);
		if (ocr & (0x1 << 31)) {
			dbg("Voltage range: ");
			if (ocr & (1 << 21))
				dbg("2.7V ~ 3.4V");
			else if (ocr & (1 << 20))
				dbg("2.7V ~ 3.3V");
			else if (ocr & (1 << 19))
				dbg("2.7V ~ 3.2V");
			else if (ocr & (1 << 18))
				dbg("2.7V ~ 3.1V");

			if (ocr & (1 << 7))
				dbg(", 1.65V ~ 1.95V\n");
			else
				dbg('\n');

			mmc_card = 0;
			return 1;
		}
		udelay(1000);
	}

	// The current card is MMC card, then there's time out error, need to be cleared.
	ClearErrInterruptStatus();
	return 0;
}

static int set_mmc_ocr (void)
{
	uint i, ocr;

	for (i = 0; i < 100; i++) {
		issue_command(MMC_SEND_OP_COND, 0x40FF8000, 0, MMC_RSP_R3);

		ocr = readl(regs + HM_RSPREG0);
		dbg("ocr1: %08x\n", ocr);

		if (ocr & (0x1 << 31)) {
			dbg("Voltage range: ");
			if (ocr & (1 << 21))
				dbg("2.7V ~ 3.4V");
			else if (ocr & (1 << 20))
				dbg("2.7V ~ 3.3V");
			else if (ocr & (1 << 19))
				dbg("2.7V ~ 3.2V");
			else if (ocr & (1 << 18))
				dbg("2.7V ~ 3.1V");
			mmc_card = 1;
			if (ocr & (1 << 7))
				dbg(", 1.65V ~ 1.95V\n");
			else
				dbg('\n');
			return 1;
		}
	}

	// The current card is SD card, then there's time out error, need to be cleared.
	ClearErrInterruptStatus();
	return 0;
}

#ifdef CONFIG_SUPPORT_MMC_PLUS

static void clock_config (uint clksrc, uint div)
{
	uint base_freq = 0, freq;

	if (clksrc == SD_HCLK)
		base_freq = HCLK;
	else if (clksrc == SD_EPLL)	/* Epll Out 48MHz */
		base_freq = 100000000;
	else
		clksrc = HCLK;

	freq = base_freq / (div * 2);
	dbg("freq = %dMHz\n", freq / (1000000));

	if (mmc_card) {
		if (mmc_spec == 4) {
			/* It is necessary to enable the high speed mode
			 * in the card before changing the clock freq
			 * to a freq higher than 20MHz.
			 */
			if (freq > 20000000) {
				set_mmc_speed(HIGH);
				dbg("Set MMC High speed mode OK!!\n");
			} else {
				set_mmc_speed(NORMAL);
				dbg("Set MMC Normal speed mode OK!!\n");
			}
		} else		// old version
			dbg("Old version MMC card can not support working frequency higher than 20MHz\n");
	} else {
		get_sd_scr();
		if (sd_spec == 1) {
			if (freq > 25000000) {
				set_sd_speed(HIGH);	//Higher than 25MHz, should use high speed mode. Max 50MHz and 25MB/sec
				dbg("Set SD High speed mode OK!!\n");
			} else {
				set_sd_speed(NORMAL);
				dbg("Set SD Normal speed mode OK!!\n");
			}
		} else
			dbg("Old version SD card can not support working frequency higher than 25MHz\n");
	}

	/* When Higher than 25MHz, it is necessary
	 * to enable high speed mode of the host controller.
	 */
	if (freq > 25000000) {
		set_hostctl_speed(HIGH);
	} else {
		set_hostctl_speed(NORMAL);
	}

	/* when change the sd clock frequency, need to stop sd clock. */
	hsmmc_clock_onoff(1);
	set_clock(clksrc, div);
	dbg("clock config rHM_HOSTCTL(0x28) = 0x%02x\n", readb(regs + HM_HOSTCTL));

}
#else

static void clock_config (uint clksrc, uint Divisior)
{
	uint SrcFreq, WorkingFreq;

	if (clksrc == SD_HCLK)
		base_freq = HCLK;
	else if (clksrc == SD_EPLL)	/* Epll Out 48MHz */
		base_freq = 100000000;
	else
		clksrc = HCLK;

	WorkingFreq = SrcFreq / (Divisior * 2);
	printf("Card Working Frequency = %dMHz\n", WorkingFreq / (1000000));

	if (mmc_card) {
		if (mmc_spec == 4) {
			if (WorkingFreq > 20000000) {	// It is necessary to enable the high speed mode in the card before changing the clock freq to a freq higher than 20MHz.
				set_mmc_speed(HIGH);
				printf("\nSet MMC High speed mode OK!!\n");
			} else {
				set_mmc_speed(NORMAL);
				printf("\nSet MMC Normal speed mode OK!!\n");
			}
		} else		// old version
			printf("Old version MMC card can not support working frequency higher than 25MHz");
	}

	if (WorkingFreq > 25000000) {	// Higher than 25MHz, it is necessary to enable high speed mode of the host controller.
		set_hostctl_speed(HIGH);
	} else {
		set_hostctl_speed(NORMAL);
	}

	hsmmc_clock_onoff(0);		// when change the sd clock frequency, need to stop sd clock.
	set_clock(clksrc, Divisior);
	printf("clock config rHM_HOSTCTL = 0x%02x\n", readb(regs + HM_HOSTCTL));

}
#endif

static void check_dma_int (void)
{
	uint i;

	for (i = 0; i < 0x10000000; i++) {
		if (readw(regs + HM_NORINTSTS) & 0x0002) {
			dbg("Transfer Complete\n");
			HS_DMA_END = 1;
			writew(readw(regs + HM_NORINTSTS) | 0x0002, regs + HM_NORINTSTS);
			break;
		}

		if (readw(regs + HM_NORINTSTS) & 0x8000) {
			printf("error found: %04x\n", readw(regs + HM_ERRINTSTS));
			break;
		}
	}
}

static uint process_ext_csd (void)
{
	u8 ext_csd[512];

	memset(ext_csd, 0, sizeof(ext_csd));

	if (ext_csd >= (u8 *)0xc0000000)
		SetSystemAddressReg(virt_to_phys((ulong)ext_csd));
	else
		SetSystemAddressReg((ulong)ext_csd);

	set_blksize_register(7, 512);
	set_blkcnt_register(1);
	set_transfer_mode_register(0, 1, 0, 1, 1);

	while(!issue_command(MMC_SEND_EXT_CSD, 0, 1, MMC_RSP_R1 | MMC_CMD_ADTC));

	check_dma_int();
	while (!HS_DMA_END);

	return (((ext_csd[215] << 24) | (ext_csd[214] << 16) | (ext_csd[213] << 8) | ext_csd[212]) / (2 * 1024));
}

static void display_card_info (void)
{
	uint card_size;

	if (mmc_card && movi_hc) {
		card_size = process_ext_csd();
	} else {
		uint i, resp[4];
		uint c_size, c_size_multi, read_bl_len, read_bl_partial, blk_size;

		for (i=0; i<4; i++) {
			resp[i] = readl(regs + HM_RSPREG0+i*4);
			dbg("%08x\n", resp[i]);
		}

		read_bl_len = ((resp[2] >> 8) & 0xf);
		read_bl_partial = ((resp[2] >> 7) & 0x1);
		blk_size = (1 << read_bl_len);

		dbg(" read_bl_len: %d\n", read_bl_len);
		dbg(" read_bl_partial: %d\n", read_bl_partial);
		dbg(" One Block Size: %d Byte\n", blk_size);

		if (movi_hc) {
			card_size = ((resp[1] >> 8) & 0x3fffff) / 2;
		} else {
			c_size_multi = ((resp[1] >> 7) & 0x7);
			c_size = ((resp[2] & 0x3) << 10) | ((resp[1] >> 22) & 0x3ff);
			card_size = (1 << read_bl_len) * (c_size + 1) * (1 << (c_size_multi + 2)) / 1048576;

			dbg(" c_size: %d\n", c_size);
			dbg(" c_size_multi: %d\n", c_size_multi);
		}

		dbg(" Total Card Size: %d MByte\n\n", card_size + 1);
	}

	printf("%d MB ", card_size + 1);

	if (!mmc_card && movi_hc)
		printf("(SDHC)");

	if (card_mid == 0x15)
		printf("(moviNAND)");

	printf("\n");
}

static void DataRead_ForCompare (int StartAddr)
{
	uint i = 0, j = 0;
	COMPARE_INT_DONE = 0;

	writew(readw(regs + HM_NORINTSIGEN) & ~(0xffff), regs + HM_NORINTSIGEN);

	Compare_buffer_HSMMC = (uint *) SDI_Compare_buffer_HSMMC;
	for (i = 0; i < (512 * BlockNum_HSMMC) / 4; i++)
		*(Compare_buffer_HSMMC + i) = 0x0;

	dbg("Polling mode data read1\n");
	dbg("Read BlockNum = %d\n", BlockNum_HSMMC);

	while (!check_card_status());

	/*  Maximum DMA Buffer Size, Block Size */
	set_blksize_register(7, 512);
	/*  Block Numbers to Write */
	set_blkcnt_register(BlockNum_HSMMC);

	if (movi_hc)
		set_arg_register(StartAddr);
	else
		set_arg_register(StartAddr * 512);

	if (BlockNum_HSMMC == 1) {
		dbg("Single block read\n");
		set_transfer_mode_register(0, 1, 1, 1, 0);
		/* MMC_READ_SINGLE_BLOCK */
		set_cmd_register(17, 1, MMC_RSP_R1);
	} else {
		dbg("Multi block read\n");
		set_transfer_mode_register(1, 1, 1, 1, 0);
		/* MMC_READ_MULTIPLE_BLOCK */
		set_cmd_register(18, 1, MMC_RSP_R1);
	}

	if (wait_for_cmd_done()) {
		printf("Command is NOT completed1\n");
	}
	ClearCommandCompleteStatus();

	for (j = 0; j < BlockNum_HSMMC; j++) {
		if (!wait_for_r_buf_ready())
			printf("ReadBuffer NOT Ready\n");
		else
			ClearBufferReadReadyStatus();
		for (i = 0; i < 512 / 4; i++) {
			*Compare_buffer_HSMMC++ = readl(regs + HM_BDATA);
			CompareCnt_INT++;
		}
	}

	dbg("Read count=0x%08x\n", CompareCnt_INT);
	if (!wait_for_data_done()) {
		printf(("Transfer NOT Complete\n"));
	}
	ClearTransferCompleteStatus();

	dbg("\n\nHM_NORINTSTS = %x", readw(regs + HM_NORINTSTS));
}

static void DataCompare_HSMMC (uint a0, uint a1, uint bytes)
{
	uint *pD0 = (uint *) a0;
	uint *pD1 = (uint *) a1;
	uint ErrCnt = 0;
	uint i;

	for (i = 0; i < bytes; i++) {
		if (*pD0 != *pD1) {
			dbg("\n%08x=%02x <-> %08x=%02x", pD0, *pD0, pD1, *pD1);
			ErrCnt++;
		}
		pD0++;
		pD1++;
	}
	dbg("\nTotal Error cnt = %d", ErrCnt);

	if (ErrCnt == 0)
		dbg("\nData Compare Ok\n");
}

int hsmmc_init (void)
{
	u32 reg;
	uint width;

	width = 4;

	HCLK = get_HCLK();
	dbg("HCLK = %08lu\n", HCLK);

	hsmmc_clock_onoff(0);

	reg = readl(SCLK_GATE);
	writel(reg | (1<<27), SCLK_GATE);

	set_clock(SD_EPLL, 0x80);
	writeb(0xe, regs + HM_TIMEOUTCON);
	set_hostctl_speed(NORMAL);

	InterruptEnable(0xff, 0xff);

	dbg("HM_NORINTSTS = %x\n", readw(regs + HM_NORINTSTS));

	/* MMC_GO_IDLE_STATE */
	issue_command(MMC_GO_IDLE_STATE, 0x00, 0, 0);

	ocr_check = 1;

	if (set_mmc_ocr()) {
		mmc_card = 1;
		dbg("MMC card is detected\n");
	} else if (set_sd_ocr()) {
		mmc_card = 0;
		dbg("SD card is detected\n");
	} else {
		printf("0 MB\n");
		return 0;
	}

	ocr_check = 0;

	/* Check the attached card and place the card
	 * in the IDENT state rHM_RSPREG0
	 */
	issue_command(MMC_ALL_SEND_CID, 0, 0, MMC_RSP_R2);

	/* Manufacturer ID */
	card_mid = (readl(regs + HM_RSPREG3) >> 16) & 0xFF;

	dbg("Product Name : %c%c%c%c%c%c\n", ((readl(regs + HM_RSPREG2) >> 24) & 0xFF),
	       ((readl(regs + HM_RSPREG2) >> 16) & 0xFF), ((readl(regs + HM_RSPREG2) >> 8) & 0xFF), (readl(regs + HM_RSPREG2) & 0xFF),
	       ((readl(regs + HM_RSPREG1) >> 24) & 0xFF), ((readl(regs + HM_RSPREG1) >> 16) & 0xFF));

	// Send RCA(Relative Card Address). It places the card in the STBY state
	rca = (mmc_card) ? 0x0001 : 0x0000;
	issue_command(MMC_SET_RELATIVE_ADDR, rca<<16, 0, MMC_RSP_R1);

	if (!mmc_card)
		rca = (readl(regs + HM_RSPREG0) >> 16) & 0xFFFF;

	dbg("Enter to the Stand-by State\n");

	issue_command(MMC_SEND_CSD, rca<<16, 0, MMC_RSP_R2);

	if (mmc_card) {
		mmc_spec = (readl(regs + HM_RSPREG3) >> 18) & 0xF;
		dbg("mmc_spec=%d\n", mmc_spec);
	}

	issue_command(MMC_SELECT_CARD, rca<<16, 0, MMC_RSP_R1);
	dbg("Enter to the Transfer State\n");

	display_card_info();

	/* Operating Clock setting */
	clock_config(SD_EPLL, 2);	// Divisor 1 = Base clk /2      ,Divisor 2 = Base clk /4, Divisor 4 = Base clk /8 ...

	while (set_bus_width(width));
	while (!check_card_status());

	/* MMC_SET_BLOCKLEN */
	while (!issue_command(MMC_SET_BLOCKLEN, 512, 0, MMC_RSP_R1));

	writew(0xffff, regs + HM_NORINTSTS);

	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.blksz = MMC_BLOCK_SIZE;
	sprintf(mmc_dev.vendor, "Man %02x %02x %02x Snr %02x %02x %02x", 0x00, 0x01, 0x02, 0x01, 0x02, 0x03);
	sprintf(mmc_dev.product, "%s", "MMC4");
	sprintf(mmc_dev.revision, "%x %x", 0x01, 0x01);
	mmc_dev.block_read = mmc_bread;

	return 0;
}

void movi_write (uint addr, uint start_blk, uint blknum)
{
	uint blksize;

	wt_cnt_HSMMC = 0;
	BlockNum_HSMMC = 0;
	wt_cnt_HSMMC = 0;
	WriteBlockCnt_INT = 0;
	HS_DMA_END = 0;

	BlockNum_HSMMC = blknum;
	blksize = Card_OneBlockSize_ver1;

	writew((readw(regs + HM_NORINTSTSEN) & ~(0xffff)) |
		BUFFER_READREADY_STS_INT_EN |
		BUFFER_WRITEREADY_STS_INT_EN |
		TRANSFERCOMPLETE_STS_INT_EN | COMMANDCOMPLETE_STS_INT_EN, regs + HM_NORINTSTSEN);

	writew((readw(regs + HM_NORINTSIGEN) & ~(0xffff)) | TRANSFERCOMPLETE_SIG_INT_EN, regs + HM_NORINTSIGEN);

	SetSystemAddressReg(addr);		// AHB System Address For Write
	set_blksize_register(7, 512);		// Maximum DMA Buffer Size, Block Size
	set_blkcnt_register(BlockNum_HSMMC);	// Block Numbers to Write

	if (movi_hc)
		set_arg_register(start_blk);
	else
		set_arg_register(start_blk * 512);

	if (BlockNum_HSMMC == 1) {
		set_transfer_mode_register(0, 0, 1, 1, 1);
		set_cmd_register(24, 1, MMC_RSP_R1);
	} else {
		set_transfer_mode_register(1, 0, 1, 1, 1);
		set_cmd_register(25, 1, MMC_RSP_R1);
	}

	if (wait_for_cmd_done()) {
		printf("\nCommand is NOT completed3\n");
	}
	ClearCommandCompleteStatus();

	/* wait for DMA transfer */
	check_dma_int();
	while (!HS_DMA_END);

	if (!wait_for_data_done()) {
		printf(("Transfer is NOT Complete\n"));
	}
	ClearTransferCompleteStatus();

	writew(readw(regs + HM_NORINTSTS) | (1 << 3), regs + HM_NORINTSTS);

	BlockNum_HSMMC = 0;
	wt_cnt_HSMMC = 0;
	WriteBlockCnt_INT = 0;
	HS_DMA_END = 0;
	BufferBoundary_INT_Cnt = 0;

	CompareCnt_INT = 0;
	Compare_buffer_HSMMC = 0;
}

void movi_read (uint addr, uint start_blk, uint blknum)
{
	uint blksize; //j, , Addr_temp = start_blk;
	uint dma = 0, cmd, multi; //, TotalReadByte, read_blk_cnt = 0;

	rd_cnt_HSMMC = 0;
	HS_DMA_END = 0;
	BlockNum_HSMMC = 0;
	rd_cnt_HSMMC = 0;
	ReadBlockCnt_INT = 0;

	BlockNum_HSMMC = blknum;

	blksize = Card_OneBlockSize_ver1;

	while (!check_card_status());

	writew(readw(regs + HM_NORINTSTSEN) & ~(DMA_STS_INT_EN | BLOCKGAP_EVENT_STS_INT_EN), regs + HM_NORINTSTSEN);
	writew((readw(regs + HM_NORINTSIGEN) & ~(0xffff)) | TRANSFERCOMPLETE_SIG_INT_EN, regs + HM_NORINTSIGEN);

	SetSystemAddressReg(addr);		// AHB System Address For Write
	dma = 1;

	set_blksize_register(7, 512);		// Maximum DMA Buffer Size, Block Size
	set_blkcnt_register(BlockNum_HSMMC);	// Block Numbers to Write

	if (movi_hc)
		set_arg_register(start_blk);		// Card Start Block Address to Write
	else
		set_arg_register(start_blk * 512);	// Card Start Block Address to Write

	cmd = (blknum > 1) ? 18 : 17;
	multi = (blknum > 1);

	set_transfer_mode_register(multi, 1, multi, 1, dma);
	set_cmd_register(cmd, 1, MMC_RSP_R1);

	if (wait_for_cmd_done()) {
		printf(("Command NOT Complete\n"));
	} else
		ClearCommandCompleteStatus();


	check_dma_int();
	while (!HS_DMA_END);
	dbg(("\nDMA Read End\n"));

	HS_DMA_END = 0;
	BlockNum_HSMMC = 0;
	rd_cnt_HSMMC = 0;
	ReadBlockCnt_INT = 0;
}

static ulong mmc_bread (int dev_num, ulong blknr, ulong blkcnt, ulong* dst)
{
	/* Note: blknr is NOTHING like blknum! */

	if (dst >= 0xc0000000)
		dst = virt_to_phys(dst);

	if (blkcnt != 0)
		movi_read((uint) dst, (uint) blknr, (uint) blkcnt);

	return blkcnt;
}

static void write_test (uint test, uint start_blk, uint blknum)
{
	uint i, blksize; //j,
	uint uTxBufAddr = SDI_Tx_buffer_HSMMC;
	uint uCompareBufAddr = SDI_Compare_buffer_HSMMC;

	wt_cnt_HSMMC = 0;
	BlockNum_HSMMC = 0;
	wt_cnt_HSMMC = 0;
	WriteBlockCnt_INT = 0;
	HS_DMA_END = 0;

	printf("\nHS-MMC block Write test: %d, 0x%x 0x%x\n", test, start_blk, blknum);

	BlockNum_HSMMC = blknum;

	blksize = Card_OneBlockSize_ver1;

	Tx_buffer_HSMMC = (uint *) SDI_Tx_buffer_HSMMC;
	for (i = 0; i < (blksize * BlockNum_HSMMC) / 4; i++) {
		*(Tx_buffer_HSMMC + i) = ((start_blk<<16) + i);
	}

	printf("## using DMA\n");
	writew((readw(regs + HM_NORINTSTSEN) & ~(0xffff)) | BUFFER_READREADY_STS_INT_EN |
		BUFFER_WRITEREADY_STS_INT_EN | TRANSFERCOMPLETE_STS_INT_EN | COMMANDCOMPLETE_STS_INT_EN,
		regs + HM_NORINTSTSEN);

	writew((readw(regs + HM_NORINTSIGEN) & ~(0xffff)) | TRANSFERCOMPLETE_SIG_INT_EN, regs + HM_NORINTSIGEN);

	SetSystemAddressReg(SDI_Tx_buffer_HSMMC);	// AHB System Address For Write
	set_blksize_register(7, 512);	// Maximum DMA Buffer Size, Block Size
	set_blkcnt_register(BlockNum_HSMMC);	// Block Numbers to Write

	if (movi_hc)
		set_arg_register(start_blk);		// Card Start Block Address to Write
	else
		set_arg_register(start_blk * 512);	// Card Start Block Address to Write

	if (BlockNum_HSMMC == 1) {
		set_transfer_mode_register(0, 0, 1, 1, 1);
		set_cmd_register(24, 1, MMC_RSP_R1);
	} else {
		set_transfer_mode_register(1, 0, 1, 1, 1);
		set_cmd_register(25, 1, MMC_RSP_R1);
	}

	if (wait_for_cmd_done()) {
		printf("\nCommand is NOT completed3\n");
	}
	ClearCommandCompleteStatus();

	/* wait for DMA transfer */
	check_dma_int();
	while (!HS_DMA_END);

	if (!wait_for_data_done()) {
		printf(("Transfer is NOT Complete\n"));
	}
	ClearTransferCompleteStatus();

	writew(readw(regs + HM_NORINTSTS) | (1 << 3), regs + HM_NORINTSTS);

	printf(("\nDMA Write End\n"));

	DataRead_ForCompare(start_blk);
	DataCompare_HSMMC(uTxBufAddr, uCompareBufAddr, BlockNum_HSMMC * 128);

	BlockNum_HSMMC = 0;
	wt_cnt_HSMMC = 0;
	WriteBlockCnt_INT = 0;
	HS_DMA_END = 0;
	BufferBoundary_INT_Cnt = 0;

	CompareCnt_INT = 0;
	Compare_buffer_HSMMC = 0;
}

static void read_test (uint test, uint start_blk, uint blknum)
{
	uint i, blksize; //j, , Addr_temp = start_blk;
	uint dma = 0, cmd, multi; //, TotalReadByte, read_blk_cnt = 0;

	rd_cnt_HSMMC = 0;
	HS_DMA_END = 0;
	BlockNum_HSMMC = 0;
	rd_cnt_HSMMC = 0;
	ReadBlockCnt_INT = 0;

	printf("\nHS-MMC block Read test: %d, 0x%x 0x%x\n", test, start_blk, blknum);

	BlockNum_HSMMC = blknum;

	blksize = Card_OneBlockSize_ver1;

	Rx_buffer_HSMMC = (uint *) SDI_Rx_buffer_HSMMC;
	for (i = 0; i < (blksize * BlockNum_HSMMC) / 4; i++)
		*(Rx_buffer_HSMMC + i) = 0x0;

	while (!check_card_status());

	writew(readw(regs + HM_NORINTSTSEN) & ~(DMA_STS_INT_EN | BLOCKGAP_EVENT_STS_INT_EN), regs + HM_NORINTSTSEN);
	writew((readw(regs + HM_NORINTSIGEN) & ~(0xffff)) | TRANSFERCOMPLETE_SIG_INT_EN, regs + HM_NORINTSIGEN);

	SetSystemAddressReg(SDI_Rx_buffer_HSMMC);	// AHB System Address For Write
	dma = 1;

	set_blksize_register(7, 512);	// Maximum DMA Buffer Size, Block Size
	set_blkcnt_register(BlockNum_HSMMC);	// Block Numbers to Write

	if (movi_hc)
		set_arg_register(start_blk);		// Card Start Block Address to Write
	else
		set_arg_register(start_blk * 512);	// Card Start Block Address to Write

	cmd = (blknum > 1) ? 18 : 17;
	multi = (blknum > 1);

	set_transfer_mode_register(multi, 1, multi, 1, dma);
	set_cmd_register(cmd, 1, MMC_RSP_R1);

	if (wait_for_cmd_done()) {
		printf(("Command NOT Complete\n"));
	} else
		ClearCommandCompleteStatus();


	check_dma_int();
	while (!HS_DMA_END);
	printf(("\nDMA Read End\n"));

	HS_DMA_END = 0;
	BlockNum_HSMMC = 0;
	rd_cnt_HSMMC = 0;
	ReadBlockCnt_INT = 0;
}

void test_hsmmc (uint width, uint test, uint start_blk, uint blknum)
{
	hsmmc_set_gpio();

#if 0
	HS_MMC_CardDetect();
#endif

	hsmmc_reset();

	if (hsmmc_init()) {
		printf("\nCard Initialization FAILED\n");
	}

	memset((ulong *)0x51300000, 0, 0x100000);
	memset((ulong *)0x51000000, 0, 0x100000);
	memset((ulong *)0x51600000, 0, 0x100000);
	write_test(test, start_blk, blknum);
	read_test(test, start_blk, blknum);
}

#endif
