#include <common.h>
#include <malloc.h>

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/s3c_onenand.h>
#include <linux/mtd/bbm.h>

#include <asm/io.h>
#include <asm/errno.h>

#include <rtc.h>

#undef DEBUG_ONENAND
#ifdef DEBUG_ONENAND
#define dbg(x...)       printf(x)
#else
#define dbg(x...)       do { } while (0)
#endif

#define	CONFIG_MTD_ONENAND_VERIFY_WRITE

/**
 * onenand_oob_64 - oob info for large (2KB) page
 */
static struct nand_ecclayout onenand_oob_64 = {
	.eccbytes	= 20,
	.eccpos		= {
		8, 9, 10, 11, 12,
		24, 25, 26, 27, 28,
		40, 41, 42, 43, 44,
		56, 57, 58, 59, 60,
		},
		
	/* For holding Yaffs2 tag */
	.oobfree	= {
		{2, 6}, {13, 3}, {18, 6}, {29, 3},
		{34, 6}, {45, 3}, {50, 6}, {61, 3}}
#if 0
	/* original */
	.oobfree	= {
		{2, 6}, {14, 2}, {18, 6}, {30, 2},
		{34, 6}, {46, 2}, {50, 6}}
#endif
};

/**
 * onenand_oob_32 - oob info for middle (1KB) page
 */
static struct nand_ecclayout onenand_oob_32 = {
	.eccbytes	= 10,
	.eccpos		= {
		8, 9, 10, 11, 12,
		24, 25, 26, 27, 28,
		},
	.oobfree	= { {2, 3}, {14, 2}, {18, 3}, {30, 2} }
};

static const unsigned char ffchars[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 16 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 32 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 48 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 64 */
};

static unsigned int onenand_readl(void __iomem *addr)
{
	return readl(addr);
}

static void onenand_writel(unsigned int value, void __iomem *addr)
{
	writel(value, addr);
}

static void onenand_irq_wait(struct onenand_chip *chip, int stat)
{
	while (!chip->read(chip->base + ONENAND_REG_INT_ERR_STAT) & stat);
}

static void onenand_irq_ack(struct onenand_chip *chip, int stat)
{
	chip->write(stat, chip->base + ONENAND_REG_INT_ERR_ACK);
}

static int onenand_irq_pend(struct onenand_chip *chip, int stat)
{
	return (chip->read(chip->base + ONENAND_REG_INT_ERR_STAT) & stat);
}

static void onenand_irq_wait_ack(struct onenand_chip *chip, int stat)
{
	onenand_irq_wait(chip, stat);
	onenand_irq_ack(chip, stat);
}


static int onenand_blkrw_complete(struct onenand_chip *chip, int cmd)
{
	int cmp_bit = 0, fail_bit = 0, ret = 0;

	if (cmd == ONENAND_CMD_READ) {
		cmp_bit = ONENAND_INT_ERR_LOAD_CMP;
		fail_bit = ONENAND_INT_ERR_LD_FAIL_ECC_ERR;
	} else if (cmd == ONENAND_CMD_PROG) {
		cmp_bit = ONENAND_INT_ERR_PGM_CMP;
		fail_bit = ONENAND_INT_ERR_PGM_FAIL;
	} else {
		ret = 1;
	}
	
	onenand_irq_wait_ack(chip, ONENAND_INT_ERR_INT_ACT);
	onenand_irq_wait_ack(chip, ONENAND_INT_ERR_BLK_RW_CMP);
	onenand_irq_wait_ack(chip, cmp_bit);

	if (onenand_irq_pend(chip, fail_bit)) {
		onenand_irq_ack(chip, fail_bit);
		ret = 1;
	}

	return ret;
}

/**
 * onenand_read_burst
 *
 * 16 Burst read: performance is improved up to 40%.
 */
static void onenand_read_burst(void *dest, const void *src, size_t len)
{
	int count;

	if (len % 16 != 0)
		return;

	count = len / 16;
	
	__asm__ __volatile__(
		"	stmdb	r13!, {r0-r3,r9-r12}\n"
		"	mov	r2, %0\n"
		"1:\n"
		"	ldmia	r1, {r9-r12}\n"
		"	stmia	r0!, {r9-r12}\n"
		"	subs	r2, r2, #0x1\n"
		"	bne	1b\n"
		"	ldmia	r13!, {r0-r3,r9-r12}\n"::"r" (count));
}

/**
 * onenand_command_map - [DEFAULT] Get command type
 * @param cmd		command
 * @return		command type (00, 01, 10, 11)
 *
 */
static int onenand_command_map(int cmd)
{
	int type = ONENAND_CMD_MAP_FF;

	switch (cmd) {
	case ONENAND_CMD_READ:
	case ONENAND_CMD_READOOB:
	case ONENAND_CMD_PROG:
	case ONENAND_CMD_PROGOOB:
		type = ONENAND_CMD_MAP_01;
		break;

	case ONENAND_CMD_UNLOCK:
	case ONENAND_CMD_LOCK:
	case ONENAND_CMD_LOCK_TIGHT:
	case ONENAND_CMD_UNLOCK_ALL:
	case ONENAND_CMD_ERASE:
	case ONENAND_CMD_OTP_ACCESS:
	case ONENAND_CMD_PIPELINE_READ:
	case ONENAND_CMD_PIPELINE_WRITE:
		type = ONENAND_CMD_MAP_10;
		break;

	case ONENAND_CMD_RESET:
	case ONENAND_CMD_READID:
		type = ONENAND_CMD_MAP_11;
		break;
	default:
		type = ONENAND_CMD_MAP_FF;
		break;
	}

	return type;
}

/**
 * onenand_addr_field - [DEFAULT] Generate address field
 * @param dev_id	device id
 * @param fba		block number
 * @param fpa		page number
 * @param fsa		sector number
 * @return		address field
 *
 * Refer to Table 7-1 MEM_ADDR Fields in S3C6400/10 User's Manual
 */
