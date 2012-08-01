/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 */

#include <common.h>

#if defined(CONFIG_S3C64XX)

/*
 *
 * New NAND support
 *
 */
#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
# define SHOW_BOOT_PROGRESS(arg)	show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

#include <jffs2/jffs2.h>

#if defined(CONFIG_S3C6400) || defined(CONFIG_S3C6410) || defined(CONFIG_S3C6430)
#include <s3c_onenand.h>
#else
#include <onenand.h>
#endif

#define cpu_to_je16(x) (x)
#define cpu_to_je32(x) (x)

extern onenand_info_t onenand_info[];       /* info for OneNAND chips */
extern void onenand_print_device_info(int device, int version);

struct jffs2_clean_oob {
	struct jffs2_unknown_node cleanmarker;
	uchar empty[52];
};

static int onenand_dump_oob(onenand_info_t *nand, ulong off)
{
	int i;
	struct mtd_oob_ops ops;
	u_int8_t *p, *buf;

	buf = malloc(nand->oobsize);
	memset(buf, 0xff, nand->oobsize);

//	ops.ooboffs = nand->writesize;
	ops.mode = MTD_OOB_PLACE;
	ops.oobbuf = buf;

	if (ops.mode == MTD_OOB_AUTO)
		ops.ooblen = nand->oobavail;
	else
		ops.ooblen = nand->oobsize;

	nand->read_oob(nand, off, &ops);

	puts("OOB:\n");

	p = ops.oobbuf;

	i = nand->oobsize >> 3;
	
	while (i--) {
		printf( "\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}

	free(buf);
	
	return 0;
}

static int onenand_write_oob(onenand_info_t *nand, ulong off, uchar *buf, int mode)
{
	struct mtd_oob_ops ops;

	ops.ooboffs = nand->writesize;
	ops.mode = mode;
	ops.oobbuf = buf;

	if (ops.mode == MTD_OOB_AUTO)
		ops.ooblen = nand->oobavail;
	else
		ops.ooblen = nand->oobsize;

	nand->write_oob(nand, off, &ops);

	return 0;
}

static int onenand_write_cleanmarker(onenand_info_t *nand, ulong off, ulong size)
{
	struct jffs2_clean_oob oob;
	int block_size;

	memset(&oob.cleanmarker, 0, sizeof(oob.cleanmarker));
	memset(oob.empty, 0xff, sizeof(oob.empty));

	oob.cleanmarker.magic = cpu_to_je16 (JFFS2_MAGIC_BITMASK);
	oob.cleanmarker.nodetype = cpu_to_je16 (JFFS2_NODETYPE_CLEANMARKER);
	oob.cleanmarker.totlen = cpu_to_je32(8);

	block_size = nand->erasesize;

	while (size) {
		onenand_write_oob(nand, off, (uchar *)&oob, MTD_OOB_AUTO);
		off += block_size;
		size -= block_size;
	}

	return 0;
}

static int onenand_dump(onenand_info_t *nand, ulong off)
{
	int i;
	u_int8_t *p, *buf;
	ulong len = nand->writesize;

	buf = malloc(len);
	memset(buf, 0xff, len);

	onenand_read(nand, off, &len, buf);

	i = nand->writesize >> 4;
	p = buf;

	printf("Page %08x dump:\n", off);

	while (i--) {
		printf( "\t%02x %02x %02x %02x %02x %02x %02x %02x"
			"  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		p += 16;
	}

	onenand_dump_oob(nand, off);

	free(buf);

	return 0;
}

/* ------------------------------------------------------------------------- */

static void
arg_off_size(int argc, char *argv[], ulong *off, ulong *size, ulong totsize)
{
	*off = 0;
	*size = 0;

#if defined(CONFIG_JFFS2_NAND) && defined(CFG_JFFS_CUSTOM_PART)
	if (argc >= 1 && strcmp(argv[0], "partition") == 0) {
		int part_num;
		struct part_info *part;
		const char *partstr;

		if (argc >= 2)
			partstr = argv[1];
		else
			partstr = getenv("partition");

		if (partstr)
			part_num = (int)simple_strtoul(partstr, NULL, 10);
		else
			part_num = 0;

		part = jffs2_part_info(part_num);
		if (part == NULL) {
			printf("\nInvalid partition %d\n", part_num);
			return;
		}
		*size = part->size;
		*off = (ulong)part->offset;
	} else
#endif
	{
		if (argc >= 1)
			*off = (ulong)simple_strtoul(argv[0], NULL, 16);
		else
			*off = 0;

		if (argc >= 2)
			*size = (ulong)simple_strtoul(argv[1], NULL, 16);
		else
			*size = totsize - *off;

	}

}

int do_onenand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i, dev, ret = 1;
	ulong addr, off, size;
	char *cmd, *s;
	onenand_info_t *nand;

	/* at least two arguments please */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (strcmp(cmd, "info") == 0) {

		putc('\n');
		for (i = 0; i < CFG_MAX_ONENAND_DEVICE; i++) {
			struct onenand_chip *this = (onenand_info[i].priv);
			printf("Device %d: (%04x) ", i, this->device_id);
			/* org: onenand_print_device_info(this->device_id); */
			onenand_print_device_info(this->device_id, 0);
		}
		return 0;
	}

	if (strcmp(cmd, "device") == 0) {
		if (argc < 3) {
			if ((onenand_curr_device < 0) ||
					(onenand_curr_device >= CFG_MAX_ONENAND_DEVICE))
				puts("\nno devices available\n");
			else {
				struct onenand_chip *this = (onenand_info[onenand_curr_device].priv);
				printf("Device %d: (%04x) ", onenand_curr_device, this->device_id);
				/* org: onenand_print_device_info(this->device_id); */
				onenand_print_device_info(this->device_id, 0);
			}
			return 0;
		}
		dev = (int)simple_strtoul(argv[2], NULL, 10);
		if (dev < 0 || dev >= CFG_MAX_ONENAND_DEVICE || !onenand_info[dev].name) {
			puts("No such device\n");
			return 1;
		}
		printf("Device %d: %s", dev, onenand_info[dev].name);
		puts("... is now current device\n");
		onenand_curr_device = dev;
		return 0;
	}

	if (strcmp(cmd, "bad") != 0 && strcmp(cmd, "erase") != 0 &&
	    strncmp(cmd, "dump", 4) != 0 && strncmp(cmd, "writeoob", 8) != 0 &&
	    strncmp(cmd, "read", 4) != 0 && strncmp(cmd, "write", 5) != 0 &&
	    strncmp(cmd, "lock", 4) != 0)
		goto usage;

	/* the following commands operate on the current device */
	if (onenand_curr_device < 0 ||
			onenand_curr_device >= CFG_MAX_ONENAND_DEVICE ||
			!onenand_info[onenand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &onenand_info[onenand_curr_device];

	if (strcmp(cmd, "bad") == 0) {
		printf("\nDevice %d bad blocks:\n", onenand_curr_device);
		for (off = 0; off < nand->size; off += nand->erasesize)
			if (onenand_block_isbad(nand, off))
				printf("  %08x\n", off);
		return 0;
	}

	if (strcmp(cmd, "erase") == 0) {
		int clean = argc > 2 && !strcmp("clean", argv[2]);

		if (clean)
			arg_off_size(argc - 3, argv + 3, &off, &size, nand->size);
		else		
			arg_off_size(argc - 2, argv + 2, &off, &size, nand->size);
		
		if (off == 0 && size == 0)
			return 1;		

		printf("\nOneNAND erase: device %d offset 0x%x, size 0x%x ",
		       onenand_curr_device, off, size);

		ret = onenand_erase(nand, off, size);

		printf("%s\n", ret ? "ERROR" : "OK");

		if (clean) {
			printf("OneNAND erase: device %d offset 0x%x, size 0x%x, writing jffs2 cleanmarker ", onenand_curr_device, off, size);
			ret = onenand_write_cleanmarker(nand, off, size);
			printf("%s\n", ret ? "ERROR" : "OK");
		}

		return ret == 0 ? 0 : 1;
	}

	if (strncmp(cmd, "dump", 4) == 0) {
		if (argc < 3)
			goto usage;

		s = strchr(cmd, '.');
		off = (int)simple_strtoul(argv[2], NULL, 16);

		if (s != NULL && strcmp(s, ".oob") == 0)
			ret = onenand_dump_oob(nand, off);
		else
			ret = onenand_dump(nand, off);

		return ret == 0 ? 1 : 0;

	}

	if (strncmp(cmd, "writeoob", 8) == 0) {
		if (argc < 4)
			goto usage;

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);
		off = (int)simple_strtoul(argv[3], NULL, 16);
		ret = onenand_write_oob(nand, off, (u_char *)addr, MTD_OOB_PLACE);

		return ret == 0 ? 1 : 0;
	}

	/* read write */
	if (strncmp(cmd, "read", 4) == 0 || strncmp(cmd, "write", 5) == 0) {
		u8 yaffs_write = 0;
		if (argc < 4)
			goto usage;
		s = strchr(cmd, '.');
#if 0
		clean = CLEAN_NONE;
		if (s != NULL) {
			if (strcmp(s, ".jffs2") == 0 || strcmp(s, ".e") == 0
			    || strcmp(s, ".i"))
				clean = CLEAN_JFFS2;
		}
#endif
		if (s != NULL) {
			if (strcmp(s, ".yaffs2") == 0)
				yaffs_write = 1;
		}
		addr = (ulong)simple_strtoul(argv[2], NULL, 16);

		arg_off_size(argc - 3, argv + 3, &off, &size, nand->size);
		if (off == 0 && size == 0)
			return 1;

		i = strncmp(cmd, "read", 4) == 0;	/* 1 = read, 0 = write */
		printf("\nOneNAND %s: device %d offset %x, size %x  addr %x... ",
		       i ? "read" : "write", onenand_curr_device, off, size, addr);

		if (i)
			ret = onenand_read(nand, off, &size, (u_char *)addr);
		else {
			if(!yaffs_write) {
				ret = onenand_write(nand, off, &size, (u_char *)addr);
			}else {
				u32 written = 0;
				ulong write_size;

				printf("\nyaffs_write :");
				if(off % nand->erasesize !=0) {
					printf(" ERROR starting address %x is not a block start address\n",off);
					return 1;
				}
				while (written < size) {
					if(off % nand->erasesize == 0) {
						// Start of a new block, 
						if(off >= nand->size) {
							printf(" ERROR reached end of flash, no more free space.\n ");
							return 1;
						}
						//Check whether the block is bad. if not erase it.
						if(onenand_block_isbad(nand,off)){
							printf("yaffs_write Skipping bad block %d at %x\n",off/nand->erasesize,off);
							off += nand->erasesize;
							continue;
						}else {
							ret = onenand_erase(nand,off,nand->erasesize);
							if(ret) {
								printf("yaffswrite: erasing block %d @ %x failed, skipping\n",off/nand->erasesize,off);
	
								off += nand->erasesize;
								continue;
							}

						}
						printf(" %4d",off/nand->erasesize);
					}
					write_size = nand->writesize;
					/* org: ret = onenand_write_ecc(nand, off, &write_size, (u_char *)addr,(u_char*)(addr+nand->writesize),NULL); */
					ret = onenand_write(nand, off, &write_size, (u_char *)addr);
					if(ret) 
						return 1;
					
					addr += nand->writesize;
					written += nand->writesize;

					ret = onenand_write_oob(nand, off, (u_char *)addr, MTD_OOB_AUTO);
					if(ret) 
						return 1;
					
					addr += nand->oobsize;
					written += nand->oobsize;
					
					off += nand->writesize;
				}
			}
		}
		printf(" %d bytes %s: %s\n", size,
		       i ? "read" : "written", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
	}

	if (strncmp(cmd, "lock", 4) == 0) {
		int lock = 0;
		if (argc != 5)
			goto usage;

		if (strcmp(argv[2], "off") == 0)
			lock = 0;
		else if (strcmp(argv[2], "on") == 0)
			lock = 1;
		else
			goto usage;

		arg_off_size(argc - 3, argv + 3, &off, &size, nand->size);
		if (off == 0 && size == 0)
			return 1;
		printf("\nOneNAND %slock: device %d offset 0x%x, size 0x%x ",
		       lock ? "":"un", onenand_curr_device, off, size);

		if (lock) {
			ret = onenand_lock(nand, off, size);
		}
		else {
			ret = onenand_unlock(nand, off, size);
		}
		printf(": %s\n", ret ? "ERROR":"OK");
		return ret == 0 ? 0 : 1;
	}

usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(onenand, 5, 0, do_onenand,
	"onenand - OneNAND sub-system\n",
	"info                  - show available OneNAND devices\n"
	"onenand device [dev]     - show or set current device\n"
	"onenand read[.jffs2]     - addr off size\n"
	"onenand write[.jffs2/.yaffs2]    - addr off size - read/write `size' bytes starting\n"
	"    at offset `off' to/from memory address `addr'\n"
	"onenand erase [clean] [off size] - erase `size' bytes from\n"
	"    offset `off' (entire device if not specified)\n"
	"onenand bad - show bad blocks\n"
	"onenand dump[.oob] off - dump page\n"
	"onenand scrub - really clean NAND erasing bad blocks (UNSAFE)\n"
	"onenand markbad off - mark bad block at offset (UNSAFE)\n"
	"onenand biterr off - make a bit error at offset (UNSAFE)\n"
	"onenand lock <on/off> <offset> <size> - lock/unlock OneNAND Block\n" );

int do_onenandboot(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *boot_device = NULL;
	char *ep;
	int dev;
	int r;
	ulong addr, cnt, offset = 0;
	image_header_t *hdr;
	onenand_info_t *nand;

	switch (argc) {
	case 1:
		addr = CFG_LOAD_ADDR;
		boot_device = getenv("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	case 4:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		offset = simple_strtoul(argv[3], NULL, 16);
		break;
	default:
		printf("Usage:\n%s\n", cmdtp->usage);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	if (!boot_device) {
		puts("\n** No boot device **\n");
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	dev = simple_strtoul(boot_device, &ep, 16);

	if (dev < 0 || dev >= CFG_MAX_ONENAND_DEVICE || !onenand_info[dev].name) {
		printf("\n** Device %d not available\n", dev);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	nand = &onenand_info[dev];
	printf("\nLoading from device %d: %s (offset 0x%lx)\n",
	       dev, nand->name, offset);

	cnt = nand->writesize;
	r = onenand_read(nand, offset, &cnt, (u_char *) addr);
	if (r) {
		printf("** Read error on %d\n", dev);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	hdr = (image_header_t *) addr;

	if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		printf("\n** Bad Magic Number 0x%x **\n", hdr->ih_magic);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	print_image_hdr(hdr);

	cnt = (ntohl(hdr->ih_size) + sizeof (image_header_t));

	r = onenand_read(nand, offset, &cnt, (u_char *) addr);
	if (r) {
		printf("** Read error on %d\n", dev);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	/* Loading ok, update default load address */

	load_addr = addr;

	/* Check if we should attempt an auto-start */
	if (((ep = getenv("autostart")) != NULL) && (strcmp(ep, "yes") == 0)) {
		char *local_args[2];
		extern int do_bootm(cmd_tbl_t *, int, int, char *[]);

		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf("Automatic boot of image at addr 0x%08lx ...\n", addr);

		do_bootm(cmdtp, 0, 1, local_args);
		return 1;
	}
	return 0;
}

U_BOOT_CMD(oneboot, 4, 1, do_onenandboot,
	"oneboot - boot from NAND device\n", "loadAddr dev\n");

#ifndef CONFIG_REMOVE_OLD_ONENAND_CMD

int cpage_size = 2048;
int cpages_in_block = 64;
int coob_size = 64;
int cblock_size = 0x20000;
int ctotal_blocks = 2048;

int do_onenand_erase (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char temp[100];
	int startblk, size;

	if ( argc != 3 ){
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	startblk = simple_strtoul(argv[1], NULL, 16);
	size  = simple_strtoul(argv[2], NULL, 16);

	sprintf(temp, "onenand erase %x %x", startblk * cblock_size, size);
	run_command(temp, 0);
	printf("use \"%s\" instead of this command\n", temp);
	return 0;
}

int do_onenand_write (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char temp[100];
	ulong startblk, size, memadr;

	if (argc !=  4) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	startblk = simple_strtoul(argv[1], NULL, 16);
	size  = simple_strtoul(argv[2], NULL, 16);
	memadr = simple_strtoul(argv[3], NULL, 16);

	sprintf(temp, "onenand erase %x %x; onenand write %x %x %x",
		startblk * cblock_size, size,
		memadr, startblk * cblock_size, size);
	run_command(temp, 0);
	printf("\n\nuse \"%s\" instead of this command\n", temp);

	return 0;
}

int do_onenand_read (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char temp[100];
	ulong startblk, size, memadr;

	if (argc !=  4) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	startblk = simple_strtoul(argv[1], NULL, 16);
	size  = simple_strtoul(argv[2], NULL, 16);
	memadr = simple_strtoul(argv[3], NULL, 16);

	sprintf(temp, "onenand read %x %x %x", memadr, startblk*cblock_size, size);
	run_command(temp, 0);
	printf("use \"%s\" instead of this command\n", temp);

	return 0;
}

int do_onenand_bbcheck (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	run_command("onenand bad", 0);
	printf("use \"onenand bad\" instead of this command\n");

	return 0;
}

U_BOOT_CMD(onenande,	4,	0,	do_onenand_erase,
       "onenande  startBlock size \n",
       "	- delete blocks in onenand while skipping bad block \n");

U_BOOT_CMD(onenandr,	4,	0,	do_onenand_read,
	 "onenandr - HEX: targetblock targetsize mem_adr \n",
	 "\n	-SMDK24X0 ONE NAND Flash Read Program\n"
	 "onenandr targetblock, targetsize memory addr \n");

U_BOOT_CMD(onenandw,	4,	0,	do_onenand_write,
       "onenandw  HEX: targetblock targetsize mem_addr \n",
       "\n	- SMDK24X0 NAND Flash Write Program\n"
       "onenandw targetblock targetsize memory addr \n");

U_BOOT_CMD(onebbcheck,    3,  0,  do_onenand_bbcheck,
       "onebbcheck \n",
       "\n  - Shows all the bad blocks in the onenand memory");
#endif

#endif	/* end of CONFIG_S3C64XX */

