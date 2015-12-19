/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!UPD 765!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1w
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module UPD.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  18/02/2003              | L.DEPLANQUE             | Version 0.1w : prise en
*                          |                         | compte de l'information
*                          |                         | disc missing dans le
*                          |                         | registre ST0
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef UPD_H
#define UPD_H

extern int DriveBusy;

void ChangeCurrTrack( int newTrack );
void ReadCHRN( void );
int RechercheSecteur( int newSect, int * pos );

int ReadUPD( int port );

void WriteUPD( int Port, int val );

void ResetUPD( void );

void SetDiskUPD( char * );

void EjectDiskUPD( void );

int GetCurrTrack( void );

void LireDiskMem(u8 *rom, u32 romsize);

/********************************************************* !NAME! **************
* Nom : CPCEMUEnt
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : En-tête d'un fichier DSK
*
********************************************************** !0! ****************/
#pragma pack(1)
typedef struct
    {
    char  debut[ 0x30 ]; // "MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
    UBYTE NbTracks;
    UBYTE NbHeads;
    SHORT DataSize; // 0x1300 = 256 + ( 512 * nbsecteurs )
    UBYTE Unused[ 0xCC ];
    } CPCEMUEnt;


/********************************************************* !NAME! **************
* Nom : CPCEMUSect
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Description d'un secteur
*
********************************************************** !0! ****************/
typedef struct
    {
    UBYTE C;                // track
    UBYTE H;                // head
    UBYTE R;                // sect
    UBYTE N;                // size
    UBYTE ST1;              // Valeur ST1
    UBYTE ST2;              // Valeur ST2
    SHORT SectSize;         // Taille du secteur en octets
    } CPCEMUSect;


/********************************************************* !NAME! **************
* Nom : CPCEMUTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Description d'une piste
*
********************************************************** !0! ****************/
typedef struct
    {
    char        ID[ 0x10 ];   // "Track-Info\r\n"
    UBYTE       Track;
    UBYTE       Head;
    SHORT       Unused;
    UBYTE       SectSize; // 2
    UBYTE       NbSect;   // 9
    UBYTE       Gap3;     // 0x4E
    UBYTE       OctRemp;  // 0xE5
    CPCEMUSect  Sect[ 29 ];
    } CPCEMUTrack;
#pragma pack()


#endif
