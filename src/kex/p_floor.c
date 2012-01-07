// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Author$
// $Revision$
// $Date$
//
//
// DESCRIPTION:
//	Floor animation: raising stairs.
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif


#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "r_texture.h"
#include "s_sound.h"
#include "doomstat.h"
#include "sounds.h"
#include "m_math.h"


//
// FLOORS
//

//
// T_MovePlane
// Move a plane (floor or ceiling) and check for crushing
//

result_e T_MovePlane(sector_t* sector, fixed_t speed, fixed_t dest,
                     dboolean crush, int floorOrCeiling, int direction)
{
    dboolean flag;
    fixed_t lastpos;
    plane_t* plane;
    
    switch(floorOrCeiling)
    {
    case 0:

        // FLOOR

        plane = &sector->floorplane;
        lastpos = plane->d;

        switch(direction)
        {
        case -1:

            // DOWN

            M_MovePlane(plane, -speed);

            if(plane->d >= -dest)
            {
                plane->d = -dest;
                flag = P_ChangeSector(sector,crush);
                if(flag == true)
                {
                    plane->d = lastpos;
                    P_ChangeSector(sector,crush);
                }
                else
                {
                    sector->floorheight += M_DiffPlaneHeight(plane, lastpos);
                }

                return pastdest;
            }
            else
            {
                flag = P_ChangeSector(sector,crush);
                if(flag == true)
                {
                    plane->d = lastpos;
                    P_ChangeSector(sector,crush);
                    return stop;
                }
                else
                {
                    sector->floorheight += M_DiffPlaneHeight(plane, lastpos);
                }
            }
            break;
            
        case 1:

            // UP

            M_MovePlane(plane, speed);
            if(plane->d <= -dest)
            {
                plane->d = -dest;
                flag = P_ChangeSector(sector,crush);
                if(flag == true)
                {
                    plane->d = lastpos;
                    P_ChangeSector(sector,crush);
                }
                else
                {
                    sector->floorheight += M_DiffPlaneHeight(plane, lastpos);
                }

                return pastdest;
            }
            else
            {
                // COULD GET CRUSHED

                flag = P_ChangeSector(sector,crush);
                if(flag == true)
                {
                    if(crush == true)
                    {
                        sector->floorheight += M_DiffPlaneHeight(plane, lastpos);
                        return crushed;
                    }

                    plane->d = lastpos;
                    P_ChangeSector(sector,crush);
                    return stop;
                }

                sector->floorheight += M_DiffPlaneHeight(plane, lastpos);
            }
            break;
        }
        break;
        
        case 1:

            // CEILING

            plane = &sector->ceilingplane;
            lastpos = plane->d;

            switch(direction)
            {
            case -1:

                // DOWN

                M_MovePlane(plane, -speed);
                if(plane->d <= dest)
                {
                    plane->d = dest;
                    flag = P_ChangeSector(sector,crush);

                    if(flag == true)
                    {
                        plane->d = lastpos;
                        P_ChangeSector(sector,crush);
                    }
                    else
                    {
                        sector->ceilingheight += M_DiffPlaneHeight(plane, lastpos);
                    }

                    return pastdest;
                }
                else
                {

                    // COULD GET CRUSHED

                    flag = P_ChangeSector(sector,crush);
                    
                    if(flag == true)
                    {
                        if(crush == true)
                        {
                            sector->ceilingheight += M_DiffPlaneHeight(plane, lastpos);
                            return crushed;
                        }

                        plane->d = lastpos;
                        P_ChangeSector(sector,crush);
                        return crushed;
                    }

                    sector->ceilingheight += M_DiffPlaneHeight(plane, lastpos);
                }
                break;
                
            case 1:

                // UP

                M_MovePlane(plane, speed);
                if(plane->d >= dest)
                {
                    plane->d = dest;
                    flag = P_ChangeSector(sector,crush);
                    if(flag == true)
                    {
                        plane->d = lastpos;
                        P_ChangeSector(sector,crush);
                    }
                    else
                        sector->ceilingheight += M_DiffPlaneHeight(plane, lastpos);

                    return pastdest;
                }
                else
                {
                    sector->ceilingheight += M_DiffPlaneHeight(plane, lastpos);
                }
                break;
            }
            break;
    }
    return ok;
}


//
// T_MoveFloor
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//

void T_MoveFloor(floormove_t* floor)
{
	result_e	res;

	res = T_MovePlane(floor->sector,
		      floor->speed,
		      floor->floordestheight,
		      floor->crush,0,floor->direction);

	if(!floor->instant)
	{
		if(!(leveltime & 3))
			S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_secmove);
	}

	if(res == pastdest)
	{
		floor->sector->specialdata = NULL;
		if(floor->direction == -1)
		{
			if(floor->type == lowerAndChange)
			{
				floor->sector->special = floor->newspecial;
				floor->sector->floorpic = floor->texture;
			}
		}
		P_RemoveThinker(&floor->thinker);
	}

}

