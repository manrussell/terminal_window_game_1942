// -------------------------------------------------------------
//|                                                             |
//|                Windows(tm) Console MegaShell V2             |
//|                                                             |
//| (c) Etanza Systems, 1998/2001/2004/2006/2007                |
//|  - Coded by Neil Crossley                                   |
//|                                                             |
//| At last, an oldskool style environment to run your code in  |
//| (just like the old DOS days).                               |
//|                                                             |
// -------------------------------------------------------------

// Set tabs to 2 to read neatly :)


// Disallow multiple includage

#ifndef __TMDSHELL__
#define __TMDSHELL__



// --------
//| Sanity |
// --------

// Firstly some #DEFINES for sensible data typing

#define uint8		unsigned char
#define uint16	unsigned short
#define uint32	unsigned long
#define int8		signed char
#define int16		signed short
#define int32		signed long
#define int64   __int64
#define uint64  unsigned __int64
#define float32 float
#define float64 double
#define bool		unsigned long

// Define true and false

#define true   1
#define false  0

// Define NULL if not already

#ifndef NULL
#define NULL 0
#endif


// ---------
//| Defines |
// ---------

// App Startup info - flags

#define ASI_MUSTMATCHMINSIZE	0x0001	// If set, console x/y MUST match minimum sizes
#define ASI_USEREALSIZE				0x0002	// If set use the real x/y size, If clear use what u are given!
#define ASI_STARTFULLSCREEN		0x0004	// If set the force fullscreen!

// CPU Capabilities

#define CPU_NOTPENTIUM				0x0001	// CPU Is NOT Pentium Class (no extra features!)
#define CPU_UNKNOWN						0x0002	// Unknown CPU (Possibly SiS/VIA/Cyrix etc)
#define CPU_INTEL							0x0004	// Intel CPU
#define CPU_AMD								0x0008	// AMD CPU
#define CPU_RDTSC							0x0010	// Supports reading the timestamp counter
#define CPU_CMOV							0x0020	// Supports the CMOV instruction
#define CPU_MMX								0x0040	// Supports MMX  (Intel / Amd)
#define CPU_SSE								0x0080	// Supports SSE  (Intel P3+ / Newer AMD)
#define CPU_SSE2							0x0100	// Supports SSE2 (Intel P4+ / AMD 64+)
#define CPU_SSE3							0x0200	// Supports SSE3 (Intel P4(Prescott)+ / Newest AMD 64+)
#define CPU_MMX_EXTENDED			0x0400	// AMD only
#define CPU_3DNOW							0x0800	// AMD only
#define CPU_3DNOW_EXTENDED		0x1000	// AMD only
#define CPU_WINDOWS9X					0x8000	// OS is Win9x Series

// Keyboard scancodes (windows vk_?)

