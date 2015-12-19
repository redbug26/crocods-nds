
int ReadBackgroundGif(u16 *dest, u8 *from, int size);
int ReadBackgroundGif8(u8 *dest, u8 *from, int size);

typedef struct {
	u32           dwTop;
	u32           dwWidth;
	u32           dwHeight;
	s32            lTracking;
	s32*           pOffset;
	s32*           pWidth;
	s32*           pKerningLeft;
	s32*           pKerningRight;
	u32           dwFlags;
	u16 *pSurface;
    
} KKFONT;

KKFONT *CreateFont(u8 *gif, u32 size);
int DrawText(u16 *dest, KKFONT *m_font, int dwX, int dwY, char *pString);
int DrawTextCenter(u16 *dest, KKFONT *m_font, int dwX, int dwY, char *pString);
int DrawTextLeft(u16 *dest, KKFONT *m_font, int color, int dwX, int dwY, char *pString);

