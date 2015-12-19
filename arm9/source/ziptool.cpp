#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlib/zlib.h"
#include "plateform.h"

#include "fs.h"
#include "ziptool.h"

struct ZipLocalHeader
{
	// 'P' 'K' 03 04
	u16 ver; // version_needed_to_extract
	u16 flg; // general_purpose_bit_flag
	u16 mhd; // compression_method
	u16 tim; // last_modified_file_time
	u16 dat; // last_modified_file_date
	u32 crc; // crc32
	u32 csz; // compressed-size
	u32 usz; // uncompressed-size
	u16 fnl; // filename-len
	u16 exl; // extra_field_length
	
	char fnm[250];
	//	u8 ext[];
};

struct zipdir {
	char *name;
	int position;
};


enum ZipMethod
{
	Stored,		// 0
		Shrunk,		// 1
		Reduced1,	// 2-5
		Reduced2,
		Reduced3,
		Reduced4,
		Imploded,	// 6
		Tokenized,	// 7 ( not supported )
		Deflated,	// 8
		EnhDeflated,// 9 ( not supported )
		DclImploded,//10 ( not supported )
		
		Err=178     // this value is used by xacrett (^^;
};

struct sf_entry
{ 
	u16 Code; 
	u8 Value; 
	u8 BitLength; 
};

struct sf_tree
{
	struct sf_entry entry[256];
	int      entries;
	int      MaxLength;
}; 

#define ZIPTOOL_CACHE_SIZE 16384

class CZipTool
{
public:
	CZipTool(unsigned char *buffer, u32 size);
	CZipTool(char *filename);
	~CZipTool();
	
	bool ReadFromZIP(char *filter, u32 *dsize, unsigned char **dBuffer);

	int open;
	int zipnbrc;
	struct zipdir *zipdirc;
	
protected:

	char zipfile[256];

	void ReadDir(void);
	
	int kgetc(void);
	int kread(unsigned char *dest, int count);
	int kwrite(unsigned char *dest, int count);

	unsigned char *cached;
	int cached_from;
	int cached_max;
	
	void pathInit(void);
	
	
	void pathSplit( const char* path, int* y, int* d );
	const char* pathExt( const char* path );
	const char* pathName( const char* path );
		
	// CRC
	
	int szip;

	
	void zipwrite( u8* dat, int len );
	
	int zipread( u8* dat, int len );
	
	void SortLengths( sf_tree* tree );
	void GenerateTrees( sf_tree* tree );
	void ReverseBits( sf_tree* tree );
		
	void initbits();
	int getbits( int n );
	int fillbits( int n );
	
	void Unstore( u32 usz, u32 csz );
	void Inflate( u32 usz, u32 csz );
	void Unshrink( u32 usz, u32 csz );

	// unreduce
	void Unreduce( u32 usz, u32 csz, int factor );
	void LoadFollowers( u8* Slen, u8 followers[][64] );
	
	// explode
	void Explode( u32 usz, u32 csz, bool _8k,bool littree );
	void LoadTree( sf_tree* tree, int treesize );
	int  ReadTree( sf_tree* tree );
	void ReadLengths( sf_tree* tree );
	
	bool read_header( ZipLocalHeader* hdr);
	
	bool doHeader( ZipLocalHeader* hdr);
	
	
	// VARIABLE
	
	unsigned char* common_buf;
	
	char lb[256];
	
	u32 wrtcrc;
	
	// bit-reader
	unsigned long bitbuf;
	int bits_left;
	bool bits_eof;
	
	u32 keys[3];
	
	unsigned char *kzip, *kout;
	int pzip, pout;
	int mzip, mout;

public:
	u32 crc32( u32 crc, const u8* dat, int len );
	
};


#define XACR_BUFSIZE (0x4000)
// must be bigger than 0x4000 !! ( for ZIP )

static int MyComp( const char* str1, const char* str2);

inline char tolower(const char toLower)
        {
            if ((toLower >= 'A') && (toLower <= 'Z'))
                return char(toLower + 0x20);
            return toLower;
        }

