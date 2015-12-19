/* emu2149.h */
#ifndef _EMU2149_H_
#define _EMU2149_H_

//#include <nds.h>

#ifdef EMU2149_DLL_EXPORTS
#define EMU2149_API __declspec(dllexport)
#elif  EMU2149_DLL_IMPORTS
#define EMU2149_API __declspec(dllimport)
#else
#define EMU2149_API
#endif

#define EMU2149_VOL_DEFAULT 1
#define EMU2149_VOL_YM2149 0
#define EMU2149_VOL_AY_3_8910 1

#define PSG_MASK_CH(x) (1<<(x))

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(1) 
  typedef struct __PSG
  {

    /* Volume Table */
    unsigned int *voltbl;

    unsigned char reg[0x20];
    signed int out;

    unsigned int clk, rate, base_incr, quality;

    unsigned int count[3];
    unsigned int volume[3];
    unsigned int freq[3];
    unsigned int edge[3];
    unsigned int tmask[3];
    unsigned int nmask[3];
    unsigned int mask;

    unsigned int base_count;

    unsigned int env_volume;
    unsigned int env_ptr;
    unsigned int env_face;

    unsigned int env_continue;
    unsigned int env_attack;
    unsigned int env_alternate;
    unsigned int env_hold;
    unsigned int env_pause;
    unsigned int env_reset;

    unsigned int env_freq;
    unsigned int env_count;

    unsigned int noise_seed;
    unsigned int noise_count;
    unsigned int noise_freq;

    /* rate converter */
    unsigned int realstep;
    unsigned int psgtime;
    unsigned int psgstep;

    /* I/O Ctrl */
    unsigned int adr;
    
    unsigned int control;
    unsigned int reg_select;
  }
#pragma pack() 
  PSG;

  EMU2149_API void PSG_set_quality (PSG * psg, unsigned int q);
  EMU2149_API void PSG_set_rate (PSG * psg, unsigned int r);
  EMU2149_API PSG *PSG_new (unsigned int clk, unsigned int rate);
  EMU2149_API void PSG_reset (PSG *);
  EMU2149_API void PSG_delete (PSG *);
  //EMU2149_API void PSG_writeReg (PSG *, unsigned int reg, unsigned int val);
  EMU2149_API void PSG_writeIO (PSG * psg, unsigned int adr, unsigned int val);
  EMU2149_API unsigned char PSG_readReg (PSG * psg, unsigned int reg);
  EMU2149_API unsigned char PSG_readIO (PSG * psg);
  EMU2149_API signed short PSG_calc (PSG *);
  EMU2149_API void PSG_setVolumeMode (PSG * psg, int type);
  EMU2149_API unsigned int PSG_setMask (PSG *, unsigned int mask);
  EMU2149_API unsigned int PSG_toggleMask (PSG *, unsigned int mask);
    
#ifdef __cplusplus
}
#endif

#endif
