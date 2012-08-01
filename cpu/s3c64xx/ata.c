#include <common.h>
#include <regs.h>
#include <asm/io.h>

#include "cf.h"
#include "ata.h"
#include "gpio.h"

#ifdef DBG_ATA
#define DbgAta(x...) printf(x)
#else
#define DbgAta(x...) do {} while(0);
#endif


#define ATA(__n)	( ( volatile oATA_REGS * ) ( g_paATASFRBase[__n] ) )

#define SMDK6400 1


#define ELFIN_ATA_BASE		(CFCON_BASE+0x1900)

//u8 g_ucOpMode;
//u8 g_ucATAConNum;
volatile ATA_oInform g_oaATAInform[ATA_CONNUM];


/*
 * ATA Register (chapter 8)
 */
#define ATA_CONTROL	(ELFIN_ATA_BASE + 0x00)
#define ATA_STATUS	(ELFIN_ATA_BASE + 0x04)
#define ATA_COMMAND	(ELFIN_ATA_BASE + 0x08)
#define ATA_SWRST	(ELFIN_ATA_BASE + 0x0c)
#define ATA_IRQ		(ELFIN_ATA_BASE + 0x10)
#define ATA_IRQ_MASK	(ELFIN_ATA_BASE + 0x14)
#define ATA_CFG		(ELFIN_ATA_BASE + 0x18)

#define ATA_PIO_TIME	(ELFIN_ATA_BASE + 0x2c)
#define ATA_UDMA_TIME	(ELFIN_ATA_BASE + 0x30)
#define ATA_XFR_NUM	(ELFIN_ATA_BASE + 0x34)
#define ATA_XFR_CNT	(ELFIN_ATA_BASE + 0x38)
#define ATA_TBUF_START	(ELFIN_ATA_BASE + 0x3c)
#define ATA_TBUF_SIZE	(ELFIN_ATA_BASE + 0x40)
#define ATA_SBUF_START	(ELFIN_ATA_BASE + 0x44)
#define ATA_SBUF_SIZE	(ELFIN_ATA_BASE + 0x48)
#define ATA_CADR_TBUF	(ELFIN_ATA_BASE + 0x4c)
#define ATA_CADR_SBUF	(ELFIN_ATA_BASE + 0x50)
#define ATA_PIO_DTR	(ELFIN_ATA_BASE + 0x54)
#define ATA_PIO_FED	(ELFIN_ATA_BASE + 0x58)
#define ATA_PIO_SCR	(ELFIN_ATA_BASE + 0x5c)
#define ATA_PIO_LLR	(ELFIN_ATA_BASE + 0x60)
#define ATA_PIO_LMR	(ELFIN_ATA_BASE + 0x64)
#define ATA_PIO_LHR	(ELFIN_ATA_BASE + 0x68)
#define ATA_PIO_DVR	(ELFIN_ATA_BASE + 0x6c)
#define ATA_PIO_CSD	(ELFIN_ATA_BASE + 0x70)
#define ATA_PIO_DAD	(ELFIN_ATA_BASE + 0x74)
#define ATA_PIO_READY	(ELFIN_ATA_BASE + 0x78)
#define ATA_PIO_RDATA	(ELFIN_ATA_BASE + 0x7c)
#define BUS_FIFO_STATUS	(ELFIN_ATA_BASE + 0x90)
#define ATA_FIFO_STATUS	(ELFIN_ATA_BASE + 0x94)

#define DEV_ERROR	(ELFIN_ATA_BASE + 0x58)
#define DEV_FEATURE	(ELFIN_ATA_BASE + 0x58)
#define DEV_SECTOR	(ELFIN_ATA_BASE + 0x5c)
#define DEV_LOWLBA	(ELFIN_ATA_BASE + 0x60)
#define DEV_MIDLBA	(ELFIN_ATA_BASE + 0x64)
#define DEV_HIGHLBA	(ELFIN_ATA_BASE + 0x68)
#define DEV_DEVICE	(ELFIN_ATA_BASE + 0x6c)
#define DEV_STATUS	(ELFIN_ATA_BASE + 0x70)
#define DEV_COMMAND	(ELFIN_ATA_BASE + 0x70)
#define DEV_ALTANATE	(ELFIN_ATA_BASE + 0x74)
#define DEV_CONTROL	(ELFIN_ATA_BASE + 0x74)

/*=========================================================================
 *          	               ata Command
 *=========================================================================
 */
#define IDENTIFYDEVICE		0xec
#define READSECTOR		0x20
#define READMULTIPLE		0xc4
#define READDMA			0xc8
#define WRITESECTOR		0x30
#define WRITEMULTIPLE		0xc5
#define WRITEDMA		0xca
#define SETFEATURES		0xEF


/*=========================================================================
 *          	               ata Command
 *=========================================================================
 */
#define ATA_CMD_IDENTIFYDEVICE	0xec
#define ATA_CMD_READSECTOR	0x20
#define ATA_CMD_READMULTIPLE	0xc4
#define ATA_CMD_READDMA		0xc8
#define ATA_CMD_WRITESECTOR	0x30
#define ATA_CMD_WRITEMULTIPLE	0xc5
#define ATA_CMD_WRITEDMA	0xca
#define ATA_CMD_SETFEATURES	0xEF