int MyComp( const char* str1, const char* str2)
{
	int i;

	i=0;

	while(1) {
		int c1,c2;

		c1=str1[i];
		c2=str2[i];

		if ((c1==0) && (c2==0)) {
			return 0;
		}

		c1=tolower(c1);
		c2=tolower(c2);

		if (c1=='\\') c1='/';
		if (c2=='\\') c2='/';

		if (c1>c2) {
			return 1;
		} 
		if (c1<c2) {
			return -1;
		} 
		i++;
	} 

	return -1;
}

int compare( const void *arg1, const void *arg2 )
{
  struct zipdir *alb1, *alb2;

  alb1=(struct zipdir*)arg1;
  alb2=(struct zipdir*)arg2;

  return MyComp( alb1->name, alb2->name);
}


CZipTool::CZipTool(unsigned char *buffer, u32 size)
{
	kzip=buffer;
	mzip=size;

	cached=NULL;

	zipdirc=NULL;
	zipnbrc=-1;
	
	kout=NULL;
	common_buf=NULL;

	szip=size;
	
	pathInit();
	common_buf = (unsigned char*)malloc(XACR_BUFSIZE*sizeof(unsigned char));

	ReadDir();

	open=1;
}




void CZipTool::ReadDir(void)
{
	int max=100;

	if (zipnbrc!=-1) {
		return;
	}

	zipdirc=(struct zipdir*)malloc(sizeof(struct zipdir)*max);
	
	zipdirc=NULL;
	zipnbrc=-1;

	ZipLocalHeader hdr;

	pzip=0;	
	pout=0;

	zipnbrc=0;

	zipdirc=(struct zipdir*)malloc(sizeof(struct zipdir)*max);
	
	while(1)
	{
		int oldpos;

		oldpos=pzip;

		if (doHeader(&hdr)==0) break;
		if (zipdirc==NULL) {
			zipnbrc=0;
			break;
		}
		zipdirc[zipnbrc].name = (char*)malloc(strlen(hdr.fnm)+1);
		strcpy(zipdirc[zipnbrc].name, hdr.fnm);
		zipdirc[zipnbrc].position=oldpos;

		pzip = pzip + hdr.csz;
		zipnbrc++;

		if (zipnbrc>=max) {
			max+=100;
			zipdirc=(struct zipdir*)realloc(zipdirc, sizeof(struct zipdir)*max);
		}
	}

	qsort(zipdirc, zipnbrc, sizeof(struct zipdir), compare);

	return;
}

CZipTool::~CZipTool(void)
{
	if (common_buf != NULL) {
		free(common_buf);
	}
	if (cached != NULL) {
		free(cached);
	}
}

int CZipTool::kgetc(void)
{
	int a;

		if (pzip<mzip) {
			a=kzip[pzip];
			pzip++;
		} else {
			a=EOF;
		}

	return a;
}


int CZipTool::kread(unsigned char *dest, int count)
{
		if (pzip+count > mzip) {
			count=mzip-pzip;
		}
		
		memcpy(dest, kzip+pzip, count);
		pzip+=count;
	
	return count;
}

int CZipTool::kwrite(unsigned char *dest, int count)
{
	memcpy(kout+pout, dest, count);
	pout+=count;
	
	return count;
}


void CZipTool::initbits()
{
	bits_eof=false, bits_left=0, bitbuf=0;
}
int CZipTool::getbits( int n )
{
	if( n <= bits_left )
	{
		int c = (int)(bitbuf & ((1<<n)-1));
		bitbuf >>= n;
		bits_left -= n;
		return c;
	}
	return fillbits( n );
}
int CZipTool::fillbits( int n )
{
	u8 next;
	
	if( !zipread( &next,1 ) )
		bits_eof = true;
	else
	{
		bitbuf |= (next<<bits_left);
		bits_left += 8;
		
		if( zipread( &next,1 ) )
		{
			bitbuf |= (next<<bits_left);
			bits_left += 8;
		}
	}
	
	int c = (int)(bitbuf & ((1<<n)-1));
	bitbuf >>= n;
	bits_left -= n;
	return c;
}


int CZipTool::zipread( u8* dat, int len )
{
	len = kread( dat, len);
	return len;
}


void CZipTool::zipwrite( u8* dat, int len )
{
	kwrite( dat, len);
}



void CZipTool::pathInit(void)
{
}



