#include "object.h"
#include "game.h"
#include "attack.h"
#include "audio.h"
#include "camera.h"
#include "cockpit.h"
#include "contpak.h"
#include "dma.h"
#include "effect.h"
#include "enemy.h"
#include "fade.h"
#include "file.h"
#include "fileres.h"
#include "gameproc.h"
#include "gauge.h"
#include "geography.h"
#include "gfxmgr.h"
#include "heap.h"
#include "joypad.h"
#include "pause.h"
#include "plmain.h"
#include "ride.h"
#include "scenario.h"
#include "sndloc.h"
#include "stage.h"
#include "swwork.h"
#include "tlb/tlb_0016.h"
#include "tlb/tlb_0017.h"
#include "tlb/tlb_0021.h"
#include "tlb/tlb_0033.h"
#include "tlb/tlb_0034.h"
#include "tlb/tlb_0035.h"
#include "tlb/tlb_0036.h"
#include "tmanager.h"
#include "vram.h"

/* The game's debug print, argument-only (strings kept, arguments evaluated): what some of this
 * file's functions were built with, where the repo's `if (0)` PRINTF moves registers. */
#define DPRINTF

/* The stage task ("Stage") and the moves between scenes.
 * - A stage comes from the stage list (file 0xD: 12 bytes a stage, its init function and its
 *   file). Its file is loaded and TLB-mapped, and its init function builds it.
 * - The stage builders make the stage's tasks and spawn the players (stageSpawnPlayers). The camera
 *   and the spawn point come from the stage, unless a saved spawn (gGameWork.spawn)
 *   overrides them.
 * - A scene change sets the next task (gGameWork.next, a handler or a TLB address in a file)
 *   and its parent, and stops drawing until it takes over.
 * - While a stage runs, Start on a playing controller opens the pause task
 *   (pause_proc). */

/* A stage list entry (file 13, assets/tables/table_0013.c). */
typedef struct StageEntry {
    /* 0x0 */ char *name; /* "EDO 1-2", "CASTLE 5-1-1"... */
    /* 0x4 */ void (*init)(void);
    /* 0x8 */ s32 file;
} StageEntry;

/* stage.c's .data (0x80108B60-0x80108B6C), in ROM order. */
Obj *gStageTask = NULL; /* the stage task */
Obj *gUtamasterTask = NULL; /* the "utamaster" task */
Obj *gStaffMTask = NULL; /* the "staffM" task */
extern Unk88282 gSaveCopyPlayers[4]; /* the players' entries for a continue */
/* The four players' characters and lives (gSaveData.players[i].chara/lives), by address. */
extern s8 gPlayer1Chara;
extern s8 gPlayerLives;
extern s8 gPlayer2Chara;
extern s8 gPlayer2Lives;
extern s8 gPlayer3Chara;
extern s8 gPlayer3Lives;
extern s8 gPlayer4Chara;
extern s8 gPlayer4Lives;
/* Tasks in files, at their TLB addresses (0x08000000 is a file's start). */
extern StageEntry gStageList[]; /* the stage list, at the start of file 0xD (undefined_syms.txt) */
s32 titleProc(Obj *obj, s16 msg, void *arg); /* FAKE: tlb/tlb_0018.h is not included here: it brings tlb.h, whose far-call macros are for the TLB files */ /* tlb_0018 */
s32 subtProc(Obj *obj, s16 msg, void *arg); /* FAKE: tlb/tlb_0018.h is not included here: it brings tlb.h, whose far-call macros are for the TLB files */ /* tlb_0018 */
s32 ntegataProc(Obj *obj, s16 msg, void *arg); /* FAKE: tlb/tlb_0018.h is not included here: it brings tlb.h, whose far-call macros are for the TLB files */ /* tlb_0018 */



#define STAGE(obj) ((StageWork *) ((u32) (obj) + sizeof(Obj)))

/* The stage task's handler. Create: the letterbox (16, 8)-(304, 198) and two retraces a frame,
 * and the stage arg (or, for -1, the current one). Run: the stage's own function, then Start
 * opens the pause task. While the pause task runs, the frame returns 0 without the default
 * handler. */
