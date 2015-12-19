/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!VGA!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1p
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module VGA.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef VGA_H
#define VGA_H


#define   ROMINF_OFF        0x04
#define   ROMSUP_OFF        0x08


extern u8 *MemCPC;

extern int adrEcr, DecodeurAdresse, RomExt, lastMode;

extern u8 TabCoul[ 32 ];

extern int RamSelect, PenSelect;

extern u16 RgbCPC[ 32 ];

extern u8 *TabPOKE[4];
extern u8 *TabPEEK[4];




void WriteVGA( u8 val );

void WriteROM( int val );

BOOL InitMemCPC( void );


#endif
