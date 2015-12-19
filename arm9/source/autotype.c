#include <nds.h>

#include "types.h"
#include "plateform.h"
#include "autotype.h"
#include "vga.h"
#include "ppi.h"

#include <string.h>

static AUTOTYPE AutoType;

/* this table converts from ASCII to CPC keyboard codes */
/* not exactly the same as converting from host keyboard
scan codes to CPC keys */

typedef struct 
{
  int        nASCII;
  CPC_SCANCODE cpcKey1;
  CPC_SCANCODE cpcKey2;
} ASCII_TO_CPCKEY_MAP;

/* many thanks to Troels K for the original table */

/* this table is not complete */
static ASCII_TO_CPCKEY_MAP ASCIIToCPCMap[]=
{	   
	{ '1'       , CPC_1    , CPC_NIL } ,
	{ '!'       , CPC_1    , CPC_SHIFT} ,
	   
	{ '2'       , CPC_2    , CPC_NIL } ,
	{ '"'       , CPC_2    , CPC_SHIFT} ,
	   
	{ '3'       , CPC_3    , CPC_NIL } ,
	{ '#'       , CPC_3    , CPC_SHIFT} ,
	   
	{ '4'       , CPC_4    , CPC_NIL } ,
	{ '$'       , CPC_4    , CPC_SHIFT} ,
	   
	{ '5'       , CPC_5    , CPC_NIL } ,
	{ '%'       , CPC_5    , CPC_SHIFT} , 
	   
	{ '6'       , CPC_6    , CPC_NIL } ,
	{ '&'       , CPC_6    , CPC_SHIFT} ,

	{ '7'       , CPC_7    , CPC_NIL } ,
	{ '\''      , CPC_7    , CPC_SHIFT} ,

	{ '8'       , CPC_8    , CPC_NIL } ,
	{ '('       , CPC_8    , CPC_SHIFT} ,
	   
	{ '9'       , CPC_9    , CPC_NIL } ,
	{ ')'       , CPC_9    , CPC_SHIFT} ,
	   
	{ '0'       , CPC_ZERO , CPC_NIL } ,
	{ '_'       , CPC_ZERO , CPC_SHIFT} , 
	   
	{ '-'       , CPC_MINUS , CPC_NIL } ,
	{ '='       , CPC_MINUS , CPC_SHIFT} ,
	   
	{ '^'       , CPC_HAT  , CPC_NIL } ,
	{ '£'       , CPC_HAT  , CPC_SHIFT} ,

	{ '\t'		,CPC_TAB  , CPC_NIL } ,

	{ 'q'       , CPC_Q    , CPC_NIL } ,
	{ 'Q'       , CPC_Q    , CPC_SHIFT} ,

	{ 'w'       , CPC_W    , CPC_NIL } ,
	{ 'W'       , CPC_W    , CPC_SHIFT} , 

	{ 'e'       , CPC_E    , CPC_NIL } ,
	{ 'E'       , CPC_E    , CPC_SHIFT} ,

	{ 'r'       , CPC_R    , CPC_NIL } ,
	{ 'R'       , CPC_R    , CPC_SHIFT} ,

	{ 't'       , CPC_T    , CPC_NIL } ,
	{ 'T'       , CPC_T    , CPC_SHIFT} ,

	{ 'y'       , CPC_Y    , CPC_NIL } ,
	{ 'Y'       , CPC_Y    , CPC_SHIFT} ,

	{ 'u'       , CPC_U    , CPC_NIL } ,
	{ 'U'       , CPC_U    , CPC_SHIFT} , 

	{ 'i'       , CPC_I    , CPC_NIL } ,
	{ 'I'       , CPC_I    , CPC_SHIFT} ,

	{ 'o'       , CPC_O    , CPC_NIL } ,
	{ 'O'       , CPC_O    , CPC_SHIFT} ,

	{ 'p'       , CPC_P    , CPC_NIL } ,
	{ 'P'       , CPC_P    , CPC_SHIFT} ,
	   
	{ '@'       , CPC_AT   , CPC_NIL } ,
	{ '|'       , CPC_AT   , CPC_SHIFT } ,
	   
	{ '['       , CPC_OPEN_SQUARE_BRACKET , CPC_NIL } ,
	{ '{'       , CPC_OPEN_SQUARE_BRACKET , CPC_SHIFT} ,
	   
	{ '\n', CPC_RETURN , CPC_NIL } ,
	   
	{ 'a'       , CPC_A    , CPC_NIL } ,
	{ 'A'       , CPC_A    , CPC_SHIFT} ,
	   
	{ 's'       , CPC_S    , CPC_NIL } ,
	{ 'S'       , CPC_S    , CPC_SHIFT} ,

	{ 'd'       , CPC_D    , CPC_NIL } ,
	{ 'D'       , CPC_D    , CPC_SHIFT} ,
	   
	{ 'f'       , CPC_F    , CPC_NIL } ,
	{ 'F'       , CPC_F    , CPC_SHIFT} ,

	{ 'g'       , CPC_G    , CPC_NIL } ,
	{ 'G'       , CPC_G    , CPC_SHIFT} ,

	{ 'h'       , CPC_H    , CPC_NIL } ,
	{ 'H'       , CPC_H    , CPC_SHIFT} ,

	{ 'j'       , CPC_J    , CPC_NIL } ,
	{ 'J'       , CPC_J    , CPC_SHIFT} ,

	{ 'k'       , CPC_K    , CPC_NIL } ,
	{ 'K'       , CPC_K    , CPC_SHIFT} ,

	{ 'l'       , CPC_L    , CPC_NIL } ,
	{ 'L'       , CPC_L    , CPC_SHIFT} ,
	   
	{ ':'       , CPC_COLON , CPC_NIL } ,
	{ '*'       , CPC_COLON , CPC_SHIFT} ,

	{ ';'       , CPC_SEMICOLON , CPC_NIL } ,
	{ '+'       , CPC_SEMICOLON , CPC_SHIFT} ,

	{ ']'       , CPC_CLOSE_SQUARE_BRACKET , CPC_NIL } ,
	{ '}'       , CPC_CLOSE_SQUARE_BRACKET , CPC_SHIFT} ,
	   
	{ '/'      , CPC_BACKSLASH , CPC_NIL } ,
	{ '`'      , CPC_BACKSLASH , CPC_SHIFT} ,

	{ 'z'       , CPC_Z    , CPC_NIL } ,
	{ 'Z'       , CPC_Z    , CPC_SHIFT} ,

	{ 'x'       , CPC_X    , CPC_NIL } ,
	{ 'X'       , CPC_X    , CPC_SHIFT} ,

	{ 'c'       , CPC_C    , CPC_NIL } ,
	{ 'C'       , CPC_C    , CPC_SHIFT} ,

	{ 'v'       , CPC_V    , CPC_NIL } ,
	{ 'V'       , CPC_V    , CPC_SHIFT} ,

	{ 'b'       , CPC_B    , CPC_NIL } ,
	{ 'B'       , CPC_B    , CPC_SHIFT} ,

	{ 'n'       , CPC_N    , CPC_NIL } ,
	{ 'N'       , CPC_N    , CPC_SHIFT} ,

	{ 'm'       , CPC_M    , CPC_NIL } ,
	{ 'M'       , CPC_M    , CPC_SHIFT} ,

	{ ','       , CPC_COMMA, CPC_NIL } ,
	{ '<'       , CPC_COMMA, CPC_SHIFT} ,
      
	{ '.'       , CPC_DOT  , CPC_NIL } ,
	{ '>'       , CPC_DOT  , CPC_SHIFT} ,
	   
	{ '\\'      , CPC_FORWARD_SLASH , CPC_NIL } ,
	{ '?'       , CPC_BACKSLASH, CPC_SHIFT} ,

	{ ' ', CPC_SPACE, CPC_NIL } ,
};

