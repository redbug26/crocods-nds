/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Accessoires!
// Définition du module        !CONFIG!=/V4!Gestion Configuration!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1v
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module CONFIG.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  06/01/2003              | L.DEPLANQUE             | Version 0.1t : Gestion
*                          |                         | nouvelles variables :
*                          |                         | localisation des roms
* ------------------------------------------------------------------------------
*  09/01/2003              | L.DEPLANQUE             | Version 0.1u : Gestion
*                          |                         | nouvelles variables :
*                          |                         | touches joystick
* ------------------------------------------------------------------------------
*  03/02/2003              | L.DEPLANQUE             | Version 0.1v : Gestion
*                          |                         | nouvelles variables :
*                          |                         | localisation fichiers
*                          |                         | DSK, SNA, TAP
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef CONFIG_H
#define CONFIG_H


extern int Sound;

extern int Multiface;

extern int LogLevel;

extern char LocRomInf[];

extern char LocRomSup[];

extern char LocRomDisc[];

extern char LocRomMulti[];

extern int JoyLeft;

extern int JoyRight;

extern int JoyUp;

extern int JoyDown;

extern int JoyFire1;

extern int JoyFire2;

extern int DureeVbl;

extern char DirSnap[];

extern char DirTape[];

extern char DirDisc[];

extern char DirEmuDisc[];


BOOL ReadConfig( void );


#endif
