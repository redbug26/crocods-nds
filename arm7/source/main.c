//////////////////////////////////////////////////////////////////////
// Simple ARM7 stub (sends RTC, TSC, and X/Y data to the ARM 9)
// -- joat
// -- modified by Darkain and others
//////////////////////////////////////////////////////////////////////

#include "ipc2.h"

#include <nds.h>
#include <string.h>

#include <dswifi7.h>

#include "emu2149.h"

int SoundPause = true;
  
PSG * psg;

#define CPC_CLK 2000000
#define MSX_CLK 3579545
#define SAMPLERATE  11025  //22050
#define SOUNDBUFCNT ((u16)(1024))   //367

#define TIMERCOUNT(N)  ((int)( 0x2000000 / (N) ))

s16 * soundbuf;
s16 * sbuf;
int wptr  = 0; // データ書込位置
int wbufp = 0; // データ書込バッファ
int SoundProcTiming = 0;

//////////////////////////////////////////////////////////////////////
EMU2149_API PSG *_PSG_new (u32 c, u32 r) {
  memset((char*)psg, 0, sizeof (PSG));

  PSG_setVolumeMode (psg, EMU2149_VOL_DEFAULT);
  psg->clk = c;
  psg->rate = r ? r : 44100;
  PSG_set_quality (psg, 0);

  return psg;
}

void InitSound(int r) {
	// Timer setting
	TIMER0_DATA = (u16)(0x10000 - TIMERCOUNT(r) );
	TIMER0_CR = TIMER_DIV_1 | TIMER_ENABLE;     // | TIMER_IRQ_REQ;

	TIMER1_DATA = (u16)(0x10000 - SOUNDBUFCNT);
	TIMER1_CR = TIMER_CASCADE | TIMER_ENABLE ;  // | TIMER_IRQ_REQ ;

	psg = (PSG*)&(IPC2->psg);
	
	soundbuf = (s16*)(IPC2->soundbuf);
	sbuf = soundbuf;

	// PSG initialize
	_PSG_new(CPC_CLK,r);
	PSG_reset( psg );
}

//////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------
void stopSound(void) {
  u8 ucBcl;
  for (ucBcl=0; ucBcl<16; ucBcl++)  {
    SCHANNEL_CR(ucBcl)     = 0;
  }
}

void startSound(int sampleRate, const void* data, uint32 bytes, u8 channel, u8 vol,  u8 pan, u8 format) {
  SCHANNEL_CR(channel)     = 0;
  SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
  SCHANNEL_SOURCE(channel) = (uint32)data;
  SCHANNEL_LENGTH(channel) = bytes >> 2;
  SCHANNEL_REPEAT_POINT(channel) = 0;
  SCHANNEL_CR(channel)     = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(vol) | SOUND_PAN(pan) | (format==1? SOUND_FORMAT_8BIT:SOUND_FORMAT_16BIT);
}

//---------------------------------------------------------------------------------
void startSoundNoRepeat(int sampleRate, const void* data, u32 bytes, u8 channel, u8 vol,  u8 pan, u8 format) {
  SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
  SCHANNEL_SOURCE(channel) = (u32)data;
  SCHANNEL_LENGTH(channel) = bytes >> 2 ;
  SCHANNEL_CR(channel)     = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(vol) | SOUND_PAN(pan) | (format==1?SOUND_8BIT:SOUND_16BIT);
}

bool isFreeSoundChannel(int i) {
  if ( (SCHANNEL_CR(i) & SCHANNEL_ENABLE) == 0 ) return true;
  return false;
}

void ProcSound() {
	signed short P;
	P=0;

	P = PSG_calc(psg)<<1;
	*sbuf = P; //O+(A>>2);

	sbuf ++;
	wptr ++ ;
	if( wptr >= SOUNDBUFCNT ){
		startSound(SAMPLERATE, soundbuf + (wbufp*SOUNDBUFCNT), SOUNDBUFCNT*2, wbufp , 0x7f, 63, 0);
		wbufp = wbufp==0?1:0; 
		sbuf = soundbuf + wbufp*SOUNDBUFCNT;
		wptr = 0;
	}
}

int vcount;
touchPosition first,tempPos;

//---------------------------------------------------------------------------------
void VcountHandler() {
  static int lastbut = -1;
  
  uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;
  
  but = REG_KEYXY;
  
  if (!( (but ^ lastbut) & (1<<6))) {
  
    tempPos = touchReadXY();
  
    x = tempPos.x;
    y = tempPos.y;
    xpx = tempPos.px;
    ypx = tempPos.py;
    z1 = tempPos.z1;
    z2 = tempPos.z2;
    
  } else {
    lastbut = but;
    but |= (1 <<6);
  }
  
  if ( vcount == 80 ) {
    first = tempPos;
  } else {
    if (	abs( xpx - first.px) > 10 || abs( ypx - first.py) > 10 ||
        (but & ( 1<<6)) ) {
  
      but |= (1 <<6);
      lastbut = but;
  
    } else { 	
      IPC->mailBusy = 1;
      IPC->touchX			= x;
      IPC->touchY			= y;
      IPC->touchXpx		= xpx;
      IPC->touchYpx		= ypx;
      IPC->touchZ1		= z1;
      IPC->touchZ2		= z2;
      IPC->mailBusy = 0;
    }
  }
  IPC->buttons		= but;
  vcount ^= (80 ^ 130);
  SetYtrigger(vcount);
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
  u32 i;
  
  // sound code  :)
  TransferSound *snd = IPC->soundData;
  if (snd) {
    for (i=0; i<snd->count; i++) {
      u8 chan = snd->data[i].pan; // getFreeSoundChannel();
      switch (chan) {
        /*
        case 1:
          //chan = getFreeSoundChannel();
          //if (chan <16)
          startSoundWav(snd->data[i].rate, snd->data[i].data, snd->data[i].len, 0, snd->data[i].vol,snd->data[i].format);
          break;
        case 8: case 9: case 10: case 11: case 12: case 13:
          startSoundPSG(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
          break;
        case 14:
          startSoundWNS(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
          break;
          */
        case 98:
          //chan = getFreeSoundChannel();
          //if (chan <16)
            startSoundNoRepeat(snd->data[i].rate, snd->data[i].data, snd->data[i].len, 1, snd->data[i].vol, 64, 1);
          break;
        case 99:
          stopSound();
          break;
        default:
          //if (snd->data[i].len == 0) {
          //  modifySound(snd->data[i].rate, chan, snd->data[i].vol, 64, 1);
         // }
          //else {
            startSound(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, 64, 1);
          //}
          break;
      }
    }
  }
  IPC->soundData = 0;

	Wifi_Update(); // update wireless in vblank
} 