s32 Stage(Obj *obj, s16 msg, void *arg) {
    StageWork *w = (StageWork *) (obj + 1);
    s32 i;
    s32 file;

    switch (msg) {
        case 2:
            obj->name = "Stage";
            gStageTask = obj;
            gGameState.retracesPerFrame = 2;
            gGameWork.frames = 0;
            gGameState.scissor.ulx = 16;
            gGameState.scissor.uly = 8;
            gGameState.scissor.lrx = 304;
            gGameState.scissor.lry = 198;
            for (i = 0; i < 4; i++) {
                joySetStickDpad(i, 0);
            }
            for (i = 0; i < 4; i++) {
                contPakInitRumble(i);
                contPakRumbleRequestStop(i);
            }
            if ((s32) arg < 0) {
                STAGE(obj)->proc = stageProcNop;
                stageLoad(gGameWork.stage);
            } else if ((s32) arg > 0) {
                STAGE(obj)->proc = stageProcNop;
                stageLoad((s32) arg);
            }
            break;
        case 1:
            gGameState.retracesPerFrame = 1;
            gGameState.scissor.ulx = 0;
            gGameState.scissor.uly = 0;
            gGameState.scissor.lrx = 0;
            gGameState.scissor.lry = 0;
            if (resFindFile(0xD) != -1) {
                resFree(0xD);
            }
            file = w->file;
            if (file != 0) {
                resFree(file);
            }
            frmDestroy(w->frame1);
            frmDestroy(w->frame2);
            gStageTask = NULL;
            break;
        case 0:
            if (w->proc(obj, w) == -1) {
                return -1;
            }
            if (w->pause == NULL) {
                if (gGameWork.scenarioRunning != 2) {
                    for (i = 0; i < 4; i++) {
                        if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].lives > 0 &&
                            joyGetHeldTime(i, START_BUTTON) == 1 &&
                            soundNoPlPoweredDown() == 1) {
                            w->pause = tskCreate(obj->parent, pause_proc, 0,
                                                              NULL);
                        }
                    }
                }
                gGameWork.frames++;
            } else {
                if (!(w->pause->flags & 1) || w->pause->proc != pause_proc) {
                    w->pause = NULL;
                }
                return 0;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A stage function that does nothing. */
s32 stageProcNop(Obj *obj, StageWork *w) {
    return 0;
}

/* The stage task, made under parent if there is none. */
Obj *stageCreate(Obj *parent) {
    if (gStageTask != NULL) {
        return gStageTask;
    }
    gStageTask = tskCreate(parent, Stage, 0, NULL);
    return gStageTask;
}

/* Moves to stage `stage` under parent, keeping the saved spawn. */
void stageGoto(Obj *parent, s32 stage) {
    gGameState.skipDraw = 2;
    gGameWork.flags |= 3;
    gGameWork.nextParent = parent;
    gGameWork.next = Stage;
    gGameWork.nextArg = -1;
    gGameWork.stage = stage;
    gGameWork.savedSpawn = gGameWork.spawn;
    gGameWork.partake = 0;
    gGameWork.scenarioRunning = 0;
    contPakRumbleSuspend();
}

/* Moves to a stage with the players at (x, y, z). */
void stageGotoSpawnNoSave(Obj *parent, s32 stage, s32 angle, f32 x, f32 y, f32 z, f32 w,
                            s32 state) {
    gGameState.skipDraw = 2;
    gGameWork.flags |= 2;
    gGameWork.nextParent = parent;
    gGameWork.next = Stage;
    gGameWork.nextArg = -1;
    gGameWork.stage = stage;
    gGameWork.spawn.on = 1;
    gGameWork.spawn.angle = angle;
    gGameWork.partake = 0;
    gGameWork.scenarioRunning = 0;
    gGameWork.spawn.x = x;
    gGameWork.spawn.y = y;
    gGameWork.spawn.z = z;
    gGameWork.spawn.facing = w;
    gGameWork.spawn.action = state;
    contPakRumbleSuspend();
}

/* The same, facing angle 0 and in the default state. */
void stageGotoSpawnPos(Obj *parent, s32 stage, s32 angle, f32 x, f32 y, f32 z) {
    stageGotoSpawnNoState(parent, stage, angle, x, y, z, 0.0f);
}

void stageGotoSpawnNoState(Obj *parent, s32 stage, s32 angle, f32 x, f32 y, f32 z, f32 w) {
    stageGotoSpawn(parent, stage, angle, x, y, z, w, -1);
}

/* Sets the spawn, then moves to the stage. */
void stageGotoSpawn(Obj *parent, s32 stage, s32 angle, f32 x, f32 y, f32 z, f32 w,
                            s32 state) {
    gGameWork.spawn.on = 1;
    gGameWork.spawn.angle = angle;
    gGameWork.spawn.x = x;
    gGameWork.spawn.y = y;
    gGameWork.spawn.z = z;
    gGameWork.spawn.facing = w;
    gGameWork.spawn.action = state;
    stageGoto(parent, stage);
}

/* Restarts the stage from the saved spawn. */
void stageRestart(Obj *parent) {
    gGameWork.tunePart = 0;
    gGameWork.sndStopAllFirst = 1;
    gGameWork.restartSdCount = fadeCountPlayers();
    gGameWork.spawn = gGameWork.savedSpawn;
    stageGoto(parent, gGameWork.stage);
}

/* Scene changes to tasks in files (TLB addresses). */
void stageGotoOpening(void) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = opmainProc;
    gGameWork.nextFile = 0x15;
    gGameWork.nextOverlay = 9;
    contPakRumbleSuspend();
}

