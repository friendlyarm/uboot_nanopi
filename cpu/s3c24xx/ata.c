#include <common.h>

#ifdef CONFIG_SMDK2443

#include <regs.h>
#include <asm/io.h>

#include "ata.h"
//#include "etc.h"

uint HCLK;

ATA_MODE m_eMode;

uint max_multiple;
uint m_uCurrentMultiple;
uint m_uMaxSectors;
PIOMODE m_eMaxPioMode;
PIOMODE m_eCurrentPioMode;
uint m_uMaxUdmaMode;
uint m_uCurrentUdmaMode;

#ifdef DBG_ATA
#define DbgAta(x...) printf(x)
#else
#define DbgAta(x...) do {} while(0);
#endif

#define rEBICON	(*(vu_long *)0x4E800008)	//Bank Configuration - edited by junon
#define rGPGCON	(*(vu_long *)0x56000060)	//Configure the pins of portt G
#define rGPACON	(*(vu_long *)0x56000000)	//Configure the pins of port A
#define rMISCCR	(*(vu_long *)0x56000080)	//Miscellaneous control
#define rGPBDAT	(*(vu_long *)0x56000014)	//The data for port B
#define rGPBCON	(*(vu_long *)0x56000010)	//Configure the pins of port B

/*=========================================================================
 *          	          ata Register Address
 *=========================================================================
 */

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

/***************************************************************************/

#define ATAPI_MASK		0xffffffe0
#define ATA_SECTORSIZE		512

#define STATUS_DEVICE_BUSY	0x80
#define STATUS_DATA_REQUEST	0x58
#define STATUS_ERR		0x1

#define CODEC_MEM_ST 0x31000000

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
	uint i;

	for (i=0; i<100000; i++) {
		reg = readl(ATA_IRQ);
		if (reg & 0x01)
			break;
	}
	if (i == 100000) {
		printf("transmission timeout\n");
		return -1;
	}

	reg = readl(ATA_STATUS);
	printf("ATA_STATUS: %08x\n", reg);
	if (reg & 3) {
		printf("transmission failed\n");
		return -1;
	}
}

static int wait_for_dev_ready (void)
{
	u8 temp;
	uint i;

	for (i=0; i<1000000; i++) {
		temp = read_dev_reg(DEV_STATUS);
//		printf("temp2: %08x\n", temp);
		if ((temp & STATUS_DEVICE_BUSY) == 0) {
			printf("temp2: %08x\n", temp);
			DbgAta("wait_for_dev_ready: %d\n", i);
			return 0;
		}
	}

	printf("wait_for_dev_ready: error\n");
	return -1;
}

/*=========================================================================
 *          	    ata controller register functions
 *=========================================================================
 */


/* GPB0,4,5,6 output setting (TOUT0, TCLK - TP21,20) */
static void init_buf_ctrl (void)
{
/* modification of EVT1 */
#if 1
	u32 reg = readl(GPADAT);

	reg &= ~(1<<5);
	writel(reg, GPADAT);
#else
	u32 reg = readl(GPBCON);

	reg &= 0xffffc0fc;
	reg |= (1<<12 | 1<<10 | 1<<8 | 1<<0);
	writel(reg, GPBCON);
	DbgAta("rGPBCON = 0x%08x\n", rGPBCON);
#endif
}

static void change_board_buf_mode (ATA_MODE mode, uint rw)
{
	u32 reg = readl(GPBDAT) & 0xffffff8e; /* 0,4,5,6 */

	if (mode == UDMA) {
		/* GPB0->high,GPB4->high => UDMA mode */
		reg |= (1<<4)|(1);
		if (rw)
			reg |= (1<<5);
	} else {
		/* GPB0->high,GPB4->low => PIO mode */
		reg |= (1<<6)|1;
	}
	writel(reg, GPBDAT);
//	printf("GPBDAT = 0x%08x\n", readl(GPBDAT));
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
	u32 reg = readl(ATA_IRQ);
	writel(reg, ATA_IRQ);

//	wait_for_host_ready();
	writel(cmd, ATA_COMMAND);
}

/*=========================================================================
 *          				FIU BASIC routine
 *=========================================================================
 */

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

