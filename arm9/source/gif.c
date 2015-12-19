#include <nds.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gif.h"

struct ColorEntry {
    u8 red, green, blue;
};

// RETOUR

u16 *outbuf16;
u8 *outbuf8;
u32     dwWidth;       // Buffer width in pixels
u32     dwHeight;      // Buffer height in pixels

typedef s16 (* pfctWritePixel)(u8);

pfctWritePixel WritePixel;


// ENTREE

int gif_format;

void OpenGif(u8 *buf, int size);

s16 SkipObject(void);

s16 WritePixel16(u8 Pixel);
s16 WritePixel8(u8 Pixel);

s16 ReadByte(void);

s16 ReadScreenDesc(u32 *w, u32 *h, s16 *ColorRez, s16 *FillColor, u16 *NumColors, struct ColorEntry ColorMap[], s16 ColorMapSize);
s16 ReadImageDesc(s16 *LeftEdge, s16 *TopEdge, s16 *Width, s16 *Height, s16 *Interlaced, u16 *NumColors, struct ColorEntry ColorMap[], s16 ColorMapSize);

s16 read_code(void);
void init_table(s16 min_code_size);

s16 Expand_Data(void);
void DisplayPictures(u16 *pWidth,u16 *pHeight);


int outpos;

u8 *inbuf;
int inpos;
int insize;

s16 BackdropWidth, BackdropHeight; // size of the GIF virtual screen */
s16 LeftEdge, TopEdge;			   // coordinates of the GIF image or */
s16 RightEdge, BottomEdge;		   // text object */
u16 DefaultNumColors;
u16 LocalNumColors;
s16 X, Y;			               // current point on screen */
s16 InterlacePass;
s16 Interlaced;		               // image is "interlaced" */

struct ColorEntry DefaultColorMap[256];
struct ColorEntry LocalColorMap[256];

char GIFsignature[7];

s16 BaseLine[5];
s16 LineOffset[5];

s16 hWidth,hHeight;	

struct code_entry {
	s16 prefix;			// prefix code 
	char suffix;		// suffix character 
	char stack;
};

s16 code_size;
s16 clear_code;
s16 eof_code;
s16 first_free;
s16 bit_offset;
u16 byte_offset, bits_left;
s16 max_code;
s16 free_code;
s16 old_code;
s16 input_code;
s16 code;
s16 suffix_char;
s16 final_char;
s16 bytes_unread;
unsigned char code_buffer[64];
struct code_entry *code_table;

s16 mask[12];

/*
            int r = ( RgbCPCdef[ i ] >> 19 ) & 0x1F;
            int g = ( RgbCPCdef[ i ] >> 11 ) & 0x1F;
            int b = ( RgbCPCdef[ i ] >> 3 ) & 0x1F;
     
            BG_PALETTE[i]=RGB15(r,g,b);
*/

void InitGif(u8 *buf, int size);

int ReadBackgroundGif8(u8 *dest, u8 *pImageFileMem, int dwImageFileSize)
{

    outbuf8=dest;
    WritePixel=WritePixel8;

    InitGif(pImageFileMem, dwImageFileSize);

    int n;
    for(n=0;n<256;n++) {
        BG_PALETTE[n] = RGB15(DefaultColorMap[n].red, DefaultColorMap[n].green, DefaultColorMap[n].blue) | 0x8000;
    }

	OpenGif(pImageFileMem, dwImageFileSize);

    return 1;	
}

int ReadBackgroundGif(u16 *dest, u8 *pImageFileMem, int dwImageFileSize)
{
    outbuf16=dest;
    WritePixel=WritePixel16;

    InitGif(pImageFileMem, dwImageFileSize);
	
	OpenGif(pImageFileMem, dwImageFileSize);
	
    return 1;	
}