typedef struct tag_ATA_REGS {
	u32 rrATA_CONTROL;	// ATA enable and clock down status
	u32 rrATA_STATUS;	// ATA status
	u32 rrATA_COMMAND;	// ATA command
	u32 rrATA_SWRST;	// ATA software reset
	u32 rrATA_IRQ;		// ATA interrupt sources
	u32 rrATA_IRQ_MASK;	// ATA interrupt mask
	u32 rrATA_CFG;		// ATA configuration for ATA interface
	u32 rReserved1;
	u32 rReserved2;
	u32 rReserved3;
	u32 rReserved4;
	u32 rrATA_PIO_TIME;	// ATA PIO timing
	u32 rrATA_UDMA_TIME;	// ATA UDMA timing
	u32 rrATA_XFR_NUM;	// ATA transfer number
	u32 rrATA_XFR_CNT;	// ATA current transfer count
	u32 rrATA_TBUF_START;	// ATA start address of track buffer
	u32 rrATA_TBUF_SIZE;	// ATA size of track buffer
	u32 rrATA_SBUF_START;	// ATA start address of source buffer
	u32 rrATA_SBUF_SIZE;	// ATA size of source buffer
	u32 rrATA_CADR_TBUF;	// ATA current write address of track buffer
	u32 rrATA_CADR_SBUF;	// ATA current read address of source buffer
	u32 rrATA_PIO_DTR;	// ATA PIO device data register
	u32 rrATA_PIO_FED;	// ATA PIO device Feature/Error register
	u32 rrATA_PIO_SCR;	// ATA PIO sector count register
	u32 rrATA_PIO_LLR;	// ATA PIO device LBA Low register
	u32 rrATA_PIO_LMR;	// ATA PIO device LBA middle register
	u32 rrATA_PIO_LHR;	// ATA PIO device LBA high register
	u32 rrATA_PIO_DVR;	// ATA PIO device register
	u32 rrATA_PIO_CSD;	// ATA PIO device command/status register
	u32 rrATA_PIO_DAD;	// ATA PIO device control/alternate status register
	u32 rrATA_PIO_READY;	// ATA PIO read data from device data register
	u32 rrATA_PIO_RDATA;	// ATA PIO read data from device data register
	u32 rReserved5;
	u32 rReserved6;
	u32 rReserved7;
	u32 rReserved8;
	u32 rrBUS_FIFO_STATUS;	// ATA internal AHB FIFO status
	u32 rrATA_FIFO_STATUS;	// ATA internal ATA FIFO status
} oATA_REGS;

// define global variables
static void *g_paATASFRBase[ATA_CONNUM];
extern volatile ATA_oInform g_oaATAInform[];

static int cmp32 (uint a0, uint a1, uint words)
{
	uint *pD0 = (uint *) a0;
	uint *pD1 = (uint *) a1;
	uint i;

	for (i = 0; i < words; i++, pD0++, pD1++) {
		if (*pD0 != *pD1)
			return -1;
	}

	return 0;
}

static void dump32 (uint addr, uint words)
{
	int i, j;
	uint *pMem;

	pMem = (uint *) (addr / 4 * 4);

	printf("\n");
	for (i = 0; i < words;) {
		printf(" %3d: ", i);

		for (j = 0; j < 8; j++)
			printf("%08x ", *pMem++), i++;
		printf("\n");
	}
}

static void CF_SetMUXReg(eCF_MUX_OUTPUT eOutputEnFlag, eCF_MUX_CARDPWR eCardPwrFlag, eCF_MUX_IDEMODE eModeFlag)
{
	Outp32(ATA_MUX, ((eOutputEnFlag << 2) | (eCardPwrFlag << 1) | (eModeFlag << 0)));
}

static void cf_set_EBI (u8 opmode)
{
	u32 reg;

	reg = readl(MEM_SYS_CFG) & (~0x0000003f);
	writel(reg | 0x00000030, MEM_SYS_CFG);

#if 0
	/* Set Buffer Direction pin */
	/* Set XhiINTR pin as CF Data Direction */
	reg = readl(GPMCON) & ~(0xf<<(4*5));
	writel(reg | 3 <<(4*5), GPMCON);
	printf("gpmcon: %08x\n", reg);

	/* Set XirSDBW pin as CF Data Direction */
	reg = readl(GPBCON) & ~(0xf<<(4*4));
	writel(reg | 4 <<(4*4), GPBCON);
	printf("gpbcon: %08x\n", reg);
#endif

 	if (opmode == DIRECT_MODE) {
		Disp(" * CF EBI MODE : Direct Mode\n\n");
		reg = readl(MEM_SYS_CFG);
		writel(reg | 0x00004000, MEM_SYS_CFG);
 	}
	else {
		Disp(" * CF EBI MODE : InDirect Mode\n\n");
		reg = readl(MEM_SYS_CFG) & (~0x00004000);
		writel(reg, MEM_SYS_CFG);
	}
}

/*=========================================================================
 *          	       ata controller register I/O fuctions
 *=========================================================================
 */

/*
 * return 0: OK
 * return -1: got error
 */
static int wait_for_host_ready (void)
{
	ulong temp;
	uint i;

	for (i=0; i<100000; i++) {
		temp = readl(ATA_FIFO_STATUS);
		if ((temp >> 28) == 0)
			return 0;
		DbgAta("ATA_FIFO_STATUS: %08x\n", temp);
	}
	printf("got error in host ready: %08lx\n", temp);
	return -1;
}

static u8 read_dev_reg (uint reg)
{
	u8 temp;

	wait_for_host_ready();
	temp = readb(reg);
	wait_for_host_ready();
	temp = readb(ATA_PIO_RDATA);
	return temp;
}

static void write_dev_reg (uint reg, u8 data)
{
	wait_for_host_ready();
	writeb(data, reg);
}

static u16 read_data (void)
{
	u16 temp16;

	wait_for_host_ready();
	temp16 = readw(ATA_PIO_DTR);
	wait_for_host_ready();
	temp16 = readw(ATA_PIO_RDATA);
	return temp16;
}

static void write_data (u16 data)
{
	wait_for_host_ready();
	writel(data, ATA_PIO_DTR);
}

static int wait_for_trans_done (void)
{
	u32 reg;

#if 1
	uint i;

	for (i=0; i<0x1000000; i++) {
		reg = readl(ATA_IRQ);
		if (reg & 0x01)
			break;
	}
	if (i > 0x100000) {
		printf("transmission timeout\n");
		return -1;
	}
#endif

	wait_for_host_ready();	/// needed
	reg = readl(ATA_STATUS);
	DbgAta("ATA_STATUS: %08x\n", reg);
	if (reg & 3) {
		printf("transmission failed\n");
		return -1;
	}

	return 0;
}

