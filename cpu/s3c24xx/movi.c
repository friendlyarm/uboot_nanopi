#include <common.h>

#ifdef CONFIG_MOVINAND

#if defined(CONFIG_S3C2450)
#include <s3c2450.h>
#elif defined(CONFIG_S3C2416)
#include <s3c2416.h>
#endif
#include <movi.h>
#include <asm/io.h>

uint movi_hc = 0;

void movi_set_capacity(void)
{
	if (MOVI_HIGH_CAPACITY & 0x1)
		movi_hc = 1;
}

int movi_set_ofs(uint last)
{
	int changed = 0;

	if (ofsinfo.last != last) {
		ofsinfo.last 	= last - (eFUSE_SIZE / MOVI_BLKSIZE);
		ofsinfo.bl1	= ofsinfo.last - MOVI_BL1_BLKCNT;
		ofsinfo.env	= ofsinfo.bl1 - MOVI_ENV_BLKCNT;
		ofsinfo.bl2	= ofsinfo.bl1 - (MOVI_BL2_BLKCNT + MOVI_ENV_BLKCNT);
		ofsinfo.kernel	= ofsinfo.bl2 - MOVI_ZIMAGE_BLKCNT;
		ofsinfo.rootfs	= ofsinfo.kernel - MOVI_ROOTFS_BLKCNT;
		changed = 1;
	}

	return changed;
}

int movi_init(void)
{
	hsmmc_set_gpio();
	hsmmc_reset();

	if (hsmmc_init()) {
		printf("\nCard Initialization failed.\n");
		return -1;
	}

	return 1;
}

void movi_write_env(ulong addr)
{
	movi_write((uint)addr, ofsinfo.env, MOVI_ENV_BLKCNT);
}

void movi_read_env(ulong addr)
{
	movi_read((uint)addr, ofsinfo.env, MOVI_ENV_BLKCNT);
}

void movi_bl2_copy(void)
{
	CopyMovitoMem(MOVI_BL2_POS, MOVI_BL2_BLKCNT, (uint *)BL2_BASE, MOVI_INIT_REQUIRED);
}

#endif
