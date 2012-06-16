#ifndef __P_LOCAL__
#define __P_LOCAL__

#include "doomdef.h"
#include "p_mobj.h"
#include "t_bsp.h"
#include "d_player.h"

#define FLOATSPEED		(FRACUNIT*4)
#define MAXHEALTH		100
#define VIEWHEIGHT		(56*FRACUNIT)	//villsa: changed from 41 to 56

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS	128
#define MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT	(FRACBITS+7)
#define MAPBMASK		(MAPBLOCKSIZE-1)
#define MAPBTOFRAC		(MAPBLOCKSHIFT-FRACBITS)


// player radius for movement checking
#define PLAYERRADIUS	19*FRACUNIT

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS		32*FRACUNIT

#define GRAVITY		FRACUNIT
#define MAXMOVE		(16*FRACUNIT)           // [d64] changed from 30 to 16
#define STOPSPEED   0x1000
#define FRICTION    0xd200

#define USERANGE		(64*FRACUNIT)
#define MELEERANGE		(80*FRACUNIT)       // [d64] changed from 64 to 80
#define ATTACKRANGE     (16*64*FRACUNIT)
#define MISSILERANGE	(32*64*FRACUNIT)
#define LASERRANGE		(4096*FRACUNIT)
#define LASERAIMHEIGHT	(40*FRACUNIT)
#define LASERDISTANCE	(30*FRACUNIT)

// follow a player
#define	BASETHRESHOLD	 	90

typedef struct
{
    fixed_t	x;
    fixed_t	y;
    fixed_t	dx;
    fixed_t	dy;
    
} divline_t;

typedef struct
{
    fixed_t	    frac;		// along trace line
    dboolean    isaline;
    union
    {
        mobj_t*	thing;
        line_t*	line;
    }			d;
} intercept_t;

#endif

