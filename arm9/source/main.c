/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Noyau!
// Définition du module        !CONFIG!=/V4!Main!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\\!./V2!\\!./V3!\\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*          SYSTEME         |      SOUS SYSTEME       |      SOUS ENSEMBLE
* ------------------------------------------------------------------------------
*  EMULATEUR CPC           | WIN-CPC                 | Noyau
* ------------------------------------------------------------------------------
*  Fichier     : MAIN.C                | Version : 0.1t
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Programme principal
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  07/11/2002              | L.DEPLANQUE             | Ajout gestion lecture et
*                          |                         | écriture cassette :
*                          |                         | fonctions WriteCas() et
*                          |                         | ReadCas() appelées à
*                          |                         | chaque Hsync.
* ------------------------------------------------------------------------------
*  03/12/2002              | L.DEPLANQUE             | Prise en compte de la
*                          |                         | directive USE_DEBUG
*                          |                         | optimisation de
*                          |                         | l'affichage et
*                          |                         | l'archivage des messages
*                          |                         | d'erreurs
* ------------------------------------------------------------------------------
*  06/01/2003              | L.DEPLANQUE             | Version 0.1t :
*                          |                         | Suppression d'un Warning
* ------------------------------------------------------------------------------
*  02/02/2004              | L.DEPLANQUE             | Version 0.1x :
*                          |                         | Modification de la
*                          |                         | fréquence d'appel des
*                          |                         | fonctions lecture et
*                          |                         | écritures cassettes,
*                          |                         | ajout d'une constante
*                          |                         | pour cette fréquence.
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#include <nds.h>

#include "snapshot.h"

#include "types.h"
#include "plateform.h"
#include "vga.h"
#include "ppi.h"
#include "upd.h"
#include "autotype.h"
#include "sound.h"
#include "config.h"
#include "crtc.h"

#include "z80.h"

#include "../../arm7/source/ipc2.h"


/********************************************************* !NAME! **************
* Nom : finMain
********************************************************** !PATHS! *************
* !./V1!\\!./V2!\\!./V3!\\!./V4!\\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\\/FLE!, ligne : !./LN!
*
* Description : Fin du programme
*
********************************************************** !0! ****************/
int finMain = 0;

extern struct kmenu *forceMenu;


/********************************************************* !NAME! **************
* Nom : Go
********************************************************** !PATHS! *************
* !./V1!\\!./V2!\\!./V3!\\!./V4!\\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\\/FLE!, ligne : !./LN!
*
* Description : Fonction principale : boucle d'émulation
*
* Résultat    : /
*
* Variables globales modifiées : Cont, CntHSync, VBL
*
********************************************************** !0! ****************/
static void Go( void )
{
    unsigned long TimeOut = 0, OldTime = 0;
    int tz80=0;
    char framebuf[128];    
    int oldDriveBusy=0;
    int turbo=0;

    framebuf[0]=0;

	PlaySound();
/*
switch(amstrad_cpc.refresh) {    // cpu_period = nombre de cycle si le cpu Z80 ne gere pas les cycleligne
    default:    
    case 1:      
        amstrad_cpc.cpu_period = (4000000 * 64) / 1000000;      
        break;    
    case 0:      
        amstrad_cpc.cpu_period = (4000000 * 53) / 1000000;     
        break;  
}
*/

    do
	{
        tz80-=nds_GetTicks();
        ExecInstZ80();    // Fais tourner le CPU
        tz80+=nds_GetTicks();

        TimeOut += (RegsCRTC[0] + 1);
		
		if (!CalcCRTCLine()) {         // Arrive en fin d'ecran ?            
			// TimeOut = ( RegsCRTC[ 0 ] + 1 ) * ( RegsCRTC[ 5 ] + ( ( 1 + RegsCRTC[ 4 ] ) * ( RegsCRTC[ 9 ] + 1 ) ) );
		
			// Rafraichissement de l'ecran...
		
            if (dispframerate) {    
		        cpcprint16i(0,192-8, framebuf, 255); 
            }
			
			UpdateScreen();
			nds_ReadKey();
			
			// Disk
			if (DriveBusy!=oldDriveBusy) {
			    DispDisk(DriveBusy);
			    oldDriveBusy=DriveBusy;
			}
			DriveBusy=0;
			
			if (inMenu) {
				LoopMenu(forceMenu);
				forceMenu=NULL;
			}
			// Synchronisation de l'image à 50 Hz
			// Pourcentage, temps espere, temps pris, temps z80, nombre de drawligne

            if (DoResync) {
                int Time;

                if (dispframerate) {
	     		    sprintf(framebuf, " %4d%% %4d %4d + %4d           ", (u32)((TimeOut * 100) / (nds_GetTicks() * 1024 - OldTime)), (u32)(TimeOut / 1024), (u32)(nds_GetTicks() - (OldTime/1024)) - tz80, tz80);
	     		    // sprintf(framebuf, " %08X %02X %02X %02X", IPC2->psgbuf, IPC2->psgbuf[0], IPC2->psgbuf[1], IPC2->psgbuf[2]);
                }

                if (!turbo) {
				    Time = nds_GetTicks() * 1024;
				    while( Time - OldTime < TimeOut ) {
					    Time = nds_GetTicks() * 1024;
                        if (dispframerate) {    
                            memcpy(framebuf, "  100%",6);
                        }
                    }
                }

		    	tz80=0;

			    OldTime = nds_GetTicks() * 1024;
                TimeOut=0;
            }
		}

	}
    while( ! finMain );

}


/********************************************************* !NAME! **************
* Nom : WinMain
********************************************************** !PATHS! *************
* !./V1!\\!./V2!\\!./V3!\\!./V4!\\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\\/FLE!, ligne : !./LN!
*
* Description : Point d'entrée du programme
*
* Résultat    : 0
*
* Variables globales modifiées :
*
********************************************************** !0! ****************/
int main(void)
{
    char * Msg;
	
	nds_init();
	
	AutoType_Init();
	
    if ( ReadConfig() )
	{
		if ( InitMemCPC() )
		{
#ifdef USE_MULTIFACE
			if ( ! Multiface || InitMultiface() )
#endif
			{
				ResetZ80();
				ResetUPD();
			//	ResetCRTC(); 
				Reset8912();
			    InitPlateforme();
			
                Autoexec();

				myprintf("CrocoDS v2.0");

				Go();
				EjectDiskUPD();
		    	PauseSound();
			}
#ifdef USE_MULTIFACE
			else
			{
				Msg = "Rom multiface non trouvée";
#ifdef USE_LOG
				Log( Msg, LOG_WARNING );
#endif
				Erreur( Msg );
			}
#endif
		}
		else
		{
			Msg = "Roms du CPC non trouvées";
#ifdef USE_LOG
			Log( Msg, LOG_WARNING );
#endif
			Erreur( Msg );
		}
		
	}
    else
        Erreur( "Fichier de configuration du CPC non trouvé." );
	
    return( 0 );
}