// callback to allow wifi library to notify arm9
void arm7_synctoarm9() { // send fifo message
   REG_IPC_FIFO_TX = 0x87654321;
}

enum {
   CMD_WAIT,
   WIFI_INIT,
   MUS_INIT
};
   
u32 fifo_status = CMD_WAIT;

void arm7_fifo() { // check incoming fifo messages
	while ( !(REG_IPC_FIFO_CR & (IPC_FIFO_RECV_EMPTY)) ) {
    u32 msg = REG_IPC_FIFO_RX;

		switch (fifo_status) {
      case WIFI_INIT:
        Wifi_Init(msg);
        Wifi_SetSyncHandler(arm7_synctoarm9); // allow wifi lib to notify arm9
        fifo_status = CMD_WAIT;
        break;

      case CMD_WAIT:
        if(msg==0x87654321)
          Wifi_Sync();
  
        if(msg==0x12345678) {
          fifo_status = WIFI_INIT;
        }
        break;
    }
	}
}

void InterruptHandler(void) {

/*	if (REG_IF & IRQ_TIMER1) {
		startSound(SAMPLERATE, soundbuf + wbufp*SOUNDBUFCNT, SOUNDBUFCNT*2, wbufp , 0x7f, 63, 0);
		wbufp = wbufp==0?1:0; 
		sbuf = soundbuf + wbufp*SOUNDBUFCNT;
		wptr = 0;
		VBLANK_INTR_WAIT_FLAGS |= IRQ_TIMER1;
	}
*/
	if (REG_IF & IRQ_FIFO_NOT_EMPTY) {
		arm7_fifo();
		VBLANK_INTR_WAIT_FLAGS |= IRQ_FIFO_NOT_EMPTY;
	}

	if (REG_IF & IRQ_WIFI) {
		Wifi_Interrupt();
		VBLANK_INTR_WAIT_FLAGS |= IRQ_WIFI;
	}

	if (REG_IF & IRQ_VCOUNT) {
		VcountHandler();
		VBLANK_INTR_WAIT_FLAGS |= IRQ_VCOUNT;
	}

	if (REG_IF & IRQ_VBLANK) {
		VblankHandler();
		VBLANK_INTR_WAIT_FLAGS |= IRQ_VBLANK;
	}

	// Acknowledge interrupts
	REG_IF = REG_IF;
}

int main(int argc, char ** argv) {
  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR; // enable & prepare fifo asap

  //u32 old_reg = readPowerManagement(PM_CONTROL_REG);
  //writePowerManagement(PM_CONTROL_REG, old_reg |PM_LED_ON); // enable fast

  // Reset the clock if needed
  rtcReset();
  
  //enable sound
  powerON(POWER_SOUND);
  SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
  IPC->soundData = 0;

  irqInit();

  // Init sound engine
  sbuf = soundbuf;

  SetYtrigger(80);
  vcount = 80;

  irqSet(IRQ_VBLANK, VblankHandler);
  irqSet(IRQ_VCOUNT, VcountHandler);
  //irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

	irqSet(IRQ_WIFI, Wifi_Interrupt); // set up wifi interrupt
	irqSet(IRQ_FIFO_NOT_EMPTY,arm7_fifo);
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;
	irqEnable(IRQ_FIFO_NOT_EMPTY | IRQ_WIFI | IRQ_VBLANK | IRQ_VCOUNT);

/*
	// Set up the interrupt handler
	REG_IME = 0;
	IRQ_HANDLER = &InterruptHandler;
	REG_IE  = IRQ_FIFO_NOT_EMPTY | IRQ_WIFI | IRQ_VBLANK | IRQ_VCOUNT ; //| IRQ_TIMER0;
	REG_IF  = ~0;
	REG_DISPSTAT = DISP_VBLANK_IRQ;

	REG_IME = 1;
*/
	Wifi_SetSyncHandler(arm7_synctoarm9);

  
	while (1) {
		switch( IPC2->soundCommand ){
			case 1: // Pause
				SoundPause = true;
				IPC2->soundCommand = 0;
				break;
			case 2: // Play
				SoundPause = false;
				SoundProcTiming = 0;
				wptr  = 0;
				wbufp = 0;
				IPC2->soundCommand = 0;
				break;
			case 3: // Sound Initialize
				InitSound(SAMPLERATE);
				IPC2->soundCommand = 0;
				break;
		}
		if( !SoundPause ){
			if( SoundProcTiming != TIMER1_DATA ){
				SoundProcTiming = TIMER1_DATA;
				ProcSound();
			}
		}
		//swiWaitForVBlank();
	}
	return 0;
}
