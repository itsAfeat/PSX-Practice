#ifndef CD_H
#define CD_H

#include <sys/types.h>  // This provides typedefs needed by libgte.h and libgpu.h
#include <stdio.h>	    // Not necessary but include it anyway
#include <stdlib.h>
#include <libetc.h>	    // Includes some functions that controls the display
#include <libgte.h>	    // GTE header, not really used but libgpu.h depends on it
#include <libgpu.h>	    // GPU library header
#include <libapi.h>     // API header, has InitPAD() and StartPAD() defs
#include <libcd.h>      // CD library header

// ==========================================
//      CUSTOM FUNCTIONS
// ==========================================
char *LoadFile(const char *filename)
{
    CdlFILE filePos;
    int numsecs;
    char *buff;

    // Locate the file on the CD
    if (CdSearchFile(&filePos, (char*)filename) == NULL)
    {
        // Print error message if file not found
        printf("%s not found", filename);
    }
    else
    {
        // Calculate number of sectors to read for the file
        numsecs = (filePos.size+2047)/2048;

        // Allocate buffer for the file
        buff = (char*)malloc3(2048*numsecs);
        printf(buff);

        // Set read target to the file
        CdControl(CdlSetloc, (u_char*)&filePos.pos, 0);

        // Start read operation
        CdRead(numsecs, (u_long*)buff, CdlModeSpeed);

        // Wait until the read operation is complete
        CdReadSync(0, 0);
    }

    return buff;
}

#endif // !CD_H