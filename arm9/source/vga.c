/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!VGA!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*          SYSTEME         |      SOUS SYSTEME       |      SOUS ENSEMBLE
* ------------------------------------------------------------------------------
*  EMULATEUR CPC           | WIN-CPC                 | Chips
* ------------------------------------------------------------------------------
*  Fichier     : VGA.C                 | Version : 0.1p
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Emulation du circuit VGA
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
********************************************************** !END! **************/

#include <nds.h>

#include  "types.h"
#include  "config.h"
#ifdef USE_LOG
#include  "log.h"
#endif
#include  "vga.h"

#include "romdisc_bin.h"

#include "cpc6128_bin.h"

#include "z80.h"

#include "plateform.h"

extern int CntHSync;

u8 *TabPOKE[4];
u8 *TabPEEK[4];

/********************************************************* !NAME! **************
* Nom : MemCPC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Mémoire du CPC
*
********************************************************** !0! ****************/
UBYTE *MemCPC;

// FAST RAM

DTCM_DATA u8 TabCoul[ 32 ];  // Couleurs R,V,B


/********************************************************* !NAME! **************
* Nom : RamSelect
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Numéro de ram sélectionnée (128K)
*
********************************************************** !0! ****************/
int RamSelect;


/********************************************************* !NAME! **************
* Nom : lastMode
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Mode écran sélectionné
*
********************************************************** !0! ****************/
int lastMode;


/********************************************************* !NAME! **************
* Nom : DecodeurAdresse
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : ROMs activées/désactivées
*
********************************************************** !0! ****************/
int DecodeurAdresse;


/********************************************************* !NAME! **************
* Nom : RomExt
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Numéro de rom supérieure sélectionnée
*
********************************************************** !0! ****************/
int RomExt;


/********************************************************* !NAME! **************
* Nom : PenSelect
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Numéro de stylot sélectionné
*
********************************************************** !0! ****************/
int PenSelect=0;


/********************************************************* !NAME! **************
* Nom : ROMINF
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Rom inférieure
*
********************************************************** !0! ****************/
// static u8 *ROMINF;
static UBYTE ROMINF[ 0x4000 ];


/********************************************************* !NAME! **************
* Nom : ROMSUP
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Rom supérieure
*
********************************************************** !0! ****************/
// static u8 *ROMSUP;
static UBYTE ROMSUP[ 0x4000 ];


/********************************************************* !NAME! **************
* Nom : ROMDISC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Rom disque (Amsdos)
*
********************************************************** !0! ****************/
// static u8 *ROMDISC;
static UBYTE ROMDISC[ 0x4000 ];



/********************************************************* !NAME! **************
* Nom : AdjRam
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Configuration de la ram en fonction du registre de sélection
*
********************************************************** !0! ****************/
static int AdjRam[4][8] =
    {
        {0x00000, 0x00000, 0x10000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000},
        {0x04000, 0x04000, 0x14000, 0x0C000, 0x10000, 0x14000, 0x18000, 0x1C000},
        {0x08000, 0x08000, 0x18000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000},
        {0x0C000, 0x1C000, 0x1C000, 0x1C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000}
    };


/********************************************************* !NAME! **************
* Nom : SetMemCPC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Mapping de la mémoire du CPC en fonction de la sélection des
*               roms et rams
*
* Résultat    : /
*
* Variables globales modifiées : TabPOKE, TabPEEK
*
********************************************************** !0! ****************/
static void SetMemCPC( void )
{
    /*
    static int n=0;
    myprintf("SetMemCPC %d", n);
    n++;
    */

    TabPOKE[ 0 ] = &MemCPC[ AdjRam[ 0 ][ RamSelect ] ];
    TabPEEK[ 0 ] = ( DecodeurAdresse & ROMINF_OFF ) ? &MemCPC[ AdjRam[ 0 ][ RamSelect ] ] : ROMINF;

    TabPOKE[ 1 ] =
    TabPEEK[ 1 ] = (&MemCPC[ AdjRam[ 1 ][ RamSelect ] ]) - 0x4000;

    TabPOKE[ 2 ] =
    TabPEEK[ 2 ] = (&MemCPC[ AdjRam[ 2 ][ RamSelect ] ]) - 0x8000;

    TabPOKE[ 3 ] = (&MemCPC[ AdjRam[ 3 ][ RamSelect ] ]) - 0xC000;
    
    TabPEEK[ 3 ] = (( DecodeurAdresse & ROMSUP_OFF ) ? &MemCPC[ AdjRam[ 3 ][ RamSelect ] ] : ( RomExt == 7 ) ?  ROMDISC : ROMSUP) - 0xC000;
    
//    memcpy(z80Bank + 0*16384, &MemCPC[ AdjRam[ 0 ][ RamSelect ] ]
}


/********************************************************* !NAME! **************
* Nom : WriteVGA
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un registre du VGA
*
* Résultat    : /
*
* Variables globales modifiées : PenSelect, TabCoul, TabInk, DecodeurAdresse,
*                                lastMode, CntHSync, RamSelect
*
********************************************************** !0! ****************/
void WriteVGA( u8 val )
{
    u8 newVal = val & 0x1F;

//    myprintf("VGA: %d %d", val&0x1F, val >> 6);

    switch( val >> 6 )
        {
        case 0 :
            if ((newVal&0x10)==0) {
                PenSelect = newVal & 0x0f;
            } else {
                PenSelect = 0x10;
            }
            break;

        case 1 :
            if (TabCoul[ PenSelect ] != newVal) {
                TabCoul[ PenSelect ] = newVal;
                UpdateInk=1;
                }
            break;

        case 2 :
            DecodeurAdresse = val;
            lastMode = val & 3;
            SetMemCPC();
            if ( val & 16 ) {
                CntHSync = 0;
                SetIRQZ80(0);
                }
            UpdateInk=1;
            break;

        case 3 :
            RamSelect = val & 7;
            SetMemCPC();
            break;
        }
}


/********************************************************* !NAME! **************
* Nom : WriteROM
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Sélection du numéro de rom supérieure
*
* Résultat    : /
*
* Variables globales modifiées : RomExt
*
********************************************************** !0! ****************/
void WriteROM( int val )
{
    RomExt = val;
    SetMemCPC();
}


/********************************************************* !NAME! **************
* Nom : InitMemCPC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialisation : lecture des roms du cpc
*
* Résultat    : TRUE si ok, FALSE sinon
*
* Variables globales modifiées : ROMINF, ROMSUP, ROMDISC
*
********************************************************** !0! ****************/
BOOL InitMemCPC(void)
{
    MemCPC = MyAlloc(0x20000, "Memory CPC"); // 128Ko
    
    // memcpy(ROMINF, rominf_bin, sizeof(ROMINF));
    // memcpy(ROMSUP, romsup_bin, sizeof( ROMSUP ));
    /* 
    ROMINF = (u8*)cpc6128_bin;
    ROMSUP = ((u8*)cpc6128_bin) + 16384;
    ROMDISC =  (u8*)romdisc_bin;
    */
    
    memcpy(ROMINF, cpc6128_bin, sizeof(ROMINF));
    memcpy(ROMSUP, cpc6128_bin + 0x4000, sizeof(ROMSUP));
    memcpy(ROMDISC, romdisc_bin, sizeof(ROMDISC));

    emulator_patch_ROM(ROMINF); // Patch de la langue (entre autres)

    WriteVGA(0x89);
    WriteVGA(0xC0);

    return( TRUE );
}