static int wait_for_dev_ready (void)
{
	u8 temp;
	uint i;

	for (i=0; i<1000000; i++) {
		temp = read_dev_reg(DEV_STATUS);
//		printf("temp2: %08x\n", temp);
		if ((temp & STATUS_DEVICE_BUSY) == 0) {
			DbgAta("temp2: %08x\n", temp);
			DbgAta("wait_for_dev_ready: %d\n", i);
			return 0;
		}
	}

	printf("wait_for_dev_ready: error\n");
	return -1;
}

static void set_ata_enable (uint on)
{
	u32 temp;

	temp = readl(ATA_CONTROL);

	if (on)
		writel(temp | 0x1, ATA_CONTROL);
	else
		writel(temp & 0xfffffffe, ATA_CONTROL);
}

static void set_endian_mode (uint little)
{
	u32 reg = readl(ATA_CFG);

	/* set Little endian */
	if (little)
		reg &= (~0x40);
	else
		reg |= 0x40;

	writel(reg, ATA_CFG);
}

static void set_trans_command (ATA_TRANSFER_CMD cmd)
{
	wait_for_host_ready();
	writel(cmd, ATA_COMMAND);
}

static void change_mode_to_ata (void)
{
	/* TDelay 1 unit = 10us */
	udelay(1000);

	/* CF controller - True IDE mode setting */

	/* Output pad disable, Card power off, ATA mode */
	writel(0x07, ATA_MUX);
	udelay(10000);
	/* Output pad enable, Card power off, ATA mode */
	writel(0x03, ATA_MUX);
	udelay(10000);
	/* Output pad enable, Card power on, ATA mode */
	writel(0x01, ATA_MUX);
	udelay(500000);	/* wait for 500ms */
}

static void init_port (u8 opmode)
{
	u32 reg;
	if (opmode == DIRECT_MODE) {
		u32 temp;

		temp = readl(GPACON);

		// D: Set XhiDATA[15:0] pins as CF Data[15:0]
		writel(0x55555555, GPKCON0);
		writel(0x55555555, GPKCON1);
		// A: Set XhiADDR[2:0] pins as CF ADDR[2:0]
		reg = readl(GPLCON0) & (~0x00000fff);
		writel(reg | 0x00000666, GPLCON0);
		// C: Set Xhi control pins as CF control pins(IORDY, IOWR, IORD, CE[1], CE[0])
		writel(0x00066666, GPMCON);

		/* This line must be here because Uart ports(GPIO_A)
		 * are changed after setting GPIO_K.
		 */
		writel(temp, GPACON);
	}

	change_mode_to_ata();

	writel(0x1C238, ATA_PIO_TIME); /* 0x1C238 */
	writel(0x20B1362, ATA_UDMA_TIME); /* 0x20B1362 */
	set_endian_mode(1);
	set_ata_enable(1);
	mdelay(400);	// wait for 200ms, be positively necessary

#if 0 // for pull-down-dedicated nCD pins
	GPIO_SetFunctionEach(eGPIO_P, eGPIO_14, 1);	// set CData/EINT as output
	GPIO_SetDataEach(eGPIO_P, eGPIO_14, 1);	//GPN[8] -> High
	DelayfrTimer(milli, 500);	//Delay about 10ms

	GPIO_SetFunctionEach(eGPIO_P, eGPIO_14, 2);	// set CData/EINT as CData
	GPIO_SetDataEach(eGPIO_P, eGPIO_14, 1);	//GPN[8] -> High
	DelayfrTimer(milli, 500);	//Delay about 10ms
#endif

	/* for CF card boot up signal */
#if 1
	reg = readl(GPNCON) & ~(0x3<<(2*8));
	writel(reg | (1<<(2*8)), GPNCON);
	reg = readl(GPNDAT);
	writel(reg | (1<<8), GPNDAT);
	mdelay(500);
	writel(reg & ~(1<<8), GPNDAT);
#endif
}

static int ata_init (u8 mode)
{
	/* EBI Setup for CF controller */
	cf_set_EBI(mode);
	g_paATASFRBase[0] = (void *) ATA_BASE;

	init_port(mode);

	return 0;
}

static int identify_device(void)
{
	u16 buf[ATA_SECTORSIZE / 2];
	uchar temp;
	vu_char *str;
	int i;

	memset(buf, 0x1, ATA_SECTORSIZE);

	/* XXX: set Big endian (must be) why? scsuh */
	set_endian_mode(0);

	writel(0xffffffff, ATA_IRQ_MASK);
	write_dev_reg(DEV_DEVICE, 0x40);
	write_dev_reg(DEV_COMMAND, IDENTIFYDEVICE);

	if (wait_for_dev_ready()) {
		printf("cannot find device\n");
		return 1;
	}

	for (i = 0; i < ATA_SECTORSIZE / 2; i++) {
		buf[i] = read_data();
		DbgAta(("DATA : 0x%04X\n", buf[i]));
	}
	wait_for_dev_ready();

	/* verify identify data */
	str = (uchar *) & buf[10];
	printf("\nSerial Number :");
	for (i = 0; i < 20; i++)
		printf("%c", str[i]);

	str = (uchar *) & buf[27];
	printf("\nModel Number :");
	for (i = 0; i < 40; i++)
		printf("%c", str[i]);

	g_oaATAInform[0].uMaxSectors = (swab16(buf[61]) << 16) + swab16(buf[60]);
	printf("\nMax Sectors : %d\n", g_oaATAInform[0].uMaxSectors);

	// Caution: buf[x] - Big Endian, so upper byte means LSB..
	g_oaATAInform[0].uMaxMultiple = (buf[47] >> 8) & 0xFF;
	printf("\nMax Multiple : %02X\n", g_oaATAInform[0].uMaxMultiple);
	if (buf[59] & 0x1) {	//multiple sector setting is valid
		g_oaATAInform[0].uMaxMultiple = (buf[59] >> 8) & 0xFF;
		printf("Current Multiple : %03X\n", g_oaATAInform[0].uMaxMultiple);
	}

	if (((buf[64] >> 8) & 0x3) == 1)
		g_oaATAInform[0].eMaxPioMode = eATA_PIO3;
	else if (((buf[64] >> 8) & 0x3) == 3)
		g_oaATAInform[0].eMaxPioMode = eATA_PIO4;
	else
		g_oaATAInform[0].eMaxPioMode = eATA_PIO2;
	printf("Max PIO Mode : %d\n", g_oaATAInform[0].eMaxPioMode);

	g_oaATAInform[0].eMaxUdmaMode = 0;
	temp = buf[88] >> 8;
	for (i = 4; i >= 0; i--) {
		if (temp & (0x01 << i)) {
			g_oaATAInform[0].eMaxUdmaMode = i;
			break;
		}
	}

	g_oaATAInform[0].eCurrentUdmaMode = 0;
	temp = buf[88] & 0x00ff;
	for (i = 0; i < 5; i++) {
		if (temp & (0x01 << i)) {
			g_oaATAInform[0].eCurrentUdmaMode = i;
			break;
		}
	}

	printf("Max UDMA Mode : %d\n", g_oaATAInform[0].eMaxUdmaMode);
	printf("Current UDMA Mode : %d\n", g_oaATAInform[0].eCurrentUdmaMode);

	set_endian_mode(1);

	return 0;
}

