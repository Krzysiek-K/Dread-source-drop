
#include "demo.h"




extern DataActor Actor_MP_Avatar;		// TBD: generate header with all that
extern DataActor Actor_Pistol;			// TBD: generate header with all that



//CommPacket comm_packets[NUM_COMM_PACKETS];


byte comm_rx_buff[256];
word comm_rx_put;
word comm_rx_get;

byte comm_tx_buff[256];
word comm_tx_put;
word comm_tx_get;
byte comm_tx_checksum;

byte comm_scan_buff[COMM_MAX_RXSCAN_BUFF];
word comm_scan_len;				// Packet length	(0: no packet)
byte comm_scan_escape;			// 0xD5 was received





byte comm_debug_buff[32];
word comm_debug1;
word comm_debug2;


//	Format:
//		D5 <function_id> <payload>... <checksum>	D5
//



void Comm_Init(void)
{
	comm_rx_put = 0;
	comm_rx_get = 0;
	comm_tx_put = 0;
	comm_tx_get = 0;
	comm_tx_checksum = 0xA5;
	comm_scan_len = 0;
	comm_scan_escape = 0;

#if ST_COMM_LEVEL < 2
	ST_Comm_Init_Asm();
#endif

}

void ST_Comm_LowLevelWrite(byte data)
{
#if ST_COMM_LEVEL == 2
	ST_Comm_Out(data);
#elif ST_COMM_LEVEL == 1
	set_supervisor();
	while( !(USART_TX_STATUS & 0x80) ) {}
	USART_DATA = data;
	set_usermode();
#else
	comm_tx_buff[comm_tx_put] = data;
	comm_tx_put = (comm_tx_put+1) & 0xFF;
#endif
}


void Comm_Packet_End(void)
{
	ST_Comm_LowLevelWrite(0xD5);

	if( comm_tx_checksum==0x5D || comm_tx_checksum==0xD5 )
		comm_tx_checksum ^= 0xAA;

	ST_Comm_LowLevelWrite(comm_tx_checksum);

	comm_tx_checksum = 0xA5;

#if ST_COMM_LEVEL == 0
	set_supervisor();
	ST_Comm_Commit();
	set_usermode();
#endif
}

void Comm_Write_Byte(__d0 byte data)
{
	comm_tx_checksum = RAND_TAB[(byte)(comm_tx_checksum + data)];

	ST_Comm_LowLevelWrite(data);
	if( data == 0xD5 )
		ST_Comm_LowLevelWrite(0x5D);
}

void Comm_Write_Word(__d0 word data)
{
	Comm_Write_Byte((byte)(data>>8));
	Comm_Write_Byte((byte)data);
}

void Comm_Write(byte *data, int len)
{
	while( len-->0 )
		Comm_Write_Byte(*data++);
}



// Parse RX buffer and return packet length if a packet was just completed
word Comm_IsPacketReady(void)
{
#if ST_COMM_LEVEL == 2
	while( 1 )
	{
		word data = ST_Comm_In();
		if( data==0xFFFF ) break;

		comm_rx_buff[comm_rx_put] = data;
		comm_rx_put = (comm_rx_put+1) & 0xFF;
	}
#endif

	while( comm_rx_put != comm_rx_get )
	{
		byte value = comm_rx_buff[comm_rx_get];
		comm_rx_get = (comm_rx_get+1) & (COMM_MAX_RXSCAN_BUFF-1);

		if( value == 0xD5 )
		{
			comm_scan_escape = 1;
			continue;
		}
		else if( comm_scan_escape )
		{
			comm_scan_escape = 0;
			if( value == 0x5D )
			{
				value = 0xD5;
			}
			else
			{
				// Packet complete (value is checksum)
				word plen = comm_scan_len;
				comm_scan_len = 0;

				if( plen && plen<=COMM_MAX_RXSCAN_BUFF )
				{
					byte chksum = 0xA5;
					for( int i=0; i<plen; i++ )
						chksum = RAND_TAB[(byte)(chksum + comm_scan_buff[i])];

					comm_debug2 = chksum;
					if( chksum==0x5D || chksum==0xD5 )
						chksum ^= 0xAA;

					if( chksum == value )
					{
						comm_debug1++;
						return plen;
					}
				}

				continue;
			}
		}

		if( comm_scan_len<COMM_MAX_RXSCAN_BUFF )
			comm_scan_buff[comm_scan_len] = value;
		comm_scan_len++;
	}

	return 0;
}


#define PACKET_WORD(offs)		((((word)packet[(offs)])<<8) | packet[(offs)+1])

