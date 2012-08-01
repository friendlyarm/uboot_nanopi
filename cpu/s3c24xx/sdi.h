#ifndef __SDI_H__
#define __SDI_H__

#define Idle_State		0
#define	Ready_State		1
#define	Identification_State	2
#define	StandBy_State		3
#define	Transmit_State		4
#define	Data_State		5
#define	Receive_State		6
#define	Progress_State		7
#define	Dis_State		8
#define	PowerUp_State		9

#define SDIO_SD_TYPE	0
#define SDIO_MMC_TYPE	1

#define SDIO_TYPE_A	0
#define SDIO_TYPE_B	1

#define SDIO_NO_RESP		0
#define SDIO_WAIT_RESP		1
#define SDIO_SHORT_RESP		0
#define SDIO_LONG_RESP		1
#define SDIO_WITHOUT_DATA	0
#define SDIO_WITH_DATA		1

//rSDICON
#define CLOCK_TYPE		(1<<5)
#define BYTE_ORDER		(1<<4)
#define ENCLK			(1<<0)

//rSDICSTA
#define RESP_INDEX		(0xFF<<0)
#define DATA_TIMEOUT		(1<<5)
#define CMD_ON			(1<<8)
#define RESP_END		(1<<9)
#define CMD_TOUT		(1<<10)
#define CMD_END			(1<<11)
#define RESP_CRC_FAIL		(1<<12)

//rSDIDSTA
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
#define WIDE_BUS_DIS		(0<<16)
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
#define TX_FIFO_HALF_FULL		(1<<11)
#define TX_FIFO_AVAILABLE_DETECT	(1<<13)
#define RX_FIFO_AVAILABLE_DETECT	(1<<12)

typedef enum _CardType {
	SD_CARD=0,		// 4 bit Data bus SD
	MMC_CARD=1,		// 1 bit Data bus MMC
	MOVI_NAND_CARD=2,	// 4 bit Data bus MMC
	ERROR_CARD=3
} CardType;;

typedef enum _Command_type {
	GO_TO_IDLE_CMD=0,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	SEND_OP_CMD=1,			// Basic Commands and Read Stream Commands (Class0 and Class1)
	ALL_SEND_SID_CMD=2,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	SET_RELATIVE_ADDR=3,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	SET_DSR_CMD=4,			// Basic Commands and Read Stream Commands (Class0 and Class1)
	SWITCH_CMD=6,			// Basic Commands and Read Stream Commands (Class0 and Class1)
	SELECT_CMD=7,			// Basic Commands and Read Stream Commands (Class0 and Class1)
	SEND_EXT_CSD_CMD=8,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	SEND_CSD_CMD=9,			// Basic Commands and Read Stream Commands (Class0 and Class1)
	SEND_CID_CMD=10,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	READ_DAT_CMD=11,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	STOP_TRANSMISSION_CMD=12,	// Basic Commands and Read Stream Commands (Class0 and Class1)
	SEND_STATUS_CMD=13,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	BUSTEST_R_CMD=14,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	GO_INACTIVE_STATE_CMD=15,	// Basic Commands and Read Stream Commands (Class0 and Class1)
	SET_BLOCKLEN_CMD=16,		// Block Oriented Read Commands(Class2)
	READ_SINGLE_BLOCK_CMD=17,	// Block Oriented Read Commands(Class2)
	READ_MULTIPLE_BLOCK_CMD=18,	// Block Oriented Read Commands(Class2)
	BUSTEST_W_CMD=19,		// Basic Commands and Read Stream Commands (Class0 and Class1)
	WRITE_DAT_UNTIL_STOP_CMD=20,	// Stream write Command (Class3)
	SET_BLOCK_COUNT_CMD=23,		// Block Oriented Write Commands( Class4 )
	WRITE_BLOCK_CMD=24,		// Block Oriented Write Commands( Class4 )
	WRITE_MULTIPLE_BLOCK=25,	// Block Oriented Write Commands( Class4 )
	PROGRAM_CID_CMD=26,		// Block Oriented Write Commands( Class4 )
	PROGRAM_CSD_CMD=27,		// Block Oriented Write Commands( Class4 )
	SET_WRITE_PROT_CMD=28,		// Block Oriented Write Protection Commands( Class6 )
	CLR_WRITE_PROT_CMD=29,		// Block Oriented Write Protection Commands( Class6 )
	SEND_WRITE_PROT_CMD=30,		// Block Oriented Write Protection Commands( Class6 )
	ERASE_GROUP_START_CMD=35,	// Erase Commands (Class5)
	ERASE_GROUP_END_CMD=36,		// Erase Commands (Class5)
	ERASE_CMD=38,			// Erase Commands (Class5)
	FAST_IO_CMD=39,			// I/O Mode Commands (Class9)
	GO_IRQ_STATE_CMD=40,		// I/O Mode Commands (Class9)
	LOCK_UNLOCK_CMD=42,		// Lock Card (Class7)
	APP_CMD=55,			// Application Specific Commands (Class8)
	GEN_CMD=56,			// Application Specific Commands (Class8)
} CommandType;


// Function prototypes
void Test_SDI(void);
void set_system_clock(void);
void SDIO_EraseBlock(void);

void Rd_cid_csd(char reg);

int Check_status(char type);



void TR_Buf_new(void);

void Card_sel_desel(char sel_desel);

void CMD0(void);
int CMD55(void);

void Rd_CSD(void);
void Rd_CID(void);

void Rd_Block(void);
void Rd_Stream(void);
void Wt_Block(void);
void Wt_Stream(void);

void Set_Prt(void);
void Clr_Prt(void);

void Flush_Rx_buf(void);
void Set_Bus(CardType type);
void View_Rx_buf(void);
void View_Tx_buf(void);

void SDIO_PrescalerSetting(uint BaudRate);
void CARD_SEL_DESEL(unchar sel_desel);
void SET_BUSWIDTH(CardType);
int Check_DATend(void);
void Rd_Int(void);
void Wt_Int(void);

void Set_4bit_bus_MoviNand(void);

#endif /*__SDI_H___*/