void stageGotoEnding(void) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = endParentProc;
    gGameWork.nextFile = 0x22;
    gGameWork.nextOverlay = 9;
    contPakRumbleSuspend();
}

/* To the staff roll. */
void stageGotoStaffRoll(void) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = staffM;
    gGameWork.nextFile = 0x24;
    gGameWork.nextOverlay = 9;
    contPakRumbleSuspend();
}

void stageGotoNtegata(void) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = ntegataProc;
    gGameWork.nextFile = 0x12;
    contPakRumbleSuspend();
}

void stageGotoGameOver(void) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = goverParentProc;
    gGameWork.nextFile = 0x23;
    gGameWork.nextOverlay = 9;
    contPakRumbleSuspend();
}

/* The clock to 5:00 and a 300-second day, then stageGotoAtlasFrom's scene. */
void stageResetClockToAtlas(void) {
    gGameWork.time = 5 << 16;
    gGameWork.daySeconds = 300;
    stageGotoAtlasFrom(NULL, gGameWork.area, gGameWork.areaStage);
    gGameWork.flags &= ~1;
}

void stageGotoSubt(s32 a) {
    gGameState.skipDraw = 2;
    gGameWork.time = 5 << 16;
    gGameWork.daySeconds = 300;
    gGameWork.area = a;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = subtProc;
    gGameWork.nextFile = 0x12;
    gGameWork.nextOverlay = 9;
    gGameWork.flags &= ~1;
    contPakRumbleSuspend();
}

void stageGotoAtlas(s32 a, s32 b) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gStageTask->parent;
    gGameWork.next = mapAtlasProc;
    gGameWork.nextFile = 0x10;
    gGameWork.nextOverlay = 9;
    gGameWork.area = a;
    gGameWork.areaStage = b;
    contPakRumbleSuspend();
}

void stageGotoAtlasFrom(Obj *obj, s32 a, s32 b) {
    gGameState.skipDraw = 2;
    if (obj == NULL) {
        gGameWork.nextParent = gGameWork.gameProcTask;
    } else {
        gGameWork.nextParent = obj->parent;
    }
    gGameWork.next = mapAtlasProc;
    gGameWork.nextFile = 0x10;
    gGameWork.nextOverlay = 9;
    gGameWork.area = a;
    gGameWork.areaStage = b;
    contPakRumbleSuspend();
}

/* Back to the start (game_proc). */
void stageGotoGameProc(void) {
    gGameState.skipDraw = 2;
    gGameWork.next = game_proc;
    contPakRumbleSuspend();
}

/* To the Controller Pak screen (next is "pak"). */
void stageGotoPak(void) {
    gGameState.skipDraw = 2;
    gGameWork.next = (void *) 0x70616B;
    contPakRumbleSuspend();
}

void stageGotoTitle(void) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = titleProc;
    gGameWork.nextFile = 0x12;
    gGameWork.nextOverlay = 9;
    contPakRumbleSuspend();
}

void stageGotoTitleArg1(void) {
    stageGotoTitle();
    gGameWork.nextArg = 1;
}

void stageGotoGeton(void) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = getonProc;
    gGameWork.nextFile = 0x11;
    gGameWork.nextOverlay = 9;
    gGameWork.nextArg = 1;
    contPakRumbleSuspend();
}

void stageGotoBattle(s32 a) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = cockpitProc;
    gGameWork.nextOverlay = 10;
    gGameWork.nextArg = a;
    contPakRumbleSuspend();
}

