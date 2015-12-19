/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!PPI 8255!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1p
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module PPI.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  07/11/2002              | L.DEPLANQUE             | Ajout gestion lecture et
*                          |                         | écriture cassette.:
*                          |                         | fonctions WriteCas() et
*                          |                         | ReadCas().
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef PPI_H
#define PPI_H


extern u8 clav[ 16 ];

extern int RegsPPI[ 4 ];

extern int RegPSGSel;



void WritePPI( int adr, int val );

int ReadPPI( int adr );

void WriteCas( void );

void ReadCas( void );

void OpenTapWrite( char * Nom );

void OpenTapRead( char * Nom );

void CloseTap( void );

BOOL Keyboard_HasBeenScanned(void);
void Keyboard_Reset(void);

#endif


