#include <common.h>

#ifdef CONFIG_S3C2443

#include <regs.h>

#include <linux/mmc/mmc.h>
#include <linux/mmc/protocol.h>
#include <asm/io.h>

#include "sdi.h"

DECLARE_GLOBAL_DATA_PTR;

#define Delay(x) udelay(x*1000)


#define rGPECON    (*(vu_long *)0x56000040)	/* Configure the pins of port E */
#define rSDIDAT    (*(vu_long *)0x5A000040)	/* SDI data */
#define SDIDAT     0x5A000040
#define BIT_SUB_SDI		(0x1<<13)

#define pISR_SDI	(*(vu_long *)(0x0+0x74))
#define pISR_DMA0	(*(vu_long *)(0x0+0x64))

/* chapter7 DMA */
#define rDISRC0		(*(vu_long *)0x4b000000)	/* DMA 0 Initial source */
#define rDISRCC0	(*(vu_long *)0x4b000004)	/* DMA 0 Initial source control */
#define rDIDST0		(*(vu_long *)0x4b000008)	/* DMA 0 Initial Destination */
#define rDIDSTC0	(*(vu_long *)0x4b00000c)	/* DMA 0 Initial Destination control */
#define rDCON0		(*(vu_long *)0x4b000010)	/* DMA 0 Control */
#define rDSTAT0		(*(vu_long *)0x4b000014)	/* DMA 0 Status (Read Only) */
#define rDCSRC0		(*(vu_long *)0x4b000018)	/* DMA 0 Current source (Read Only) */
#define rDCDST0		(*(vu_long *)0x4b00001c)	/* DMA 0 Current destination (Read Only) */
#define rDMASKTRIG0	(*(vu_long *)0x4b000020)	/* DMA 0 Mask trigger */
#define rDMAREQSEL0	(*(vu_long *)0x4b000024)	/* DMA 0 Request Selection register */

/* chapter13 INTERRUPT */
#define rSRCPND		(*(vu_long *)0x4a000000)	/* Interrupt request status */
#define rINTMOD		(*(vu_long *)0x4a000004)	/* Interrupt mode control */
#define rINTMSK		(*(vu_long *)0x4a000008)	/* Interrupt mask control */
#define rPRIORITY	(*(vu_long *)0x4a00000c)	/* IRQ priority control */
#define rINTPND		(*(vu_long *)0x4a000010)	/* Interrupt request status */
#define rINTOFFSET	(*(vu_long *)0x4a000014)	/* Interrupt request source offset */
#define rSUBSRCPND	(*(vu_long *)0x4a000018)	/* Interrupt request status */
#define rINTSUBMSK	(*(vu_long *)0x4a00001c)	/* Interrupt source mask */

/* chapter18 MMC/SD/SDI CONTROLLER */
#define rSDICON		(*(vu_long *)0x5A000000)	/* SDI Control */
#define rSDIPRE		(*(vu_long *)0x5A000004)	/* SDI baud rate prescaler */
#define rSDICARG	(*(vu_long *)0x5A000008)	/* SDI command argument */
#define rSDICCON	(*(vu_long *)0x5A00000c)	/* SDI command control */
#define rSDICSTA	(*(vu_long *)0x5A000010)	/* SDI command status */
#define rSDIRSP0	(*(vu_long *)0x5A000014)	/* SDI response 0 */
#define rSDIRSP1	(*(vu_long *)0x5A000018)	/* SDI response 1 */
#define rSDIRSP2	(*(vu_long *)0x5A00001c)	/* SDI response 2 */
#define rSDIRSP3	(*(vu_long *)0x5A000020)	/* SDI response 3 */
#define rSDIDTIMER	(*(vu_long *)0x5A000024)	/* SDI data/busy timer */
#define rSDIBSIZE	(*(vu_long *)0x5A000028)	/* SDI block size */
#define rSDIDCON	(*(vu_long *)0x5A00002c)	/* SDI data control */
#define rSDIDCNT	(*(vu_long *)0x5A000030)	/* SDI data remain counter */
#define rSDIDSTA	(*(vu_long *)0x5A000034)	/* SDI data status */
#define rSDIFSTA	(*(vu_long *)0x5A000038)	/* SDI FIFO status */
#define rSDIIMSK	(*(vu_long *)0x5A00003c)	/* SDI Interrupt Mask */

#define INICLK	400000
#define SDCLK	25000000
/* #define MMCCLK        17 500 000 */
#define MMCCLK	20000000
/* #define MMCCLK        17500000 */

#define POL	0
#define INT	1
#define DMA	2
#define SDI_Tx_buffer (0x31000000)
#define SDI_Rx_buffer (0x31000000+(0x800000))

#define Card_OneBlockSize 512

void DMA_end(void);

/* Global variables */
unsigned int *Tx_buffer;	/* 128[word]*16[blk]=8192[byte] */
unsigned int *Rx_buffer;	/* 128[word]*16[blk]=8192[byte] */
vu_long rd_cnt;
vu_long wt_cnt;
vu_long BlockNum = 0;
vu_long SD_MMC_Ins;
vu_long TR_end = 0;

CardType card_type;
uint Wide = 0;			/*  0:1bit, 1:4bit */
#if 0
uint MMC = 0;			/* 0:SD  , 1:MMC */
#endif

