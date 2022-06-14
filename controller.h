#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>  // This provides typedefs needed by libgte.h and libgpu.h
#include <stdio.h>	    // Not necessary but include it anyway
#include <stdlib.h>
#include <libetc.h>	    // Includes some functions that controls the display
#include <libgte.h>	    // GTE header, not really used but libgpu.h depends on it
#include <libgpu.h>	    // GPU library header
#include <libapi.h>     // API header, has InitPAD() and StartPAD() defs

// ==========================================
//      STRUCTS
// ==========================================
typedef struct _PADTYPE
{
    unsigned char       stat;
    unsigned char       len:4;
    unsigned char       type:4;
    unsigned short      btn;
    unsigned char       rs_x,rs_y;
    unsigned char       ls_x,ls_y;
} PADTYPE;

// ==========================================
//      GLOBAL VARIABLES & DEFINES
// ==========================================
// --- PAD BUTTONS ---
#define PAD_SELECT      1
#define PAD_L3          2
#define PAD_R3          4
#define PAD_START       8
#define PAD_UP          16
#define PAD_RIGHT       32
#define PAD_DOWN        64
#define PAD_LEFT        128
#define PAD_L2          256
#define PAD_R2          512
#define PAD_L1          1024
#define PAD_R1          2048
#define PAD_TRIANGLE    4096
#define PAD_CIRCLE      8192
#define PAD_CROSS       16384
#define PAD_SQUARE      32768

// --- PAD TYPES ---
#define PTYPE_MOUSE     0X1
#define PTYPE_NEGCON    0x2
#define PTYPE_DIGPAD    0x4
#define PTYPE_ANALOG    0x5
#define PTYPE_DUALSH    0x7
#define PTYPE_NAMCO     0xE

u_char padbuff[2][34];

// ==========================================
//      CUSTOM FUNCTIONS
// ==========================================
void InitPads(void)
{
    InitPAD((char*)padbuff[0], 34, (char*)padbuff[1], 34);
    
    padbuff[0][0] = padbuff[0][1] = 0xff;
    padbuff[1][0] = padbuff[1][1] = 0xff;
    
    StartPAD();
}

PADTYPE* PoolPads()
{ return (PADTYPE*)padbuff[0]; }

int IsConnected(PADTYPE *pad)
{ return pad->stat == 0; }

int IsPressed(PADTYPE *pad, int button)
{ return !( pad->btn & button ); }

#endif // !CONTROLLER_H