void Parse_MP_Packets(void)
{
	word len;
	
	while( (len = Comm_IsPacketReady()) )
	{
		byte *packet = comm_scan_buff;

		do {
			if( packet[0]==0x01 && len>=6 )							//	0x01	Avatar location				<xp>> <yp>> <deaths>
			{
				short xp = (short)PACKET_WORD(1);
				short yp = (short)PACKET_WORD(3);
				e_globals.mp_frags = packet[5];

				if( !mp_avatar )
					mp_avatar = Engine_SpawnActor(&Actor_MP_Avatar, xp, yp, 0);
				else
					Engine_SetThingPosition(mp_avatar, xp, yp);
				if( mp_avatar )
					Engine_ThingSetVisible(mp_avatar);

				packet += 5;
				len -= 5;
			}
			else if( packet[0]==0x02 && len>=3 )					// 0x02		Player damaged				<damage>>
			{
				if( view_player )
				{
					view_player->damage += PACKET_WORD(1);
					if( mp_avatar )
					{
						e_globals.player_damage_source_x = mp_avatar->xp;
						e_globals.player_damage_source_y = mp_avatar->yp;
					}
					Script_OnDamage(view_player);
				}

				packet += 3;
				len -= 3;
			}
			else if( packet[0]==0x03 && len>=5 )					// 0x03		Send pulse to a thing		<index>> <damage>>
			{
				word index = PACKET_WORD(1);
				EngineThing *th = NULL;

				if( index < e_n_things )
				{
					th = &e_things[index];
				}
				else
				{
					switch( index )
					{
					case 0xFFF0:	th = view_player;		break;
					case 0xFFF1:	th = mp_avatar;			break;
					}
				}

				if( th )
				{
					e_globals.pulse = PACKET_WORD(3);
					Script_OnPulse(th);
				}

				packet += 5;
				len -= 5;
			}
			else if( packet[0]==0x04 && len>=3 )					// 0x04		Send pulse to avatar		<damage>>
			{
				if( mp_avatar )
				{
					e_globals.pulse = PACKET_WORD(1);
					Script_OnPulse(mp_avatar);
				}

				packet += 3;
				len -= 3;
			}
			else if( packet[0]==0x05 && len==1 )					// 0x05		Master announcement
			{
				e_globals.mp_commstate = 2;			// set as slave

				packet += 1;
				len -= 1;
			}
			else if( packet[0]==0x06 && len==1 )					// 0x06		Spawn location request
			{
				if( e_globals.mp_commstate )
				{
					Comm_Write_Byte(0x07);
					Comm_Write_Byte(MP_ChooseLocation());
					Comm_Packet_End();
				}

				packet += 1;
				len -= 1;
			}
			else if( packet[0]==0x07 && len==2 )					// 0x07		Spawn location response		<location_index>
			{
				e_globals.mp_spawnresponse = packet[1];
				packet += 2;
				len -= 2;
			}
			else
				break;

		} while( len>0 );
	}
}


void MP_Respawn(void)
{
	// Detect master/slave
	{
		// Wait for 1s or until master is announced
		word startint = rawintnum;
		while( !e_globals.mp_commstate && (word)(rawintnum - startint) < 50 )
		{
			Parse_MP_Packets();
		}

	}

	// We are the master
	if( !e_globals.mp_commstate )
		e_globals.mp_commstate = 1;
										
	
	// Get spawn location number from the other computer
	{
		e_globals.player_health = 0;
		e_globals.mp_spawnresponse = 0xFF;
		while( e_globals.mp_spawnresponse == 0xFF )
		{
			if( e_globals.mp_commstate == 1 )
			{
				Comm_Write_Byte(0x05);
				Comm_Packet_End();
			}

			Comm_Write_Byte(0x06);
			Comm_Packet_End();

			word startint = rawintnum;
			while( rawintnum == startint ){}
			{
				Parse_MP_Packets();
			}
		}
	}
	//e_globals.mp_spawnresponse = e_globals.mp_commstate;

	Script_OnCreate(view_player);

	view_weapon->actor = &Actor_Pistol;
	Script_OnCreate(view_weapon);

	const DataLocation *loc = Map_GetLocation(e_globals.mp_spawnresponse);
	view_fine_pos_x = ((int)loc->xp) << 16;
	view_fine_pos_y = ((int)loc->yp) << 16;
	view_angle = loc->angle & 0xFF;
	view_angle_fine = (view_angle<<8) + 128;
	view_subsector = NULL;
}


byte MP_ChooseLocation(void)
{
	if( e_globals.player_health <= 0 )
		return e_globals.mp_commstate;

	word best_index = e_globals.mp_commstate;
	int best_dist = 0;

	for( word index = 0;; index++ )
	{
		const DataLocation *loc = Map_GetLocation(index);
		if( !loc ) break;

		if( loc->xp!=0 || loc->yp!=0 )
		{
			int dx = (view_fine_pos_x>>16) - loc->xp;
			int dy = (view_fine_pos_y>>16) - loc->yp;
			int dist = dx*dx + dy*dy;

			if( dist > best_dist )
			{
				best_index = index;
				best_dist = dist;
			}
		}
	}


	return best_index;
}
