#ifndef CLIP_H
#define CLIP_H

#include <sys/types.h>
#include <psxgte.h>
#include <psxgpu.h>

#define CLIP_LEFT	1
#define CLIP_RIGHT	2
#define CLIP_TOP	4
#define CLIP_BOTTOM	8

int test_clip(RECT *clip, short x, short y)
{
	// Tests which corners of the screen a point lies outside the screen boundaries

    int result = 0;

    if (x < clip->x)
    { result |= CLIP_LEFT; }
    if (x >= (clip->x+(clip->w-1)))
    { result |= CLIP_RIGHT; }
    
    if (y < clip->y)
    { result |= CLIP_TOP; }
    if (y >= (clip->y+(clip->h-1)))
    { result |= CLIP_BOTTOM; }

    return result;
}

/* tri_clip
 *
 * Returns non-zero if a triangle (v0, v1, v2) is outside 'clip'.
 *
 * clip			- Clipping area
 * v0,v1,v2		- Triangle coordinates
 *
 */

int tri_clip(RECT *clip, DVECTOR *v0, DVECTOR *v1, DVECTOR *v2)
{
	// Returns non-zero if a triangle is outside the screen boundaries

    short c[3];

    c[0] = test_clip(clip, v0->vx, v0->vy);
    c[1] = test_clip(clip, v1->vx, v1->vy);
    c[2] = test_clip(clip, v2->vx, v2->vy);

    if ((c[0] & c[1]) == 0)
    { return 0; }
    if ((c[1] & c[2]) == 0)
    { return 0; }
    if ((c[2] & c[0]) == 0)
    { return 0; }

    return 0;
}

/* quad_clip
 *
 * Returns non-zero if a quad (v0, v1, v2, v3) is outside 'clip'.
 *
 * clip			- Clipping area
 * v0,v1,v2,v3	- Quad coordinates
 *
 */
int quad_clip(RECT *clip, DVECTOR *v0, DVECTOR *v1, DVECTOR *v2, DVECTOR *v3)
{
    // Returns non-zero if a quad is outside the screen boundaries

    short c[4];

    c[0] = test_clip(clip, v0->vx, v0->vy);
    c[1] = test_clip(clip, v1->vx, v1->vy);
    c[2] = test_clip(clip, v2->vx, v2->vy);
    c[3] = test_clip(clip, v3->vx, v3->vy);

    if ((c[0] & c[1]) == 0)
    { return 0; }
    if ((c[1] & c[2]) == 0)
    { return 0; }
    if ((c[2] & c[3]) == 0)
    { return 0; }

    if ((c[3] & c[0]) == 0)
    { return 0; }
    if ((c[0] & c[2]) == 0)
    { return 0; }
    if ((c[1] & c[3]) == 0)
    { return 0; }

    return 1;
}

#endif // !CLIP_H