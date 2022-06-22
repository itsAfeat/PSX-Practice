/* Controls:
 *  Up			- Look up
 *	Down		- Look down
 *	Left		- Look left
 *	Right		- Look right
 *	Triangle	- Move forward
 *	Cross		- Move backward
 *	Square		- Strafe left
 *	Circle		- Strafe right
 *	R1			- Slide up
 *	R2			- Slide down
 */


#include "core.h"

    // ==========================================
    //      GLOBAL VARIABLES
    // ==========================================
	int i,p,xy_temp;
	int px,py;
    
    CdlFILE	file;
    CdlFILTER filter;
    
    int counter=0,sel_channel=0;
	int p_up=0,p_down=0,p_right=0,p_cross=0,p_circle=0;
	
	SVECTOR	rot;			// Rotation vector for cube
	VECTOR	pos;			// Position vector for cube
	
	SVECTOR verts[17][17];	// Vertex array for floor
	
	VECTOR	cam_pos;		// Camera position (in fixed point integers)
	VECTOR	cam_rot;		// Camera view angle (in fixed point integers)
	int		cam_mode;		// Camera mode (between first-person and look-at)
	
	VECTOR	tpos;			// Translation value for matrix calculations
	SVECTOR	trot;			// Rotation value for matrix calculations
	MATRIX	mtx,lmtx;		// Rotation matrices for geometry and lighting
	
	PADTYPE *pad;			// Pad structure pointer for parsing controller
	
	POLY_F4	*pol4;			// Flat shaded quad primitive pointer

    // ==========================================
    //      FUNCTION DECLARATIONS
    // ==========================================
    void Update(void);
    void Draw(void);

    // ==========================================
    //      MAIN GAME CODE
    // ==========================================
    int main()
    {
        // Init graphics and GTE
        Init();
        InitPads();

        // Set coordinates to the vertex array for the floor
        for (py = 0; py < 17; py++) {
            for (px = 0; px < 17; px++) {
                setVector(&verts[py][px],
                    (100*(px-8))-50,
                    0,
                    (100*(py-8))-50
                );
            }
        }

        // Default camera coordinates
        setVector(&cam_pos, 0, ONE*-200, 0);
        setVector(&cam_rot, 0, 0, 0); // 180 degress = ONE*2048

        if (!CdSearchFile(&file, "\\JUGPOT.XA"))
        {
            printf("\nUNABLE TO FIND JUGPOT.XA\n");
            return 0;
        }
        else
        {
            int sec;
            sec = CdPosToInt(&file.pos);
            printf("\nXA LOCATED AT SECTOR %d : SIZE %d.\n", sec, file.size);
        }

        // Save file location as XA location
        xa_loc = file.pos;

        // Hook XA callback function to CdReadyCallback (for auto stop/loop)
        CdReadCallback(xa_callback);

        // Set CD mode for XA streaming (2x speed, send XA to SPU, enable filter
        i = CdlModeSpeed|CdlModeRT|CdlModeSF;
        CdControl(CdlSetmode, (u_char*)&i, 0);

        // Set file 1 on filter for channels 0-7
        filter.file = 1;

        // Main loop
        while (1)
        {
            FntPrint(-1, "\n XA AUDIO DEBUG\n\n");
            FntPrint(-1, " CHANNEL:\n");
            
            for(i=0; i<8; i++)
            {
                if( i == sel_channel )
                { FntPrint(-1, " -->%d\n", i); }
                else
                { FntPrint(-1, "    %d\n", i); }
            }
            
            FntPrint(-1, "\n CURRENT=%d STATUS=%x LOOPS=%d\n", 
                xa_play_channel, CdStatus(), num_loops);
            FntPrint(-1, "\n <X>-PLAY (START) <O>-STOP\n <R>-SET CHANNEL\n");


            Update();
            Draw();
            Display();


            counter++;
            if ((counter%60) == 59)
            {
                CdControl(CdlNop, 0, 0);
            }
        }

        return 0;
    }