/* The "utamaster" task's work. */
typedef struct UtaMaster {
    /* 0x0 */ u8 state;
    /* 0x2 */ u16 timer;
} UtaMaster;

/* The "utamaster" task: waits for A or Start on any controller, then fades out and moves on
 * (stageGotoTitle) after 0x24 frames. */
s32 utamaster(Obj *obj, s16 msg, void *arg) {
    UtaMaster *w = (UtaMaster *) ((u32) obj + sizeof(Obj));
    s32 i;

    switch (msg) {
        case 2:
            gUtamasterTask = obj;
            obj->name = "utamaster";
            fileLoadNow(0x21, NULL);
            tskCreate(obj, utaDemoProc, 0x21, NULL);
            break;
        case 1:
            gUtamasterTask = NULL;
            break;
        case 0:
            w->timer++;
            switch (w->state) {
                case 0:
                    for (i = 0; i < 4; i++) {
                        if (joyGetButton(i, 0) & (A_BUTTON | START_BUTTON)) {
                            w->timer = 0;
                            w->state = 1;
                        }
                    }
                    break;
                case 1:
                    if (w->timer == 1) {
                        tskDestroy(fadeCreate(gGameWork.gameProcTask));
                        fadeCreate(gGameWork.gameProcTask);
                        fadeSetAll(32.0f, 0, 255.0f, 0, 0, 0);
                        sndPlay(7);
                        return 0;
                    }
                    if (w->timer == 0x24) {
                        (*stageGotoTitle)();
                    }
                    return 0;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* To the "utamaster" task. */
void stageGotoUtamaster(void) {
    gGameState.skipDraw = 2;
    gGameWork.nextParent = gGameWork.gameProcTask;
    gGameWork.next = utamaster;
    gGameWork.nextOverlay = 9;
    contPakRumbleSuspend();
}

void stageGotoSabi(void) {
    gGameState.skipDraw = 2;
    gGameWork.flags |= 1;
    if (gUtamasterTask != NULL) {
        gGameWork.nextParent = gUtamasterTask;
    } else {
        gGameWork.nextParent = gGameWork.gameProcTask;
    }
    gGameWork.next = sabiProc;
    gGameWork.nextFile = 0x11;
    gGameWork.nextArg = 1;
    contPakRumbleSuspend();
}

/* The "staffM" task: the staff roll's manager. After 8 frames it plays sound 0xB2. (Its work
 * is two words; the calls go through pointers; the work pointer is set at its declaration and
 * three pad words put its spill where the game has it.) */
s32 staffM(Obj *obj, s16 msg, void *arg) {
    s32 *w = (s32 *) ((u32) obj + sizeof(Obj));
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "staffM";
            gStaffMTask = obj;
            (*tskCreateBlocks)(obj, staffMain, 2, 0, NULL);
            w[0] = 0;
            w[1] = 0;
            break;
        case 0:
            switch (w[0]) {
                case 0:
                    w[1]++;
                    if (w[1] >= 8) {
                        (*sndPlay)(0xB2);
                        w[0] = 1;
                    }
                    break;
                case 1:
                case 2:
                    break;
            }
            break;
        case 1:
            break;
    }
    return (*tskDefaultProc)(obj, msg, arg);
}

/* A stage file's address (0x08xxxxxx) in RAM; others as they are. */
u32 stageAddrToRam(u32 addr) {
    StageWork *w = STAGE(gStageTask);

    if (addr == 0) {
        return 0;
    }
    if ((addr & 0xFF000000) == 0x08000000) {
        return fileRamAddr(w->file) + (addr & 0xFFFFFF);
    }
    return addr;
}

/* Loads stage `stage`: its entry from the stage list, its file, then its init function with
 * the file mapped. The list's offset is its own address in file 0xD, (u32) gStageList &
 * 0xFFFFFF; size and entries hold the frame's slots above e. */
void stageLoad(s32 stage) {
    StageWork *w = STAGE(gStageTask);
    s32 size;
    u8 *list;
    StageEntry *entries;
    StageEntry e;
    s32 pad;
    u32 saved;

    gGameWork.stage = stage;
    DPRINTF("ステージ一覧データーをロードします \n");
    size = fileRamSize(0xD);
    list = heapMallocTop(size);
    if (list != NULL) {
        fileLoadNow(0xD, list);
        entries = (StageEntry *) (list + ((u32) gStageList & 0xFFFFFF));
        e = entries[stage];
        heapFree(list);
        if (w->file != 0) {
            resFree(w->file);
            w->file = 0;
        }
        w->file = e.file;
        DPRINTF("ステージデーターをロードします \n");
        fileLoadNow(w->file, NULL);
        saved = fileMappedAddr();
        fileMapTlb(w->file);
        DPRINTF("ステージ初期化関数をコールします \n");
        e.init();
        if (saved != 0) {
            fileMapTlb(saved);
        }
        resDrop(0xD);
    } else {
        DPRINTF("<stageLoad> メモリーが足りません \n");
        if (list != NULL) {
            heapFree(list);
        }
    }
}

/* Stage data i (file 0xE), read from ROM; the first if i is past the end. */
StageData *stageReadData(StageData *out, s32 i) {
    StageData d;
    u32 rom = fileRomStart(0xE) + i * sizeof(StageData);

    if (fileRomEnd(0xE) < rom) {
        rom = fileRomStart(0xE);
    }
    dmaReadRom(rom, &d, sizeof(StageData));
    *out = d;
    return out;
}

/* Sets up stage i's data: its file, terrain, collision and course. */
void stageSetData(s32 i) {
    StageData d;

    stageReadData(&d, i);
    fileLoadNow(d.file, NULL);
    PRINTF("地形データの書き換え\n");
    geoLoadStage(i);
    PRINTF("当たりデータの書き換え\n");
    vramLoadColl(d.coll, d.file);
    PRINTF("コースデータの書き換え\n");
    vramLoadCourses(i);
    PRINTF("データセット完了\n");
}

/* No player left: in a two-player game with the other players' continue entries alive, they
 * carry on (unk13 = 2). Otherwise every player gets 3 lives, and health at least 100, and the
 * game goes to stageGotoGameOver's scene. */
s32 stageAllOut(void) {
    s32 i;

    swWorkReset(0x1B);
    if (gGameWork.mission == 1) {
        for (i = 0; i < 4; i++) {
            if (gSaveCopyPlayers[i].chara != -1 && gSaveCopyPlayers[i].lives > 0) {
                gGameWork.mission = 2;
                return 0;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        gSaveData.players[i].lives = 3;
        gSaveData.players[i].life = 3;
        if (gSaveData.players[i].money < 100) {
            gSaveData.players[i].money = 100;
        }
    }
    gGameWork.sndStopAllFirst = 0;
    gGameWork.restartSdCount = 0;
    stageGotoGameOver();
    return 1;
}

/* Where player idx of count starts: the saved spawn, or the stage's, spread out along x (or
 * z) by the number of players. */
void stageStartPos(PlInit *init, s32 idx, s32 count, s32 angle, f32 x, f32 y,
                            f32 z) {
    f32 d;

    if (gGameWork.spawn.on) {
        init->zone = gGameWork.spawn.angle;
        init->place = gGameWork.spawn.x;
        init->height = gGameWork.spawn.y;
        init->side = gGameWork.spawn.z;
        init->facing = gGameWork.spawn.facing;
    } else {
        init->zone = angle;
        init->place = x;
        init->height = y;
        init->side = z;
    }
    if (angle == 0) {
        init->flags |= 0x10;
        init->action = 0x122;
        switch (count) {
            case 2:
                d = 5 - idx * 10;
                init->place += d;
                init->side += d;
                break;
            case 3:
                d = 5 - idx * 5;
                init->place += d;
                init->side += d;
                break;
            case 4:
                init->place += 5.0 - idx * 3.3;
                init->side += 5.0 - idx * 3.3;
                break;
        }
    } else {
        switch (count) {
            case 2:
                init->side += 3 - idx * 6;
                break;
            case 3:
                init->side += 3 - idx * 3;
                break;
            case 4:
                init->side += 3 - idx * 2;
                break;
        }
    }
    if (gGameWork.spawn.on && gGameWork.spawn.action >= 0) {
        init->action = gGameWork.spawn.action;
    }
}

/* How many players in play are character `who`. FAKE (decomp-permuter): the four tests on one
 * line, as one macro would expand them; on four lines IDO schedules the luis otherwise. */
s32 stageCountChara(s32 who) {
    s32 n = 0;

    if (who == gPlayer1Chara && gPlayerLives > 0) { n = 1; } if (who == gPlayer2Chara && gPlayer2Lives > 0) { n++; } if (who == gPlayer3Chara && gPlayer3Lives > 0) { n++; } if (who == gPlayer4Chara && gPlayer4Lives > 0) { n++; }
    return n;
}

/* Spawns the players in play (two of the same character are made different). The saved spawn
 * is then cleared; with no player made, stageAllOut. The zeroes go in the for inits (after
 * i = 0), as the game schedules them. */
void stageSpawnPlayers(u32 flags, s32 angle, f32 x, f32 y, f32 z) {
    s32 count;
    s32 i;
    s32 n;
    PlInit init;
    Obj *o;

    for (i = 0, count = 0; i < 4; i++) {
        if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].chara < 4 &&
            gSaveData.players[i].lives > 0) {
            count++;
        }
    }
    for (i = 0, n = 0; i < 4; i++) {
        if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].chara < 4 &&
            gSaveData.players[i].lives > 0) {
            if (stageCountChara(gSaveData.players[i].chara) >= 2) {
                gSaveData.players[i].chara =
                    gaugeNextChara(gSaveData.players[i].chara, 1);
            }
            plMakeInit(&init, i, gSaveData.players[i].chara);
            init.flags |= flags;
            stageStartPos(&init, n, count, angle, x, y, z);
            n++;
            o = plCreate(gStageTask, &init);
            if (o != NULL) {
                camAiAddLookat(o, i);
            } else {
                PRINTF("!!!! プレイヤーの作成に失敗しました !!!!\n");
                gGameWork.partake = 1;
                gSaveData.players[i].chara = -1;
            }
        }
    }
    heapMemClear(&gGameWork.spawn, sizeof(Spawn));
    n = 0;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) != 0) {
            n++;
        }
    }
    if (n == 0) {
        stageAllOut();
    }
}

