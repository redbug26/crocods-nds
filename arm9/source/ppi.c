/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!PPI 8255!
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
*  Fichier     : PPI.C                 | Version : 0.1x
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Emulation du PPI 8255
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  06/11/2002              | L.DEPLANQUE             | Précisions bits du port
*                          |                         | B (Busy, 50Hz refresh)
* ------------------------------------------------------------------------------
*  07/11/2002              | L.DEPLANQUE             | Ajout gestion lecture et
*                          |                         | écriture cassette.:
*                          |                         | fonctions WriteCas() et
*                          |                         | ReadCas().
* ------------------------------------------------------------------------------
*  13/11/2002              | L.DEPLANQUE             | Optimisation WriteCas() 
*                          |                         | et ReadCas() : supprime
*                          |                         | test pointeur du fichier
*                          |                         | car inclus dans la
*                          |                         | variable Mode
* ------------------------------------------------------------------------------
*  03/12/2002              | L.DEPLANQUE             | Suppression de la
*                          |                         | génération de sons lors
*                          |                         | de la lecture/écriture
*                          |                         | cassette
* ------------------------------------------------------------------------------
*  09/01/2003              | L.DEPLANQUE             | Version 0.1u :
*                          |                         | corection d'un bugg :
*                          |                         | inversion des bits 
*                          |                         | PRINTER_BUSY et
*                          |                         | REFRESH_HZ
* ------------------------------------------------------------------------------
*  09/01/2004              | L.DEPLANQUE             | Version 0.1x :
*                          |                         | corection d'un bugg :
*                          |                         | problème d'écriture du
*                          |                         | buffer cassette dans le
*                          |                         | fichier correspondant
*                          |                         | (écrivait une taille=0)
* ------------------------------------------------------------------------------
*  02/02/2004              | L.DEPLANQUE             | Ajout de l'émission d'un
*                          |                         | son lors de la lecture
*                          |                         | ou l'écriture sur
*                          |                         | cassette.
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#include  <nds.h>

#include "types.h"
#include "ppi.h"
#ifdef USE_LOG
#include "log.h"
#endif
#ifdef USE_SOUND
#include "sound.h"
#endif
#include "crtc.h"

#include "plateform.h"


#define PRINTER_BUSY    0x40    // Printer busy

#define REFRESH_HZ      0x10    // Screen refresh = 50Hz

#define CONSTRUCTEUR    0x07    // valeurs possibles :
                                // 0x00 = Isp
                                // 0x01 = Triumph
                                // 0x02 = Saisho
                                // 0x03 = Solavox
                                // 0x04 = Awa
                                // 0x05 = Schneider
                                // 0x06 = Orion
                                // 0x07 = Amstrad


#define SIZE_BUF_TAPE   0x800   // Taille buffer tampon cassette
                                // !!! Doit être une puissance de 2 !!!




/********************************************************* !NAME! **************
* Nom : modePSG
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Mode du PSG
*
********************************************************** !0! ****************/
int modePSG;


/********************************************************* !NAME! **************
* Nom : RegPSGSel
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Numéro de registre du PSG sélectionné
*
********************************************************** !0! ****************/
int RegPSGSel;


/********************************************************* !NAME! **************
* Nom : clav
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Matrice du clavier (16 * 8 bits)
*
********************************************************** !0! ****************/
u8 clav[ 16 ];

static bool KeyboardScanned = FALSE;
 

/********************************************************* !NAME! **************
* Nom : RegsPPI
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Registres inernes du PPI 8255
*
********************************************************** !0! ****************/
int RegsPPI[ 4 ];


/********************************************************* !NAME! **************
* Nom : Output
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Sorties du PPI 8255
*
********************************************************** !0! ****************/
static int Output[ 3 ];


/********************************************************* !NAME! **************
* Nom : Input
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Entrées du PPI 8255
*
********************************************************** !0! ****************/
static int Input[ 3 ];


/********************************************************* !NAME! **************
* Nom : Mask
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Masques calculés pour masquage entrées/sorties
*
********************************************************** !0! ****************/
static int Mask[ 3 ];


/********************************************************* !NAME! **************
* Nom : ligneClav
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Numéro de ligne du clavier sélectionnée
*
********************************************************** !0! ****************/
static int ligneClav;


#ifdef USE_TAPE
/********************************************************* !NAME! **************
* Nom : fCas
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Handle fichier lecture ou écriture cassette
*
********************************************************** !0! ****************/
static FILE * fCas = NULL;


enum { MODE_OFF, MODE_WRITE, MODE_READ };


/********************************************************* !NAME! **************
* Nom : BitTapeIn
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Bit de lecture cassette pour le port F5xx
*
********************************************************** !0! ****************/
static UBYTE BitTapeIn = 0;


/********************************************************* !NAME! **************
* Nom : PosBit
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Position de bit de calcul pour lecture/écriture cassette
*
********************************************************** !0! ****************/
static int PosBit = 0;