void Update(void)
{
    pad = PoolPads();   // Set pad buffer data to pad pointer
    cam_mode = 0;       // Parse controller input

    // Dividde out fractions of camera rotation
    trot.vx = cam_rot.vx >> 12;
    trot.vy = cam_rot.vy >> 12;
    trot.vz = cam_rot.vz >> 12;

    if (IsConnected(pad))
    {
        /*
        if ((pad->type == PTYPE_DIGPAD) ||
            (pad->type == PTYPE_ANALOG) ||
            (pad->type == PTYPE_DUALSH) )
        {
            if (IsPressed(pad, PAD_UP))
            { cam_rot.vx -= ONE*16; }    // Look up
            if (IsPressed(pad, PAD_DOWN))
            { cam_rot.vx += ONE*16; }    // Look down
            if (IsPressed(pad, PAD_RIGHT))
            { cam_rot.vy -= ONE*16; }    // Look right
            if (IsPressed(pad, PAD_LEFT))
            { cam_rot.vy += ONE*16; }    // Look left
        }
        */

        if (IsPressed(pad, PAD_UP))
        {
            if (!p_up)
            {
                if (sel_channel > 0) { sel_channel--; }
                p_up = 1;
            }
        }
        else { p_up = 0; }

        if (IsPressed(pad, PAD_DOWN))
        {
            if (!p_down)
            {
                if (sel_channel < 7) { sel_channel++; }
                p_down = 1;
            }
        }
        else { p_down = 0; }

        if (IsPressed(pad, PAD_CROSS))
        {
            if (!p_cross)
            {
                filter.chan = sel_channel;
                CdControl(CdlSetfilter, (u_char*)&filter, 0);
                CdControl(CdlReadS, (u_char*)&xa_loc, 0);
                xa_play_channel = sel_channel;
                p_cross = 1;
            }
        }
        else { p_cross = 0; }
        
        if (IsPressed(pad, PAD_CIRCLE))
        {
            if (!p_circle)
            {
                CdControl(CdlPause, 0, 0);
                p_circle = 1;
            }
        }
        else { p_circle = 0; }
        
        if (IsPressed(pad, PAD_RIGHT))
        {
            if (!p_right)
            {
                filter.chan = sel_channel;
                CdControl(CdlSetfilter, (u_char*)&filter, 0);
                xa_play_channel = sel_channel;
                p_right = 1;
            }
        }
        else { p_right = 0; }

        // Up and down sliding
        if(IsPressed(pad, PAD_R2))
        {
            // Slide up
            cam_pos.vx -= ((isin(trot.vy)*isin(trot.vx))>>12)<<2;
            cam_pos.vy -= icos(trot.vx)<<2;
            cam_pos.vz += ((icos(trot.vy)*isin(trot.vx))>>12)<<2;
        }
        
        if(IsPressed(pad, PAD_L2))
        {
            // Slide down
            cam_pos.vx += ((isin(trot.vy)*isin(trot.vx))>>12)<<2;
            cam_pos.vy += icos(trot.vx)<<2;
            cam_pos.vz -= ((icos( trot.vy)*isin(trot.vx))>>12)<<2;
        }

        // Analog stick looking and moving

        if ((pad->type == PTYPE_ANALOG) ||
            (pad->type == PTYPE_DUALSH) )
        {
            // Moving forwards and backwards
            if (((pad->ls_y-128) < 16) || ((pad->ls_y-128) > 16))
            {
                cam_pos.vx += (((isin(trot.vy)*icos(trot.vx))>>12)*(pad->ls_y-128))>>4;
                cam_pos.vy -= (isin(trot.vx)*(pad->ls_y-128))>>4;
                cam_pos.vz -= (((icos(trot.vy)*icos(trot.vx))>>12)*(pad->ls_y-128))>>4;
            }
            // Strafing left and right

            if(((pad->ls_x-128) < -16) || ((pad->ls_x-128) > 16) )
            {
                cam_pos.vx += (icos(trot.vy)*(pad->ls_x-128))>>4;
                cam_pos.vz += (isin(trot.vy)*(pad->ls_x-128))>>4;
            }
				
            // Look up and down
            if(((pad->rs_y-128) < -16) || ((pad->rs_y-128) > 16))
            { cam_rot.vx += (pad->rs_y-128)<<10; }
            
            // Look left and right
            if(((pad->rs_x-128) < -16) || ((pad->rs_x-128) > 16))
            { cam_rot.vy -= (pad->rs_x-128)<<10; }


            // Debug reset
            if (IsPressed(pad, PAD_SELECT))
            {
                setVector(&cam_pos, 0, ONE*-200, 0);
                setVector(&cam_rot, 0, 0, 0);
            }
        }
    }
}

