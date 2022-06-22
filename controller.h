#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>  // This provides typedefs needed by libgte.h and libgpu.h
#include <stdio.h>	    // Not necessary but include it anyway
#include <stdlib.h>
#include <psxetc.h>	    // Includes some functions that controls the display
#include <psxgte.h>	    // GTE header, not really used but libgpu.h depends on it
#include <psxgpu.h>	    // GPU library header
#include <psxapi.h>     // API header, has InitPAD() and StartPAD() defs
#include <psxpad.h>     // Header for containing all the pad buttons

// ==========================================
//      GLOBAL VARIABLES & DEFINES
// ==========================================
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
    InitPAD(&padbuff[0][0], 34, &padbuff[1][0], 34);
    
    padbuff[0][0] = padbuff[0][1] = 0xff;
    padbuff[1][0] = padbuff[1][1] = 0xff;
    
    StartPAD();
    ChangeClearPAD(1);
}

PADTYPE* PoolPads()
{ return (PADTYPE*)padbuff[0]; }

int IsConnected(PADTYPE *pad)
{ return pad->stat == 0; }

int IsPressed(PADTYPE *pad, int button)
{ return !( pad->btn & button ); }

#endif // !CONTROLLER_H