void InitGif(u8 *buf, int size)
{
    s16 ColorRez;			/* not used yet */
	s16 FillColor;			/* color index of fill color */

	char DGIFsignature[7] = "GIF87a";
	
	s16 DBaseLine[5] = {0, 4, 2, 1, 0};
	s16 DLineOffset[5] = {8, 8, 4, 2, 0};
	
	s16 Dmask[12] = {0x001, 0x003, 0x007, 0x00F, 0x01F, 0x03F, 0x07F, 0x0FF, 0x1FF, 0x3FF, 0x7FF, 0xFFF};
	
	memcpy(GIFsignature, DGIFsignature, 7*sizeof(char));
	memcpy(BaseLine, DBaseLine, 5*sizeof(s16));
	memcpy(LineOffset, DLineOffset, 5*sizeof(s16));
	memcpy(mask, Dmask, 12*sizeof(s16));
	dwHeight = 0;
	dwWidth = 0;

    inbuf=buf;
    inpos=0;
    insize=size;
	
    // Get the screen description 
	
    if (!ReadScreenDesc(&dwWidth, &dwHeight, &ColorRez, &FillColor, &DefaultNumColors, DefaultColorMap, 256)) { // Invalid GIF dataset
		dwWidth=0; 
        dwHeight=0;
		return;
	}
}


void OpenGif(u8 *buf, int size)
{
	
	s16 Width, Height;
	unsigned int Done;
	unsigned int WaitNeeded; // determines whether the default disposition action is needed 
	
    s16 Id;				/* object identifier */
	s16 Status;			/* return status code */
	
	outpos = 0;
	
    WaitNeeded = FALSE;
    Done = FALSE;
	
    /* Now display one or more GIF objects */
	
    while(!Done) {
		switch (ReadByte()) {
		case -1:    // ERREUR DANS LE GIF ! A SUPPRIMER APRES // ADD BY REDBUG ?
		case ';':	// End of the GIF dataset 		
			Done = TRUE;
			break;
			
		case ',':   // Start of an image object Read the image description.
			if (!ReadImageDesc( &LeftEdge, &TopEdge, &Width, &Height, &Interlaced, &LocalNumColors, LocalColorMap, 256)) {
				dwWidth=0; 
                dwHeight=0;
				return;
			}
			
			/* Setup the color palette for the image */
			dwHeight = Height;
			dwWidth = Width;
			
			if (LocalNumColors > 0)  // Change the palette table
			{
			}
			else if (DefaultNumColors > 0) // Reset the palette table back to the default setting 
			{			
			}
			
			X = LeftEdge;
			Y = TopEdge;
			RightEdge = (u16)(LeftEdge + Width - 1);
			BottomEdge = (u16)(TopEdge + Height - 1);
			InterlacePass = 0;
			
			Status = Expand_Data();
			
			if (Status != 0) {         // Error expanding the raster image 
				dwWidth=0; 
                dwHeight=0;
				return;
				// Done = TRUE;
			}
			
			WaitNeeded = TRUE;
			break;
			
		case '!':
			/* Start of an extended object (not in rev 87a) */
			
			Id = ReadByte();	/* Get the object identifier */
			
			if (Id < 0)	{ // Error reading object identifier 
				dwWidth=0; 
                dwHeight=0;
				return;
				// 	Done = TRUE;
				break;
			}
			
			/* Since there no extented objects defined in rev 87a, we
			will just skip over them.  In the future, we will handle
			the extended object here. */
			
			if (!SkipObject())
				Done = TRUE;
			
			break;
			
		default: // Error
			dwWidth=0; dwHeight=0;
			return;
			// Done = TRUE;
			break;
		}
	}
}


void init_table(s16 min_code_size)
{
    code_size = (s16)(min_code_size + 1);
    clear_code = (s16)(1 << min_code_size);
    eof_code = (s16)(clear_code + 1);
    first_free = (s16)(clear_code + 2);
    free_code = first_free;
    max_code = (s16)(1 << code_size);
}

s16 read_code(void)
{
    s16 bytes_to_move, i, ch;
    s32 temp;
	
    byte_offset = (u16)(bit_offset >> 3);
    bits_left = (u16)(bit_offset & 7);
	
    if (byte_offset >= 61) {
		bytes_to_move = (s16)(64 - byte_offset);
		
		for (i = 0; i < bytes_to_move; i++) {
			code_buffer[i] = code_buffer[byte_offset + i];
        }
		
		while (i < 64) {
			if (bytes_unread == 0) {  // Get the length of the next record. A zero-length record denotes "end of data".
				bytes_unread = ReadByte();
				
				if (bytes_unread < 1) {
					if (bytes_unread == 0)	/* end of data */
						break;
					else  {
						free((char *) code_table);
						return (s16)(bytes_unread);
					}
                }
			}
			
			ch = ReadByte();
			
			if (ch < 0)  {
				return (s16)(ch);
			}
			code_buffer[i++] = (unsigned char)ch;
			bytes_unread--;
		}
		
		bit_offset = bits_left;
		byte_offset = 0;
	}
	
    bit_offset = (s16)(bit_offset + code_size);
    temp = (long) code_buffer[byte_offset] | (long) code_buffer[byte_offset + 1] << 8 | (long) code_buffer[byte_offset + 2] << 16;
	
    if (bits_left > 0)
		temp >>= bits_left;
	
    return temp & mask[code_size - 1];
}