void stageBuildEnd(void) {
}

/* Builds a stage: its tasks, the camera (mode, and camAiSetPos's three values), the stage
 * data (stageSetData), its two enemy placement lists from the stage file with their counts
 * (eneMakeBench), the players and the clock. */
void stageBuild(s32 stage, s32 clock, s32 mode, u32 flags, s32 angle, f32 x, f32 y,
                            f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2, f32 cam0, f32 cam1,
                            f32 cam2) {
    if (gStageTask != NULL) {
        places = stageAddrToRam(places);
        places2 = stageAddrToRam(places2);
        tskCreate(gStageTask, effect, 0, NULL);
        tskCreate(gStageTask, gaugeProc, 0, NULL);
        tskCreate(gStageTask, Geography, 0, NULL);
        vramCreate(gStageTask);
        tskCreate(gStageTask, attack_Proc, 0, NULL);
        camAiCreate(gStageTask);
        camAiSetMode(mode);
        camAiSetPos(cam0, cam1, cam2);
        stageSetData(stage);
        eneMakeManager(gStageTask);
        tskCreate(gStageTask, Ride, 0, NULL);
        eneMakeBench(places, nPlaces, places2, nPlaces2);
        stageSpawnPlayers(flags, angle, x, y, z);
        camAiSnap();
        geoSetMode(clock);
        eneSpawnStart();
        stageBuildEnd();
    }
}

