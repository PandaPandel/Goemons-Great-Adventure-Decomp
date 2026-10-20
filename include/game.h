#ifndef GAME_H
#define GAME_H

/* The game-state block at 0x800881C0: 0x2C bytes (the game's init clears exactly that). Fields are
 * named where their use is known; every width is the one the game's own loads and stores use.
 * The display-list head (0x800881EC) and the loading flag (0x800881F0) follow it as separate
 * variables. */

#include "common.h"

/* An RGB colour, a halfword a channel. */
typedef struct Rgb16 {
    u16 r, g, b;
} Rgb16;

/* The screen's scissor box (all 0: the whole screen). */
typedef struct Scissor {
    u16 ulx, uly, lrx, lry;
} Scissor;

typedef struct GameState {
    /* 0x00 */ Rgb16 fadeFrom;        /* RGB the fill fades from (sysClearFrame) */
    /* 0x06 */ u16 fillAlpha;              /* 1 at init; bit 0 is the fill colour's alpha */
    /* 0x08 */ u16 fadeStart;         /* first line of the fade */
    /* 0x0A */ u16 fadeTo[3];         /* RGB it fades to */
    /* 0x10 */ u8 pad10[0x2];
    /* 0x12 */ u16 fadeEnd;           /* last line of the fade */
    /* 0x14 */ Scissor scissor;       /* all 0: the whole screen */
    /* 0x1C */ u16 colorImageSize;    /* G_IM_SIZ_* of the framebuffer; 2 at init */
    /* 0x1E */ s8 fbIndex;            /* the framebuffer being drawn: 0, 1, 2 */
    /* 0x1F */ u8 unused1F;
    /* 0x20 */ s8 shownIndex;         /* the scheduler's buffer index, as a byte */
    /* 0x21 */ u8 unused21;
    /* 0x22 */ s8 skipDraw;              /* 2 skips drawing; 1 while a framebuffer is borrowed */
    /* 0x23 */ u8 unused23;
    /* 0x24 */ s8 retracesPerFrame;   /* 1 at init */
    /* 0x25 */ s8 fade;               /* nonzero: fill with a vertical fade, else one colour */
    /* 0x26 */ s8 noDraw;              /* nonzero skips drawing too (frmGraphicManagerProc) */
    /* 0x27 */ u8 unused27;
    /* 0x28 */ s32 frameCount;
} GameState; /* size 0x2C */

extern GameState gGameState;

/* One of four players' entries at 0x80088282 (src/sys.c sets them up). A port is in play while
 * chara is not -1 and lives is above 0 (fadeCountPlayers). The HUD (gauge.c) shows them. */
typedef struct Unk88282 {
    /* 0x0 */ s8 chara; /* the character (0-3), -1 for none */
    /* 0x1 */ s8 lives; /* lives */
    /* 0x2 */ s8 life; /* the first row of the HUD's three pips */
    /* 0x3 */ s8 armor; /* and the second */
    /* 0x4 */ s16 money; /* money (ryo), 0-999; at least 100 after a continue */
    /* 0x6 */ s8 unused6; /* read once (gauge.c), into a parameter gaugeDrawFace ignores */
    /* 0x7 */ s8 weaponLevel; /* the face picture shown */
    /* 0x8 */ s8 food; /* the item picture shown (1-3) */
    /* 0x9 */ u8 unused9;
} Unk88282; /* size 0xA */

/* The save data (a Controller Pak save slot, src/contpak.c, whose first word is its CRC). */
typedef struct Unk88208 {
    /* 0x000 */ u32 crcHi; /* the slot's first u64 is its CRC (contpak.c): this word is always 0 */
    /* 0x004 */ u32 crc; /* the CRC's low word (contPakCrc32 over the 0x130 bytes, with these 8 zeroed) */
    /* 0x008 */ u8 savePoint; /* where the save was made ("SAVE_POINT", sccheck.c); 0: the beginning */
    /* 0x009 */ u8 charaCount; /* the characters available (2-4; gauge.c) */
    /* 0x00A */ u8 flags[0x64]; /* event flags 0-799, a bit each (swwork.c) */
    /* 0x06E */ u8 tegata[6]; /* summed and shown on the pause screen (pause.c) */
    /* 0x074 */ u8 pad74[2];
    /* 0x076 */ u16 debtPaid; /* ryo paid toward Kensuke's debt of 3500 (tlb_0137's money box; sccheck.c) */
    /* 0x078 */ u16 debt2Paid; /* and toward the second debt, of 100 */
    /* 0x07A */ Unk88282 players[4];
    /* 0x0A2 */ u8 areaFlags[0x8];
    /* 0x0AA */ u8 stageFlags[0x71]; /* by area * 16 + stage; bit 0: the stage can be left */
    /* 0x11B */ s8 costume[4][4]; /* each character's: [0] the one worn (lb), [type] 1 if owned */
    /* 0x12B */ s8 allClear; /* the game has been finished (sccheck.c) */
    /* 0x12C */ u8 pad12C[4];
} Unk88208; /* size 0x130 */

extern Unk88208 gSaveData;

#endif
