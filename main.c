#include "main.h"       // Library with self-made drawing functions and such
#include "controller.h"

    SPRITE       spr;
    PADTYPE     *pad;

    SVECTOR player_tri[] =
    {
        {   0, -20, 0 },
        {  10,  20, 0 },
        { -10,  20, 0 }
    };

    int pos_x, pos_y,
        vel_x, vel_y,
        angle;

void Draw(void)
{
    SortRotSprite(pos_x>>12, pos_y>>12, 128, 128, angle, ONE, &spr);
    //SortSprite(pos_x, pos_y, &spr);
    
    FntPrint("ANGLE = %d\n", angle );
    FntPrint("POS_X = %d (%d.%d)\n", pos_x, (pos_x>>12), (pos_x&0xfff) );
    FntPrint("POS_Y = %d (%d.%d)\n", pos_y, (pos_y>>12), (pos_y&0xfff) );
    FntPrint("VEL_X = %d (%d.%d)\n", vel_x, (vel_x>>12), (vel_x&0xfff) );
    FntPrint("VEL_Y = %d (%d.%d)\n", vel_y, (vel_y>>12), (vel_y&0xfff) );
}

void Update(void)
{
    pad = PoolPads();

    if (IsConnected(pad))
    {
        if ((pad->type == PTYPE_DIGPAD) ||
            (pad->type == PTYPE_ANALOG) ||
            (pad->type == PTYPE_DUALSH) )
        {
            if (IsPressed(pad, PAD_UP))
            {
                vel_x += csin( angle )>>3;
                vel_y -= ccos( angle )>>3;
            }
            if (IsPressed(pad, PAD_DOWN))
            {
                vel_x -= csin( angle )>>3;
                vel_y += ccos( angle )>>3;
            }
            if (IsPressed(pad, PAD_LEFT))
            { angle -= 16; }
            if (IsPressed(pad, PAD_RIGHT))
            { angle += 16; }

            if (IsPressed(pad, PAD_START))
            {
                vel_x = 0;
                vel_y = 0;
                pos_x = ONE*(disp[0].disp.w>>1);
                pos_y = ONE*(disp[0].disp.h>>1);
                angle = 0;
            }
        }
    }

    pos_x += vel_x;
    pos_y += vel_y;
    
    // wrap player coordinates from going off-screen
    if( (pos_x>>12) < 0 )
    { pos_x += (320<<12); }
    if( (pos_x>>12) > 320 )
    { pos_x -= (320<<12); }
    if( (pos_y>>12) < 0 )
    { pos_y += (256<<12); }
    if( (pos_y>>12) > 256 )
    { pos_y -= (256<<12); }

    // equivalent to multiplying each axis by 0.9765625
    //vel_x = (vel_x*4000)>>12;
    //vel_y = (vel_y*4000)>>12;
    vel_x *= 0.9765625;
    vel_y *= 0.9765625;
}


// ==========================================
//      MAIN GAME CODE
// ==========================================
int main()
{
    printf("FORTNITE BATTLE PASS");

    // Initialize graphics and such
    Init();
    InitPads();
    GetSprite(&img, &spr);

    pos_x = ONE*(disp[0].disp.w>>1);
    pos_y = ONE*(disp[0].disp.h>>1);
    vel_x = 0;
    vel_y = 0,
    angle = 0;

    // Main loop
    while(1)
    {
        Update();
        Draw();
        Display();
    }

    return 0;
}