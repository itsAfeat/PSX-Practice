#ifndef MAIN_H
#define MAIN_H

#include <sys/types.h>  // This provides typedefs needed by libgte.h and libgpu.h
#include <stdio.h>	    // Not necessary but include it anyway
#include <libetc.h>	    // Includes some functions that controls the display
#include <libgte.h>	    // GTE header, not really used but libgpu.h depends on it
#include <libgpu.h>	    // GPU library header
#include "CD.h"

// ==========================================
//      STRUCTS
// ==========================================
    typedef struct _SPRITE
    {
        u_short tpage;  // Tpage value
        u_short clut;   // CLUT value
        u_char  u, v;   // UV offset (useful for non page aligned TIMs)
        u_char  w, h;   // Size (primitives can only draw 256x256 anyway)
        CVECTOR col;    // The color... duh
    } SPRITE;
    

// ==========================================
//      GLOBAL VARIABLES & DEFINES
// ==========================================
    // Ordering table length
    #define OTLEN 8

    // Pointers
    TILE     *tile;
    SPRT     *sprt;
    POLY_FT4 *poly;
    SVECTOR   s[4];
    SVECTOR   v[4];

    // Define environment pairs and buffer counter
    DISPENV disp[2];
    DRAWENV draw[2];
    int db;

    u_long ot[2][OTLEN];    // Ordering table length
    char pribuff[2][32768]; // Primitive buffer (32KB should be enough)
    char *nextpri;          // Next primitive pointer

    // TIM image parameters
    int tim_mode;
    int tim_uoffs,tim_voffs;
    RECT tim_prect,tim_crect;
    TIM_IMAGE img;

    // For switching between PAL and NTSC
    int video_mode = 0;   // 0 = PAL | 1 = NTSC

    int i, cx, cy;  // For for loops
    int prim_size;  // For primitive sizes

// ==========================================
//      CUSTOM FUNCTIONS
// ==========================================
void GetSprite(TIM_IMAGE *tim, SPRITE *sprite)
{
    // Get tpage value
    sprite->tpage = getTPage(tim->mode&0x3, 0,
        tim->prect->x, tim->prect->y);
    
    // Get CLUT value
    if (tim->mode & 0x8)
    { sprite->clut = getClut(tim->crect->x, tim->crect->y); }

    // Set sprite size
    sprite->w = tim->prect->w<<(2-tim->mode&0x3);
    sprite->h = tim->prect->h;

    // Set UV offset
    sprite->u = (tim->prect->x&0x3f)<<(2-tim->mode&0x3);
    sprite->v = tim->prect->y&0xff;

    // Set neutral color
    sprite->col.r = 128;
    sprite->col.g = 128;
    sprite->col.b = 128;
}

void SortSprite(int x, int y, SPRITE *sprite)
{
    SPRT *sprt;
    DR_TPAGE *tpage;

    sprt = (SPRT*)nextpri;
    setSprt(sprt);

    setXY0(sprt, x, y);
    setWH(sprt, sprite->w, sprite->h);
    setUV0(sprt, sprite->u, sprite->v);
    setRGB0(sprt,
        sprite->col.r,
        sprite->col.g,
        sprite->col.b);
    sprt->clut = sprite->clut;

    addPrim(ot[db], sprt);
    nextpri += sizeof(SPRT);

    tpage = (DR_TPAGE*)nextpri;
    setDrawTPage(tpage, 0, 1, sprite->tpage);
    addPrim(ot[db], tpage);

    nextpri += sizeof(DR_TPAGE);
}

void SortRotSprite(int x, int y, int pw, int ph, int angle, int scale, SPRITE *sprite)
{
    // Calculate the pivot point (center) of the sprite
    cx = pw>>1;
    cy = ph>>1;

    // Increment by 0.5 on the bottom and right coords so scaling
    // would increment a bit smoother
    s[0].vx = -(((pw*scale)>>12)-cx);
    s[0].vy = -(((ph*scale)>>12)-cy);

    s[1].vx = (((pw*scale)+2048)>>12)-cx;
    s[1].vy = s[0].vy;

    s[2].vx = -(((pw*scale)>>12)-cx);
    s[2].vy = (((ph*scale)+2048)>>12)-cy;

    s[3].vx = (((pw*scale)+2048)>>12)-cx;
    s[3].vy = s[2].vy;

    // A simple but pretty effective optimization trick
    cx = ccos( angle );
    cy = csin( angle );

    // Calculate rotated sprite coordinates
    for (i=0; i<4; i++)
    {
        v[i].vx = (((s[i].vx*cx)
            -(s[i].vy*cy))>>12)+x;
        v[i].vy = (((s[i].vy*cx)
            +(s[i].vx*cy))>>12)+y;
    }

    // Initialize the poly primitive for the sprite
    poly = (POLY_FT4*)nextpri;
    setPolyFT4(poly);

    // Set CLUT and tpage to the primitive
    setTPage(poly, tim_mode&0x3, 0, tim_prect.x, tim_prect.y);
    setClut(poly, tim_crect.x, tim_crect.y);

    // Set color, screen and texture coords of the primitive
    setRGB0(poly, 128, 128, 128);
    setXY4(poly,
        v[0].vx, v[0].vy,
        v[1].vx, v[1].vy,
        v[2].vx, v[2].vy,
        v[3].vx, v[3].vy);
    setUVWH(poly, tim_uoffs, tim_voffs, pw, ph);

    // Add it to the ordering table
    addPrim(ot[db], poly);
    nextpri += sizeof(POLY_FT4);
}