void CZipTool::pathSplit( const char* path, int* y, int* d )
{
	*y=-1, *d=-1;
	for( const char* x=path; *x!='\0'; x++)
	{
		if( *x=='\\' || *x=='/' )	*y=x-path,*d=-1;
		else if( *x=='.' )			*d=x-path;
	}
}

const char* CZipTool::pathExt( const char* path )
{
	int y,d;
	pathSplit( path,&y,&d );
	return (d!=-1) ? path+d+1 : path+strlen(path);
}

const char* CZipTool::pathName( const char* path )
{
	int y,d;
	pathSplit( path,&y,&d );
	return path+y+1;
}


/*
bool Check( const char* fname, unsigned long fsize )
{
const unsigned char* hdr = common_buf;
unsigned long siz = (fsize>XACR_BUFSIZE ? XACR_BUFSIZE : fsize);
//--------------------------------------------------------------------//

  int i;
  for( i=0; i<(signed)siz-30; i++ )
  {
		if( hdr[i+0] != 'P'  )continue;
		if( hdr[i+1] != 'K'  )continue;
		if( hdr[i+2] != 0x03 )continue;
		if( hdr[i+3] != 0x04 )continue;
		if( hdr[i+8]+(hdr[i+9]<<8) > 12 )continue;
		if( hdr[i+26]==0 && hdr[i+27]==0 )continue;
		break;
		}
		if( (unsigned)i+30>=siz )
		return false;
		
		  if( !(zip=fopen( fname, "rb" )) )
		  return false;
		  fseek( zip, i+4, SEEK_SET );
		  ZipLocalHeader zhdr;
		  bool ans = read_header( &zhdr );
		  fclose( zip );
		  
			return ans;
			}
*/

bool CZipTool::ReadFromZIP(char *filter, u32 *dsize, unsigned char **dBuffer)
{
	ZipLocalHeader hdr;

	pzip=0;	
	pout=0;
	
	int i,j,k;
	
	i=0;
	j=zipnbrc-1;
	k=0;
	
	if (zipnbrc<=0) {
		*dBuffer=NULL;
		*dsize=0;
		
		return 0;
	}
	
	while(i<=j) {
		k=(i+j)/2;
		
		if (!MyComp(zipdirc[k].name, filter)) {
			break;
		}
		
		if (MyComp(zipdirc[k].name, filter)>0) {
			j=k-1;
		} else {
			i=k+1;
		}
	}
	
	if (!MyComp(zipdirc[k].name, filter)) {
		pzip=zipdirc[k].position;
		
		doHeader(&hdr);
		
		kout=(unsigned char*)malloc(hdr.usz);
		pout=0;
		mout=0; // ??
		
		switch( hdr.mhd )
		{
		case Stored:	 
			Unstore( hdr.usz, hdr.csz );	
			break;
		case Deflated:	 
			Inflate( hdr.usz, hdr.csz );	
			break;
		case Shrunk:	
			Unshrink( hdr.usz, hdr.csz );	
			break;
		case Reduced1:	
			Unreduce( hdr.usz, hdr.csz, 1 );	
			break;
		case Reduced2:	
			Unreduce( hdr.usz, hdr.csz, 2 );	
			break;
		case Reduced3:	
			Unreduce( hdr.usz, hdr.csz, 3 );	
			break;
		case Reduced4:	
			Unreduce( hdr.usz, hdr.csz, 4 );	
			break;
		case Imploded:	 
			Explode( hdr.usz, hdr.csz,	 0!=(hdr.flg&0x02), 0!=((hdr.flg)&0x04) );
			break;
		}
		
		if (pout!=0) {	
			*dBuffer=kout;
			*dsize=pout;
		} else {
			*dBuffer=NULL;
			*dsize=0;
		}
		
		
	} else {
		*dBuffer=NULL;
		*dsize=0;
		return false;
	}
	
	
	return true;
}