#define KEY_CLEAR							0x0c
#define	KEY_ESC    						0x1b
#define	KEY_F1     						0x70
#define	KEY_F2     						0x71
#define	KEY_F3     						0x72
#define	KEY_F4     						0x73
#define	KEY_F5     						0x74
#define	KEY_F6     						0x75
#define	KEY_F7     						0x76
#define	KEY_F8     						0x77
#define	KEY_F9     						0x78
#define	KEY_F10    						0x79
#define	KEY_F11    						0x7a
#define	KEY_F12    						0x7b
#define	KEY_PSC								0x2c		// print screen
#define	KEY_SCRLLOCK					0x91		// scroll lock
#define	KEY_PAUSE							0x13		// pause/break
#define	KEY_APOS2							0xdf
#define	KEY_1      						0x31
#define	KEY_2      						0x32
#define	KEY_3      						0x33
#define	KEY_4      						0x34
#define	KEY_5      						0x35
#define	KEY_6      						0x36
#define	KEY_7      						0x37
#define	KEY_8      						0x38
#define	KEY_9      						0x39
#define	KEY_0      						0x30
#define	KEY_MINUS  						0xbd
#define	KEY_EQUAL  						0xbb
#define	KEY_BSPACE 						0x08
#define	KEY_TAB    						0x09
#define	KEY_Q      						0x51
#define	KEY_W      						0x57
#define	KEY_E      						0x45
#define	KEY_R      						0x52
#define	KEY_T      						0x54
#define	KEY_Y      						0x59
#define	KEY_U      						0x55
#define	KEY_I      						0x49
#define	KEY_O      						0x4f
#define	KEY_P      						0x50
#define	KEY_LSQU   						0xdb
#define	KEY_RSQU   						0xdd
#define	KEY_RETURN 						0x0d
#define	KEY_CAPSLOCK					0x14	
#define	KEY_A      						0x41
#define	KEY_S      						0x53
#define	KEY_D      						0x44
#define	KEY_F      						0x46
#define	KEY_G      						0x47
#define	KEY_H      						0x48
#define	KEY_J      						0x4a
#define	KEY_K      						0x4b
#define	KEY_L      						0x4c
#define	KEY_SEMIC  						0xba
#define	KEY_APOS   						0xc0
#define	KEY_HASH   						0xde
#define	KEY_BSLASH 						0xdc
#define	KEY_Z      						0x5a
#define	KEY_X      						0x58
#define	KEY_C      						0x43
#define	KEY_V      						0x56
#define	KEY_B      						0x42
#define	KEY_N      						0x4e
#define	KEY_M      						0x4d
#define	KEY_COMMA  						0xbc
#define	KEY_FSTOP  						0xbe
#define	KEY_FSLASH 						0xbf
#define	KEY_SPACE							0x20
#define	KEY_INSERT						0x2d		// Cursor + editing keys
#define	KEY_HOME							0x24
#define	KEY_PGUP							0x21
#define	KEY_DELETE						0x2e
#define	KEY_END								0x23
#define	KEY_PGDN							0x22
#define	KEY_CRSRUP						0x26
#define	KEY_CRSRLF						0x25
#define	KEY_CRSRDN						0x28
#define	KEY_CRSRRT						0x27
#define	KEY_NUMLOCK						0x90		// Numeric keypad
#define	KEY_NDIVIDE						0x6f
#define	KEY_NMULTIPLY					0x6a
#define	KEY_NMINUS						0x6d
#define	KEY_N7								0x67
#define	KEY_N8								0x68
#define	KEY_N9								0x69
#define	KEY_NPLUS							0x6b
#define	KEY_N4								0x64
#define	KEY_N5								0x65
#define	KEY_N6								0x66
#define	KEY_N1								0x61
#define	KEY_N2								0x62
#define	KEY_N3								0x63
#define	KEY_N0								0x60
#define	KEY_NDPOINT						0x6e 
#define	KEY_NENTER						0x6c
#define	KEY_LWIN							0x5b		// Windoze keys
#define	KEY_RWIN							0x5c
#define	KEY_WIN								0x5d		// key to the right of the right windows key
#define KEY_SHIFT							0x10
#define KEY_CTRL							0x11
#define KEY_ALT								0x12
#define	KEY_LSHIFT 						0xa0		
#define	KEY_RSHIFT 						0xa1		
#define	KEY_LCTRL							0xa2
#define	KEY_RCTRL							0xa3		
#define	KEY_LALT							0xa4
#define	KEY_RALT							0xa5		

// Masks for shift state (value read using read_key)

#define SSTATE_MASK_ASCII			0x000000ff
#define SSTATE_MASK_KEYCODE		0x0000ff00
#define SSTATE_ANYSHIFT				0x00010000
#define SSTATE_ANYCTRL				0x00020000
#define SSTATE_ANYALT					0x00040000
#define SSTATE_NUMPAD					0x00080000
#define SSTATE_SCRLLOCK				0x00100000	
#define SSTATE_NUMLOCK				0x00200000
#define SSTATE_CAPSLOCK				0x00400000	
#define	SSTATE_LSHIFT					0x01000000
#define SSTATE_RSHIFT					0x02000000
#define SSTATE_LCTRL					0x04000000
#define SSTATE_RCTRL					0x08000000
#define SSTATE_LALT						0x10000000
#define SSTATE_RALT						0x20000000

// Masks for shift state 1 (stored at keytable[0])

#define	SS1_ANYSHIFT					0x01
#define SS1_ANYCTRL						0x02
#define SS1_ANYALT						0x04
#define SS1_NUMPAD						0x08
#define SS1_SCRLLOCK					0x10
#define SS1_NUMLOCK						0x20
#define SS1_CAPSLOCK					0x40

// Masks for shift state 2 (stored at keytable[1])

#define	SS2_LSHIFT						0x01
#define SS2_RSHIFT						0x02
#define SS2_LCTRL							0x04
#define SS2_RCTRL							0x08
#define SS2_LALT							0x10
#define SS2_RALT							0x20

// Extended asc value (read scancode if you get this)

#define	EXT_ASCII							0x00

// Keyboard mode flags

#define	KBMODE_NO_REPEAT			1
#define KBMODE_FORCE_NUMLOCK	2
#define KBMODE_MSDOS_ENTER		4

// Sizes

#define KILO									1024
#define MEGA									(1024*KILO)
#define GIGA									(1024*MEGA)

// Mouse Buttons and Flags

#define MBUT_LEFT							1
#define MBUT_RIGHT						2
#define MBUT_MIDDLE						4

// File open modes

#define FILE_READONLY					0
#define FILE_WRITEONLY				1
#define FILE_READWRITE				2

