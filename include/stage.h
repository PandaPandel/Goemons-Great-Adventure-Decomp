#ifndef STAGE_H
#define STAGE_H

#include "object.h"
#include "game.h"
#include "player.h"

/* The stage task's work. */
typedef struct StageWork {
    /* 0x00 */ s32 file; /* the stage's file */
    /* 0x04 */ s32 (*proc)(Obj *obj, struct StageWork *w); /* each frame; -1 ends the stage */
    /* 0x08 */ Frame *frame1;
    /* 0x0C */ Frame *frame2;
    /* 0x10 */ u8 pad10[0x20];
    /* 0x30 */ Obj *pause;
} StageWork;

/* A course in the stage data. */
typedef struct StageRail {
    /* 0x0 */ s16 n; /* 0 ends the list */
    /* 0x4 */ u32 points;
} StageRail;

/* A stage's data (file 0xE, 0x60 bytes each). */
typedef struct StageData {
    /* 0x00 */ s32 file;
    /* 0x04 */ s32 objFile;
    /* 0x08 */ u32 geo;
    /* 0x0C */ u32 coll;
    /* 0x10 */ StageRail rails[10];
} StageData;

s32 Stage(Obj *obj, s16 msg, void *arg);
s32 stageProcNop(Obj *obj, StageWork *w);
Obj *stageCreate(Obj *parent);
void stageGoto(Obj *parent, s32 stage);
void stageGotoSpawnNoSave(Obj *parent, s32 stage, s32 angle, f32 x, f32 y, f32 z, f32 w, s32 state);
void stageGotoSpawnPos(Obj *parent, s32 stage, s32 angle, f32 x, f32 y, f32 z);
void stageGotoSpawnNoState(Obj *parent, s32 stage, s32 angle, f32 x, f32 y, f32 z, f32 w);
void stageGotoSpawn(Obj *parent, s32 stage, s32 angle, f32 x, f32 y, f32 z, f32 w, s32 state);
void stageRestart(Obj *parent);
void stageGotoOpening(void);
void stageGotoEnding(void);
void stageGotoStaffRoll(void);
void stageGotoNtegata(void);
void stageGotoGameOver(void);
void stageResetClockToAtlas(void);
void stageGotoSubt(s32 a);
void stageGotoAtlas(s32 a, s32 b);
void stageGotoAtlasFrom(Obj *obj, s32 a, s32 b);
void stageGotoGameProc(void);
void stageGotoPak(void);
void stageGotoTitle(void);
void stageGotoTitleArg1(void);
void stageGotoGeton(void);
void stageGotoBattle(s32 a);
s32 utamaster(Obj *obj, s16 msg, void *arg);
void stageGotoUtamaster(void);
void stageGotoSabi(void);
s32 staffM(Obj *obj, s16 msg, void *arg);
u32 stageAddrToRam(u32 addr);
void stageLoad(s32 stage);
StageData *stageReadData(StageData *out, s32 i);
void stageSetData(s32 i);
s32 stageAllOut(void);
void stageStartPos(PlInit *init, s32 idx, s32 count, s32 angle, f32 x, f32 y, f32 z);
s32 stageCountChara(s32 who);
void stageSpawnPlayers(u32 flags, s32 angle, f32 x, f32 y, f32 z);
void stageBuildEnd(void);
void stageBuild(s32 stage, s32 clock, s32 mode, u32 flags, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2, f32 cam0, f32 cam1, f32 cam2);
void stageBuildBasic(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces);
void stageBuildBasic2(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2);
void stageBuildCamMode(s32 stage, s32 mode, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces);
void stageBuildFlagged(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces);
void stageBuildFlagged2(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2);
void stageBuildDive(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces);
void stageBuildDive2(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2);
void stageBuildFixedCam(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces, f32 cam0, f32 cam1, f32 cam2);
void stageBuildZoneCam(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces);
void stageBuildZoneCam2(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2);
void stageBuildPlain(s32 angle, f32 x, f32 y, f32 z, u32 addr, s32 arg);
void stageBuild3Places(s32 angle0, f32 x0, f32 y0, f32 z0, s32 angle1, f32 x1, f32 y1, f32 z1, s32 angle2, f32 x2, f32 y2, f32 z2, u32 addr, s32 arg);
void stageBuildAtk150(s32 stage, s32 clock, s32 mode, u32 flags, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2, f32 cam0, f32 cam1, f32 cam2);
void stageBuildBasic2Atk150(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2);

#endif
