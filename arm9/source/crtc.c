/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!PC-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!CRTC 6845!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*          SYSTEME         |      SOUS SYSTEME       |      SOUS ENSEMBLE
* ------------------------------------------------------------------------------
*  EMULATEUR CPC           | PC-CPC                  | Chips
* ------------------------------------------------------------------------------
*  Fichier     : CRTC.C                | Version : 0.1am
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Emulation du CRTC 6845
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  13/11/2002              | L.DEPLANQUE             | Changement du type de
*                          |                         | la variable TabAdrCrtc : 
*                          |                         | passage en USHORT, pour
*                          |                         | optimiser le memset
*                          |                         | dans CalcAdrEcrCPC()
* ------------------------------------------------------------------------------
*  08/02/2006              | L.DEPLANQUE             | Version 0.1ak :
*                          |                         | Réécriture complète du
*                          |                         | code
* ------------------------------------------------------------------------------
*  06/12/2006              | L.DEPLANQUE             | Version 0.1am :
*                          |                         | Variable "VBL" remplacée
*                          |                         | par "VSync" et supprimée
*                          |                         | du module PPI.
*                          |                         | Correction problème de
*                          |                         | synchro multiples.
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#include  "types.h"
#include  "plateform.h"
#include  "z80.h"


/********************************************************* !NAME! **************
* Nom : RegsCRTC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Registres du CRTC
*
********************************************************** !0! ****************/
int RegsCRTC[ 32 ];


/********************************************************* !NAME! **************
* Nom : RegCRTCSel
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Numéro du registre CRTC sélectionné
*
********************************************************** !0! ****************/
int RegCRTCSel;


/********************************************************* !NAME! **************
* Nom : XStart
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Position début (en octets) de l'affichage vidéo sur une ligne
*
********************************************************** !0! ****************/
int XStart=8;


/********************************************************* !NAME! **************
* Nom : XEnd
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Position fin (en octets) de l'affichage vidéo sur une ligne
*
********************************************************** !0! ****************/
int XEnd;


/********************************************************* !NAME! **************
* Nom : CntHSync
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\\/FLE!, ligne : !./LN!
*
* Description : Compteur position horizontale raster
*
********************************************************** !0! ****************/
int CntHSync = 0;


/********************************************************* !NAME! **************
* Nom : VSync
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Signal VSync
*
********************************************************** !0! ****************/
int VSync;


/********************************************************* !NAME! **************
* Nom : DoResync
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Indique qu'il faut synchroniser l'affichage
*
********************************************************** !0! ****************/
int DoResync;


/********************************************************* !NAME! **************
* Nom : ResetCRTC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Effectue un reset du CRTC
*
* Résultat    : /
*
* Variables globales modifiées : RegsCRTC
*
********************************************************** !0! ****************/
void ResetCRTC( void )
{
    memset( RegsCRTC, -1, sizeof( RegsCRTC ) );

    RegsCRTC[0]=0;
}