static void ata_init (void)
{

	HCLK = get_HCLK();
	/*  TDelay timer setting */
	udelay(100);	/*  TDelay 1 unit = 10us */

	/*  GPIO, EBI setting */
	/*  bank3_cfg->CF,bank2_cfg->CF */
	rEBICON |= (1 << 10) | (1 << 9);
	/* nCARD_PWREN, RESET_CF,nRE3G_CF,nINPACK,nIREQ_CF */
	rGPGCON |= (3 << 30) | (3 << 28) | (3 << 26) | (3 << 24) | (3 << 22);
	/*  nWE_CF,nOE_CF,nRCS3,nRCS2 enable */
	rGPACON |= (1 << 27) | (1 << 11) | (1 << 14) | (1 << 13);
	/*  card detect when card is detected ,the bit should be '0'. */
	rMISCCR &= (~(1 << 30));

	/* Buffer direction mode setting for SMDK2443 060902 */
	init_buf_ctrl();
	change_board_buf_mode(PIO_CPU, 0);

	/* CF controller - True IDE mode setting */
	change_mode_to_ata();

	/* Card configuration */
	writel(0x1C238, ATA_PIO_TIME); /* 0x1C238 */
	writel(0x20B1362, ATA_UDMA_TIME); /* 0x20B1362 */
	set_endian_mode(1);
	set_ata_enable(1);
	udelay(400000);		/*  wait for 200ms, be positively necessary */

/*       ResetAll(); //x. why? */
}

/*
 * no error : return 0
 * got error: return 1
 */
static int identify_device(void)
{
	ushort buf[ATA_SECTORSIZE / 2];
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

	m_uMaxSectors = (swab16(buf[61]) << 16) + swab16(buf[60]);
	printf("\nMax Sectors : %d\n", m_uMaxSectors);

	// Caution: buf[x] - Big Endian, so upper byte means LSB..
	max_multiple = (buf[47] >> 8) & 0xFF;
	printf("\nMax Multiple : %02X\n", max_multiple);
	if (buf[59] & 0x1) {	//multiple sector setting is valid
		m_uCurrentMultiple = (buf[59] >> 8) & 0xFF;
		printf("Current Multiple : %03X\n", m_uCurrentMultiple);
	}

	if (((buf[64] >> 8) & 0x3) == 1)
		m_eMaxPioMode = PIO3;
	else if (((buf[64] >> 8) & 0x3) == 3)
		m_eMaxPioMode = PIO4;
	else
		m_eMaxPioMode = PIO2;
	printf("Max PIO Mode : %d\n", m_eMaxPioMode);

	m_uMaxUdmaMode = 0;
	temp = buf[88] >> 8;
	for (i = 4; i >= 0; i--) {
		if (temp & (0x01 << i)) {
			m_uMaxUdmaMode = i;
			break;
		}
	}

	m_uCurrentUdmaMode = 0;
	temp = buf[88] & 0x00ff;
	for (i = 0; i < 5; i++) {
		if (temp & (0x01 << i)) {
			m_uCurrentUdmaMode = i;
			break;
		}
	}

	printf("Max UDMA Mode : %d\n", m_uMaxUdmaMode);
	printf("Current UDMA Mode : %d\n", m_uCurrentUdmaMode);

	set_endian_mode(1);

	return 0;
}

static uint get_max_sector (void)
{
	return m_uMaxSectors;
}

#if 0
static void ATA_GetMaxMultiple(uint * maxMulti)
{
	*maxMulti = max_multiple;
}
#endif

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

