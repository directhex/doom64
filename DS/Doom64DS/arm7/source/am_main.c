#include "arm7.h"
#include "doomtype.h"
#include "d_player.h"
#include "m_fixed.h"
#include "p_local.h"
#include "t_bsp.h"
#include "tables.h"

// automap flags

enum
{
    AF_CONTROL  = 1
};

// automap vars

int             amCheating      = 0;        //villsa: no longer static..
dboolean        automapactive   = false;

// scale on entry
#define INITSCALEMTOF   (.2*FRACUNIT)
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC        8
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((int)(1.05f*FRACUNIT))
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((int)(FRACUNIT/1.05f))

// translates between frame-buffer and map distances
#define FTOM(x)         FixedMul(((x)<<16),scale_ftom)
#define MTOF(x)         (FixedMul((x),scale_mtof)>>16)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)       (f_x + MTOF((x)-m_x))
#define CYMTOF(y)       (f_y + (f_h - MTOF((y)-m_y)))

typedef struct
{
    fixed_t x, y;
} mpoint_t;

typedef struct
{
    int x, y;
} fpoint_t;

typedef struct
{
    fpoint_t a, b;
} fline_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

typedef struct
{
    fixed_t slp, islp;
} islope_t;

// location of window on screen
static int  f_x;
static int  f_y;

// size of window on screen
static int  f_w;
static int  f_h;

static mpoint_t m_paninc;       // how far the window pans each tic (map coords)
static fixed_t mtof_zoommul;    // how far the window zooms each tic (map coords)
static fixed_t ftom_zoommul;    // how far the window zooms each tic (fb coords)

static fixed_t m_x, m_y;        // LL x,y window location on the map (map coords)
static fixed_t m_x2, m_y2;      // UR x,y window location on the map (map coords)

// width/height of window on map (map coords)
static fixed_t  m_w;
static fixed_t  m_h;

// based on level size
static fixed_t  min_x;
static fixed_t  min_y;
static fixed_t  max_x;
static fixed_t  max_y;

static fixed_t  max_w;          // max_x-min_x,
static fixed_t  max_h;          // max_y-min_y

// based on player size
static fixed_t  min_w;
static fixed_t  min_h;


static fixed_t  min_scale_mtof; // used to tell when to stop zooming out
static fixed_t  max_scale_mtof; // used to tell when to stop zooming in

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

// old location used by the Follower routine
static mpoint_t f_oldloc;

static player_t *plr = NULL;    // the player represented by an arrow
static dboolean stopped = true;
static byte     am_flags = 0;   // action flags for automap. Mostly for controls
static byte     am_blinktic = 0;

//
// AM_ActivateNewScale()
//
// Changes the map scale after zooming or translating
//

static void AM_ActivateNewScale(void)
{
    m_x += m_w/2;
    m_y += m_h/2;
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    m_x -= m_w/2;
    m_y -= m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

//
// AM_MinOutWindowScale()
//
// Set the window scale to the maximum size
//

static void AM_MinOutWindowScale()
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_ActivateNewScale();
}

//
// AM_MaxOutWindowScale(void)
//
// Set the window scale to the minimum size
//

static void AM_MaxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_ActivateNewScale();
}

//
// AM_rotate()
//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
// Passed the coordinates of a point, and an angle
// Returns the coordinates rotated by the angle
//
// CPhipps - made static & enhanced for automap rotation

static void AM_Rotate(fixed_t* x,  fixed_t* y, angle_t a, fixed_t xorig, fixed_t yorig)
{
    fixed_t tmpx;

    tmpx =
        FixedMul(*x - xorig,finecosine[a>>ANGLETOFINESHIFT])
        - FixedMul(*y - yorig,finesine[a>>ANGLETOFINESHIFT]);

    *y = yorig +
        FixedMul(*x - xorig,finesine[a>>ANGLETOFINESHIFT])
        + FixedMul(*y - yorig,finecosine[a>>ANGLETOFINESHIFT]);

    *x = tmpx + xorig;
}