static ulong ATA_GetRegValue (u8 ucCon, eATA_Id uATARegId)
{
	vu_long *puATABaseAddr;
	vu_long *puATAReg;

	puATABaseAddr = (vu_long*)ELFIN_ATA_BASE;
	puATAReg = puATABaseAddr + uATARegId;
#if 0
	Disp("[ATA_GetRegValue]ATABaseAddr : 0x%x, ATAId:0x%d, ATAReg: 0x%x, Value:0x%x \n",
				puATABaseAddr, uATARegId, puATAReg, *puATAReg);
#endif
	return *puATAReg;
}

static void ATA_SetRegValue (u8 ucCon, eATA_Id uATARegId, u32 uValue)
{
	vu_long *puATABaseAddr;
	vu_long *puATAReg;

	puATABaseAddr = (vu_long*)ELFIN_ATA_BASE;
	puATAReg = puATABaseAddr + uATARegId;
#if 0
	Disp("[ATA_SetRegValue]ATABaseAddr : 0x%x, ATAReg: 0x%x, Value:0x%x \n",
				puATABaseAddr, puATAReg, uValue);
#endif
	*puATAReg = uValue;
}

static char *ATA_GetModeName (eATA_MODE_6400 eAtaMode)
{
	switch (eAtaMode) {
	case eATA_MODE_PIOCPU:
		return "PIO";
	case eATA_MODE_PIODMA:
		return "PIODMA";
	case eATA_MODE_UDMA:
		return "UDMA";
	case eATA_MODE_NONE:
	default:
		return "NONE";
	}
}

static u8 ATA_GetTaskFileRegValue (u8 ucCon, eCF_TASKFILE_Id uATATaskFileRegId)
{
	volatile u32 *puATABaseAddr;
	volatile u8 *puATAReg;

	puATABaseAddr = &(ATA(ucCon)->rrATA_PIO_DTR);
	puATAReg = (u8 *) (puATABaseAddr + uATATaskFileRegId);
//      Disp("[ATA_GetTaskFileRegValue]puATABase : 0x%x, RegID:0x%x, DestAddr : 0x%x, Result : 0x%x\n", puATABaseAddr, uATATaskFileRegId, puATAReg, *puATAReg);
	return *puATAReg;
}

static void ata_set_xfer_dir (u8 ucCon, eATA_DMA_DIR eDmaDir)
{
	u32 uRegValue = 0;

	uRegValue = ATA_GetRegValue(ucCon, eATA_CFG) & ~(0x10);
	uRegValue |= (eDmaDir << 4);

	ATA_SetRegValue(ucCon, eATA_CFG, uRegValue);
}

static void ata_set_xfer_mode (u8 ucCon, eATA_MODE_6400 eAtaMode)
{
	u32 uRegValue = 0;

	uRegValue = ATA_GetRegValue(ucCon, eATA_CFG) & ~(0xc);
	uRegValue |= (eAtaMode << 2);

	ATA_SetRegValue(ucCon, eATA_CFG, uRegValue);
}

static void ATA_SetEnable (u8 ucCon, u8 ucEnFlag)
{
	u32 uRegValue = 0;

	uRegValue = ATA_GetRegValue(ucCon, eATA_CONTROL) & ~(0x1);
	uRegValue |= (ucEnFlag << 0);

	ATA_SetRegValue(ucCon, eATA_CONTROL, uRegValue);
}

static u8 ATA_GetDeviceReg(u8 ucCon, eCF_TASKFILE_Id eTaskFileRegId)
{
	u8 ucTempRead = 0;

	wait_for_host_ready();
	ucTempRead = ATA_GetTaskFileRegValue(ucCon, eTaskFileRegId);
	wait_for_host_ready();
	ucTempRead = (u8) (ATA_GetRegValue(ucCon, eATA_PIO_RDATA) & 0xff);

	return ucTempRead;
}

static void ATA_WaitForDeviceReady (u8 ucCon)
{
	u8 ucReadData = 0;

	while (1) {
		ucReadData = ATA_GetDeviceReg(ucCon, eCF_TASKFILE_ALTANATE);
		ucReadData = ATA_GetDeviceReg(ucCon, eCF_TASKFILE_STATUS);

		if ((ucReadData & eATA_XFR_STATUS_DEVICE_BUSY) == 0)
			break;
	}
}

ulong get_HCLK(void);

static void set_ata_device (uint lba, uint sector)
{
	write_dev_reg(DEV_SECTOR, sector & 0xFF);
	write_dev_reg(DEV_LOWLBA, lba & 0xFF);
	write_dev_reg(DEV_MIDLBA, (lba >> 8) & 0xFF);
	write_dev_reg(DEV_HIGHLBA, (lba >> 16) & 0xFF);
	write_dev_reg(DEV_DEVICE, ((lba >> 24) & 0xF) | 0x40);
}

/**********************************
 * PIO mode maximum transfer rate
 * PIO0 : 3.3MB/s
 * PIO1 : 5.2MB/s
 * PIO2 : 8.3MB/s
 * PIO3 : 11.1MB/s
 * PIO4 : 16.7MB/s
 *
 ***********************************/