void Draw(void)
{
    /* Print out some info
    FntPrint(-1, " BUTTONS=%04x\n", pad->btn);
    FntPrint(-1, " X=%d Y=%d Z=%d\n", 
        cam_pos.vx>>12, 
        cam_pos.vy>>12, 
        cam_pos.vz>>12);
    FntPrint(-1, " RX=%d RY=%d\n", 
        cam_rot.vx>>12, 
        cam_rot.vy>>12);
        */

    // First-person camera mode
    if (cam_mode == 0)
    {
        // Set rotation to the matrix
        RotMatrix(&trot, &mtx);

        // Divide out the fractions of camera coordinates and invert
        // the sign, so camera coordinates will line up to world
        // (or geometry) coordinates
        tpos.vx = -cam_pos.vx >> 12;
        tpos.vy = -cam_pos.vy >> 12;
        tpos.vz = -cam_pos.vz >> 12;
    
        // Apply rotation of matrix to translation value to achieve a
        // first person perspective
        ApplyMatrixLV( &mtx, &tpos, &tpos );
        
        // Set translation matrix
        TransMatrix( &mtx, &tpos );
        
    // Tracking mode
    } else {
        // Amnogus
    }

    // Set rotation and translation matrix
    gte_SetRotMatrix(&mtx);
    gte_SetTransMatrix( &mtx );


    // --- FLOOR DRAWING ---
    pol4 = (POLY_F4*)db_nextpri;

    for (py = 0; py < 16; py++) {
        for (px = 0; px < 16; px++) {
            // Load first three vertices to GTE
            gte_ldv3(
                &verts[py][px],
                &verts[py][px+1],
                &verts[py+1][px]
            );

            gte_rtpt();

            gte_avsz3();
            gte_stotz(&p);

            if (((p>>2) >= OT_LEN) || ((p>>2) <= 0))
            { continue; }

            setPolyF4(pol4);

            // Set the projected vertices to the primitive
            gte_stsxy0(&pol4->x0);
            gte_stsxy1(&pol4->x1);
            gte_stsxy2(&pol4->x2);

            // Compute the last vertex and set the result
            gte_ldv0(&verts[py+1][px+1]);
            gte_rtps();
            gte_stsxy(&pol4->x3);

            // Test if quad is off-screen, discard if so
            // Clipping is important as it not only prevents primitive
            // overflows (tends to happen on textured polys) but also
            // saves packet buffer space and speeds up rendering.
            if (quad_clip(&screen_clip,
                (DVECTOR*)&pol4->x0, (DVECTOR*)&pol4->x1,
                (DVECTOR*)&pol4->x2, (DVECTOR*)&pol4->x3
            ))
            { continue; }

            if ((px+py)&0x1)
            { setRGB0(pol4, 128, 128, 128); }
            else
            { setRGB0(pol4, 255, 255, 255); }

            addPrim(db[db_active].ot + (p>>2), pol4);
            pol4++;
        }
    }

    // Update nextpri variable
    db_nextpri = (char*)pol4;
    
    // Position the cube
    setVector(&pos,
        300,
        -175,
        300
    );

    // Sort cube
    SortCube(&mtx, &pos, &rot);

    // Make the cube SPEEN
    rot.vx += 8;
    rot.vy += 8;
}