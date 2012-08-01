/**************************************************************************************
*
*	Project Name : S3C6400 Validation
*
*	Copyright 2006 by Samsung Electronics, Inc.
*	All rights reserved.
*
*	Project Description :
*		This software is only for validating functions of the S3C6400.
*		Anybody can use this software without our permission.
*
*--------------------------------------------------------------------------------------
*
*	File Name : ata.h
*
*	File Description : This file defines the register access function
*						and declares prototypes of adcts funcions
*
*	Author : Sunil,Roe
*	Dept. : AP Development Team
*	Created Date : 2007/1/26
*	Version : 0.1
*
*	History
*	- Created(Sunil,Roe 2007/1/26)
*
**************************************************************************************/

#ifndef __ATA6400_H__
#define __ATA6400_H__

typedef uint bool;

#define FALSE				(0)
#define TRUE				(1)
#define false				(0)
#define true				(1)

#define Outp8(a,v)	writeb(v,a)
#define Outp16(a,v)	writew(v,a)
#define Outp32(a,v)	writel(v,a)
#define Inp8(a)		readb(a)
#define Inp16(a)	readw(a)
#define Inp32(a)	readl(a)
#define Assert(x)	printf("error\n");
#define mdelay(x)	udelay(1000*x)
#define Disp		printf

#define	ENABLE		1
#define	DISABLE		0

#define	MASK		1
#define	UNMASK		0

#define	RESET		1
#define	NORESET		0

#define ATA_INT_NUM	5
#define	ATA_CONNUM	1

#define ATA_CON0	0
//#define ATA_CON1	1

#define CFCON_BASE	(0x70300000)
#define ATA_BASE	(CFCON_BASE+0x1900)

#define ATA_SECTORSIZE	512

typedef enum ATA_MODE_6400 {
	eATA_MODE_PIO = 0,
	eATA_MODE_PIOCPU = 0,
	eATA_MODE_PIODMA = 1,
	eATA_MODE_UDMA = 2,
	eATA_MODE_NONE = 3,
} eATA_MODE_6400;

typedef enum ATA_PIOMODE {
	eATA_PIO0 = 0,
	eATA_PIO1 = 1,
	eATA_PIO2 = 2,
	eATA_PIO3 = 3,
	eATA_PIO4 = 4,
} eATA_PIOMODE;

typedef enum ATA_UDMAMODE {
	eATA_UDMA0 = 0,
	eATA_UDMA1 = 1,
	eATA_UDMA2 = 2,
	eATA_UDMA3 = 3,
	eATA_UDMA4 = 4,
} eATA_UDMAMODE;

typedef enum ATA_INT_SRC_6400 {
	eATA_INT_XFER_DONE = 0,
	eATA_INT_UDMA_HOLD = 1,
	eATA_INT_IRQ = 2,
	eATA_INT_TBUF_FULL = 3,
	eATA_INT_SBUF_EMPTY = 4,
} eATA_INT_SRC_6400;