int Maker_ID;
char Product_Name[7];
int Serial_Num;
ulong PCLK;
vu_long RCA;

int GetIntNum(void)
{
	char temp[30];
	int i;

	for (i = 0; i < 30; i++) {
		temp[i] = getc();
		if (temp[i] == '\r') {
			temp[i] = 0;
			putc('\n');
			break;
		}
		putc(temp[i]);
	}
	i = simple_strtoul(temp, NULL, 10);
	return i;
}


static int wait_for_cmd_done1 (uint cmd, uint resp)
{
	uint i;
	ushort n_int, e_int;

	dbg("wait_for_cmd_done\n");
	for (i=0; i<0x10000000; i++) {
		n_int = readw(HM_NORINTSTS);
		dbg("  HM_NORINTSTS: %04x\n", n_int);
		if (n_int & 0x8000) break;
		if (n_int & 0x0001) return 0;
	}

	e_int = readw(HM_ERRINTSTS);
	writew(e_int, HM_ERRINTSTS);
	writew(n_int, HM_NORINTSTS);
	printf("cmd error1: 0x%04x\n", e_int);
	return -1;
}

/*  0: Timeout */
static int wait_for_cmd_done (CommandType cmd, int be_resp)
{
	ulong finish0 = 0;

	if (!be_resp) { /*  No response */
		finish0 = rSDICSTA;
		while ((finish0 & CMD_END) != CMD_END)	/*  Check cmd end */
			finish0 = rSDICSTA;

		rSDICSTA = finish0;	/*  Clear cmd end state */

		return 1;
	} else { /*  With response */
		/*  Check cmd/rsp end */
#if 0
		finish0 = rSDICSTA;
		printf("%d: rSDICSTA: %08lx\n", cmd, finish0);
		while (!(((finish0 & RESP_END) == RESP_END) | ((finish0 & CMD_TOUT) == CMD_TOUT)))
			finish0 = rSDICSTA;
#else
		while (1) {
			finish0 |= rSDICSTA;
			printf("%d: rSDICSTA: %08lx\n", cmd, finish0);
			if (finish0 & RESP_END)
				break;
			if (finish0 & CMD_TOUT) {
				printf("cmd timeout error\n");
				return 0;
			}
		}
#endif

		/*  CRC no check, CMD9 is a long Resp. command. */
		if ((cmd == SEND_OP_CMD) || (cmd == 41)) {
			/*  Check error */
			if ((finish0 & 0xf00) != (CMD_END | RESP_END)) {
				rSDICSTA = finish0;	/*  Clear error state */
				if (((finish0 & CMD_TOUT) == CMD_TOUT))
					return 0;	/*  Timeout error */
			}
			rSDICSTA = finish0;	/*  Clear cmd & rsp end state */
		} else {	/*  CRC check */
			/*  Check error */
			if ((finish0 & 0x1f00) != (CMD_END | RESP_END)) {
				/* printf("CMD%d:rSDICSTA=0x%x, rSDIRSP0=0x%x\n",cmd, rSDICSTA, rSDIRSP0); */
				rSDICSTA = finish0;	/*  Clear error state */

				if (((finish0 & CMD_TOUT) == CMD_TOUT))
					return 0;	/*  Timeout error */
			}
			rSDICSTA = finish0;
		}
		return 1;
	}
}

static int sdmmc_issue_command (uint cmd, uint arg, uint data, uint flags)
{
	int i;
	uint val = (cmd | 0x40 | (1<<8));
	uint reg = 0;

	rSDICARG = arg;

	if (flags & MMC_RSP_PRESENT)	/* Normal RSP */
		val |= (1<<9);

	if (flags & MMC_RSP_136)	/* Long RSP */
		val |= (1<<10);

	if (data)
		val |= (1<<11);

	if (cmd == 12)
		val |= (1<<12);

	writel(val, SDICCON);

	for (i=0; i<100000; i++) {
		reg = readl(SDICSTA);
		printf("reg = %08x\n", reg);
		if (reg & CMD_TOUT) {
			printf("cmd timeout: %d\n", cmd);
			break;
		}
		if (flags & MMC_RSP_CRC) {
			if (reg & RESP_CRC_FAIL) {
				printf("RSP CRC failed: %d\n", cmd);
				break;
			}
		}
		if (flags & MMC_RSP_PRESENT) {
			if (reg & RESP_END) {
				writel(reg, SDICSTA);
				return 0;
			}
		}
		else {
			if (reg & CMD_END) {
				writel(reg, SDICSTA);
				return 0;
			}

		}
	}

	writel(reg, SDICSTA);
	return -1;
}

static void SDIO_Command (uint cmdarg, uint cmd, uint wait_resp, uint long_resp, uint data)
{
	rSDICARG = cmdarg;
	rSDICCON = (data << 11) | (long_resp << 10) | (wait_resp << 9) | (cmd | 0x40);

	/* Command Start */
	rSDICCON |= (1 << 8);
}

static void SDIO_TypeSetting (uint ClkType, uint ByteType)
{
	if (ClkType == SDIO_SD_TYPE)
		rSDICON &= ~CLOCK_TYPE;
	else if (ClkType == SDIO_MMC_TYPE)
		rSDICON |= CLOCK_TYPE;

	if (ByteType == SDIO_TYPE_A)
		rSDICON &= ~BYTE_ORDER;
	else if (ByteType == SDIO_TYPE_B)
		rSDICON |= BYTE_ORDER;
}