/*
* Function:
*	Decompress a LZW compressed data stream.
*
* Inputs:
*	get_byte_routine - address of the caller's "get_byte" routine.
*	put_byte_routine - address of the caller's "put_byte" routine.
*
* Returns:
*	0	OK
*	-1	expected end-of-file
*	-2	cannot allocate memory
*	-3	bad "min_code_size"
*	< -3	error status from the get_byte or put_byte routine
*/


s16 Expand_Data(void)
{
	int status;
	s16 sp;				/* stack ptr */
    s16 min_code_size;
	int largest_code = 4095;
	
    code_table = (struct code_entry *)malloc(sizeof(struct code_entry)*(largest_code + 1));
	
    if (code_table == NULL) {
		return -2;
    }
	
    /* Get the minimum code size (2 to 9) */
	
    min_code_size = ReadByte();
	
    if (min_code_size < 0) {
		free(code_table);
		return (s16)(min_code_size);
	} else if ( (min_code_size<2) || (min_code_size>9) ) {
		free(code_table);
		return (s16)(-3);
	}
	
    init_table(min_code_size);
    sp = 0;
    bit_offset = 64*8;			// force "read_code" to start a new 
    bytes_unread = 0;			// record 
	
    while ((code = (s16)read_code()) != eof_code) {
		if (code==-1) {
			break;
		}
		if (code == clear_code) {
			init_table(min_code_size);
			code = (s16)read_code();
			old_code = code;
			suffix_char = code;
			final_char = code;
			if ((status = WritePixel((unsigned char)suffix_char)) != 0)  {
				free((char *) code_table);
				return (s16)(status);
			}
		} else {
			input_code = code;
			
			if (code >= free_code) {
				code = old_code;
				code_table[sp++].stack = (char)final_char;
			}
			
			while (code >= first_free)
			{
				code_table[sp++].stack = code_table[code].suffix;
				code = code_table[code].prefix;
			}
			
			final_char = code;
			suffix_char = code;
			code_table[sp++].stack = (char)final_char;
			
			while (sp > 0) {
				if ((status =  WritePixel(code_table[--sp].stack)) != 0)  {
					free((char *) code_table);
					return (s16)(status);
				}
            }
			
			code_table[free_code].suffix = (char)suffix_char;
			code_table[free_code].prefix = old_code;
			free_code++;
			old_code = input_code;
			
			if (free_code >= max_code) {
				if (code_size < 12) {
					code_size++;
					max_code <<= 1;
				}
			}
		}
		// if (code==0) break; // ADD BY REDBUG ?
	}
	
    free((char *) code_table);
    return 0;
}

/*
* Read the signature, the screen description, and the optional default
* color map.
*/

s16 ReadScreenDesc(u32 *w, u32 *h, s16 *ColorRez, s16 *FillColor, u16 *NumColors, struct ColorEntry ColorMap[], s16 ColorMapSize)
{
    u8 Buffer[16];
    s16 I, J, Status, HaveColorMap, NumPlanes;
	
    for (I = 0; I < 13; I++) {
		Status = ReadByte();
		if (Status < 0) {
            return FALSE;
        }
		Buffer[I] = (u8) Status;
	}
	
	gif_format=0;
	
    for (I = 0; I < 6; I++)
		if ((Buffer[I] != GIFsignature[I]) & (I!=4)) {
			return FALSE;
        }
		
		if (Buffer[4]=='7') {
			gif_format = 87;
		}
		if (Buffer[4]=='9') {
			gif_format = 89;
		}
		if (gif_format == 0) {
			return FALSE;
		}
		
		*w = (u32)(Buffer[6] | Buffer[7] << 8);
		*h = (u32)(Buffer[8] | Buffer[9] << 9);
		NumPlanes = (s16)((Buffer[10] & 0x0F) + 1);
		
		/* bit 3 should be 0 in rev 87a */
		
		*ColorRez = (s16)(((Buffer[10] & 0x70) >> 4) + 1);
		HaveColorMap = (Buffer[10] & 0x80) != 0;
		*NumColors = (u16)(1 << NumPlanes);
        if (*NumColors>256) {
            *NumColors=256;
        }
		*FillColor = Buffer[11];
		/*  Reserved = Buffer[12]; */
		
		if (HaveColorMap) {
			for (I = 0; I < *NumColors; I++) {
				for (J = 0; J < 3; J++) {
					Status = ReadByte();
					if (Status < 0) return FALSE;
					Buffer[J] = Status & 255;
				}
				
				if (I < ColorMapSize) {
					ColorMap[I].red = Buffer[0] >> 3;    // NDS: >> 3
					ColorMap[I].green = Buffer[1] >> 3;  // NDS: >> 3
					ColorMap[I].blue = Buffer[2] >> 3;  // NDS: >> 3
				}
			}
		} else {
			*NumColors = 0;
        }
		
		return TRUE;
}

