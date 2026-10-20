#include "common.h"
#include "tlb/tlb_0017.h"

extern u8 effShowerTex[]; /* the shower's texture: I8, 32 x 64, in file 0x13A */

/* Loads the shower's texture (tlb_0017_2's "shower"), its top row at `scroll` (10.2 fixed point)
 * so that it scrolls. gDPLoadTextureBlock's commands, but for the tile size's row. */
Gfx *effShowerLoadTex(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetTextureImage(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 1, effShowerTex);
    gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 6,
               G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 1023, 512);
    gDPPipeSync(gfx++);
    gDPSetTile(gfx++, G_IM_FMT_I, G_IM_SIZ_8b, 4, 0, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 6,
               G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, scroll, 31 << 2, 63 << 2);
    return gfx;
}
