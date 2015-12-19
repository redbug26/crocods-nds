
#define    USE_FAT
#undef    USE_GBFS

typedef void ( * FS_AddFile )( int, char * );

u8 *FS_Readfile(char *filename, u32 *romsize);
void FS_getFileList(FS_AddFile AddFile);
void FS_Init(void);
void FS_Delete(char *filename);

int FileExists(char *filename);


#ifdef __cplusplus
extern "C" {
#endif

void strtolower(char *str);

#ifdef __cplusplus
}
#endif



