#include <common.h>

#include <command.h>
#include <movi.h>
#include <nand.h>

struct movi_offset_t ofsinfo;
extern nand_info_t nand_info[];

#if 0
int do_testhsmmc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uint start_blk, blknum;
	start_blk = simple_strtoul(argv[1], NULL, 10);
	blknum = simple_strtoul(argv[2], NULL, 10);

	if(argc < 3)
		return 0;

	printf("COMMAND --> start_blk = 0x%x, blknum = %d\n", start_blk, blknum);
	test_hsmmc(4, 1, start_blk, blknum);
	return 1;
}

U_BOOT_CMD(
	testhsmmc,	4,	0,	do_testhsmmc,
	"testhsmmc - hsmmc testing write/read\n",
	NULL
);
#endif

int do_movi(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;
	ulong addr, blk, blocks;
	uint last_blkpos, bytes;

	cmd = argv[1];

	if (strcmp(cmd, "init") == 0) {
		if (argc < 4)
			goto usage;

		last_blkpos = (uint) simple_strtoul(argv[2], NULL, 10);
		movi_hc = (uint) simple_strtoul(argv[3], NULL, 10);

		if (movi_set_ofs(last_blkpos))
			movi_init();
	} else {
		if (argc == 4 || (strcmp(argv[2], "rootfs") == 0))
			addr = (ulong) simple_strtoul(argv[3], NULL, 16);
		else if (argc == 5)
			addr = (ulong) simple_strtoul(argv[4], NULL, 16);
		else
			goto usage;

		if (addr >= 0xc0000000)
			addr = virt_to_phys(addr);

		if (ofsinfo.last == 0) {
			printf("### Execute 'movi init' command first!\n");
			return -1;
		}

		if (strcmp(cmd, "read") == 0) {
			if (argc == 4 || (strcmp(argv[2], "rootfs") == 0)) {
				if (strcmp(argv[2], "u-boot") == 0) {
					printf("Reading bootloader.. ");
					movi_read((uint) addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
					printf("completed\n");
				} else if (strcmp(argv[2], "kernel") == 0) {
					printf("Reading kernel from sector %d (%d sectors).. ", ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					movi_read((uint) addr, ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					printf("completed\n");
				} else if (strcmp(argv[2], "rootfs") == 0) {
					if (argc == 5)
						blocks = (ulong) (simple_strtoul(argv[4], NULL, 16) / MOVI_BLKSIZE) + 1;
					else
						blocks = MOVI_ROOTFS_BLKCNT;

					blk = ofsinfo.rootfs;
					printf("Reading rootfs from sector %d (%d sectors).. ", blk, blocks);

					while (blocks > 0) {
						if (blocks < MOVI_RW_MAXBLKS) {
							movi_read((uint) addr, blk, blocks);
							blocks = 0;
						} else {
							movi_read((uint) addr, blk, MOVI_RW_MAXBLKS);
							addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
							blk += MOVI_RW_MAXBLKS;
							blocks -= MOVI_RW_MAXBLKS;
						}
					}

					printf("completed\n");
				} else
					goto usage;
			} else {
				blk = (uint) simple_strtoul(argv[2], NULL, 10);
				bytes = (uint) simple_strtoul(argv[3], NULL, 16);

				if (blk >= 0 && blk <= ofsinfo.last) {
					blocks = (int) (bytes / MOVI_BLKSIZE) + 1;
					printf("Reading data from sector %d (%d sectors).. ", blk, blocks);

					while (blocks > 0) {
						if (blocks < MOVI_RW_MAXBLKS) {
							movi_read((uint) addr, blk, blocks);
							blocks = 0;
						} else {
							movi_read((uint) addr, blk, MOVI_RW_MAXBLKS);
							addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
							blk += MOVI_RW_MAXBLKS;
							blocks -= MOVI_RW_MAXBLKS;
						}
					}

					printf("completed\n");
				} else
					goto usage;
			}
		} else if (strcmp(cmd, "write") == 0) {
			if (argc == 4 || (strcmp(argv[2], "rootfs") == 0)) {
				if (strcmp(argv[2], "u-boot") == 0) {
					printf("Writing 1st bootloader to sector %d (%d sectors).. ", ofsinfo.bl1, MOVI_BL1_BLKCNT);
					movi_write((uint) addr, ofsinfo.bl1, MOVI_BL1_BLKCNT);
					printf("completed\nWriting 2nd bootloader to sector %d (%d sectors).. ", ofsinfo.bl2, MOVI_BL2_BLKCNT);
					movi_write((uint) addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
					printf("completed\n");
				} else if (strcmp(argv[2], "kernel") == 0) {
					printf("Writing kernel to sector %d (%d sectors).. ", ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					movi_write((uint) addr, ofsinfo.kernel, MOVI_ZIMAGE_BLKCNT);
					printf("completed\n");
				} else if (strcmp(argv[2], "env") == 0) {
					printf("Writing env to sector %d (%d sectors).. ", ofsinfo.env, MOVI_ENV_BLKCNT);
					movi_write((uint) addr, ofsinfo.env, MOVI_ENV_BLKCNT);
					printf("completed\n");
				} else if (strcmp(argv[2], "rootfs") == 0) {
					if (argc == 5)
						blocks = (ulong) (simple_strtoul(argv[4], NULL, 16) / MOVI_BLKSIZE) + 1;
					else
						blocks = MOVI_ROOTFS_BLKCNT;

					blk = ofsinfo.rootfs;
					printf("Writing rootfs to sector %d (%d sectors).. ", blk, blocks);

					while (blocks > 0) {
						if (blocks < MOVI_RW_MAXBLKS) {
							movi_write((uint) addr, blk, blocks);
							blocks = 0;
						} else {
							movi_write((uint) addr, blk, MOVI_RW_MAXBLKS);
							addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
							blk += MOVI_RW_MAXBLKS;
							blocks -= MOVI_RW_MAXBLKS;
						}
					}

					printf("completed\n");
				} else
					goto usage;
			} else {
				blk = (uint) simple_strtoul(argv[2], NULL, 10);
				bytes = (uint) simple_strtoul(argv[3], NULL, 16);

				if (blk >= 0 && blk <= ofsinfo.last) {
					blocks = (int) (bytes / MOVI_BLKSIZE) + 1;
					printf("Writing data to sector %d (%d sectors).. ", blk, blocks);

					while (blocks > 0) {
						if (blocks < MOVI_RW_MAXBLKS) {
							movi_write((uint) addr, blk, blocks);
							blocks = 0;
						} else {
							movi_write((uint) addr, blk, MOVI_RW_MAXBLKS);
							addr += (MOVI_RW_MAXBLKS * MOVI_BLKSIZE);
							blk += MOVI_RW_MAXBLKS;
							blocks -= MOVI_RW_MAXBLKS;
						}
					}

					printf("completed\n");
				} else
					goto usage;
			}
		} else {
			goto usage;
		}
	}

	return 1;

usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return -1;
}

U_BOOT_CMD(
	movi,	5,	0,	do_movi,
	"movi\t- moviNAND sub-system\n",
	"init {total sectors} {hc(0/1)} - Initialize moviNAND\n"
	"movi read  {u-boot | kernel} {addr} - Read data from moviNAND\n"
	"movi write {u-boot | kernel} {addr} - Write data to moviNAND\n"
	"movi read  rootfs {addr} [bytes(hex)] - Read rootfs data from moviNAND by size\n"
	"movi write rootfs {addr} [bytes(hex)] - Write rootfs data to moviNAND by size\n"
	"movi read  {sector#} {bytes(hex)} {addr} - Read data from moviNAND sector#\n"
	"movi write {sector#} {bytes(hex)} {addr} - Write data to moviNAND sector#\n"
);

int do_insnand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong total = CFG_ENV_OFFSET;
	ulong addr = PHYS_SDRAM_1;

	movi_read((uint) addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
	nand_erase(&nand_info[0], 0x0, CFG_ENV_OFFSET + CFG_ENV_SIZE);
	nand_write(&nand_info[0], 0x0, &total, (u_char *) addr);

	printf("done\n");

	return 1;
}

U_BOOT_CMD(
	insnand, 1, 0, do_insnand,
	"insnand - install SD/MMC bootloader image to NAND flash\n",
	": install SD/MMC bootloader to NAND flash\n"
);