static void set_pio_mode (eATA_PIOMODE pmode)
{
	uint uT1, uT2, uTeoc;
	uint i;

	uint pio_time[5];
	uint uPioT1[5] = { 70, 50, 30, 30, 30 };
	uint uPioT2[5] = { 290, 290, 290, 80, 70 };
	uint uPioTeoc[5] = { 20, 20, 10, 10, 10 };

	uint uCycleTime = (unsigned int) (1000000000 / get_HCLK());
	u32 reg = readl(ATA_CFG);

	for (i = 0; i < 5; i++) {
		uT1 = (uPioT1[i] / uCycleTime) & 0xff;
		uT2 = (uPioT2[i] / uCycleTime) & 0xff;
		uTeoc = (uPioTeoc[i] / uCycleTime) & 0x0f;
		pio_time[i] = (uTeoc << 12) | (uT2 << 4) | uT1;
		Disp("PIO%dTIME = %x\n", i, pio_time[i]);
	}

	writel(0xff, ATA_IRQ);
	writel(0, ATA_IRQ_MASK);

	write_dev_reg(DEV_CONTROL, 0);
	/* set transfer mode based on value in Sector Count register */
	write_dev_reg(DEV_FEATURE, 0x03);
	/*  PIO flow control transfer mode */
	write_dev_reg(DEV_SECTOR, 0x08 | (pmode & 0x7));
	write_dev_reg(DEV_LOWLBA, 0x00);
	write_dev_reg(DEV_MIDLBA, 0x00);
	write_dev_reg(DEV_HIGHLBA, 0x00);
	write_dev_reg(DEV_DEVICE, 0x40);

	/* set feature command */
	write_dev_reg(DEV_COMMAND, SETFEATURES);

	wait_for_dev_ready();

	switch (pmode) {
	case eATA_PIO0:
	case eATA_PIO1:
	case eATA_PIO2:
		reg &= (~0x2);	/* IORDY disable */
		break;
	case eATA_PIO3:
	case eATA_PIO4:
		reg |= 0x2;	/* IORDY enable */
		break;
	default:
		printf("unknown pmode: %d\n", pmode);
		return;
	}
	writel(pio_time[pmode], ATA_PIO_TIME);
	writel(reg, ATA_CFG);
	g_oaATAInform[0].eCurrentPioMode = pmode;
}

/**********************************
 * UDMA mode maximum transfer rate
 * UDMA0 : 16.7MB/s
 * UDMA1 : 25.0MB/s
 * UDMA2 : 33.3MB/s
 * UDMA3 : 44.4MB/s
 * UDMA4 : 66.7MB/s
 * UDMA5 : 100 MB/s
 *
***********************************/
static void set_udma_mode(UDMAMODE umode)
{
	uint uTdvh1, uTdvs, uTrp, uTss, uTackenv;
	uint i;

	u32 uUdmaTime[5];
	u32 uUdmaTdvh[5] = { 7, 7, 7, 7, 7 };
	u32 uUdmaTdvs[5] = { 70, 48, 31, 20, 7 };
	u32 uUdmaTrp[5] = { 160, 125, 100, 100, 100 };
	u32 uUdmaTss[5] = { 50, 50, 50, 50, 50 };
	u32 uUdmaTackenvMin[5] = { 20, 20, 20, 20, 20 };

	u32 uCycleTime = (u32) (1000000000 / get_HCLK());

	for (i = 0; i < 5; i++) {
		uTdvh1 = (uUdmaTdvh[i] / uCycleTime) & 0x0f;
		uTdvs = (uUdmaTdvs[i] / uCycleTime) & 0xff;
		uTrp = (uUdmaTrp[i] / uCycleTime) & 0xff;
		uTss = (uUdmaTss[i] / uCycleTime) & 0x0f;
		uTackenv = (uUdmaTackenvMin[i] / uCycleTime) & 0x0f;
		uUdmaTime[i] = (uTdvh1 << 24) | (uTdvs << 16) | (uTrp << 8) | (uTss << 4) | uTackenv;
		DbgAta("UDMA%dTIME = %x\n", i, uUdmaTime[i]);
	}

	writel(0xff, ATA_IRQ);
	writel(0, ATA_IRQ_MASK);

	write_dev_reg(DEV_CONTROL, 0);
	write_dev_reg(DEV_FEATURE, 0x03);
	write_dev_reg(DEV_SECTOR, 0x40 | (umode & 0x7));
	write_dev_reg(DEV_LOWLBA, 0x00);
	write_dev_reg(DEV_MIDLBA, 0x00);
	write_dev_reg(DEV_HIGHLBA, 0x00);
	write_dev_reg(DEV_DEVICE, 0x40);
	write_dev_reg(DEV_COMMAND, SETFEATURES);

	wait_for_dev_ready();

	switch (umode) {
	case UDMA0:
	case UDMA1:
	case UDMA2:
	case UDMA3:
	case UDMA4:
		writel(uUdmaTime[umode], ATA_UDMA_TIME);
		g_oaATAInform[0].eCurrentUdmaMode = umode;
		break;
	default:
		DbgAta(("Wrong UDMA mode in set_udma_mode()\n"));
		return;
	}
}

static bool rw_sector_pio (uint rw, uint lba, uint sector, uint buf)
{
	uint sector_current;
	uint sector_remain = sector;
	uint lba_current = lba;
	uint round = 0;
	u16 *addr;
	uint i;
	uint cmd = rw ? WRITESECTOR : READSECTOR;

	while (sector_remain != 0) {
		if (sector_remain > 256) {
			sector_current = 256;	//0 means 256
			sector_remain -= 256;
		} else {
			sector_current = sector_remain;
			sector_remain = 0;
		}
		lba_current = lba + round * 256;
		addr = (u16 *)(buf + round * 256 * ATA_SECTORSIZE);

		set_ata_device(lba_current, sector_current);
		write_dev_reg(DEV_COMMAND, cmd);

		while (sector_current--) {
			wait_for_dev_ready();
			for (i = 0; i < ATA_SECTORSIZE / 2; i++) {
				if (rw)
					write_data(*addr);
				else
					*addr = read_data();
				addr++;
			}
		}
		wait_for_dev_ready();

		round++;
	}

	return TRUE;
}

