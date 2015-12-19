/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!WIN-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!UPD 765!
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
*  Fichier     : UPD.C                 | Version : 0.1w
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Emulation du UPD 765
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  06/02/2003              | L.DEPLANQUE             | Version 0.1v : mémorise
*                          |                         | la longueur du fichier lu
*                          |                         | pour réécrire
*                          |                         | si nécessaire
* ------------------------------------------------------------------------------
*  18/02/2003              | L.DEPLANQUE             | Version 0.1w : prise en
*                          |                         | compte de l'information
*                          |                         | disc missing dans le
*                          |                         | registre ST0
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#include  <nds.h>

#include "types.h"
#include "plateform.h"

#include "upd.h"

//
// Constantes generales...
//
#define SECTSIZE   512
#define NBSECT     9


// Bits de Status
#define STATUS_CB       0x10
#define STATUS_NDM      0x20
#define STATUS_DIO      0x40
#define STATUS_RQM      0x80

// ST0
#define ST0_NR          0x08
#define ST0_SE          0x20
#define ST0_IC1         0x40
#define ST0_IC2         0x80

// ST1
#define ST1_ND          0x04
#define ST1_EN          0x80

// ST3
#define ST3_TS          0x08
#define ST3_T0          0x10
#define ST3_RY          0x20


/********************************************************* !NAME! **************
* Nom : ImgDsk
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : ImgDsk de stockage du fichier image disquette
*
********************************************************** !0! ****************/
u8 *ImgDsk=NULL;

int LongFic;



typedef int ( * pfctUPD )( int );


static int Rien( int );


/********************************************************* !NAME! **************
* Nom : fct
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Pointeur de fonction du UPD
*
********************************************************** !0! ****************/
static pfctUPD fct = Rien;


/********************************************************* !NAME! **************
* Nom : etat
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Compteur phase fonction UPD
*
********************************************************** !0! ****************/
static int etat;


/********************************************************* !NAME! **************
* Nom : CurrTrackDatasDSK
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Image d'une piste
*
********************************************************** !0! ****************/
CPCEMUTrack CurrTrackDatasDSK;


/********************************************************* !NAME! **************
* Nom : Infos
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : En-tête du fichier image
*
********************************************************** !0! ****************/
static CPCEMUEnt Infos;


/********************************************************* !NAME! **************
* Nom : FlagWrite
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Indique si écriture sur disquette effectuée
*
********************************************************** !0! ****************/
static int FlagWrite;

/********************************************************* !NAME! **************
* Nom : Image
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Indique si fichier image chargé
*
********************************************************** !0! ****************/
int Image;


/********************************************************* !NAME! **************
* Nom : PosData
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Position des données dans l'image
*
********************************************************** !0! ****************/
int PosData;


/********************************************************* !NAME! **************
* Nom : NomFic
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Nom du fichier image ouvert
*
********************************************************** !0! ****************/

int DriveBusy;

/********************************************************* !NAME! **************
* Nom : Status
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status UPD
*
********************************************************** !0! ****************/
static int Status;


/********************************************************* !NAME! **************
* Nom : ST0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status interne 0
*
********************************************************** !0! ****************/
static int ST0;


/********************************************************* !NAME! **************
* Nom : ST1
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status interne 1
*
********************************************************** !0! ****************/
static int ST1;


/********************************************************* !NAME! **************
* Nom : ST2
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status interne 2
*
********************************************************** !0! ****************/
static int ST2;


/********************************************************* !NAME! **************
* Nom : ST3
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Status interne 3
*
********************************************************** !0! ****************/
static int ST3;


/********************************************************* !NAME! **************
* Nom : C
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Cylindre (nø piste)
*
********************************************************** !0! ****************/
int C;


/********************************************************* !NAME! **************
* Nom : H
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Head     (nø tête)
*
********************************************************** !0! ****************/
int H;


/********************************************************* !NAME! **************
* Nom : R
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Record   (nø secteur)
*
********************************************************** !0! ****************/
int R;


/********************************************************* !NAME! **************
* Nom : N
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Number   (nbre d'octet poids forts)
*
********************************************************** !0! ****************/
int N;