/* Stage builders by kind, each with a task for its players (scParentNoFilesProc or scParentProc). */
void stageBuildBasic(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces) {
    stageBuild(stage, 0, 0, 0, angle, x, y, z, places, nPlaces, 0, 0, 0.0f, 0.0f, 0.0f);
    tskCreate(gStageTask, scParentNoFilesProc, 0, NULL);
}

void stageBuildBasic2(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces,
                            u32 places2, s32 nPlaces2) {
    stageBuild(stage, 0, 0, 0, angle, x, y, z, places, nPlaces, places2, nPlaces2, 0.0f, 0.0f,
                           0.0f);
    tskCreate(gStageTask, scParentNoFilesProc, 0, NULL);
}

void stageBuildCamMode(s32 stage, s32 mode, s32 angle, f32 x, f32 y, f32 z, u32 places,
                            s32 nPlaces) {
    stageBuild(stage, 1, mode, 0, angle, x, y, z, places, nPlaces, 0, 0, 0.0f, 0.0f, 0.0f);
    tskCreate(gStageTask, scParentNoFilesProc, 0, NULL);
}

void stageBuildFlagged(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces) {
    stageBuild(stage, 0, 0, 0x20000000, angle, x, y, z, places, nPlaces, 0, 0, 0.0f, 0.0f,
                           0.0f);
    tskCreate(gStageTask, scParentNoFilesProc, 0, NULL);
}