//
// AM_FindMinMaxBoundaries()
//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//

static void AM_FindMinMaxBoundaries(void)
{
    int i;
    fixed_t a;
    fixed_t b;

    min_x = min_y =  INT_MAX;
    max_x = max_y = -INT_MAX;

    for(i = 0; i < numvertexes; i++)
    {
        if(vertexes[i].x < min_x)
            min_x = vertexes[i].x;
        else if (vertexes[i].x > max_x)
            max_x = vertexes[i].x;

        if(vertexes[i].y < min_y)
            min_y = vertexes[i].y;
        else if (vertexes[i].y > max_y)
            max_y = vertexes[i].y;
    }

    max_w = max_x - min_x;
    max_h = max_y - min_y;

    min_w = 2 * PLAYERRADIUS;
    min_h = 2 * PLAYERRADIUS;

    a = FixedDiv(f_w << FRACBITS, max_w);
    b = FixedDiv(f_h << FRACBITS, max_h);

    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * PLAYERRADIUS);
}

//
// AM_ChangeWindowLoc()
//
// Moves the map window by the global variables m_paninc.x, m_paninc.y
//

static void AM_ChangeWindowLoc(void)
{
    if(m_paninc.x || m_paninc.y)
    {
        f_oldloc.x = INT_MAX;
    }

    m_x += m_paninc.x;
    m_y += m_paninc.y;

    if(m_x + m_w/2 > max_x)
        m_x = max_x - m_w/2;
    else if (m_x + m_w/2 < min_x)
        m_x = min_x - m_w/2;

    if(m_y + m_h/2 > max_y)
        m_y = max_y - m_h/2;
    else if(m_y + m_h/2 < min_y)
        m_y = min_y - m_h/2;

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

//
// AM_changeWindowScale()
//
// Automap zooming
//
// Passed nothing, returns nothing
//
static void AM_ChangeWindowScale(void)
{
    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    if(scale_mtof < min_scale_mtof)
        AM_MinOutWindowScale();
    else if(scale_mtof > max_scale_mtof)
        AM_MaxOutWindowScale();
    else
        AM_ActivateNewScale();
}

//
// AM_DoFollowPlayer()
//
// Turn on follow mode - the map scrolls opposite to player motion
//

static void AM_DoFollowPlayer(int offsetx, int offsety)
{
    if(f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
    {
        m_x = FTOM(MTOF(plr->mo->x + (offsetx >> 7))) - m_w/2;
        m_y = FTOM(MTOF(plr->mo->y + offsety)) - m_h/2;
        m_x2 = m_x + m_w;
        m_y2 = m_y + m_h;
        f_oldloc.x = plr->mo->x;
        f_oldloc.y = plr->mo->y;
    }
}

//
// AM_ClipMline()
//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes. If the speed is needed,
// use a hash algorithm to handle the common cases.
//
// Passed the line's coordinates on map and in the frame buffer performs
// clipping on them in the lines frame coordinates.
// Returns true if any part of line was not clipped
//

static dboolean AM_ClipMline(mline_t* ml, fline_t* fl)
{
    register int outcode1 = 0;
    register int outcode2 = 0;
    register int outside;
    fpoint_t tmp;
    int dx;
    int dy;

    enum
    {
        LEFT    =1,
        RIGHT   =2,
        BOTTOM  =4,
        TOP     =8
    };

#define DOOUTCODE(oc, mx, my)                   \
    (oc) = 0;                                   \
    if((my) < f_y) (oc) |= TOP;                 \
    else if((my) >= f_y + f_h) (oc) |= BOTTOM;  \
    if((mx) < f_x) (oc) |= LEFT;                \
    else if((mx) >= f_x + f_w) (oc) |= RIGHT;


    // do trivial rejects and outcodes
    if(ml->a.y > m_y2)
        outcode1 = TOP;
    else if(ml->a.y < m_y)
        outcode1 = BOTTOM;

    if(ml->b.y > m_y2)
        outcode2 = TOP;
    else if(ml->b.y < m_y)
        outcode2 = BOTTOM;

    if(outcode1 & outcode2)
        return false; // trivially outside

    if(ml->a.x < m_x)
        outcode1 |= LEFT;
    else if(ml->a.x > m_x2)
        outcode1 |= RIGHT;

    if(ml->b.x < m_x)
        outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
        outcode2 |= RIGHT;

    if(outcode1 & outcode2)
        return false; // trivially outside

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);

    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    DOOUTCODE(outcode2, fl->b.x, fl->b.y);

    if(outcode1 & outcode2)
        return false;

    while(outcode1 | outcode2)
    {
        // may be partially inside box
        // find an outside point
        if(outcode1)
            outside = outcode1;
        else
            outside = outcode2;

        // clip to each side
        if(outside & TOP)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + dx*(fl->a.y) / dy;
            tmp.y = f_y;
        }
        else if(outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + dx*(fl->a.y-(f_y+f_h)) / dy;
            tmp.y = f_y + f_h-1;
        }
        else if(outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + dy*(f_x+f_w-1 - fl->a.x) / dx;
            tmp.x = f_x + f_w-1;
        }
        else if(outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + dy*(f_x-fl->a.x) / dx;
            tmp.x = f_x;
        }

        if(outside == outcode1)
        {
            fl->a = tmp;
            DOOUTCODE(outcode1, fl->a.x, fl->a.y);
        }
        else
        {
            fl->b = tmp;
            DOOUTCODE(outcode2, fl->b.x, fl->b.y);
        }

        if(outcode1 & outcode2)
            return false; // trivially outside
    }

    return true;
}
#undef DOOUTCODE

