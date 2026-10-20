#ifndef GAUGE_H
#define GAUGE_H

#include "object.h"
#include "game.h"

/* The gauge task's work. */
typedef struct GaugeWork {
    /* 0x00 */ void (*proc[2])(s32 player, struct GaugeWork *w); /* each player's panel logic */
    /* 0x08 */ u8 pad8[0x8];
    /* 0x10 */ DLFrame *frame; /* its display-list frame */
    /* 0x14 */ u32 seg; /* segment 3: the HUD's textures (file set 0x10E's first) */
    /* 0x18 */ s32 money[2]; /* shown, in quarters */
    /* 0x20 */ u8 pad20[0x8];
    /* 0x28 */ u16 timer;
    /* 0x2A */ u8 panel[2]; /* 0 none, 1-2 "press Start", 3-6 a character (bit 7: the arrows) */
    /* 0x2C */ u8 pad2C[2];
    /* 0x2E */ s8 choice[2]; /* the character being chosen */
    /* 0x30 */ u8 pad30[2];
    /* 0x32 */ s8 lives[4];
} GaugeWork;

/* A player task's work, as far as the join needs it. */
typedef struct PlayerObj {
    /* 0x000 */ u8 pad0[0x4C];
    /* 0x04C */ s32 zone;
    /* 0x050 */ f32 x;
    /* 0x054 */ f32 y;
    /* 0x058 */ f32 z;
    /* 0x05C */ u8 pad5C[0xC0];
    /* 0x11C */ f32 startZ;
    /* 0x120 */ u8 pad120[0x30];
    /* 0x150 */ u32 flags;
} PlayerObj;

Gfx *gaugeDrawNumber(Gfx *gfx, s32 x, s32 y, s32 w, s32 h, s32 space, s32 value, s32 div);
Gfx *gaugeDrawPips(Gfx *gfx, s32 player, s32 a, s32 b);
Gfx *gaugeDrawName(Gfx *gfx, s32 player, s32 who);
Gfx *gaugeDrawLives(Gfx *gfx, s32 player, s32 lives);
Gfx *gaugeDrawJoinPanel(Gfx *gfx, GaugeWork *w, s32 player);
Gfx *gaugeDrawFace(Gfx *gfx, GaugeWork *w, s32 player, s32 chara, s32 unused);
Gfx *gaugeDrawItem(Gfx *gfx, s32 player);
Gfx *gaugeDrawClock(Gfx *gfx, Mtx *m, f32 unused);
Gfx *gaugeDrawMoney(Gfx *gfx, s32 player, GaugeWork *w);
Gfx *gaugeDrawItems(Gfx *gfx, GaugeWork *w);
Gfx *gaugeDrawHud(Gfx *gfx, GaugeWork *w, Mtx *m);
s32 gaugeProc(Obj *obj, s16 msg, void *arg);
PlayerObj *gaugeGetNthPlayer(s32 n);
void gaugeSpreadPlayers(s32 reset);
PlayerObj *gaugeJoinPartner(void);
void gaugeJoinPlayer(s32 player, s32 who);
s32 gaugeNextChara(s32 who, s32 step);
void gaugeWaitStart(s32 player, GaugeWork *w);
void gaugeSelectChara(s32 player, GaugeWork *w);
void gaugeSetOn(s32 on);

#endif