/********************************************************* !NAME! **************
* Nom : Drive
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Drive    ( 0=A, 1=B)
*
********************************************************** !0! ****************/
static int Drive;


/********************************************************* !NAME! **************
* Nom : EOT
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Secteur final;
*
********************************************************** !0! ****************/
static int EOT;


/********************************************************* !NAME! **************
* Nom : Busy
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : UPD occupé
*
********************************************************** !0! ****************/
static int Busy;


/********************************************************* !NAME! **************
* Nom : Inter
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Génération d'une interruption UPD
*
********************************************************** !0! ****************/
static int Inter;


/********************************************************* !NAME! **************
* Nom : Moteur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Etat moteur UPD
*
********************************************************** !0! ****************/
static int Moteur;


/********************************************************* !NAME! **************
* Nom : IndexSecteur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Indique la position du secteur en cour sur la piste courante
*
********************************************************** !0! ****************/
int IndexSecteur = 0;


/********************************************************* !NAME! **************
* Nom : RechercheSecteur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Recherche un secteur sur la piste courrante
*
* Résultat    : L'index du secteur trouvé
*
* Variables globales modifiées : ST0, ST1
*
********************************************************** !0! ****************/
int RechercheSecteur( int newSect, int * pos )
{
	int i;

	* pos = 0;
	for ( i = 0; i < CurrTrackDatasDSK.NbSect; i++ )
	if ( CurrTrackDatasDSK.Sect[ i ].R == newSect )
	return( ( UBYTE )i );
	else
	* pos += CurrTrackDatasDSK.Sect[ i ].SectSize;

	ST0 |= ST0_IC1;
	ST1 |= ST1_ND;
#ifdef USE_LOG
	sprintf( MsgLog, "secteur (C:%02X,H:%02X,R:%02X) non trouvé", C, H, R );
	Log( MsgLog, LOG_DEBUG );
#endif
	return( -1 );
}


/********************************************************* !NAME! **************
* Nom : ReadCHRN
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Renvoie les paramètres C, H, R, N de la piste courante
*
* Résultat    : /
*
* Variables globales modifiées : C, H, R, N, IndexSecteur
*
********************************************************** !0! ****************/
void ReadCHRN( void )
{
	C = CurrTrackDatasDSK.Sect[ IndexSecteur ].C;
	H = CurrTrackDatasDSK.Sect[ IndexSecteur ].H;
	R = CurrTrackDatasDSK.Sect[ IndexSecteur ].R;
	N = CurrTrackDatasDSK.Sect[ IndexSecteur ].N;
	if ( ++IndexSecteur == CurrTrackDatasDSK.NbSect )
	IndexSecteur = 0;
}


/********************************************************* !NAME! **************
* Nom : SetST0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Calcul des registres ST0, ST1, ST2
*
* Résultat    : /
*
* Variables globales modifiées : ST0, ST1, ST2
*
********************************************************** !0! ****************/
static void SetST0( void )
{
	ST0 = 0; // drive A
	if ( ! Moteur || Drive || ! Image )
	ST0 |= ST0_IC1 | ST0_NR;

	ST1 = 0;
	ST2 = 0;
}


/********************************************************* !NAME! **************
* Nom : Rien
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction non reconnue du UPD
*
* Résultat    : /
*
* Variables globales modifiées : etat, ST0
*
********************************************************** !0! ****************/
static int Rien( int val )
{
	Status &= ~STATUS_CB & ~STATUS_DIO;
	etat = 0;
	ST0 = ST0_IC2;
#ifdef USE_LOG
	Log( "Appel fonction FDC Rien", LOG_DEBUG );
#endif
	return( Status );
}