//
// AM_DrawLine
//

static fline_t fl;

static void AM_DrawLine(mline_t* ml, int c)
{
    register int x;
    register int y;
    register int dx;
    register int dy;
    register int sx;
    register int sy;
    register int ax;
    register int ay;
    register int d;

    if(!AM_ClipMline(ml, &fl))
        return;

    dx = fl.b.x - fl.a.x;
    ax = 2 * (dx < 0 ? -dx : dx);
    sx = dx < 0 ? -1 : 1;

    dy = fl.b.y - fl.a.y;
    ay = 2 * (dy < 0 ? -dy : dy);
    sy = dy < 0 ? -1 : 1;

    x = fl.a.x;
    y = fl.a.y;

    if(ax > ay)
    {
        d = (ay - ax) >> 1;

        while(1)
        {
            automapbuffer[(y * 256) + x] = c;
            if(x == fl.b.x)
                return;

            if(d >= 0)
            {
                y += sy;
                d -= ax;
            }

            x += sx;
            d += ay;
        }
    }
    else
    {
        d = (ax - ay) >> 1;

        while(1)
        {
            automapbuffer[(y * 256) + x] = c;
            if(y == fl.b.y)
                return;

            if(d >= 0)
            {
                x += sx;
                d -= ay;
            }

            y += sy;
            d += ax;
        }
    }
}

//
// AM_DrawWalls
//

static mline_t l;

static void AM_DrawWalls(void)
{
    int i;
    line_t* line;
    fixed_t x;
    fixed_t y;
    angle_t angle;

    x = plr->mo->x;
    y = plr->mo->y;
    angle = plr->mo->angle;

    for(i = 0; i < numlines; i++)
    {
        byte c = 0;

        line = &lines[i];

        if(line->flags & ML_DONTDRAW)
            continue;

        l.a.x = line->v1->x;
        l.a.y = line->v1->y;
        l.b.x = line->v2->x;
        l.b.y = line->v2->y;

        AM_Rotate(&l.a.x, &l.a.y, ANG90 - angle, x, y);
        AM_Rotate(&l.b.x, &l.b.y, ANG90 - angle, x, y);

        if(line->flags & ML_MAPPED || plr->powers[pw_allmap] || amCheating)
        {
            c = 3;

            if((plr->powers[pw_allmap] || amCheating) && !(line->flags & ML_MAPPED))
                c = 4;
            else if(line->flags & ML_SECRET)
                c = 2;
            else if(line->special)
                c = 5;
            else if(!(line->flags & ML_TWOSIDED))
                c = 2;

            AM_DrawLine(&l, c);
        }
    }
}

