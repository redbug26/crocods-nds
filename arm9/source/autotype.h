#ifndef __AUTOTYPE_HEADER_INCLUDED__
#define __AUTOTYPE_HEADER_INCLUDED__

void ASCII_to_CPC(int nASCII, BOOL bKeyDown);

/* auto-type is active and is "typing" */
#define AUTOTYPE_ACTIVE 0x01
/* auto-type is performing key release action */
/* if clear, auto-type is performing key press action */
#define AUTOTYPE_RELEASE 0x02
/* if set, auto-type is waiting for first keyboard scan to be done */
#define AUTOTYPE_WAITING 0x04

typedef struct
{
	char ch;
	/* the string; as ascii characters to type */
	char sString[256];
	/* current position within the string */
	int nPos;
	/* number of characters remaining to type */
	int nCountRemaining;
	/* number of frames to waste before continuing */
	int nFrames;

	unsigned long nFlags;
}  AUTOTYPE;

void AutoType_Init();

BOOL AutoType_Active();

/* set the string to auto type */
void AutoType_SetString(const char *sString, BOOL bWaitInput);

/* execute this every emulated frame; even if it will be skipped */
void AutoType_Update(void);

#endif