bool CZipTool::read_header( ZipLocalHeader* hdr)
{
	if( 26 != kread(common_buf, 26) ) { 
		return false; 
	}
	
	hdr->ver = ((common_buf[ 0])|(common_buf[ 1]<<8));
	hdr->flg = ((common_buf[ 2])|(common_buf[ 3]<<8));
	hdr->mhd = ((common_buf[ 4])|(common_buf[ 5]<<8));
	hdr->tim = ((common_buf[ 6])|(common_buf[ 7]<<8));
	hdr->dat = ((common_buf[ 8])|(common_buf[ 9]<<8));
	hdr->crc = ((common_buf[10])|(common_buf[11]<<8)|(common_buf[12]<<16)|(common_buf[13]<<24));
	hdr->csz = ((common_buf[14])|(common_buf[15]<<8)|(common_buf[16]<<16)|(common_buf[17]<<24));
	hdr->usz = ((common_buf[18])|(common_buf[19]<<8)|(common_buf[20]<<16)|(common_buf[21]<<24));
	hdr->fnl = ((common_buf[22])|(common_buf[23]<<8));  // Longueur du filename
	hdr->exl = ((common_buf[24])|(common_buf[25]<<8));

	if (hdr->fnl>=256) {
		return false;
	}
	
	if( hdr->fnl!=kread((unsigned char*)hdr->fnm, hdr->fnl) ) {  // common_buf <- filename
		return false; 
	}
	
	hdr->fnm[hdr->fnl]=0;

	if( hdr->mhd > Deflated || hdr->mhd==Tokenized ){ 
		return false; 
	}
	if ((hdr->exl!=0) && (hdr->exl != kread(common_buf, hdr->exl)) ) { 
		return false; 
	}
	
	return true;
}

bool CZipTool::doHeader( ZipLocalHeader* hdr)
{
	unsigned char key[4];

	kread(key, 4);

	if ( (key[0]=='P') && (key[1]=='K') && (key[2]==0x03) && (key[3]==0x04) ) 	{
		int x=pzip;
		if (read_header(hdr)) {
			return true;
		}
		pzip = x;
	}
	
	return false;
}

////////////////////////////////////////////////////////////////
// Store
////////////////////////////////////////////////////////////////

void CZipTool::Unstore( u32 usz, u32 csz )
{
	unsigned char* buf = common_buf;
	//--------------------------------------------------------------------
	
	int how_much;
	while( csz )
	{
		how_much = csz > XACR_BUFSIZE ? XACR_BUFSIZE : csz;
		if( 0>=(how_much=zipread( buf, how_much )) )
			break;
		zipwrite( buf, how_much );
		csz -= how_much;
	}
}

////////////////////////////////////////////////////////////////
// Deflate
////////////////////////////////////////////////////////////////

void CZipTool::Inflate( u32 usz, u32 csz )
{
	unsigned char* outbuf = common_buf;
	unsigned char*  inbuf = common_buf + (XACR_BUFSIZE/2);
	//--------------------------------------------------------------------//
	
	// zlib
	z_stream_s zs;
	zs.zalloc   = NULL;
	zs.zfree    = NULL;
	
	int outsiz = (XACR_BUFSIZE/2);
	zs.next_out = outbuf;
	zs.avail_out= outsiz;
	
	int insiz = zipread( inbuf,
		(XACR_BUFSIZE/2) > csz ? csz : (XACR_BUFSIZE/2) );
	if( insiz<=0 )
		return;
	csz        -= insiz;
	zs.next_in  = inbuf;
	zs.avail_in = insiz;
	
	inflateInit2( &zs, -15 );
	
	int err = Z_OK;
	while( csz )
	{
		while( zs.avail_out > 0 )
		{
			err = inflate( &zs,Z_PARTIAL_FLUSH );
			if( err!=Z_STREAM_END && err!=Z_OK )
				csz=0;
			if( !csz )
				break;
			
			if( zs.avail_in<=0 )
			{
				int insiz = zipread( inbuf, (XACR_BUFSIZE/2) > csz ? csz : (XACR_BUFSIZE/2) );
				if( insiz<=0 )
				{
					err = Z_STREAM_END;
					csz = 0;
					break;
				}
				
				csz        -= insiz;
				zs.next_in  = inbuf;
				zs.avail_in = insiz;
			}
		}
		
		zipwrite( outbuf, outsiz-zs.avail_out );
		zs.next_out  = outbuf;
		zs.avail_out = outsiz;
	}
	
	while( err!=Z_STREAM_END )
	{
		err = inflate(&zs,Z_PARTIAL_FLUSH);
		if( err!=Z_STREAM_END && err!=Z_OK )
			break;
		
		zipwrite( outbuf, outsiz-zs.avail_out );
		zs.next_out  = outbuf;
		zs.avail_out = outsiz;
	}
	
	inflateEnd(&zs);
}