// Seek modes

#define SEEK_START						0
#define SEEK_CURRENT					1
#define SEEK_END							2

// Directory equates

#define	DIR_READABLE					0x00000001		// 0 = not readable, 1 = readable
#define	DIR_WRITABLE					0x00000002		// 0 = not writeable, 1 = writable
#define DIR_EXECUTABLE				0x00000004		// 0 = not executable, 1 = executable
#define DIR_DELETABLE					0x00000008		// 0 = not deletable, 1 = deletable
#define DIR_HIDDEN						0x00000010		// 0 = not hidden, 1 = hidden
#define DIR_DIRECTORY					0x00000400		// 1 = this is a subdirectory
#define DIR_ARCHIVE						0x00000800		// 1 = archive bit set
#define	DIR_SYSTEM						0x00001000 		// 1 = system file/directory
#define DIR_INVISIBLE					0x00002000		// 0 = visible, 1 = hidden
#define DIR_SHELLCMD					0x00004000		// 0 = normal file, 1 = .BAT file

// Flags for allocmem

#define MEM_NOCLEAR						0x80000000

// Attribute Colours
// - Pass these to setfgcol and setbgcol
// - If poking the attributes yourself or using setattribute
//   then shift up by 4 for the background colours

#define DK_BLACK							0x0		// Dark Colours
#define DK_BLUE								0x1
#define DK_GREEN							0x2
#define DK_CYAN								0x3
#define DK_RED								0x4
#define DK_MAGENTA						0x5
#define DK_YELLOW							0x6
#define DK_WHITE							0x7		
#define LT_BLACK							0x8		// Light Colours
#define LT_BLUE								0x9
#define LT_GREEN							0xA
#define LT_CYAN								0xB
#define LT_RED								0xC
#define LT_MAGENTA						0xD
#define LT_YELLOW							0xE
#define LT_WHITE							0xF

// Types for getattribute

#define ATTR_FGCOL						0
#define ATTR_BGCOL						1
#define ATTR_ATTRIBUTE				2

// Flags for outchar

#define OCFLAG_USEATTRIBUTE		1		// If set use the supplied attribute
#define OCFLAG_NOTRANSLATE    2  	// If set BS/TAB/CR/LF are printed as normal chars

// Flags for outstring

#define OSFLAG_HASATTRIBUTES	1		// If set use supplied attribute (outchar) or string contains attributes (outstring)
#define OSFLAG_NOTRANSLATE		2		// If set BS/TAB/CR/LF are printed as normal chars
#define OSFLAG_USELENGTH			4		// If set then use the length, else use string zero terminator

// Flags for set_console_mode

#define CMFLAG_NOSCROLL				1		// If set no scroll at screen bottom
#define CMFLAG_NOXWRAP				2		// If set no wrap at screen right
#define CMFLAG_NOYWRAP				4		// If set no wrap at screen bottom (only if NOSCROLL)
#define CMFLAG_MSDOSLF				8		// If set LF (linefeed) is treated as MSDOS
#define CMFLAG_NOCURSOR				16	// If set then hide the cursor

// Modes for scrollscreen

#define SCROLL_UP							0	
#define SCROLL_DOWN						1
#define SCROLL_LEFT						2
#define SCROLL_RIGHT					3

// Waveio Sample formats

#define WIO_SMPFORMAT_INT16		0
#define WIO_SMPFORMAT_INT32		1
#define WIO_SMPFORMAT_FLOAT32 2


// --------
//| Macros |
// --------

#define kbhit()				keypressed()
#define getch()				(read_key() & 255)
#define putch(a)			outchar(a,0,0)
#define printf				_print
#define getbgcolour() getattribute(ATTR_BGCOL)
#define getfgcolour() getattribute(ATTR_FGCOL)

#define dprint				_print	//russelm2 addition

// ------------
//| Structures |
// ------------

#pragma pack(1)

// App Startup infoblock 

typedef struct
{
	int32		cxsize;							// Client Width Suggested Size
	int32		cysize;							// Client Height Suggestrd Size
	int32		appsyncrate;				// Preferred sync rate
	int32		flags;							// see defines section for defs (ASI_)
} APPSTARTUPINFO;

// Machine info

typedef struct
{
	uint16	cpuflags;						// see defines above
	uint16	windowsversion;			// current windows version (9x/NT is bit 15 of cpuflags)
	uint32	cyclesperframe;			// total # of cycles per frame
	int32		vs_frames;					// main app number of frames since last vsync
	int32		app_percent;				// main app percentage time for last update
	int32		blit_percent;				// blitter percentage time for last update (total)
} MACHINE_INFO;

// Videomode infoblock

