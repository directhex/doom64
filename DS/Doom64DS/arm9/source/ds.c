#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "ds.h"
#include "d_main.h"
#include "z_zone.h"
#include "r_local.h"

static char msg[256];

//
// I_Error
//

void I_Error(const char *s, ...)
{
    va_list v;

    va_start(v, s);
    vsprintf(msg, s, v);
    va_end(v);

    consoleDemoInit();
    iprintf(msg);

    while (1) { swiWaitForVBlank(); }
}

//
// I_Printf
//

void I_Printf(const char *s, ...)
{
    va_list v;

    va_start(v, s);
    vsprintf(msg, s, v);
    va_end(v);

    iprintf(msg);
}

//
// I_PrintWait
//

void I_PrintWait(const char *s, ...)
{
    va_list v;
    int keys = 0;

    va_start(v, s);
    vsprintf(msg, s, v);
    va_end(v);

    iprintf(msg);

    while(!(keys & KEY_START))
    {
        scanKeys();
        keys = keysDown();
        swiWaitForVBlank();
    }
}

//
// I_FilePath
//

const char* I_FilePath(const char* file)
{
    static char fname[256];

    sprintf(fname, "fat0:/%s", file);
    return fname;
}

//
// I_ReadFile
//

int I_ReadFile(char const* name, byte** buffer)
{
    FILE *fp;

    errno = 0;
    
    if((fp = fopen(I_FilePath(name), "rb")))
    {
        size_t length;

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = Z_Malloc(length, PU_STATIC, 0);
      
        if(fread(*buffer, 1, length, fp) == length)
        {
            fclose(fp);
            return length;
        }
        
        fclose(fp);
   }
   
   return -1;
}

//
// I_FileLength
//

long I_FileLength(FILE *handle)
{ 
    long savedpos;
    long length;

    // save the current position in the file
    savedpos = ftell(handle);
    
    // jump to the end and find the length
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);

    // go back to the old location
    fseek(handle, savedpos, SEEK_SET);

    return length;
}

//
// I_FileExists
//

int I_FileExists(char *filename)
{
    FILE *fstream;

    fstream = fopen(I_FilePath(filename), "r");

    if (fstream != NULL)
    {
        fclose(fstream);
        return 1;
    }
    else
    {
        // If we can't open because the file is a directory, the 
        // "file" exists at least!

        if(errno == 21)
            return 2;
    }

    return 0;
}

//
// I_Init
//

void I_Init(void)
{
    fatInitDefault();

    REG_POWERCNT    = POWER_3D_CORE | POWER_MATRIX | POWER_LCD | POWER_2D_B | POWER_SWAP_LCDS;
    REG_DISPCNT     = MODE_0_3D;
    REG_DISPCNT_SUB = MODE_5_2D | DISPLAY_BG3_ACTIVE;
    TIMER0_CR       = TIMER_ENABLE | TIMER_DIV_1024;
    TIMER1_CR       = TIMER_ENABLE | TIMER_CASCADE;

    vramSetBankA(VRAM_A_LCD);
    vramSetBankB(VRAM_B_LCD);
    vramSetBankC(VRAM_C_LCD);
    vramSetBankD(VRAM_D_LCD);
    vramSetBankE(VRAM_E_LCD);
    vramSetBankF(VRAM_F_LCD);
    vramSetBankG(VRAM_G_LCD);
    vramSetBankH(VRAM_H_SUB_BG);
    vramSetBankI(VRAM_I_SUB_BG_0x06208000);

    while(GFX_BUSY);

    //
    // make sure there are no push/pops that haven't executed yet
    //
    while(GFX_STATUS & GFX_MTX_BUSY)
        GFX_STATUS |= GFX_MTX_STACK_RESET;  // clear push/pop errors or push/pop busy bit never clears

    //
    // pop the projection stack to the top; poping 0 off an empty stack causes an error... weird?
    //
    if((GFX_STATUS & GFX_MTX_PROJ_STACK) != 0)
    {
        MATRIX_CONTROL  = GL_PROJECTION;
        MATRIX_POP      = 1;
    }

    //
    // 31 deep modelview matrix; 32nd entry works but sets error flag
    //
    MATRIX_CONTROL  = GL_MODELVIEW;
    MATRIX_POP      = GFX_MTX_STACK_LEVEL;

    consoleInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x256, 11, 8, false, true);
    consoleClear();
}

//
// I_CheckGFX
//

dboolean (I_CheckGFX)(char* file, int line)
{
    if(GFX_VERTEX_RAM_USAGE >= 4096)
    {
#ifdef CHECKGFX_ABORT
        I_Error("I_CheckGFX: vertex overflowed by %d\n(%s : %i)",
        GFX_VERTEX_RAM_USAGE - 4096, file, line);
#else
        return false;
#endif
    }

    if(GFX_POLYGON_RAM_USAGE >= 2048)
    {
#ifdef CHECKGFX_ABORT
        I_Error("I_CheckGFX: polygon overflowed by %d\n(%s : %i)",
        GFX_POLYGON_RAM_USAGE - 2048, file, line);
#else
        return false;
#endif
    }

    return true;
}

//
// I_ClearFrame
//

