#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <nds.h>
#include <nds/jtypes.h>
#include <nds/arm9/console.h> //basic print funcionality
#include <nds/arm9/image.h>

#include <sys/dir.h>

#include "fat.h"

#include "fs.h"
#include "plateform.h"
#include "ziptool.h"

int isFATSystem=0;

void strtolower(char *str)
{
int n;
n=0;
while(str[n]!=0) {
    if ((str[n] >= 'A') && (str[n] <= 'Z')) {
        str[n]+=0x20;
    }
    n++;
}
}

u8 *FS_Readfile(char *filename, u32 *romsize)
{   	
    u8 *rom=NULL;
	
    if (isFATSystem) {        
        char *slash;
        slash=strstr(filename,".zip/");
        if (slash!=NULL) {
            char zipfile[256], file[256], *sl;
            u32 zipsize;
            u8 *zipbuf;

            slash+=4;
			
            strcpy(zipfile,filename+1); // On retire le premier slash
            sl=strrchr(zipfile,'/');
            sl[0]=0;
            strcpy(file, slash+1);
			
			FILE *romfile;
			romfile=fopen(zipfile,"rb");
			
			if (romfile!=NULL) {
				fseek(romfile,0,SEEK_END);
				zipsize = ftell(romfile);
				
				zipbuf=(u8*)malloc(zipsize);
				
				fseek(romfile,0,SEEK_SET);
				fread(zipbuf,1,zipsize,romfile);
				fclose(romfile);

                rom=unzip(zipbuf, zipsize, file, romsize);
                free(zipbuf);
            } else {
				return NULL;
			}				
        } else {
			FILE *romfile;
			romfile=fopen(filename,"rb");
		
			if (romfile!=NULL) {
				fseek(romfile,0,SEEK_END);
				*romsize = ftell(romfile);
				
				rom=(u8*)malloc(*romsize);
				
				fseek(romfile,0,SEEK_SET);
				fread(rom,1,*romsize,romfile);
				
				fclose(romfile);
			}
		} 
    }
	
    if (*romsize==0) {
        rom=NULL;
    }
	
    return rom;
}

/*
typedef void ( * FS_AddFile )( int, char * );
void FS_zipgetFileList(FS_AddFile AddFile, char *fileName)
{
char file[256], zipfile[256];
int len;
strcpy(file, fileName);
strcpy(zipfile, fileName);
len=strlen(fileName);
fileName[len-4]=0;
sprintf(file,"%s/%s.sna", zipfile, fileName);
AddFile(1, file);
}
*/

void FS_getFileList_FAT(FS_AddFile AddFile, char *root) {
	char fileName[256]; 
    char fullfile[256];
	
	if (isFATSystem) { 
        DIR_ITER *entry;
        struct stat fs;

myprintf("Read FS");

        entry=diropen(root);
        if (entry!=NULL) 
		while (1) {
			int len;


            if (dirnext(entry, fileName, &fs)==-1) break;
myprintf("%s", fileName);
            strtolower(fileName);
			
            sprintf(fullfile, "%s%s", root, fileName);

			len=strlen(fullfile);
			if (len>4) {
				if (!strcasecmp(".zip",fullfile+len-4)) {
					u32 zipsize;
					u8 *zipbuf;
					
					FILE *romfile;
					romfile=fopen(fullfile,"rb");
					
					if (romfile!=NULL) {
						fseek(romfile,0,SEEK_END);
						zipsize = ftell(romfile);
						
						zipbuf=(u8*)malloc(zipsize);
						
						fseek(romfile,0,SEEK_SET);
						fread(zipbuf,1,zipsize,romfile);
						fclose(romfile);
					
                        FS_zipgetFileList(AddFile,  fullfile, zipbuf, zipsize);
					    free(zipbuf);
                    }
				}
			}
			AddFile(0, fullfile);
		}
        chdir("/");
	}
} 

void FS_getFileList(FS_AddFile AddFile) {
	if (isFATSystem) {
        FS_getFileList_FAT(AddFile, "/");
        FS_getFileList_FAT(AddFile, "/ROMS/CPC/");
	}
} 

void FS_Init(void)
{
// Map Game Cartridge memory to ARM9 (thnks to gpf)
    sysSetCartOwner( BUS_OWNER_ARM9 );

    isFATSystem = fatInitDefault();

    if (isFATSystem) {
        printf("fat initialise");
    } else {
        printf("fat init echec");
    }
}

void FS_Delete(char *filename)
{
	unlink(filename);
}

int FileExists(char *filename)
{
FILE *fic;
fic=fopen(filename,"rb");
if (fic==NULL) return 0;
fclose(fic);
return 1;
}