void LoadTexture(u_int *tim, TIM_IMAGE *tparam)
{
    // Read TIM parameters
    OpenTIM((u_long*) tim);
    ReadTIM(tparam);

    // Upload pixel data to framebuffer
    LoadImage(tparam->prect, tparam->paddr);
    DrawSync(0);

    // Upload CLUT to framebuffer if present
    if (tparam->mode & 0x8)
    {
        LoadImage(tparam->crect, tparam->caddr);
        DrawSync(0);
    }
}

void LoadTIM(void)
{
    TIM_IMAGE    img;       // TIM image parameters
    u_int       *filebuff;  // Pointer for the file loaded from the disc

    if (filebuff = (u_int*)LoadFile("\\SEJT.TIM;1"))
    {
        // On successful file read, load the texture to VRAM
        LoadTexture(filebuff, &img);

        // Copy the TIM coordinates;
        tim_prect   = *img.prect;
        tim_crect   = *img.crect;
        tim_mode    =  img.mode;

        // Calculate U,V offset for TIMs that are not page aligned
        tim_uoffs = (tim_prect.x%64)<<(2-(tim_mode&0x3));
        tim_voffs = (tim_prect.y&0xff);

        // Free the file buffer
        free(filebuff);
    }
    else
    {
        // Output error text that the image failed to load
        printf( "Error: TIM file not found.\n" );
    }
}


void Init(void)
{
    // --- GPU SETUP ---
    // Reset GPU and enable interrupts
    ResetGraph(0);

    // Initialize the CD-ROM library
    CdInit();

    // Checks whether we want PAL or NTSC
    if (video_mode == 0)
    {
        // Configures the pair of DISPENVs for 320x256 mode (PAL)
        SetDefDispEnv(&disp[0], 0, 0, 320, 256);
        SetDefDispEnv(&disp[1], 0, 256, 320, 256);

        // Screen offset to center the picture vertically
        disp[0].screen.y = 24;
        disp[1].screen.y = disp[0].screen.y;

        // Forces PAL video standard
        SetVideoMode(MODE_PAL);

        // Configures the pair of DRAWENVs for the DISPENVs
        SetDefDrawEnv(&draw[0], 0, 256, 320, 256);
        SetDefDrawEnv(&draw[1], 0, 0, 320, 256);
    }
    else    
    {
        // Configures the pair of DISPENVs for 320x240 mode (NTSC)
        SetDefDispEnv(disp, 0, 0, 320, 240);
        SetDefDispEnv(disp, 0, 240, 320, 240);

        // Configures the pair of DRAWENVs for the DISPENVs
        SetDefDrawEnv(&draw[0], 0, 240, 320, 240);
        SetDefDrawEnv(&draw[1], 0, 0, 320, 240);
    }

    // Specifies the clear color of the DRAWENV
    setRGB0(&draw[0], 63, 0, 127);
    setRGB0(&draw[1], 63, 0, 127);

    // Enable background clear
    draw[0].isbg = 1;
    draw[1].isbg = 1;

    // Apply environements
    PutDispEnv(&disp[0]);
    PutDrawEnv(&draw[0]);

    db = 0;                 // Make sure db starts at zero
    nextpri = pribuff[0];   // Set initial primitive pointer address

    // Load textures
    LoadTIM();

    // Set TPage of lone texture as initial TPage
    draw[0].tpage = getTPage(tim_mode&0x3, 0, tim_prect.x, tim_prect.y);
    draw[1].tpage = getTPage( tim_mode&0x3, 0, tim_prect.x, tim_prect.y );

    // apply initial drawing environment
    PutDrawEnv(&draw[!db]);

    // --- FONT STUFF ---
    // Load the internal font texture
    FntLoad(960, 0);
    // Create the text stream
    FntOpen(0, 8, 320, 224, 0, 100);
}

void Display(void)
{
    // Wait for the GPU to finish drawing and V-Blank
    DrawSync(0);
    VSync(0);

    // Apply environements
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);

    // Enable display
    SetDispMask(1);

    // Draw the ordering table
    DrawOTag(ot[db]+OTLEN-1);

    // Flip buffer counter
    db = !db;
    // Reset next primitive pointer
    nextpri = pribuff[db];

    FntFlush(-1);
    ClearOTagR(ot[db], OTLEN);
}

#endif // !MAIN_H