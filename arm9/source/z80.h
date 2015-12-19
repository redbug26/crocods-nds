/******************************************************************************/
/* Configuration pour l'archivage des différents éléments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// Définition du système       !CONFIG!=/V1!EMULATEUR CPC!
// Définition du sous système  !CONFIG!=/V2!PC-CPC!
// Définition du sous ensemble !CONFIG!=/V3!Chips!
// Définition du module        !CONFIG!=/V4!CPU Z80!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*  Fichier     :                       | Version : 0.1z
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Définitions pour le module Z80.C
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  30/03/2004              | L.DEPLANQUE             | Version 0.1z :
*                          |                         | Optimisations fonctions
*                          |                         | émulation Z80 : retour
*                          |                         | du nombre de cycles
*                          |                         | plutôt que incrément
*                          |                         | variable globale
* ------------------------------------------------------------------------------
********************************************************** !END! **************/


#ifndef Z80_H
#define Z80_H


#define     BIT0        0x01
#define     BIT1        0x02
#define     BIT2        0x04
#define     BIT3        0x08
#define     BIT4        0x10
#define     BIT5        0x20
#define     BIT6        0x40
#define     BIT7        0x80

//
// Flags Z80
//
#define     FLAG_0      0x00
#define     FLAG_C      0x01
#define     FLAG_N      0x02
#define     FLAG_V      0x04

#define     FLAG_H      0x10

#define     FLAG_Z      0x40
#define     FLAG_S      0x80


/********************************************************* !NAME! **************
* Nom : Registre
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Structure d'un registre Z80
*
********************************************************** !0! ****************/
typedef union
    {
    USHORT Word;
    struct
        {
        UBYTE Low;
        UBYTE High;
        } Byte;
    } Registre;


/********************************************************* !NAME! **************
* Nom : SRegs
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Structure Z80 (registres)
*
********************************************************** !0! ****************/
typedef struct
    {
    Registre    AF;
    Registre    BC;
    Registre    DE;
    Registre    HL;
    Registre    IR;
    UBYTE       IFF1;
    UBYTE       IFF2;
    Registre    IX;
    Registre    IY;
    Registre    SP;
    Registre    PC;
    UBYTE       InterruptMode;
    Registre    _AF;
    Registre    _BC;
    Registre    _DE;
    Registre    _HL;
    } SRegs;


typedef int ( * pfct )( void );

void ReadZ80(SRegs *z0);
void WriteZ80(SRegs *z0);

#define     RegAF           Z80.AF.Word
#define     RegBC           Z80.BC.Word
#define     RegDE           Z80.DE.Word
#define     RegHL           Z80.HL.Word

#define     Reg_AF          Z80._AF.Word
#define     Reg_BC          Z80._BC.Word
#define     Reg_DE          Z80._DE.Word
#define     Reg_HL          Z80._HL.Word

#define     RegSP           Z80.SP.Word
#define     RegPC           Z80.PC.Word
#define     RegIX           Z80.IX.Word
#define     RegIY           Z80.IY.Word
#define     RegIR           Z80.IR.Word

#define     RegA            Z80.AF.Byte.High
#define     FLAGS           Z80.AF.Byte.Low
#define     RegB            Z80.BC.Byte.High
#define     RegC            Z80.BC.Byte.Low
#define     RegD            Z80.DE.Byte.High
#define     RegE            Z80.DE.Byte.Low
#define     RegH            Z80.HL.Byte.High
#define     RegL            Z80.HL.Byte.Low
#define     RegI            Z80.IR.Byte.High
#define     RegR            Z80.IR.Byte.Low
#define     RegIXH          Z80.IX.Byte.High
#define     RegIXL          Z80.IX.Byte.Low
#define     RegIYH          Z80.IY.Byte.High
#define     RegIYL          Z80.IY.Byte.Low




extern int IRQ;


UBYTE Peek8Ext( USHORT adr );

void Poke8Ext( USHORT adr, UBYTE val );

int Z80_NMI( void );
int ___C9( void );

int ExecInstZ80_orig(void);
void ResetZ80_orig(void);
void SetIRQZ80_orig(int i);

#endif