/********************************************************* !NAME! **************
* Nom : PosBit
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Octet de calcul pour lecture/écriture cassette
*
********************************************************** !0! ****************/
static UBYTE OctetCalcul = 0;


/********************************************************* !NAME! **************
* Nom : BufTape
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Buffer fichier cassette
*
********************************************************** !0! ****************/
static UBYTE BufTape[ SIZE_BUF_TAPE ];


/********************************************************* !NAME! **************
* Nom : PosBufTape
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Index sur buffer fichier cassette
*
********************************************************** !0! ****************/
static int PosBufTape;


/********************************************************* !NAME! **************
* Nom : Mode
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Mode d'accès cassette (lecture, écriture, ou off)
*
********************************************************** !0! ****************/
static int Mode = MODE_OFF;


/********************************************************* !NAME! **************
* Nom : CloseTap
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fermeture fichier cassette
*
* Résultat    : /
*
* Variables globales modifiées : fCas, OctetCalcul, PosBit, Mode
*
********************************************************** !0! ****************/
void CloseTap( void )
{
    if ( fCas )
        {
        fwrite( BufTape, PosBufTape, 1, fCas );
        fclose( fCas );
        fCas = NULL;
        }
    OctetCalcul = 0;
    PosBit = 0;
    PosBufTape = 0;
    Mode = MODE_OFF;
#ifdef USE_SOUND_CAS
    SetSound( 0, 0, 0 );
#endif
}

/********************************************************* !NAME! **************
* Nom : OpenTapWrite
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ouverture fichier cassette en écriture
*
* Résultat    : /
*
* Variables globales modifiées : fCas, OctetCalcul, PosBit, Mode
*
********************************************************** !0! ****************/
void OpenTapWrite( char * Nom )
{
    CloseTap();
    fCas = fopen( Nom, "wb" );
    if ( fCas )
        Mode = MODE_WRITE;
}


/********************************************************* !NAME! **************
* Nom : OpenTapRead
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ouverture fichier cassette en lecture
*
* Résultat    : /
*
* Variables globales modifiées : fCas, OctetCalcul, PosBit, Mode
*
********************************************************** !0! ****************/
void OpenTapRead( char * Nom )
{
    CloseTap();
    fCas = fopen( Nom, "rb" );
    if ( fCas )
        Mode = MODE_READ;
}

#ifdef USE_SOUND_CAS
/********************************************************* !NAME! **************
* Nom : SoundCas
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Joue un son en fonction des bits lecture/écriture cassette
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
static void SoundCas( int BitCas )
{
    static int ValBit = 0;
    static int CntBit = 0;
    static int CntFreq = 0;

    //
    // Calcul de la fréquence : WriteCas est appelé tous les 1/384 cycles
    // d'horloge du Z80, qui est cadencé à 3,3 Mhz, soit :
    // 3,3*1000000 / 384 = 8667 fois par secondes, soit ~ 4333 Hz 
    //
    if ( ValBit != BitCas )
        {
        ValBit = BitCas;
        CntFreq++;
        }
    CntBit++;
    if ( CntBit == 255 )      // Attentre 255 mesures d'échantillons
        {
        CntBit = 0;
        if ( ! CntFreq )
            {
            SetSound( 0, 0, 0 );
            SetSound( 3, 0, 0 );
            }
        else
            {
            SetSound( 0, 17 * CntFreq, 32 );       // 255 * 17 = 4301
            SetSound( 3, 17 * CntFreq, 16 );
            }
        CntFreq = 0;
        }
}
#endif

/********************************************************* !NAME! **************
* Nom : WriteCas
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un bit dans le fichier cassette
*
* Résultat    : /
*
* Variables globales modifiées : OctetCalcul, PosBit
*
********************************************************** !0! ****************/
void WriteCas( void )
{
    if ( ( Output[ 2 ] & 0x10 ) && Mode == MODE_WRITE )
        {
#ifdef USE_SOUND_CAS
        SoundCas( Output[ 2 ] & 0x20 );
#endif
        OctetCalcul |= ( ( Output[ 2 ] & 0x20 ) >> 5 ) << PosBit++;
        if ( PosBit == 8 )
            {
            BufTape[ PosBufTape++ ] = OctetCalcul;
            PosBufTape &= ( SIZE_BUF_TAPE - 1 );
            if ( ! PosBufTape )
                {
                //
                // Optimisiation de la taille écrite : si le buffer est rempli
                // de zéros, alors on ne l'écrit pas.
                // Attention : le buffer doit être supérieur à 1024 Octets
                //
                int i = 0, vmax = 0;
                for ( i = 0; i < SIZE_BUF_TAPE; i++ )
                    vmax += BufTape[ i ];

                if ( vmax )
                    fwrite( BufTape, SIZE_BUF_TAPE, 1, fCas );
                }
            OctetCalcul = 0;
            PosBit = 0;
            }
        }
}