#if defined(CONFIG_S3C6400)
static u_int onenand_addr_field(int dev_id, int fba, int fpa, int fsa)
{
	u_int mem_addr = 0;
	int ddp, density;	

	ddp = dev_id & ONENAND_DEVICE_IS_DDP;
	density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;

	switch (density & 0xf) {
	case ONENAND_DEVICE_DENSITY_128Mb:
		mem_addr = (((fba & ONENAND_FBA_MASK_128Mb) << ONENAND_FBA_SHIFT_128Mb) | \
				((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_128Mb) | \
				(fsa << ONENAND_FSA_SHIFT));
		break;

	case ONENAND_DEVICE_DENSITY_256Mb:
		mem_addr = (((fba & ONENAND_FBA_MASK_256Mb) << ONENAND_FBA_SHIFT_256Mb) | \
				((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_256Mb) | \
				(fsa << ONENAND_FSA_SHIFT));
		break;

	case ONENAND_DEVICE_DENSITY_512Mb:
		mem_addr = (((fba & ONENAND_FBA_MASK_512Mb) << ONENAND_FBA_SHIFT_512Mb) | \
				((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_512Mb) | \
				((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		break;

	case ONENAND_DEVICE_DENSITY_1Gb:
		if (ddp) {
			mem_addr = ((ddp << ONENAND_DDP_SHIFT_1Gb) | \
				((fba & ONENAND_FBA_MASK_1Gb_DDP) << ONENAND_FBA_SHIFT_1Gb_DDP) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_1Gb_DDP) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		} else {
			mem_addr = (((fba & ONENAND_FBA_MASK_1Gb) << ONENAND_FBA_SHIFT_1Gb) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_1Gb) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		}
		
		break;

	case ONENAND_DEVICE_DENSITY_2Gb:
		if (ddp) {
			mem_addr = ((ddp << ONENAND_DDP_SHIFT_2Gb) | \
					((fba & ONENAND_FBA_MASK_2Gb_DDP) << ONENAND_FBA_SHIFT_2Gb_DDP) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_2Gb_DDP) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		} else {
			mem_addr = (((fba & ONENAND_FBA_MASK_2Gb) << ONENAND_FBA_SHIFT_2Gb) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_2Gb) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		}
		
		break;

	case ONENAND_DEVICE_DENSITY_4Gb:
		if (ddp) {
			mem_addr = ((ddp << ONENAND_DDP_SHIFT_4Gb) | \
					((fba & ONENAND_FBA_MASK_4Gb_DDP) << ONENAND_FBA_SHIFT_4Gb_DDP) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_4Gb_DDP) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		} else {
			mem_addr = (((fba & ONENAND_FBA_MASK_4Gb) << ONENAND_FBA_SHIFT_4Gb) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT_4Gb) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		}
		
		break;
	}

	return mem_addr;
}
#else
static u_int onenand_addr_field(int dev_id, int fba, int fpa, int fsa)
{
	u_int mem_addr = 0;
	int ddp, density;	

	ddp = dev_id & ONENAND_DEVICE_IS_DDP;
	density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;

	switch (density & 0xf) {
	case ONENAND_DEVICE_DENSITY_128Mb:
		mem_addr = (((fba & ONENAND_FBA_MASK_128Mb) << ONENAND_FBA_SHIFT) | \
				((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
				(fsa << ONENAND_FSA_SHIFT));
		break;

	case ONENAND_DEVICE_DENSITY_256Mb:
		mem_addr = (((fba & ONENAND_FBA_MASK_256Mb) << ONENAND_FBA_SHIFT) | \
				((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
				(fsa << ONENAND_FSA_SHIFT));
		break;

	case ONENAND_DEVICE_DENSITY_512Mb:
		mem_addr = (((fba & ONENAND_FBA_MASK_512Mb) << ONENAND_FBA_SHIFT) | \
				((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
				((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		break;

	case ONENAND_DEVICE_DENSITY_1Gb:
		if (ddp) {
			mem_addr = ((ddp << ONENAND_DDP_SHIFT_1Gb) | \
					((fba & ONENAND_FBA_MASK_1Gb_DDP) << ONENAND_FBA_SHIFT) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		} else {
			mem_addr = (((fba & ONENAND_FBA_MASK_1Gb) << ONENAND_FBA_SHIFT) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		}
		
		break;

	case ONENAND_DEVICE_DENSITY_2Gb:
		if (ddp) {
			mem_addr = ((ddp << ONENAND_DDP_SHIFT_2Gb) | \
					((fba & ONENAND_FBA_MASK_2Gb_DDP) << ONENAND_FBA_SHIFT) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		} else {
			mem_addr = (((fba & ONENAND_FBA_MASK_2Gb) << ONENAND_FBA_SHIFT) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		}
		
		break;

	case ONENAND_DEVICE_DENSITY_4Gb:
		if (ddp) {
			mem_addr = ((ddp << ONENAND_DDP_SHIFT_4Gb) | \
					((fba & ONENAND_FBA_MASK_4Gb_DDP) << ONENAND_FBA_SHIFT) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		} else {
			mem_addr = (((fba & ONENAND_FBA_MASK_4Gb) << ONENAND_FBA_SHIFT) | \
					((fpa & ONENAND_FPA_MASK) << ONENAND_FPA_SHIFT) | \
					((fsa & ONENAND_FSA_MASK) << ONENAND_FSA_SHIFT));
		}
		
		break;
	}

	return mem_addr;
}
#endif

/**
 * onenand_command_address - [DEFAULT] Generate command address
 * @param mtd		MTD device structure
 * @param cmd_type	command type
 * @param fba		block number
 * @param fpa		page number
 * @param fsa		sector number
 * @param onenand_addr	onenand device address to access directly (command 00/11)
 * @return		command address
 *
 * Refer to 'Command Mapping' in S3C6400 User's Manual
 */
static uint onenand_command_address(struct mtd_info *mtd, int cmd_type, int fba, int fpa, int fsa, int onenand_addr)
{
	struct onenand_chip *chip = mtd->priv;
	uint cmd_addr = (ONENAND_AHB_ADDR | (cmd_type << ONENAND_CMD_SHIFT));
	int dev_id;

	dev_id = chip->read(chip->base + ONENAND_REG_DEVICE_ID);
	
	switch (cmd_type) {
	case ONENAND_CMD_MAP_00:
		cmd_addr |= ((onenand_addr & 0xffff) << 1);
		break;
		
	case ONENAND_CMD_MAP_01:
	case ONENAND_CMD_MAP_10:
		cmd_addr |= (onenand_addr_field(dev_id, fba, fpa, fsa) & ONENAND_MEM_ADDR_MASK);
		break;
		
	case ONENAND_CMD_MAP_11:
		cmd_addr |= ((onenand_addr & 0xffff) << 2);
		break;
		
	default:
		cmd_addr = 0;
		break;
	}

	return cmd_addr;
}

/**
 * onenand_command - [DEFAULT] Generate command address
 * @param mtd		MTD device structure
 * @param cmd		command
 * @param addr		onenand device address
 * @return		command address
 *
 */
static uint onenand_command(struct mtd_info *mtd, int cmd, loff_t addr)
{
	struct onenand_chip *chip = mtd->priv;
	int sectors = 4, onenand_addr = -1;
	int cmd_type, fba = 0, fpa = 0, fsa = 0, page;
	uint cmd_addr;

	cmd_type = onenand_command_map(cmd);

	switch (cmd) {
	case ONENAND_CMD_UNLOCK:
	case ONENAND_CMD_UNLOCK_ALL:
	case ONENAND_CMD_LOCK:
	case ONENAND_CMD_LOCK_TIGHT:	
	case ONENAND_CMD_ERASE:
		fba = (int) (addr >> chip->erase_shift);
		page = -1;
		break;

	default:
		fba = (int) (addr >> chip->erase_shift);
		page = (int) (addr >> chip->page_shift);
		page &= chip->page_mask;
		fpa = page & ONENAND_FPA_MASK;
		fsa = sectors & ONENAND_FSA_MASK;
		break;
	}

	cmd_addr = onenand_command_address(mtd, cmd_type, fba, fpa, fsa, onenand_addr);

	if (!cmd_addr) {
		printk("Command address mapping failed\n");
		return -1;
	}

	return cmd_addr;
}

static int onenand_get_device(struct mtd_info *mtd, int new_state)
{
	return 0;
}

/**
 * onenand_release_device - [GENERIC] release chip
 * @param mtd		MTD device structure
 *
 * Deselect, release chip lock and wake up anyone waiting on the device
 */
static void onenand_release_device(struct mtd_info *mtd)
{
	return;
}

/**
 * onenand_block_checkbad - [GENERIC] Check if a block is marked bad
 * @param mtd		MTD device structure
 * @param ofs		offset from device start
 *
 * Check, if the block is bad. Either by reading the bad block table or
 * calling of the scan function.
 */
static int onenand_block_checkbad(struct mtd_info *mtd, loff_t ofs)
{
	struct onenand_chip *chip = mtd->priv;
	void __iomem *cmd_addr;
	u_int *buf_poi;
	int i, isbad = 0;

	buf_poi = (u_int *)malloc(mtd->writesize);

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_READING);

	/* on the TRANSFER SPARE bit */
	chip->write(ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);

	/* get start address to read data */
	cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_READ, ofs);

	switch (chip->options & ONENAND_READ_MASK) {
	case ONENAND_READ_BURST:
		onenand_read_burst(buf_poi, cmd_addr, mtd->writesize);
		onenand_read_burst(buf_poi, cmd_addr, mtd->oobsize);
		break;

	case ONENAND_READ_POLLING:
		/* read main data and throw into garbage box */
		for (i = 0; i < (mtd->writesize / 4); i++)
			*buf_poi = chip->read(cmd_addr);

		/* read spare data */
		for (i = 0; i < (mtd->oobsize / 4); i++)
			*buf_poi++ = chip->read(cmd_addr);

		buf_poi -= (mtd->oobsize / 4);

		break;
	}

	onenand_blkrw_complete(chip, ONENAND_CMD_READ);

	/* off the TRANSFER SPARE bit */
	chip->write(~ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);

	buf_poi[0] &= 0xffff;

	if (!buf_poi[0])
		isbad = 1;

	free(buf_poi);

	return isbad;
}

/**
 * onenand_block_isbad - [MTD Interface] Check whether the block at the given offset is bad
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 *
 * Check whether the block is bad
 */
static int onenand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	/* Check for invalid offset */
	if (ofs > mtd->size)
		return -EINVAL;

	return onenand_block_checkbad(mtd, ofs);
}

/**
 * onenand_set_pipeline - [MTD Interface] Set pipeline ahead
 * @param mtd		MTD device structure
 * @param from		offset to read from
 * @param len		number of bytes to read
 *
 */
static int onenand_set_pipeline(struct mtd_info *mtd, loff_t from, size_t len)
{
	struct onenand_chip *chip = mtd->priv;	
	int page_cnt = (int) (len >> chip->page_shift);
	void __iomem *cmd_addr;

	if (len % mtd->writesize > 0)
		page_cnt++;

	if (page_cnt > 1) {
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_PIPELINE_READ, from);
		chip->write(ONENAND_DATAIN_PIPELINE_READ | page_cnt, cmd_addr);
	}

	return 0;
}

/**
 * onenand_read - [MTD Interface] Read data from flash
 * @param mtd		MTD device structure
 * @param from		offset to read from
 * @param len		number of bytes to read
 * @param retlen	pointer to variable to store the number of read bytes
 * @param buf		the databuffer to put data
 *
 */
static int onenand_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct onenand_chip *chip = mtd->priv;
	struct mtd_ecc_stats stats;
	int i, ret = 0, read = 0, thislen;
	u_int *buf_poi = (u_int *)buf;
	void __iomem *cmd_addr;

	DEBUG(MTD_DEBUG_LEVEL3, "onenand_read: from = 0x%08x, len = %i\n", (unsigned int) from, (int) len);

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL3, "onenand_read: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_READING);

	/* TODO handling oob */
	stats = mtd->ecc_stats;

 	while (!ret) {
		if (chip->options & ONENAND_CHECK_BAD) {
			if (onenand_block_isbad(mtd, from)) {
				printk (KERN_WARNING "\nonenand_read: skipped to read from a bad block at addr 0x%08x.\n", (unsigned int) from);
				from += (1 << chip->erase_shift);
				continue;
			}
		}

		/* pipeline ahread read is only applied for reading from page 0 of any block in u-boot */
		if ((chip->options & ONENAND_PIPELINE_AHEAD) && ((from & 0x1ffff) == 0))			
			onenand_set_pipeline(mtd, from, mtd->erasesize);

		/* get start address to read data */
		cmd_addr = (void __iomem *)(chip->command(mtd, ONENAND_CMD_READ, from));

		switch (chip->options & ONENAND_READ_MASK) {
		case ONENAND_READ_BURST:
			onenand_read_burst(buf, cmd_addr, mtd->writesize);
			break;

		case ONENAND_READ_POLLING:
			for (i = 0; i < (mtd->writesize / 4); i++)
				*buf_poi++ = chip->read(cmd_addr);

			buf_poi -= (mtd->writesize / 4);
			break;

		default:
			printk("onenand_read: read mode undefined.\n");
			return -1;
		}
#if 0 // fixme: bug??
		if (onenand_blkrw_complete(chip, ONENAND_CMD_READ)) {
			printk(KERN_WARNING "onenand_read: Read operation failed.\n");
			return -1;
		}
#endif
		thislen = min_t(int, mtd->writesize, len - read);

		read += thislen;

		if (read == len)
 			break;
		
		buf += thislen;
		from += mtd->writesize;
 	}

	/* Deselect and wake up anyone waiting on the device */
	onenand_release_device(mtd);

	/*
	 * Return success, if no ECC failures, else -EBADMSG
	 * fs driver will take care of that, because
	 * retlen == desired len and result == -EBADMSG
	 */
	*retlen = read;

	if (mtd->ecc_stats.failed - stats.failed)
		return -EBADMSG;

	if (ret)
		return ret;

	return mtd->ecc_stats.corrected - stats.corrected ? -EUCLEAN : 0;
}

/**
 * onenand_transfer_auto_oob - [Internal] oob auto-placement transfer
 * @param mtd		MTD device structure
 * @param buf		destination address
 * @param column	oob offset to read from
 * @param thislen	oob length to read
 */
static int onenand_transfer_auto_oob(struct mtd_info *mtd, uint8_t *buf, int column,
				int thislen)
{
	struct onenand_chip *chip = mtd->priv;
	struct nand_oobfree *free;
	int readcol = column;
	int readend = column + thislen;
	int lastgap = 0;
	uint8_t *oob_buf = chip->oob_buf;

	for (free = chip->ecclayout->oobfree; free->length; ++free) {
		if (readcol >= lastgap)
			readcol += free->offset - lastgap;
		if (readend >= lastgap)
			readend += free->offset - lastgap;
		lastgap = free->offset + free->length;
	}

	for (free = chip->ecclayout->oobfree; free->length; ++free) {
		int free_end = free->offset + free->length;
		if (free->offset < readend && free_end > readcol) {
			int st = max_t(int,free->offset,readcol);
			int ed = min_t(int,free_end,readend);
			int n = ed - st;
			memcpy(buf, oob_buf + st, n);
			buf += n;
		} else
			break;
	}
	return 0;
}

/**
 * onenand_do_read_oob - [MTD Interface] OneNAND read out-of-band
 * @param mtd		MTD device structure
 * @param from		offset to read from
 * @param len		number of bytes to read
 * @param retlen	pointer to variable to store the number of read bytes
 * @param buf		the databuffer to put data
 * @param mode		operation mode
 *
 * OneNAND read out-of-band data from the spare area
 */
static int onenand_do_read_oob(struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, mtd_oob_mode_t mode)
{
	struct onenand_chip *chip = mtd->priv;
	int read = 0, thislen, column, oobsize;
	int i, ret = 0;
	void __iomem *cmd_addr;
	u_int *buf_poi;

	DEBUG(MTD_DEBUG_LEVEL3, "onenand_read_oob: from = 0x%08x, len = %i\n", (unsigned int) from, (int) len);

	/* Initialize return length value */
	*retlen = 0;

	if (mode == MTD_OOB_AUTO)
		oobsize = chip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = from & (mtd->oobsize - 1);

	if (unlikely(column >= oobsize)) {
		printk(KERN_ERR "onenand_read_oob: Attempted to start read outside oob\n");
		return -EINVAL;
	}

	/* Do not allow reads past end of device */
	if (unlikely(from >= mtd->size ||
		     column + len > ((mtd->size >> chip->page_shift) -
				     (from >> chip->page_shift)) * oobsize)) {
		printk(KERN_ERR "onenand_read_oob: Attempted to read beyond end of device\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_READING);

	while (read < len) {
		if (chip->options & ONENAND_CHECK_BAD) {
			if (onenand_block_checkbad(mtd, from)) {
				printk (KERN_WARNING "\nonenand_do_read_oob: skipped to read oob from a bad block at addr 0x%08x.\n", (unsigned int) from);
				from += (1 << chip->erase_shift);

				if (column != 0)
					column = from & (mtd->oobsize - 1);

				continue;
			}
		}

		/* on the TRANSFER SPARE bit */
		chip->write(ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);

		/* get start address to read data */
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_READ, from);

		thislen = oobsize - column;
		thislen = min_t(int, thislen, len);

		if (mode == MTD_OOB_AUTO)
			buf_poi = (u_int *)chip->oob_buf;
		else
			buf_poi = (u_int *)buf;

		switch (chip->options & ONENAND_READ_MASK) {
		case ONENAND_READ_BURST:
			onenand_read_burst(buf_poi, cmd_addr, mtd->writesize);
			onenand_read_burst(buf_poi, cmd_addr, mtd->oobsize);
			break;

		case ONENAND_READ_POLLING:
			/* read main data and throw into garbage box */
			for (i = 0; i < (mtd->writesize / 4); i++)
				*buf_poi = chip->read(cmd_addr);

			/* read spare data */
			for (i = 0; i < (mtd->oobsize / 4); i++)
				*buf_poi++ = chip->read(cmd_addr);

			break;
		}

		if (onenand_blkrw_complete(chip, ONENAND_CMD_PROG)) {
			printk(KERN_WARNING "\nonenand_write: Program operation failed.\n");
			return -1;
		}

		if (mode == MTD_OOB_AUTO)
			onenand_transfer_auto_oob(mtd, buf, column, thislen);

		read += thislen;

		if (read == len)
			break;

		buf += thislen;

		/* Read more? */
		if (read < len) {
			/* Page size */
			from += mtd->writesize;
			column = 0;
		}

		/* off the TRANSFER SPARE bit */
		chip->write(~ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);
	}

	/* Deselect and wake up anyone waiting on the device */
	onenand_release_device(mtd);

	*retlen = read;
	return ret;
}

/**
 * onenand_read_oob - [MTD Interface] NAND write data and/or out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operation description structure
 */
static int onenand_read_oob(struct mtd_info *mtd, loff_t from,
			    struct mtd_oob_ops *ops)
{
	return onenand_do_read_oob(mtd, from, ops->ooblen,
				   &ops->oobretlen, ops->oobbuf, ops->mode);
}

#ifdef CONFIG_MTD_ONENAND_VERIFY_WRITE
/**
 * onenand_verify_page - [GENERIC] verify the chip contents after a write
 * @param mtd		MTD device structure
 * @param buf		the databuffer to verify
 * @param addr		address to read
 * @return		0, if ok
 */
static int onenand_verify_page(struct mtd_info *mtd, const uint *buf, loff_t addr)
{
	struct onenand_chip *chip = mtd->priv;	
	void __iomem *cmd_addr;
	int i, ret = 0;
	uint *written = (uint *)malloc(mtd->writesize);

	cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_READ, addr);

	/* write all data of 1 page by 4 bytes at a time */
	for (i = 0; i < (mtd->writesize / 4); i++) {
		*written = chip->read(cmd_addr);
		written++;		
	}

	written -= (mtd->writesize / 4);

	/* Check, if data is same */
	if (memcmp(written, buf, mtd->writesize))
		ret = -EBADMSG;

	free(written);

	return ret;
}

/**
 * onenand_verify_oob - [GENERIC] verify the oob contents after a write
 * @param mtd		MTD device structure
 * @param buf		the databuffer to verify
 * @param to		offset to read from
 *
 */
static int onenand_verify_oob(struct mtd_info *mtd, const u_char *buf, loff_t to, size_t len)
{
	struct onenand_chip *chip = mtd->priv;
	char oobbuf[64];
	u_int *buf_poi, *dbuf_poi;
	int read = 0, thislen, column, oobsize, i;
	void __iomem *cmd_addr;
	mtd_oob_mode_t	mode = MTD_OOB_AUTO;

	if (mode == MTD_OOB_AUTO)
		oobsize = chip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = to & (mtd->oobsize - 1);
	dbuf_poi = (u_int *)chip->page_buf;

	while (read < len) {
		/* get start address to read data */
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_READ, to);
		
		thislen = oobsize - column;
		thislen = min_t(int, thislen, len);

		if (mode == MTD_OOB_AUTO) 
			buf_poi = (u_int *)chip->oob_buf;
		else
			buf_poi = (u_int *)buf;

		onenand_read_burst(dbuf_poi, cmd_addr, mtd->writesize);
		onenand_read_burst(buf_poi, cmd_addr, mtd->oobsize);

		if (onenand_blkrw_complete(chip, ONENAND_CMD_READ)) {
			printk(KERN_WARNING "onenand_verify_oob: Read operation failed:0x%x\n", (unsigned int)to);
#if 0
			chip->write(~ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);
			return -1;
#endif
		}
		
		if (mode == MTD_OOB_AUTO)
			onenand_transfer_auto_oob(mtd, (uint8_t *) oobbuf, column, thislen);

		read += thislen;

		if (read == len)
			break;
		
	}

	
	for (i = 0; i < len; i++)
		if (buf[i] != oobbuf[i]) 
			return -EBADMSG;

	return 0;
}


/**
 * onenand_verify_ops - [GENERIC] verify the oob contents after a write
 * @param mtd		MTD device structure
 * @param ops		oob operation description structure
 * @param to		offset to read from
 * @param len		number of bytes to read
 *
 */
static int onenand_verify_ops(struct mtd_info *mtd, struct mtd_oob_ops *ops, loff_t to, size_t len)
{
	struct onenand_chip *chip = mtd->priv;
	char oobbuf[64];
	u_int *buf_poi, *dbuf_poi;
	int read = 0, thislen, column, oobsize, i;
	void __iomem *cmd_addr;
	int ret = 0;

	if (ops->mode == MTD_OOB_AUTO)
		oobsize = chip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = to & (mtd->oobsize - 1);
	
	while (read < len) {
		/* get start address to read data */
		//cmd_addr = onenand_phys_to_virt(chip->command(mtd, ONENAND_CMD_READ, to));
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_READ, to);
		
		thislen = oobsize - column;
		thislen = min_t(int, thislen, len);

		dbuf_poi = (u_int *)chip->page_buf;
		
		if (ops->mode == MTD_OOB_AUTO) 
			buf_poi = (u_int *)chip->oob_buf;
		else
			buf_poi = (u_int *)oobbuf;

		onenand_read_burst(dbuf_poi, cmd_addr, mtd->writesize);
		onenand_read_burst(buf_poi, cmd_addr, mtd->oobsize);

		if (onenand_blkrw_complete(chip, ONENAND_CMD_READ)) {
			printk(KERN_WARNING "onenand_verify_oob: Read operation failed:0x%x\n", (unsigned int)to);
			return -EBADMSG;
		}

		if (ops->mode == MTD_OOB_AUTO)
			onenand_transfer_auto_oob(mtd, (uint8_t *) oobbuf, column, thislen);

		read += thislen;

		if (read == len)
			break;
		
	}
	
	/* Check, if data is same */
	if (memcmp(chip->page_buf, ops->datbuf, mtd->writesize)) {
		printk("Invalid data buffer : 0x%x\n", (unsigned int)to);
		ret = -EBADMSG;
	}
	
	for (i = 0; i < len; i++)
		if (ops->oobbuf[i] != oobbuf[i]) {
			printk("Invalid OOB buffer :0x%x\n", (unsigned int)to);
			ret = -EBADMSG;
		}

	return ret;
}
#else
#define onenand_verify_page(...)	(0)
#define onenand_verify_oob(...)		(0)
#define onenand_verify_ops(...)		(0)
#endif

#define NOTALIGNED(x)	((x & (chip->subpagesize - 1)) != 0)

/**
 * onenand_write - [MTD Interface] write buffer to FLASH
 * @param mtd		MTD device structure
 * @param to		offset to write to
 * @param len		number of bytes to write
 * @param retlen	pointer to variable to store the number of written bytes
 * @param buf		the data to write
 *
 */
int onenand_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct onenand_chip *chip = mtd->priv;
	int written = 0;
	int i, ret = 0;
	void __iomem *cmd_addr;
	uint *buf_poi = (uint *)buf;

	DEBUG(MTD_DEBUG_LEVEL3, "onenand_write: to = 0x%08x, len = %i\n", (unsigned int) to, (int) len);

	/* Initialize retlen, in case of early exit */
	*retlen = 0;

	/* Do not allow writes past end of device */
	if (unlikely((to + len) > mtd->size)) {
		DEBUG(MTD_DEBUG_LEVEL3, "\nonenand_write: Attempt write to past end of device\n");
		return -EINVAL;
	}

#if 0	/* disabled for jffs2 writing by jsgood */
	/* Reject writes, which are not page aligned */
        if (unlikely(NOTALIGNED(to)) || unlikely(NOTALIGNED(len))) {
                DEBUG(MTD_DEBUG_LEVEL3, "\nonenand_write: Attempt to write not page aligned data\n");
                return -EINVAL;
        }
#endif
	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_WRITING);

	/* Loop until all data write */
	while (written < len) {
		if (chip->options & ONENAND_CHECK_BAD) {
			if (onenand_block_checkbad(mtd, to)) {
				printk (KERN_WARNING "\nonenand_write: skipped to write to a bad block at addr 0x%08x.\n", (unsigned int) to);
				to += (1 << chip->erase_shift);
				continue;
			}
		}

		/* get address to write data */
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_PROG, to);

		/* write all data of 1 page by 4 bytes at a time */
		for (i = 0; i < (mtd->writesize / 4); i++) {
			chip->write(*buf_poi, cmd_addr);
			buf_poi++;
		}

		if (onenand_blkrw_complete(chip, ONENAND_CMD_PROG)) {
			printk(KERN_WARNING "\nonenand_write: Program operation failed.\n");
			return -1;
		}

		/* Only check verify write turn on */
		ret = onenand_verify_page(mtd, buf_poi - (mtd->writesize / 4), to);
		if (ret) {
			DEBUG(MTD_DEBUG_LEVEL3, "\nonenand_write: verify failed.\n");
			break;
		}

		written += mtd->writesize;

		if (written == len)
			break;

		to += mtd->writesize;
	}

	/* Deselect and wake up anyone waiting on the device */
	onenand_release_device(mtd);

	*retlen = written;

	return ret;
}

/**
 * onenand_fill_auto_oob - [Internal] oob auto-placement transfer
 * @param mtd		MTD device structure
 * @param oob_buf	oob buffer
 * @param buf		source address
 * @param column	oob offset to write to
 * @param thislen	oob length to write
 */
static int onenand_fill_auto_oob(struct mtd_info *mtd, u_char *oob_buf,
				  const u_char *buf, int column, int thislen)
{
	struct onenand_chip *chip = mtd->priv;
	struct nand_oobfree *free;
	int writecol = column;
	int writeend = column + thislen;
	int lastgap = 0;

	for (free = chip->ecclayout->oobfree; free->length; ++free) {
		if (writecol >= lastgap)
			writecol += free->offset - lastgap;
		if (writeend >= lastgap)
			writeend += free->offset - lastgap;
		lastgap = free->offset + free->length;
	}
	for (free = chip->ecclayout->oobfree; free->length; ++free) {
		int free_end = free->offset + free->length;
		if (free->offset < writeend && free_end > writecol) {
			int st = max_t(int,free->offset,writecol);
			int ed = min_t(int,free_end,writeend);
			int n = ed - st;
			memcpy(oob_buf + st, buf, n);
			buf += n;
		} else
			break;
	}
	return 0;
}

/**
 * onenand_write_ops - [OneNAND Interface] write main and/or out-of-band
 * @param mtd		MTD device structure
 * @param to		offset to write to
 * @param ops		oob operation description structure
 *
 * Write main and oob with ECC
 */
static int onenand_write_ops(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	struct onenand_chip *chip = mtd->priv;
	int i, column, ret = 0, oobsize;
	int written = 0;

	int len = ops->ooblen;
	u_char *buf = ops->datbuf;
	u_char *sparebuf = ops->oobbuf;
	u_char *oobbuf;
	
	void __iomem *cmd_addr;
	u_int *buf_poi;

	DEBUG(MTD_DEBUG_LEVEL3, "onenand_write_ops: to = 0x%08x, len = %i\n", (unsigned int) to, (int) len);
	
	/* Initialize retlen, in case of early exit */
	ops->retlen = 0;
	ops->oobretlen = 0;

	if (ops->mode == MTD_OOB_AUTO)
		oobsize = chip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = to & (mtd->oobsize - 1);

	if (unlikely(column >= oobsize)) {
		printk(KERN_ERR "onenand_write_ops: Attempted to start write outside oob\n");
		return -EINVAL;
	}

	/* For compatibility with NAND: Do not allow write past end of page */
	if (unlikely(column + len > oobsize)) {
		printk(KERN_ERR "onenand_write_ops: "
		      "Attempt to write past end of page\n");
		return -EINVAL;
	}

	/* Do not allow reads past end of device */
	if (unlikely(to >= mtd->size ||
		     column + len > ((mtd->size >> chip->page_shift) -
				     (to >> chip->page_shift)) * oobsize)) {
		printk(KERN_ERR "onenand_write_ops: Attempted to write past end of device\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_WRITING);

	oobbuf = chip->oob_buf;
	buf_poi = (u_int *)buf;

	/* on the TRANSFER SPARE bit */
	chip->write(ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);
	
	/* Loop until all data write */
	while (written < len) {
		int thislen = min_t(int, oobsize, len - written);

		if (chip->options & ONENAND_CHECK_BAD) {
			if (onenand_block_isbad(mtd, to)) {
				printk (KERN_WARNING "onenand_write_ops: skipped to write oob to a bad block at addr 0x%08x.\n", (unsigned int) to);
				to += (1 << chip->erase_shift);

				if (column != 0)
					column = to & (mtd->oobsize - 1);

				continue;
			}
		}

		/* get start address to write data */
		//cmd_addr = onenand_phys_to_virt(chip->command(mtd, ONENAND_CMD_PROG, to));
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_PROG, to);
		
		/* write all data of 1 page by 4 bytes at a time */
		for (i = 0; i < (mtd->writesize / 4); i++) {
			chip->write(*buf_poi, cmd_addr);
			buf_poi++;
		}

		/* We send data to spare ram with oobsize
		 * to prevent byte access */
		memset(oobbuf, 0xff, mtd->oobsize);

		if (ops->mode == MTD_OOB_AUTO)
			onenand_fill_auto_oob(mtd, oobbuf, sparebuf, column, thislen);
		else
			memcpy(oobbuf + column, buf, thislen);

		buf_poi = (u_int *)chip->oob_buf;
		for (i = 0; i < (mtd->oobsize / 4); i++) {
			chip->write(*buf_poi, cmd_addr);
			buf_poi++;
		}

		if (onenand_blkrw_complete(chip, ONENAND_CMD_PROG)) {
			printk(KERN_WARNING "onenand_write_ops: Program operation failed.\n");
			chip->write(~ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);
			return -1;
		}


#ifdef CONFIG_MTD_ONENAND_VERIFY_WRITE
		ret = onenand_verify_ops(mtd, ops, to, len);

		if (ret) {
			printk(KERN_ERR "onenand_write_ops: verify failed :0x%x\n", (unsigned int)to);
			break;
		}
#endif
		written += thislen;

		if (written == len)
			break;

		to += mtd->writesize;
		buf += thislen;
		column = 0;
	}

	/* off the TRANSFER SPARE bit */
	chip->write(~ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);

	/* Deselect and wake up anyone waiting on the device */
	onenand_release_device(mtd);

	ops->retlen = mtd->writesize;
	ops->oobretlen = written;

	return ret;
}

/**
 * onenand_do_write_oob - [Internal] OneNAND write out-of-band
 * @param mtd		MTD device structure
 * @param to		offset to write to
 * @param len		number of bytes to write
 * @param retlen	pointer to variable to store the number of written bytes
 * @param buf		the data to write
 * @param mode		operation mode
 *
 * OneNAND write out-of-band
 */
static int onenand_do_write_oob(struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf, mtd_oob_mode_t mode)
{
	struct onenand_chip *chip = mtd->priv;
	int i, column, ret = 0, oobsize;
	int written = 0;
	u_char *oobbuf, *orgbuf;
	void __iomem *cmd_addr;
	u_int *buf_poi;

	DEBUG(MTD_DEBUG_LEVEL3, "onenand_do_write_oob: to = 0x%08x, len = %i\n", (unsigned int) to, (int) len);

	/* Initialize retlen, in case of early exit */
	*retlen = 0;

	if (mode == MTD_OOB_AUTO)
		oobsize = chip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = to & (mtd->oobsize - 1);

	if (unlikely(column >= oobsize)) {
		printk(KERN_ERR "onenand_do_write_oob: Attempted to start write outside oob\n");
		return -EINVAL;
	}

	/* For compatibility with NAND: Do not allow write past end of page */
	if (unlikely(column + len > oobsize)) {
		printk(KERN_ERR "onenand_do_write_oob: "
		      "Attempt to write past end of page\n");
		return -EINVAL;
	}

	/* Do not allow reads past end of device */
	if (unlikely(to >= mtd->size ||
		     column + len > ((mtd->size >> chip->page_shift) -
				     (to >> chip->page_shift)) * oobsize)) {
		printk(KERN_ERR "onenand_do_write_oob: Attempted to write past end of device\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_WRITING);
	
	orgbuf = (u_char *) buf;
	oobbuf = chip->oob_buf;
	buf_poi = (u_int *) chip->oob_buf;

	/* Loop until all data write */
	while (written < len) {
		int thislen = min_t(int, oobsize, len - written);

		if (chip->options & ONENAND_CHECK_BAD) {
			if (onenand_block_checkbad(mtd, to)) {
				printk (KERN_WARNING "\nonenand_do_write_oob: skipped to write oob to a bad block at addr 0x%08x.\n", (unsigned int) to);
				to += (1 << chip->erase_shift);

				if (column != 0)
					column = to & (mtd->oobsize - 1);

				continue;
			}
		}

		/* on the TRANSFER SPARE bit */
		chip->write(ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);

		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_PROG, to);

		/* We send data to spare ram with oobsize
		 * to prevent byte access */
		memset(oobbuf, 0xff, mtd->oobsize);

		if (mode == MTD_OOB_AUTO)
			onenand_fill_auto_oob(mtd, oobbuf, buf, column, thislen);
		else
			memcpy(oobbuf + column, buf, thislen);

		for (i = 0; i < (mtd->writesize / 4); i++)
			chip->write(0xffffffff, cmd_addr);
		
		for (i = 0; i < (mtd->oobsize / 4); i++) {
			chip->write(*buf_poi, cmd_addr);
			buf_poi++;
		}

		if (onenand_blkrw_complete(chip, ONENAND_CMD_PROG)) {
			printk(KERN_WARNING "\nonenand_do_write_oob: Program operation failed.\n");
			return -1;
		}

		ret = onenand_verify_oob(mtd, orgbuf, to, len);
		if (ret) {
			printk(KERN_ERR "onenand_do_write_oob: verify failed %d\n", ret);
			break;
		}

		written += thislen;
		if (written == len)
			break;

		to += mtd->writesize;
		buf += thislen;
		column = 0;

		/* off the TRANSFER SPARE bit */
		chip->write(~ONENAND_TRANS_SPARE_TSRF_INC, chip->base + ONENAND_REG_TRANS_SPARE);
	}

	/* Deselect and wake up anyone waiting on the device */
	onenand_release_device(mtd);

	*retlen = written;

	return ret;
}

/**
 * onenand_write_oob - [MTD Interface] NAND write data and/or out-of-band
 * @param mtd:		MTD device structure
 * @param to:		offset to write
 * @param ops:		oob operation description structure
 */
static int onenand_write_oob(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops)
{
	int ret;
	struct onenand_chip *chip = mtd->priv;
	
	if (chip->options & ONENAND_CHECK_BAD) {
		if (onenand_block_checkbad(mtd, to)) {
			printk (KERN_WARNING "\nonenand_write_oob: failed to write oob to a bad block at addr 0x%08x.\n", (unsigned int) to);
			return -1;
		}
	}

	switch (ops->mode) {
	case MTD_OOB_PLACE:
	case MTD_OOB_AUTO:
		break;
	case MTD_OOB_RAW:
		/* Not implemented yet */
	default:
		return -EINVAL;
	}
#if 0
	/* For YAFFS2 */
	if (ops->datbuf != NULL)
		ret = onenand_write_ops(mtd, to, ops);
	else
#endif
		ret = onenand_do_write_oob(mtd, to, ops->ooblen,
				    &ops->oobretlen, ops->oobbuf, ops->mode);
	return ret;
}

/**
 * onenand_erase - [MTD Interface] erase block(s)
 * @param mtd		MTD device structure
 * @param instr		erase instruction
 *
 * Erase one ore more blocks
 */
static int onenand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct onenand_chip *chip = mtd->priv;
	unsigned int block_size;
	loff_t addr;
	int len, ret = 0;
	void __iomem *cmd_addr = 0;

	DEBUG(MTD_DEBUG_LEVEL3, "onenand_erase: start = 0x%08x, len = %i\n", (unsigned int) instr->addr, (unsigned int) instr->len);

	block_size = (1 << chip->erase_shift);

	/* Start address must align on block boundary */
	if (unlikely(instr->addr & (block_size - 1))) {
		DEBUG(MTD_DEBUG_LEVEL3, "\nonenand_erase: Unaligned address\n");
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (unlikely(instr->len & (block_size - 1))) {
		DEBUG(MTD_DEBUG_LEVEL3, "\nonenand_erase: Length not block aligned\n");
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if (unlikely((instr->len + instr->addr) > mtd->size)) {
		DEBUG(MTD_DEBUG_LEVEL3, "\nonenand_erase: Erase past end of device\n");
		return -EINVAL;
	}

	instr->fail_addr = 0xffffffff;

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_ERASING);

	/* Loop throught the pages */
	len = instr->len;
	addr = instr->addr;

	instr->state = MTD_ERASING;

	while (len) {
		if (chip->options & ONENAND_CHECK_BAD) {
			if (onenand_block_checkbad(mtd, addr)) {
				printk (KERN_WARNING "\nonenand_erase: skipped to erase a bad block at addr 0x%08x.\n", (unsigned int) addr);
				len -= block_size;
				addr += block_size;
				continue;
			}
		}

		/* get address to erase */
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_ERASE, addr);

		chip->write(ONENAND_DATAIN_ERASE_SINGLE, cmd_addr); /* single block erase */

		/* wait irq */
		onenand_irq_wait_ack(chip, ONENAND_INT_ERR_INT_ACT);
		onenand_irq_wait_ack(chip, ONENAND_INT_ERR_ERS_CMP);

		chip->write(ONENAND_DATAIN_ERASE_VERIFY, cmd_addr);

		/* wait irq */
		onenand_irq_wait_ack(chip, ONENAND_INT_ERR_INT_ACT);
		onenand_irq_wait_ack(chip, ONENAND_INT_ERR_ERS_CMP);

		/* check fail */
		if (onenand_irq_pend(chip, ONENAND_INT_ERR_ERS_FAIL)) {
			DEBUG(MTD_DEBUG_LEVEL3, "\nonenand_erase: block %d erase verify failed.\n", addr >> chip->erase_shift);
			onenand_irq_ack(chip, ONENAND_INT_ERR_ERS_FAIL);

			/* check lock */
			if (onenand_irq_pend(chip, ONENAND_INT_ERR_LOCKED_BLK)) {
				DEBUG(MTD_DEBUG_LEVEL3, "\nonenand_erase: block %d is locked.\n", addr >> chip->erase_shift);
				onenand_irq_ack(chip, ONENAND_INT_ERR_LOCKED_BLK);
			}
		}

		len -= block_size;
		addr += block_size;
	}

	instr->state = MTD_ERASE_DONE;

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;
	/* Do call back function */
	if (!ret)
		mtd_erase_callback(instr);

	/* Deselect and wake up anyone waiting on the device */
	onenand_release_device(mtd);

	return ret;
}

/**
 * onenand_sync - [MTD Interface] sync
 * @param mtd		MTD device structure
 *
 * Sync is actually a wait for chip ready function
 */
static void onenand_sync(struct mtd_info *mtd)
{
	DEBUG(MTD_DEBUG_LEVEL3, "onenand_sync: called\n");

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_SYNCING);

	/* Release it and go back */
	onenand_release_device(mtd);
}

/**
 * onenand_do_lock_cmd - [OneNAND Interface] Lock or unlock block(s)
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 * @param len		number of bytes to lock or unlock
 *
 * Lock or unlock one or more blocks
 */
static int onenand_do_lock_cmd(struct mtd_info *mtd, loff_t ofs, size_t len, int cmd)
{
	struct onenand_chip *chip = mtd->priv;
	int start, end, ofs_end, block_size;
	int datain1, datain2;
	void __iomem *cmd_addr;	

	start = ofs >> chip->erase_shift;
	end = len >> chip->erase_shift;
	block_size = 1 << chip->erase_shift;
	ofs_end = ofs + len - block_size;

	if (cmd == ONENAND_CMD_LOCK) {
		datain1 = ONENAND_DATAIN_LOCK_START;
		datain2 = ONENAND_DATAIN_LOCK_END;
	} else {
		datain1 = ONENAND_DATAIN_UNLOCK_START;
		datain2 = ONENAND_DATAIN_UNLOCK_END;
	}

	if (ofs < ofs_end) {
		cmd_addr = (void __iomem *)chip->command(mtd, cmd, ofs);
		chip->write(datain1, cmd_addr);
	}
	
	cmd_addr = (void __iomem *)chip->command(mtd, cmd, ofs_end);
	chip->write(datain2, cmd_addr);

	if (cmd == ONENAND_CMD_LOCK) {
		if (!chip->read(chip->base + ONENAND_REG_INT_ERR_STAT) & ONENAND_INT_ERR_LOCKED_BLK) {
			DEBUG(MTD_DEBUG_LEVEL3, "onenand_do_lock_cmd: lock failed.\n");
			return -1;
		}
	} else {
		if (chip->read(chip->base + ONENAND_REG_INT_ERR_STAT) & ONENAND_INT_ERR_LOCKED_BLK) {
			DEBUG(MTD_DEBUG_LEVEL3, "onenand_do_lock_cmd: unlock failed.\n");
			return -1;
		}
	}

	return 0;
}


/**
 * onenand_lock - [MTD Interface] Lock block(s)
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 * @param len		number of bytes to lock
 *
 * Lock one or more blocks
 */
int onenand_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	return onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_LOCK);
}


/**
 * onenand_unlock - [MTD Interface] Unlock block(s)
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 * @param len		number of bytes to unlock
 *
 * Unlock one or more blocks
 */
int onenand_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	return onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_UNLOCK);
}

/**
 * onenand_check_lock_status - [OneNAND Interface] Check lock status
 * @param chip		onenand chip data structure
 *
 * Check lock status
 */
static void onenand_check_lock_status(struct mtd_info *mtd)
{
	struct onenand_chip *chip = mtd->priv;
	unsigned int block, end;
	void __iomem *cmd_addr;
	int tmp;

	end = chip->chipsize >> chip->erase_shift;	
	
	for (block = 0; block < end; block++) {
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_READ, block << chip->erase_shift);
		tmp = chip->read(cmd_addr);

		if (chip->read(chip->base + ONENAND_REG_INT_ERR_STAT) & ONENAND_INT_ERR_LOCKED_BLK) {
			printk(KERN_ERR "block %d is write-protected!\n", block);
			chip->write(ONENAND_INT_ERR_LOCKED_BLK, chip->base + ONENAND_REG_INT_ERR_ACK);
		}		
	}
}

/**
 * onenand_unlock_all - [OneNAND Interface] unlock all blocks
 * @param mtd		MTD device structure
 *
 * Unlock all blocks
 */
static int onenand_unlock_all(struct mtd_info *mtd)
{
	struct onenand_chip *chip = mtd->priv;
	void __iomem *cmd_addr;

	if (chip->options & ONENAND_HAS_UNLOCK_ALL) {
		/* write unlock command */
		cmd_addr = (void __iomem *)chip->command(mtd, ONENAND_CMD_UNLOCK_ALL, 0);
		chip->write(ONENAND_DATAIN_UNLOCK_ALL, cmd_addr);

		/* Workaround for all block unlock in DDP */
		if (chip->device_id & ONENAND_DEVICE_IS_DDP) {
			loff_t ofs;
			size_t len;

			/* 1st block on another chip */
			ofs = chip->chipsize >> 1;
			len = 1 << chip->erase_shift;

			onenand_unlock(mtd, ofs, len);
		}

		onenand_check_lock_status(mtd);

		return 0;
	}

	onenand_unlock(mtd, 0x0, chip->chipsize);

	return 0;
}

/**
 * onenand_lock_scheme - Check and set OneNAND lock scheme
 * @param mtd		MTD data structure
 *
 * Check and set OneNAND lock scheme
 */
static void onenand_lock_scheme(struct mtd_info *mtd)
{
	struct onenand_chip *chip = mtd->priv;
	unsigned int density, process;

	/* Lock scheme depends on density and process */
	density = chip->device_id >> ONENAND_DEVICE_DENSITY_SHIFT;
	process = chip->version_id >> ONENAND_VERSION_PROCESS_SHIFT;

	/* Lock scheme */
	if (density >= ONENAND_DEVICE_DENSITY_1Gb) {
		/* A-Die has all block unlock */
		if (process) {
			DEBUG(MTD_DEBUG_LEVEL3 "Chip support all block unlock\n");
			chip->options |= ONENAND_HAS_UNLOCK_ALL;
		}
	} else {
		/* Some OneNAND has continues lock scheme */
		if (!process) {
			DEBUG(MTD_DEBUG_LEVEL3 "Lock scheme is Continues Lock\n");
			chip->options |= ONENAND_HAS_CONT_LOCK;
		}
	}
}

/**
 * onenand_print_device_info - Print device ID
 * @param device        device ID
 *
 * Print device ID
 */
void onenand_print_device_info(int device, int version)
{
        int vcc, demuxed, ddp, density;

        vcc = device & ONENAND_DEVICE_VCC_MASK;
        demuxed = device & ONENAND_DEVICE_IS_DEMUX;
        ddp = device & ONENAND_DEVICE_IS_DDP;
        density = device >> ONENAND_DEVICE_DENSITY_SHIFT;
        printk(KERN_INFO "%sOneNAND%s %dMB %sV 16-bit (0x%02x)\n",
                demuxed ? "" : "Muxed ",
                ddp ? "(DDP)" : "",
                (16 << density),
                vcc ? "2.65/3.3" : "1.8",
                device);
	printk(KERN_DEBUG "OneNAND version = 0x%04x\n", version);
}

static const struct onenand_manufacturers onenand_manuf_ids[] = {
        {ONENAND_MFR_SAMSUNG, "Samsung"},
};

/**
 * onenand_check_maf - Check manufacturer ID
 * @param manuf         manufacturer ID
 *
 * Check manufacturer ID
 */
static int onenand_check_maf(int manuf)
{
	int size = ARRAY_SIZE(onenand_manuf_ids);
	char *name;
        int i;

	for (i = 0; i < size; i++)
                if (manuf == onenand_manuf_ids[i].id)
                        break;

	if (i < size)
		name = onenand_manuf_ids[i].name;
	else
		name = "Unknown";

#if 0
	printk(KERN_DEBUG "OneNAND Manufacturer: %s (0x%0x)\n", name, manuf);
#endif

	return (i == size);
}

/**
 * onenand_probe - [OneNAND Interface] Probe the OneNAND device
 * @param mtd		MTD device structure
 *
 * OneNAND detection method:
 *   Compare the the values from command with ones from register
 */
static int onenand_probe(struct mtd_info *mtd)
{
	struct onenand_chip *chip = mtd->priv;
	int maf_id, dev_id, ver_id, density;

	/* Read manufacturer and device IDs from Register */
	maf_id = chip->read(chip->base + ONENAND_REG_MANUFACT_ID);
	dev_id = chip->read(chip->base + ONENAND_REG_DEVICE_ID);
	ver_id = chip->read(chip->base + ONENAND_REG_FLASH_VER_ID);

	/* Check manufacturer ID */
	if (onenand_check_maf(maf_id))
		return -ENXIO;

	chip->device_id = dev_id;
	chip->version_id = ver_id;

	density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;
	chip->chipsize = (16 << density) << 20;
	/* Set density mask. it is used for DDP */
	chip->density_mask = (1 << (density + 6));

	/* OneNAND page size & block size */
	/* The data buffer size is equal to page size */
	mtd->writesize = chip->read(chip->base + ONENAND_REG_DATA_BUF_SIZE);
	mtd->oobsize = mtd->writesize >> 5;
	/* Pagers per block is always 64 in OneNAND */
	mtd->erasesize = mtd->writesize << 6;

	chip->erase_shift = ffs(mtd->erasesize) - 1;
	chip->page_shift = ffs(mtd->writesize) - 1;
	chip->ppb_shift = (chip->erase_shift - chip->page_shift);
	chip->page_mask = (mtd->erasesize / mtd->writesize) - 1;

	/* REVIST: Multichip handling */

	mtd->size = chip->chipsize;

	/* Check OneNAND lock scheme */
	onenand_lock_scheme(mtd);

	return 0;
}

/**
 * onenand_scan - [OneNAND Interface] Scan for the OneNAND device
 * @param mtd		MTD device structure
 * @param maxchips	Number of chips to scan for
 *
 * This fills out all the not initialized function pointers
 * with the defaults.
 * The flash ID is read and the mtd/chip structures are
 * filled with the appropriate values.
 */
int onenand_scan(struct mtd_info *mtd, int maxchips)
{
	int i;
	struct onenand_chip *chip = mtd->priv;

	if (!chip->read)
		chip->read = onenand_readl;
	if (!chip->write)
		chip->write = onenand_writel;

	if (!chip->command)
		chip->command = onenand_command;

	if (onenand_probe(mtd))
		return -ENXIO;

	/* Allocate buffers, if necessary */
	if (!chip->page_buf) {
		size_t len;
		len = mtd->writesize + mtd->oobsize;
		chip->page_buf = kmalloc(len, GFP_KERNEL);
		if (!chip->page_buf) {
			printk(KERN_ERR "onenand_scan(): Can't allocate page_buf\n");
			return -ENOMEM;
		}
		chip->options |= ONENAND_PAGEBUF_ALLOC;
	}

	if (!chip->oob_buf) {
		chip->oob_buf = kmalloc(mtd->oobsize, GFP_KERNEL);
		if (!chip->oob_buf) {
			printk(KERN_ERR "onenand_scan(): Can't allocate oob_buf\n");
			return -ENOMEM;
		}
		chip->options |= ONENAND_OOBBUF_ALLOC;
	}

	/*
	 * Allow subpage writes up to oobsize.
	 */
	switch (mtd->oobsize) {
	case 64:
		chip->ecclayout = &onenand_oob_64;
		mtd->subpage_sft = 2;
		break;

	case 32:
		chip->ecclayout = &onenand_oob_32;
		mtd->subpage_sft = 1;
		break;

	default:
		printk(KERN_WARNING "No OOB scheme defined for oobsize %d\n",
			mtd->oobsize);
		mtd->subpage_sft = 0;
		/* To prevent kernel oops */
		chip->ecclayout = &onenand_oob_32;
		break;
	}

	chip->subpagesize = mtd->writesize >> mtd->subpage_sft;

	/*
	 * The number of bytes available for a client to place data into
	 * the out of band area
	 */
	chip->ecclayout->oobavail = 0;
	for (i = 0; chip->ecclayout->oobfree[i].length; i++)
		chip->ecclayout->oobavail +=
			chip->ecclayout->oobfree[i].length;

	mtd->oobavail = chip->ecclayout->oobavail;	
	mtd->ecclayout = chip->ecclayout;

	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->ecctype = MTD_ECC_SW;
	mtd->erase = onenand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = onenand_read;
	mtd->write = onenand_write;
	mtd->read_oob = onenand_read_oob;
	mtd->write_oob = onenand_write_oob;
	mtd->sync = onenand_sync;

	if (chip->options & ONENAND_CHECK_BAD)
		mtd->block_isbad = onenand_block_isbad;

	/* Unlock whole block */
	onenand_unlock_all(mtd);

	return chip->scan_bbt(mtd);
}

void onenand_release(struct mtd_info *mtd)
{
	struct onenand_chip *chip = mtd->priv;

	/* Buffers allocated by onenand_scan */
	if (chip->options & ONENAND_PAGEBUF_ALLOC)
		kfree(chip->page_buf);
	if (chip->options & ONENAND_OOBBUF_ALLOC)
		kfree(chip->oob_buf);
}