/********************************************************* !NAME! **************
* Nom : ReadST0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadST0
*
* Résultat    : ST0, C
*
* Variables globales modifiées : etat, ST0, ST1, Inter, Busy
*
********************************************************** !0! ****************/
static int ReadST0( int val )
{
	if ( ! Inter )
	ST0 = ST0_IC2;
	else
	{
		Inter = 0;
		if ( Busy )
		{
			ST0 = ST0_SE;
			Busy = 0;
		}
		else
		ST0 |= ST0_IC1 | ST0_IC2;
	}
	if ( Moteur && Image && ! Drive )
	ST0 &= ~ST0_NR;
	else
	{
		ST0 |= ST0_NR;
		if ( ! Image )
		ST0 |= ( ST0_IC1 | ST0_IC2 );
	}

	if ( etat++ == 1 )
	{
		Status |= STATUS_DIO;
		return( ST0 );
	}

	etat = val = 0;
	Status &= ~STATUS_CB & ~STATUS_DIO;
	ST0 &= ~ST0_IC1 & ~ST0_IC2;
	ST1 &= ~ST1_ND;
	return( C );
}


/********************************************************* !NAME! **************
* Nom : ReadST3
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadST3
*
* Résultat    : 0, ST3
*
* Variables globales modifiées : etat, Status, ST3
*
********************************************************** !0! ****************/
static int ReadST3( int val )
{
	if ( etat++ == 1 )
	{
		Drive = val;
		Status |= STATUS_DIO;
		return( 0 );
	}
	etat = 0;
	Status &= ~STATUS_CB & ~STATUS_DIO;
	if ( Moteur && ! Drive )
	ST3 |= ST3_RY;
	else
	ST3 &= ~ST3_RY;

	return( ST3 );
}


