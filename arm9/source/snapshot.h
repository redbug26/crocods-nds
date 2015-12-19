/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Accessoires!
// Définition du module        !CONFIG!=/V4!Snapshots!
/********************************************************* !NAME! **************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : V0.1p
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module SNAPSHOT.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  13/11/2002              | L.DEPLANQUE             | Passage du nom de fichier
*                          |                         | snapshot en paramètre aux
*                          |                         | fonctions LireSnap() et
*                          |                         | SauveSnap().  
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef SNAPSHOT_H
#define SNAPSHOT_H


void SauveSnap(char *Nom);
void SauveScreen(char *Nom);
void LireSnap(char *Nom);
void LireSnapshotMem(u8 *snap);

#endif