static void set_pio_mode (PIOMODE pmode)
{
	uint uT1, uT2, uTeoc;
	uint i;

	uint pio_time[5];
	uint m_uPioT1[5] = {70,50,30,30,30};
	uint m_uPioT2[5] = {290,290,290,80,70};
	uint m_uPioTeoc[5] = {20,20,10,10,10};

	uint uCycleTime = (uint) (1000000000 / HCLK);
	u32 reg = readl(ATA_CFG);

	for (i = 0; i < 5; i++) {
		uT1 = (m_uPioT1[i] / uCycleTime) & 0xff;
		uT2 = (m_uPioT2[i] / uCycleTime) & 0xff;
		uTeoc = (m_uPioTeoc[i] / uCycleTime) & 0x0f;
		pio_time[i] = (uTeoc << 12) | (uT2 << 4) | uT1;
 		DbgAta("PIO%dTIME = %x\n", i, pio_time[i]);
	}

	writel(0xff, ATA_IRQ);
	writel(ATAPI_MASK, ATA_IRQ_MASK);


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
	case PIO0:
	case PIO1:
	case PIO2:
		reg &= (~0x2);	/* IORDY disable */
		break;
	case PIO3:
	case PIO4:
		reg |= 0x2;	/* IORDY enable */
		break;
	default:
		printf("unknown pmode: %d\n", pmode);
		return;
	}
	writel(pio_time[pmode], ATA_PIO_TIME);
	writel(reg, ATA_CFG);
	m_eCurrentPioMode = pmode;
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
static void set_udma_mode (UDMAMODE umode)
{
	uint uTdvh1, uTdvs, uTrp, uTss, uTackenv;
	uint i;

	uint uUdmaTime[5];
	uint uUdmaTdvh[5] = {10,10,10,10,10};
	uint uUdmaTdvs[5] = {100,60,50,35,20};
	uint uUdmaTrp[5] = {160,125,100,100,100};
	uint uUdmaTss[5] = {50,50,50,50,50};
	uint uUdmaTackenvMin[5] = {20,20,20,20,20};
	uint uCycleTime = (uint) (1000000000 / HCLK);

	printf("umode : %d\n", umode);

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
	writel(ATAPI_MASK, ATA_IRQ_MASK);

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
		m_uCurrentUdmaMode = umode;
		break;
	default:
		DbgAta(("Wrong UDMA mode in set_udma_mode()\n"));
		return;
	}
}

#if 0
static bool ATA_OpenMedia (void)
{
	ATA_MODE mode;

	ata_init();

	if (identify_device()) {
		return 0;
	}

	// Decide what mode is best choice in this media.
	if (m_uMaxUdmaMode == 0) {
		mode = PIO_DMA;
	} else {
		mode = UDMA;
	}

	if (mode == PIO_CPU || mode == PIO_DMA) {
		set_pio_mode(m_eMaxPioMode);
	} else {
		UDMAMODE udmaMode =
		    (m_uMaxUdmaMode == 0) ? UDMA0 :
		    (m_uMaxUdmaMode == 1) ? UDMA1 :
		    (m_uMaxUdmaMode == 2) ? UDMA2 :
		    (m_uMaxUdmaMode == 3) ? UDMA3 : UDMA4;
		set_pio_mode(PIO0);
		set_udma_mode(udmaMode);
	}

	return true;
}
#endif

static int open_media (ATA_MODE mode)
{
	ata_init();

	if (identify_device()) {
		return 1;
	}

	if (mode == PIO_CPU || mode == PIO_DMA) {
		set_pio_mode(m_eMaxPioMode);
	} else {
		set_pio_mode(PIO0);
		set_udma_mode(m_uMaxUdmaMode);
	}

	return 0;
}

