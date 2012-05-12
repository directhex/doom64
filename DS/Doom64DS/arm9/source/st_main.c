#include "doomdef.h"
#include "d_englsh.h"
#include "d_main.h"
#include "p_local.h"
#include "r_local.h"
#include "sounds.h"
#include "s_sound.h"
#include "w_wad.h"
#include "st_main.h"

//
// STATUS BAR DATA
//

typedef struct
{
    dboolean    active;
    dboolean    doDraw;
    int         delay;
    int         times;
} keyflash_t;

static keyflash_t flashCards[NUMCARDS];	/* INFO FOR FLASHING CARDS & SKULLS */

#define	FLASHDELAY      8       /* # of tics delay (1/30 sec) */
#define FLASHTIMES      6       /* # of times to flash new frag amount (EVEN!) */

#define ST_HEALTHTEXTX  29
#define ST_HEALTHTEXTY  203

#define ST_ARMORTEXTX   253
#define ST_ARMORTEXTY   203

#define ST_KEYX         78
#define ST_KEYY         216

#define ST_JMESSAGES    45

#define ST_MSGTIMEOUT   (5*TICRATE)
#define ST_MSGFADESTART (ST_MSGTIMEOUT - (1*TICRATE))
#define ST_MSGFADETIME  5


static rcolor   st_flashcolor;
static byte     st_flashalpha;
static int      st_msgtic = 0;
static int      st_msgalpha = 0xff;
static char*    st_msg = NULL;
static byte*    lump_sfont;
static byte*    lump_status;
static int      lump_symbols;

//
// ST_ClearMessage
//

void ST_ClearMessage(void)
{
    st_msgtic = 0;
    st_msgalpha = 0xff;
    st_msg = NULL;
}

//
// ST_Ticker
//

void ST_Ticker(void)
{
    int ind = 0;
    player_t* plyr;
    
    plyr = &players[consoleplayer];
    
    //
    // keycard stuff
    //
    
    /* */
    /* Tried to open a CARD or SKULL door? */
    /* */
    for(ind = 0; ind < NUMCARDS; ind++)
    {
        /* CHECK FOR INITIALIZATION */
        if(plyr->tryopen[ind])
        {
            plyr->tryopen[ind] = false;
            flashCards[ind].active = true;
            flashCards[ind].delay = FLASHDELAY;
            flashCards[ind].times = FLASHTIMES+1;
            flashCards[ind].doDraw = false;
        }
        
        /* MIGHT AS WELL DO TICKING IN THE SAME LOOP! */
        if(flashCards[ind].active && !--flashCards[ind].delay)
        {
            flashCards[ind].delay = FLASHDELAY;
            flashCards[ind].doDraw ^= 1;
            
            if(!--flashCards[ind].times)
                flashCards[ind].active = false;
            
            if(flashCards[ind].doDraw && flashCards[ind].active)
                S_StartSound(NULL, sfx_itemup);
        }
    }
    
    //
    // messages
    //
    if(plyr->message)
    {
        I_Printf("%s\n", plyr->message);

        ST_ClearMessage();
        st_msg = plyr->message;
        plyr->message = NULL;
    }
    
    if(st_msg || plyr->messagepic >= 0)
    {
        st_msgtic++;
        
        if(st_msgtic >= ST_MSGFADESTART)
            st_msgalpha = MAX((st_msgalpha -= ST_MSGFADETIME), 0);
        
        if(st_msgtic >= ST_MSGTIMEOUT)
        {
            ST_ClearMessage();
            plyr->messagepic = -1;
        }
    }

    //
    // flashes
    //
    if(plyr->cameratarget == plyr->mo)
        ST_UpdateFlash();
}

//
// ST_UpdateFlash
//

#define ST_MAXDMGCOUNT  160
#define ST_MAXSTRCOUNT  32
#define ST_MAXBONCOUNT  100

void ST_UpdateFlash(void)
{
    player_t* p = &players[consoleplayer];

    st_flashcolor = 0;

    // invulnerability flash (white)
    if(p->powers[pw_invulnerability] > 61 || (p->powers[pw_invulnerability] & 8))
    {
        st_flashcolor = RGB15(16, 16, 16);
        st_flashalpha = 16;
    }
    // bfg flash (green)
    else if(p->bfgcount)
    {
        st_flashcolor = RGB15(0, 31, 0);
        st_flashalpha = ((p->bfgcount << 1) >> 3);
    }
    // damage and strength flash (red)
    else if(p->damagecount || (p->powers[pw_strength] > 1))
    {
        int r1 = p->damagecount;
        int r2 = p->powers[pw_strength];

        if(r1)
        {
            if(r1 > ST_MAXDMGCOUNT)
                r1 = ST_MAXDMGCOUNT;
        }

        if(r2 == 1)
            r2 = 0;
        else if(r2 > ST_MAXSTRCOUNT)
            r2 = ST_MAXSTRCOUNT;

        // take priority based on value
        if(r1 > r2)
        {
            st_flashcolor = RGB15(31, 0, 0);
            st_flashalpha = ((r1 << 1) >> 3);
        }
        else
        {
            st_flashcolor = RGB15(31, 0, 0);
            st_flashalpha = ((r2 << 1) >> 3);
        }
    }
    // suit flash (green/yellow)
    else if(p->powers[pw_ironfeet] > 61 || (p->powers[pw_ironfeet] & 8))
    {
        st_flashcolor = RGB15(0, 31, 4);
        st_flashalpha = 26;
    }
    // bonus flash (yellow)
    else if(p->bonuscount)
    {
        int c1 = p->bonuscount + 8;

        if(c1 > ST_MAXBONCOUNT)
            c1 = ST_MAXBONCOUNT;

        st_flashcolor = RGB15(31, 31, 15);
        st_flashalpha = ((c1 + (c1 >> 1)) >> 3);
    }
}

//
// ST_Drawer
//

void ST_Drawer(void)
{
    I_CheckGFX();

    GFX_ORTHO(0);

    if(st_flashcolor && st_flashalpha)
    {
        GFX_POLY_FORMAT =
            POLY_ALPHA(st_flashalpha)   |
            POLY_ID(1)                  |
            POLY_CULL_NONE              |
            POLY_MODULATION             |
            POLY_NEW_DEPTH;

        GFX_TEX_FORMAT  = 0;
        GFX_PAL_FORMAT  = 0;
        GFX_COLOR       = st_flashcolor;
        GFX_SCREENRECT();

        GFX_POLY_FORMAT =
            POLY_ALPHA(st_flashalpha)   |
            POLY_ID(1)                  |
            POLY_CULL_NONE              |
            POLY_MODULATION             |
            POLY_NEW_DEPTH              |
            POLY_DEPTHTEST_EQUAL;

        GFX_TEX_FORMAT  = 0;
        GFX_PAL_FORMAT  = 0;
        GFX_COLOR       = st_flashcolor;
        GFX_SCREENRECT();
    }
}

//
// ST_Init
//

void ST_Init(void)
{
    int i = 0;

    lump_sfont  = (byte*)W_CacheLumpName("SFONT", PU_STATIC);
    lump_status = (byte*)W_CacheLumpName("STATUS", PU_STATIC);

    lump_symbols = W_GetNumForName("SYMBOLS");
    
    // setup keycards
    
    for (i = 0; i < NUMCARDS; i++)
    {
        flashCards[i].active = false;
        players[consoleplayer].tryopen[i] = false;
    }
    
    // setup hud messages
    
    ST_ClearMessage();
}