static void ATA_SetUdmaAutoMode(u8 ucCon, u8 ucEnFlag)
{
	u32 uRegValue = 0;

	uRegValue = ATA_GetRegValue(ucCon, eATA_CFG) & ~(0x200);
	uRegValue |= ucEnFlag << 9;

	ATA_SetRegValue(ucCon, eATA_CFG, uRegValue);
}

static void ata_set_config (ATA_MODE mode, eATA_DMA_DIR dir)
{
	u8 ucCon = 0;

	switch (mode) {
	case PIO_CPU:
		ata_set_xfer_mode(ucCon, eATA_MODE_PIOCPU);
		break;

	case PIO_DMA:
		ata_set_xfer_mode(ucCon, eATA_MODE_PIODMA);
		ata_set_xfer_dir(ucCon, dir);
		break;

	case UDMA:
		ata_set_xfer_mode(ucCon, eATA_MODE_UDMA);
		ATA_SetUdmaAutoMode(ucCon, ENABLE);
		ata_set_xfer_dir(ucCon, dir);
		break;

	default:
		break;
	}
}

static void ATA_SetTransferCommand(u8 ucCon, eATA_XFR_CMD eXfrCmd)
{
	u32 uRegValue = 0;

	uRegValue = ATA_GetRegValue(ucCon, eATA_COMMAND) & ~(0x3);
	uRegValue |= (eXfrCmd << 0);

	ATA_SetRegValue(ucCon, eATA_COMMAND, uRegValue);
}

static void ATA_WaitForTransferDone(u8 ucCon)
{
	u32 uRegValue = 0;
//      Disp("MODE3:%s\n", ATA_GetModeName(g_oaATAInform[ucCon].eAtaMode));
	do {
		wait_for_host_ready();	/// needed
		uRegValue = ATA_GetRegValue(ucCon, eATA_STATUS);
//              Disp("%s,%d:Value:0x%x\n", __FUNCTION__, __LINE__, uRegValue);
	} while ((uRegValue & 3) != 0);
}

static bool ATA_IsDmaDone(u8 ucCon)
{
//      Disp("MODE4:%s\n", ATA_GetModeName(g_oaATAInform[ucCon].eAtaMode));
	ATA_SetTransferCommand(ucCon, eATA_XFR_CMD_ABORT);

	ata_set_config(eATA_MODE_PIOCPU, eATA_DMA_WRITE_DATA);
	ATA_WaitForDeviceReady(ucCon);

	if (g_oaATAInform[ucCon].eAtaMode == eATA_MODE_UDMA) {
		ATA_SetUdmaAutoMode(ucCon, DISABLE);
	}
	return TRUE;
}

static bool rw_sector_piodma (uint rw, uint lba, uint sector, uint buf)
{
	uint cmd = rw ? WRITESECTOR : READSECTOR;

	/* Buffer Setting */
	if (rw) {
		writel(buf, ATA_SBUF_START);
		writel(sector * ATA_SECTORSIZE, ATA_SBUF_SIZE);
	} else {
		writel(buf, ATA_TBUF_START);
		writel(sector * ATA_SECTORSIZE, ATA_TBUF_SIZE);
	}
	writel(sector * ATA_SECTORSIZE, ATA_XFR_NUM);
	set_ata_device(lba, sector);

	write_dev_reg(DEV_COMMAND, cmd);
	wait_for_dev_ready();

#if 0
	ata_set_config(g_oaATAInform[0].eAtaMode, rw);
#else
	ata_set_xfer_mode(0, PIO_DMA);
	ata_set_xfer_dir(0, rw);
#endif
	set_trans_command(ATA_CMD_START);

	ATA_WaitForTransferDone(0);
	wait_for_trans_done();
	ATA_IsDmaDone(0);

	return TRUE;

}

#if 0
static bool ATA_IsWritingBlocksDone(u8 ucCon)
{
	ATA_WaitForTransferDone(ucCon);

	return ATA_IsDmaDone(ucCon);
}

static bool ATA_IsReadingBlocksDone(u8 ucCon)
{
	return ATA_IsWritingBlocksDone(ucCon);
}
#endif

static bool ATA_WriteSectors_PIODMA (u8 ucCon, u32 lba, uint buf)
{
#if 0
	ATA_StartWritingBlocks(ucCon, lba, 1, buf);
	return ATA_IsWritingBlocksDone(0);
#else
	return rw_sector_piodma(1, lba, 1, buf);
#endif
}

static bool ATA_ReadSectors_PIODMA (u8 ucCon, u32 lba, u32 buf)
{
#if 0
	ATA_StartReadingBlocks(ucCon, lba, 1, buf);
	return ATA_IsReadingBlocksDone(0);
#else
	return rw_sector_piodma(0, lba, 1, buf);
#endif
}

static void set_config_mode (ATA_MODE mode, int rw)
{
	u32 reg = readl(ATA_CFG);

	/* set mode */
	reg = ((reg & 0x1E3) | (mode << 2));

	/* DMA write mode */
	if (mode && rw)
		reg |= 0x10;

#if 0
	/* set ATA DMA auto mode (enable multi block transfer) */
	if (mode == UDMA)
		reg |= 0x200;
#else
	if (mode == UDMA)
		reg |= 0x380;
#endif
	writel(reg, ATA_CFG);
	printf("ATA_CFG: %08x\n", readl(ATA_CFG));
}

