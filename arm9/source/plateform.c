#include <nds.h>
#include <nds.h>
#include <string.h>
#include <nds/arm9/console.h> //basic print funcionality
#include <nds/arm9/image.h>

#include "fs.h"

#include <stdarg.h>
#include <ctype.h>

#include "amsdos.h"

#include "types.h"
#include "autotype.h"
#include "plateform.h"
#include "snapshot.h"
#include "config.h"
#include "sound.h"
#include "wifimanage.h"

#ifdef USE_Z80_ORIG
#include "z80.h"
#endif

#ifdef USE_Z80_ASM2
#include "z80_asm.h"
#endif

#ifdef USE_Z80_ASM
#include "drz80_cpc.h"
#endif


#include "ppi.h"
#include "vga.h"
#include "upd.h"
#include "crtc.h"

#include "gif.h"

#include "icons_gif.h"
#include "background_gif.h"
#include "filesel_gif.h"
#include "background_menu_gif.h"
#include "filefont_gif.h"
#include "smallfont_gif.h"
#include "smallfont_red_gif.h"


#ifdef USE_SPLASH1
#include "splash1_gif.h"
#endif

#ifdef USE_SPLASH2
#include "splash2_gif.h"
#endif

#include "cpcfont.h"

#include "../../arm7/source/ipc2.h"

#define timers2ms(tlow,thigh)(tlow | (thigh<<16)) >> 5

#define AlphaBlendFast(pixel,backpixel) (((((pixel) & 0x7bde) >> 1) | (((backpixel) & 0x7bde) >> 1)) | 0x8000)

#define SCR_TOUCH (((~IPC->buttons) >> 6) & 1)

extern int turbo;        // dans main.c
extern int Image;        // dans upd.c

int usestylus=0, usestylusauto=1;
int usemagnum=0;

int multikeypressed=0;

int iconAutoInsert=-1;

static int hack_tabcoul=0;

static u16 AlphaBlend(u16 pixel, u16 backpixel, u16 opacity)
{
	// Blend image with background, based on opacity
	// Code optimization from http://www.gamedev.net/reference/articles/article817.asp
	// result = destPixel + ((srcPixel - destPixel) * ALPHA) / 256

	u16 dwAlphaRBtemp = (backpixel & 0x7c1f);
	u16 dwAlphaGtemp = (backpixel & 0x03e0);
	u16 dw5bitOpacity = (opacity >> 3);

	return (
	((dwAlphaRBtemp + ((((pixel & 0x7c1f) - dwAlphaRBtemp) * dw5bitOpacity) >> 5)) & 0x7c1f) |
	((dwAlphaGtemp + ((((pixel & 0x03e0) - dwAlphaGtemp) * dw5bitOpacity) >> 5)) & 0x03e0) | 0x8000
	);
}


#define MAX_ROM_MODS 2
#include "rom_mods.h"

/*
inline char toupper(const char toLower)
{
if ((toLower >= 'a') && (toLower <= 'z'))
return char(toLower - 0x20);
return toLower;
}
*/

int emulator_patch_ROM (u8 *pbROMlo)
{
	u8 *pbPtr;
	// int CPCkeyboard=1; // French 
	int CPCkeyboard=0;  // Default

	if (CPCkeyboard<1) {
		return 0;
	}

	// pbPtr = pbROMlo + 0x1d69; // location of the keyboard translation table on 664 or 6128
	pbPtr = pbROMlo + 0x1eef; // location of the keyboard translation table on 6128    
	memcpy(pbPtr, cpc_keytrans[CPCkeyboard-1], 240); // patch the CPC OS ROM with the chosen keyboard layout

	pbPtr = pbROMlo + 0x3800;
	memcpy(pbPtr, cpc_charset[CPCkeyboard-1], 2048); // add the corresponding character set

	return 0;
}

#define MAXFILE 1024

int resize=1;

static char currentfile[256];
static int currentsnap=0; // 0,1 ou 2
int snapsave=0;
int iconmenu=-1;
int styluspressed=1;

u16 *filesel;
u16 *icons;
u16 *menubuffer;
u16 *menubufferlow;

int consolepos=0;
static char consolestring[1024];

int Fmnbr;

void myconsoleClear(void);
void UpdateKeyMenu(void);

static u8 offsetY200[272]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 255, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 255, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 255, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 255, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 255, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 255, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 255, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 255, 191, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
static u8 offsetY192[272]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
static u8 offsetY272[272]={0, 1, 2, 255, 3, 4, 255, 5, 6, 7, 255, 8, 9, 255, 10, 11, 255, 12, 13, 14, 255, 15, 16, 255, 17, 18, 19, 255, 20, 21, 255, 22, 23, 255, 24, 25, 26, 255, 27, 28, 255, 29, 30, 31, 255, 32, 33, 255, 34, 35, 255, 36, 37, 38, 255, 39, 40, 255, 41, 42, 255, 43, 44, 45, 255, 46, 47, 255, 48, 49, 50, 255, 51, 52, 255, 53, 54, 255, 55, 56, 57, 255, 58, 59, 255, 60, 61, 62, 255, 63, 64, 255, 65, 66, 255, 67, 68, 69, 255, 70, 71, 255, 72, 73, 74, 255, 75, 76, 255, 77, 78, 255, 79, 80, 81, 255, 82, 83, 255, 84, 85, 255, 86, 87, 88, 255, 89, 90, 255, 91, 92, 93, 255, 94, 95, 255, 96, 97, 255, 98, 99, 100, 255, 101, 102, 255, 103, 104, 105, 255, 106, 107, 255, 108, 109, 255, 110, 111, 112, 255, 113, 114, 255, 115, 116, 255, 117, 118, 119, 255, 120, 121, 255, 122, 123, 124, 255, 125, 126, 255, 127, 128, 255, 129, 130, 131, 255, 132, 133, 255, 134, 135, 136, 255, 137, 138, 255, 139, 140, 255, 141, 142, 143, 255, 144, 145, 255, 146, 147, 148, 255, 149, 150, 255, 151, 152, 255, 153, 154, 155, 255, 156, 157, 255, 158, 159, 255, 160, 161, 162, 255, 163, 164, 255, 165, 166, 167, 255, 168, 169, 255, 170, 171, 255, 172, 173, 174, 255, 175, 176, 255, 177, 178, 179, 255, 180, 181, 255, 182, 183, 255, 184, 185, 186, 255, 187, 188, 255, 189, 190, 255, 191};

static u8 offsetY[272];

int UpdateInk=1;

static int x0,y0;
static int maxy;

// 384

// En offsetY200: x0(0,0) yO(0,72)
// En offsetY192: x0(0,384-256) y0(0,80)
// En offsetY272: x0(0,0) y0(0,0)

pfctExecInstZ80 ExecInstZ80;
pfctResetZ80 ResetZ80;
pfctSetIRQZ80 SetIRQZ80;

static byte bit_values[8] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

#ifndef USE_CONSOLE
u16 *backBuffer=NULL;
#endif

u8 *frontBuffer;

int capslock=0;

int dispframerate=0;
static int frame=0, msgframe=0;
static char msgbuf[33] = {0};

static int Regs1=0, Regs2=0, Regs6=0, Regs7=0;       // Utilisé par le resize automatique

u16 *kbdBuffer;

void DispIcons(void);
int zicon, nbricon, xicon;

int inMenu=0;
struct kmenu *forceMenu=NULL;

int keyEmul=3; // 2: Emul du clavier, 3: normal
int monochrome=0;

KKFONT *filefont;
KKFONT *font;
KKFONT *fontred;

static u8 *MemBitmap;
int Turbo = 1;

static u8 TabPoints[ 4 ][ 256 ][ 4 ];
static u8 TabPointsDef[ 4 ][ 256 ][ 4 ];

char *keyname0[] = {
	"CURSOR_UP", // = 0
	"CURSOR_RIGHT",
	"CURSOR_DOWN",
	"F9",
	"F6",
	"F3",
	"SMALL_ENTER",
	"FDOT",
	/* line 1", bit 0..bit 7 */
	"CURSOR_LEFT",
	"COPY",
	"F7",
	"F8",
	"F5",
	"F1",
	"F2",
	"F0",
	/* line 2", bit 0..bit 7 */
	"CLR",
	"OPEN_SQUARE_BRACKET",
	"RETURN",
	"CLOSE_SQUARE_BRACKET",
	"F4",
	"SHIFT",
	"FORWARD_SLASH",
	"CONTROL",
	/* line 3", bit 0.. bit 7 */
	"HAT",
	"MINUS",
	"AT",
	"P",
	"SEMICOLON",
	"COLON",
	"BACKSLASH",
	"DOT",
	/* line 4", bit 0..bit 7 */
	"ZERO",
	"9",
	"O",
	"I",
	"L",
	"K",
	"M",
	"COMMA",
	/* line 5", bit 0..bit 7 */
	"8",
	"7",
	"U",
	"Y",
	"H",
	"J",
	"N",
	"SPACE",
	/* line 6", bit 0..bit 7 */
	"6",
	"5",
	"R",
	"T",
	"G",
	"F",
	"B",
	"V",
	/* line 7", bit 0.. bit 7 */
	"4",
	"3",
	"E",
	"W",
	"S",
	"D",
	"C",
	"X",
	/* line 8", bit 0.. bit 7 */
	"1",
	"2",
	"ESC",
	"Q",
	"TAB",
	"A",
	"CAPS_LOCK",
	"Z",
	/* line 9", bit 7..bit 0 */
	"JOY_UP",
	"JOY_DOWN",
	"JOY_LEFT",
	"JOY_RIGHT",
	"JOY_FIRE1",
	"JOY_FIRE2",
	"SPARE",
	"DEL", 

	/* no key press */
	"NIL"
};

int RgbCPCdef[ 32 ] =  {
	0x7F7F7F,                 // Blanc            (13)
	0x7F7F7F,                 // Blanc            (13)
	0x00FF7F,                 // Vert Marin       (19)
	0xFFFF7F,                 // Jaune Pastel     (25)
	0x00007F,                 // Bleu              (1)
	0xFF007F,                 // Pourpre           (7)
	0x007F7F,                 // Turquoise        (10)
	0xFF7F7F,                 // Rose             (16)
	0xFF007F,                 // Pourpre           (7)
	0xFFFF00,                 // Jaune vif        (24)
	0xFFFF00,                 // Jaune vif        (24)
	0xFFFFFF,                 // Blanc Brillant   (26)
	0xFF0000,                 // Rouge vif         (6)
	0xFF00FF,                 // Magenta vif       (8)
	0xFF7F00,                 // Orange           (15)
	0xFF7FFF,                 // Magenta pastel   (17)
	0x00007F,                 // Bleu              (1)
	0x00FF7F,                 // Vert Marin       (19)
	0x00FF00,                 // Vert vif         (18)
	0x00FFFF,                 // Turquoise vif    (20)
	0x000000,                 // Noir              (0)
	0x0000FF,                 // Bleu vif          (2)
	0x007F00,                 // Vert              (9)
	0x007FFF,                 // Bleu ciel        (11)
	0x7F007F,                 // Magenta           (4)
	0x7FFF7F,                 // Vert pastel      (22)
	0x7FFF00,                 // Vert citron      (21)
	0x7FFFFF,                 // Turquoise pastel (23)
	0x7F0000,                 // Rouge             (3)
	0x7F00FF,                 // Mauve             (5)
	0x7F7F00,                 // Jaune            (12)
	0x7F7FFF                  // Bleu pastel      (14)
};

