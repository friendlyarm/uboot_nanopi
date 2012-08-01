/*----------------------------------------------------------------------
 *
 * Filename: ata.h
 *
 * Contents: class ATA
 *
 * Authors: Whing, Anderson, Sean, ...
 *
 * Notes:
 *   1.
 *   2.
 *
 * Copyright (c) 2005 SAMSUNG Electronics.
 *----------------------------------------------------------------------
 */

#ifndef __ATA_H__
#define __ATA_H__

typedef uint bool;

#define true  1
#define false 0

#define CFCON_MODE

typedef enum {
	PIO0, PIO1, PIO2, PIO3, PIO4
} PIOMODE;

typedef enum {
	UDMA0, UDMA1, UDMA2, UDMA3, UDMA4
} UDMAMODE;

typedef enum {
	ATA_CMD_STOP, ATA_CMD_START, ATA_CMD_ABORT, ATA_CMD_CONTINUE
} ATA_TRANSFER_CMD;

typedef enum {
	ATA_INT_XFER_DONE, ATA_INT_UDMA_HOLD, ATA_INT_IRQ,
	ATA_INT_TBUF_FULL, ATA_INT_SBUF_EMPTY
} ATA_INT_SRC;

typedef enum {
	PIO_CPU, PIO_DMA, UDMA
} ATA_MODE;

#define SRC_INT_NUM 5

#endif				/* __ATA_H__ */

