#ifndef __MMC_H__
#define __MMC_H__

#define INITCLK				400000		// Init. clk 		400KHz
#define MMCCLK				20000000	// MMC clk 		20MHz
#define SDCLK				25000000	// SD normal clk	25MHz

#define	NORMAL	0
#define	HIGH	1

//SDICON
#define CLOCK_TYPE_MMC		(1<<5)
#define CLOCK_TYPE_SD		(0<<5)
#define BYTE_ORDER_TYPEB	(1<<4)
#define BYTE_ORDER_TYPEA	(0<<4)
#define ENABLE_SDCLK		(1<<0)
#define DISABLE_SDCLK		(1<<0)

//SDICCON
#define WITH_DATA		(1<<11)
#define WITHOUT_DATA		(0<<11)
#define LONG_RESP		(1<<10)
#define SHORT_RESP		(0<<10)
#define WAIT_RESP		(1<<9)
#define NO_WAIT_RESP		(0<<9)
#define CMD_START		(1<<8)

//SDICSTA
#define RESP_CRC_FAIL		(1<<12)
#define CMD_END			(1<<11)
#define CMD_TOUT		(1<<10)
#define RESP_END		(1<<9)
#define CMD_ON			(1<<8)
#define DATA_TIMEOUT		(1<<5)
#define RESP_INDEX		(0xFF<<0)

//SDIDSTA
#define CRC_STAT_FAIL		(1<<7)
#define RECEIVE_CRC_FAIL	(1<<6)
#define DATA_TOUT		(1<<5)
#define DATA_FINISH		(1<<4)
#define BUSY_FINISH		(1<<3)

//SDIDCON
#define BLOCK_NUM		(0xFFF<<0)
#define DATA_TRANSMIT_MODE	(3<<12)
#define ONLYBUSY_CHECK		(1<<12)
#define DATA_RECEIVE_MODE	(1<<13)
#define DATA_TRANSMIT_START	(1<<14)
#define DMA_EN			(1<<15)
#define WIDE_BUS_EN		(1<<16)
#define STREAM_TRANSFER		(0<<17)
#define BLOCK_TRANSFER		(1<<17)
#define BUSY_AFTER_CMD		(1<<18)
#define RCV_AFTER_CMD		(1<<19)
#define TRANS_AFTER_RESP	(1<<20)
#define INT_PERIOD_TYPE		(1<<21)
#define BYTE_TRANSFER		(0<<22)
#define HALFWORD_TRANSFER	(1<<22)
#define WORD_TRANSFER		(1<<23)
#define BURST4_EN		(1<<24)

//SDIFSTA
#define FIFO_RESET			(1<<16)
#define TX_FIFO_AVAILABLE_DETECT	(1<<13)
#define RX_FIFO_AVAILABLE_DETECT	(1<<12)

typedef enum CardType {
	ERROR_CARD	= 0,
	MMC_CARD	= 1,		// 1 bit Data bus MMC
	SD_CARD		= 2,		// 4 bit Data bus SD
	MOVI_NAND_CARD	= 3,		// 4 bit Data bus MMC	
} CardType;

#endif /*__HS_MMC_H__*/
