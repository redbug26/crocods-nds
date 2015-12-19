/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Noyau!
// Définition du module        !CONFIG!=/V4!Types!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1p
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions de types prédéfinis et de constantes de compilation
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


typedef unsigned short          USHORT;
typedef signed short            SHORT;
typedef unsigned char           UBYTE;
typedef unsigned long           ULONG;

#ifndef _BOOL
#define _BOOL
typedef int BOOL;
#endif


#ifndef FALSE
enum { FALSE, TRUE };
#endif

#define CPC_VISIBLE_SCR_WIDTH 256
#define CPC_VISIBLE_SCR_HEIGHT 240

#undef     USE_TAPE
#undef     USE_LOG
#undef     USE_PRINTER
#undef     USE_MULTIFACE
#define     USE_SOUND
#undef     USE_SOUND_CAS
#define    USE_SNAPSHOT

#define     USE_Z80_ORIG
#undef     USE_Z80_ASM

#define    USE_SAVESNAP
#undef   USE_SAVESNAP_SELECT

#undef USE_CONSOLE
#undef USE_ALTERSCREEN

#undef USE_SPLASH1
#undef USE_SPLASH2

#endif
