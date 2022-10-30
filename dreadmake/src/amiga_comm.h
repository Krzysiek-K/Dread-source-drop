

//#if MULTIPLAYER_MODE


#define COMM_MAX_RXSCAN_BUFF	256

extern byte comm_rx_buff[256];
extern word comm_rx_put;
extern word comm_rx_get;

extern byte comm_debug_buff[32];
extern word comm_debug1;
extern word comm_debug2;

extern byte comm_scan_buff[COMM_MAX_RXSCAN_BUFF];
extern word comm_scan_len;				// Packet length	(0: no packet)
extern byte comm_scan_escape;			// 0xD5 was received


void Comm_Init(void);
void Comm_Packet_End(void);
void Comm_Write_Byte(__d0 byte data);
void Comm_Write_Word(__d0 word data);
void Comm_Write(byte *data, int len);

void Parse_MP_Packets(void);
void MP_Respawn(void);
byte MP_ChooseLocation(void);

void Comm_Commit(void);					// asm

//void Amiga_Comm_Init_Asm(void);			// asm
//word Amiga_Comm_In(void);				// asm
//void Amiga_Comm_Out(__d0 word data);	// asm


//#endif
