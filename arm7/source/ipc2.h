#ifndef NDS_IPC2_INCLUDE
#define NDS_IPC2_INCLUDE


#include <nds/ipc.h>
#include "emu2149.h"

//---------------------------------------------------------------------------------
typedef struct sTransferRegion2 {
//---------------------------------------------------------------------------------
  TransferRegion ipc;
  int16 touchXave, touchYave;

  uint16 soundCommand;
  PSG psg;
  u16 * soundbuf;

} 
TransferRegion2, * pTransferRegion2;

#define IPC2 ((TransferRegion2 volatile *)(0x027FF000))

#endif
