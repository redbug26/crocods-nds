/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!Ports!
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
*  Fichier     : GESTPORT.C            | Version : 0.1x
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Gestion des ports d'entrée/sortie
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  09/01/2003              | L.DEPLANQUE             | Version 0.1u : Test
*                          |                         | nbre de ports écrits
*                          |                         | simultanéments (proto)
* ------------------------------------------------------------------------------
*  22/01/2004              | L.DEPLANQUE             | Version 0.1x : Ajout
*                          |                         | adresse du PC dans les
*                          |                         | traces du fichier .LOG
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#include  <nds.h>

#include  "types.h"
#include  "plateform.h"
#include  "config.h"
#include  "ppi.h"
#include  "crtc.h"
#include  "upd.h"
#include  "vga.h"


/********************************************************* !NAME! **************
* Nom : ReadPort
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un port d'entrée
*
* Résultat    : La valeur du port selectionné
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
u8 ReadPort( int port )
{
    if ( ! ( port & 0x0480 ) )
        return( ReadUPD( port ) );

    if ( ! ( port & 0x0800 ) )
        return( ReadPPI( port ) );

    // myprintf("Accès lecture port %04X", port);
    return( 0xFF );
}


/********************************************************* !NAME! **************
* Nom : WritePort
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un port de sortie
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void WritePort( u16 port, u8 val )
{
    //char chaine[256];
    //sprintf(chaine, "P%04X %d", port, val);
    if ( ( port & 0xC000 ) == 0x04000 ) {
        // strcat(chaine, " VGA");
        WriteVGA( val );
        }

    if ( ! ( port & 0x4000 ) ) {
        // strcat(chaine, " CRC");
        WriteCRTC( port, val );
        }

    if ( ! ( port & 0x2000 ) ) {
        // strcat(chaine, " ROM");
        WriteROM( val );
        }

    if ( ! ( port & 0x1000 ) ) {
        // strcat(chaine, " PRN");
    //    PrintVal( val );
        }

    if ( ! ( port & 0x0800 ) ) {
        // strcat(chaine, " PPI");
        WritePPI( port, val );
        }

    if ( ! ( port & 0x0480 ) ) {
        // strcat(chaine, " UPD");
        WriteUPD( port, val );
        }

    // myprintf(chaine);
}