typedef enum ATA_Id {
	eATA_CONTROL = 0,	// ATA enable and clock down status
	eATA_STATUS = 1,	// ATA status
	eATA_COMMAND = 2,	// ATA command
	eATA_SWRST = 3,		// ATA software reset
	eATA_IRQ = 4,		// ATA interrupt sources
	eATA_IRQ_MASK = 5,	// ATA interrupt mask
	eATA_CFG = 6,		// ATA configuration for ATA interface
	rReserved1 = 7,
	rReserved2 = 8,
	rReserved3 = 9,
	rReserved4 = 10,
	eATA_PIO_TIME = 11,	// ATA PIO timing
	eATA_UDMA_TIME = 12,	// ATA UDMA timing
	eATA_XFR_NUM = 13,	// ATA transfer number
	eATA_XFR_CNT = 14,	// ATA current transfer count
	eATA_TBUF_START = 15,	// ATA start address of track buffer
	eATA_TBUF_SIZE = 16,	// ATA size of track buffer
	eATA_SBUF_START = 17,	// ATA start address of source buffer
	eATA_SBUF_SIZE = 18,	// ATA size of source buffer
	eATA_CADR_TBUF = 19,	// ATA current write address of track buffer
	eATA_CADR_SBUF = 20,	// ATA current read address of source buffer
	eATA_PIO_DTR = 21,	// ATA PIO device data register
	eATA_PIO_FED = 22,	// ATA PIO device Feature/Error register
	eATA_PIO_SCR = 23,	// ATA PIO sector count register
	eATA_PIO_LLR = 24,	// ATA PIO device LBA Low register
	eATA_PIO_LMR = 25,	// ATA PIO device LBA middle register
	eATA_PIO_LHR = 26,	// ATA PIO device LBA high register
	eATA_PIO_DVR = 27,	// ATA PIO device register
	eATA_PIO_CSD = 28,	// ATA PIO device command/status register
	eATA_PIO_DAD = 29,	// ATA PIO device control/alternate status register
	eATA_PIO_READY = 30,	// ATA PIO read data from device data register
	eATA_PIO_RDATA = 31,	// ATA PIO read data from device data register
	rReserved5 = 32,
	rReserved6 = 33,
	rReserved7 = 34,
	rReserved8 = 35,
	eBUS_FIFO_STATUS = 36,	// ATA internal AHB FIFO status
	eATA_FIFO_STATUS = 37,	// ATA internal ATA FIFO status
} eATA_Id;

/*---------------------------------- Init Function ---------------------------------*/
/*---------------------------------- ATA_CONTROL Register ---------------------------------*/
/*---------------------------------- ATA_STATUS Register ---------------------------------*/
#define STATUS_DEVICE_BUSY	0x80
#define STATUS_DATA_REQUEST	0x58
#define STATUS_ERR				0x1
typedef enum ATA_XFR_STATUS {
	eATA_XFR_STATUS_DEVICE_BUSY = 0x80,
	eATA_XFR_STATUS_DATA_REQUEST = 0x58,
	eATA_XFR_STATUS_ERR = 0x01,
} eATA_XFR_STATUS;

/*---------------------------------- ATA_COMMAND Register ---------------------------------*/
typedef enum ATA_XFR_CMD {
	eATA_XFR_CMD_STOP = 0,
	eATA_XFR_CMD_START = 1,
	eATA_XFR_CMD_ABORT = 2,
	eATA_XFR_CMD_CONTINUE = 3,
} eATA_XFR_CMD;
/*---------------------------------- ATA_SWRST Register ---------------------------------*/
/*---------------------------------- ATA_IRQ Register ---------------------------------*/
/*---------------------------------- ATA_IRQ_MASK Register ---------------------------------*/
typedef enum ATA_IRQ_SRC {
	eATA_IRQ_SBUF_EMPTY = 0x10,
	eATA_IRQ_TBUF_FULL = 0x08,
	eATA_IRQ_ATADEV_IRQ = 0x04,
	eATA_IRQ_UDMA_HOLD = 0x02,
	eATA_IRQ_XFR_DONE = 0x01,
	eATA_IRQ_ALL = 0x1f,
} eATA_IRQ_SRC;

/*---------------------------------- ATA_CFG Register ---------------------------------*/
typedef enum ATA_ENDIAN {
	eATA_ENDIAN_LITTLE = 0,
	eATA_ENDIAN_BIG = 1,
} eATA_ENDIAN;

typedef enum ATA_DMA_DIR {
	eATA_DMA_READ_DATA = 0,
	eATA_DMA_WRITE_DATA = 1,
} eATA_DMA_DIR;

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

typedef struct ATA_Inform {
	eATA_MODE_6400 eAtaMode;
	u32 uMaxMultiple;
	u32 uCurrentMultiple;
	u32 uMaxSectors;
	eATA_PIOMODE eMaxPioMode;
	eATA_PIOMODE eCurrentPioMode;
	eATA_UDMAMODE eMaxUdmaMode;
	eATA_UDMAMODE eCurrentUdmaMode;
	bool bIsXferDone;
	bool bIsDone;

	u32 *puATAWriteBuf;
	u32 *puATAReadBuf;
} ATA_oInform;

#endif				// __ATA6400_H__
