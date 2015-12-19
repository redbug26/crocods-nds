/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Noyau!
// Définition du module        !CONFIG!=/V4!Ecran Clavier!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1y
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module PLATEFORM.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef PLATEFORM_H
#define PLATEFORM_H

#ifdef __cplusplus
extern "C" {
#endif



#include  <nds.h>

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif



extern int Turbo;
extern int inMenu;

struct kmenu {
    struct kmenu *parent;
    char title[256];
    char *object;
    int nbr;
    int id;
    struct kmenu *firstchild, *lastchild;
    struct kmenu *next;
	int beg,pos;
};


typedef struct {
	int left,top,right,bottom;
} RECT;


void DispIcons(void);
void Erreur( char * Msg );
void Info( char * Msg );
void InitPlateforme( void );

void SetRect(RECT *R, int left, int top, int right, int bottom);
void FillRect(RECT *R, u16 color);
void DrawRect(RECT *R, u16 color);



typedef void (* pfctDraw)(int, signed int, int );
typedef int (* pfctExecInstZ80)(void);
typedef void (* pfctResetZ80)(void);
typedef void (* pfctSetIRQZ80)(int);

extern pfctDraw DrawFct;

extern pfctExecInstZ80 ExecInstZ80;
extern pfctResetZ80 ResetZ80;
extern pfctSetIRQZ80 SetIRQZ80;

void UpdateScreen( void );
int TraceLigneCrit( void );

void videoinit(void);

int emulator_patch_ROM (u8 *rominf);

int nds_ReadKey(void);

void nds_init(void);
// void nds_set_palette(mycolor *colours);
// unsigned int *nds_video_init(t_CPC *CPC);

int nds_video_unlock(void);
int nds_video_lock(void);
void nds_video_close(void);

void LoopMenu(struct kmenu *parent);

int nds_GetTicks(void);

int nds_MapRGB(int r, int g, int b);

inline void printDebug(const char *fmt, ...);

extern int usestylus, usestylusauto;


#define MOD_CPC_SHIFT   (0x01 << 8)
#define MOD_CPC_CTRL    (0x02 << 8)
#define MOD_EMU_KEY     (0x10 << 8)

#define NBCPCSCANCODE 81

typedef struct {
    char *value;
} keyname;

typedef enum {
	/* line 0, bit 0..bit 7 */
    	CPC_CURSOR_UP, // = 0
		CPC_CURSOR_RIGHT,
		CPC_CURSOR_DOWN,
		CPC_F9,
		CPC_F6,
		CPC_F3,
		CPC_SMALL_ENTER,
		CPC_FDOT,
		/* line 1, bit 0..bit 7 */
		CPC_CURSOR_LEFT,
		CPC_COPY,
		CPC_F7,
		CPC_F8,
		CPC_F5,
		CPC_F1,
		CPC_F2,
		CPC_F0,
		/* line 2, bit 0..bit 7 */
		CPC_CLR,
		CPC_OPEN_SQUARE_BRACKET,
		CPC_RETURN,
		CPC_CLOSE_SQUARE_BRACKET,
		CPC_F4,
		CPC_SHIFT,
		CPC_FORWARD_SLASH,
		CPC_CONTROL,
		/* line 3, bit 0.. bit 7 */
		CPC_HAT,
		CPC_MINUS,
		CPC_AT,
		CPC_P,
		CPC_SEMICOLON,
		CPC_COLON,
		CPC_BACKSLASH,
		CPC_DOT,
		/* line 4, bit 0..bit 7 */
		CPC_ZERO,
		CPC_9,
		CPC_O,
		CPC_I,
		CPC_L,
		CPC_K,
		CPC_M,
		CPC_COMMA,
		/* line 5, bit 0..bit 7 */
		CPC_8,
		CPC_7,
		CPC_U,
		CPC_Y,
		CPC_H,
		CPC_J,
		CPC_N,
		CPC_SPACE,
		/* line 6, bit 0..bit 7 */
		CPC_6,
		CPC_5,
		CPC_R,
		CPC_T,
		CPC_G,
		CPC_F,
		CPC_B,
		CPC_V,
		/* line 7, bit 0.. bit 7 */
		CPC_4,
		CPC_3,
		CPC_E,
		CPC_W,
		CPC_S,
		CPC_D,
		CPC_C,
		CPC_X,
		/* line 8, bit 0.. bit 7 */
		CPC_1,
		CPC_2,
		CPC_ESC,
		CPC_Q,
		CPC_TAB,
		CPC_A,
		CPC_CAPS_LOCK,
		CPC_Z,
		/* line 9, bit 7..bit 0 */
		CPC_JOY_UP,
		CPC_JOY_DOWN,
		CPC_JOY_LEFT,
		CPC_JOY_RIGHT,
		CPC_JOY_FIRE1,
		CPC_JOY_FIRE2,
		CPC_SPARE,
		CPC_DEL, 
		
		/* no key press */
		CPC_NIL
} CPC_SCANCODE;


#define NBCPCKEY 74
typedef int CPC_KEY;

void CPC_SetScanCode(CPC_SCANCODE cpc_scancode);
void CPC_ClearScanCode(CPC_SCANCODE cpc_scancode);

void ResetCPC(void);

void CalcPoints(void);

void myprintf(const char *fmt, ...);
void myprintf0(const char *fmt, ...) ;
void drawconsole(void);

void cpcprint(int x, int y, char *pchStr, u8 Colour);
void cpcprint16(int x, int y, char *pchStr, u16 Colour);
void cpcprint16i(int x, int y, char *pchStr, int alpha);

u16 MyReadKey(void);

extern u8 z80bank[16384*4];

extern int UpdateInk;

void DispDisk(int reading);

void Autoexec(void);

extern int dispframerate;

extern int snapsave;

u8 *MyAlloc(int size, char *title);
//
// Tailles affichage ‚cran suivant la r‚solution voulue
//
#define     TAILLE_X_LOW    384
#define     TAILLE_Y_LOW    272

#define     TAILLE_X_HIGH   768
#define     TAILLE_Y_HIGH   544

#ifdef __cplusplus
}
#endif