static bool close_media (void)
{
	set_ata_enable(0);

	/* Output pad disable, Card power off, PC card mode */
	writel(0x06, ATA_MUX);

	return true;
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

static int check_dma_done (uint mode, int rw)
{
	ulong reg;

	printf("@@@check_dma_done\n");
	wait_for_trans_done();
	printf("ATA_IRQ: %08lx\n", reg = readl(ATA_IRQ));
	writel(reg, ATA_IRQ);
	wait_for_dev_ready();

#if 0
	set_trans_command(ATA_CMD_ABORT);
	printf("ATA_IRQ1: %08x\n", readl(ATA_IRQ));
	wait_for_dev_ready();
#endif

	/* apply at specific set using buffer control */
	if (mode == UDMA) {
		change_board_buf_mode(PIO_CPU, 0);
	}
	printf("ATA_IRQ2: %08lx\n", reg = readl(ATA_IRQ));
	writel(reg, ATA_IRQ);
	wait_for_dev_ready();

	set_config_mode(PIO_CPU, rw);
	printf("ATA_IRQ3: %08lx\n", reg = readl(ATA_IRQ));
	writel(reg, ATA_IRQ);
	wait_for_dev_ready();

#if 0
	/* disable ATA DMA auto mode */
	if (mode == UDMA) {
		u32 reg = readl(ATA_CFG);
		reg &= (~0x200);
		writel(reg, ATA_CFG);
		printf("ATA_CFG: %08x\n", readl(ATA_CFG));
	}
	printf("ATA_IRQ4: %08x\n", readl(ATA_IRQ));
	wait_for_dev_ready();
#endif
	return 0;
}

#if 0
static void ATA_ContinueReadingBlocks(void)
{

}
#endif

#if 1
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
#else
/* test code for kernel. do not use on common test. by scsuh */
static int start_rw_block (uint rw, uint mode, uint sector, uint count, uint buf)
{
	int i, cnt_over = 0;
	int left = count;
	uint cmd;
	ulong reg;

	if (left < 2) {
		printf("input more than 2\n");
		return 0;
	}
	printf("  start_rw_block: %d, %d, %x, %x, %08x\n", rw, mode, sector, count, buf);
	if (rw) {
		cmd = (mode == UDMA) ? WRITEDMA : WRITESECTOR;
	} else {
		cmd = (mode == UDMA) ? READDMA : READSECTOR;
	}
	writel(count * ATA_SECTORSIZE, ATA_XFR_NUM);

	set_ata_device(sector, count);

	write_dev_reg(DEV_COMMAND, cmd);
	wait_for_dev_ready();

	/* apply at specific set using buffer control */
	if (mode == UDMA) {
		change_board_buf_mode(mode, rw);
	}

	set_config_mode(mode, rw);

	for (i=0; i<128, 0<left; i++, left-=2) {
		printf("left: %d\n", left);

		writel(2*ATA_SECTORSIZE-0x20, rw ? ATA_SBUF_SIZE:ATA_TBUF_SIZE);
		writel(buf+1024*i, rw ? ATA_SBUF_START:ATA_TBUF_START);
		set_trans_command(i ? ATA_CMD_CONTINUE:ATA_CMD_START);

		/* now just wait till done */
		do {
			reg = readl(ATA_IRQ);
			cnt_over++;
			if (cnt_over > 100000) {
				printf("IRQ time out\n");
				break;
			}
		} while(!reg);

		printf("ATA_IRQ: %08lx\n", reg);
		printf("ATA_XFR_CNT: %08lx\n", readl(ATA_XFR_CNT));
		printf("ATA_STATUS: %08lx\n", readl(ATA_STATUS));
		printf("==========================\n\n");
		if (reg & 1)
			break;
		writel(reg, ATA_IRQ);
	}
	wait_for_dev_ready();
	wait_for_trans_done();

	/* apply at specific set using buffer control */
	if (mode == UDMA) {
		change_board_buf_mode(PIO_CPU, 0);
	}
	printf("ATA_IRQ2: %08lx\n", reg = readl(ATA_IRQ));
	writel(reg, ATA_IRQ);
	wait_for_dev_ready();

	set_config_mode(PIO_CPU, rw);
	printf("ATA_IRQ3: %08lx\n", reg = readl(ATA_IRQ));
	writel(reg, ATA_IRQ);
	wait_for_dev_ready();

	return 0;
}
#endif

static bool rw_sector_pio (uint rw, uint lba, uint sector, uint buf)
{
	uint sector_remain = sector, lba_current = lba;
	uint sector_current, round = 0;
	u16 * addr;
	uint i;
	uint cmd = rw ? WRITESECTOR : READSECTOR;
	u32 reg;

	writel(0xff, ATA_IRQ);
	printf("\nATA_IRQ0: %08x\n", readl(ATA_IRQ));
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
		printf("ATA_IRQ1: %08x\n", reg = readl(ATA_IRQ));
		writel(reg, ATA_IRQ);
		write_dev_reg(DEV_COMMAND, cmd);

		while (sector_current--) {
			printf("ATA_IRQ3: %08x\n", reg = readl(ATA_IRQ));
			writel(reg, ATA_IRQ);
			wait_for_dev_ready();
			printf("ATA_IRQ4: %08x\n", reg = readl(ATA_IRQ));
			writel(reg, ATA_IRQ);
			for (i = 0; i < ATA_SECTORSIZE / 2; i++) {
				if (rw)
					write_data(*addr);
				else
					*addr = read_data();

				addr++;
			}
			printf("ATA_IRQ5: %08x\n", reg = readl(ATA_IRQ));
			writel(reg, ATA_IRQ);
		}
		printf("ATA_IRQ6: %08x\n", reg = readl(ATA_IRQ));
		writel(reg, ATA_IRQ);
		wait_for_dev_ready();
		printf("ATA_IRQ7: %08x\n", reg = readl(ATA_IRQ));
		writel(reg, ATA_IRQ);

		round++;
	}

	return 0;
}

