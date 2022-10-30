
#pragma once



// blitter.cpp


#define MINT_A		0xF0
#define MINT_B		0xCC
#define MINT_C		0xAA
#define MINTERM(x)	((x)&0xFF)


class Blitter {
public:
	struct MemPool {
		enum {
			AERR_INVALID_READ_ADDRESS		= (1<<0),
			AERR_INVALID_WRITE_ADDRESS		= (1<<1),
			AERR_UNALIGNED_WRITE			= (1<<2),
		};
		word	*mem_start;
		dword	mem_size;
		int		access_error;


		MemPool() : mem_start(NULL), mem_size(0), access_error(0) {}

		template<class T>
		void SetRef(vector<T> &v) {
			mem_start = v.size()>0 ? (word*)&v[0] : NULL;
			mem_size = v.size()*sizeof(T);
			access_error = 0;
		}

		void SetMem(const void *ptr,int size) {
			mem_start = (word*)ptr;
			mem_size = size;
			access_error = 0;
		}

		word Read(dword addr) {
			if(addr<mem_size)
				return mem_start[addr/2];
			access_error |= AERR_INVALID_READ_ADDRESS;
			return 0xA5A5;
		}
		void Write(dword addr,word value) {
			if(addr&1)
				addr |= AERR_UNALIGNED_WRITE;
			if(addr<mem_size)
				mem_start[addr/2] = value;
			else
				addr |= AERR_INVALID_WRITE_ADDRESS;
		}
	};

	enum {
		BC0_ASH_SHIFT	= 12,
		BC0_USEA		= (1<<11),
		BC0_USEB		= (1<<10),
		BC0_USEC		= (1<< 9),
		BC0_USED		= (1<< 8),
		BC1_BSH_SHIFT	= 12,
	};

	//		AREA MODE("normal")
	// 	-------------------------
	// 	BIT# BLTCON0     BLTCON1
	// 	---- -------     -------
	// 	15   ASH3        BSH3
	// 	14   ASH2        BSH2
	// 	13   ASH1        BSH1
	// 	12   ASA0        BSH0
	// 	11   USEA         X
	// 	10   USEB         X
	// 	09   USEC         X
	// 	08   USED         X
	// 	07   LF7          DOFF
	// 	06   LF6          X
	// 	05   LF5          X
	// 	04   LF4         EFE
	// 	03   LF3         IFE
	// 	02   LF2         FCI
	// 	01   LF1         DESC
	// 	00   LF0         LINE(=0)
	// 
	// 	ASH3-0  Shift value of A source
	// 	BSH3-0  Shift value of B source
	// 	USEA    Mode control bit to use source A
	// 	USEB    Mode control bit to use source B
	// 	USEC    Mode control bit to use source C
	// 	USED    Mode control bit to use destination D
	// 	LF7-0   Logic function minterm select lines
	// 	EFE     Exclusive fill enable
	// 	IFE     Inclusive fill enable
	// 	FCI     Fill carry input
	// 	DESC    Descending(decreasing address) control bit
	// 	LINE    Line mode control bit(set to 0)

	MemPool	pool_a;
	MemPool	pool_b;
	MemPool	pool_c;
	MemPool	pool_d;
	int		cycle_counter;

	word	BLTCON0;		// Blitter control register 0
	word	BLTCON1;		// Blitter control register 1
	word	BLTAFWM;		// Blitter first word mask for source A
	word	BLTALWM;		// Blitter last word mask for source A
	dword	BLTCPT;			// Blitter pointer to source C
	dword	BLTBPT;			// Blitter pointer to source B
	dword	BLTAPT;			// Blitter pointer to source A
	dword	BLTDPT;			// Blitter pointer to destination D
	word	BLTSIZE;		// Blitter start and size (window width,height)
	//word	BLTSIZV;		// Blitter V size (for 15 bit vertical size)
	//word	BLTSIZH;		// Blitter H size and start (for 11 bit H size)
	short	BLTCMOD;		// Blitter modulo for source C
	short	BLTBMOD;		// Blitter modulo for source B
	short	BLTAMOD;		// Blitter modulo for source A
	short	BLTDMOD;		// Blitter modulo for destination D
	word	BLTCDAT;		// Blitter source C data register
	word	BLTBDAT;		// Blitter source B data register
	word	BLTADAT;		// Blitter source A data register


	Blitter() { InitDefaults(); }

	void InitDefaults();
	void Run();
};