typedef struct
{
	int32  	width;							// Window Width (chars)
	int32  	height;							// Window Height (chars)
  int32  	bytespp;						// Bytes per character (always 2)
	uint8*  vram_base;          // Base address of character map
	int32  	vram_size;          // Size of character map (bytes)
	int32 	refresh_rate;       // Current Refresh Rate (Hz)
	int32		framecount;         // Current Frame count
} VMODE_INFO;

// Mouse info structure

typedef struct
{
  uint32  buttons;  					// bit 0 = left, 1 = right, 2 = middle
	int32		x;									// absoulute X 
	int32		y;									// absoulute Y 
	int32		z;									// scroll wheel, always relative co-ords (0 if no scroll wheel)
} MOUSEINFO;

// Directory entry
//
// - E3K names are limited to 100 chars (99 + \0) but windows isn't !

typedef struct
{
	char		name[480];					// 479 chars + terminator
	uint32	flags;							// see defines section
	uint32	size;
	uint32	sizehigh;
	uint32	date;
	uint32	time;
} DIR_ENTRY;

// Waveio Info

typedef struct
{
	int32		sample_rate;			// In HZ
	int32		number_channels;	// 2 (Stereo)
	int32		sample_format;		// INT16
	int32		bytes_per_sample;	// 4
	uint16	buffer_size;			// 256
	uint16	buffer_count;			// Number of extra buffers (same number passed to waveio_open)
	float32	response_time;		// in Hz, (sample_rate / buffer_size)
	float32	latency;					// in milliseconds
	uint32	flags;						// Device flags
} WIO_INFO;

#pragma pack()


// ------------
//| Prototypes |
// ------------

// System

MACHINE_INFO*	get_machine_info(void);
uint32				get_time(void);
uint32				get_date(void);
void					sleep(uint32 sleeptime);
char*					get_app_directory(void);

// Video

VMODE_INFO*		get_videomode_info(void);
void					lock_charmap(bool lockstate);
void					set_vblank_callback(void* vbc);
int32					vsync(void);
int32					get_framecount(void);

// Text Output

void					set_console_mode(uint32 mode);
uint32				get_console_mode(void);
void					clrscr(void);
void					gotoxy(int32 x, int32 y);
int32					wherex(void);
int32					wherey(void);
void					clreol(void);
void					setfgcolour(uint8 fg);
void					setbgcolour(uint8 bg);
void					setattribute(uint8 attr);
uint8					getattribute(int32 which);
void					settabsize(int32 size);
void					outchar(uint8 chr, uint8 attr, uint32 flags);
void					outstring(char* str, int32 len, uint32 flags);
void					scrollscreen(int32 how);
int32					_print(char* fmt,...);


// Keyboard and mouse

bool					keypressed(void);
uint32				read_key(void);
uint8*				get_keytable(void);
uint8					get_shiftstate(void);
void					flush_keybuffer(void);
void					set_keyboard_mode(uint32 flags);
void					enable_mouse(bool enable);
void					read_mouse(MOUSEINFO* mi);

// File routines

uint32				file_open(char* fname,uint32 flags);
uint32				file_create(char* fname);
void					file_close(uint32 handle);
uint32				file_seek(uint32 handle,int32 pos,uint32 mode);
uint32				file_getpos(uint32 handle);
uint32				file_blockread(uint32 handle, void* address, uint32 size);
uint32				file_blockwrite(uint32 handle, void* address, uint32 size);
bool 					file_exists(char* fname);
uint32				file_size(char* fname);
uint32				load_data(char* fname, void* address, uint32 loadsize);
uint32				save_data(char* fname, void* address, uint32 savesize);
char*					get_directory(void);
bool					set_directory(char* dirname);
bool					create_directory(char* dirname);
bool					remove_directory(char* dirname);
uint32				file_findfirst(char* findstr, DIR_ENTRY* de);
bool					file_findnext(uint32 handle, DIR_ENTRY* de);
void					file_findclose(uint32 handle);

// Memory stuff 

void*					allocmem(uint32 size);
void					freemem(void* addr);
void					memcopy(void* src, void* dst, uint32 count);
void					memfill(void* dst, uint8 fill, uint32 count);
bool					memcompare(void* src, void* dst, uint32 count);

// Misc routines

void					randomize(uint32 rndval);
uint32				random(void);
void					call_routine(void* routaddr);
int64					read_tsc64(void);
int32 				stringlen(char* str);

// Audio Callback Services

void					set_audio_callback(void* cb_addr);

// Waveio

int32					waveio_open(int32 buffsize, int32 samrate, bool dual, bool clone);
void      		waveio_close(void);
WIO_INFO*			waveio_get_info(void);
void      		waveio_set_callback(void* cbaddr);
char*					waveio_get_error_string(int32 err);

#endif // #ifndef __TMDSHELL__


