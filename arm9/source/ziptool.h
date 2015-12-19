/* ziptool.h internal header */
#ifndef _ZIPTOOL
#define _ZIPTOOL

#ifdef __cplusplus
extern "C" {
#endif 

u8 *unzip(u8 *zipbuf, u32 zipsize, char *filename, u32 *size);
void FS_zipgetFileList(FS_AddFile AddFile, char *zipfile, u8 *zipbuf, u32 zipsize);

#ifdef __cplusplus
}
#endif 


#endif