static int start_rw_block (uint rw, uint mode, uint sector, uint count, uint buf)
{
	uint cmd;

	printf("  start_rw_block: %d, %d, %x, %x, %08x\n", rw, mode, sector, count, buf);
	if (rw) {
		cmd = (mode == UDMA) ? WRITEDMA : WRITESECTOR;
		writel(buf, ATA_SBUF_START);
		writel(count * ATA_SECTORSIZE, ATA_SBUF_SIZE);
	} else {
		cmd = (mode == UDMA) ? READDMA : READSECTOR;
		writel(buf, ATA_TBUF_START);
		writel(count * ATA_SECTORSIZE, ATA_TBUF_SIZE);
	}
	writel(count * ATA_SECTORSIZE, ATA_XFR_NUM);

	set_ata_device(sector, count);

	printf("cmd: %02x\n", cmd);
	write_dev_reg(DEV_COMMAND, cmd);
	wait_for_dev_ready();

	/* apply at specific set using buffer control */
	if (mode == UDMA) {
		change_board_buf_mode(mode, rw);
	}

	set_config_mode(mode, rw);

	set_trans_command(ATA_CMD_START);
	check_dma_done(mode, rw);

	return 0;
}

/*
 * rw: 1 write
 * rw: 0 read
 */
static bool rw_sector_udma (uint rw, uint lba, uint sector, uint buf)
{
	uint sector_current;
	uint sector_remain = sector;
	uint lba_current;
	uint addr;
	uint round = 0;
	uint cmd = rw ? WRITEDMA : READDMA;
	uint i,j;
	u32 reg;

	while (sector_remain) {
		reg = readl(ATA_IRQ);
		printf("irq: %08x\n", reg);
		writel(reg, ATA_IRQ);

		if (sector_remain > 256) {
			sector_current = 256;	// 0 means 256
			sector_remain -= 256;
		} else {
			sector_current = sector_remain;
			sector_remain = 0;
		}
		printf("sector_current: %d\n", sector_current);

		lba_current = lba + round * 256;
		addr = buf + round * 256 * ATA_SECTORSIZE;

		set_ata_device(lba_current, sector_current);
		write_dev_reg(DEV_COMMAND, cmd);
		wait_for_dev_ready();

		set_config_mode(UDMA, rw);

		if (rw) {
			writel(addr, ATA_SBUF_START);
			writel(2 * ATA_SECTORSIZE-1, ATA_SBUF_SIZE);
		} else {
			writel(addr, ATA_TBUF_START);
			writel(2 * ATA_SECTORSIZE-1, ATA_TBUF_SIZE);
		}
		writel(sector_current * ATA_SECTORSIZE, ATA_XFR_NUM);

		/* ATA Transfer Command */
		set_trans_command(ATA_CMD_START);

		/* Source Buffer 1 Setting */
		for (i=1; i<sector_current; i+=2) {
			for (j=0; j<0x100000; j++) {
				reg = readl(ATA_IRQ);
				if (reg & 0x18) {
					printf("reg: %08x\n", reg);
					break;
				}
			}
			if (rw) {
				writel(addr+i*ATA_SECTORSIZE, ATA_SBUF_START);
				writel(2 * ATA_SECTORSIZE-1, ATA_SBUF_SIZE);
			} else {
				writel(addr+i*ATA_SECTORSIZE, ATA_TBUF_START);
				writel(2 * ATA_SECTORSIZE-1, ATA_TBUF_SIZE);
			}
			set_trans_command(ATA_CMD_CONTINUE);
		}

		if (rw == 0) {
			for (i=0; i<0x1000000; i++) {
				reg = readl(ATA_IRQ);
				if (reg & 0x8) {
					set_trans_command(ATA_CMD_CONTINUE);
					break;
				}
			}
			if (i > 0x100000) {
				printf("transmission timeout\n");
			}
		}
		wait_for_trans_done();
		set_trans_command(ATA_CMD_ABORT);

		set_config_mode(PIO_CPU, rw);
		wait_for_dev_ready();

		round++;
	}

	return TRUE;
}

#if 0
/*
 * rw: 1 write
 * rw: 0 read
 */
static bool rw_sector_udma_org (uint rw, uint lba, uint sector, uint buf)
{
	uint sector_current;
	uint sector_remain = sector;
	uint lba_current;
	uint addr;
	uint round = 0;
	uint cmd = rw ? WRITEDMA : READDMA;
	u32 reg;

	while (sector_remain != 0) {
		reg = readl(ATA_IRQ);
		printf("irq: %08x\n", reg);
		writel(reg, ATA_IRQ);

		if (sector_remain > 256) {
			sector_current = 256;	// 0 means 256
			sector_remain -= 256;
		} else {
			sector_current = sector_remain;
			sector_remain = 0;
		}
		lba_current = lba + round * 256;
		addr = buf + round * 256 * ATA_SECTORSIZE;

		/* Source Buffer 1 Setting */
		if (rw) {
			writel(addr, ATA_SBUF_START);
			writel(sector_current * ATA_SECTORSIZE-1, ATA_SBUF_SIZE);
		} else {
			writel(addr, ATA_TBUF_START);
			writel(sector_current * ATA_SECTORSIZE-1, ATA_TBUF_SIZE);
		}
		writel(sector_current * ATA_SECTORSIZE, ATA_XFR_NUM);

		set_ata_device(lba_current, sector_current);
		write_dev_reg(DEV_COMMAND, cmd);
		wait_for_dev_ready();

		set_config_mode(UDMA, rw);

		/* ATA Transfer Command */
		set_trans_command(ATA_CMD_START);
		wait_for_trans_done();
		set_trans_command(ATA_CMD_ABORT);

		set_config_mode(PIO_CPU, rw);
		wait_for_dev_ready();

		round++;
	}

	return TRUE;
}
#endif

static bool read_block (u32 lba, u32 sector, u32 buf)
{
	ATA_MODE mode = g_oaATAInform[0].eAtaMode;
	bool bStatus = FALSE;
	uint i = 0;

	u8 ucCon = 0;

	Disp("ReadBlocks() in %s mode...\n", ATA_GetModeName(mode));

	switch (mode) {
	case PIO_CPU:
		bStatus = rw_sector_pio(0, lba, sector, buf);
		break;

	case PIO_DMA:
		for (i = 0; i < sector; i++) {
			ATA_ReadSectors_PIODMA(ucCon, lba + i,
					       buf + ATA_SECTORSIZE * i);
			ATA_WaitForTransferDone(ucCon);
		}
		bStatus = TRUE;
		break;

	case UDMA:
		bStatus = rw_sector_udma(0, lba, sector, buf);
		break;

	default:
		Disp("Not supported mode in WriteBlocks()\n");
		break;
	}

	return bStatus;
}