////////////////////////////////////////////////////////////////
// Shrink LZW with partial_clear PkZip 0.x-1.x
////////////////////////////////////////////////////////////////

void CZipTool::Unshrink( u32 usz, u32 csz )
{
	// 8192Bytes = 13bits
	unsigned char* stack     = common_buf;
	unsigned char* suffix_of = common_buf + 8192 + 1;
	signed int*  prefix_of = (signed int*)common_buf + (8192 + 1) * 2;
	
#define GetCode() getbits(codesize)
	int left=(signed)usz-1;
	//--------------------------------------------------------------------
	
	
	u32 codesize, maxcode, free_ent, offset, sizex;
	u32 code, stackp, finchar, oldcode, incode;
	
	initbits();
	codesize = 9; 
	maxcode  = (1<<codesize) - 1;
	free_ent = 257; 
	offset   = 0;
	sizex    = 0;
	
	for( code=(1<<13); code > 255; code-- )
		prefix_of[code] = -1;
	for( code=255; code >= 0; code-- )
	{
		prefix_of[code] = 0;
		suffix_of[code] = code;
	}
	
	oldcode = GetCode();
	if( bits_eof )
		return;
	finchar = oldcode;
	u8 f=finchar;
	zipwrite( &f,1 );
	
	stackp = 8192;
	
	while( left>0 && !bits_eof )
	{
		code = GetCode();
		if( bits_eof )
			break;
		
		// clear!
		while( code == 256 )
		{
			switch( GetCode() )
			{
			case 1:
				codesize++;
				if( codesize == 13 )
					maxcode = (1 << codesize);
				else
					maxcode = (1 << codesize) - 1;
				break;
				
			case 2: // partial_clear !!
				{
					u32 pr,cd;
					
					for( cd=257; cd<free_ent; cd++ )
						prefix_of[cd] |= 0x8000;
					
					for( cd=257; cd<free_ent; cd++)
					{
						pr = prefix_of[cd] & 0x7fff;
						if( pr >= 257 )
							prefix_of[pr] &= 0x7fff;
					}
					
					for( cd=257; cd<free_ent; cd++ )
						if( (prefix_of[cd] & 0x8000) != 0 )
							prefix_of[cd] = -1;
						
						cd = 257;
						while( (cd < (1<<13)) && (prefix_of[cd] != -1) )
							cd++;
						free_ent = cd;
				}
				break;
			}
			
			code = GetCode();
			if( bits_eof )
				break;
		}
		if( bits_eof )
			break;
		
		incode = code;
		if( prefix_of[code] == -1 )
		{
            stack[--stackp] = finchar;
			code = oldcode;
		}
		
		while( code >= 257 )
		{
			stack[--stackp] = suffix_of[code];
			code = prefix_of[code];
		}
		finchar = suffix_of[code];
		stack[--stackp] = finchar;
		
		left -= (8192-stackp);
		zipwrite( stack+stackp, (8192-stackp) );
		stackp = 8192;
		
		
		code = free_ent;
		if( code < (1<<13) )
		{
			prefix_of[code] = oldcode;
			suffix_of[code] = finchar;
			
			do
			code++;
			while( (code < (1<<13)) && (prefix_of[code] != -1) );
			
			free_ent = code;
		}
		
		
		oldcode = incode;
	}
	
#undef GetCode
}

////////////////////////////////////////////////////////////////
// Reduce lz77 ; PkZip 0.x
////////////////////////////////////////////////////////////////

void CZipTool::LoadFollowers( u8* Slen, u8 followers[][64] )
{
	for( int x=255; x>=0; x-- )
	{
		Slen[x] = getbits(6);
		for( int i=0; i<Slen[x]; i++ )
			followers[x][i] = getbits(8);
	}
}

