#include "common.h"

/* The characters' files (overlay 9's .data, 0x801A8BA0-0x801A8C60), by character (Goemon,
 * Ebisumaru, Sasuke, Yae). Only data: plmodel.c reads the ids and the first two sizes, plmain.c's
 * table at 0x801A87A0 points at the five sizes. */

u32 gPlModelFileTable[4] = { 0x1A0, 0x1A5, 0x1AA, 0x1AF }; /* the model */
u32 gPlTexFileTable[4] = { 0x1B3, 0x1B4, 0x1B5, 0x1B6 }; /* its textures */
u32 gPlSeg2FileTable[4] = { 0x1B7, 0x1B8, 0x1B9, 0x1BA };
u32 gPlPiecesFileTable[4] = { 0x19F, 0x1A4, 0x1A9, 0x1AE }; /* the model pieces (segment 3) */
u32 gPlAtkBoxFileTable[4] = { 0x1A1, 0x1A6, 0x1AB, 0x1B0 }; /* the attack box pieces (0x0B) */
u32 gPlOtherFileTable[4] = { 0x1A2, 0x1A7, 0x1AC, 0x1B1 }; /* the other pieces (segment 5) */

/* The sizes of the buffers the files decode into. */
u32 gPlBuffSizeModel[4] = { 0xCA00, 0xCA00, 0xCA00, 0xCA00 }; /* the model */
u32 gPlBuffSizeTex[4] = { 0x3A00, 0x3A00, 0x3A00, 0x3A00 }; /* its textures */
u32 gPlBuffSize2[4] = { 0x1480, 0x1480, 0x1480, 0x1480 };
u32 gPlBuffSize3[4] = { 0x320, 0x320, 0x320, 0x320 };
u32 gPlBuffSize4[4] = { 0x440, 0x440, 0x440, 0x440 };
u32 gPlFileUnusedTable[4] = { 0, 0, 0, 0 }; /* no code or table names it */