static bool write_block (uint lba, uint sector, uint buf)
{
	ATA_MODE mode = g_oaATAInform[0].eAtaMode;
	bool bStatus = FALSE;
	uint i = 0;

	u8 ucCon = 0;

	Disp("WriteBlocks() in %s mode...\n", ATA_GetModeName(mode));

	switch (mode) {
	case PIO_CPU:
		bStatus = rw_sector_pio(1, lba, sector, buf);
		break;

	case PIO_DMA:
		for (i = 0; i < sector; i++) {
			ATA_WriteSectors_PIODMA(ucCon, lba + i,
						buf + i*ATA_SECTORSIZE);
			wait_for_trans_done();
		}
		bStatus = TRUE;
		break;

	case UDMA:
		bStatus = rw_sector_udma(1, lba, sector, buf);
		break;
	default:
		Disp("Not supported mode in WriteBlocks()\n");
		break;
	}

	return bStatus;
}

static bool ATA_OpenMedia (u8 ucCon, eATA_MODE_6400 eAtaMode)
{
	eATA_MODE_6400 eMode = eATA_MODE_PIO;

	Disp("ATA Mode : %s\n", ATA_GetModeName(eAtaMode));

	identify_device();

	if (eAtaMode == eATA_MODE_NONE) {
		if (g_oaATAInform[ucCon].eMaxUdmaMode == 0) {
			eMode = eATA_MODE_PIO;
		} else {
			eMode = eATA_MODE_UDMA;
		}
	} else {
		eMode = eAtaMode;
	}

	g_oaATAInform[ucCon].eAtaMode = eMode;

	switch (eMode) {
	case eATA_MODE_PIOCPU:
	case eATA_MODE_PIODMA:
		set_pio_mode(g_oaATAInform[ucCon].eMaxPioMode);
		break;
	case eATA_MODE_UDMA:
		set_pio_mode(eATA_PIO0);
		set_udma_mode(g_oaATAInform[ucCon].eMaxUdmaMode);
		break;
	default:
		Disp("Invalid ATA Mode!!\n");
		return false;
	}
	return true;
}

static bool ATA_CloseMedia(u8 ucCon)
{
	return true;
}


static void ATA_PrintDeviceInfo(void)
{
	identify_device();
}

#define CODEC_MEM_ST 0x51000000

static void test_pio_mode (void)
{
	uint *wbuf = (uint*)CODEC_MEM_ST;
	uint *rbuf = (uint*)(CODEC_MEM_ST + 0x400000);
	uint lba = 0;
	uint sector = 0;
	uint i;

	ATA_OpenMedia(0, eATA_MODE_PIOCPU);

	lba = 0x6a00;
	sector = 0x1242;
	printf("make buffers\n");
	for (i = 0; i < sector * (512/4); i++) {
		wbuf[i] = (lba << 16) | i;
	}
	memset(rbuf, 0, sector * 512);

	write_block(lba, sector, (u32)wbuf);
	read_block(lba, sector, (u32)rbuf);

	if (cmp32((u32)wbuf, (u32)rbuf, sector * (512/4))) {
		Disp("Error detected\n");
		dump32((u32)rbuf, sector * (512/4));
	} else {
		Disp("Write/Read operation is OK\n");
	}

	ATA_CloseMedia(0);
}

static void test_piodma_mode (void)
{
	uint *wbuf = (uint*)CODEC_MEM_ST;
	uint *rbuf = (uint*)(CODEC_MEM_ST + 0x400000);
	u32 lba = 0;
	u32 sector = 0;
	uint i;

	ATA_OpenMedia(0, eATA_MODE_PIODMA);

	lba = 0x5b00;
	sector = 0x412;
	for (i = 0; i < sector * 512/4; i++) {
		wbuf[i] = (lba << 16) | i;
	}
	memset(rbuf, 0, sector * 512);

	write_block(lba, sector, (u32)wbuf);
	read_block(lba, sector, (u32)rbuf);

	if (cmp32((u32)wbuf, (u32)rbuf, sector * 128)) {
		Disp("Error detected\n");
		dump32((u32)rbuf, sector * 128);
	} else {
		Disp("Write/Read operation is OK\n");
	}

	ATA_CloseMedia(0);
}

static void test_udma_mode (void)
{
	uint *wbuf = (uint*)CODEC_MEM_ST;
	uint *rbuf = (uint*)(CODEC_MEM_ST + 0x1000000);
	u32 lba = 0;
	u32 sector = 0;
	uint i;

	ATA_OpenMedia(0, eATA_MODE_UDMA);

	lba = 0x5400;
	sector = 0x10;
	for (i = 0; i < sector * 512/4; i++) {
		wbuf[i] = (lba << 16) | i;
	}
	memset(rbuf, 0, sector * 512);

	write_block(lba, sector, (u32)wbuf);
	read_block(lba, sector, (u32)rbuf);

	if (cmp32((u32)wbuf, (u32)rbuf, sector * 128)) {
		Disp("Error detected\n");
//		dump32((u32)rbuf, sector * 128);
	} else {
		Disp("Write/Read operation is OK\n");
	}

	ATA_CloseMedia(0);
}

void test_ata (void)
{
	uint i;

//	GPIO_SetPullUpDownEach(eGPIO_P, eGPIO_14, 2);

	Disp("\n");

	/* direct mode */
	if (ata_init(DIRECT_MODE)) {
		Disp("Fail to initialize ATA Mode...\n");
		return;
	}

	/* init ata devices several times */
	for (i=0; i<3; i++) {
		Disp("\n");
		ATA_PrintDeviceInfo();
	}

	test_pio_mode();
//	test_piodma_mode();
	test_udma_mode();

//      ATA_ReturnPort(0);
	ATA_SetEnable(0, DISABLE);
	CF_SetMUXReg(eCF_MUX_OUTPUT_DISABLE, eCF_MUX_CARDPWR_OFF, eCF_MUX_MODE_PCCARD);

}