/*
* Read the image description and the optional color map.
*/

s16 ReadImageDesc(s16 *LeftEdge, s16 *TopEdge, s16 *Width, s16 *Height, s16 *Interlaced, u16 *NumColors, struct ColorEntry ColorMap[], s16 ColorMapSize)
{
    unsigned char Buffer[16];
    s16 I, J, NumPlanes, HaveColorMap, Status;
	
    for (I = 0; I < 9; I++)
	{
		if ((Status = (ReadByte())) < 0) return FALSE;
		Buffer[I] = (unsigned char) Status;
	}
	
    *LeftEdge = (s16)(Buffer[0] | Buffer[1] << 8);
    *TopEdge = (s16)(Buffer[2] | Buffer[3] << 8);
    *Width = (s16)(Buffer[4] | Buffer[5] << 8);
    *Height = (s16)(Buffer[6] | Buffer[7] << 8);
    NumPlanes = (s16)((Buffer[8] & 0x0F) + 1);
    *NumColors = (s16)(1 << NumPlanes);
	
    /* Bits 3, 4, and 5 should be zero (reserved) in rev 87a */
	
    *Interlaced = (Buffer[8] & 0x40) != 0;
    HaveColorMap = (Buffer[8] & 0x80) != 0;
	
    if (HaveColorMap)
	{
		for (I = 0; I < *NumColors; I++)
		{
			for (J = 0; J < 3; J++)
			{
				if ((Status = (ReadByte())) < 0) return FALSE;
				Buffer[J] = (unsigned char) Status;
			}
			
			if (I < ColorMapSize)
			{
				ColorMap[I].red = Buffer[0] >> 3; // NDS >> 3
				ColorMap[I].green = Buffer[1] >> 3; // NDS >> 3
				ColorMap[I].blue = Buffer[2] >> 3; // NDS >> 3
			}
		}
	}
    else
		*NumColors = 0;
	
    return TRUE;
}








/*
* Read the next byte from the GIF data stream.
*
* Returns:
*	0 .. 255	the byte
*	-1		end of data
*	-4		read error
*/

s16 ReadByte(void)
{
	s16 a;
	
	if (inpos >= insize) {
		return -1;
	}
	
	a=inbuf[inpos];
	inpos++;
	
	return a;
}


s16 WritePixel16(u8 Pixel)
{
    // outbuf[outpos] = RGB15( DefaultColorMap[Pixel].red, DefaultColorMap[Pixel].green, DefaultColorMap[Pixel].blue);
    if (outpos>=dwHeight*dwWidth) {
        return 1;
    }
	
    // Pixel=outpos&255;
	
    outbuf16[outpos] = RGB15(DefaultColorMap[Pixel].red, DefaultColorMap[Pixel].green, DefaultColorMap[Pixel].blue) | 0x8000;
	outpos++;
	
    /* Advance the point */
	
    X++;
	
    if (X > RightEdge)
	{
		X = LeftEdge;
		
		if (Interlaced)
		{
			Y = (s16)(Y + LineOffset[InterlacePass]);
			
			if (Y > BottomEdge)
			{
				InterlacePass++;
				Y = (s16)(TopEdge + BaseLine[InterlacePass]);
			}
		}
		else
			Y++;
	}
	
    return 0;
}

