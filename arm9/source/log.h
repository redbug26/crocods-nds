/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Accessoires!
// Définition du module        !CONFIG!=/V4!Log!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1p
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module LOG.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef LOG_H
#define LOG_H


enum { LOG_NONE = 0, LOG_WARNING, LOG_INFO, LOG_DEBUG };


#define     MAX_SIZE_MSG_LOG    128


extern char MsgLog[ MAX_SIZE_MSG_LOG ];


BOOL StartLog( char * LogName );

void Log( char * Message, int Niveau );

void EndLog( void );


#endif