static int rw_sector_single_piodma (uint rw, uint lba, uint buf)
{
	start_rw_block(rw, PIO_DMA, lba, 1, buf);
	return 0;
}

/*
 * static int rw_sector_udma (int rw, uint lba, uint sector, uint buf)
 * rw: 0-r, 1-w
 */
static int rw_sector_udma (int rw, uint lba, uint sector, uint buf)
{
	uint sector_current;
	uint sector_remain = sector;
	uint lba_current;
	uint addr;
	uint round = 0;

	while (sector_remain != 0) {
		if (sector_remain > 256) {
			sector_current = 256;
			sector_remain -= 256;
		} else {
			sector_current = sector_remain;
			sector_remain = 0;
		}
		lba_current = lba + round * 256;
		addr = buf + round * 256 * ATA_SECTORSIZE;

		start_rw_block(rw, UDMA, lba_current, sector_current, addr);

		round++;
	}

	return 0;
}

static int write_sector_piodma (uint rw, uint lba, uint buf)
{
	/*Source Buffer 1 Setting */
	writel(buf, ATA_SBUF_START);
	writel(m_uCurrentMultiple * ATA_SECTORSIZE, ATA_SBUF_SIZE);
	writel(m_uCurrentMultiple * ATA_SECTORSIZE, ATA_XFR_NUM);

	set_ata_device(lba, m_uCurrentMultiple);
	write_dev_reg(DEV_COMMAND, WRITEMULTIPLE);

	wait_for_dev_ready();
	set_config_mode(PIO_DMA, rw);

	/*ATA Transfer Command */
	set_trans_command(ATA_CMD_START);
	wait_for_trans_done();
	set_trans_command(ATA_CMD_ABORT);

	set_config_mode(PIO_CPU, rw);
	wait_for_dev_ready();

	return true;
}

static int read_sector_piodma (uint rw, uint lba, uint buf)
{
	/*Track Buffer 1 Setting */
	writel(buf, ATA_TBUF_START);
	writel((m_uCurrentMultiple * ATA_SECTORSIZE), ATA_TBUF_SIZE);
	writel((m_uCurrentMultiple * ATA_SECTORSIZE), ATA_XFR_NUM);

	set_ata_device(lba, m_uCurrentMultiple);
	write_dev_reg(DEV_COMMAND, READMULTIPLE);

	wait_for_dev_ready();
	set_config_mode(PIO_DMA, rw);

	set_trans_command(ATA_CMD_START);
	wait_for_trans_done();
	set_trans_command(ATA_CMD_ABORT);

	set_config_mode(PIO_CPU, rw);
	wait_for_dev_ready();

	return true;
}

static int read_block (ATA_MODE mode, uint lba, uint count, uint buf)
{
	uint k;
	bool status = 0;

	switch (mode) {
	case PIO_CPU:
		printf("PIO cpu mode: read");
		status = rw_sector_pio(0, lba, count, buf);
		break;

	case PIO_DMA:
		printf("PIO DMA mode: read");
		for (k = 0; k < count; k++) {
			rw_sector_single_piodma(0, lba + k, buf + ATA_SECTORSIZE * k);
			wait_for_trans_done();
		}
		status = 0;	/* need to move.. */
		break;

	case UDMA:
		printf("UDMA mode: read");
		status = rw_sector_udma(0, lba, count, buf);
		break;

	default:
		printf("Not supported mode in ReadBlocks()\n");
		return 1;
	}

	printf("  - done\n");
	return status;
}

