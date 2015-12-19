#include <nds.h>
#include "types.h"

#ifndef SOUND_H
#define SOUND_H

#include "sound.h"
#include "plateform.h"

#include "../../arm7/source/ipc2.h"

PSG *psg=NULL;


bool PlaySound( void )
{
	IPC2->soundCommand = 2; 
    
	return 1;
}

void PauseSound( void )
{
	IPC2->soundCommand = 1; 
}


void Reset8912( void )
{
  // PSG
	if (psg==NULL) {
		psg = (PSG*)&(IPC2->psg);
		IPC2->soundbuf = (u16*) malloc( 1024 * 4 * 2 );
	}

	  psg->control = 0;

	IPC2->soundCommand = 3;
	while( IPC2->soundCommand )
		swiWaitForVBlank();
  swiWaitForVBlank();
  swiWaitForVBlank();
  IPC2->soundCommand=2;

	IPC2->soundCommand = 3; 
}

void Write8912( u32 reg, u32 val )            
{
  int c;

  if (reg > 15) return;

  psg->reg[reg] = (u8) (val & 0xff);
  switch (reg) {
    case 0:
    case 2:
    case 4:
    case 1:
    case 3:
    case 5:
      c = reg >> 1;
      psg->freq[c] = ((psg->reg[c * 2 + 1] & 15) << 8) + psg->reg[c * 2];
      break;
    case 6:
      psg->noise_freq = (val == 0) ? 1 : ((val & 31) << 1);
      break;
    case 7:
      psg->tmask[0] = (val & 1);
      psg->tmask[1] = (val & 2);
      psg->tmask[2] = (val & 4);
      psg->nmask[0] = (val & 8);
      psg->nmask[1] = (val & 16);
      psg->nmask[2] = (val & 32);
      break;
    case 8:
    case 9:
    case 10:
      psg->volume[reg - 8] = val << 1;
      break;
    case 11:
    case 12:
      psg->env_freq = (psg->reg[12] << 8) + psg->reg[11];
      break;
    case 13:
      psg->env_continue = (val >> 3) & 1;
      psg->env_attack = (val >> 2) & 1;
      psg->env_alternate = (val >> 1) & 1;
      psg->env_hold = val & 1;
      psg->env_face = psg->env_attack;
      psg->env_pause = 0;
      psg->env_count = 0x10000 - psg->env_freq;
      psg->env_ptr = psg->env_face?0:0x1f;
      break;
    case 14:
    case 15:
    default:
      break;
  }

  return;
}

int Read8912( int r )
{
    return (u8) (psg->reg[r & 0x1f]);
}


#endif
