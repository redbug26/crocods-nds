/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!CRTC 6845!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1r
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module CRTC.C
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
********************************************************** !END! **************/


#ifndef CRTC_H
#define CRTC_H


extern int RegsCRTC[ 32 ];

extern int OfsEcr;

extern int RegCRTCSel;

extern USHORT TabAdrCrtc[ 0x10000 ];
extern int VSync;


extern int XStart;

extern int XEnd;

extern int DoResync;


void WriteCRTC( u16 adr, u8 val );

int CalcCRTCLine( void );

#endif

void ResetCRTC( void );