/********************************************************* !NAME! **************
* Nom : Specify
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD Specify, non émulée
*
* Résultat    : 0
*
* Variables globales modifiées : etat, Status
*
********************************************************** !0! ****************/
static int Specify( int val )
{
	if ( etat++ == 1 )
	return( 0 );

	etat = val = 0;
	Status &= ~STATUS_CB & ~STATUS_DIO;
	return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadID
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadID
*
* Résultat    : ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifiées : Drive, Status, Inter, etat
*
********************************************************** !0! ****************/
static int ReadID( int val )
{
	switch( etat++ )  {
	case 1 :
		Drive = val;
		Status |= STATUS_DIO;
		Inter = 1;
		break;

	case 2 :
		return( 0 /*ST0*/ );

	case 3 :
		return( ST1 );

	case 4 :
		return( ST2 );

	case 5 :
		ReadCHRN();
		return( C );

	case 6 :
		return( H );

	case 7 :
		return( R );

	case 8 :
		etat = 0;
		Status &= ~STATUS_CB & ~STATUS_DIO;
		return( N );
	}
	return( 0 );
}



/********************************************************* !NAME! **************
* Nom : FormatTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD FormatTrack, non émulée
*
* Résultat    : 0
*
* Variables globales modifiées : etat, Status
*
********************************************************** !0! ****************/
static int FormatTrack( int val )
{
#ifdef USE_LOG
	Log( "Appel fonction FDC format", LOG_DEBUG );
#endif
	etat = val = 0;
	Status &= ~STATUS_CB & ~STATUS_DIO;
	return( 0 );
}


/********************************************************* !NAME! **************
* Nom : Scan
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD Scan, non émulée
*
* Résultat    : 0
*
* Variables globales modifiées : etat
*
********************************************************** !0! ****************/
static int Scan( int val )
{
#ifdef USE_LOG
	Log( "Appel fonction FDC scan", LOG_DEBUG );
#endif
	etat = val = 0;
	return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ChangeCurrTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Changement de la piste courrante
*
* Résultat    : /
*
* Variables globales modifiées : C, H, R, N, ST3
*
********************************************************** !0! ****************/
void ChangeCurrTrack( int newTrack )
{
	ULONG Pos = 0;
	int t, s;

	if ( ! Infos.DataSize )
	{
		memcpy( &CurrTrackDatasDSK, ImgDsk, sizeof( CurrTrackDatasDSK ) );
		for ( t = 0; t < newTrack; t++ )
		{
			for ( s = 0; s < CurrTrackDatasDSK.NbSect; s++ )
			Pos += CurrTrackDatasDSK.Sect[ s ].SectSize;

			Pos += sizeof( CurrTrackDatasDSK );
			memcpy( &CurrTrackDatasDSK, &ImgDsk[ Pos ], sizeof( CurrTrackDatasDSK ) );
		}
	}
	else
	Pos += Infos.DataSize * newTrack;

	memcpy( &CurrTrackDatasDSK, &ImgDsk[ Pos ], sizeof( CurrTrackDatasDSK ) );

	PosData = Pos + sizeof( CurrTrackDatasDSK );
	IndexSecteur = 0;
	ReadCHRN();

	if ( ! newTrack )
	ST3 |= ST3_T0;
	else
	ST3 &= ~ST3_T0;
}


/********************************************************* !NAME! **************
* Nom : MoveTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD MoveTrack
*
* Résultat    : 0
*
* Variables globales modifiées : etat, Drive, Status, Busy, Inter
*
********************************************************** !0! ****************/
static int MoveTrack( int val )
{
	switch( etat++ )
	{
	case 1 :
		Drive = val;
		SetST0();            
		Status |= STATUS_NDM;
		break;

	case 2 :
		ChangeCurrTrack( C = val );
		etat = 0;
		Status &= ~STATUS_CB & ~STATUS_DIO & ~STATUS_NDM;
		Busy = 1;
		Inter = 1;
		break;
	}
	return( 0 );
}


/********************************************************* !NAME! **************
* Nom : MoveTrack0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD MoveTrack0
*
* Résultat    : 0
*
* Variables globales modifiées : etat, C, Status, Busy, Inter
*
********************************************************** !0! ****************/
static int MoveTrack0( int val )
{
	Drive = val;
	ChangeCurrTrack( C = 0 );
	etat = 0;
	Status &= ~STATUS_CB & ~STATUS_DIO & ~STATUS_NDM;
	SetST0();
	Busy = 1;
	Inter = 1;
	return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadData
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadData
*
* Résultat    : Datas, ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifiées : C, H, R, N, EOT, etat, Status
*
********************************************************** !0! ****************/
static int ReadData( int val )
{
	static int sect = 0, cntdata = 0, newPos;
	static signed int TailleSect;

	switch( etat++ )
	{
	case 1 :
		Drive = val;
		SetST0();            
		break;

	case 2 :
		C = val;
		break;

	case 3 :
		H = val;
		break;

	case 4 :
		R = val;
		break;

	case 5 :
		N = val;
		break;

	case 6 :
		EOT = val;
		break;

	case 7 :
		sect = RechercheSecteur( R, &newPos );
		if (sect != -1) {
			TailleSect = 128 << CurrTrackDatasDSK.Sect[ sect ].N;
			if ( ! newPos )
			cntdata = ( sect * CurrTrackDatasDSK.SectSize ) << 8;
			else
			cntdata = newPos;
		}
		break;

	case 8 :
		Status |= STATUS_DIO | STATUS_NDM;
		break;

	case 9 :
		if ( ! ( ST0 & ST0_IC1 ) )
		{
			if ( --TailleSect )
			etat--;
			else
			{
				if ( R++ < EOT )
				etat = 7;
				else
				Status &= ~STATUS_NDM;
			}
			return( ImgDsk[ PosData + cntdata++ ] );
		}
		Status &= ~STATUS_NDM;
		return( 0 );

	case 10 :
		return( ST0 );

	case 11 :
		return( ST1 | ST1_EN );       // ### ici PB suivant logiciels... ###

	case 12 :
		return( ST2 );

	case 13 :
		return( C );

	case 14 :
		return( H );

	case 15 :
		return( R );

	case 16 :
		etat = 0;
		Status &= ~STATUS_CB & ~STATUS_DIO;
		return( N );
	}
	return( 0 );
}


/********************************************************* !NAME! **************
* Nom : WriteData
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD WriteData
*
* Résultat    : ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifiées : C, H, R, N, EOT, etat, Status
*
********************************************************** !0! ****************/
static int WriteData( int val )
{
	static int sect = 0, cntdata = 0, newPos;
	static signed int TailleSect;

	switch( etat++ )
	{
	case 1 :
		Drive = val;
		SetST0();            
		break;

	case 2 :
		C = val;
		break;

	case 3 :
		H = val;
		break;

	case 4 :
		R = val;
		break;

	case 5 :
		N = val;
		break;

	case 6 :
		EOT = val;
		break;

	case 7 :
		sect = RechercheSecteur( R, &newPos );
		if (sect != -1) {
			TailleSect = 128 << CurrTrackDatasDSK.Sect[ sect ].N;
			if ( ! newPos )
			cntdata = ( sect * CurrTrackDatasDSK.SectSize ) << 8;
			else
			cntdata = newPos;
		}
		break;

	case 8 :
		Status |= STATUS_DIO | STATUS_NDM;
		break;

	case 9 :
		if ( ! ( ST0 & ST0_IC1 ) )
		{
			ImgDsk[ PosData + cntdata++ ] = ( UBYTE )val;
			if ( --TailleSect )
			etat--;
			else
			{
				if ( R++ < EOT )
				etat = 7;
				else
				Status &= ~STATUS_NDM;
			}
			return( 0 );
		}
		Status &= ~STATUS_NDM;
		return( 0 );

	case 10 :
		if ( ! ( ST0 & ST0_IC1 ) )
		FlagWrite = 1;

		return( ST0 );

	case 11 :
		return( ST1 );

	case 12 :
		return( ST2 );

	case 13 :
		return( C );

	case 14 :
		return( H );

	case 15 :
		return( R );

	case 16 :
		etat = 0;
		Status &= ~STATUS_CB & ~STATUS_DIO;
		return( N );
	}
	return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un registre/donnée du UPD
*
* Résultat    : Valeur registre/donnée du UPD
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
int ReadUPD( int port )
{
	if ( port & 1 )
	return( fct( port ) );

	return( Status );
}


/********************************************************* !NAME! **************
* Nom : WriteUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un registre/donnée du UPD
*
* Résultat    : /
*
* Variables globales modifiées : Status, etat, fct, Moteur
*
********************************************************** !0! ****************/
void WriteUPD( int port, int val )
{
	DriveBusy=1;
	
	if ( port == 0xFB7F ) {     
		if (etat) {    
			fct(val);
		} else {
			Status |= STATUS_CB;
			etat = 1;
			switch( val & 0x1F )
			{
			case 0x03 :
				// Specify
				fct = Specify;
				break;

			case 0x04 :
				// Lecture ST3
				fct = ReadST3;
				break;

			case 0x05 :
				// Ecriture données
				fct = WriteData;
				break;

			case 0x06 :
				// Lecture données
				fct = ReadData;
				break;

			case 0x07 :
				// Déplacement tête piste 0
				//                    Status |= STATUS_NDM;
				fct = MoveTrack0;
				break;

			case 0x08 :
				// Lecture ST0, track
				Status |= STATUS_DIO;
				fct = ReadST0;
				break;

			case 0x09 :
				// Ecriture données
				fct = WriteData;
				break;

			case 0x0A :
				// Lecture champ ID
				fct = ReadID;
				break;

			case 0x0C :
				// Lecture données
				fct = ReadData;
				break;

			case 0x0D :
				// Formattage piste
				fct = FormatTrack;
				break;

			case 0x0F :
				// Déplacement tête
				fct = MoveTrack;
				break;

			case 0x11 :
				fct = Scan;
				break;

				default :
				Status |= STATUS_DIO;
				fct = Rien;
			}
		}
	} else {
		if ( port == 0xFA7E ) {
			Moteur = val & 1;
		}
	}
}


/********************************************************* !NAME! **************
* Nom : ResetUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Effectue un reset du UPD
*
* Résultat    : /
*
* Variables globales modifiées : Status, ST0, ST1, ST2, ST3, Busy, Inter, etat
*
********************************************************** !0! ****************/
void ResetUPD( void )
{
	Status = STATUS_RQM;
	ST0 = ST0_SE;
	ST1 = 0;
	ST2 = 0;
	ST3 = ST3_RY | ST3_TS;
	Busy = 0;
	Inter = 0;
	etat = 0;
}


/********************************************************* !NAME! **************
* Nom : EjectDiskUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Termine l'accès à une disquette (ejection)
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void EjectDiskUPD( void )
{
	if (Image!=0) {
		Image = 0;
		DispIcons();
	}

	/*
	FILE * handle;

	if ( FlagWrite )
		{
			handle = fopen( NomFic, "wb" );
			if ( handle )
				{
				fwrite( &Infos, sizeof( Infos ), 1, handle );
				fwrite( ImgDsk, LongFic, 1, handle );
				fclose( handle );
				}
		FlagWrite = 0;
		}
*/
	/*
	if (ImgDsk!=NULL) {
		free(ImgDsk);
	}
	ImgDsk=NULL;
			*/
}


/********************************************************* !NAME! **************
* Nom : SetDiskUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialise l'accès à une disquette (insertion)
*
* Résultat    : /
*
* Variables globales modifiées : NomFic, Image, FlagWrite
*
********************************************************** !0! ****************/
void SetDiskUPD( char * n )
{
	/*
	FILE * handle;
	
	EjectDiskUPD();
	NomFic = n;
	handle = fopen( NomFic, "rb" );
	FAT_fseek(handle,0,SEEK_END);
	LongFic = ftell(handle) - sizeof(Infos);
	ImgDsk = (u8*)MyAlloc(dsksize, "UPD Disk");
	FAT_fseek(handle,0,SEEK_SET);
		
	if ( handle )
		{
		fread( &Infos, sizeof( Infos ), 1, handle );
		fread( ImgDsk, 1, LongFic, handle );
		fclose( handle );
		Image = 1;
		}
	FlagWrite = 0;
	ChangeCurrTrack( 0 );
	*/
}


/********************************************************* !NAME! **************
* Nom : GetCurrTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Retourne la piste courrante
*
* Résultat    : C
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
int GetCurrTrack( void )
{
	return( C );
}

void LireDiskMem(u8 *rom, u32 romsize)
{
	// myprintf("EjectDiskUPD");
	EjectDiskUPD();

	LongFic=romsize-sizeof(Infos);

	if (ImgDsk!=NULL) {
		free(ImgDsk);
	}
	ImgDsk=(u8*)MyAlloc(LongFic, "Loading IMAGE");
	
	memcpy(&Infos, rom, sizeof(Infos));
	memcpy(ImgDsk, rom+sizeof(Infos), LongFic);
	Image=1;
	DispIcons();	
	FlagWrite=0;
	
	ChangeCurrTrack(0);  // Met a jour posdata
}

/*
int dsk_load (char *pchFileName, t_drive *drive, char chID) {
	int iRetCode;
	u32 dwTrackSize, track, side, sector, dwSectorSize, dwSectors;
	u8 *pbPtr, *pbDataPtr, *pbTempPtr, *pbTrackSizeTable;

	iRetCode = 0;

	dsk_eject(drive);
	//if ((pfileObject = FAT_fopen(pchFileName, "rb")) != NULL) {
	pfileObject = fopen(pchFileName, "rb");
	fread(pbGPBuffer, 0x100, 1, pfileObject); // read DSK header
	pbPtr = pbGPBuffer;
	if (memcmp(pbPtr, "MV - CPC", 8) == 0) { // normal DSK image?
		drive->tracks = *(pbPtr + 0x30); // grab number of tracks
		if (drive->tracks > DSK_TRACKMAX) { // compare against upper limit
			drive->tracks = DSK_TRACKMAX; // limit to maximum
		}
		drive->sides = *(pbPtr + 0x31); // grab number of sides
		if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
			iRetCode = ERR_DSK_SIDES;
			goto exit;
		}
		dwTrackSize = (*(pbPtr + 0x32) + (*(pbPtr + 0x33) << 8)) - 0x100; // determine track size in bytes, minus track header
		drive->sides--; // zero base number of sides

		for (track = 0; track < drive->tracks; track++) { // loop for all tracks
			for (side = 0; side <= drive->sides; side++) { // loop for all sides
				fread(pbGPBuffer+0x100, 0x100, 1, pfileObject); // read track header
				pbPtr = pbGPBuffer + 0x100;
				
				if (memcmp(pbPtr, "Track-Info", 10) != 0) { // abort if ID does not match
					iRetCode = ERR_DSK_INVALID;//*(pbGPBuffer+ 0x100) + *pbPtr;
					goto exit;
				}
				dwSectorSize = 0x80 << *(pbPtr + 0x14); // determine sector size in bytes
				dwSectors = *(pbPtr + 0x15); // grab number of sectors
				if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
					iRetCode = ERR_DSK_SECTORS;
					goto exit;
				}

				drive->track[track][side].sectors = dwSectors; // store sector count
				drive->track[track][side].size = dwTrackSize; // store track size
				drive->track[track][side].data = (u8 *)malloc(dwTrackSize); // attempt to allocate the required memory
				
				if (drive->track[track][side].data == NULL) { // abort if not enough
					iRetCode = ERR_OUT_OF_MEMORY;
					goto exit;
				}
				pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
				pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
				for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
					memcpy(drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
					memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
					drive->track[track][side].sector[sector].size = dwSectorSize;
					drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
					pbDataPtr += dwSectorSize;
					
					pbPtr += 8;
				}
				if (!fread(pbTempPtr, dwTrackSize, 1, pfileObject)) { // read entire track data in one go
					iRetCode = ERR_DSK_INVALID;
					goto exit;
				}
			}
		}
		drive->altered = 0; // disk is as yet unmodified
	} else {
		if (memcmp(pbPtr, "EXTENDED", 8) == 0) { // extended DSK
			drive->tracks = *(pbPtr + 0x30); // number of tracks
			if (drive->tracks > DSK_TRACKMAX) {  
				drive->tracks = DSK_TRACKMAX;
			}
			drive->random_DEs = *(pbPtr + 0x31) & 0x80; // simulate random Data Errors?
			drive->sides = *(pbPtr + 0x31) & 3; // number of sides
			if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
				iRetCode = ERR_DSK_SIDES;
				goto exit;
			}
			pbTrackSizeTable = pbPtr + 0x34; // pointer to track size table in DSK header
			drive->sides--; // zero base number of sides
			for (track = 0; track < drive->tracks; track++) { // loop for all tracks
				for (side = 0; side <= drive->sides; side++) { // loop for all sides
					dwTrackSize = (*pbTrackSizeTable++ << 8); // track size in bytes
					if (dwTrackSize != 0) { // only process if track contains data
						dwTrackSize -= 0x100; // compensate for track header
						fread(pbGPBuffer+0x100, 0x100, 1, pfileObject); // read track header
						pbPtr = pbGPBuffer + 0x100;
						if (memcmp(pbPtr, "Track-Info", 10) != 0) { // valid track header?
							iRetCode = ERR_DSK_INVALID;
							goto exit;
						}
						dwSectors = *(pbPtr + 0x15); // number of sectors for this track
						if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
							iRetCode = ERR_DSK_SECTORS;
							goto exit;
						}
						drive->track[track][side].sectors = dwSectors; // store sector count
						drive->track[track][side].size = dwTrackSize; // store track size
						drive->track[track][side].data = (u8 *) malloc(dwTrackSize); // attempt to allocate the required memory
						if (drive->track[track][side].data == NULL) { // abort if not enough
							iRetCode = ERR_OUT_OF_MEMORY;
							goto exit;
						}
						pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
						pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
						for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
							memcpy(drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
							memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
							dwSectorSize = *(pbPtr + 0x1e) + (*(pbPtr + 0x1f) << 8); // sector size in bytes
							drive->track[track][side].sector[sector].size = dwSectorSize;
							drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
							pbDataPtr += dwSectorSize;
							pbPtr += 8;
						}
						if (!fread(pbTempPtr, dwTrackSize, 1, pfileObject)) { // read entire track data in one go
							iRetCode = ERR_DSK_INVALID;
							goto exit;
						}
					} 
					else {
						memset(&drive->track[track][side], 0, sizeof(t_track)); // track not formatted
					}
				}
			}
			drive->altered = 0; // disk is as yet unmodified
		} else {
			iRetCode = ERR_DSK_INVALID; // file could not be identified as a valid DSK
		}
	}
exit:
	fclose(pfileObject);

	if (iRetCode != 0) { // on error, 'eject' disk from drive
		dsk_eject(drive);
	}
	return iRetCode;
}
*/