//
// AM_DrawTriangle
//

static void AM_DrawTriangle(fixed_t x, fixed_t y, angle_t angle, int c)
{
    l.a.x = (dcos(angle) << 5) + x;
    l.a.y = (dsin(angle) << 5) + y;
    l.b.x = (dcos(angle + 0xA0000000) << 5) + x;
    l.b.y = (dsin(angle + 0xA0000000) << 5) + y;
    AM_Rotate(&l.a.x, &l.a.y, ANG90 - plr->mo->angle, plr->mo->x, plr->mo->y);
    AM_Rotate(&l.b.x, &l.b.y, ANG90 - plr->mo->angle, plr->mo->x, plr->mo->y);
    AM_DrawLine(&l, c);
    l.a.x = l.b.x;
    l.a.y = l.b.y;
    l.b.x = (dcos(angle + 0x60000000) << 5) + x;
    l.b.y = (dsin(angle + 0x60000000) << 5) + y;
    AM_Rotate(&l.b.x, &l.b.y, ANG90 - plr->mo->angle, plr->mo->x, plr->mo->y);
    AM_DrawLine(&l, c);
    l.a.x = l.b.x;
    l.a.y = l.b.y;
    l.b.x = (dcos(angle) << 5) + x;
    l.b.y = (dsin(angle) << 5) + y;
    AM_Rotate(&l.b.x, &l.b.y, ANG90 - plr->mo->angle, plr->mo->x, plr->mo->y);
    AM_DrawLine(&l, c);
}

//
// AM_DrawThings
//

static void AM_DrawThings(void)
{
    mobj_t* m;
    
    for(m = mobjhead->next; m != mobjhead; m = m->next)
    {
        int c;

        if(m->type == MT_PLAYER || m->flags & (MF_NOSECTOR|MF_RENDERLASER))
            continue;

        if(m->flags & (MF_SHOOTABLE|MF_MISSILE))
            c = 2;
        else
            c = 6;

        AM_DrawTriangle(m->x, m->y, m->angle, c);
    }
}

//
// AM_DrawPlayer
//

static void AM_DrawPlayer(player_t* player)
{
    angle_t angle;
    fixed_t x, y;
    int c;

    angle = player->mo->angle;
    x = player->mo->x;
    y = player->mo->y;
    c = (am_blinktic++ & 3) ? 1 : 0;

    AM_DrawTriangle(x, y, angle, c);
}

//
// AM_Stop
//

void AM_Stop(void)
{
    automapactive = false;
    stopped = true;
}

//
// AM_Start
//

void AM_Start(void)
{
    stopped = false;
    automapactive = false;
    
    plr = player;

    f_oldloc.x = INT_MAX;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    f_x = f_y = 0;
    f_w = 256;
    f_h = 192;

    AM_FindMinMaxBoundaries();

    scale_mtof = 0x1B00;

    if(scale_mtof > max_scale_mtof)
        scale_mtof = min_scale_mtof;

    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
// AM_Ticker()
//
// Updates on gametic - enter follow mode, zoom, or change map location
//

void AM_Ticker(void)
{
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // Change the zoom if necessary
    if(ftom_zoommul != FRACUNIT)
        AM_ChangeWindowScale();

  // Change x,y location
    if(am_flags & AF_CONTROL)
        AM_ChangeWindowLoc();
    else
        AM_DoFollowPlayer(0, 0);
}

//
// AM_Drawer
//

void AM_Drawer(void)
{
    swiWaitForVBlank();

    AM_Ticker();
    AM_DrawWalls();
    AM_DrawPlayer(plr);

#if 0
    if(plr->powers[pw_allmap] || amCheating)
        AM_DrawThings();
#endif

    // 'ping' the arm9 to let it know that can
    // start blitting the buffer to the background
    fifoSendValue32(FIFO_USER_02, 0);
}

