#include "d_player.h"
#include "t_bsp.h"

int                 numvertexes;
vertex_t*           vertexes;
int                 numsegs;
seg_t*              segs;
int                 numsectors;
sector_t*           sectors;
int                 numsubsectors;
subsector_t*        subsectors;
int                 numnodes;
node_t*             nodes;
int                 numleafs;
leaf_t*             leafs;
int                 numlines;
line_t*             lines;
int                 numsides;
side_t*             sides;
light_t*            lights;
int                 numlights;
macroinfo_t         macros;

player_t*           player;