#include "ds.h"
#include "d_main.h"
#include "p_local.h"
#include "r_local.h"
#include "m_fixed.h"
#include "tables.h"
#include "g_game.h"

// automap vars

int             amCheating      = 0;        //villsa: no longer static..
dboolean        automapactive   = false;

static dboolean am_ready = true;

//
// AM_Stop
//

void AM_Stop(void)
{
    while(I_DmaBGBusy());

    automapactive = false;
    memset(bg_buffer, 0, 0xC000);
    dmaCopyWords(3, bg_buffer, BG_GFX_SUB, 0xC000);
}

//
// AM_Start
//

void AM_Start(void)
{
    fifomsg_t msg;

    REG_BG3CNT_SUB  = BG_BMP8_256x256;
    REG_BG3PA_SUB   = 256;
    REG_BG3PB_SUB   = 0;
    REG_BG3PC_SUB   = 0;
    REG_BG3PD_SUB   = 256;
    REG_BG3X_SUB    = 0;
    REG_BG3Y_SUB    = 0;

    BG_PALETTE_SUB[0] = RGB8(0, 0, 0);
    BG_PALETTE_SUB[1] = RGB8(0, 0xFF, 0);
    BG_PALETTE_SUB[2] = RGB8(0xA4, 0, 0);
    BG_PALETTE_SUB[3] = RGB8(0x8A, 0x5C, 0x30);
    BG_PALETTE_SUB[4] = RGB8(0x80, 0x80, 0x80);
    BG_PALETTE_SUB[5] = RGB8(0xCC, 0xCC, 0x00);
    BG_PALETTE_SUB[6] = RGB8(0x33, 0x73, 0xB3);
    
    automapactive = true;
    am_ready = true;

    msg.type = FIFO_MSG_AUTOMAP;
    msg.arg[0].arg_i = FIFO_AUTOMAP_BUFFER;
    msg.arg[1].arg_p = (void*)bg_buffer;
    FIFO_SEND_MSG(msg);
}

//
// AM_Responder
//

dboolean AM_Responder(event_t* ev)
{
    int rc = false;

    if(ev->type == ev_btndown)
    {
        /*if(buttons & KEY_A && automapactive)
        {
            am_flags |= AF_CONTROL;

            if(buttons & KEY_L)
            {
                mtof_zoommul = M_ZOOMIN;
                ftom_zoommul = M_ZOOMOUT;
                buttons &= ~KEY_L;
                rc = true;
            }
            else if(buttons & KEY_R)
            {
                mtof_zoommul = M_ZOOMOUT;
                ftom_zoommul = M_ZOOMIN;
                buttons &= ~KEY_R;
                rc = true;
            }

            if(buttons & KEY_RIGHT)
            {
                m_paninc.x = FTOM(F_PANINC);
                buttons &= ~KEY_RIGHT;
                rc = true;
            }

            if(buttons & KEY_LEFT)
            {
                m_paninc.x = -FTOM(F_PANINC);
                buttons &= ~KEY_LEFT;
                rc = true;
            }

            if(buttons & KEY_UP)
            {
                m_paninc.y = FTOM(F_PANINC);
                buttons &= ~KEY_UP;
                rc = true;
            }

            if(buttons & KEY_DOWN)
            {
                m_paninc.y = -FTOM(F_PANINC);
                buttons &= ~KEY_DOWN;
                rc = true;
            }
        }
        else */if(buttons & KEY_SELECT)
        {
            automapactive ^= true;
            buttons &= ~KEY_SELECT;
            if(!automapactive)
                AM_Stop();
        }
    }
    /*else if(ev->type == ev_btnup && automapactive)
    {
        if(ev->data & KEY_L || ev->data & KEY_R)
        {
            ftom_zoommul = FRACUNIT;
            mtof_zoommul = FRACUNIT;
        }

        if(ev->data & KEY_UP || ev->data & KEY_DOWN)
            m_paninc.y = 0;

        if(ev->data & KEY_RIGHT || ev->data & KEY_LEFT)
            m_paninc.x = 0;

        if(ev->data & KEY_A)
            am_flags &= ~AF_CONTROL;
    }*/

    return rc;
}

//
// AM_Ticker()
//
// Updates on gametic - enter follow mode, zoom, or change map location
//

void AM_Ticker(void)
{
}

//
// AM_Drawer
//

void AM_Drawer(void)
{
    if(I_DmaBGBusy())
        return;

    // if ready, ping the arm7 to start drawing
    if(am_ready)
    {
        fifomsg_t msg;

        msg.type = FIFO_MSG_AUTOMAP;
        msg.arg[0].arg_i = FIFO_AUTOMAP_DRAW;
        FIFO_SEND_MSG(msg);
        am_ready = false;
    }
    // is the arm7 done drawing?
    else if(fifoCheckValue32(FIFO_USER_02))
    {
        fifoGetValue32(FIFO_USER_02);
        I_RefreshBG();
        am_ready = true;
    }
}