void TraceLigne8B512( int y, signed int AdrLo, int AdrHi );


typedef struct {
	int normal;
} CPC_MAP;

int cpckeypressed[NBCPCKEY];

RECT keypos[NBCPCKEY] = { 
	{0,51,14,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{15,51,33,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{34,51,52,72}, // 17    0x40 | MOD_CPC_SHIFT,   // CPC_0        
	{53,51,70,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{71,51,87,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{88,51,104,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0                
	{105,51,121,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{122,51,138,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{139,51,155,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0                        
	{156,51,172,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{173,51,189,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{190,51,206,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0        
	{207,51,223,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_LEFT
	{224,51,240,72},   //    0x41 | MOD_CPC_SHIFT,   // CPC_UP
	{241,51,255,72},  // CP_RIGHT

	{0,73,14,94},    // (0)
	{15,73,33,94},   //    0x80 | MOD_CPC_SHIFT,   // CPC_1
	{34,73,52,94},   //    0x81 | MOD_CPC_SHIFT,   // CPC_2
	{53,73,70,94}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{71,73,87,94}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{88,73,104,94}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{105,73,121,94}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{122,73,138,94}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{139,73,155,94}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{156,73,172,94}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{173,73,189,94}, // (10)  0x40 | MOD_CPC_SHIFT,   // CPC_0
	{190,73,206,94}, //    0x70 | MOD_CPC_SHIFT,   // CPC_=
	{207,73,223,94}, //    0x61 | MOD_CPC_SHIFT,   // CPC_LAMBDA
	{224,73,240,94}, //    0x60 | MOD_CPC_SHIFT,   // CPC_CLR
	{241,73,256,94}, //    0x51 | MOD_CPC_SHIFT,   // CPC_DEL

	{0,95,19,116},
	{20,95,38,116}, //    0x83,                   // CPC_a
	{39,95,57,116}, //    0x73,                   // CPC_z
	{58,95,76,116}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{77,95,95,116}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{96,95,114,116}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{115,95,133,116}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{134,95,152,116}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{153,95,171,116}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{172,95,190,116}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{191,95,207,116}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{208,95,224,116}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{225,95,241,116}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{242,95,256,138},//    0x22,                   // CPC_RETURN

	{0,117,21,138},
	{22,117,40,138}, //    0x83,                   // CPC_A
	{41,117,59,138}, //    0x73,                   // CPC_S
	{60,117,78,138}, //    0x71 | MOD_CPC_SHIFT,   // CPC_D
	{79,117,97,138}, //    0x70 | MOD_CPC_SHIFT,   // CPC_F
	{98,117,116,138}, //    0x61 | MOD_CPC_SHIFT,   // CPC_G
	{117,117,135,138}, //    0x60 | MOD_CPC_SHIFT,   // CPC_H
	{136,117,154,138}, //    0x51 | MOD_CPC_SHIFT,   // CPC_J
	{155,117,173,138}, //    0x50 | MOD_CPC_SHIFT,   // CPC_K
	{174,117,190,138}, //    0x41 | MOD_CPC_SHIFT,   // CPC_L
	{191,117,207,138}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{208,117,224,138}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{225,117,241,138}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5        

	{0,139,28,160},  // SHIFT
	{29,139,47,160}, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
	{48,139,66,160}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{67,139,85,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{86,139,104,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{105,139,123,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{124,139,142,160}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{143,139,161,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{162,139,178,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{179,139,195,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{196,139,212,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{213,139,229,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{230,139,256,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6

	{0,161,57,182}, //    0x55,                   // CPC_j
	{58,161,95,182}, //    0x55,                   // CPC_j        
	{96,161,207,182}, //    0x55,                   // CPC_j        
	{208,161,256,182} //    0x55,                   // CPC_j        	
};


int keyown[11];

CPC_MAP keymap[NBCPCKEY] = {
	{ CPC_FDOT },	    
	{ CPC_F1 },   
	{ CPC_F2 },   
	{ CPC_F3 },       
	{ CPC_F4 },   
	{ CPC_F5 },      
	{ CPC_F6 },
	{ CPC_F7 },
	{ CPC_F8 },
	{ CPC_F9 },
	{ CPC_F0 }, 
	{ CPC_CURSOR_UP },            	
	{ CPC_CURSOR_LEFT },   
	{ CPC_CURSOR_DOWN },            
	{ CPC_CURSOR_RIGHT },

	{ CPC_ESC },
	{ CPC_1 },
	{ CPC_2 },
	{ CPC_3 },
	{ CPC_4 },
	{ CPC_5 },
	{ CPC_6 },            
	{ CPC_7 },
	{ CPC_8 },      
	{ CPC_9 },         
	{ CPC_ZERO },         
	{ CPC_MINUS },   
	{ CPC_HAT },   
	{ CPC_CLR },
	{ CPC_DEL },

	{ CPC_TAB },       //
	{ CPC_Q },         
	{ CPC_W },         
	{ CPC_E },            
	{ CPC_R },            
	{ CPC_T },            
	{ CPC_Y },   
	{ CPC_U },      
	{ CPC_I },   
	{ CPC_O },   
	{ CPC_P },   
	{ CPC_AT },   
	{ CPC_OPEN_SQUARE_BRACKET },            
	{ CPC_RETURN },            

	{ CPC_CAPS_LOCK },   //
	{ CPC_A },   
	{ CPC_S },   
	{ CPC_D },   
	{ CPC_F },   
	{ CPC_G },      
	{ CPC_H },   
	{ CPC_J },   
	{ CPC_K },   
	{ CPC_L },   
	{ CPC_COLON },   
	{ CPC_SEMICOLON },   
	{ CPC_CLOSE_SQUARE_BRACKET },   

	{ CPC_SHIFT },   //
	{ CPC_Z },   
	{ CPC_X },   
	{ CPC_C },   
	{ CPC_V },   
	{ CPC_B },   
	{ CPC_N },   
	{ CPC_M },            
	{ CPC_COMMA },   
	{ CPC_DOT },   
	{ CPC_FORWARD_SLASH },   
	{ CPC_BACKSLASH },   
	{ CPC_SHIFT },   


	{ CPC_CONTROL },   
	{ CPC_COPY },   
	{ CPC_SPACE },   
	{ CPC_SMALL_ENTER }
};


struct kmenu root;
struct kmenu *menuRomId;
struct kmenu *FirstROM;
struct kmenu *keyMenu;

struct kmenu *AddMenu(struct kmenu *parent, char *title, int id)
{
	struct kmenu *kcur;

	kcur=(struct kmenu *)calloc(sizeof(struct kmenu),1);
	kcur->parent=parent;
	kcur->firstchild=NULL;
	kcur->lastchild=NULL;
	kcur->next=NULL;
	kcur->nbr=0;
	strcpy(kcur->title, title);
	kcur->id = id;

	if (kcur->parent->nbr==0) {
		kcur->parent->firstchild=kcur;
		kcur->parent->lastchild=kcur;
		kcur->parent->nbr=1;
	} else {
		struct kmenu *i, *i0;

		i0=NULL;
		i=kcur->parent->firstchild;
		do {
			if (stricmp(kcur->title,i->title)<0) {
				break;  // placer kcur juste avant i
			}
			i0=i;
			i=i->next;
		} while(i!=NULL);

		if (i==NULL) {
			kcur->parent->lastchild->next=kcur;
			kcur->parent->lastchild=kcur;
		} else {
			kcur->next=i;
			if (i0==NULL) {
				kcur->parent->firstchild=kcur;
			} else {
				i0->next=kcur;
			}
		}
		kcur->parent->nbr++;
	}

	return kcur;
}


void LoadROMFile(char *filename)
{
	struct kmenu *file;
	char title[256];
	char *name;
	int len;

	name=strrchr(filename,'/');
	if (name==NULL) return;

	name++;

	len=strlen(name);
	if (!strcasecmp(".dsk",&name[len-4])) {
		sprintf(title,"%c %s", 2, name);
		title[2]=toupper(title[2]);
		title[len-2]=0;
	}

	if (!strcasecmp(".sna",&name[len-4])) {
		sprintf(title,"%c %s", 1, name);
		title[2]=toupper(title[2]);
		title[len-2]=0;
	}

	if (!strcasecmp(".urm",&name[len-4])) {
		sprintf(title,"%c %s", 1, name);
		title[2]=toupper(title[2]);
		title[len-2]=0;
	}   


	file=AddMenu(menuRomId, title, ID_FILE);

	if (Fmnbr==0) {
		FirstROM=file;
	}

	file->object = (char*)MyAlloc(strlen(filename)+1,"ROM Text");
	strcpy(file->object, filename);
	Fmnbr++;
}


void SelectSNAP(void);

int HaveSlotSnap(char *file, int c)
{
	char snap[256];
	char *buf;

	sprintf(snap, "/%s.%d", file, c+1);
	buf=strchr(snap,'.');
	*buf='_';

	return FileExists(snap);
}

void LoadSlotSnap(char *file, int c)
{
	char snap[256];
	char *buf;
	u8 *rom=NULL;
	u32 romsize=0;

	sprintf(snap, "/%s.%d", file, c+1);
	buf=strchr(snap,'.');
	*buf='_';

	rom = FS_Readfile(snap, &romsize);

	if (rom!=NULL) {
		LireSnapshotMem(rom);
		free(rom);
	}
}

void DeleteSlotSnap(char *file, int c)
{
	char snap[256];
	char *buf;

	sprintf(snap, "/%s.%d", file, c+1);
	buf=strchr(snap,'.');
	*buf='_';

	FS_Delete(snap);
}

void LireRom(struct kmenu *FM, int autostart, int snapshot)
{
	u8 *rom=NULL;
	u32 romsize=0;

	myprintf("Loading %s", FM->object); // FM->title
	myprintf("%s", FM->object); // FM->title
	rom = FS_Readfile(FM->object, &romsize);

	if (rom==NULL) {
		myprintf("Rom not found");
		return;
	}

	// myprintf("(%d) %02x %02x %02x %02x %02x", romsize, rom[0], rom[1], rom[2], rom[3], rom[4]);

	if (!memcmp(rom, "PK", 2)) {
		//
	}

	/*
	char str[10];
	memcpy(str, rom, 8);
	str[8]=0;
	
	myprintf("ok: %s", str);
	*/
	
#ifdef USE_SNAPSHOT    
	if (!memcmp(rom, "MV - SNA", 8 )) {
		LireSnapshotMem(rom);
		currentsnap=0;
		strcpy(currentfile, FM->object);
		if (snapshot!=0) {
			if (HaveSlotSnap(currentfile, currentsnap)) {
				LoadSlotSnap(currentfile, currentsnap);
			} 
		}
	}
#endif
	if ( (!memcmp(rom, "MV - CPCEMU", 11)) || (!memcmp(rom,"EXTENDED", 8)) ) {        
		currentsnap=0;
		strcpy(currentfile, FM->object);
		myprintf("Disk loaded");
		if (autostart) {
			if (HaveSlotSnap(currentfile, currentsnap)) {
				LireDiskMem(rom, romsize);
				LoadSlotSnap(currentfile, currentsnap);
			} else {
				if (strstr(currentfile,"(cpm)")!=NULL) {
					LireDiskMem(rom, romsize);
					AutoType_SetString("|cpm\n", TRUE);
				} else {
					char autofile[256];

					LireDiskMem(rom, romsize);
					// if (AMSDOS_GenerateAutorunCommand(autofile)!=AUTORUN_NOT_POSSIBLE) {
					AMSDOS_GenerateAutorunCommand(autofile);
					AutoType_SetString(autofile, TRUE);		//TRUE
					// }
				}
			}
		} else {
			LireDiskMem(rom, romsize);
		}
	}
	free(rom);

#ifdef USE_SAVESNAP_SELECT
	if (snapsave) {
		SelectSNAP();
	}
#endif

}

void DiskSelection(struct kmenu *current)
{
#ifndef USE_CONSOLE
	u16 *savebuf;
	int keys_pressed,styluspressed,x,y;

	if (inMenu==0) {
		PauseSound();
	}

	savebuf=(u16*)malloc(256*192*2);
	dmaCopy(backBuffer, savebuf, 256*192*2);
	dmaCopy(filesel, backBuffer, 256*192*2);

	if (!HaveSlotSnap(current->object, 0)) {		// Cache load and erase
		RECT r;

		SetRect(&r, 100,165, 175,188);
		FillRect(&r, RGB15((135>>3), (177>>3), (243>>3))|0x8000);

		SetRect(&r, 179,165, 254,188);
		FillRect(&r, RGB15((135>>3), (177>>3), (243>>3))|0x8000);		
	}

	// Vide keybuffer
	do {
		keys_pressed = ~(REG_KEYINPUT);
	} while ((keys_pressed & (KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);



	x=y=styluspressed=0;
	while(1) {
		if ((~IPC->buttons) & (1 << 6)) {
			if (styluspressed==0) {
				styluspressed=1;

				x=IPC->touchXpx;
				y=IPC->touchYpx;
			}
		} else {
			if (styluspressed==1) {
				if ((x>=180) && (y>=105) && (x<=250) && (y<=125)) {
					LireRom(current,1,0);			// autostart			
					break;
				}
				if ((x>=180) && (y>=133) && (x<=250) && (y<=253)) {
					LireRom(current,0,0);			// insert disk 
					break;
				}
				if ((x>=100) && (y>=165) && (x<=170) && (y<=185)) {
					LireRom(current,0,1);			// load snapshot
					break;
				}
			}
			styluspressed=0;
		}
	}

	dmaCopy(savebuf, backBuffer, 256*192*2);
	free(savebuf);

	if (inMenu==0) {
		PlaySound();
	}
#endif	
}

void SelectSNAP(void)
{
#ifdef USE_FAT
#ifndef USE_CONSOLE
	u16 keys_pressed;

	dmaCopy(menubufferlow, backBuffer, 256*192*2);

	currentsnap=0;

	// Wait key off
	do {
		keys_pressed = ~(REG_KEYINPUT);
	} while ((keys_pressed & (KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);

	DrawTextLeft(backBuffer, filefont, RGB15(31,31,0) | 0x8000, 15,  1, "Select Save Slot");

	int x;
	RECT r;

	for(x=0;x<3;x++) {
		char id[2];
		char *buf;
		char snap[256];
		int haveimg, havesnap;


		sprintf(snap, "/%s.%d", currentfile, currentsnap+1);
		buf=strchr(snap,'.');
		*buf='_';
		havesnap = FileExists(snap);

		sprintf(snap, "/%s_i.%d", currentfile, currentsnap+1);
		buf=strchr(snap,'.');
		*buf='_';
		haveimg = FileExists(snap);

		sprintf(id,"%d", x+1);
		SetRect(&r, 5+x*83, 130, 5+x*83+80, 50+130);
		FillRect(&r, RGB15((156>>3), (178>>3), (165>>3))|0x8000);
		DrawText(backBuffer, font, 7+x*83, 132, id);
		if (!havesnap) {
			DrawText(backBuffer, font, 7+x*83, 152, "Empty");
		}
	}

	while(1) {
		for(x=0;x<3;x++) {
			SetRect(&r, 4+x*83, 129, 6+x*83+80, 51+130);

			if (x==currentsnap) {
				DrawRect(&r,  RGB15(0,31,31) | 0x8000);
			} else {
				DrawRect(&r,  RGB15(16,0,0) | 0x8000);
			}
		}

		keys_pressed = MyReadKey();

		if ((keys_pressed & KEY_LEFT)==KEY_LEFT) {
			if (currentsnap==0) {
				currentsnap=2;
			} else {
				currentsnap--;
			}
		}
		if ((keys_pressed & KEY_RIGHT)==KEY_RIGHT) {
			if (currentsnap==2) {
				currentsnap=0;
			} else {
				currentsnap++;
			}
		}

		if ((keys_pressed & KEY_A)==KEY_A) {
			break;
		}

	}
#endif
#endif
	return;
}


void addFile(int type, char *fname)
{
	u32 len;

	len=strlen(fname);
	if (len<4) return;

	if ( (!strcasecmp(".urm",&fname[len-4])) || (!strcasecmp(".dsk",&fname[len-4])) || (!strcasecmp(".sna",&fname[len-4])) || (!strcasecmp(".tap",&fname[len-4]))) { 
		LoadROMFile(fname);
	}
}

void SetRect(RECT *R, int left, int top, int right, int bottom)
{
	R->left=left;
	R->top=top;
	R->right=right;
	R->bottom=bottom;
}


void FillRect(RECT *R, u16 color)
{
#ifndef USE_CONSOLE
	int x,y;

	for(y=R->top;y<R->bottom;y++) {
		for(x=R->left;x<R->right;x++) {
			backBuffer[x+y*256]=color;
		}
	}    
#endif	
}

void DrawRect(RECT *R, u16 color)
{
#ifndef USE_CONSOLE
	int x,y;

	for(y=R->top;y<R->bottom;y++) {
		backBuffer[R->left+y*256]=color;
		backBuffer[(R->right-1)+y*256]=color;
	}    
	for(x=R->left;x<R->right;x++) {
		backBuffer[x+R->top*256]=color;
		backBuffer[x+(R->bottom-1)*256]=color;
	}
#endif
}

#ifndef USE_CONSOLE
void DrawLift(RECT *max, RECT *dest, u16 coloron, u16 coloroff)
{
	int sizelift=8;
	RECT *r;
	RECT r0;
	/*	
	if ( (max->right-max->left) > (dest->right-dest->left) ) {
	int x1,x2;

	int dr0,dr,dl,mr,ml;

	dr=dest->right;
	if ( (max->bottom-max->top) > (dest->bottom-dest->top) ) {
	dr0=dest->right-sizelift;
	} else {
	dr0=dr;
	}
	dl=dest->left;
	mr=max->right;
	ml=max->left;

	x1=dl+((dl-ml)*(dr0-dl))/(mr-ml);
	x2=dr0-((mr-dr)*(dr0-dl))/(mr-ml);

	r=&r0;

	SetRect(r, x1, dest->top, x2, dest->top+sizelift);
	FillRect(r, coloroff);

	SetRect(r, x1, dest->top, x2, dest->top+sizelift);
	FillRect(r, coloron);

	SetRect(r, x1, dest->top, x2, dest->top+sizelift);
	FillRect(r, coloroff);		
	}
	*/	

	if ( (max->bottom-max->top) > (dest->bottom-dest->top) ) {
		int y1,y2;

		int dt0,dt,db,mt,mb;

		db=dest->bottom;
		dt=dest->top;

		if ( (max->right-max->left) > (dest->right-dest->left) ) {
			dt0=dest->top+sizelift;
		} else {
			dt0=dt;
		}
		mb=max->bottom;
		mt=max->top;

		y1=dt0+((dt-mt)*(db-dt0))/(mb-mt);
		y2=db-((mb-db)*(db-dt))/(mb-mt);

		//	y2=y1+((db-dt)*(db-dt))/(mb-mt);

		if (y2>mb) {
			y2=mb;  // pas normal... mais ca arrive :(
		}

		r=&r0;

		if (dest->top<y1) {
			SetRect(r, dest->right - sizelift, dest->top, dest->right, y1);
			FillRect(r, coloroff);
		}		    

		SetRect(r, dest->right - sizelift, y1, dest->right, y2);
		FillRect(r, coloron);

		if (dest->bottom>y2) {
			SetRect(r, dest->right - sizelift, y2, dest->right, dest->bottom);
			FillRect(r, coloroff);  
		}
	}

	return;
}
#endif

// -1: restore derniere couleur 
// 0: switch vert
// 1: switch couleur
// 3: switch to inactif

void SetPalette(int color)
{
	static int lastcolour=1;
	int i;

	if (color==-1) {
		color=lastcolour;
		monochrome = (lastcolour==0) ? 1:0;
	}

	if (color==1) {
		for ( i = 0; i < 32; i++ ) {
			int r = ( RgbCPCdef[ i ] >> 19 ) & 0x1F;
			int g = ( RgbCPCdef[ i ] >> 11 ) & 0x1F;
			int b = ( RgbCPCdef[ i ] >> 3 ) & 0x1F;

			BG_PALETTE[i]=RGB15(r,g,b);
			monochrome=0;
		}             
		lastcolour=color;
	}

	if (color==0) {        
		for ( i = 0; i < 32; i++ ) {
			int r = ( RgbCPCdef[ i ] >> 19 ) & 0x1F;
			int g = ( RgbCPCdef[ i ] >> 11 ) & 0x1F;
			int b = ( RgbCPCdef[ i ] >> 3 ) & 0x1F;


			g=(r+g+b)/3;
			b=0;
			r=0;		

			BG_PALETTE[i]=RGB15(r,g,b);	
			monochrome=1;
		}             
		lastcolour=color;
	}

	if (color==3) {

		for ( i = 0; i < 32; i++ ) {
			int z;
			int r = ( RgbCPCdef[ i ] >> 19 ) & 0x1F;
			int g = ( RgbCPCdef[ i ] >> 11 ) & 0x1F;
			int b = ( RgbCPCdef[ i ] >> 3 ) & 0x1F;

			z=(r+g+b)/3;

			BG_PALETTE[i]=RGB15(z,z,z);
		}             
	}
}

void RedefineKey(int key)
{
#ifndef USE_CONSOLE
	int x,y,n;
	dmaCopy(kbdBuffer, backBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2);

	keyEmul=3;

	while(((~IPC->buttons) & (1 << 6))==0);

	x=IPC->touchXpx;
	y=IPC->touchYpx;

	for (n=0;n<NBCPCKEY;n++) {
		if ( (x>=keypos[n].left) && (x<=keypos[n].right) && (y>=keypos[n].top) && (y<=keypos[n].bottom) ) {
			keyown[key]=keymap[n].normal;
			break;
		}
	}
	UpdateKeyMenu();
#endif
}

void UpdateTitlePalette(struct kmenu *current)
{
	/*
	if (lastcolour==1) {
	sprintf(current->title,"Monitor: [COLOR] - Green");
	} else {
	sprintf(current->title,"Monitor: Color - [GREEN]");
	}
	*/
}

// Retour: 1 -> return emulator
//         0 -> return to parent
//         2 -> return to item (switch)

int ExecuteMenu(int n, struct kmenu *current)
{
	switch(n) 
	{
	case ID_SWITCH_MONITOR:
		/*
		if (lastcolour==1) {
		SetPalette(0);
		} else {
		SetPalette(1);
		}
		UpdateTitlePalette(current);
		*/
		return 0;
		break;
	case ID_COLOR_MONITOR:
		SetPalette(1);   
		myprintf("Set Color Monitor");
		return 0;
		break;
	case ID_GREEN_MONITOR:
		SetPalette(0);   
		myprintf("Set Green Monitor");
		return 0;
		break;
	case ID_SCREEN_AUTO:
		resize=1;
		Regs1=0; 
		Regs2=0; 
		Regs6=0;    
		Regs7=0;
		myprintf("Auto-resize");
		return 1;
		break;
	case ID_SCREEN_320:
		resize=2;
		BG3_XDX = 320; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
		BG3_CX = (XStart*4) << 8; 
		memcpy(offsetY, offsetY200,272);
		x0=(XStart*4);
		y0=36;
		maxy=64;
		myprintf("Resize to 320");
		return 0;
		break;
	case ID_SCREEN_NORESIZE:
		resize=3;
		BG3_XDX = 256; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
		BG3_CX = (XStart*4) << 8; 
		memcpy(offsetY, offsetY192,272);
		x0=(XStart*4);
		y0=40;
		maxy=80;
		myprintf("No Resize");
		return 0;
		break;
	case ID_SCREEN_OVERSCAN:
		resize=4;
		BG3_XDX = 384; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
		BG3_CX = 0;
		memcpy(offsetY, offsetY272,272);
		x0=(XStart*4);
		y0=0;
		myprintf("Overscan");
		return 0;
		break;
	case ID_KEY_KEYBOARD:
		keyEmul=2; //  Emul du clavier
		myprintf("Keyboard emulation");
		return 0;
		break;
	case ID_KEY_KEYPAD:
		keyown[0]=CPC_CURSOR_UP;
		keyown[1]=CPC_CURSOR_DOWN;
		keyown[2]=CPC_CURSOR_LEFT;
		keyown[3]=CPC_CURSOR_RIGHT;
		keyown[4]=CPC_RETURN;
		keyown[5]=CPC_SPACE;
		keyown[6]=CPC_SPACE;

		keyEmul=3; //  Emul du clavier fleche
		myprintf("Keypad emulation");
		return 0;
		break;
	case ID_KEY_JOYSTICK:
		keyown[0]=CPC_JOY_UP;
		keyown[1]=CPC_JOY_DOWN;
		keyown[2]=CPC_JOY_LEFT;
		keyown[3]=CPC_JOY_RIGHT;
		keyown[4]=CPC_RETURN;
		keyown[5]=CPC_JOY_FIRE1;
		keyown[6]=CPC_JOY_FIRE2;

		keyEmul=3; //  Emul du joystick
		myprintf("Joystick emulation");
		return 0;
		break;
	case ID_MULTIKEYPRESSED:
		multikeypressed=1;
		myprintf("Multi key pressed");
		break;
	case ID_NOMULTIKEYPRESSED:
		multikeypressed=0;
		myprintf("Single key pressed");
		break;
	case ID_DISPFRAMERATE:
		dispframerate=1;
		myprintf("Display framerate");
		return 0;
		break;
	case ID_NODISPFRAMERATE:
		cpcprint16i(0,192-8, "                                  ", 255);
		dispframerate=0;
		myprintf("Hide framerate");
		return 0;
		break;
	case ID_RESET:
		ResetCPC();
		myprintf("Reset CPC");
		return 0;
		break;
	case ID_SAVESNAP:
		{
			char *buf;
			char snap[256];

			sprintf(snap, "/%s_i.%d", currentfile, currentsnap+1);
			buf=strchr(snap,'.');
			*buf='_';
			SauveScreen(snap);

			sprintf(snap, "/%s.%d", currentfile, currentsnap+1);
			buf=strchr(snap,'.');
			*buf='_';
			SauveSnap(snap);

			myprintf("Snapshot saved");
			return 1;
			break;
		}
	case ID_LOADSNAP:
		LoadSlotSnap(currentfile, currentsnap);
		myprintf("Snapshot loaded");
		break;
	case ID_ERASESNAP:
		DeleteSlotSnap(currentfile, currentsnap);
		myprintf("Snapshot deleted");
		break;
	case ID_FILE:
		if (iconAutoInsert!=-1) {
			LireRom(current,iconAutoInsert,0);	
		} else {
			DiskSelection(current);			
		}
		return 1;
		break;
	case ID_REDEFINE_UP:
		RedefineKey(0);
		return 2;
		break;
	case ID_REDEFINE_DOWN:
		RedefineKey(1);
		return 2;
		break;
	case ID_REDEFINE_LEFT:
		RedefineKey(2);
		return 2;
		break;
	case ID_REDEFINE_RIGHT:
		RedefineKey(3);
		return 2;
		break;
	case ID_REDEFINE_START:
		RedefineKey(4);
		return 2;
		break;
	case ID_REDEFINE_A:
		RedefineKey(5);
		return 2;
		break;
	case ID_REDEFINE_B:
		RedefineKey(6);
		return 2;
		break;
	case ID_REDEFINE_X:
		RedefineKey(7);
		return 2;
		break;
	case ID_REDEFINE_Y:
		RedefineKey(8);
		return 2;
		break;
	case ID_REDEFINE_L:
		RedefineKey(9);
		return 2;
		break;
	case ID_REDEFINE_R:
		RedefineKey(0);
		return 2;
		break;
	case ID_HACK_TABCOUL:
		hack_tabcoul =  (hack_tabcoul==1) ? 0:1;
		current->title[strlen(current->title)-1]= (hack_tabcoul==1) ? 'Y' : 'N';
		return 2;
		break;
	case ID_ACTIVE_MAGNUM:
		usemagnum=1;
		break;
	case ID_SAVESCREEN:
		{
			char *buf;
			char snap[256];
			int n;

			n=1;
			do {
				sprintf(snap, "/%s.%d.snab", currentfile, n);
				buf=strchr(snap,'.');
				*buf='_';
				if (!FileExists(snap)) {
					break;
				}
				n++;
			} while(1);
			SauveSnap(snap);

			myprintf("Save screen");
			return 1;
			break;
		}

		break;
	default:
		break;
	}

	return 1;
}


// retour 1 si on doit revenir a l'emulator
// retour 0 si on doit revenir au parent

int LoadMenu(struct kmenu *parent)
{
	uint16 keys_pressed;
	// RECT r, ralbum;
	struct kmenu *first;

	if (parent->nbr==0) {
		return 1;
	}

	// SetRect(&ralbum,248,4,253,85);   // 4, 110

	do {
		keys_pressed = ~(REG_KEYINPUT);
	} while ((keys_pressed & (KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);

	while(1) {
		int i,n;
		struct kmenu *selected=NULL;
		char *bufpos[7];

		swiWaitForVBlank();
		myconsoleClear();

		first=parent->firstchild;
		i=0;
		n=0;

		memset(bufpos, 0, sizeof(u8*)*7);

		while(1) {
			if ( ((i-parent->pos+3)>=0) & ((i-parent->pos+3)<7) ) {
				bufpos[i-parent->pos+3]=first->title;
			}
			if (i==parent->pos) {
				selected=first;
			}
			i++;
			first=first->next;
			if (first==NULL) {
				break;
			}
		}
#ifndef USE_CONSOLE
		dmaCopy(menubufferlow, backBuffer, 256*192*2);

		{
			u32 n;
			char buffer[256];

			/*
			buf=backBuffer+256*(192-24)/2;
			for(n=0;n<256*20;n++) {
			// *buf=(*buf) & 0xFC1F; // <<1; // AlphaBlendFast(*buf,backcolor);
			// *buf=(*buf) & 0x81E0; // <<1; // AlphaBlendFast(*buf,backcolor);
			*buf=AlphaBlendFast(*buf,0x81E0);
			buf++;
			}
			*/

			for(n=0;n<7;n++) {
				if (bufpos[n]!=NULL) {
					u16 color;
					color = RGB15((31 - abs(3-n)*6), (31 - abs(3-n)*6), (31 - abs(3-n)*6)) | 0x8000;
					DrawTextLeft(backBuffer, filefont, color, 2, (n*20)+26, bufpos[n]);
				}
			}
			sprintf(buffer,"\x03 Select \x04 Back");
			DrawTextCenter(backBuffer, filefont, RGB15(0,0,31) | 0x8000,  176, buffer);
			if (parent!=&root) {
				DrawTextLeft(backBuffer, filefont, RGB15(31,31,0) | 0x8000, 15, 1, parent->title);
			} else {
				DrawTextLeft(backBuffer, filefont, RGB15(31,31,0) | 0x8000, 15, 1, "Root");
			}
		}
#endif


#ifndef USE_CONSOLE 
		// SetRect(&r, ralbum.left, ralbum.top-beg*10, ralbum.right, ralbum.top-beg*10+parent->nbr*10);
		// DrawLift(&r, &ralbum, RGB15(0,10,0)|0x8000, RGB15((136>>3), (158>>3), (145>>3))|0x8000);
#endif

		keys_pressed = MyReadKey();

		if ((keys_pressed & KEY_UP)==KEY_UP) {
			parent->pos--;
			if(parent->pos<0) {
				parent->pos=parent->nbr-1;
				parent->beg=parent->nbr-7;
			}
			while(parent->pos-parent->beg<0) {
				parent->beg--;
			}
		}
		if ((keys_pressed & KEY_DOWN)==KEY_DOWN)  {
			parent->pos++;
			if (parent->pos>=parent->nbr) {
				parent->pos=0;
				parent->beg=0;
			}
			while(parent->pos-parent->beg>=7) {
				parent->beg++;
			}
		}
		if ((keys_pressed & KEY_A)==KEY_A) {
			int retour;
			if (selected->firstchild!=NULL) {
				if (LoadMenu(selected)==1) {
					return 1;
				}
			}          
			retour=ExecuteMenu(selected->id, selected);
			if (retour!=2) {
				return retour;
			}
		}
		if ((keys_pressed & KEY_L)==KEY_L) {
			parent->pos-=8;
			if(parent->pos<0) {
				parent->pos=parent->nbr-1;
				parent->beg=parent->nbr-7;
			}
			while(parent->pos-parent->beg<0) {
				parent->beg--;
			}
		}
		if ((keys_pressed & KEY_R)==KEY_R)  {
			parent->pos+=7;
			if (parent->pos>=parent->nbr) {
				parent->pos=0;
				parent->beg=0;
			}
			while(parent->pos-parent->beg>=7) {
				parent->beg++;
			}
		}

		if ((keys_pressed & KEY_B)==KEY_B) {
			return 0;
		}
	}
}

u16 MyReadKey(void)
{
	u16 keys_pressed, my_keys_pressed;

	do {
		keys_pressed = ~(REG_KEYINPUT);
	} while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))==0);

	my_keys_pressed = keys_pressed;

	// wait until all key are released.
	do {
		keys_pressed = ~(REG_KEYINPUT);
	} while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);

	return my_keys_pressed;
}

void InitCalcPoints( void )
{
	int a, b, c, d, i;

	// Pour le mode 0
	for ( i = 0; i < 256; i++ )
	{
		a = ( i >> 7 )
		+ ( ( i & 0x20 ) >> 3 )
		+ ( ( i & 0x08 ) >> 2 )
		+ ( ( i & 0x02 ) << 2 );
		b = ( ( i & 0x40 ) >> 6 )
		+ ( ( i & 0x10 ) >> 2 )
		+ ( ( i & 0x04 ) >> 1 )
		+ ( ( i & 0x01 ) << 3 );
		TabPointsDef[ 0 ][ i ][ 0 ] = (u8)a;
		TabPointsDef[ 0 ][ i ][ 1 ] = (u8)a;
		TabPointsDef[ 0 ][ i ][ 2 ] = (u8)b;
		TabPointsDef[ 0 ][ i ][ 3 ] = (u8)b;
	}

	// Pour le mode 1
	for ( i = 0; i < 256; i++ )
	{
		a = ( i >> 7 ) + ( ( i & 0x08 ) >> 2 );
		b = ( ( i & 0x40 ) >> 6 ) + ( ( i & 0x04 ) >> 1 );
		c = ( ( i & 0x20 ) >> 5 ) + ( i & 0x02 );
		d = ( ( i & 0x10 ) >> 4 ) + ( ( i & 0x01 ) << 1 );
		TabPointsDef[ 1 ][ i ][ 0 ] = (u8)a;
		TabPointsDef[ 1 ][ i ][ 1 ] = (u8)b;
		TabPointsDef[ 1 ][ i ][ 2 ] = (u8)c;
		TabPointsDef[ 1 ][ i ][ 3 ] = (u8)d;
	}

	// Pour le mode 2
	for ( i = 0; i < 256; i++ )
	{
		TabPointsDef[ 2 ][ i ][ 0 ] = i >> 7;
		TabPointsDef[ 2 ][ i ][ 1 ] = ( i & 0x20 ) >> 5;
		TabPointsDef[ 2 ][ i ][ 2 ] = ( i & 0x08 ) >> 3;
		TabPointsDef[ 2 ][ i ][ 3 ] = ( i & 0x02 ) >> 1;
	}

	// Mode 3 = Mode 0 ???
	for ( i = 0; i < 256; i++ )
	for ( a = 0; a < 4; a++ )
	TabPointsDef[ 3 ][ i ][ a ] = TabPointsDef[ 0 ][ i ][ a ];
}

void CalcPoints( void )
{
	int i;

	if ((lastMode>=0) && (lastMode<=3)) {
		for (i=0;i<256;i++) {
			//    for(j=0;j<4;j++) TabPoints[lastMode][i][j] = TabCoul[ TabPointsDef[lastMode][i][j] ];
			*(u32*)(&TabPoints[lastMode][i][0]) = (TabCoul[ TabPointsDef[lastMode][i][0] ] << 0) + (TabCoul[ TabPointsDef[lastMode][i][1] ] << 8) + (TabCoul[ TabPointsDef[lastMode][i][2] ] << 16) + (TabCoul[ TabPointsDef[lastMode][i][3] ] << 24);
		}
	}
	UpdateInk=0;
}




/********************************************************* !NAME! **************
* Nom : InitPlateforme
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialisation diverses
*
* Résultat    : /
*
* Variables globales modifiées : JoyKey, clav
*
********************************************************** !0! ****************/
void InitPlateforme( void )
{
	myprintf("InitPlateform");

	InitCalcPoints();
	CalcPoints();
	memset( clav, 0xFF, sizeof( clav ) );
	memset( cpckeypressed, 0, sizeof(cpckeypressed));

	inMenu=0;
}


/********************************************************* !NAME! **************
* Nom : Erreur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affichage d'un message d'erreur
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void Erreur( char * Msg )
{
	myprintf("Error: %s", Msg);
}


/********************************************************* !NAME! **************
* Nom : Info
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affichage d'un message d'information
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void Info( char * Msg )
{
	myprintf("Info: %s", Msg);
}

//Bordure: 32 a gauche, 32 a droite (donc, max 384)
//Border: 36 en haut, 36 en bas (donc, max 272)

static int updated=0;


void TraceLigne8B512( int y, signed int AdrLo, int AdrHi )
{
	// if (y>yMax) yMax=y;
	// if (y<yMin) yMin=y;

	y-=y0;

	if (y<0) {
		return;
	}
	y=offsetY[y];
	if (y>=192) {
		return;
	}

	if ((!hack_tabcoul) && (UpdateInk==1)) {    // It's would be beter to put before each lines
		CalcPoints();
	} 

	updated=1;

	u32 *p;

	p = (u32*)MemBitmap;
	p += (y*128);

	if ( AdrLo < 0 ) {
		memset( p, TabCoul[ 16 ], 384);
	} else {
		int x;

		memset( p, TabCoul[ 16 ], XStart*4);
		memset( p+ XEnd, TabCoul[ 16 ], (96-XEnd)*4);

		p += XStart;
		for (x = XStart; x < XEnd; x++ ) { 
			u8 *ad = TabPoints[ lastMode ][ MemCPC[ ( AdrLo++ & 0x7FF ) | AdrHi ] ];

			*p=*(u32*)ad;
			p++;
		}
	}
}

/********************************************************* !NAME! **************
* Nom : UpdateScreen
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affiche l'écran du CPC
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void UpdateScreen( void )
{

	frame++;

	if (resize==1) {       // Auto resize ?
		if ((RegsCRTC[2]!=Regs2) || (RegsCRTC[6]!=Regs6) || (RegsCRTC[1]!=Regs1) || (RegsCRTC[7]!=Regs7)) {
			int n;
			int x1,x2, y1,y2;
			int height;

			x1 = max( ( 50 - RegsCRTC[2] ) << 3, 0 );
			x2 = min( x1 + ( RegsCRTC[1] << 3 ), 384 );

			// y1 = max( (272-(RegsCRTC[6]<<3))>>1, 0);
			y1 = max( ( 35 - RegsCRTC[7] ) << 3, 0);
			y2 = min( y1 + (RegsCRTC[6] << 3), 272);

			BG3_XDX = (x2-x1); 
			BG3_CX = x1 << 8; 

			height=(y2-y1);
			if (height<192) height=192;

			for(n=0;n<272;n++) {
				offsetY[n]=255;
			}

			for(n=0;n<192;n++) {
				int y;
				y=(n*height)/191+y1;
				if ((y>=0) && (y<272)) {
					offsetY[y]=n;
				}
			}

			x0=x1;
			y0=0;
			maxy=0;

			/*
			char chaine[256];
			sprintf(chaine,"x1:%03d x2:%03d y1:%03d y2:%03d", x1,x2,y1,y2);
			cpcprint16(0,0, chaine, RGB15(0,0,0) | 0x8000);

			sprintf(chaine,"%02X %02X %02X %02X", RegsCRTC[6], RegsCRTC[7], RegsCRTC[8], RegsCRTC[9]);
			cpcprint16(0,8, chaine, RGB15(0,0,0) | 0x8000);
			*/

			//myprintf("Resize to %dx%d", x2-x1, y2-y1); // FM->title

			Regs1=RegsCRTC[1];
			Regs2=RegsCRTC[2];
			Regs6=RegsCRTC[6];
			Regs7=RegsCRTC[7];
		}
	}

	if (msgframe>frame-50*3) {
		if (iconmenu==-1) {
			int alpha;
			alpha=(msgframe-(frame-50*3))*4;
			if (alpha>255) alpha=255;

			cpcprint16i(0,40, msgbuf, alpha);
		} else {
			msgframe=frame; 
		}
	}


	if (updated) {	
		// dmaCopy(MemBitmap, frontBuffer, CPC_VISIBLE_SCR_WIDTH * CPC_VISIBLE_SCR_HEIGHT);
		//
		updated=0;

		if (UpdateInk==1) {    // It's would be beter to put before each lines
			CalcPoints();
		} 
	}
}


int nds_GetTicks(void)
{
	return timers2ms(TIMER0_DATA, TIMER1_DATA);
}


// --- MENU STUFF ---
//



void LoopMenu(struct kmenu *parent)
{
	PauseSound();

	menubuffer=(u16*)malloc(256*192*2);

#ifndef USE_CONSOLE
	dmaCopy(backBuffer, menubuffer, 256*192*2);
#endif

	while(1) {
		uint16 keys_pressed = ~(REG_KEYINPUT);
		if ((keys_pressed & KEY_SELECT)!=KEY_SELECT) {
			break;
		}
	}

	// printText(3, 10,10, "Coucou");

	if (inMenu==1) {	
		SetPalette(3);
	} else {
		inMenu=1;
	}


	if (parent==NULL) {
		iconAutoInsert=-1;
		LoadMenu(&root);
	} else {
		LoadMenu(parent);
	}
	SetPalette(-1);


#ifndef USE_CONSOLE
	dmaCopy(menubuffer, backBuffer, 256*192*2);

#ifdef USE_ALTERSCREEN
	int x,y,width,n;

	for(n=0;n<8;n++) {
		if (n<=consolepos) {
			if (consolestring[n*128]==1) {
				width = DrawText(backBuffer, fontred, 110, 5+n*10, consolestring+n*128+1);
			} else {
				width = DrawText(backBuffer, font, 110, n*10+5, consolestring+n*128);
			}
		} else {
			width = 0;
		}
		for(y=5+n*10;y<5+(n+1)*10;y++) {
			for(x=110+width;x<253;x++) {
				backBuffer[x+y*256]=RGB15((156>>3), (178>>3), (165>>3))|0x8000;  // 156, 178, 165
			}
		}
	}
#endif
#endif

	free(menubuffer);

	DispIcons();	

	inMenu=0;

	if (dispframerate==0) {
		cpcprint16i(0,192-8, "                                  ", 255);
	}

	PlaySound();
}

int shifted=0;
int ctrled=0;


void Dispkey(CPC_KEY n, int status);
void DispScanCode(CPC_SCANCODE n, int status);
void PressKey(CPC_KEY n);

void PressKey(CPC_KEY n)
{
	CPC_SCANCODE cpc_scancode;
	cpc_scancode=keymap[n].normal;

	Dispkey(n, 1);

	if ((shifted) && (cpc_scancode!=CPC_SHIFT)) {
		DispScanCode(CPC_SHIFT, 0 | 16);
		shifted=0;
		clav[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
	}
	if ((ctrled) && (cpc_scancode!=CPC_CONTROL)) {
		DispScanCode(CPC_CONTROL, 0 | 16);
		ctrled=0;
		clav[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
	}

	clav[(byte)cpc_scancode >> 3] &= ~bit_values[(byte)cpc_scancode & 7];

	switch(cpc_scancode) 
	{
	case CPC_CAPS_LOCK:
		if (capslock) {
			DispScanCode(cpc_scancode, 0 | 16);
			capslock=0;
		} else {
			DispScanCode(cpc_scancode, 1 | 16);
			capslock=1;
		}
		break;
	case CPC_SHIFT:
		if (shifted) {
			DispScanCode(cpc_scancode, 0 | 16);
			shifted=0;
		} else {
			DispScanCode(cpc_scancode, 1 | 16);
			shifted=1;
		}
		break;
	case CPC_CONTROL:
		if (ctrled) {
			DispScanCode(cpc_scancode, 0 | 16);
			ctrled=0;
		} else {
			DispScanCode(cpc_scancode, 1 | 16);
			ctrled=1;
		}
		break;
	default:
		break;
	}
}

void CPC_ClearScanCode(CPC_SCANCODE cpc_scancode)		// Juste pour autotype
{
	clav[(byte)cpc_scancode >> 3] |= bit_values[(byte)cpc_scancode & 7];
	DispScanCode(cpc_scancode, 0);
}

void CPC_SetScanCode(CPC_SCANCODE cpc_scancode)
{
	clav[(byte)cpc_scancode >> 3] &= ~bit_values[(byte)cpc_scancode & 7];
	DispScanCode(cpc_scancode, 1);
}

void DispScanCode(CPC_SCANCODE scancode, int status)
{
	int n;

	for(n=0;n<NBCPCKEY;n++) {
		if (keymap[n].normal == scancode)  {
			Dispkey(n, status);
		}
	}    
}

// 1: active
// 2: on
// 0: off

char statuskey[NBCPCKEY];

void Dispkey(CPC_KEY n, int status)
{
#ifndef USE_CONSOLE    
	int x,y;
	u16 color;
	
	if ((status&16)!=16) {
		if ((keymap[n].normal==CPC_SHIFT) || (keymap[n].normal==CPC_CONTROL)  || (keymap[n].normal==CPC_CAPS_LOCK)) {
			return;
		}
	} 

	switch(status) 
	{
	case 0:
	case 16:
		for(y=keypos[n].top;y<keypos[n].bottom;y++) {
			for(x=keypos[n].left;x<keypos[n].right;x++) {
				backBuffer[x+y*256]=kbdBuffer[x+y*256];
			}
		}
		statuskey[n]=0;
		break;
	case 17:		
	case 1:
		cpckeypressed[n]=2;	
		if (statuskey[n]==1) break;
		color=RGB15(15,0,0);
		for(y=keypos[n].top;y<keypos[n].bottom;y++) {
			for(x=keypos[n].left;x<keypos[n].right;x++) {
				backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
			}
		}
		statuskey[n]=1;
		break;
	case 2:
	case 18:
		if (statuskey[n]==2) break;
		color=RGB15(0,15,0);
		for(y=keypos[n].top;y<keypos[n].bottom;y++) {
			for(x=keypos[n].left;x<keypos[n].right;x++) {
				backBuffer[x+y*256]=~kbdBuffer[x+y*256]|0x8000;
				// backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
			}
		}
		statuskey[n]=2;
		break;
	}
#endif
}

void DispDisk(int reading)
{
#ifndef USE_CONSOLE
	if (Image==1) {
		int x,y;
		RECT r;
		u16 color;

		SetRect(&r, 0, 0, 32, 32);

		switch(reading) 
		{
		case 0:
			for(y=r.top;y<r.bottom;y++) {
				for(x=r.left;x<r.right;x++) {
					backBuffer[(256-32)+x+y*256]=icons[x+y*224];
				}
			}
			break;
		case 1:
			color=RGB15(15,0,0);
			for(y=r.top;y<r.bottom;y++) {
				for(x=r.left;x<r.right;x++) {
					backBuffer[(256-32)+x+y*256]=icons[x+(y+32*3)*224];
				}
			}
			break;
		}   
	}
#endif
}

void DispIcons(void)
{
#ifndef USE_CONSOLE
	int i;

	if (iconmenu!=-1) {
		cpcprint16i(0,40, "     ", 255);
	}


	for(i=0;i<7;i++) {
		int x,y,z,cur,j;
		u16 transpcolor;
		int nbr=0;

		transpcolor = 64543; // RGB15(31,0,31);
		z=0;
		switch(i) {
		case 0:
			z=0; 
			nbr=3;
			break;
		case 1:
			z= (monochrome==1) ? 0 : 1;
			nbr=2;
			break;
		case 2:
			if (resize==1) {
				z=2;
			} else if (resize==2) {
				z=3;
			} else if (resize==4) {
				z=0;
			} else if (resize==3) {
				z=1;
			}
			nbr=4;
			break;
		case 3:
			if (keyEmul==2) {
				z=0;
			} else if (keyEmul==3) {
				if (keyown[0]==CPC_JOY_UP) {
					z=1;
				} else if (keyown[0]==CPC_CURSOR_UP) {
					z=2;
				}
			}
			nbr=3;
			break;
		case 4:
			z= dispframerate ? 0 : 1;
			nbr=2;
			break;
		case 5:
			z= (multikeypressed==1) ? 1 : 0;
			nbr=2;
			break;
		case 6:
			z=0;
			if (HaveSlotSnap(currentfile, currentsnap)) {
				nbr=4;
			} else {
				nbr=2;
			}
			break;
		}

		int dispicon;

		if (i==0) {
			dispicon = (Image==0) ? 4 : 0;
		} else {
			dispicon = z;
		}

		for(y=0;y<32;y++) {
			for(x=0;x<32;x++) {
				u16 car;
				car=icons[(x+i*32)+(y+dispicon*32)*224];
				if (car!=transpcolor) {
					backBuffer[(224+x)-(i*32)+y*256]=car;
				} 
			}
		}
		if (iconmenu==i) {
			zicon=z;
			nbricon=nbr;

			cur=0;
			for(j=1;j<nbr;j++) {
				if (cur==z) cur++;

				for(y=0;y<32;y++) {
					for(x=0;x<32;x++) {
						u16 car;
						car=icons[(x+i*32)+(y+cur*32)*224];
						if (car!=transpcolor) {
							backBuffer[(224+x)-(i*32)+(y+j*32)*256]=car;
						} 
					}
				}
				cur++;
			}
		} else {
			for(j=1;j<4;j++) {		// MAX 4
				for(y=0;y<32;y++) {
					for(x=0;x<32;x++) {
						u16 car;
						car=kbdBuffer[(224+x)-(i*32)+(y+j*32)*256];
						backBuffer[(224+x)-(i*32)+(y+j*32)*256]=car;
					}
				}
			}
		}
	}
#endif 
}

int nds_ReadKey(void)
{
	if (AutoType_Active()) {
		AutoType_Update();
	} else {
		uint16 keys_pressed;
		static uint16 oldkey;
		int n;

		scanKeys();
		keys_pressed = keysHeld();
		
		if (!multikeypressed) {
			memset( clav, 0xFF, sizeof( clav ) );
		}

		if (SCR_TOUCH) {			// styluspressed
			int x,y,n;

			x=IPC->touchXpx;
			y=IPC->touchYpx;

			if (!usemagnum) {
				if (styluspressed==0) {

					if ((x>0) & (x<32) & (y>=0) & (y<=24)) {
						ExecuteMenu(ID_SAVESCREEN, NULL);
					}

					if ((x>0) & (x<32) & (y>=25) & (y<=36)) {
						ExecuteMenu(ID_RESET, NULL);
					}

					if ((x>=32) && (y>=0) && (y<32)) {
						int z;

						z=(256-x)/32;
						if (z==iconmenu) {
							iconmenu=-1;
						} else {
							iconmenu=z;
							msgbuf[0]=0;
						}
						DispIcons();
					}

					if (iconmenu!=-1) {
						int zx,zy;

						zx=(256-x)/32;

						if ((zx==iconmenu) && (y>=0) && (y<32*nbricon)) {
							zy=y/32;

							if (y>=32) {
								if (zy<=zicon) {
									zy--;
								} 

								switch(zx) 
								{
								case 0:
									if (zy==1) {
										iconAutoInsert=1;
										inMenu=1;
										forceMenu=menuRomId;										
										// Auto insert
									} else if (zy==2) {
										iconAutoInsert=0;
										inMenu=1;
										forceMenu=menuRomId;										
										// Load
									}
									break;
								case 1:
									if (zy==0) {
										ExecuteMenu(ID_GREEN_MONITOR, NULL);
									} else if (zy==1) {
										ExecuteMenu(ID_COLOR_MONITOR, NULL);
									}
									break;
								case 2:
									if (zy==0) {
										ExecuteMenu(ID_SCREEN_OVERSCAN, NULL);
									} else if (zy==1) {
										ExecuteMenu(ID_SCREEN_NORESIZE, NULL);
									}  else if (zy==2) {
										ExecuteMenu(ID_SCREEN_AUTO, NULL);
									}  else if (zy==3) {
										ExecuteMenu(ID_SCREEN_320, NULL);
									}
									break;
								case 3:
									if (zy==0) {
										ExecuteMenu(ID_KEY_KEYBOARD, NULL);
									} else if (zy==1) {
										ExecuteMenu(ID_KEY_JOYSTICK, NULL);
									} else if (zy==2) {
										ExecuteMenu(ID_KEY_KEYPAD, NULL);
									}
									break;
								case 4:
									if (zy==0) {
										ExecuteMenu(ID_DISPFRAMERATE, NULL);
									} else if (zy==1) {
										ExecuteMenu(ID_NODISPFRAMERATE, NULL);
									}
									break;
								case 5:
									if (zy==0) {
										ExecuteMenu(ID_NOMULTIKEYPRESSED, NULL);
									} else {
										ExecuteMenu(ID_MULTIKEYPRESSED, NULL);
									}
									break;
								case 6:
									if (zy==1) {
										ExecuteMenu(ID_SAVESNAP, NULL);
									} else if (zy==2) {
										ExecuteMenu(ID_LOADSNAP, NULL);
									} else if (zy==3) {
										ExecuteMenu(ID_ERASESNAP, NULL);
									}	
									break;
								}
								iconmenu=-2;
								DispIcons();
							}
						}
					}
				}	// Fin de if styluspressed=0 (avant ou apres la condition if (iconmenu ??)
				
					if (iconmenu==-1) {
						for (n=0;n<NBCPCKEY;n++) {
							if ( (x>=keypos[n].left) && (x<=keypos[n].right) && (y>=keypos[n].top) && (y<=keypos[n].bottom) ) {
								if ( ((keymap[n].normal!=CPC_SHIFT) && (keymap[n].normal!=CPC_CONTROL)  && (keymap[n].normal!=CPC_CAPS_LOCK)) | (styluspressed==0)) {
									PressKey(n);
									}
								break;
							}
						}
					} 
				
			}
			if (usemagnum) {

			}
			styluspressed=1;
		} else {
			if (iconmenu==-2) {
				iconmenu=-1;
			}
			styluspressed=0;
		}

		if ((keys_pressed & KEY_L)==KEY_L) {
            ExecuteMenu(ID_SAVESCREEN, NULL);
		}

		if ((keys_pressed & KEY_R)==KEY_R) {
			if ((keys_pressed & KEY_UP)==KEY_UP) {
				if (y0>0) y0--;
			}

			if ((keys_pressed & KEY_DOWN)==KEY_DOWN) {
				if (y0<maxy) y0++;
			}

			if ((keys_pressed & KEY_LEFT)==KEY_LEFT) {
				if (x0>0) x0--;
				BG3_CX=x0<<8;
			}

			if ((keys_pressed & KEY_RIGHT)==KEY_RIGHT) {
				int maxx;
				maxx=384-BG3_XDX;
				if (x0<maxx) x0++;
				BG3_CX=x0<<8;
			}

			if ((keys_pressed & KEY_A)==KEY_A) {
				BG3_CX=(XStart*4)<<8;
			}

		} else {

			if (keyEmul==3) {
				if ((keys_pressed & KEY_UP)==KEY_UP)
				CPC_SetScanCode(keyown[0]);

				if ((keys_pressed & KEY_DOWN)==KEY_DOWN)
				CPC_SetScanCode(keyown[1]);

				if ((keys_pressed & KEY_LEFT)==KEY_LEFT)
				CPC_SetScanCode(keyown[2]);

				if ((keys_pressed & KEY_RIGHT)==KEY_RIGHT)
				CPC_SetScanCode(keyown[3]);

				if ((keys_pressed & KEY_START)==KEY_START)
				CPC_SetScanCode(keyown[4]);

				if ((keys_pressed & KEY_A)==KEY_A) 
				CPC_SetScanCode(keyown[5]);

				if ((keys_pressed & KEY_B)==KEY_B) 
				CPC_SetScanCode(keyown[6]);

				if ((keys_pressed & KEY_X)==KEY_X) 
				CPC_SetScanCode(keyown[7]);

				if ((keys_pressed & KEY_Y)==KEY_Y) 
				CPC_SetScanCode(keyown[8]);
			}


			if (keyEmul==2) {
				int x,y;
				static CPC_KEY cur=0;

				Dispkey(cur,0);

				x=keypos[cur].left;
				y=keypos[cur].top;

				if ( ((keys_pressed & KEY_UP)==KEY_UP) && ((oldkey & KEY_UP)==0)) {
					while(1) {
						cur--;
						if (cur<0) {
							cur=NBCPCKEY-1;
						}
						if ((keypos[cur].left>=x) && (cur-1>0) && (keypos[cur-1].left<=x)) {
							break;
						}
					}
				}
				if ( ((keys_pressed & KEY_DOWN)==KEY_DOWN) && ((oldkey & KEY_DOWN)==0)) {
					while(1) {
						cur++;
						if (cur==NBCPCKEY) {
							cur=0;
						}
						if ((keypos[cur].left<=x) && (cur+1<NBCPCKEY) && (keypos[cur+1].left>=x)) {
							break;
						}
					}
				}
				if ( ((keys_pressed & KEY_LEFT)==KEY_LEFT) && ((oldkey & KEY_LEFT)==0)) {
					if (cur>0) {
						cur--;
					}
				}
				if ( ((keys_pressed & KEY_RIGHT)==KEY_RIGHT) && ((oldkey & KEY_RIGHT)==0)) {
					if (cur+1<NBCPCKEY) {
						cur++;
					}
				}

				if ((keys_pressed & KEY_A)==KEY_A) {
					PressKey(cur);
				}

				Dispkey(cur,1);
			}	

			if ( ((keys_pressed & KEY_SELECT)==KEY_SELECT) && ((oldkey & KEY_SELECT)==0)) {
				inMenu=1;
				forceMenu=NULL;
			}
		}

		if (!multikeypressed) {
			for(n=0;n<NBCPCKEY;n++) {
				if (cpckeypressed[n]>0) {
					cpckeypressed[n]--;
					if(cpckeypressed[n]==0) {
						if ((keymap[n].normal!=CPC_SHIFT) && (keymap[n].normal!=CPC_CONTROL)  && (keymap[n].normal!=CPC_CAPS_LOCK)) {
							Dispkey(n, 0);
						}
					}
				}
			}	        
		}
	
		oldkey = keys_pressed;
	}

	return 0;
}

void videoinit(void)
{
}

void UpdateKeyMenu(void)
{
	struct kmenu *first;

	// keyMenu

	first=keyMenu->firstchild;

	while(first!=NULL) {
		switch(first->id) 
		{
		case ID_REDEFINE_UP:
			sprintf(first->title, "Up: %s", keyname0[keyown[0]]);
			break;
		case ID_REDEFINE_DOWN:
			sprintf(first->title, "Down: %s", keyname0[keyown[1]]);
			break;
		case ID_REDEFINE_LEFT:
			sprintf(first->title, "Left: %s", keyname0[keyown[2]]);
			break;
		case ID_REDEFINE_RIGHT:
			sprintf(first->title, "Right: %s", keyname0[keyown[3]]);
			break;
		case ID_REDEFINE_START:
			sprintf(first->title, "Start: %s", keyname0[keyown[4]]);
			break;
		case ID_REDEFINE_A:
			sprintf(first->title, "A: %s", keyname0[keyown[5]]);
			break;
		case ID_REDEFINE_B:
			sprintf(first->title, "B: %s", keyname0[keyown[6]]);
			break;
		case ID_REDEFINE_X:
			sprintf(first->title, "X: %s", keyname0[keyown[7]]);
			break;
		case ID_REDEFINE_Y:
			sprintf(first->title, "Y: %s", keyname0[keyown[8]]);
			break;
		case ID_REDEFINE_L:
			sprintf(first->title, "L: %s", keyname0[keyown[9]]);
			break;
		case ID_REDEFINE_R:
			sprintf(first->title, "R: %s", keyname0[keyown[10]]);
			break;
		}

		first=first->next;
	}
}

// Video Management
volatile u16 vusCptVBL; 

void irqVBlank(void) { 
	// Manage time
	vusCptVBL++;
}


void nds_init(void)
{
	IPC2->soundCommand = 0;
	powerON(POWER_ALL_2D);

	irqInit();                    // IRQ basic setup
	irqSet(IRQ_VBLANK, irqVBlank);
	irqEnable(IRQ_VBLANK);

	//	irqSet(IRQ_VBLANK, 0);

	//	 wifiInit();
	//   wifiTest();

	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000); // Pourquoi faire ?
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_LCD);

	BG3_CR = BG_BMP8_512x256 | BG_BMP_BASE(0) | BG_PRIORITY(3);
	BG3_XDX = 320; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
	BG3_XDY = 0;
	BG3_YDX = 0;
	BG3_YDY = 256; // Taille d'affichage360 - 108; // 360 = DISPLAY_X
	BG3_CX = 0<<8;
	BG3_CY = 0<<8; // 32<<8;
	frontBuffer = (u8*)BG_BMP_RAM(0);

	MemBitmap = frontBuffer;
	// MemBitmap=MyAlloc(256*(192+1),"MemBitmap"); // (192+1) for overflow

#ifndef USE_CONSOLE
	SUB_BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(3);
	SUB_BG3_XDX = 256;
	SUB_BG3_XDY = 0;
	SUB_BG3_YDX = 0;
	SUB_BG3_YDY = 256;
	SUB_BG3_CX = 0;
	SUB_BG3_CY = 0;
	backBuffer=(u16*)BG_BMP_RAM_SUB(0);

	SUB_BG0_CR = BG_256_COLOR | BG_TILE_BASE(0) | BG_MAP_BASE(20) | BG_PRIORITY(0);
#else
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankC(VRAM_C_SUB_BG);
	SUB_BG0_CR = BG_MAP_BASE(31);
	BG_PALETTE_SUB[255] = RGB15(31,31,31);
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
#endif


	TIMER0_DATA=0;
	TIMER1_DATA=0;
	TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024; 
	TIMER1_CR=TIMER_ENABLE|TIMER_CASCADE;


#ifndef USE_CONSOLE    
	// Chargement du clavier   

	filefont = CreateFont((u8*)filefont_gif, filefont_gif_size);
	font = CreateFont((u8*)smallfont_gif, smallfont_gif_size);
	fontred = CreateFont((u8*)smallfont_red_gif, smallfont_red_gif_size);

	icons=(u16*)malloc(224*160*2);
	ReadBackgroundGif(icons, (u8*)icons_gif, icons_gif_size);

	filesel=(u16*)malloc(256*192*2);
	ReadBackgroundGif(filesel, (u8*)filesel_gif, filesel_gif_size);

	menubufferlow=(u16*)malloc(256*192*2);
	ReadBackgroundGif(menubufferlow, (u8*)background_menu_gif, background_menu_gif_size);    

#ifndef BEEK
	kbdBuffer = (u16*)MyAlloc(SCREEN_WIDTH*SCREEN_HEIGHT*2, "Keyboard"); 
	ReadBackgroundGif(kbdBuffer, (u8*)background_gif, background_gif_size);    
#else
	sImage pcx;       

	loadPCX((u8*)keyboard_pcx, &pcx);
	image8to16(&pcx);     
	dmaCopy(pcx.data16, kbdBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
	imageDestroy(&pcx);
#endif

	// Affichage du clavier

	dmaCopy(kbdBuffer, backBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
#endif


	/*
	// Neopsring
	{
	u8 *gif;
	int n;
	int oldbg3xdx;

	oldbg3xdx=BG3_XDX;

	BG3_XDX = 256;

	gif=(u8*)malloc(256*192);
	ReadBackgroundGif8(gif, (u8*)neospring2007_gif, neospring2007_gif_size);
	for(n=0;n<192;n++) {
	memcpy(frontBuffer + n*512, gif + n*256, 256);
	}
	free(gif);

	MyReadKey();

	memset(frontBuffer, 0, 256*192*2);

	BG3_XDX = oldbg3xdx;
	}
	*/

	/*
	{
	u8 *gif;
	int n;
	int oldbg3xdx;

	oldbg3xdx=BG3_XDX;

	BG3_XDX = 256;

	gif=(u8*)malloc(256*192);
	ReadBackgroundGif8(gif, (u8*)splash1_gif, splash1_gif_size);
	for(n=0;n<192;n++) {
	memcpy(frontBuffer + n*512, gif + n*256, 256);
	}
	free(gif);

	MyReadKey();

	memset(frontBuffer, 0, 256*192*2);

	BG3_XDX = oldbg3xdx;
	}
	*/

#ifdef USE_SPLASH1
	ReadBackgroundGif8(MemBitmap, (u8*)splash1_gif, splash1_gif_size);    
	dmaCopy(MemBitmap, frontBuffer, 256*192);
	MyReadKey();
#endif

#ifdef USE_SPLASH2
	ReadBackgroundGif8(MemBitmap, (u8*)splash2_gif, splash2_gif_size);    
	dmaCopy(MemBitmap, frontBuffer, 256*192);
	MyReadKey();
#endif


	myprintf("Trying to init FAT...");
	FS_Init();


	myprintf("Creating menu...");
	Fmnbr=0;	

	myprintf("Reset 8912");
	Reset8912();

	struct kmenu *id;

	root.nbr=0;

	menuRomId=AddMenu(&root, "Disk", ID_MENU);
	FS_getFileList(addFile);

	id=AddMenu(&root, "Switch monitor", ID_MONITOR_MENU);
	AddMenu(id, "Color Monitor", ID_COLOR_MONITOR);
	AddMenu(id, "Green Monitor", ID_GREEN_MONITOR);

	id=AddMenu(&root, "Resize", ID_SCREEN_MENU);
	AddMenu(id, "Auto", ID_SCREEN_AUTO);
	AddMenu(id, "320x200", ID_SCREEN_320);
	AddMenu(id, "No resize", ID_SCREEN_NORESIZE);
	AddMenu(id, "Overscan", ID_SCREEN_OVERSCAN);

	id=AddMenu(&root, "Pad emulation", ID_KEY_MENU);
	AddMenu(id, "Keyboard Emulation", ID_KEY_KEYBOARD);
	AddMenu(id, "Set to Joystick", ID_KEY_JOYSTICK);
	AddMenu(id, "Set to Keypad", ID_KEY_KEYPAD);
	keyMenu=AddMenu(id, "Redefine Keys", ID_MENU);
	AddMenu(keyMenu, "Up: XXXXXXXXXX", ID_REDEFINE_UP);
	AddMenu(keyMenu, "Down: XXXXXXXXXX", ID_REDEFINE_DOWN);
	AddMenu(keyMenu, "Left: XXXXXXXXXX", ID_REDEFINE_LEFT);
	AddMenu(keyMenu, "Right: XXXXXXXXXX", ID_REDEFINE_RIGHT);
	AddMenu(keyMenu, "A: XXXXXXXXXX", ID_REDEFINE_A);
	AddMenu(keyMenu, "B: XXXXXXXXXX", ID_REDEFINE_B);
	AddMenu(keyMenu, "X: XXXXXXXXXX", ID_REDEFINE_X);
	AddMenu(keyMenu, "Y: XXXXXXXXXX", ID_REDEFINE_Y);
	AddMenu(keyMenu, "START: XXXXXXXXXX", ID_REDEFINE_START);
	// AddMenu(keyMenu, "L: XXXXXXXXXX", ID_REDEFINE_L);
	// AddMenu(keyMenu, "R: XXXXXXXXXX", ID_REDEFINE_R);

	id=AddMenu(&root, "Debug", ID_MENU);
	AddMenu(id, "Display framerate", ID_DISPFRAMERATE);
	AddMenu(id, "Don't display framerate", ID_NODISPFRAMERATE);

	id=AddMenu(&root,"Hack", ID_MENU);
	AddMenu(id, "Only one ink refresh per frame: N", ID_HACK_TABCOUL);

	AddMenu(&root, "Reset CPC", ID_RESET);

#ifdef USE_SAVESNAP
	AddMenu(&root, "Save State", ID_SAVESNAP);
#endif

	ExecuteMenu(ID_SCREEN_AUTO, NULL);
	ExecuteMenu(ID_KEY_JOYSTICK, NULL);
	ExecuteMenu(ID_DISPFRAMERATE, NULL);
	ExecuteMenu(ID_COLOR_MONITOR, NULL);

	UpdateKeyMenu();

#ifdef USE_Z80_ORIG 
	ExecInstZ80 = ExecInstZ80_orig;
	ResetZ80 = ResetZ80_orig;
	SetIRQZ80 = SetIRQZ80_orig;
#endif

#ifdef USE_Z80_ASM
	ExecInstZ80= ExecInstZ80_asm;
	ResetZ80 = ResetZ80_asm;
	SetIRQZ80 = SetIRQZ80_asm;
#endif

#ifdef USE_Z80_ASM2
	ExecInstZ80= ExecInstZ80_asm2;
	ResetZ80 = ResetZ80_asm2;
	SetIRQZ80 = SetIRQZ80_asm2;
#endif

	DispIcons();

	strcpy(currentfile,"basic");	
}

void Autoexec(void)
{
	if (Fmnbr==0) {
		myprintf("No ROMs found");
		//		SetPalette(-1);
		return;
	}
	if (Fmnbr==1) {
		LireRom(FirstROM,1,0);
	} else {
		//		LoopMenu(menuRomId);
	}
	//	LireRom(FirstROM,1,0);
}

int nds_video_unlock(void)
{
	return 1; // OK
}

int nds_video_lock(void)
{
	return 1; // OK
}

void nds_video_close(void)
{
}

int nds_MapRGB(int r, int g, int b)
{
	return RGB15(r,g,b);
}


void myconsoleClear(void)
{
	memset(consolestring,0,1024);
	consolepos=0;
}


void myprintf0(const char *fmt, ...) 
{
	char tmp[512];

	va_list args;

	va_start(args, fmt);
	vsprintf(tmp,fmt,args);
	va_end(args);

	if (tmp[0]=='\n') {
		consolepos++;
		if (consolepos==8) {
			memcpy(consolestring,consolestring+128,1024-128);
			consolepos=7;
		}
	}

	memcpy(consolestring+consolepos*128, tmp, 128);
	consolestring[consolepos*128-1]=0;
}

void myprintf(const char *fmt, ...) 
{
	char tmp[512];
	int n;

	va_list args;

	va_start(args, fmt);
	vsprintf(tmp,fmt,args);
	va_end(args);

	strncpy(msgbuf, tmp, 32);
	msgbuf[32]=0;
	msgframe=frame;
	for(n=strlen(msgbuf);n<32;n++) {
		msgbuf[n]=' ';
	}


#ifdef USE_CONSOLE
	iprintf("%s\n", tmp);
	for(n=0;n<20;n++) swiWaitForVBlank();
#else

	if (backBuffer==NULL) {
		return;
	} else {
#ifdef USE_ALTERSCREEN
		int n;
		int width;
		int x,y;

		memcpy(consolestring+consolepos*128, tmp, 128);
		consolestring[consolepos*128-1]=0;

		if (!inMenu) {
			for(n=0;n<8;n++) {
				if (n<=consolepos) {
					if (consolestring[n*128]==1) {
						width = DrawText(backBuffer, fontred, 110, 5+n*10, consolestring+n*128+1);
					} else {
						width = DrawText(backBuffer, font, 110, n*10+5, consolestring+n*128);
					}
				} else {
					width = 0;
				}
				for(y=5+n*10;y<5+(n+1)*10;y++) {
					for(x=110+width;x<253;x++) {
						backBuffer[x+y*256]=RGB15((156>>3), (178>>3), (165>>3))|0x8000;  // 156, 178, 165
					}
				}
			}
		}
#endif

		consolepos++;
		if (consolepos==8) {
			memcpy(consolestring,consolestring+128,1024-128);
			consolepos=7;
		}
		// for(n=0;n<20;n++) swiWaitForVBlank();
	}
#endif
}

void ResetCPC(void)
{
	capslock=0;
	DispScanCode(CPC_CAPS_LOCK, 0 | 16);
	Keyboard_Reset();
	WriteVGA( 0x89 );
	ResetZ80(); 
	ResetCRTC();
	
	Reset8912();
}

void cpcprint(int x, int y, char *pchStr, u8 bColor)
{
	int iLen, iIdx, iRow, iCol;
	u8 bRow;
	u8 *pdwAddr;
	int n;

	pdwAddr = (u8*)MemBitmap + (y*256) + x;   

	iLen = strlen(pchStr); // number of characters to process
	for (n = 0; n < iLen; n++) {
		u8 *pdwLine;
		iIdx = (int)pchStr[n]; // get the ASCII value
		if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
			iIdx = FNT_BAD_CHAR;
		}
		iIdx -= FNT_MIN_CHAR; // zero base the index
		pdwLine = pdwAddr; // keep a reference to the current screen position
		for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
			u32 *pdPixel;

			pdPixel = (u32 *)pdwLine;
			bRow = bFont[iIdx]; // get the bitmap information for one row
			for (iCol = 0; iCol < 2; iCol++) { // loop for all columns in the font character
				*pdPixel = ((bRow & 0x80) ? (bColor<<0) : 0) +
				((bRow & 0x40) ? (bColor<<8) : 0) +
				((bRow & 0x20) ? (bColor<<16) : 0) +
				((bRow & 0x10) ? (bColor<<24) : 0);

				pdPixel++; // update the screen position
				bRow <<= 4; // advance to the next bit
			}
			pdwLine += 256;
			iIdx += FNT_CHARS; // advance to next row in font data
		}
		pdwAddr += FNT_CHAR_WIDTH; // set screen address to next character position
	}
}

void cpcprint16(int x, int y, char *pchStr, u16 bColor)
{
#ifndef USE_CONSOLE
	int iLen, iIdx, iRow, iCol;
	u8 bRow;
	u16 *pdwAddr, *pdwBkg;
	int n;

	pdwAddr = backBuffer + (y*256) + x;   
	pdwBkg = kbdBuffer + (y*256) + x;

	iLen = strlen(pchStr); // number of characters to process
	for (n = 0; n < iLen; n++) {
		u16 *pdwLine, *pdwBkgLine;
		iIdx = (int)pchStr[n]; // get the ASCII value
		if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
			iIdx = FNT_BAD_CHAR;
		}
		iIdx -= FNT_MIN_CHAR; // zero base the index
		pdwLine = pdwAddr; // keep a reference to the current screen position
		pdwBkgLine = pdwBkg; // keep a reference to the current screen position
		for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
			u32 *pdPixel;
			u16 *pwPixelBkg;

			pwPixelBkg=pdwBkgLine;
			pdPixel = (u32 *)pdwLine;

			bRow = bFont[iIdx]; // get the bitmap information for one row
			for (iCol = 0; iCol < 4; iCol++) { // loop for all columns in the font character
				*pdPixel = ((bRow & 0x80) ? (bColor<<0) : pwPixelBkg[0]) +
				((bRow & 0x40) ? (bColor<<16) : (pwPixelBkg[1]<<16));

				pdPixel++; // update the screen position
				pwPixelBkg+=2;
				bRow <<= 2; // advance to the next bit
			}
			pdwLine += 256;
			pdwBkgLine += 256;
			iIdx += FNT_CHARS; // advance to next row in font data
		}
		pdwAddr += FNT_CHAR_WIDTH; // set screen address to next character position
		pdwBkg += FNT_CHAR_WIDTH;
	}
#endif
}

void cpcprint16i(int x, int y, char *pchStr, int alpha)
{
#ifndef USE_CONSOLE
	int iLen, iIdx, iRow, iCol;
	u8 bRow;
	u16 *pdwAddr, *pdwBkg;
	int n;

	pdwAddr = backBuffer + (y*256) + x;   
	pdwBkg = kbdBuffer + (y*256) + x;

	iLen = strlen(pchStr); // number of characters to process
	for (n = 0; n < iLen; n++) {
		u16 *pdwLine, *pdwBkgLine;
		iIdx = (int)pchStr[n]; // get the ASCII value
		if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
			iIdx = FNT_BAD_CHAR;
		}
		iIdx -= FNT_MIN_CHAR; // zero base the index
		pdwLine = pdwAddr; // keep a reference to the current screen position
		pdwBkgLine = pdwBkg; // keep a reference to the current screen position
		for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
			u32 *pdPixel;
			u16 *pwPixelBkg;

			pwPixelBkg=pdwBkgLine;
			pdPixel = (u32 *)pdwLine;

			bRow = bFont[iIdx]; // get the bitmap information for one row
			for (iCol = 0; iCol < 4; iCol++) { // loop for all columns in the font character
				*pdPixel = ((bRow & 0x80) ? (AlphaBlend(~pwPixelBkg[0], pwPixelBkg[0], alpha) <<0) : pwPixelBkg[0]) +
				((bRow & 0x40) ? (AlphaBlend(~pwPixelBkg[1], pwPixelBkg[1], alpha) <<16) : (pwPixelBkg[1]<<16));

				pdPixel++; // update the screen position
				pwPixelBkg+=2;
				bRow <<= 2; // advance to the next bit
			}
			pdwLine += 256;
			pdwBkgLine += 256;
			iIdx += FNT_CHARS; // advance to next row in font data
		}
		pdwAddr += FNT_CHAR_WIDTH; // set screen address to next character position
		pdwBkg += FNT_CHAR_WIDTH;
	}
#endif
}

u8 *MyAlloc(int size, char *title)
{
	u8 *mem;
	mem=(u8*)malloc(size);
	if (mem==NULL) {
		myprintf("Allocate %s (%d): FAILED", title, size);
		while(1) swiWaitForVBlank();
	} else {
		//    myprintf("Allocate %s (%d): OK", title, size);
	}
	return mem;
}