static int write_block (ATA_MODE mode, uint lba, uint count, uint buf)
{
	uint i;
	bool status = 0;

	switch (mode) {
	case PIO_CPU:
		printf("PIO cpu mode: write");
		status = rw_sector_pio(1, lba, count, buf);
		break;

	case PIO_DMA:
		printf("PIO DMA mode: write");
		for (i = 0; i < count; i++) {
			rw_sector_single_piodma(1, lba + i, buf + ATA_SECTORSIZE * i);
			wait_for_trans_done();
		}
		status = 0;
		break;

	case UDMA:
		printf("UDMA mode: write");
		status = rw_sector_udma(1, lba, count, buf);
		break;

	default:
		printf("Not supported mode in WriteBlocks()\n");
		return 1;
	}

	printf(" - done\n");
	return status;
}

static void init_test_mode (int trans_mode)
{
	char temp[10];

	if (open_media(trans_mode)) {
		return;
	}

	switch (trans_mode) {
	case PIO_CPU:
		sprintf(temp, "PIO_CPU");
		break;
	case PIO_DMA:
		sprintf(temp, "PIO_DMA");
		break;
	case UDMA:
		sprintf(temp, "UDMA");
		break;
	}

}

/*
 * static void test_mode (int mode, uint lba, uint sector, uint rw_mode)
 *
 * mode: Transfer mode
 *	0 - pio cpu
 *	1 - pio dma
 *	2 - UDMA
 * lba: sector number
 * sector: sector count
 * rw_mode: r/o, w/o, rw
 *	1 - read only
 *	2 - write only
 *	3 - read and write
 */
static int test_mode (int trans_mode, uint lba, uint sector, uint rw_mode)
{
	uint *wbuf = (uint*)CODEC_MEM_ST;
	uint *rbuf = (uint*)(CODEC_MEM_ST + 0x400000);
	uint max_sector;
	uint i;
	char temp[10];
	int ret = 0;

	max_sector = get_max_sector();
	if ((lba + sector) > max_sector) {
		printf("input value error\n");
	}

	for (i = 0; i < sector * 512/4; i++) {
		wbuf[i] = (lba << 16) | i;
	}
	memset(rbuf, 0, sector * 512);

	if (rw_mode & 2)
		write_block(trans_mode, lba, sector, wbuf);
	if (rw_mode & 1)
		read_block(trans_mode, lba, sector, rbuf);

	if (rw_mode == 3) {
		if (cmp32(wbuf, rbuf, sector * 128)) {
			printf("Error detected\n");
			if (sector <0x10) {
				dump32(wbuf, sector * 128);
				printf("\n\n\n\n");
				dump32(rbuf, sector * 128);
			}
			ret = -1;
		} else {
			printf("Write/Read in %s mode is OK\n", temp);
			ret = 0;
		}
	}
	printf("=========================================================\n\n\n");

	return ret;
}

void test_ata (int mode, uint lba, uint sector)
{
	int i;

	if (mode & 0x1) {
		init_test_mode(PIO_CPU);
		test_mode(PIO_CPU, lba, sector, 3);
		close_media();
	}
	if (mode & 0x2) {
		init_test_mode(PIO_DMA);
		test_mode(PIO_DMA, lba+5, sector, 3);
		close_media();
	}
	if (mode & 0x4) {
		init_test_mode(UDMA);
		test_mode(UDMA, lba+0x10, sector, 3);
		close_media();
	}

	if (mode & 0x8) {
		init_test_mode(UDMA);
		for (i=0; i<0x1000; i++) {
			printf("%d: ", i);
			if (test_mode(UDMA, lba+i*2, sector, 2)) {
				break;
			}
			if (serial_tstc()) {
				break;
			}
		}
		close_media();
	}
	if (mode & 0x10) {
		init_test_mode(UDMA);
		for (i=0; i<0x1000; i++) {
			printf("%d: ", i);
			if (test_mode(UDMA, lba+i*2, sector, 1)) {
				break;
			}
			if (serial_tstc()) {
				break;
			}
		}
		close_media();
	}

#if 0
	if (mode & 0x8) {
		memset(0x31000000, 0, 0x00800000);

		test_mode(PIO_CPU, lba, sector, 2);
		test_mode(UDMA, lba, sector, 1);
	}
#endif
}

#endif