void CZipTool::Unreduce( u32 usz, u32 csz, int factor )
{
	unsigned char* outbuf = common_buf;
	unsigned char* outpos=outbuf;
	memset( outbuf,0,0x4000 );
	int left = (signed)usz;
#define RED_FLUSH() zipwrite(outbuf,outpos-outbuf), outpos=outbuf;
#define RED_OUTC(c) {(*outpos++)=c; left--; if(outpos-outbuf==0x4000) RED_FLUSH();}
	//-------------------------------------------------------------------
	
	static const int Length_table[] = {0, 0x7f, 0x3f, 0x1f, 0x0f};
	static const int D_shift[] = {0, 0x07, 0x06, 0x05, 0x04};
	static const int D_mask[]  = {0, 0x01, 0x03, 0x07, 0x0f};
	static const int B_table[] = {
		8, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5,
			5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8};
		
		initbits();
		
		u8 followers[256][64];
		u8 Slen[256];
		LoadFollowers( Slen, followers );
		
		int l_stage = 0;
		int l_char  = 0;
		int ch, V=0, Len=0;
		
		while( !bits_eof && left>0 )
		{
			if( Slen[l_char] == 0 )
				ch = getbits(8);
			else
			{
				if( getbits(1) )
					ch = getbits(8);
				else
				{
					int bitsneeded = B_table[ Slen[l_char] ];
					ch = followers[ l_char ][ getbits(bitsneeded) ];
				}
			}
			
			// Repeater Decode
			switch( l_stage )
			{
			case 0:
				if( ch == 0x90 )
					l_stage++;
				else			
					RED_OUTC( ch )
					break;
				
			case 1:
				if( ch == 0x00 )
				{
					RED_OUTC(0x90)
						l_stage = 0;
				}
				else			
				{
					V   = ch;
					Len = V & Length_table[factor];
					if( Len == Length_table[factor] )
						l_stage = 2;
					else
						l_stage = 3;
				}
				break;
				
			case 2:				
				Len += ch;
				l_stage++;
				break;
				
			case 3:				
				{
					int i = Len+3;
					int offset = (((V>>D_shift[factor]) & D_mask[factor]) << 8) + ch + 1;
					int op = (((outpos-outbuf)-offset) & 0x3fff);
					
					while( i-- )
					{
						RED_OUTC( outbuf[op++] );
						op &= 0x3fff;
					}
					
					l_stage = 0;
				}
				break;
			}
			l_char = ch;
		}
		
		RED_FLUSH();
		
#undef RED_FLUSH
#undef RED_OUTC
}

////////////////////////////////////////////////////////////////
// Implode lz77 + shanon-fano ? PkZip 1.x
////////////////////////////////////////////////////////////////

void CZipTool::Explode( u32 usz, u32 csz, bool _8k, bool littree )
{
	unsigned char* outbuf = common_buf;
	unsigned char* outpos=outbuf;
	memset( outbuf,0,0x4000 );
	int left = (signed)usz;

#define EXP_FLUSH() zipwrite(outbuf,outpos-outbuf), outpos=outbuf;
#define EXP_OUTC(c) {(*outpos++)=c; left--; if(outpos-outbuf==0x4000) EXP_FLUSH();}
	//--------------------------------------------------------------------
	
	u8 ch;
	initbits();
	
	int dict_bits     = ( _8k ? 7 : 6);
	int min_match_len = (littree ? 3 : 2);
	sf_tree lit_tree; 
	sf_tree length_tree; 
	sf_tree distance_tree; 
	
	if( littree ) 
		LoadTree( &lit_tree, 256 );
	LoadTree( &length_tree, 64 ); 
	LoadTree( &distance_tree, 64 ); 
	
	while( !bits_eof && left>0 )
	{
		if( getbits(1) ) 
		{
			if( littree )
				ch = ReadTree( &lit_tree );
			else
				ch = getbits(8);
			
			EXP_OUTC( ch );
		}
		else 
		{
			int Distance = getbits(dict_bits);
			Distance |= ( ReadTree(&distance_tree) << dict_bits );
			
			int Length = ReadTree( &length_tree );
			
			if( Length == 63 )
				Length += getbits(8);
			Length += min_match_len;
			
			int op = (((outpos-outbuf)-(Distance+1)) & 0x3fff);
			while( Length-- )
			{
				EXP_OUTC( outbuf[op++] );
				op &= 0x3fff;
			}
		}
	}
	
	EXP_FLUSH();
	
#undef EXP_OUTC
#undef EXP_FLUSH
}