s16 WritePixel8(u8 Pixel)
{
    // outbuf[outpos] = RGB15( DefaultColorMap[Pixel].red, DefaultColorMap[Pixel].green, DefaultColorMap[Pixel].blue);
    if (outpos>=dwHeight*dwWidth) {
        return 1;
    }
	
    // Pixel=outpos&255;
	
    outbuf8[outpos] = Pixel;
	outpos++;
	
    /* Advance the point */
	
    X++;
	
    if (X > RightEdge) {
		X = LeftEdge;
		
		if (Interlaced) {
			Y = (s16)(Y + LineOffset[InterlacePass]);
			
			if (Y > BottomEdge) {
				InterlacePass++;
				Y = (s16)(TopEdge + BaseLine[InterlacePass]);
			}
		} else {
			Y++;
        }
	}
	
    return 0;
}

/*
* Skip over an extended GIF object.
*/

s16 SkipObject(void)
{
    s16 Count;
	
    while ((Count = ReadByte()) > 0)
		do
		{
			if (ReadByte() < 0) 				/* Error reading data stream */
			{
				dwWidth=0; 
                dwHeight=0;
				return FALSE;
			}
		}
		while (--Count > 0);
		
		if (Count < 0) 			/* Error reading data stream */
		{
			dwWidth=0; dwHeight=0;
			return FALSE;
		}
		else
			return TRUE;
}

/*---
  Gestion des fonts
---*/  

#define DEFAULT_STR " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ý\x01\x02\x03\x04"


KKFONT *CreateFont(u8 *gif, u32 size)
{
    KKFONT *m_font = (KKFONT*)malloc(sizeof(KKFONT));
    
    WritePixel=WritePixel16;

    InitGif(gif, size);

    outbuf16=(u16*)malloc(dwHeight*dwWidth*2);

	OpenGif(gif,size);
	m_font->pSurface = outbuf16;
	m_font->dwWidth = dwWidth;
	m_font->dwHeight = dwHeight;
	

	const u16 ck = RGB15(31, 0, 31) | 0x8000;
    u16 color;
	u8 chaine[256];
	int n,w;
	
	unsigned int cur;
	

	int mo; // 0: in transparent color, 1: in font
	
	strcpy((char*)chaine,DEFAULT_STR);
	
	int len;
	
	len = 256; // strlen(chaine)
	
	m_font->pKerningLeft = (s32*)malloc(len*sizeof(s32));
	m_font->pKerningRight = (s32*)malloc(len*sizeof(s32));
	
	for(n=0;n<len;n++) {
		m_font->pKerningLeft[n]=0;
		m_font->pKerningRight[n]=0;
	}
	
	w=m_font->dwWidth;
	cur=0;
	
	mo=0;
	
	for(n=0;n<w;n++) {
		color = m_font->pSurface[n];
		
		if (color==ck) {
			if (mo==1) {
				m_font->pKerningRight[chaine[cur]]=n;
				cur++;
				if (chaine[cur]==0) {
					break;
				}
				mo=0;
			}
		} else {
			if (mo==0) {
				m_font->pKerningLeft[chaine[cur]]=n;
				mo=1;
			}
		}
	}

    return m_font;
}

int DrawText(u16 *destbuf, KKFONT *m_font, int dwX, int dwY, char *pString)
{
	unsigned int n;
	int h;
	int width;
	
	int posx;

    int y;

	h = m_font->dwHeight;

	posx=dwX;

	n=0;
	width=0;
	
	int max;
	
	max=252;

	while(1) {
		u32 car;
		u16 *src, *dest;
		
		car=(u32)(pString[n]);
			if (car==0) break;
			
			src=m_font->pSurface+m_font->pKerningLeft[car]+2*m_font->dwWidth;
			dest=destbuf+posx+width+dwY*256;
		
			if (m_font->pKerningRight[car] != 0) {
                int w;
                
                w=m_font->pKerningRight[car]-m_font->pKerningLeft[car];
                if (posx+width+w>=max) {
                    w=max-(posx+width);
                }

				for(y=0;y<h-2;y++) {
//                    memcpy(dest, src, w*2); // Ne marche pas... pq ?
                    int x;
                    for(x=0;x<w;x++) {
                    dest[x]= src[x]; 
                    }
                    dest+=256;
                    src+=m_font->dwWidth;
				}

				width+=w;

                if (w!=m_font->pKerningRight[car]-m_font->pKerningLeft[car]) {
                    break;
                }
			}
			n++;
		}

    return width;
}