#define ID_MENU 0
#define ID_ROM 10000000
#define ID_MONITOR_MENU 2
#define ID_COLOR_MONITOR 3
#define ID_GREEN_MONITOR 4
#define ID_SCREEN_MENU 5
#define ID_SCREEN_320 6
#define ID_SCREEN_NORESIZE 7
#define ID_SCREEN_OVERSCAN 8
#define ID_KEY_MENU 9
#define ID_KEY_KEYBOARD 10
#define ID_KEY_KEYPAD 11
#define ID_KEY_JOYSTICK 12
#define ID_DISPFRAMERATE 13
#define ID_NODISPFRAMERATE 14
#define ID_RESET 15
#define ID_SAVESNAP 16
#define ID_FILE 17
#define ID_DISK 18
#define ID_REDEFINE_UP 20
#define ID_REDEFINE_DOWN 21
#define ID_REDEFINE_LEFT 22
#define ID_REDEFINE_RIGHT 23
#define ID_REDEFINE_A 24
#define ID_REDEFINE_B 25
#define ID_REDEFINE_X 26
#define ID_REDEFINE_Y 27
#define ID_REDEFINE_L 28
#define ID_REDEFINE_R 29
#define ID_REDEFINE_START 30
#define ID_SCREEN_AUTO 31
#define ID_HACK_TABCOUL 32
#define ID_SWITCH_MONITOR 33
#define ID_ACTIVE_STYLUS 34
#define ID_DEACTIVE_STYLUS 35
#define ID_ACTIVE_MAGNUM 36
#define ID_SAVESCREEN 37
#define ID_ERASESNAP 38
#define ID_MULTIKEYPRESSED 39
#define ID_NOMULTIKEYPRESSED 40
#define ID_LOADSNAP 41


#endif
