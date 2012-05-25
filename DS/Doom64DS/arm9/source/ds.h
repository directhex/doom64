#ifndef __DS_MAIN__
#define __DS_MAIN__

#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include "doomtype.h"
#include "doomdef.h"
#include "z_zone.h"

void I_Init(void);
void I_Error(const char *s, ...);
void I_Printf(const char *s, ...);
void I_PrintWait(const char *s, ...);
void I_ClearFrame(void);
void I_StartTic(void);
int I_GetTime(void);
int I_GetTimeMS(void);
void I_Sleep(uint32 ms);
int I_GetTimeTicks(void);
void I_FinishFrame(void);

typedef struct
{
    uint32 params;
    vramblock_t* vram;
} gfx_t;

//
// disable for debug/testing only
//
#define CHECKGFX_ABORT

dboolean (I_CheckGFX)(char* file, int line);

#ifdef CHECKGFX_ABORT
#define I_CheckGFX() (I_CheckGFX) (__FILE__, __LINE__)
#else
#define I_CheckGFX() if(!(I_CheckGFX)(__FILE__, __LINE__)) return
#endif

uint32 I_SetPalette(uint16* data, int offset, int size);
dboolean I_AllocVBlock(gfx_t* gfx, byte* data, int size,
                        int flags, int texel_w, int texel_h, int type);

const char* I_FilePath(const char* file);
int I_ReadFile(char const* name, byte** buffer);
long I_FileLength(FILE *handle);
int I_FileExists(char *filename);

static inline uint16_t I_Swap16(x)
{
    return((x<<8)|(x>>8));
}

static inline uint32_t I_Swap32(x)
{
    return((x<<24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x>>24));
}

void memcpy32(void *dst, const void *src, uint wdcount) ITCM_CODE;
void memcpy16(void *dst, const void *src, uint wdcount) ITCM_CODE;

#define SHORT(x) x
#define LONG(x) x

#define POLY_NEW_DEPTH          (1 << 11)
#define POLY_DEPTHTEST_EQUAL    (1 << 14)
#define COORD_PACK(u, v)        ((((u) << 4) & 0xFFFF) | (((v) << 4) << 16))

#define GFX_MTX_STACK_LEVEL     ((GFX_STATUS >> 8) & 0x1F)
#define GFX_MTX_PROJ_STACK      (1 << 13)
#define GFX_MTX_BUSY            (1 << 14)
#define GFX_MTX_STACK_RESET     (1 << 15)
#define GFX_SIZE_S(x)           (x << 20)
#define GFX_SIZE_T(x)           (x << 23)
#define GFX_FORMAT(x)           (x << 26)
#define GFX_VRAM_OFFSET(x)      (((uint32)x >> 3) & 0xFFFF)

#define GFX_TEXTURE(f, w, h, fmt, offs) \
                    f               |   \
                    GFX_SIZE_S(w)   |   \
                    GFX_SIZE_T(h)   |   \
                    GFX_FORMAT(fmt) |   \
                    GFX_VRAM_OFFSET(offs)

#define GFX_ORTHO()                         \
    MATRIX_CONTROL      = GL_PROJECTION;    \
    MATRIX_IDENTITY     = 0;                \
    MATRIX_CONTROL      = GL_MODELVIEW;     \
    MATRIX_IDENTITY     = 0;                \
    MATRIX_MULT4x4      = 0x20000;          \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0xFFFD5556;       \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0xFFFFF7FF;       \
    MATRIX_MULT4x4      = 0;                \
    MATRIX_MULT4x4      = 0xFFFFF000;       \
    MATRIX_MULT4x4      = 0x1000;           \
    MATRIX_MULT4x4      = 0xFFFFEFFF;       \
    MATRIX_MULT4x4      = 0x1000

#define GFX_SCREENRECT()                                        \
    GFX_BEGIN       = GL_QUADS;                                 \
    GFX_VERTEX16    = VERTEX_PACK(0, 0);                        \
    GFX_VERTEX16    = VERTEX_PACK(-2, 0);                       \
    GFX_VERTEX16    = VERTEX_PACK(SCREENWIDTH, 0);              \
    GFX_VERTEX16    = VERTEX_PACK(-2, 0);                       \
    GFX_VERTEX16    = VERTEX_PACK(SCREENWIDTH, SCREENHEIGHT);   \
    GFX_VERTEX16    = VERTEX_PACK(-2, 0);                       \
    GFX_VERTEX16    = VERTEX_PACK(0, SCREENHEIGHT);             \
    GFX_VERTEX16    = VERTEX_PACK(-2, 0)

#endif // __DS_MAIN__