//
// HANDLE FLOOR TYPES
//
int EV_DoFloor(line_t* line, floor_e floortype, fixed_t speed)
{
	int				secnum;
	int				rtn;
	int				i;
	sector_t*		sec;
	floormove_t*	floor;

	secnum = -1;
	rtn = 0;
	while((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

		// ALREADY MOVING?  IF SO, KEEP GOING...
		if(sec->specialdata)
			continue;

		// new floor thinker
		rtn = 1;
		floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
		P_AddThinker (&floor->thinker);
		sec->specialdata = floor;
		// Midway assumed that ceiling->instant is true only if the
		// speed is equal to 2048*FRACUNIT, which doesn't seem very sufficient
		floor->instant = (speed >= (FLOORSPEED * 1024));
		floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
		floor->type = floortype;
		floor->crush = false;
		floor->sector = sec;

		switch(floortype)
		{
			case lowerFloor:
				floor->direction = -1;
				floor->speed = speed;
				floor->floordestheight =
					P_FindHighestFloorSurrounding(sec);
				break;

			case lowerFloorToLowest:
				floor->direction = -1;
				floor->speed = speed;
				floor->floordestheight =
					P_FindLowestFloorSurrounding(sec);
				break;

			case turboLower:
				floor->direction = -1;
				floor->speed = speed;
				floor->floordestheight =
					P_FindHighestFloorSurrounding(sec);
				if(floor->floordestheight != sec->floorheight)
					floor->floordestheight += 8*FRACUNIT;
				break;

			case raiseFloorCrush:
				floor->crush = true;
			case raiseFloor:
				floor->direction = 1;
				floor->speed = speed;
				floor->floordestheight =
					P_FindLowestCeilingSurrounding(sec);
				if(floor->floordestheight > sec->ceilingheight)
					floor->floordestheight = sec->ceilingheight;
				floor->floordestheight -= (8*FRACUNIT) * (floortype == raiseFloorCrush);
				break;

			case raiseFloorToNearest:
				floor->direction = 1;
				floor->speed = speed;
				floor->floordestheight =
					P_FindNextHighestFloor(sec,sec->floorheight);
				break;

			case raiseFloor24:
				floor->direction = 1;
				floor->speed = speed;
				floor->floordestheight = floor->sector->floorheight +
					24 * FRACUNIT;
				break;

			case raiseFloor24AndChange:
				floor->direction = 1;
				floor->speed = speed;
				floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
				sec->floorpic = line->frontsector->floorpic;
				sec->special = line->frontsector->special;
				break;

			case customFloor:
				floor->direction = (globalint > 0) ? 1 : -1;
				floor->speed = speed;
				floor->floordestheight = floor->sector->floorheight + (globalint * FRACUNIT);
				break;

			case customFloorToHeight:
				floor->direction = (globalint > (floor->sector->floorheight/FRACUNIT)) ? 1 : -1;
				floor->speed = speed;
				floor->floordestheight = (globalint * FRACUNIT);
				break;

			case lowerAndChange:
				floor->direction = -1;
				floor->speed = speed;
				floor->floordestheight =
				P_FindLowestFloorSurrounding(sec);
				floor->texture = sec->floorpic;

				for(i = 0; i < sec->linecount; i++)
				{
					if(twoSided(secnum, i))
					{
						if(getSide(secnum,i,0)->sector-sectors == secnum)
						{
							sec = getSector(secnum,i,1);

							if(sec->floorheight == floor->floordestheight)
							{
								floor->texture = sec->floorpic;
								floor->newspecial = sec->special;
								break;
							}
						}
						else
						{
							sec = getSector(secnum,i,0);

							if(sec->floorheight == floor->floordestheight)
							{
								floor->texture = sec->floorpic;
								floor->newspecial = sec->special;
								break;
							}
						}
					}
				}
			default:
				break;
		}
	}
	return rtn;
}

//
// BUILD A STAIRCASE!
//

int EV_BuildStairs(line_t* line, stair_e type)
{
    int             secnum;
    int             height;
    int             i;
    int             newsecnum;
    int             texture;
    int             ok;
    int             rtn;
    sector_t*       sec;
    sector_t*       tsec;
    floormove_t*    floor;
    fixed_t         stairsize;
    fixed_t         speed;
    
    secnum = -1;
    rtn = 0;
    while((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];
        
        // ALREADY MOVING?  IF SO, KEEP GOING...
        if(sec->specialdata)
            continue;
        
        rtn = 1;

        // new floor thinker
        floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker (&floor->thinker);

        sec->specialdata = floor;

        floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
        floor->direction = 1;
        floor->sector = sec;
        floor->instant = false;

        switch(type)
        {
        case build8:
            speed = FLOORSPEED / 2;     // [d64] changed from 4 to 2
            stairsize = 8 * FRACUNIT;
            break;
        case turbo16:
            speed = FLOORSPEED * 2;     // [d64] changed from 4 to 2
            stairsize = 16*FRACUNIT;
            break;
        }

        floor->speed = speed;
        height = sec->floorheight + stairsize;
        floor->floordestheight = height;
        
        texture = sec->floorpic;
        
        // Find next sector to raise
        // 1.   Find 2-sided line with same sector side[0]
        // 2.   Other side is the next sector to raise
        do
        {
            ok = 0;
            for(i = 0; i < sec->linecount; i++)
            {
                if( !((sec->lines[i])->flags & ML_TWOSIDED))
                    continue;
                
                tsec = (sec->lines[i])->frontsector;
                newsecnum = tsec-sectors;
                
                if(secnum != newsecnum)
                    continue;
                
                tsec = (sec->lines[i])->backsector;
                newsecnum = tsec - sectors;
                
                if(tsec->floorpic != texture)
                    continue;
                
                height += stairsize;
                
                if(tsec->specialdata)
                    continue;
                
                sec = tsec;
                secnum = newsecnum;

                floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
                P_AddThinker(&floor->thinker);
                
                sec->specialdata = floor;

                floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = speed;
                floor->floordestheight = height;
                floor->instant = false;

                ok = 1;

                break;
            }

        } while(ok);

        secnum = -1;
    }
    return rtn;
 }

//
// T_MoveSplitPlane
//

void T_MoveSplitPlane(splitmove_t *split)
{
    sector_t *sector	= split->sector;
    fixed_t lastceilpos = 0;
    fixed_t lastflrpos	= 0;
    dboolean cdone		= false;
    dboolean fdone		= false;
    plane_t* fp;
    plane_t* cp;
    
    fp = &sector->floorplane;
    cp = &sector->ceilingplane;
    
    //
    // DOWN
    //
    if(split->ceildir == -1)
    {
        lastceilpos = cp->d;
        M_MovePlane(cp, -(2*FRACUNIT));
        
        if(cp->d <= split->ceildest)
        {
            cp->d = split->ceildest;
            cdone = true;
        }
        else
            sector->ceilingheight += M_DiffPlaneHeight(cp, lastceilpos);
    }
    //
    // UP
    //
    else
    {
        lastceilpos = cp->d;
        M_MovePlane(cp, (2*FRACUNIT));
        
        if(cp->d >= split->ceildest)
        {
            cp->d = split->ceildest;
            cdone = true;
        }
        else
            sector->ceilingheight += M_DiffPlaneHeight(cp, lastceilpos);
    }
    
    //
    // DOWN
    //
    if(split->flrdir == -1)
    {
        lastflrpos = fp->d;
        M_MovePlane(fp, -(2*FRACUNIT));
        
        if(fp->d >= -split->flrdest)
        {
            fp->d = -split->flrdest;
            fdone = true;
        }
        else
            sector->floorheight += M_DiffPlaneHeight(fp, lastflrpos);
    }
    //
    // UP
    //
    else
    {
        lastflrpos = fp->d;
        M_MovePlane(fp, (2*FRACUNIT));
        
        if(fp->d <= -split->flrdest)
        {
            fp->d = -split->flrdest;
            fdone = true;
        }
        else
            sector->floorheight += M_DiffPlaneHeight(fp, lastflrpos);
    }
    
    if(!P_ChangeSector(sector, false))
    {
        if(!cdone || !fdone)
            return;
        
        P_RemoveThinker(&split->thinker);
        sector->specialdata = NULL;
        return;
    }
    else
    {
        cp->d = lastceilpos;
        fp->d = lastflrpos;
        P_ChangeSector(sector, false);
    }
}

//
// EV_SplitSector
//

int EV_SplitSector(line_t *line, dboolean sync)
{
	int				secnum;
	int				rtn;
	sector_t*		sec;
	splitmove_t*	split;

	secnum = -1;
	rtn = 0;

	while((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

		// ALREADY MOVING?  IF SO, KEEP GOING...
		if(sec->specialdata)
			continue;

		rtn = 1;
		split = Z_Malloc (sizeof(*split), PU_LEVSPEC, 0);
		P_AddThinker(&split->thinker);
		sec->specialdata = split;

		split->thinker.function.acp1 = (actionf_p1)T_MoveSplitPlane;
		split->sector = sec;
		split->ceildest = sec->ceilingheight + (globalint * FRACUNIT);

		if(sync)
		{
			split->flrdest = sec->floorheight + (globalint * FRACUNIT);
			split->ceildir = (globalint >= 0) ? 1 : -1;
			split->flrdir = (globalint >= 0) ? 1 : -1;
		}
		else
		{
			split->flrdest = sec->floorheight - (globalint * FRACUNIT);
			split->ceildir = (globalint >= 0) ? 1 : -1;
			split->flrdir = (globalint >= 0) ? -1 : 1;
		}
	}

	return rtn;
}