/********************************************************* !NAME! **************
* Nom : WriteCRTC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un registre du CRTC
*
* Résultat    : /
*
* Variables globales modifiées : RegsCRTC, RegCRTCSel, XStart, XEnd
*
********************************************************** !0! ****************/
void WriteCRTC( u16 adr, u8 val )
{
    adr &= 0xBF00;
    if (adr == 0xBC00) {
        RegCRTCSel = val & 0x1F;  
    } else {
        if (adr == 0xBD00) {
            switch(RegCRTCSel) {    // only registers 0 - 15 can be written to
                case 1:  // horizontal displayed
                    RegsCRTC[RegCRTCSel] = val;
                    break;                    
                case 2:  // horizontal sync position
                    RegsCRTC[RegCRTCSel] = val;
                    break;                    
                case 8:  // interlace and skew                    
                    val &= 0xF3; // sur PreAsic, Asic, HD6845S
                    // val &= 0x03;  // 03 sur HD6845R, UM6845R, MC6845
                    RegsCRTC[RegCRTCSel] = val;
                    break;                    
                case 0:  // horizontal total                
                    RegsCRTC[0] = val;
                    break;                
                case 3:  // sync width                
                case 13: // start address low byte
                case 15: // cursor address low byte
                    RegsCRTC[RegCRTCSel] = val;
                    break;                
                case 4:  // vertical total
                    val &= 0x7F; // sur PreAsic, Asic, HD6845S
                    // val &= 0x0F; // sur HD6845R, UM6845R, MC6845
                    RegsCRTC[RegCRTCSel] = val;
                    break;
                case 6:  // vertical displayed
                case 7:  // vertical sync position
                case 10: // cursor start raster
                    val &= 0x7F;
                    RegsCRTC[RegCRTCSel] = val;
                    break;
                case 5: // vertical total adjust
                case 9: // maximum raster count
                case 11: // cursor end raster
                    val &= 0x1F;
                    RegsCRTC[RegCRTCSel] = val;
                    break;
                case 12: // start address high byte
                case 14: // cursor address high byte
                    val &= 0x3F;
                    RegsCRTC[RegCRTCSel] = val;
                    break;
            }
        }
    }
    XStart = max( ( 50 - RegsCRTC[ 2 ] ) << 1, 0 );                    
    XEnd = min( XStart + ( RegsCRTC[ 1 ] << 1 ), 96 );    
}


/********************************************************* !NAME! **************
* Nom : CalcCRTCLine
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Calcul et affiche une ligne CRTC
*
* Résultat    : La ligne suivante du CRTC à afficher
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
int CalcCRTCLine( void )
{
    static int TailleVBL = 16, CptCharVert = 0, NumLigneChar = 0;
    static int LigneCRTC = 0, MaCRTC = 0, SyncCount = 0;

    if ( ! --TailleVBL )
        VSync = 0;

    // if ( ++LigneCRTC >= RegsCRTC[ 5 ] )
    if ( ++LigneCRTC )
        {
        if ( NumLigneChar == RegsCRTC[ 9 ] )
            {
            NumLigneChar = 0;
            CptCharVert = ( CptCharVert + 1 ) & 0x7F;
            MaCRTC += RegsCRTC[ 1 ];
            }
        else
            NumLigneChar = ( NumLigneChar + 1 ) & 0x1F;

        if ( CptCharVert == RegsCRTC[ 4 ] + 1 )
            {
            NumLigneChar = 0; // ### Pas sur...
            CptCharVert = 0;
            MaCRTC = RegsCRTC[ 13 ] | ( RegsCRTC[ 12 ] << 8 );
            }
        if ( CptCharVert == RegsCRTC[ 7 ]  && ! NumLigneChar )
            {
            LigneCRTC = 0;
            TailleVBL = 16;
            SyncCount = 2;
            VSync = 1;
            DoResync = 1;
            }
        else
            {
            int y = LigneCRTC - 32;
            if ( y >= 0 && y < TAILLE_Y_LOW )
                TraceLigne8B512( y
                       , CptCharVert < RegsCRTC[ 6 ] ? MaCRTC << 1 : -1
                       , ( ( NumLigneChar ) << 11 ) | ( ( MaCRTC & 0x3000 ) << 2 )
                       );
            else
                if ( LigneCRTC > 312 )
                    {
                    LigneCRTC = 0;
                    DoResync = 0;
                    }
            }
        }
    CntHSync++;
    if ( ! SyncCount )
        {
        //
        // Si 52 lignes comptée -> Génération d'une interruption
        //
        if ( CntHSync == 52 )
            {
            CntHSync = 0;
            SetIRQZ80(1);
            }
        }
    else
        if ( ! --SyncCount )
            {
            if ( CntHSync & 32 )
                SetIRQZ80(1);

            CntHSync = 0;
            }
    return( LigneCRTC );
}