void I_ClearFrame(void)
{
    int i;

    GFX_CONTROL         = GL_FOG | GL_BLEND | GL_TEXTURE_2D | GL_ALPHA_TEST;
    GFX_ALPHA_TEST      = 0;
    GFX_CUTOFF_DEPTH    = GL_MAX_DEPTH;
    GFX_CLEAR_COLOR     = 0x1F0000;
    GFX_CLEAR_DEPTH     = GL_MAX_DEPTH;
    GFX_VIEWPORT        = 0xBFFF0000;
    GFX_TEX_FORMAT      = 0;
    GFX_PAL_FORMAT      = 0;
    GFX_POLY_FORMAT     = 0;

    //
    // load identity to all the matrices
    //
    MATRIX_CONTROL      = GL_PROJECTION;
    MATRIX_IDENTITY     = 0;
    MATRIX_CONTROL      = GL_MODELVIEW;
    MATRIX_IDENTITY     = 0;
    MATRIX_CONTROL      = GL_TEXTURE;
    MATRIX_IDENTITY     = 0;
    MATRIX_CONTROL      = GL_POSITION;
    MATRIX_IDENTITY     = 0;

    for(i = 0; i < 32; i++)
        GFX_FOG_TABLE[i] = (i * 4);

    GFX_FOG_TABLE[31] = 0x7F;

    GFX_CONTROL = (GFX_CONTROL & 0xF0FF) | 0x700;

    if(sky)
    {
        GFX_FOG_COLOR = sky->fogcolor;
        GFX_FOG_OFFSET = GL_MAX_DEPTH - ((1000 - sky->fognear) * 10);
    }
    else
    {
        GFX_FOG_COLOR = 0;
        GFX_FOG_OFFSET = GL_MAX_DEPTH - 192;
    }
}

//
// I_AllocVBlock
//

dboolean I_AllocVBlock(uint32* user, vramblock_t** vblock, byte* data, int index, int size,
                       int flags, int texel_w, int texel_h, int type)
{
    if(user[index] == 0)
    {
        if(!(vblock[index] = Z_VAlloc(vramzone, size, PU_CACHE, &user[index])))
            return false;

        swiCopy(data, vblock[index]->block, (size >> 2) | COPY_MODE_WORD);
    }
    else
        Z_VTouch(vramzone, vblock[index]);

    user[index] = GFX_TEXTURE(flags, texel_w, texel_h, type,
        ((uint32*)gfx_base + ((vblock[index]->block - gfx_tex_buffer) >> 2)));

    return true;
}

//
// I_SetPalette
//

uint32 I_SetPalette(uint16* data, int offset, int size)
{
    swiCopy(data, VRAM_E + (offset >> 1), (size >> 1) | COPY_MODE_HWORD);
    return GFX_VRAM_OFFSET((VRAM_E + (offset >> 2)));
}

//
// I_FinishFrame
//

void I_FinishFrame(void)
{
    int i;
    byte* buff;
    byte* vram;
    
    I_CheckGFX();
    
    // wait for vblank before flushing cache
    swiWaitForVBlank();
    DC_FlushAll();

    buff = (byte*)gfx_tex_buffer;
    vram = (byte*)VRAM_A;

    // lock banks
    vramSetBankA(VRAM_A_LCD);
    vramSetBankB(VRAM_B_LCD);
    vramSetBankC(VRAM_C_LCD);
    vramSetBankD(VRAM_D_LCD);

    for(i = 0; i < 32; i++)
    {
        // transfer 16kb of data at a time
        dmaCopyWords(0, (uint32*)buff, (uint32*)vram, 0x4000);

        // force scanline back at 192
        REG_VCOUNT = 192;

        buff += 0x4000;
        vram += 0x4000;
    }

    // unlock banks
    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankB(VRAM_B_TEXTURE);
    vramSetBankC(VRAM_C_TEXTURE);
    vramSetBankD(VRAM_D_TEXTURE);

    frametic++;

    GFX_FLUSH = 1;
}

//
// I_StartTic
//

void I_StartTic(void)
{
    u16 keys;
    event_t ev;
    
    scanKeys();

    if((keys = keysDown()))
    {
        ev.type = ev_btndown;
        ev.data = keys;
        D_PostEvent(&ev);
    }

    if((keys = keysUp()))
    {
        ev.type = ev_btnup;
        ev.data = keys;
        D_PostEvent(&ev);
    }
}

//
// I_GetTimeTicks
//

int I_GetTimeTicks(void)
{
    return (TIMER0_DATA >> 5) + (TIMER1_DATA << 11);
}

//
// I_GetTimeMS
//

static int basetime = 0;

int I_GetTime(void)
{
    uint32 ticks;

    ticks = I_GetTimeTicks();

    if(basetime == 0)
        basetime = ticks;

    ticks -= basetime;

    return (ticks * TICRATE) / 1000;
}

//
// I_GetTimeMS
//
// Same as I_GetTime, but returns time in milliseconds
//

int I_GetTimeMS(void)
{
    uint32 ticks;
    
    ticks = I_GetTimeTicks();
    
    if (basetime == 0)
        basetime = ticks;
    
    return ticks - basetime;
}

//
// I_Sleep
//

void I_Sleep(uint32 ms)
{
    uint32 now;

    now = I_GetTimeTicks();
    while(I_GetTimeTicks() < now + ms);
}

//
// main
//

int main(void)
{
    defaultExceptionHandler();
    D_DoomMain();
    return 0;
}

