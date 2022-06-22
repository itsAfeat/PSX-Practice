#ifndef CORE_H
#define CORE_H

    #include <sys/types.h>
    #include <inline_c.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <psxetc.h>
    #include <psxgte.h>
    #include <psxgpu.h>
    #include <psxapi.h>
    #include <psxpad.h>
    #include <psxsio.h>
    #include <psxspu.h>
    #include <psxcd.h>

    #include "controller.h"
    #include "clip.h"
    

    // ==========================================
    //      GLOBAL DEFINES
    // ==========================================
    // OT and Packet Buffer sizes
    #define OT_LEN              1024
    #define PACKET_LEN          8192


    // For switching between PAL and NTSC (0 = PAL | 1 = NTSC)
    #define VIDEO_MODE          0

    #if VIDEO_MODE
        // NTSC screen resolution (320x240)
        #define SCREEN_XRES     320
        #define SCREEN_YRES     240
    #else
        // PAL screen resolution (320x256)
        #define SCREEN_XRES     320
        #define SCREEN_YRES     256
    #endif

    // Screen center position
    #define CENTERX             SCREEN_XRES>>1
    #define CENTERY             SCREEN_YRES>>1

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

    // Sector header structure for video sector terminator
    typedef struct SECTOR_HEAD
    {
        u_short	id;
        u_short chan;
        u_char	pad[28];
    } SECTOR_HEAD;

    
    // For easier handling of vertex indexes
    typedef struct {
        short v0,v1,v2,v3;
    } INDEX;

    // Double buffer structure
    typedef struct {
        DISPENV	disp;			// Display environment
        DRAWENV	draw;			// Drawing environment
        u_long 	ot[OT_LEN];		// Ordering table
        char 	p[PACKET_LEN];	// Packet buffer
    } DB;


    // ==========================================
    //      GLOBAL VARIABLES
    // ==========================================
    // XA audio handling stuff
    volatile int num_loops=0;			// Loop counter
    volatile int xa_play_channel;		// Currently playing channel number
    CdlLOC xa_loc;						// XA data start location

    // Double buffer variables
    DB		db[2];
    int		db_active = 0;
    char	*db_nextpri;
    RECT	screen_clip;

    // Pad data buffer
    char pad_buff[2][34];

    char xa_sector_buff[2048];

    // --- TEST CUBE ---

    // Cube vertices
    SVECTOR cube_verts[] = {
        { -100, -100, -100, 0 },
        {  100, -100, -100, 0 },
        { -100,  100, -100, 0 },
        {  100,  100, -100, 0 },
        {  100, -100,  100, 0 },
        { -100, -100,  100, 0 },
        {  100,  100,  100, 0 },
        { -100,  100,  100, 0 }
    };

    // Cube face normals
    SVECTOR cube_norms[] = {
        { 0, 0, -ONE, 0 },
        { 0, 0, ONE, 0 },
        { 0, -ONE, 0, 0 },
        { 0, ONE, 0, 0 },
        { -ONE, 0, 0, 0 },
        { ONE, 0, 0, 0 }
    };

    // Cube vertex indices
    INDEX cube_indices[] = {
        { 0, 1, 2, 3 },
        { 4, 5, 6, 7 },
        { 5, 4, 0, 1 },
        { 6, 7, 3, 2 },
        { 0, 2, 5, 7 },
        { 3, 1, 6, 4 }
    };

    // Number of faces of cube
    #define CUBE_FACES 6


    // Light color matrix
    // Each column represents the color matrix of each light source and is
    // used as material color when using gte_ncs() or multiplied by a
    // source color when using gte_nccs(). 4096 is 1.0 in this matrix
    // A column of zeroes effectively disables the light source.
    MATRIX color_mtx = {
        0, 0, 0,	// Red
        0, 0, 0,	// Green
        ONE, 0, 0	// Blue
    };

    // Light matrix
    // Each row represents a vector direction of each light source.
    // An entire row of zeroes effectively disables the light source.
    MATRIX light_mtx = {
        /* X,  Y,  Z */
        -2048 , -2048 , -2048,
        0	  , 0	  , 0,
        0	  , 0	  , 0
    };


    // ==========================================
    //      FUNCTION DECLARATIONS
    // ==========================================
    // --- CORE FUNCTIONS ---
    void Init();

    // --- DISPLAY FUNCTIONS ---
    void Display();
    void SortCube(MATRIX *mtx, VECTOR *pos, SVECTOR *rot);
    
    // --- CD FUNCTIONS ---
    char *LoadFile(const char *filename);

    // --- XA FUNCTIONS ---
    void xa_callback(int intr, unsigned char *result);

    // ==========================================
    //      CORE FUNCTIONS
    // ==========================================
    void Init()
    {
        // --- GPU SETUP ---
        // Reset GPU and enable interrupts
        ResetGraph(0);

        // Initialize the CD-ROM library
        CdInit();

        // Initialize the GTE
        InitGeom(),

        SpuInit();

	    // Set display and draw environment areas
	    // (display and draw areas must be separate, otherwise "hello flicker")
        SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_XRES, SCREEN_YRES);
        SetDefDrawEnv(&db[0].draw, SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);

	    // Enable draw area clear and dither processing
        setRGB0(&db[0].draw, 63, 0, 127);
        db[0].draw.isbg = 1;
        db[0].draw.dtd = 1;

        // Set display and draw environment areas
	    // (display and draw areas must be separate, otherwise "hello flicker")
        SetDefDispEnv(&db[1].disp, SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
        SetDefDrawEnv(&db[1].draw, 0, 0, SCREEN_XRES, SCREEN_YRES);

	    // Enable draw area clear and dither processing
        setRGB0(&db[1].draw, 63, 0, 127);
        db[1].draw.isbg = 1;
        db[1].draw.dtd = 1;

        // Checks whether we want PAL or NTSC
        if (VIDEO_MODE == 0)    // PAL
        {
            // Screen offset to center the picture vertically
            db[0].disp.screen.y = 24;
            db[1].disp.screen.y = 24;

            // Forces PAL video standard
            SetVideoMode(MODE_PAL);
        } else {                // NTSC
            SetVideoMode(MODE_NTSC);
        }


        // Apply initial drawing environment
        PutDrawEnv(&db[0].draw);
        
        // Clear both ordering tables to make sure they are clean at the start
        ClearOTagR(db[0].ot, OT_LEN);
        ClearOTagR(db[1].ot, OT_LEN);

	    // Set primitive pointer address
        db_nextpri = db[0].p;

        // Set clip region
        setRECT(&screen_clip, 0, 0, SCREEN_XRES, SCREEN_YRES);

        // --- GTE STUFF ---
        // Set GTE offset (recommended method  of centering)
        gte_SetGeomOffset(CENTERX, CENTERY);
        
        // Set screen depth (basically FOV control, W/2 works best)
        gte_SetGeomScreen(CENTERX);
        
        // Set light ambient color and light color matrix
        gte_SetBackColor(63, 63, 63);
        gte_SetColorMatrix(&color_mtx);


        // Set TPage of lone texture as initial TPage
        //draw[0].tpage = getTPage(tim_mode&0x3, 0, tim_prect.x, tim_prect.y);
        //draw[1].tpage = getTPage( tim_mode&0x3, 0, tim_prect.x, tim_prect.y );

        // --- FONT STUFF ---
        // Load the internal font texture
        FntLoad(960, 0);
        // Create the text stream
        FntOpen(32, 32, 256, 176, 2, 200);

        /*
        printf("\n\n");
        tmp_arr = LoadFile("\\ENGINE.VAG;1");
        for (int i = 0; i < sizeof(tmp_arr) / sizeof(tmp_arr[0]); i++)
        {
            printf("ENGINE.VAG [%d] = %d\n", i, tmp_arr[i]);
        }
        printf("\n");
        */
    }

    // ==========================================
    //      DISPLAY FUNCTIONS
    // ==========================================
    void Display()
    {
        // Wait for the GPU to finish drawing and V-Blank
        DrawSync(0);
        VSync(0);

        // Apply display/drawing environements
        PutDispEnv(&db[db_active].disp);
        PutDrawEnv(&db[db_active].draw);

        // Swap buffers
        db_active ^= 1;
        db_nextpri = db[db_active].p;

	    // Flush the font and clear the OT of the next frame
        FntFlush(-1);
        ClearOTagR(db[db_active].ot, OT_LEN);
        
        // Enable display
        SetDispMask(1);

        // Draw the ordering table
        DrawOTag(db[1-db_active].ot+(OT_LEN-1));
    }

    void SortCube(MATRIX *mtx, VECTOR *pos, SVECTOR *rot)
    {
        int i,p;
        POLY_F4 *pol4;
        
        // Object and light matrix for object
        MATRIX omtx,lmtx;
        
        // Set object rotation and position
        RotMatrix( rot, &omtx );
        TransMatrix( &omtx, pos );
        
        // Multiply light matrix to object matrix
        MulMatrix0( &light_mtx, &omtx, &lmtx );
        
        // Set result to GTE light matrix
        gte_SetLightMatrix( &lmtx );
        
        // Composite coordinate matrix transform, so object will be rotated and
        // positioned relative to camera matrix (mtx), so it'll appear as
        // world-space relative.
        CompMatrixLV( mtx, &omtx, &omtx );
        
        // Save matrix
        PushMatrix();
        
        // Set matrices
        gte_SetRotMatrix( &omtx );
        gte_SetTransMatrix( &omtx );
        
        // Sort the cube
        pol4 = (POLY_F4*)db_nextpri;
        
        for( i=0; i<CUBE_FACES; i++ ) {
            
            // Load the first 3 vertices of a quad to the GTE 
            gte_ldv3( 
                &cube_verts[cube_indices[i].v0], 
                &cube_verts[cube_indices[i].v1], 
                &cube_verts[cube_indices[i].v2] );
                
            // Rotation, Translation and Perspective Triple
            gte_rtpt();
            
            // Compute normal clip for backface culling
            gte_nclip();
            
            // Get result
            gte_stopz( &p );
            
            // Skip this face if backfaced
            if( p < 0 )
                continue;
            
            // Calculate average Z for depth sorting
            gte_avsz3();
            gte_stotz( &p );
            
            // Skip if clipping off
            // (the shift right operator is to scale the depth precision)
            if( ((p>>2) <= 0) || ((p>>2) >= OT_LEN) )
                continue;
            
            // Initialize a quad primitive
            setPolyF4( pol4 );
            
            // Set the projected vertices to the primitive
            gte_stsxy0( &pol4->x0 );
            gte_stsxy1( &pol4->x1 );
            gte_stsxy2( &pol4->x2 );
            
            // Compute the last vertex and set the result
            gte_ldv0( &cube_verts[cube_indices[i].v3] );
            gte_rtps();
            gte_stsxy( &pol4->x3 );
            
            // Test if quad is off-screen, discard if so
            if( quad_clip( &screen_clip,
            (DVECTOR*)&pol4->x0, (DVECTOR*)&pol4->x1, 
            (DVECTOR*)&pol4->x2, (DVECTOR*)&pol4->x3 ) )
                continue;
            
            // Load primitive color even though gte_ncs() doesn't use it.
            // This is so the GTE will output a color result with the
            // correct primitive code.
            gte_ldrgb( &pol4->r0 );
            
            // Load the face normal
            gte_ldv0( &cube_norms[i] );
            
            // Normal Color Single
            gte_ncs();
            
            // Store result to the primitive
            gte_strgb( &pol4->r0 );
            
            gte_avsz4();
            gte_stotz( &p );
            
            // Sort primitive to the ordering table
            addPrim( db[db_active].ot+(p>>2), pol4 );
            
            // Advance to make another primitive
            pol4++;
            
        }
        
        // Update nextpri
        db_nextpri = (char*)pol4;
        
        // Restore matrix
        PopMatrix();
    }


    // ==========================================
    //      CD FUNCTIONS
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
            buff = (char*)malloc(2048*numsecs);

            // Set read target to the file
            CdControl(CdlSetloc, (u_char*)&filePos.pos, 0);

            // Start read operation
            CdRead(numsecs, (u_long*)buff, CdlModeSpeed);

            // Wait until the read operation is complete
            CdReadSync(0, 0);
        }

        printf("\n\nENGINE.VAG FILESIZE = %d\n\n", filePos.size);
        return buff;
    }

    
    // ==========================================
    //      XA FUNCTIONS
    // ==========================================
    void xa_callback(int intr, unsigned char *result)
    {
        SECTOR_HEAD *sec;

        	// Only respond to data ready callbacks
	        if (intr == CdlDataReady)
            {
                // Fetch data sector
                CdGetSector((u_long*)&xa_sector_buff, 512);

                // Check if sector belongs to the currently playing channel
                sec  = (SECTOR_HEAD*)xa_sector_buff;

                if (sec->id == 352)
                {
                    // Debug
                    printf("ID=%d CHAN=%d PL=%d\n", sec->id, (sec->chan>>10)&0xF, xa_play_channel);

                    // Check if sector is of the currently playing channel
                    if (((sec->chan>>10)&0xF) == xa_play_channel)
                    {
                        num_loops++;

                        // Retry playback by seeking to start of XA data and stream
                        CdControlF(CdlReadS, (u_char*)&xa_loc); 

                        /* Stop playback */
				        //CdControlF(CdlPause, 0);
                    }
                }
            }
    }

#endif // !CORE_H