int DrawTextCenter(u16 *destbuf, KKFONT *m_font, int color, int dwY, char *pString)
{
	unsigned int n;
	int h;
	int width;
	
	int posx;

    int y;
    int dwX;

	h = m_font->dwHeight;

	posx=0;
    dwX=0;
	n=0;
	width=0;
	
	int max;

	const u16 ck = RGB15(31, 0, 31) | 0x8000;

    width=0;

    max=255;

	while(1) {
		u32 car;
		
		car=(u32)(pString[n]);
			if (car==0) break;
			
			if (m_font->pKerningRight[car] != 0) {
                int w;
                
                w=m_font->pKerningRight[car]-m_font->pKerningLeft[car];
                if (posx+width+w>=max) {
                    w=max-(posx+width);
                }
				width+=w;

                if (w!=m_font->pKerningRight[car]-m_font->pKerningLeft[car]) {
                    break;
                }
			}
			n++;
		}

    dwX=(255-width)/2;
    posx=dwX;
    width=0;
    n=0;

    
	while(1) {
		u32 car;
		u16 *src, *dest;
		
		car=(u32)(pString[n]);
			if (car==0) break;
			
			src=m_font->pSurface+m_font->pKerningLeft[car]+2*m_font->dwWidth;
			dest=destbuf+posx+width+dwY*256;
		
			if (m_font->pKerningRight[car] != 0) {
                int w;
                
                w=m_font->pKerningRight[car]-m_font->pKerningLeft[car];
                if (posx+width+w>=max) {
                    w=max-(posx+width);
                }

                if (car>=32) {

				for(y=0;y<h-2;y++) {
//                    memcpy(dest, src, w*2); // Ne marche pas... pq ?
                    int x;
                    for(x=0;x<w;x++) {
                    if (src[x]!=ck) {
                        dest[x]=color;
                    }
                    }
                    dest+=256;
                    src+=m_font->dwWidth;
				}
                } else {
				for(y=0;y<h-2;y++) {
//                    memcpy(dest, src, w*2); // Ne marche pas... pq ?
                    int x;
                    for(x=0;x<w;x++) {
                    if (src[x]!=ck) {
                        dest[x]=src[x];
                    }
                    }
                    dest+=256;
                    src+=m_font->dwWidth;
				}
    }

				width+=w;

                if (w!=m_font->pKerningRight[car]-m_font->pKerningLeft[car]) {
                    break;
                }
			}
			n++;
		}

    return width;
}

int DrawTextLeft(u16 *destbuf, KKFONT *m_font, int color, int dwX, int dwY, char *pString)
{
	unsigned int n;
	int h;
	int width;
	
	int posx;

    int y;

	h = m_font->dwHeight;

	posx=0;
	n=0;
	width=0;
	
	int max;

	const u16 ck = RGB15(31, 0, 31) | 0x8000;

    width=0;

    max=255;

    posx=dwX;
    width=0;
    n=0;

    
	while(1) {
		u32 car;
		u16 *src, *dest;
		
		car=(u32)(pString[n]);
			if (car==0) break;
			
			src=m_font->pSurface+m_font->pKerningLeft[car]+2*m_font->dwWidth;
			dest=destbuf+posx+width+dwY*256;
		
			if (m_font->pKerningRight[car] != 0) {
                int w;
                
                w=m_font->pKerningRight[car]-m_font->pKerningLeft[car];
                if (posx+width+w>=max) {
                    w=max-(posx+width);
                }

                if (car>=32) {

				for(y=0;y<h-2;y++) {
//                    memcpy(dest, src, w*2); // Ne marche pas... pq ?
                    int x;
                    for(x=0;x<w;x++) {
                    if (src[x]!=ck) {
                        dest[x]=color;
                    }
                    }
                    dest+=256;
                    src+=m_font->dwWidth;
				}
                } else {
				for(y=0;y<h-2;y++) {
//                    memcpy(dest, src, w*2); // Ne marche pas... pq ?
                    int x;
                    for(x=0;x<w;x++) {
                    if (src[x]!=ck) {
                        dest[x]=src[x];
                    }
                    }
                    dest+=256;
                    src+=m_font->dwWidth;
				}
    }

				width+=w;

                if (w!=m_font->pKerningRight[car]-m_font->pKerningLeft[car]) {
                    break;
                }
			}
			n++;
		}

    return width;
}