void CZipTool::SortLengths( sf_tree* tree )
{ 
	int gap,a,b;
	sf_entry t; 
	bool noswaps;
	
	gap = tree->entries >> 1; 
	
	do
	{
		do
		{
			noswaps = true;
			for( int x=0; x<=(tree->entries - 1)-gap; x++ )
			{
				a = tree->entry[x].BitLength;
				b = tree->entry[x + gap].BitLength;
				if( (a>b) || ((a==b) && (tree->entry[x].Value > tree->entry[x + gap].Value)) )
				{
					t = tree->entry[x];
					tree->entry[x] = tree->entry[x + gap];
					tree->entry[x + gap] = t;
					noswaps = false;
				}
			}
		} while (!noswaps);
		
		gap >>= 1;
	} while( gap > 0 );
}

void CZipTool::ReadLengths( sf_tree* tree )
{ 
	int treeBytes,i,num,len;
	
	treeBytes = getbits(8) + 1;
	i = 0; 
	
	tree->MaxLength = 0;
	
	while( treeBytes-- )
	{
		len = getbits(4)+1;
		num = getbits(4)+1;
		
		while( num-- )
		{
			if( len > tree->MaxLength )
				tree->MaxLength = len;
			tree->entry[i].BitLength = len;
			tree->entry[i].Value = i;
			i++;
		}
	} 
} 

void CZipTool::GenerateTrees( sf_tree* tree )
{ 
	u16 Code = 0;
	int CodeIncrement = 0, LastBitLength = 0;
	
	int i = tree->entries - 1;   // either 255 or 63
	while( i >= 0 )
	{
		Code += CodeIncrement;
		if( tree->entry[i].BitLength != LastBitLength )
		{
			LastBitLength = tree->entry[i].BitLength;
			CodeIncrement = 1 << (16 - LastBitLength);
		}
		
		tree->entry[i].Code = Code;
		i--;
	}
}

void CZipTool::ReverseBits( sf_tree* tree )
{
	for( int i=0; i<=tree->entries-1; i++ )
	{
		u16 o = tree->entry[i].Code,
			v = 0,
			mask = 0x0001,
			revb = 0x8000;
		
		for( int b=0; b!=16; b++ )
		{
			if( (o&mask) != 0 )
				v = v | revb;
			
			revb = (revb >> 1);
			mask = (mask << 1);
		}
		
		tree->entry[i].Code = v;
	}
}

void CZipTool::LoadTree( sf_tree* tree, int treesize )
{ 
	tree->entries = treesize; 
	ReadLengths( tree ); 
	SortLengths( tree ); 
	GenerateTrees( tree ); 
	ReverseBits( tree ); 
} 

int CZipTool::ReadTree( sf_tree* tree )
{ 
	int bits=0, cur=0, b;
	u16 cv=0;
	
	while( true )
	{
		b = getbits(1);
		cv = cv | (b << bits);
		bits++;
		
		while( tree->entry[cur].BitLength < bits )
		{
			cur++;
			if( cur >= tree->entries )
				return -1;
		}
		
		while( tree->entry[cur].BitLength == bits )
		{
			if( tree->entry[cur].Code == cv )
				return tree->entry[cur].Value;
			cur++;
			if( cur >= tree->entries )
				return -1;
		}
	}
}

// ----

void FS_zipgetFileList(FS_AddFile AddFile, char *zipfile, u8 *zipbuf, u32 zipsize)
{          
CZipTool *SZ;
char file[256];
int i;
SZ = new CZipTool(zipbuf, zipsize);
for(i=0;i<SZ->zipnbrc;i++) {
    sprintf(file,"%s/%s", zipfile, SZ->zipdirc[i].name);
    strtolower(file);
    AddFile(1, file);
    }  
}

u8 *unzip(u8 *zipbuf, u32 zipsize, char *filename, u32 *size)
{
CZipTool *SZ;
u8 *dBuffer;
SZ = new CZipTool(zipbuf, zipsize);
SZ->ReadFromZIP(filename, size, &dBuffer);
return dBuffer;
}

// -- INDEP TOOLS ----------------------------------------------------------------
