/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Accessoires!
// Définition du module        !CONFIG!=/V4!Log!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*          SYSTEME         |      SOUS SYSTEME       |      SOUS ENSEMBLE
* ------------------------------------------------------------------------------
*  EMULATEUR CPC           | WIN-CPC                 | Accessoires
* ------------------------------------------------------------------------------
*  Fichier     : LOG.C                 | Version : 0.1y
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Archivage d'information dans un fichier log
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  06/11/2002              | L.DEPLANQUE             | suppression de la fct
*                          |                         | LogMessage qui ne servait
*                          |                         | pas.
* ------------------------------------------------------------------------------
*  08/01/2003              | L.DEPLANQUE             | V0.1t : optimisation
*                          |                         | minime.
* ------------------------------------------------------------------------------
*  25/02/2004              | L.DEPLANQUE             | V0.1y : Ajout du titre
*                          |                         | et du numéro de version
*                          |                         | lors du log du démarrage
*                          |                         | et du log de l'arrêt.
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#include  <nds.h>

#include  "types.h"
#include  "log.h"
#include  "config.h"
#include  "plateform.h"


#ifdef USE_LOG
/********************************************************* !NAME! **************
* Nom : MsgLog
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Chaine de caractères servant au formattage des messages à
*               archiver dans le fichier LOG
*
********************************************************** !0! ****************/
char MsgLog[ MAX_SIZE_MSG_LOG ];


/********************************************************* !NAME! **************
* Nom : fLog
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Handle fichier LOG
*
********************************************************** !0! ****************/
static FILE * fLog = NULL;


/********************************************************* !NAME! **************
* Nom : StartLog
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Description : Ouvre le fichier .LOG
*
* Résultat    : 
*
* Variables globales modifiées : 
*
********************************************************** !0! ****************/
BOOL StartLog( char * LogName )
{
    EndLog();
    fLog = fopen( LogName, "a+" );
    sprintf( MsgLog, "Démarrage application %s", Titre );
    Log( MsgLog, LOG_INFO );
    return( fLog != NULL );
}


/********************************************************* !NAME! **************
* Nom : Log
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Description : Archive un message dans le fichier .LOG
*
* Résultat    : 
*
* Variables globales modifiées : 
*
********************************************************** !0! ****************/
void Log( char * Message, int Niveau )
{
    static SYSTEMTIME sTime;

    if ( fLog && ( Niveau <= LogLevel ) )
        {
        if ( * Message )
            {
            GetLocalTime( &sTime );
            fprintf( fLog, "%02d/%02d/%04d %02d:%02d:%02d %s %s\n"
                   , sTime.wDay
                   , sTime.wMonth
                   , sTime.wYear
                   , sTime.wHour
                   , sTime.wMinute
                   , sTime.wSecond
                   , Niveau == LOG_WARNING ? "!!!" : ""
                   , Message
                   );
            }
        else
            fprintf( fLog, "\n" );

        fflush( fLog );
        }
}


/********************************************************* !NAME! **************
* Nom : EndLog
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Description : Ferme le fichier .LOG
*
* Résultat    : 
*
* Variables globales modifiées : 
*
********************************************************** !0! ****************/
void EndLog( void )
{
    if ( fLog )
        {
        sprintf( MsgLog, "Arrêt application %s", Titre );
        Log( MsgLog, LOG_INFO );
        Log( "", LOG_INFO );
        fclose( fLog );
        fLog = NULL;
        }
}
#endif
