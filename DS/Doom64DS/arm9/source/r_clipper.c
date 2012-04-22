#include <math.h>

#include "r_local.h"
#include "tables.h"
#include "m_fixed.h"
#include "z_zone.h"
#include "d_main.h"

typedef struct clipnode_s
{
    struct clipnode_s *prev, *next;
    angle_t start, end;
} clipnode_t;

//
// GhostlyDeath <10/3/11>
//
clipnode_t *freelist    = NULL;
clipnode_t *clipnodes   = NULL;
clipnode_t *cliphead    = NULL;

static clipnode_t * R_Clipnode_NewRange(angle_t start, angle_t end);
static dboolean R_Clipper_IsRangeVisible(angle_t startAngle, angle_t endAngle);
static void R_Clipper_AddClipRange(angle_t start, angle_t end);
static void R_Clipper_RemoveRange(clipnode_t * range);
static void R_Clipnode_Free(clipnode_t *node);

static clipnode_t *R_Clipnode_GetNew(void)
{
    if(freelist)
    {
        clipnode_t *p = freelist;
        freelist = p->next;
        return p;
    }

    return (clipnode_t*)Z_Malloc(sizeof(clipnode_t), PU_STATIC, NULL);
}

static clipnode_t * R_Clipnode_NewRange(angle_t start, angle_t end)
{
    clipnode_t *c = R_Clipnode_GetNew();
    c->start = start;
    c->end = end;
    c->next = c->prev=NULL;
    return c;
}

//
// R_Clipper_SafeCheckRange
//

dboolean R_Clipper_SafeCheckRange(angle_t startAngle, angle_t endAngle)
{
    if(startAngle > endAngle)
        return (R_Clipper_IsRangeVisible(startAngle, ANGLE_MAX) ||
        R_Clipper_IsRangeVisible(0, endAngle));
    
    return R_Clipper_IsRangeVisible(startAngle, endAngle);
}

static dboolean R_Clipper_IsRangeVisible(angle_t startAngle, angle_t endAngle)
{
    clipnode_t *ci;
    ci = cliphead;
    
    if(endAngle == 0 && ci && ci->start == 0)
        return false;
    
    while(ci != NULL && ci->start < endAngle)
    {
        if(startAngle >= ci->start && endAngle <= ci->end)
            return false;

        ci = ci->next;
    }
    
    return true;
}

static void R_Clipnode_Free(clipnode_t *node)
{
    node->next = freelist;
    freelist = node;
}

static void R_Clipper_RemoveRange(clipnode_t *range)
{
    if(range == cliphead)
        cliphead = cliphead->next;
    else
    {
        if(range->prev)
            range->prev->next = range->next;

        if (range->next)
            range->next->prev = range->prev;
    }
    
    R_Clipnode_Free(range);
}

//
// R_Clipper_SafeAddClipRange
//

void R_Clipper_SafeAddClipRange(angle_t startangle, angle_t endangle)
{
    if(startangle > endangle)
    {
        // The range has to added in two parts.
        R_Clipper_AddClipRange(startangle, ANGLE_MAX);
        R_Clipper_AddClipRange(0, endangle);
    }
    else
    {
        // Add the range as usual.
        R_Clipper_AddClipRange(startangle, endangle);
    }
}

static void R_Clipper_AddClipRange(angle_t start, angle_t end)
{
    clipnode_t *node, *temp, *prevNode;
    if(cliphead)
    {
        //check to see if range contains any old ranges
        node = cliphead;
        while(node != NULL && node->start < end)
        {
            if(node->start >= start && node->end <= end)
            {
                temp = node;
                node = node->next;
                R_Clipper_RemoveRange(temp);
            }
            else
            {
                if(node->start <= start && node->end >= end)
                    return;
                else
                    node = node->next;
            }
        }
        //check to see if range overlaps a range (or possibly 2)
        node = cliphead;
        while(node != NULL)
        {
            if(node->start >= start && node->start <= end)
            {
                node->start = start;
                return;
            }
            if(node->end >= start && node->end <= end)
            {
                // check for possible merger
                if(node->next && node->next->start <= end)
                {
                    node->end = node->next->end;
                    R_Clipper_RemoveRange(node->next);
                }
                else
                    node->end = end;
                
                return;
            }

            node = node->next;
        }

        //just add range
        node = cliphead;
        prevNode = NULL;

        temp = R_Clipnode_NewRange(start, end);

        while(node != NULL && node->start < end)
        {
            prevNode = node;
            node = node->next;
        }

        temp->next = node;

        if(node == NULL)
        {
            temp->prev = prevNode;

            if(prevNode)
                prevNode->next = temp;

            if(!cliphead)
                cliphead = temp;
        }
        else
        {
            if(node == cliphead)
            {
                cliphead->prev = temp;
                cliphead = temp;
            }
            else
            {
                temp->prev = prevNode;
                prevNode->next = temp;
                node->prev = temp;
            }
        }
    }
    else
    {
        temp = R_Clipnode_NewRange(start, end);
        cliphead = temp;
        return;
    }
}

//
// R_Clipper_Clear
//

void R_Clipper_Clear(void)
{
    clipnode_t *node = cliphead;
    clipnode_t *temp;
    
    while(node != NULL)
    {
        temp = node;
        node = node->next;
        R_Clipnode_Free(temp);
    }
    
    cliphead = NULL;
}

//
// R_FrustumAngle
//

angle_t R_FrustumAngle(void)
{
    angle_t tilt;
    float range;
    float floatangle;

    tilt = ANG45 - D_abs((int)(viewpitch - ANG90));

    if(tilt > ANG90)
        return ANG270 - ANG90;

    range = (64.0f / (74.0f - 10.0f));

    if(range > 1.0f)
        range = 1.0f;

    floatangle = (float)(tilt / ANG1) * range;

    return ANG270 - ((int)floatangle * ANG1);
}