void stageBuildFlagged2(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces,
                            u32 places2, s32 nPlaces2) {
    stageBuild(stage, 0, 0, 0x20000000, angle, x, y, z, places, nPlaces, places2, nPlaces2, 0.0f,
                           0.0f, 0.0f);
    tskCreate(gStageTask, scParentNoFilesProc, 0, NULL);
}

void stageBuildDive(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces) {
    stageBuild(stage, 0, 5, 0x10000, angle, x, y, z, places, nPlaces, 0, 0, 0.0f, 0.0f,
                           0.0f);
    tskCreate(gStageTask, scParentNoFilesProc, 0, NULL);
    gGameWork.partake = 1;
}

void stageBuildDive2(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces,
                            u32 places2, s32 nPlaces2) {
    stageBuild(stage, 0, 5, 0x10000, angle, x, y, z, places, nPlaces, places2, nPlaces2, 0.0f,
                           0.0f, 0.0f);
    tskCreate(gStageTask, scParentNoFilesProc, 0, NULL);
    gGameWork.partake = 1;
}

void stageBuildFixedCam(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces,
                            f32 cam0, f32 cam1, f32 cam2) {
    stageBuild(stage, 1, 4, 0x80000000, angle, x, y, z, places, nPlaces, 0, 0, cam0, cam1,
                           cam2);
    tskCreate(gStageTask, scParentProc, 0, NULL);
    camSunSetClockStop(1);
    sndPlay(0x51F);
    camSunAddSoundPair(0x12E, 0x51F);
    camSunAddSoundPair(0x12F, 0x51F);
    camSunAddSoundPair(0x12D, 0x51F);
    camSunAddSoundPair(0x2AA, 0x51F);
    camSunAddSoundPair(0x289, 0x51F);
    camSunAddSoundPair(0x28B, 0x51F);
    camSunAddSoundPair(0x28A, 0x51F);
    gGameWork.partake = 1;
}

void stageBuildZoneCam(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces) {
    stageBuild(stage, 0, 3, 0x80000000, angle, x, y, z, places, nPlaces, 0, 0, 0.0f, 0.0f,
                           0.0f);
    tskCreate(gStageTask, scParentProc, 0, NULL);
}

void stageBuildZoneCam2(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces,
                            u32 places2, s32 nPlaces2) {
    stageBuild(stage, 0, 3, 0x80000000, angle, x, y, z, places, nPlaces, places2, nPlaces2, 0.0f,
                           0.0f, 0.0f);
    tskCreate(gStageTask, scParentProc, 0, NULL);
}

/* A plain stage: no camera mode, no stage data, the players at one place, one thing from the
 * stage file (eneMakeBenchOne), and the clock left alone (geoRefresh). */
void stageBuildPlain(s32 angle, f32 x, f32 y, f32 z, u32 addr, s32 arg) {
    if (gStageTask != NULL) {
        addr = stageAddrToRam(addr);
        tskCreate(gStageTask, effect, 0, NULL);
        tskCreate(gStageTask, gaugeProc, 0, NULL);
        tskCreate(gStageTask, attack_Proc, 0, NULL);
        camAiCreate(gStageTask);
        eneMakeManager(gStageTask);
        tskCreate(gStageTask, Ride, 0, NULL);
        eneMakeBenchOne(addr, arg);
        tskCreate(gStageTask, scParentProc, 0, NULL);
        stageSpawnPlayers(0, angle, x, y, z);
        camAiSnap();
        geoRefresh();
        eneSpawnStart();
        stageBuildEnd();
    }
}

/* A stage with the players placed by how many are in play, at three given places: one player
 * at the first; two at the second and third; three at the second, first and third; four at
 * the second, a third of the way from the first back toward and past it, and the third. */