/********************************************************* !NAME! **************
* Nom : ReadCas
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un bit depuis le fichier cassette
*
* Résultat    : /
*
* Variables globales modifiées : OctetCalcul, PosBit, BitTapeIn
*
********************************************************** !0! ****************/
void ReadCas( void )
{
    BitTapeIn = 0;
    if ( ( Output[ 2 ] & 0x10 ) && Mode == MODE_READ )
        {
        if ( ! PosBufTape && ! PosBit )
            fread( BufTape, SIZE_BUF_TAPE, 1, fCas );

        if ( ! PosBit )
            {
            OctetCalcul = BufTape[ PosBufTape++ ];
            PosBufTape &= ( SIZE_BUF_TAPE - 1 );
            }
        if ( OctetCalcul & ( 1 << PosBit++ ) )
            BitTapeIn = 0x80;

        if ( PosBit == 8 )
            PosBit = 0;

#ifdef USE_SOUND_CAS
        SoundCas( BitTapeIn );
#endif
        }
}
#endif


/********************************************************* !NAME! **************
* Nom : UpdatePSG
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Mise à jour des registres du PSG en fonction du port A du PPI
*
* Résultat    : /
*
* Variables globales modifiées : RegPSGSel
*
********************************************************** !0! ****************/
static void UpdatePSG( void )
{
    switch ( modePSG )
        {
        case 2:
            Write8912( RegPSGSel, Output[ 0 ] );
            break;

        case 3:
            RegPSGSel = Output[ 0 ];
            break;
        }
}


/********************************************************* !NAME! **************
* Nom : PPI_WriteControl
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture du registre de controle du PPI
*
* Résultat    : /
*
* Variables globales modifiées : RegsPPI, Output, Mask
*
********************************************************** !0! ****************/
static void PPI_WriteControl( int val )
{
    RegsPPI[ 3 ] = val;
    if ( val & 0x80 )
        {
        RegsPPI[ 0 ] = RegsPPI[ 2 ] = 0;
        Mask[ 0 ] = ( val & 0x10 ) ? 0xFF : 0;
        Mask[ 2 ] = 0xFF;
        if ( ! ( val & 0x01 ) )
            Mask[ 2 ] &= 0xF0;

        if ( ! ( val & 0x08 ) )
            Mask[ 2 ] &= 0x0F;
        }
    else
        {
        int BitMask = 1 << ( ( val >> 1 ) & 0x07 );
        if ( val & 1 )
            RegsPPI[ 2 ] |= BitMask;
        else
            RegsPPI[ 2 ] &= ~BitMask;
        }
    Output[ 0 ] = ( RegsPPI[ 0 ] & ~Mask[ 0 ] ) | Mask[ 0 ];
    Output[ 2 ] = ( RegsPPI[ 2 ] & ~Mask[ 2 ] ) | Mask[ 2 ];
}


/********************************************************* !NAME! **************
* Nom : WritePPI
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un registre du PPI
*
* Résultat    : /
*
* Variables globales modifiées : RegsPPI, Output, ligneClav, modePSG
*
********************************************************** !0! ****************/
void WritePPI( int adr, int val )
{
    switch( ( adr >> 8 ) & 3 )
        {
        case 0 : // 0xF4xx
            RegsPPI[ 0 ] = val;
            Output[ 0 ] = ( val & ~Mask[ 0 ] ) | Mask[ 0 ];
            UpdatePSG();
            break;

        case 1 : // 0xF5xx
            break;

        case 2 : // 0xF6xx
            RegsPPI[ 2 ] = val;
            Output[ 2 ] = ( val & ~Mask[ 2 ] ) | Mask[ 2 ];
            ligneClav = Output[ 2 ] & 0x0F;
            modePSG = Output[ 2 ] >> 6;
            UpdatePSG();
            break;

        case 3 : // 0xF7xx
            PPI_WriteControl( val );
            break;
        }
}


/********************************************************* !NAME! **************
* Nom : ReadPPI
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un registre du PPI
*
* Résultat    : Le registre du PPI selectionné
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
int ReadPPI( int adr )
{
    switch( ( adr >> 8 ) & 3 )
        {
        case 0 : // 0xF4xx
            if ( modePSG == 1 )
                {
                if ( RegPSGSel == 14 ) {
                    KeyboardScanned = TRUE; 
                    return( clav[ ligneClav ] );
                }

#ifdef USE_SOUND
                // return( RegsPSG[ RegPSGSel ] );
                return(Read8912(RegPSGSel));
#else
                return( 0xFF );
#endif
                }
            return( 0xFF );

        case 1 : // 0xF5xx
            return( ( CONSTRUCTEUR << 1 )
                  | REFRESH_HZ
                  | VSync 
#ifdef USE_TAPE
                  | BitTapeIn
#endif
                  ); // Port B toujours en lecture

        case 2 : // 0xF6xx
            return( ( Input[ 2 ] & Mask[ 2 ] ) 
                  | ( RegsPPI[ 2 ] & ~Mask[ 2 ] )
                  );
        }
    return( 0xFF );
}

BOOL Keyboard_HasBeenScanned(void)
{
	return KeyboardScanned;
} 

void Keyboard_Reset(void)
{
    KeyboardScanned = FALSE;
}
