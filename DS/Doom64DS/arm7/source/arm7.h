#ifndef __ARM7_MAIN__
#define __ARM7_MAIN__

#include <nds.h>

#define ARM7_MAIN   main

#define FIFO_MSG_PLAYERDATA     0
#define FIFO_MSG_LINEDEFDATA    1
#define FIFO_MSG_SSECTDATA      2

typedef union
{
    void* arg_p;
    int arg_i;
} fifoargs_t;

typedef struct
{
    int type;
    fifoargs_t arg[5];
} fifomsg_t;

static inline uint16_t I_Swap16(x)
{
    return((x<<8)|(x>>8));
}

static inline uint32_t I_Swap32(x)
{
    return((x<<24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x>>24));
}

int I_SendMsgToArm9(fifomsg_t* sendmsg);

#endif // __ARM7_MAIN__