#ifndef _countof
   #define _countof(array) (sizeof(array)/sizeof((array)[0]))
#endif

void ASCII_to_CPC(int nASCII, BOOL bKeyDown)
{
   int i;
   ASCII_TO_CPCKEY_MAP *pMap = ASCIIToCPCMap;
   int nMap = _countof(ASCIIToCPCMap);

   for (i = 0; i < nMap; i++)
   {
      if (pMap->nASCII == nASCII)
      {
         if (bKeyDown)
         {
            if (pMap->cpcKey2 != CPC_NIL)
            {
               CPC_SetScanCode(pMap->cpcKey2);
            }
            CPC_SetScanCode(pMap->cpcKey1);
         }
         else 
         {
            CPC_ClearScanCode(pMap->cpcKey1);
            if (pMap->cpcKey2!= CPC_NIL)
            {
               CPC_ClearScanCode(pMap->cpcKey2);
            }
         }
         break;
      }
   
	  pMap++;
   }
}


/* init the auto type functions */
void AutoType_Init()
{
	AutoType.nFlags = 0;
	AutoType.sString[0]=0;
	AutoType.nPos = 0;
	AutoType.nFrames = 0;
	AutoType.nCountRemaining = 0;
}

BOOL AutoType_Active()  
{
	/* if actively typing, or waiting for first keyboard scan
	before typing then auto-type is active */
	return ((AutoType.nFlags & (AUTOTYPE_ACTIVE|AUTOTYPE_WAITING))!=0);
}