void stageBuild3Places(s32 angle0, f32 x0, f32 y0, f32 z0, s32 angle1, f32 x1, f32 y1, f32 z1,
                            s32 angle2, f32 x2, f32 y2, f32 z2, u32 addr, s32 arg) {
    s32 count;
    s32 i;
    s32 pad[3];
    s32 sky[2];

    if (gStageTask != NULL) {
        addr = stageAddrToRam(addr);
        tskCreate(gStageTask, effect, 0, NULL);
        tskCreate(gStageTask, gaugeProc, 0, NULL);
        sky[0] = 70;
        sky[1] = 70;
        tskCreate(gStageTask, attack_Proc, 0, sky);
        camAiCreate(gStageTask);
        eneMakeManager(gStageTask);
        tskCreate(gStageTask, Ride, 0, NULL);
        eneMakeBenchOne(addr, arg);
        tskCreate(gStageTask, scParentProc, 0, NULL);
        for (i = 0, count = 0; i < 4; i++) {
            if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].chara < 4 &&
                gSaveData.players[i].lives > 0) {
                count++;
            }
        }
        for (i = 0; i < 4; i++) {
            if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].chara < 4 &&
                gSaveData.players[i].lives > 0) {
                PlInit init;

                plMakeInit(&init, i, gSaveData.players[i].chara);
                if (count == 1) {
                    init.zone = angle0;
                    init.place = x0;
                    init.height = y0;
                    init.side = z0;
                } else if (count == 2) {
                    if (i == 0) {
                        init.zone = angle1;
                        init.place = x1;
                        init.height = y1;
                        init.side = z1;
                    } else {
                        init.zone = angle2;
                        init.place = x2;
                        init.height = y2;
                        init.side = z2;
                    }
                } else if (count == 3) {
                    if (i == 0) {
                        init.zone = angle1;
                        init.place = x1;
                        init.height = y1;
                        init.side = z1;
                    } else if (i == 1) {
                        init.zone = angle0;
                        init.place = x0;
                        init.height = y0;
                        init.side = z0;
                    } else {
                        init.zone = angle2;
                        init.place = x2;
                        init.height = y2;
                        init.side = z2;
                    }
                } else if (count == 4) {
                    if (i == 0) {
                        init.zone = angle1;
                        init.place = x1;
                        init.height = y1;
                        init.side = z1;
                    } else if (i == 1) {
                        init.zone = angle0;
                        init.place = x0 + (x1 - x0) / 3.0f;
                        init.height = y0 + (y1 - y0) / 3.0f;
                        init.side = z0 + (z1 - z0) / 3.0f;
                    } else if (i == 2) {
                        init.zone = angle0;
                        init.place = x0 - (x1 - x0) / 3.0f;
                        init.height = y0 - (y1 - y0) / 3.0f;
                        init.side = z0 - (z1 - z0) / 3.0f;
                    } else {
                        init.zone = angle2;
                        init.place = x2;
                        init.height = y2;
                        init.side = z2;
                    }
                }
                camAiAddLookat(plCreate(gStageTask, &init), i);
            }
        }
        heapMemClear(&gGameWork.spawn, sizeof(Spawn));
        eneSpawnStart();
        stageBuildEnd();
    }
}

/* stageBuild with the sky (attack_Proc) given 150 and 50. */
void stageBuildAtk150(s32 stage, s32 clock, s32 mode, u32 flags, s32 angle, f32 x, f32 y,
                            f32 z, u32 places, s32 nPlaces, u32 places2, s32 nPlaces2, f32 cam0, f32 cam1,
                            f32 cam2) {
    s32 pad[5];
    s32 sky[2];
    s32 pad2;

    if (gStageTask != NULL) {
        places = stageAddrToRam(places);
        places2 = stageAddrToRam(places2);
        tskCreate(gStageTask, effect, 0, NULL);
        tskCreate(gStageTask, gaugeProc, 0, NULL);
        tskCreate(gStageTask, Geography, 0, NULL);
        vramCreate(gStageTask);
        sky[0] = 150;
        sky[1] = 50;
        tskCreate(gStageTask, attack_Proc, 0, sky);
        camAiCreate(gStageTask);
        camAiSetMode(mode);
        camAiSetPos(cam0, cam1, cam2);
        stageSetData(stage);
        eneMakeManager(gStageTask);
        tskCreate(gStageTask, Ride, 0, NULL);
        eneMakeBench(places, nPlaces, places2, nPlaces2);
        stageSpawnPlayers(flags, angle, x, y, z);
        camAiSnap();
        geoSetMode(clock);
        eneSpawnStart();
        stageBuildEnd();
    }
}

void stageBuildBasic2Atk150(s32 stage, s32 angle, f32 x, f32 y, f32 z, u32 places, s32 nPlaces,
                            u32 places2, s32 nPlaces2) {
    stageBuildAtk150(stage, 0, 0, 0, angle, x, y, z, places, nPlaces, places2, nPlaces2, 0.0f, 0.0f, 0.0f);
    tskCreate(gStageTask, scParentNoFilesProc, 0, NULL);
}