static void SDIO_BlockSizeSetting (uint size)
{
	rSDIBSIZE = (size & 0xFFF);
}

static CardType Check_OCR(void)
{
	int i;
	uint ocr;

#if 0
	/*  MMC Card Test */
	for (i = 0; i < 600; i++) {	/*  original 200 time -> 600 */
		SDIO_Command(0x00ff8000, 1, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);

		if (wait_for_cmd_done(SEND_OP_CMD, 1)) {
			if (rSDIRSP0 == 0x80ff8080) {
//				rSDICSTA |= CMD_END | RESP_END;
				printf("\n\n[In MoviNand Memory ready....]\n");
				return MOVI_NAND_CARD;
			} else if ((rSDIRSP0 >> 16) == 0x80ff) {	/* [31]:Card Power up status bit (busy) */
//				rSDICSTA |= CMD_END | RESP_END;
				printf("\n\n[In MMC Memory ready....]\n");
				return MMC_CARD;	/*  Success */
			}
		}
	}
#else
	for (i = 0; i < 250; i++) {
		sdmmc_issue_command(MMC_SEND_OP_COND, 0x0, 0, MMC_RSP_R3);
		ocr = readl(SDIRSP0);
		dbg("ocr1: %08x\n", ocr);

		sdmmc_issue_command(MMC_SEND_OP_COND, ocr, 0, MMC_RSP_R3);
		ocr = readl(SDIRSP0);
		dbg("ocr2: %08x\n", ocr);
		if (ocr & (0x1 << 31)) {
			printf("Voltage range: ");
			if (ocr & (1 << 21))
				printf("2.7V ~ 3.4V");
			else if (ocr & (1 << 20))
				printf("2.7V ~ 3.3V");
			else if (ocr & (1 << 19))
				printf("2.7V ~ 3.2V");
			else if (ocr & (1 << 18))
				printf("2.7V ~ 3.1V");
			if (ocr & (1 << 7))
				printf(", 1.65V ~ 1.95V\n");
			else
				putc('\n');
			return MMC_CARD;	/*  Success */
		}
	}

#endif

	/*  SD Card Test */
	SDIO_Command(0, 55, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
	if (wait_for_cmd_done(55, 1)) {
		Delay(200);
		SDIO_Command(0, 41, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
		if (wait_for_cmd_done(41, 1)) {
			 ocr = rSDIRSP0;
			 printf("ocr = %08x\n", ocr);
		}
	}
	for (i = 0; i < 600; i++) {	/*  original 200 time -> 600 */
		printf("%d: RCA: %08x\n", i, RCA);
		SDIO_Command((RCA << 16), 55, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITH_DATA);
		if (wait_for_cmd_done(55, 1)) {
			Delay(200);
			SDIO_Command(ocr, 41, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
			if (wait_for_cmd_done(41, 1)) {
				printf("rSDIRSP0 = %08lx\n", rSDIRSP0);
//				rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */
				printf("\n\n[In SD/TFlash Memory Ready State....]\n");
				return SD_CARD;	/*  Success */
			}
			Delay(200);	/*  Wait Card power up status */
		}
#if 0
		while (wait_for_cmd_done(55, 1) != 1) {
			printf("\nCommand55 Fail...");
			return ERROR_CARD;
		}

		SDIO_Command(0xff8000, 41, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
		if ((wait_for_cmd_done(41, 1) & rSDIRSP0) == 0x80ff8000) {
			rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */
			printf("\n\n[In SD/TFlash Memory Ready State....]\n");
			return SD_CARD;	/*  Success */
		}
		Delay(200);	/*  Wait Card power up status */
#endif
	}
	return ERROR_CARD;
}

void Set_Bus (CardType type)
{
	if (type == SD_CARD) {
		Wide = 1;
	} else if (type == MMC_CARD) {
		Wide = 0;
	} else if (type == MOVI_NAND_CARD) {
		Wide = 1;
	}
	SET_BUSWIDTH(type);

	if (Wide)
		printf("\n****4bit bus****\n");
	else
		printf("\n****1bit bus****\n");
}

void SET_BUSWIDTH (CardType type)
{
	int arg;

	if (type == MOVI_NAND_CARD) {
		arg = (3 << 24) | (183 << 16) | (1 << 8);
		do {
			SDIO_Command(arg, SWITCH_CMD, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);	/* ACMD6 select bus width */
		} while ((wait_for_cmd_done(6, 1) != 1));
	} else if (type == SD_CARD) {
		do {
			SDIO_Command((RCA << 16), APP_CMD, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITH_DATA);	/* befoe using ACMD */
		} while ((wait_for_cmd_done(APP_CMD, 1) != 1));

		do {
			SDIO_Command((Wide << 1), SWITCH_CMD, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);	/* ACMD6 select bus width */
		} while ((wait_for_cmd_done(SWITCH_CMD, 1) != 1));
	} else if (type == MMC_CARD) {
		/*  do not need anything. */
	}

	rSDICSTA |= CMD_END | RESP_END;
}


void CARD_SEL_DESEL (unchar sel_desel)
{
	printf("\nCMD7 : Command toggles a card between the stand-by and transfer state\n");
	if (sel_desel) {
		printf("\nCMD7 : Select\n");
		SDIO_Command((RCA << 16), 7, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
		if ((wait_for_cmd_done(7, 1) != 1) || (rSDIRSP0 != 0x800))	/* entering the standby mode */
			SDIO_Command((RCA << 16), 7, SDIO_WAIT_RESP, SDIO_SHORT_RESP,
				     SDIO_WITHOUT_DATA);

		rSDICSTA |= CMD_END | RESP_END;
		printf("\nEntered to the Transfer state\n");
	} else {
		printf("\nCMD7 : Deselect\n");
		SDIO_Command((RCA << 16), 7, SDIO_NO_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
		if ((wait_for_cmd_done(7, 0) != 1))
			SDIO_Command((RCA << 16), 7, SDIO_NO_RESP, SDIO_SHORT_RESP,
				     SDIO_WITHOUT_DATA);

		rSDICSTA |= CMD_END;
		printf("\nEntered to the Stand-by State\n");
	}
}


void SDIO_PrescalerSetting (uint baudrate)
{
	rSDIPRE = (PCLK / baudrate) - 1;
}

static int Check_BUSYend(void)
{
	int finish;

	finish = rSDIDSTA;
	printf("\nBusy end =%x", finish);
	while (!(((finish & 0x08) == 0x08) | ((finish & 0x20) == 0x20)))
		finish = rSDIDSTA;

	while ((finish & (0x1 << 3)) != 0x8) {
		printf("\nDATA:finish=0x%x\n", finish);
		finish = rSDIDSTA;
		/* return 0; */
	}
	rSDIDSTA = 0xf4;	/* clear error state */
	return 1;
}

#if 0
int Check_DATend(void)
{
	int finish;

	finish = rSDIDSTA;

	while (!(((finish & 0x10) == 0x10) || ((finish & 0x20) == 0x20))) {	/*  Chek timeout or data end */
		/* printf("\nData End Check =%x",finish); */
		finish = rSDIDSTA;
	}

	if ((finish & 0x10) != 0x10) {
		/* printf("\nDATA:finish=0x%x\n", finish); */
		rSDIDSTA = finish;	/*  Clear error state */
		return 0;
	}
	return 1;
}

#else
int Check_DATend(void)
{
	vu_long finish;

	finish = rSDIDSTA;
	/* printf("DATA:finish=0x%x\n", finish); */
	printf("DATA end check\n");
	while (!((rSDIDSTA & DATA_FINISH) == DATA_FINISH)) {	/*  Chek timeout or data end */
//		printf("x");
		printf("rSDIDSTA = %08lx\n", rSDIDSTA);
		finish = rSDIDSTA;
	}
	printf("Check_DATend End\n");

	if ((finish & 0xfc) != DATA_FINISH) {
		/* printf("DATA:finish=0x%x\n", finish); */
		rSDIDSTA = 0xec;	/*  Clear error state */
		return 0;
	}
	return 1;
}
#endif

void DMA_end(void)
{
	/* printf("\nDMA ISR\n"); */
	/* rSUBSRCPND |= BIT_SUB_DMA0; */
	ClearPending(BIT_DMA0);
	TR_end = 1;
}

void Rd_Int(void)
{
	uint i, status;

	rINTSUBMSK |= BIT_SUB_SDI;
	rINTMSK |= BIT_SDI;

	rSUBSRCPND |= BIT_SUB_SDI;
	ClearPending(BIT_SDI);

	status = rSDIFSTA;
	if ((status & 0x200) == 0x200) {	// Check Last interrupt?
		for (i = (status & 0x7f) / 4; i > 0; i--) {
			*Rx_buffer++ = rSDIDAT;
			rd_cnt++;
		}
		rSDIFSTA = rSDIFSTA & 0x200;	//Clear Rx FIFO Last data Ready, YH 040221
	} else if ((status & 0x80) == 0x80) {	// Check Half interrupt?
		for (i = 0; i < 8; i++) {
			*Rx_buffer++ = rSDIDAT;
			rd_cnt++;
		}
	}

	rINTMSK &= ~(BIT_SDI);
	rINTSUBMSK &= ~(BIT_SUB_SDI);
}

void Wt_Int(void)
{
	rINTSUBMSK |= BIT_SUB_SDI;
	rINTMSK |= BIT_SDI;

	rSUBSRCPND |= BIT_SUB_SDI;
	ClearPending(BIT_SDI);
	rINTMSK |= (BIT_SDI);

	rSDIDAT = *Tx_buffer++;
	wt_cnt++;

	if (wt_cnt == (Card_OneBlockSize / 4) * BlockNum) {
		rINTMSK |= BIT_SDI;
		rSDIDAT = *Tx_buffer;
		TR_end = 1;
	}

	rINTMSK &= ~(BIT_SDI);
	rINTSUBMSK &= ~(BIT_SUB_SDI);
}

static void sdmmc_set_gpio(void)
{
	ulong reg = readl(GPECON);

	reg &= ~(0x003ffc00);
	reg |= 0x002aa800;
	writel(reg, GPECON);
#if 0
	rGPECON = (rGPECON & ~(0x003ffc00)) | (0x1 << 21) | (0x1 << 19) | (0x1 << 17) |
	    (0x1 << 15) | (0x1 << 13) | (0x1 << 11);
#endif
}

int sdmmc_init (void)
{
	uint C_SIZE, C_SIZE_MULT, READ_BL_LEN, CardSize, OneBlockSize;

	PCLK = get_PCLK();

#if 0
	rSDIPRE = PCLK / (INICLK) - 1;	/*  Clock setting */
#else
	writel(0xfe, SDIPRE);
#endif

	SDIO_TypeSetting(SDIO_SD_TYPE, SDIO_TYPE_A);	/*  clock type setting */
	rSDICON |= ENCLK;	/* Clock enable */
	rSDIFSTA |= (1 << 16);	/* FIFO reset */

	SDIO_BlockSizeSetting(Card_OneBlockSize);	/* block size setting */
	rSDIDTIMER |= 0x7fffff;	/* busy timer setting */

	Delay(0x1000);		/*  Wait 74SDCLK for MMC card */

#if 0
	SDIO_Command(0x0, 0x0, SDIO_NO_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
	wait_for_cmd_done(0, 0);
#else
	sdmmc_issue_command(MMC_GO_IDLE_STATE, 0, 0, 0);
#endif
	printf("\nEnter to the Idle State\n");

	card_type = Check_OCR();
	if (card_type == ERROR_CARD) {
		printf("\n\nInitialize fail\nNo Card assertion\n");
		return -1;
	}

	printf("CMD2 : CID response\n");
	do {
		SDIO_Command(0x0, ALL_SEND_SID_CMD, SDIO_WAIT_RESP, SDIO_LONG_RESP,
			     SDIO_WITHOUT_DATA);
	} while (wait_for_cmd_done(ALL_SEND_SID_CMD, 1) != 1);
	printf(" - Product Name : %c%c%c%c%c%c\n", (rSDIRSP0 & 0xFF), ((rSDIRSP1 >> 24) & 0xFF),
	       ((rSDIRSP1 >> 16) & 0xFF), ((rSDIRSP1 >> 8) & 0xFF), (rSDIRSP1 & 0xFF),
	       ((rSDIRSP2 >> 24) & 0xFF));

	printf("CMD3 : assign relative address\n");
	do {
		if (card_type == SD_CARD)
			SDIO_Command((0 << 16), 3, SDIO_WAIT_RESP, SDIO_SHORT_RESP,
				     SDIO_WITHOUT_DATA);
		else if (card_type == MMC_CARD)
			SDIO_Command((1 << 16), 3, SDIO_WAIT_RESP, SDIO_SHORT_RESP,
				     SDIO_WITHOUT_DATA);
		else if (card_type == MOVI_NAND_CARD)
			SDIO_Command((0 << 16), 3, SDIO_WAIT_RESP, SDIO_SHORT_RESP,
				     SDIO_WITHOUT_DATA);
	} while (wait_for_cmd_done(3, 1) != 1);

	if (card_type == MMC_CARD) {
		RCA = 1;
		SDIO_PrescalerSetting(MMCCLK);

		while (PCLK / (rSDIPRE + 1) > 20000000)
			rSDIPRE = rSDIPRE + 1;

		printf("\nMMC Frequency is %dHz\n", (PCLK / (rSDIPRE + 1)));
	} else {
		SDIO_PrescalerSetting(SDCLK);
		while (PCLK / (rSDIPRE + 1) > 25000000)
			rSDIPRE = rSDIPRE + 1;
		printf("\nSD Frequency is %dHz\n", (PCLK / (rSDIPRE + 1)));
		RCA = (rSDIRSP0 & 0xffff0000) >> 16;
		printf("RCA=0x%x\n", RCA);
	}

	do {
		SDIO_Command(RCA << 16, 9, SDIO_WAIT_RESP, SDIO_LONG_RESP, SDIO_WITHOUT_DATA);
	} while (wait_for_cmd_done(9, 1) != 1);

	READ_BL_LEN = (rSDIRSP1 >> 16 & 0xF);
	C_SIZE = ((rSDIRSP1 & 0x3ff) << 2) | (rSDIRSP2 >> 30 & 0x3);
	C_SIZE_MULT = (rSDIRSP2 >> 15 & 0x7);

	CardSize = (1 << READ_BL_LEN) * (C_SIZE + 1) * (1 << (C_SIZE_MULT + 2)) / 1048576;
	OneBlockSize = (1 << READ_BL_LEN);

	printf("\n READ_BL_LEN: %d", READ_BL_LEN);
	printf("\n C_SIZE: %d", C_SIZE);
	printf("\n C_SIZE_MULT: %d\n", C_SIZE_MULT);

	printf("\n One Block Size: %dByte", OneBlockSize);
	printf("\n Total Card Size: %dMByte\n", CardSize + 1);

/* -------------------------------------------------------------------------------------// */

	CARD_SEL_DESEL(1);	/* Enter Card state to the transfer mode */

	do {
		SDIO_Command(Card_OneBlockSize, SET_BLOCKLEN_CMD, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
	} while (wait_for_cmd_done(SET_BLOCKLEN_CMD, 1) != 1);
	printf("\nSet Block Length");

	Set_Bus(card_type);

	return 0;
}

static void sdmmc_read (uint test, uint start_blk, uint blknum)
{
	uint i, StartAddr, OneBlockSize, Addr_temp;
	uint status, time;
	int j;

	rd_cnt = 0;

	rSRCPND = BIT_DMA0;
	rINTPND = BIT_DMA0;

	printf("\nSD/MMC block Read test\n");

	StartAddr = start_blk;
	BlockNum = blknum;

	OneBlockSize = Card_OneBlockSize;

	Rx_buffer = (uint *) SDI_Rx_buffer;
	for (i = 0; i < (OneBlockSize * BlockNum) / 4; i++)
		*(Rx_buffer + i) = 0x0;

	/* rSDIDSTA |= (1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2); */
	rSDIDSTA = CRC_STAT_FAIL | RECEIVE_CRC_FAIL | DATA_TOUT | DATA_FINISH | BUSY_FINISH;
	rSDIDCON = 0;
	rSDIFSTA |= (1 << 16);	/* FIFO reset */

	if (0 == Wide)
		rSDIDCON &= ~WIDE_BUS_EN;
	else
		rSDIDCON |= WIDE_BUS_EN;

	rSDIDCON |= WORD_TRANSFER | RCV_AFTER_CMD | BLOCK_TRANSFER | BlockNum;
	rSDIDCON |= DATA_RECEIVE_MODE | DATA_TRANSMIT_START;

	switch (test) {
	case POL:
		if (BlockNum < 2) {
			do {	/*  StartAddr * 512 */
				SDIO_Command(StartAddr << 9, READ_SINGLE_BLOCK_CMD, SDIO_WAIT_RESP,
					     SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
			} while (wait_for_cmd_done(READ_SINGLE_BLOCK_CMD, 1) != 1);
		} else {
			do {
				SDIO_Command(StartAddr << 9, READ_MULTIPLE_BLOCK_CMD,
					     SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
			} while (wait_for_cmd_done(READ_MULTIPLE_BLOCK_CMD, 1) != 1);
		}

		/* Clear cmd_end(with rsp) */
		rSDICSTA = DATA_TIMEOUT | CMD_END | RESP_END;
		/* printf("\nData status=%x",rSDICSTA); */

		printf("\nTotal size =%x", OneBlockSize * BlockNum / 4);
		while (rd_cnt < (OneBlockSize * BlockNum) / 4) {
			if (rSDIDSTA & DATA_TOUT) {	/*  Check timeout */
				printf("\nData status=%x", rSDICSTA);
				rSDIDSTA |= DATA_TOUT;	/*  Clear timeout flag */
				break;
			}

			status = rSDIFSTA;
			/* If FIFO is not empty,
			 * receive data from Data Register(rSDIDAT0)
			 */
			if (status & RX_FIFO_AVAILABLE_DETECT) {	/*  Is Rx data? */
				*Rx_buffer++ = rSDIDAT;
				rd_cnt++;
			}
//			printf("%d\n", rd_cnt);
		}

		printf("\nRead Count =%x", rd_cnt);
		break;

	case DMA:
		printf("\nDMA mode data read\n");

		pISR_DMA0 = (unsigned) DMA_end;
		rINTMSK &= ~(BIT_DMA0);
		/* rINTSUBMSK&=~(BIT_SUB_DMA0); */

		rDISRC0 = (uint) (SDIDAT);
		rDISRCC0 = (1 << 1) | (1 << 0);
		rDIDST0 = (int) (Rx_buffer);
		rDIDSTC0 = (0 << 1) | (0 << 0);
		rDCON0 = (1 << 31) | (0 << 30) | (1 << 29) | (1 << 28) |
		    (0 << 27) | (1 << 24) | (1 << 22) | (2 << 20) | (128 * BlockNum) / 4;

		/* no-stop, DMA0 channel on, no-sw trigger */
		rDMASKTRIG0 = (0 << 2) | (1 << 1) | 0;
		rDMAREQSEL0 = (10 << 1) | 1;

		if (BlockNum < 2) {
			/* StartAddr * 512 */
			do {
				SDIO_Command(StartAddr << 9, READ_SINGLE_BLOCK_CMD, SDIO_WAIT_RESP,
					     SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
			} while (wait_for_cmd_done(READ_SINGLE_BLOCK_CMD, 1) != 1);
		} else {
			do {
				SDIO_Command(StartAddr << 9, READ_MULTIPLE_BLOCK_CMD,
					     SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
			} while (wait_for_cmd_done(READ_MULTIPLE_BLOCK_CMD, 1) != 1);
		}

		/*  Clear cmd_end(with rsp) */
		rSDICSTA = DATA_TIMEOUT | CMD_END | RESP_END;

		rSDIDCON |= (BURST4_EN | WORD_TRANSFER | RCV_AFTER_CMD |
			     BLOCK_TRANSFER | DMA_EN | BlockNum);
		rSDIDCON |= (DATA_RECEIVE_MODE | DATA_TRANSMIT_START);

		if (Wide != 0)
			rSDIDCON |= WIDE_BUS_EN;
		else
			rSDIDCON &= ~WIDE_BUS_EN;

		while (!(rSRCPND & BIT_DMA0));
		rINTMSK |= (BIT_DMA0);
		/* rINTSUBMSK |= (BIT_SUB_DMA0); */
		TR_end = 0;
		rDMASKTRIG0 = (1 << 2);	/* DMA0 stop */
		break;

	default:
		break;
	}

/*       time=Timer_Stop(); */

	if (!Check_DATend())
		printf("\nData End Error....\n");


	if (BlockNum > 1) {
		do {
			SDIO_Command(0, STOP_TRANSMISSION_CMD, SDIO_WAIT_RESP,
				     SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
		} while (wait_for_cmd_done(STOP_TRANSMISSION_CMD, 1) != 1);
		/* Clear cmd_end(with rsp) */
		rSDICSTA = CMD_END | RESP_END;
	}
#ifdef CONFIG_PRINT_OUTPUT
	Rx_buffer = (uint *) SDI_Rx_buffer;

	for (j = 0; j < (OneBlockSize * BlockNum) / 4; j++) {
		if (j % 4 == 0)
			printf("\n0x%04xh : ", Addr_temp);
		printf("0x%08x ", *Rx_buffer++);
		Addr_temp += 4;
	}
#endif

	printf("TIME is : %d ns\n", time << 7);	/*  time is 1/128 mux */
	printf("Transfer Rate is : %f MB per sec \n",
	       ((float) (BlockNum << 9)) / ((float) (time << 7)));

	BlockNum = 0;
	printf("\n");
}

static void sdmmc_write (uint test, uint start_blk, uint blknum)
{
	uint i, StartAddr, Offset, Testmode;
	unsigned int time = 1;
	uint status;
	wt_cnt = 0;

	printf("\nSD/MMC block write test\n");

	Testmode = test;
	StartAddr = start_blk;
	BlockNum = blknum;
	Offset = 0;

	Tx_buffer = (uint *) SDI_Tx_buffer;
	for (i = 0; i < (Card_OneBlockSize / 4 * BlockNum); i++)
		*(Tx_buffer + i) = (i + Offset);

	rSDIFSTA |= (1 << 16);	/* FIFO reset */
	rSDIDCON = (2 << 22) | (1 << 20) | (1 << 17) | (Wide << 16) |
		(1 << 14) | (3 << 12) | BlockNum;

	switch (Testmode) {
	case POL:
		printf("\nPolling mode data write\n");

		if (BlockNum < 2) {	/* single block */
			do {	/*  StartAddr * 512 */
				SDIO_Command(StartAddr << 9, WRITE_BLOCK_CMD, SDIO_WAIT_RESP,
					     SDIO_SHORT_RESP, SDIO_WITH_DATA);
			} while ((wait_for_cmd_done(WRITE_BLOCK_CMD, 1) != 1));
		} else {	/* multi block */
			do {
				SDIO_Command(StartAddr << 9, WRITE_MULTIPLE_BLOCK, SDIO_WAIT_RESP,
					     SDIO_SHORT_RESP, SDIO_WITH_DATA);
			} while ((wait_for_cmd_done(WRITE_MULTIPLE_BLOCK, 1) != 1));
		}
		rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */
		printf("\nBlock size=%x\n", Card_OneBlockSize / 4 * BlockNum);
		while (wt_cnt < (Card_OneBlockSize / 4 * BlockNum)) {
			if (rSDIFSTA & TX_FIFO_AVAILABLE_DETECT) {
				rSDIDAT = *Tx_buffer++;
				wt_cnt++;
			}
		}

		printf("\nWrite count=%x\n", wt_cnt);
		break;

	case DMA:
		printf("\nDMA mode data write\n");

		rINTMSK &= ~(BIT_DMA0);
		/* rINTSUBMSK&=~(BIT_SUB_DMA0); */

		rDISRC0 = (int) (Tx_buffer);
		rDISRCC0 = (0 << 1) | (0 << 0);
		rDIDST0 = (uint) (SDIDAT);
		rDIDSTC0 = (1 << 1) | (1 << 0);
		rDCON0 = (1 << 31) | (0 << 30) | (1 << 29) | (1 << 28) | (0 << 27) |
		    (1 << 24) | (1 << 22) | (2 << 20) | (128 * BlockNum) / 4;
		/* no-stop, DMA0 channel on, no-sw trigger */
		rDMASKTRIG0 = (0 << 2) | (1 << 1) | 0;
		rDMAREQSEL0 = (10 << 1) | 1;

		if (BlockNum < 2) {	/* single block */
			do {	/*  StartAddr * 512 */
				SDIO_Command(StartAddr << 9, WRITE_BLOCK_CMD, SDIO_WAIT_RESP,
					     SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
			} while ((wait_for_cmd_done(WRITE_BLOCK_CMD, 1) != 1));
		} else {	/* multi block */
			do {
				SDIO_Command(StartAddr << 9, WRITE_MULTIPLE_BLOCK, SDIO_WAIT_RESP,
					     SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
			} while ((wait_for_cmd_done(WRITE_MULTIPLE_BLOCK, 1) != 1));
		}
		rSDICSTA |= CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */

		if (Wide != 0)
			rSDIDCON |= WIDE_BUS_EN;
		else
			rSDIDCON &= ~WIDE_BUS_EN;

		rSDIDCON |= BURST4_EN | WORD_TRANSFER | TRANS_AFTER_RESP |
			BLOCK_TRANSFER | DMA_EN | BlockNum;
		rSDIDCON |= DATA_TRANSMIT_MODE | DATA_TRANSMIT_START;

#if 0
		while (!TR_end);
#else
		while(!(rSRCPND & BIT_DMA0));
		ClearPending(BIT_DMA0);
#endif

		rINTMSK |= (1 << 20);
		rINTSUBMSK |= (1 << 27);
		TR_end = 0;
		rDMASKTRIG0 |= (1 << 2);	/* DMA Stop */

	default:
		break;
	}

	while (!Check_DATend());
	/* printf("\nData End Error\n"); */
	rSDIDCON = rSDIDCON & ~(7 << 12);	/* YH 040220, Clear Data Transfer mode => no operation, Cleata Data Transfer start */
	rSDIDSTA |= DATA_FINISH;	/*  Clear data Tx/Rx end */

	if (BlockNum > 1) {
		rSDIDCON = BUSY_AFTER_CMD | BLOCK_TRANSFER | WIDE_BUS_DIS |
			DATA_TRANSMIT_START | ONLYBUSY_CHECK | (BlockNum << 0);
		do {
			SDIO_Command(0, 12, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
		} while (wait_for_cmd_done(12, 1) != 1);

		rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */

		if (!Check_BUSYend())
			printf("error\n");

		rSDIDSTA |= DATA_FINISH;	/* ! Should be cleared by writing '1'. */
	}

	printf("TIME is : %d ns\n", time << 7);	/*  time is 1/128 mux */
	printf("Transfer Rate is : %f MB per sec \n",
	       ((float) (BlockNum << 9)) / ((float) (time << 7)));
	BlockNum = 0;
}


void SDIO_EraseBlock(void)
{
	uint StartAddr, EndAddr;

	printf("\nSD/MMC block erase test\n");

	printf("\nInput Write Start Block Address(0x200, 0x400...) : ");	/* ex) 0x200[512bytes], 0x400, 0x800..... */
	StartAddr = GetIntNum();

	printf("Input Write End Block Address(0x200, 0x400...) : ");	/* ex) 0x200[512bytes], 0x400, 0x800....... */
	EndAddr = GetIntNum();
#if 0
	SDIO_Command(StartAddr, 32, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
	while ((wait_for_cmd_done(32, 1) != 1)) {
		SDIO_Command(StartAddr, 32, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
		printf("\nCMD32 fail\n");
	}

	printf("\nCMD32 OK\n");
#else

	if (card_type == SD_CARD) {
		do {
			SDIO_Command(StartAddr, 32, SDIO_WAIT_RESP, SDIO_SHORT_RESP,
				     SDIO_WITH_DATA);
		} while (wait_for_cmd_done(32, 1) != 1);
		rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */

		do {
			SDIO_Command(EndAddr, 33, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITH_DATA);
		} while (wait_for_cmd_done(33, 1) != 1);
		rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */
	} else if (card_type == MMC_CARD || card_type == MOVI_NAND_CARD) {
		do {
			SDIO_Command(StartAddr, ERASE_GROUP_START_CMD, SDIO_WAIT_RESP,
				     SDIO_SHORT_RESP, SDIO_WITH_DATA);
		} while (wait_for_cmd_done(ERASE_GROUP_START_CMD, 1) != 1);
		rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */

		do {
			SDIO_Command(EndAddr, ERASE_GROUP_END_CMD, SDIO_WAIT_RESP, SDIO_SHORT_RESP,
				     SDIO_WITH_DATA);
		} while (wait_for_cmd_done(ERASE_GROUP_END_CMD, 1) != 1);
		rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */
	}
#endif

	rSDIDSTA |= (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);
	rSDIDCON = 0;

	if (Wide == 0)
		rSDIDCON &= ~WIDE_BUS_EN;
	else
		rSDIDCON |= WIDE_BUS_EN;

	rSDIDCON |= BUSY_AFTER_CMD | BLOCK_TRANSFER | BlockNum;
	rSDIDCON |= ONLYBUSY_CHECK | DATA_TRANSMIT_START;


	do {
		SDIO_Command(0, ERASE_CMD, SDIO_WAIT_RESP, SDIO_SHORT_RESP, SDIO_WITHOUT_DATA);
	} while (wait_for_cmd_done(ERASE_CMD, 1) != 1);
	rSDICSTA = CMD_END | RESP_END;	/*  Clear cmd_end(with rsp) */

	if (!Check_BUSYend())
		printf("\nBusy Check Error...\n");

	rSDIDSTA = (1 << 3);
	printf("\n[SD/TFlash card Erase Block Test...End]\n");


}

void test_sdi (uint test, uint start_blk, uint blknum)
{
	int i;
	RCA = 0;
	card_type = SD_CARD;
	SD_MMC_Ins = 0;

	sdmmc_set_gpio();

	if (sdmmc_init()) {
		printf("Card initialization failed\n");
		return;
	}

	if (card_type == SD_CARD)
		printf("[SD card test...PCLK:%d,INICLK:%d,SDCLK:%d]\n", PCLK, INICLK, SDCLK);
	else if(card_type==MMC_CARD)
		printf("[MMC card test...PCLK:%d,INICLK:%d,MMCCLK:%d]\n", PCLK, INICLK, MMCCLK);
	else if(card_type==MOVI_NAND_CARD)
		printf("[moviNAND card test...PCLK:%d,INICLK:%d,MOVICLK:%d]\n",PCLK, INICLK, MMCCLK);

	sdmmc_write(test, start_blk, blknum);
	sdmmc_read(test, start_blk, blknum);
}

#endif