/* set the string to auto type */
void AutoType_SetString(const char *sString, BOOL bWaitInput)
{
	strcpy(AutoType.sString, sString);
	AutoType.ch = 0;
	AutoType.nPos = 0;
	AutoType.nFrames = 0;
	AutoType.nCountRemaining = strlen(sString);
	if (bWaitInput)
	{
	    ResetCPC();

		/* wait for first keyboard */
		AutoType.nFlags|=AUTOTYPE_WAITING;
		AutoType.nFlags&=~AUTOTYPE_ACTIVE;
	} else {
		AutoType.nFlags |= AUTOTYPE_ACTIVE;
	}
}

/* execute this every emulated frame; even if it will be skipped */
void AutoType_Update(void)
{
	if ((AutoType.nFlags & AUTOTYPE_ACTIVE)==0)
	{
		if ((AutoType.nFlags & AUTOTYPE_WAITING)!=0)
		{
			if (Keyboard_HasBeenScanned())
			{
				/* auto-type is now active */
				AutoType.nFlags |= AUTOTYPE_ACTIVE;
				/* no longer waiting */
				AutoType.nFlags &=~AUTOTYPE_WAITING;
			}
		}
	}
	else
	{
		/* auto-type is active */

		/* delay frames? */
		if (AutoType.nFrames!=0)
		{
			AutoType.nFrames--;
			return;
		}

		/* NOTES:
			- if SHIFT or CONTROL is pressed, then they must be released
			for at least one whole frame for the CPC operating system to recognise them 
			as released.
			
			- When the same key is pressed in sequence (e.g. press, release, press, release)
			then there must be at least two frames for the key to be recognised as released.
			The CPC operating system is trying to 'debounce' the key
		*/
		if (AutoType.nFlags & AUTOTYPE_RELEASE)
		{
			if (AutoType.nCountRemaining==0)
			{
				/* auto type is no longer active */
				AutoType.nFlags &=~AUTOTYPE_ACTIVE;
			}

			AutoType.nFlags &=~AUTOTYPE_RELEASE;

			if (AutoType.ch!=1)
			{
				/* release the key */
				ASCII_to_CPC(AutoType.ch, FALSE);
			}

			/* number of frames for release to be acknowledged */
			AutoType.nFrames = 1;
		}
		else
		{
			char ch;

			/* get the current character */
			ch = AutoType.sString[AutoType.nPos];

			/* update position in string */
			if ((ch!=0) & (AutoType.nPos<255)) {
				AutoType.nPos++;
			}

			/* update count */
			AutoType.nCountRemaining--;

			AutoType.ch = ch;

			if (ch==1) {
				AutoType.nFrames = 2;
			} else {
				/* number of frames for key to be acknowledged */
				AutoType.nFrames=1;
			
				ASCII_to_CPC(ch, TRUE);
			}

			AutoType.nFlags |= AUTOTYPE_RELEASE;
		}
	}
}
