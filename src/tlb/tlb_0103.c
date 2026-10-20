#include "tlb.h"

/* The town's shops: the inn (Yadoya: its table, "yadoRoom", the room camera "y_cam", the
 * "steam" and "yuge"), the Yorozu, Ganso and Honke shops' tables, and the tea house
 * ("chamise") where the four players sit down (chaGOE, chaEBI, chaSAS, chaYAE). */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* A seated player's work ("chaGOE" and the others), at its task + 0x20; also the argument
 * its task is made with (the player and the set are copied from it). Up to 0x44 a talking
 * clerk's (tlb_0109's DemoClerkW). */
typedef struct ChaW {
    /* 0x00 */ u8 state; /* 0 sitting, 1 talked to, 2 turning, 3 talking, 4 standing up */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box;
    /* 0x3C */ u8 flags;
    /* 0x3D */ u8 pad3D[3];
    /* 0x40 */ Obj *talker;
    /* 0x44 */ Obj *body; /* its player-shaped model task */
    /* 0x48 */ MPolFrame *shadow;
    /* 0x4C */ f32 animSpeed;
    /* 0x50 */ u8 player;
    /* 0x51 */ u8 chara;
    /* 0x52 */ u16 started;
    /* 0x54 */ u8 pad54;
    /* 0x55 */ u8 set; /* which seats: 1 in stage 0x8D */
    /* 0x56 */ u8 count;
    /* 0x57 */ u8 done;
} ChaW; /* size 0x58 */

#define CHA(obj) ((ChaW *) FX(obj))

/* The tea house ("chamise"), at its task + 0x20: an enemy's work (eneSetup) up to 0x34. */
typedef struct ChamiseW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *seated[4]; /* the seated players' tasks */
    /* 0x44 */ s32 timer;
    /* 0x48 */ u8 unused48;
    /* 0x49 */ u8 chara; /* the character the talker changes to */
    /* 0x4A */ s8 fade; /* frames of the fade: up when changing, down (below 0) when coming in */
    /* 0x4B */ u8 players; /* bit n: player n is in the game */
    /* 0x4C */ u8 charas; /* bit n: character n is in the game (or seated) */
} ChamiseW;

#define CHAMISE(obj) ((ChamiseW *) FX(obj))

/* A player's entry in main's table at 0x80088280 (10 bytes each), from its character on
 * (tlb_0035's). */
typedef struct PlayerSlot {
    /* 0x00 */ s8 chara; /* gPlayer1Chara: -1 for none */
    /* 0x01 */ s8 lives;
    /* 0x02 */ s8 life;
    /* 0x03 */ s8 armor;
    /* 0x04 */ s16 money;
    /* 0x06 */ s8 unused6;
    /* 0x07 */ s8 weaponLevel;
    /* 0x08 */ s8 food;
    /* 0x09 */ u8 unused9;
} PlayerSlot;

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

/* Where each player sits: [set][chara] = x, y, z, the facing (radians). */
extern f32 chaSeats[][4][4];

s32 chaGOE_proc(Obj *o, s16 msg, void *arg);
s32 chaEBI_proc(Obj *o, s16 msg, void *arg);
s32 chaSAS_proc(Obj *o, s16 msg, void *arg);
s32 chaYAE_proc(Obj *o, s16 msg, void *arg);
s32 chaAnimStep(AnimeFrame *f, f32 speed, s32 mode);
void shopTalkStart(Obj *o);
void shopTalkEnd(Obj *o);
void shopDemoFinish(Obj *o);
void roomDollMake(s32 player);
s32 roomTakPlSet(s32 *kinds, s32 n);
void roomDollMake2(s32 player);
void roomDollMakeAll(void);
void yorozuyaGoodsSet(s32 stage, u16 *items);
void YadoSetMusicT(void);
void roomDollKindsSet(void);
void roomDollMakeAll2(void);
void roomCamSetRoom0(Obj *o);
void roomCamSetRoom1(Obj *o);
void roomCamSetRoom2(Obj *o);
void chaDollInit(Obj *o, s32 index, s32 chara);
void chaWait(Obj *o);
void chaTalkStart(Obj *o);
void chaStandUp(Obj *o);
void chaSitDown(Obj *o);
void chaAnimPlay(Obj *o);
void chaDestroy(Obj *o);

/* The dolls' places in the inn room, for one, two, three and four players. */
extern Vec3f innDollPlaces1[1];
extern Vec3f innDollPlaces2[2];
extern Vec3f innDollPlaces3[3];
extern Vec3f innDollPlaces4[4];

/* "steam": the inn's bath steam, at its task + 0x20: an enemy's work up to 0x34. */
typedef struct SteamW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u32 timer;
} SteamW;

/* A shop's counter ("struct SHOP_TABLE_W"), at its task + 0x20: a talking clerk's work
 * (tlb_0109's DemoClerkW) up to 0x44. The clerk stands behind it; talking to it opens the
 * shop's menu, and the goods chosen are shown on it. */
typedef struct ShopTableW {
    /* 0x00 */ u8 state; /* 0 waiting, 1 talked to, 3 the menu */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box;
    /* 0x3C */ u8 flags;
    /* 0x3D */ u8 pad3D[3];
    /* 0x40 */ Obj *talker;
    /* 0x44 */ Obj *item; /* the goods shown */
    /* 0x48 */ u16 timer;
    /* 0x4A */ s16 fade; /* the general store's change: frames up, or down (below 0) coming in */
    /* 0x4C */ u16 goods[3]; /* the goods' kinds */
    /* 0x52 */ u8 unused52;
    /* 0x53 */ s8 choice; /* the menu's choice shown (-2 or -1: none) */
    /* 0x54 */ s8 player; /* the talker's number, from 1 */
    /* 0x55 */ u8 chimed; /* the general store's door chime played */
} ShopTableW;

/* The inn's counter ("YadoyaTable"), at its task + 0x20: a talking clerk's work up to 0x44. */
typedef struct YadoyaW {
    /* 0x00 */ u8 state; /* 0 waiting, 1 talked to, 2 the morning, 3 the talk, 4 the fade */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box;
    /* 0x3C */ u8 flags;
    /* 0x3D */ u8 pad3D[3];
    /* 0x40 */ Obj *talker;
    /* 0x44 */ s32 timer;
    /* 0x48 */ u16 fade; /* frames of the fade out to the room */
    /* 0x4A */ u8 pad4A;
    /* 0x4B */ s8 player;
    /* 0x4C */ u8 chimed; /* the door chime played */
    /* 0x4D */ u8 music; /* frames until the town's tunes come back (0: none) */
} YadoyaW;

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

/* "y_cam", the inn room's camera (its proc roomYadoya_camera): the helpers set its mode byte. */
typedef struct YCamW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Camera *cam; /* the AI camera */
    /* 0x38 */ u8 mode; /* 0-2 a room (set by the helpers), 3 back, 4 moving */
    /* 0x39 */ u8 room;
    /* 0x3A */ u16 timer;
} YCamW;

/* The room camera's places, for each room: where it starts (eye, at), where it moves to, and
 * where it moves back to. */
extern f32 yCamStartEyeX[3];
extern f32 yCamStartEyeY[3];
extern f32 yCamStartEyeZ[3];
extern f32 yCamStartAtX[3];
extern f32 yCamStartAtY[3];
extern f32 yCamStartAtZ[3];
extern f32 yCamToEyeX[3];
extern f32 yCamToEyeY[3];
extern f32 yCamToEyeZ[3];
extern f32 yCamToAtX[3];
extern f32 yCamToAtY[3];
extern f32 yCamToAtZ[3];
extern f32 yCamBackEyeX[3];
extern f32 yCamBackEyeY[3];
extern f32 yCamBackEyeZ[3];
extern f32 yCamBackAtX[3];
extern f32 yCamBackAtY[3];
extern f32 yCamBackAtZ[3];

/* The stage the inn goes back to when the night is over ("GO YADO_END back[%d]"). */
#define GW_INN_BACK gGameWork.backStage

/* "yadoRoom": a night in the inn's room, at its task + 0x20: an enemy's work up to 0x34. */
typedef struct YadoRoomW {
    /* 0x00 */ u8 state; /* 0 the dolls, 1 the night, 2 out */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Obj *cam; /* the room's camera ("y_cam") */
    /* 0x38 */ u16 timer;
} YadoRoomW;

/* "yuge": a puff of the bath's steam, at its task + 0x20: an enemy's work up to 0x2C. */
typedef struct YugeW {
    /* 0x00 */ u8 state; /* 0 growing, 1 fading */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3]; /* [1] eases off */
    /* 0x34 */ Anim anim;
    /* 0x44 */ s32 timer;
    /* 0x48 */ u8 alpha;
} YugeW;

extern u8 roomYadoyaYugePuffDl[]; /* the puff's display list (segment 3 of file 0x123) */

/* The seated players' talks. */
extern PersonData chaGoeTalk;
extern PersonData chaEbiTalk;
extern PersonData chaSasTalk;
extern PersonData chaYaeTalk;

/* The file's .data, in address order (its externs above stay as declarations). */
Vec3f innDollPlaces1[1] = { { 0.1f, 2.0f, 0.0f } };
Vec3f innDollPlaces2[2] = { { -2e+01f, 2.0f, -2e+01f }, { -2e+01f, 2.0f, 2e+01f } };
Vec3f innDollPlaces3[3] = { { -2e+01f, 2.0f, -2e+01f }, { -2e+01f, 2.0f, 2e+01f }, { -4e+01f, 2.0f, -2e+01f } };
Vec3f innDollPlaces4[4] = { { -2e+01f, 2.0f, -2e+01f }, { -4e+01f, 2.0f, -2e+01f }, { -1e+01f, 2.0f, 2e+01f }, { -3e+01f, 2.0f, 2e+01f } };
f32 yCamStartEyeX[3] = { 35.0f, 28.0f, 103.0f };
f32 yCamStartEyeY[3] = { 20.0f, 46.0f, 49.0f };
f32 yCamStartEyeZ[3] = { 21.0f, 32.0f, 11.0f };
f32 yCamStartAtX[3] = { 26.0f, -23.0f, 4.0f };
f32 yCamStartAtY[3] = { 12.0f, -12.0f, 22.0f };
f32 yCamStartAtZ[3] = { -61.0f, -22.0f, -33.0f };
f32 yCamToEyeX[3] = { 55.0f, 37.0f, 110.0f };
f32 yCamToEyeY[3] = { 38.0f, 50.0f, 61.0f };
f32 yCamToEyeZ[3] = { 91.0f, 25.0f, 8.0f };
f32 yCamToAtX[3] = { 32.0f, -67.0f, -59.0f };
f32 yCamToAtY[3] = { 29.0f, -12.0f, 22.0f };
f32 yCamToAtZ[3] = { -26.0f, -12.0f, -13.0f };
f32 yCamBackEyeX[3] = { 83.0f, 88.0f, 85.0f };
f32 yCamBackEyeY[3] = { 61.0f, 59.0f, 49.0f };
f32 yCamBackEyeZ[3] = { 130.0f, 21.0f, 78.0f };
f32 yCamBackAtX[3] = { 47.0f, -57.0f, 8.0f };
f32 yCamBackAtY[3] = { 29.0f, 5.0f, 22.0f };
f32 yCamBackAtZ[3] = { 20.0f, -19.0f, 0.1f };
PersonData chaGoeTalk = { 35, 0x00000000, 0x05, 0x08, 0x00, 0.0f, 0x00 };
PersonData chaEbiTalk = { 36, 0x00000000, 0x05, 0x08, 0x00, 0.0f, 0x00 };
PersonData chaSasTalk = { 37, 0x00000000, 0x05, 0x08, 0x00, 0.0f, 0x00 };
PersonData chaYaeTalk = { 38, 0x00000000, 0x05, 0x08, 0x00, 0.0f, 0x00 };
f32 chaSeats[][4][4] = {
    { { 20.0f, 0.0f, 1.0f, 3.7699113f }, { -34.0f, 0.0f, 18.0f, 2.1991148f }, { -20.0f, 7.0f, 39.0f, 2.5132742f }, { -6.0f, 0.0f, 22.0f, 3.7699113f } },
    { { 0.0f, 0.0f, -38.0f, 0.9424778f }, { 7.0f, 0.0f, -5.0f, 1.5707964f }, { 12.0f, 0.0f, 15.0f, 2.1991148f }, { -8.0f, 0.0f, -25.0f, 2.1991148f } },
};

/* "YadoyaTable" (tableYadoya_proc): the inn's counter. Talked to, the clerk's menu takes a
 * room (flags 0x8B-0x8D: the fade, then the room's stage); in the morning after (flag 0x92)
 * the players in the game (flags 0x8E-0x91) get their pips back (0x8C: one more, 0x8B: all),
 * the day starts again and the inn's tune plays. */
s32 tableYadoya_proc(Obj *o, s16 msg, void *arg) {
    YadoyaW *w = (YadoyaW *) FX(o);
    s32 pad;
    u32 data;
    s32 pad2;
    s32 healed;
    s32 filled;
    s32 pad3;

    switch (msg) {
        case 2:
            o->name = "YadoyaTable";
            data = eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.15f;
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 0, 0, 0);
            demoTalkInit(o, data);
            w->music = 0;
            if (swWorkTest(0x92)) {
                w->chimed++;
                if (swWorkTest(0x8D)) {
                    swWorkReset(0x8D);
                    if (swWorkTest(0x8E)) {
                        swWorkReset(0x8E);
                    }
                    if (swWorkTest(0x8F)) {
                        swWorkReset(0x8F);
                    }
                    if (swWorkTest(0x90)) {
                        swWorkReset(0x90);
                    }
                    if (swWorkTest(0x91)) {
                        swWorkReset(0x91);
                    }
                } else if (swWorkTest(0x8C)) {
                    healed = 0;
                    swWorkReset(0x8C);
                    if (swWorkTest(0x8E)) {
                        swWorkReset(0x8E);
                        if (gSaveData.players[0].life != 3) {
                            healed = 1;
                            gSaveData.players[0].life++;
                        }
                    }
                    if (swWorkTest(0x8F)) {
                        swWorkReset(0x8F);
                        if (gSaveData.players[1].life != 3) {
                            healed++;
                            gSaveData.players[1].life++;
                        }
                    }
                    if (swWorkTest(0x90)) {
                        swWorkReset(0x90);
                        if (gSaveData.players[2].life != 3) {
                            healed++;
                            gSaveData.players[2].life++;
                        }
                    }
                    if (swWorkTest(0x91)) {
                        swWorkReset(0x91);
                        if (gSaveData.players[3].life != 3) {
                            healed++;
                            gSaveData.players[3].life++;
                        }
                    }
                    if (healed) {
                        sndPlay(0x227);
                    }
                } else if (swWorkTest(0x8B)) {
                    filled = 0;
                    swWorkReset(0x8B);
                    if (swWorkTest(0x8E)) {
                        swWorkReset(0x8E);
                        if (gSaveData.players[0].life != 3) {
                            filled = 1;
                        }
                        gSaveData.players[0].life = 3;
                    }
                    if (swWorkTest(0x8F)) {
                        swWorkReset(0x8F);
                        if (gSaveData.players[1].life != 3) {
                            filled++;
                        }
                        gSaveData.players[1].life = 3;
                    }
                    if (swWorkTest(0x90)) {
                        swWorkReset(0x90);
                        if (gSaveData.players[2].life != 3) {
                            filled++;
                        }
                        gSaveData.players[2].life = 3;
                    }
                    if (swWorkTest(0x91)) {
                        swWorkReset(0x91);
                        if (gSaveData.players[3].life != 3) {
                            filled++;
                        }
                        gSaveData.players[3].life = 3;
                    }
                    if (filled) {
                        sndPlay(0x227);
                    }
                }
                gGameWork.time = 0x4B000;
                camSunClearTunes();
                swWorkReset(0x92);
                w->state = 2;
                w->music = 1;
            }
            w->timer = 0;
            if (gGameWork.stage != 0xFA) {
                eneMakeEx(o, ENE_SHOP_YADOYA, 0, 60.0f, 11.0f, -41.0f, 0, NULL);
            } else {
                eneMakeEx(o, ENE_SHOP_TENGU, 0, 60.0f, 11.0f, -41.0f, 0, NULL);
                w->chimed = 1;
            }
            DPRINTF("tableYadoya_proc: clerk NULL!!\n");
            fadeCreate(gEffectTask);
            DPRINTF("tableYadoya_proc: 宿屋テーブルセットＯＫ\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                        gGameWork.scenarioRunning = 2;
                    }
                    break;
                case 1:
                    w->player = 0;
                    scRunStart(w->data->talk, w->player, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    (*shopTalkStart)(o);
                    w->state = 3;
                    break;
                case 2:
                    sndPlay(0x51F);
                    demoPlStandAll();
                    if (gGameWork.stage != 0xFA) {
                        scRunStart(0x2F0, 0, 0);
                    } else {
                        scRunStart(0x2F1, 0, 0);
                    }
                    w->state = 3;
                    break;
                case 3:
                    scGetMenuChoice1();
                    if (!scIsRunning()) {
                        if (swWorkTest(0x8B)) {
                            if (w->fade == 0) {
                                w->state = 4;
                            } else {
                                GW_INN_BACK = gGameWork.stage;
                                sndPlay(0xA);
                                stageGotoSpawnPos(gStageTask->parent, 0x7D, 0, 25.0f, -30.0f, -20.0f);
                            }
                        } else if (swWorkTest(0x8C)) {
                            if (w->fade == 0) {
                                w->state = 4;
                            } else {
                                GW_INN_BACK = gGameWork.stage;
                                sndPlay(0xA);
                                stageGotoSpawnPos(gStageTask->parent, 0x94, 0, 25.0f, -30.0f, -20.0f);
                            }
                        } else if (swWorkTest(0x8D)) {
                            if (w->fade == 0) {
                                w->state = 4;
                            } else {
                                GW_INN_BACK = gGameWork.stage;
                                sndPlay(0xA);
                                stageGotoSpawnPos(gStageTask->parent, 0x95, 0, 25.0f, -30.0f, -20.0f);
                            }
                        } else {
                            (*shopTalkEnd)(o);
                            w->state = 0;
                        }
                        gGameWork.scenarioRunning = 0;
                    }
                    break;
                case 4:
                    if (w->fade == 0) {
                        fadeSetAll(10.0f, 0.0f, 255.0f, 0, 0, 0);
                    }
                    w->fade++;
                    if (w->fade >= 11) {
                        w->state = 3;
                    }
                    break;
                case 5:
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (shimPlNearCheckWorld(o, 25.0f) && w->chimed == 0 && !scIsRunning()) {
                sndPlay(0x312);
                w->chimed++;
            }
            if (w->music != 0) {
                w->music++;
                if (w->music == 8) {
                    YadoSetMusicT();
                    w->music = 0;
                }
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "YadoSetMusicT": unless the inn's tune (0x42F) is playing, the town's four tunes of the day
 * go back for its stage, with the inn's at dawn. */
void YadoSetMusicT(void) {
    if (sndGetMusic() != 0x42F) {
        DPRINTF("YadoSetMusicT : 鳴いてないから鳴く\n");
        switch (gGameWork.stage) {
            case 0x99:
                camSunSetTunes4(0x79, 0x76, 0x77, 0x78);
                camSunSetDawnSound(0x42F);
                break;
            case 0xB4:
                camSunSetTunes4(0x96, 0x93, 0x94, 0x95);
                camSunSetDawnSound(0x42F);
                break;
            case 0xCA:
                camSunSetTunes4(0x84, 0x81, 0x82, 0x83);
                camSunSetDawnSound(0x42F);
                break;
            case 0xE9:
            case 0xEB:
                camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
                camSunSetDawnSound(0x42F);
                break;
            case 0xD9:
                camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
                camSunSetDawnSound(0x42F);
                break;
            case 0xFA:
                camSunSetTunes4(0x30, 0x2D, 0x2E, 0x2F);
                camSunSetDawnSound(0x42F);
                break;
            default:
                DPRINTF("YadoSetMusicT : unknown MUSIC\n");
                break;
        }
    } else {
        DPRINTF("YadoSetMusicT : 鳴いてるから鳴かない\n");
    }
}

/* "yadoRoom" (roomYadoya_proc): the night at the inn. The players' dolls go to bed (by how
 * the room was taken: flags 0x8B-0x8D), the night passes for 170 frames, then the letterbox
 * closes and the stage before goes on (flag 0x92 set). */
s32 roomYadoya_proc(Obj *o, s16 msg, void *arg) {
    s32 pad0[8];
    YadoRoomW *w = (YadoRoomW *) FX(o);
    s32 pad;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "yadoRoom";
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            fadeCreate(gEffectTask);
            fadeSetAll(30.0f, 255.0f, 0.0f, 0, 0, 0);
            w->cam = eneMakeEx(o, ENE_ROOM_YADOYA_CAMERA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
            if (w->cam == NULL) {
                DPRINTF("dcam is NULL!\n");
            }
            camAiSetHold(1);
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    ((Player *) (gGameWork.players[i] + 1))->flags |= 0x40;
                    ((Player *) (gGameWork.players[i] + 1))->flags2 |= 1;
                }
            }
            sndPlay(1);
            if (swWorkTest(0x8B)) {
                roomCamSetRoom2(w->cam);
            } else if (swWorkTest(0x8C)) {
                roomCamSetRoom1(w->cam);
            } else if (swWorkTest(0x8D)) {
                roomCamSetRoom0(w->cam);
            }
            gaugeSetOn(0);
            DPRINTF("roomYadoya_proc: 宿部屋監視セットＯＫ\n");
            gGameWork.scenarioRunning = 2;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->cam != NULL) {
                tskDestroy(w->cam);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (swWorkTest(0x8B)) {
                        roomDollMakeAll();
                    } else if (swWorkTest(0x8C)) {
                        roomDollKindsSet();
                    } else if (swWorkTest(0x8D)) {
                        roomDollMakeAll2();
                    }
                    w->timer = 0;
                    w->state = 1;
                    break;
                case 1:
                    if (w->timer == 3) {
                        if (swWorkTest(0x8B)) {
                            sndPlay(0xC1);
                        } else if (swWorkTest(0x8C)) {
                            sndPlay(0x1B5);
                        } else if (swWorkTest(0x8D)) {
                            sndPlay(0x1B6);
                        }
                    }
                    if (w->timer >= 170) {
                        swWorkSet(0x92);
                        w->state = 2;
                        sndPlay(0x51D);
                    }
                    break;
                case 2:
                    if (fadeCloseLetterbox(120) == 1) {
                        DPRINTF("GO YADO_END back[%d]\n", GW_INN_BACK);
                        stageGotoSpawnNoState(gStageTask->parent, GW_INN_BACK, 0, 52.0f, 0.0f, -7.0f,
                                              3.455752134f);
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A player's doll in the inn room, by its character (kinds 0x1A9-0x1AC). */
void roomDollMake(s32 player) {
    switch (plGetChara(gGameWork.players[player])) {
        case 0:
            eneMakeEx(NULL, ENE_SHOP_MAT_G_RUN, 0, 18.0f, 20.0f, -38.0f, 0, NULL);
            break;
        case 1:
            eneMakeEx(NULL, ENE_SHOP_MAT_E, 0, 38.0f, 22.0f, 38.0f, 0, NULL);
            break;
        case 2:
            eneMakeEx(NULL, ENE_SHOP_MAT_S, 0, -42.0f, 20.0f, -2.0f, 0, NULL);
            break;
        case 3:
            eneMakeEx(NULL, ENE_SHOP_MAT_Y, 0, 38.0f, 22.0f, 50.0f, 0, NULL);
            break;
    }
}

/* Each player in the game (flags 0x8E-0x91) gets its own. */
void roomDollMakeAll(void) {
    if (swWorkTest(0x8E)) {
        roomDollMake(0);
    }
    if (swWorkTest(0x8F)) {
        roomDollMake(1);
    }
    if (swWorkTest(0x90)) {
        roomDollMake(2);
    }
    if (swWorkTest(0x91)) {
        roomDollMake(3);
    }
}

/* "roomTakPlSet": the players' dolls (kinds) on their futons, the places by how many there
 * are; the first kind takes the last place. */
s32 roomTakPlSet(s32 *kinds, s32 n) {
    s32 i;
    s32 left = n;

    for (i = 0; i < 4; i++) {
        if (kinds[i] != 0) {
            DPRINTF("roomTakPlSet: Ekind[%d] = %d\n", i, kinds[i]);
            left--;
            switch (n) {
                case 1:
                    eneMakeEx(NULL, kinds[i], 0, innDollPlaces1[0].x, innDollPlaces1[0].y,
                              innDollPlaces1[0].z, 0, NULL);
                    break;
                case 2:
                    eneMakeEx(NULL, kinds[i], 0, innDollPlaces2[left].x, innDollPlaces2[left].y,
                              innDollPlaces2[left].z, 0, NULL);
                    break;
                case 3:
                    eneMakeEx(NULL, kinds[i], 0, innDollPlaces3[left].x, innDollPlaces3[left].y,
                              innDollPlaces3[left].z, 0, NULL);
                    break;
                case 4:
                    eneMakeEx(NULL, kinds[i], 0, innDollPlaces4[left].x, innDollPlaces4[left].y,
                              innDollPlaces4[left].z, 0, NULL);
                    break;
            }
            if (left == 0) {
                return 0;
            }
        }
    }
    return 0;
}

/* A character's doll enemy kind. */
s32 roomDollKind(s32 chara) {
    switch (chara) {
        case 0:
            return 0x1AD;
        case 1:
            return 0x1AE;
        case 2:
            return 0x1AF;
        case 3:
            return 0x1B0;
        default:
            return 0;
    }
}

/* The players in the game (flags 0x8E-0x91): their dolls' kinds, to the room. */
void roomDollKindsSet(void) {
    s32 i;
    s32 n = 0;
    s32 kinds[4];

    for (i = 0; i < 4; i++) {
        kinds[i] = 0;
    }
    if (swWorkTest(0x8E)) {
        n++;
        kinds[0] = roomDollKind(plGetChara(gGameWork.players[0]));
    }
    if (swWorkTest(0x8F)) {
        n++;
        kinds[1] = roomDollKind(plGetChara(gGameWork.players[1]));
    }
    if (swWorkTest(0x90)) {
        n++;
        kinds[2] = roomDollKind(plGetChara(gGameWork.players[2]));
    }
    if (swWorkTest(0x91)) {
        n++;
        kinds[3] = roomDollKind(plGetChara(gGameWork.players[3]));
    }
    roomTakPlSet(kinds, n);
}

/* A player's doll in the inn room's second set, by its character (kinds 0x1B1-0x1B4). */
void roomDollMake2(s32 player) {
    switch (plGetChara(gGameWork.players[player])) {
        case 0:
            eneMakeEx(NULL, ENE_SHOP_UME_G, 0, 32.0f, 2.0f, -7.0f, 0, NULL);
            break;
        case 1:
            eneMakeEx(NULL, ENE_SHOP_UME_E, 0, 6.0f, 2.0f, -28.0f, 0, NULL);
            break;
        case 2:
            eneMakeEx(NULL, ENE_SHOP_UME_S, 0, -13.0f, 2.0f, -5.0f, 0, NULL);
            break;
        case 3:
            eneMakeEx(NULL, ENE_SHOP_UME_Y, 0, 10.0f, 2.0f, -10.0f, 0, NULL);
            break;
    }
}

/* Each player in the game (flags 0x8E-0x91) gets its own. */
void roomDollMakeAll2(void) {
    if (swWorkTest(0x8E)) {
        roomDollMake2(0);
    }
    if (swWorkTest(0x8F)) {
        roomDollMake2(1);
    }
    if (swWorkTest(0x90)) {
        roomDollMake2(2);
    }
    if (swWorkTest(0x91)) {
        roomDollMake2(3);
    }
}

/* "y_cam" (roomYadoya_camera): the inn room's camera. It flies to its room's place, back after
 * 85 frames; in the third room sparkles fall. */
s32 roomYadoya_camera(Obj *o, s16 msg, void *arg) {
    YCamW *w = (YCamW *) FX(o);
    CamView *v;
    s32 pad[7];
    s32 room;
    s32 r;
    f32 s;

    switch (msg) {
        case 2:
            o->name = "y_cam";
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 1.0f;
            w->cam = camAiGetCam();
            w->timer = 0;
            DPRINTF("roomYadoya_camera セット完了\n");
            break;
        case 0:
            eneStepHookPre(o);
            v = camGetView(w->cam);
            v->zNear = 20.0f;
            v->zFar = 10000.0f;
            switch (w->mode) {
                case 0:
                case 1:
                case 2:
                    DPRINTF("DEFAULT\n");
                    room = w->room = w->mode;
                    v->eye[0] = yCamStartEyeX[room];
                    v->eye[1] = yCamStartEyeY[room];
                    v->eye[2] = yCamStartEyeZ[room];
                    v->at[0] = yCamStartAtX[room];
                    v->at[1] = yCamStartAtY[room];
                    v->at[2] = yCamStartAtZ[room];
                    DPRINTF("pos(%d,%d,%d)->ipos(%d,%d,%d) view(%f)\n", (s32) v->eye[0], (s32) v->eye[1],
                            (s32) v->eye[2], (s32) v->at[0], (s32) v->at[1], (s32) v->at[2], v->fovy);
                    camStop(w->cam);
                    camSetSpeed(w->cam, 85.0f);
                    camMoveEye(w->cam, yCamToEyeX[room], yCamToEyeY[room],
                               yCamToEyeZ[room]);
                    camMoveAt(w->cam, yCamToAtX[room], yCamToAtY[room],
                              yCamToAtZ[room]);
                    w->mode = 4;
                    break;
                case 3:
                    room = w->room;
                    DPRINTF("SET2 go\n");
                    camStop(w->cam);
                    camSetSpeed(w->cam, 85.0f);
                    camMoveEye(w->cam, yCamBackEyeX[room], yCamBackEyeY[room],
                               yCamBackEyeZ[room]);
                    camMoveAt(w->cam, yCamBackAtX[room], yCamBackAtY[room],
                              yCamBackAtZ[room]);
                    w->mode = 4;
                    break;
                case 4:
                    if (w->timer == 85) {
                        w->mode = 3;
                    }
                    break;
                default:
                    DPRINTF("roomYadoya_camera CRAZY\n");
                    break;
            }
            if (w->room == 2 && w->timer % 8 == 2) {
                r = fadeRand(4);
                s = fadeRandRange(2, 5) / 100.0f;
                switch (r) {
                    case 0:
                        itexSparkle(22.0f, 18.0f, -10.0f, s);
                        break;
                    case 1:
                        itexSparkle(15.0f, 18.0f, -30.0f, s);
                        break;
                    case 2:
                        itexSparkle(41.0f, 18.0f, -20.0f, s);
                        break;
                    case 3:
                        itexSparkle(35.0f, 18.0f, -35.0f, s);
                        break;
                }
            }
            w->timer++;
            eneStepHookPost(o);
            break;
        case 1:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void roomUnusedStub1(void) {
}

/* The room camera's helpers set its mode byte (the room). */
void roomCamSetRoom0(Obj *o) {
    ((YCamW *) FX(o))->mode = 0;
}

void roomCamSetRoom1(Obj *o) {
    ((YCamW *) FX(o))->mode = 1;
}

void roomCamSetRoom2(Obj *o) {
    ((YCamW *) FX(o))->mode = 2;
}

void roomUnusedStub2(void) {
}

/* "steam": puffs of steam (kind 0xFD), one to three of every 30 frames. */
s32 roomYadoya_steam_proc(Obj *o, s16 msg, void *arg) {
    s32 i;
    SteamW *w = (SteamW *) FX(o);
    s32 n;

    switch (msg) {
        case 2:
            o->name = "steam";
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            w->timer = 0;
            break;
        case 0:
            n = fadeRandRange(1, 4);
            if (w->timer % 30 < n) {
                for (i = 0; i < 1; i++) {
                    eneMakeEx(NULL, ENE_ROOM_YADOYA_YUGE, 0, w->x, w->y, w->z, 0, NULL);
                }
            }
            w->timer++;
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "yuge": a puff of steam. It grows for 8 frames, then fades out with its animation. */
s32 roomYadoya_yuge_proc(Obj *o, s16 msg, void *arg) {
    YugeW *w = (YugeW *) FX(o);
    f32 s;

    switch (msg) {
        case 2:
            o->name = "yuge";
            eneSetup(o, arg);
            w->timer = 8;
            w->model = enePolModelMake(o, (Gfx *) roomYadoyaYugePuffDl, NULL, 1);
            if (w->model == NULL) {
                DPRINTF("roomYadoya_yuge_proc :frame is NULL\n");
                return -1;
            }
            floadGetSetRam((void **) w->model->segs, 0x123);
            w->model->flags |= 7;
            s = fadeRandRange(1, 3) / 100.0f;
            w->model->scale[2] = s;
            w->model->scale[1] = s;
            w->model->scale[0] = s;
            fadeAnimStart(&w->anim, gItexPuffAnim, 0, &w->model->segs[3], floadGetSetFile(0x123, 0));
            w->vel[1] = 0.3f;
            w->state = 0;
            w->alpha = 0xFF;
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (--w->timer <= 0) {
                        w->state = 1;
                    }
                    w->model->scale[0] *= 1.06;
                    w->model->scale[1] *= 1.04;
                    w->model->scale[2] *= 1.06;
                    break;
                case 1:
                    w->vel[1] *= 0.95;
                    if (fadeAnimStep(&w->anim)) {
                        return -1;
                    }
                    w->alpha -= 9;
                    if (w->alpha < 10) {
                        w->alpha = 10;
                    }
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            break;
        case 3:
            w->model->dl = eneDlEnvPrimColor(o, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, w->alpha);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "YorozuTable" (tableYorozuya_proc): the general store's counter. Its goods come from
 * yorozuyaGoodsSet; in the last town (stage 0xF5) another clerk stands there, and with flag 0x137
 * set the talk ends in a change (a fade and the stage again). */
s32 tableYorozuya_proc(Obj *o, s16 msg, void *arg) {
    s32 pad[2];
    ShopTableW *w = (ShopTableW *) FX(o);
    s32 i; /* also its data, and the menu's choice */
    Obj *clerk;

    switch (msg) {
        case 2:
            o->name = "YorozuTable";
            i = eneSetup(o, arg);
            DPRINTF("struct SHOP_TABLE_W [%d]\n", sizeof(ShopTableW));
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.4f;
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 0, 0, 0);
            demoTalkInit(o, i);
            yorozuyaGoodsSet(gGameWork.stage, w->goods);
            w->timer = 0;
            if (gGameWork.stage != 0xF5) {
                clerk = eneMakeEx(o, ENE_SHOP_YOROZU_DEBU, 0, w->x, w->y, w->z, 0, NULL);
            } else {
                clerk = eneMakeEx(o, ENE_SHOP_TENGU, 0, w->x, w->y, w->z, 0, NULL);
                w->chimed = 1;
            }
            if (clerk == NULL) {
                DPRINTF("tableYorozu_proc: clerk NULL!!\n");
                return -1;
            }
            floadLoadSet(0x3F);
            if (swWorkTest(0x137)) {
                swWorkReset(0x137);
                fadeSetAll(15.0f, 255.0f, 0.0f, 0xFF, 0x80, 0x80);
                w->fade = -17;
            }
            DPRINTF("tableYorozuya_proc: 万屋テーブルセットＯＫ\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                        gGameWork.scenarioRunning = 2;
                    }
                    break;
                case 1:
                    w->player = plGetIndex(w->talker) + 1;
                    scRunStart(w->data->talk, w->player, plGetChara(w->talker));
                    (*shopTalkStart)(o);
                    w->state = 2;
                    break;
                case 2:
                    i = scGetMenuChoice1();
                    if (i != w->choice && i != -1 && swWorkTest(0x12C)) {
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                        switch (i) {
                            case 0:
                                w->item = eneMakeEx(o, w->goods[0], 0, 0.0f, 0.0f, 0.0f, 2,
                                                    (u32 (*)(void *, void *, void *)) 1);
                                break;
                            case 1:
                                /* FAKE: three spellings of zero, so IDO keeps them apart (IDO_NOTES 365) */
                                w->item = eneMakeEx(o, w->goods[1], 0, 0.0f, 0.00f, 0.000f, 2,
                                                    (u32 (*)(void *, void *, void *)) 2);
                                break;
                            case 2:
                                w->item = eneMakeEx(o, w->goods[2], 0, 0, 0, 0, 2,
                                                    (u32 (*)(void *, void *, void *)) 3);
                                break;
                        }
                    }
                    w->choice = i;
                    if (!swWorkTest(0x12E)) {
                        w->choice = -1;
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                    }
                    if (swWorkTest(0x12D)) {
                        w->choice = -1;
                    }
                    if (!scIsRunning()) {
                        (*shopTalkEnd)(o);
                        w->state = 0;
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                        gGameWork.scenarioRunning = 0;
                        if (swWorkTest(0x137)) {
                            DPRINTF("ここでちぇんじ！\n");
                            fadeSetAll(15.0f, 0.0f, 255.0f, 0xFF, 0x80, 0x80);
                            w->fade = 17;
                            sndPlay(0x161);
                            for (i = 0; i < 4; i++) {
                                if (plIsInPlay(i)) {
                                    ((Player *) (gGameWork.players[i] + 1))->flags |= 0x40;
                                }
                            }
                        }
                    }
                    break;
            }
            if (w->fade > 0) {
                if (--w->fade == 0) {
                    stageGotoSpawnNoSave(gStageTask->parent, gGameWork.stage, 0, 36.0f, 0.0f, 17.0f,
                                         1.256637096f, 0);
                    DPRINTF("tableYorozuya_proc: UNKNOWN AREA!!!\n");
                }
            } else if (w->fade < 0) {
                if (++w->fade == 0) {
                    DPRINTF("tableYorozuya_proc: UNKNOWN AREA!!!\n");
                    w->fade = 0;
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (w->timer == 15 && w->fade == 0 && w->chimed == 0) {
                sndPlay(0x313);
                w->chimed++;
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The general store's three goods in each town (by stage); once flag 0x135 is set, fewer. */
void yorozuyaGoodsSet(s32 stage, u16 *items) {
    if (!swWorkTest(0x135)) {
        switch (stage) {
            case 0x96:
                items[0] = 0x10A;
                items[1] = 0x10B;
                items[2] = 0x107;
                break;
            case 0xB8:
                items[0] = 0x10B;
                items[1] = 0x107;
                items[2] = 0x108;
                break;
            case 0xC8:
                items[0] = 0x10A;
                items[1] = 0x10C;
                items[2] = 0x108;
                break;
            case 0xE7:
                items[0] = 0x10A;
                items[1] = 0x10B;
                items[2] = 0x10C;
                break;
            case 0xEA:
                items[0] = 0x10D;
                items[1] = 0x107;
                items[2] = 0x109;
                break;
            case 0xDA:
                items[0] = 0x10D;
                items[1] = 0x10C;
                items[2] = 0x109;
                break;
            case 0xF5:
                items[0] = 0x10D;
                items[1] = 0x10C;
                items[2] = 0x109;
                break;
        }
    } else {
        switch (stage) {
            case 0x96:
                items[0] = 0;
                items[1] = 0;
                items[2] = 0x107;
                break;
            case 0xB8:
                items[0] = 0;
                items[1] = 0x107;
                items[2] = 0x108;
                break;
            case 0xC8:
                items[0] = 0;
                items[1] = 0x10C;
                items[2] = 0x108;
                break;
            case 0xE7:
                items[0] = 0;
                items[1] = 0x10B;
                items[2] = 0x10C;
                break;
            case 0xEA:
                items[0] = 0;
                items[1] = 0x107;
                items[2] = 0x109;
                break;
            case 0xDA:
                items[0] = 0;
                items[1] = 0;
                items[2] = 0;
                break;
            case 0xF5:
                items[0] = 0;
                items[1] = 0;
                items[2] = 0;
                break;
        }
    }
}

/* "GansoTable" (tableGanso_proc): the Ganso dango shop's counter, in six towns. */
s32 tableGanso_proc(Obj *o, s16 msg, void *arg) {
    ShopTableW *w = (ShopTableW *) FX(o);
    s32 pad;
    s32 data; /* also the menu's choice */
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "GansoTable";
            data = eneSetup(o, arg);
            DPRINTF("sizeof struct SHOP_TABLE_W [%d]\n", sizeof(ShopTableW));
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.2f;
            w->rot[1] += 3.1415927f;
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("tableGanso_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            switch (gGameWork.stage) {
                case 0x97:
                    w->goods[0] = 0x10E;
                    w->goods[1] = 0x10F;
                    w->goods[2] = 0x110;
                    break;
                case 0xB5:
                    w->goods[0] = 0x114;
                    w->goods[1] = 0x115;
                    w->goods[2] = 0x116;
                    break;
                case 0xCB:
                    w->goods[0] = 0x117;
                    w->goods[1] = 0x118;
                    w->goods[2] = 0x119;
                    break;
                case 0xE8:
                    w->goods[0] = 0x11A;
                    w->goods[1] = 0x11B;
                    w->goods[2] = 0x11C;
                    break;
                case 0xE3:
                    w->goods[0] = 0x120;
                    w->goods[1] = 0x121;
                    w->goods[2] = 0x122;
                    break;
                case 0xF7:
                    w->goods[0] = 0x11D;
                    w->goods[1] = 0x11E;
                    w->goods[2] = 0x11F;
                    break;
                default:
                    DPRINTF("tableGanso_proc: UNKNOWN STAGE!!!\n");
                    break;
            }
            w->timer = 0;
            w->state = 0;
            DPRINTF("tableGanso_proc: 元祖テーブルセットＯＫ\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->item != NULL) {
                tskDestroy(w->item);
            }
            demoTalkerFree(o);
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                        gGameWork.scenarioRunning = 2;
                    }
                    break;
                case 1:
                    w->player = plGetIndex(w->talker) + 1;
                    scRunStart(w->data->talk, w->player, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    (*shopTalkStart)(o);
                    w->state = 3;
                    break;
                case 3:
                    data = scGetMenuChoice1();
                    if (data != w->choice && data != -1 && swWorkTest(0x12C)) {
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                        switch (data) {
                            case 0:
                                w->item = eneMakeEx(o, w->goods[0], 0, 0.0f, 0.0f, 0.0f, 2,
                                                    (u32 (*)(void *, void *, void *)) 1);
                                break;
                            case 1:
                                w->item = eneMakeEx(o, w->goods[1], 0, 0.0f, 0.0f, 0.0f, 2,
                                                    (u32 (*)(void *, void *, void *)) 2);
                                break;
                            case 2:
                                w->item = eneMakeEx(o, w->goods[2], 0, 0.0f, 0.0f, 0.0f, 2,
                                                    (u32 (*)(void *, void *, void *)) 3);
                                break;
                        }
                    }
                    w->choice = data;
                    if (!swWorkTest(0x12E)) {
                        w->choice = -2;
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                    }
                    if (swWorkTest(0x12D)) {
                        w->choice = -2;
                    }
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 0;
                        (*shopTalkEnd)(o);
                        w->state = 0;
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                    }
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "HonkeTable" (tableHonke_proc): the Honke dango shop's counter, in two towns. */
s32 tableHonke_proc(Obj *o, s16 msg, void *arg) {
    ShopTableW *w = (ShopTableW *) FX(o);
    s32 pad;
    s32 data; /* also the menu's choice */
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "HonkeTable";
            data = eneSetup(o, arg);
            DPRINTF("sizeof struct SHOP_TABLE_W [%d]\n", sizeof(ShopTableW));
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.4f;
            w->rot[1] += 3.1415927f;
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 0, 0, 0);
            demoTalkInit(o, data);
            switch (gGameWork.stage) {
                case 0x98:
                    w->goods[0] = 0x111;
                    w->goods[1] = 0x112;
                    w->goods[2] = 0x113;
                    break;
                case 0xD8:
                    w->goods[0] = 0x123;
                    w->goods[1] = 0x124;
                    w->goods[2] = 0x125;
                    break;
                default:
                    DPRINTF("tableHonke_proc: UNKNOWN STAGE!!!\n");
                    break;
            }
            w->timer = 0;
            w->state = 0;
            if (eneMakeEx(o, ENE_SHOP_BABA, 0, w->x, w->y, w->z, 0, NULL) == NULL) {
                DPRINTF("tableHonke_proc: clerk NULL!!\n");
                return -1;
            }
            DPRINTF("tableHonke_proc: 本家テーブルセットＯＫ\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->item != NULL) {
                tskDestroy(w->item);
            }
            demoTalkerFree(o);
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                        gGameWork.scenarioRunning = 2;
                    }
                    break;
                case 1:
                    w->player = plGetIndex(w->talker) + 1;
                    scRunStart(w->data->talk, w->player, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    (*shopTalkStart)(o);
                    w->state = 3;
                    break;
                case 3:
                    data = scGetMenuChoice1();
                    if (data != w->choice && data != -1 && swWorkTest(0x12C)) {
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                        switch (data) {
                            case 0:
                                w->item = eneMakeEx(o, w->goods[0], 0, 0.0f, 0.0f, 0.0f, 2,
                                                    (u32 (*)(void *, void *, void *)) 1);
                                break;
                            case 1:
                                w->item = eneMakeEx(o, w->goods[1], 0, 0.0f, 0.0f, 0.0f, 2,
                                                    (u32 (*)(void *, void *, void *)) 2);
                                break;
                            case 2:
                                w->item = eneMakeEx(o, w->goods[2], 0, 0.0f, 0.0f, 0.0f, 2,
                                                    (u32 (*)(void *, void *, void *)) 3);
                                break;
                        }
                    }
                    w->choice = data;
                    if (!swWorkTest(0x12E)) {
                        w->choice = -2;
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                    }
                    if (swWorkTest(0x12D)) {
                        w->choice = -2;
                    }
                    if (!scIsRunning()) {
                        (*shopTalkEnd)(o);
                        w->state = 0;
                        if (w->item != NULL) {
                            w->item = tskDestroy(w->item);
                        }
                        gGameWork.scenarioRunning = 0;
                    }
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "chaGOE": Goemon at the tea house, sitting and drinking; he talks, and stands up when flag
 * 0x130 is set. */
s32 chaGOE_proc(Obj *o, s16 msg, void *arg) {
    ChaW *w = CHA(o);
    s32 pad[4];
    Player *p;

    switch (msg) {
        case 2:
            DPRINTF("goecreate\n");
            DPRINTF("struct DOLL_PLAYER size[%d]\n", sizeof(ChaW));
            o->name = "chaGOE";
            chaDollInit(o, ((ChaW *) arg)->player, 0);
            w->data = (PersonData *) fadeSegToRam(&chaGoeTalk, o->file);
            w->player = ((ChaW *) arg)->player;
            w->set = ((ChaW *) arg)->set;
            w->chara = 0;
            break;
        case 1:
            chaDestroy(o);
            break;
        case 8:
            return demoTalk(o, arg);
        case 0:
            switch (w->state) {
                case 0:
                    chaWait(o);
                    p = PL(w->body);
                    if (w->started == 0) {
                        p->action = 0;
                        plAnimeSet(p, p->action);
                        plFaceAnimeStart(p);
                        w->count = 0;
                        w->animSpeed = 0.4f;
                    }
                    w->started = 1;
                    if (chaAnimStep(w->model, w->animSpeed, 1) == 1) {
                        w->count++;
                        if (w->count < 6) {
                            p->action = 0;
                            w->animSpeed = 0.4f;
                        } else if (w->count == 6) {
                            p->action = 0x70;
                            w->animSpeed = 0.4f;
                        } else if (w->count < 11) {
                            p->action = 0x71;
                            w->animSpeed = 0.4f;
                        } else if (w->count == 11) {
                            p->action = 0x72;
                            w->animSpeed = 0.6f;
                        } else {
                            p->action = 0;
                            w->animSpeed = 0.4f;
                            w->count = 0;
                        }
                        plAnimeSet(p, p->action);
                        plFaceAnimeStart(p);
                    }
                    break;
                case 1:
                    chaTalkStart(o);
                    break;
                case 2:
                    chaStandUp(o);
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x130)) {
                            DPRINTF("GOE CHANGE!!!\n");
                            w->done = 1;
                            w->state = 4;
                        }
                        chaSitDown(o);
                    }
                    break;
                case 4:
                    chaAnimPlay(o);
                    break;
                default:
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->x, w->y, w->z, 1.0f, NULL);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "chaEBI": Ebisumaru at the tea house, sitting and eating; he talks, and with flag 0x131
 * set he is done. */
s32 chaEBI_proc(Obj *o, s16 msg, void *arg) {
    ChaW *w = CHA(o);
    s32 pad[4];
    Player *p;

    switch (msg) {
        case 2:
            DPRINTF("ebicreate\n");
            o->name = "chaEBI";
            chaDollInit(o, ((ChaW *) arg)->player, 1);
            w->data = (PersonData *) fadeSegToRam(&chaEbiTalk, o->file);
            w->player = ((ChaW *) arg)->player;
            w->set = ((ChaW *) arg)->set;
            w->chara = 1;
            break;
        case 1:
            chaDestroy(o);
            break;
        case 8:
            return demoTalk(o, arg);
        case 0:
            switch (w->state) {
                case 0:
                    chaWait(o);
                    p = PL(w->body);
                    if (w->started == 0) {
                        p->action = 0;
                        plAnimeSet(p, p->action);
                        plFaceAnimeStart(p);
                        w->animSpeed = 0.3f;
                        w->count = 0;
                    }
                    w->started = 1;
                    if (chaAnimStep(w->model, w->animSpeed, 1) == 1) {
                        w->count++;
                        if (w->count < 2) {
                            p->action = 0;
                            w->animSpeed = 0.3f;
                        } else if (w->count == 2) {
                            p->action = 0x70;
                            w->animSpeed = 0.55f;
                        } else {
                            p->action = 0x71;
                            w->animSpeed = 0.4f;
                            w->count = 3;
                        }
                        plAnimeSet(p, p->action);
                        plFaceAnimeStart(p);
                    }
                    break;
                case 1:
                    chaTalkStart(o);
                    break;
                case 2:
                    chaStandUp(o);
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x131)) {
                            DPRINTF("EBI CHANGE!!!\n");
                            w->done = 1;
                        }
                        chaSitDown(o);
                    }
                    break;
                case 4:
                    chaAnimPlay(o);
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->x, w->y, w->z, 1.0f, NULL);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "chaSAS": Sasuke at the tea house; with flag 0x132 set he is done. */
s32 chaSAS_proc(Obj *o, s16 msg, void *arg) {
    ChaW *w = CHA(o);
    s32 pad[4];
    Player *p;

    switch (msg) {
        case 2:
            DPRINTF("sascreate\n");
            DPRINTF("struct DOLL_PLAYER size[%d]\n", sizeof(ChaW));
            o->name = "chaSAS";
            chaDollInit(o, ((ChaW *) arg)->player, 2);
            w->data = (PersonData *) fadeSegToRam(&chaSasTalk, o->file);
            w->player = ((ChaW *) arg)->player;
            w->set = ((ChaW *) arg)->set;
            w->chara = 2;
            break;
        case 1:
            chaDestroy(o);
            break;
        case 8:
            return demoTalk(o, arg);
        case 0:
            switch (w->state) {
                case 0:
                    chaWait(o);
                    p = PL(w->body);
                    if (w->started == 0) {
                        p->action = 0;
                        plAnimeSet(p, p->action);
                        plFaceAnimeStart(p);
                        w->animSpeed = 0.3f;
                        w->count = 0;
                    }
                    w->started = 1;
                    if (chaAnimStep(w->model, w->animSpeed, 1) == 1) {
                        w->count++;
                        if (w->count < 2) {
                            p->action = 0;
                            w->animSpeed = 0.4f;
                        } else if (w->count == 2) {
                            p->action = 0x70;
                            w->animSpeed = 0.3f;
                        } else {
                            p->action = 0x71;
                            w->animSpeed = 0.2f;
                            w->count = 3;
                        }
                        plAnimeSet(p, p->action);
                        plFaceAnimeStart(p);
                    }
                    break;
                case 1:
                    chaTalkStart(o);
                    break;
                case 2:
                    chaStandUp(o);
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x132)) {
                            DPRINTF("SAS CHANGE!!!\n");
                            w->done = 1;
                        }
                        chaSitDown(o);
                    }
                    break;
                case 4:
                    chaAnimPlay(o);
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->x, w->y, w->z, 1.0f, NULL);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "chaYAE": Yae at the tea house; with flag 0x133 set she is done. */
s32 chaYAE_proc(Obj *o, s16 msg, void *arg) {
    ChaW *w = CHA(o);
    s32 pad[4];
    Player *p;

    switch (msg) {
        case 2:
            DPRINTF("yaecreate\n");
            DPRINTF("struct DOLL_PLAYER size[%d]\n", sizeof(ChaW));
            o->name = "chaYAE";
            chaDollInit(o, ((ChaW *) arg)->player, 3);
            w->data = (PersonData *) fadeSegToRam(&chaYaeTalk, o->file);
            w->player = ((ChaW *) arg)->player;
            w->set = ((ChaW *) arg)->set;
            w->chara = 3;
            break;
        case 1:
            chaDestroy(o);
            break;
        case 8:
            return demoTalk(o, arg);
        case 0:
            switch (w->state) {
                case 0:
                    chaWait(o);
                    p = PL(w->body);
                    if (w->started == 0) {
                        p->action = 0;
                        plAnimeSet(p, p->action);
                        plFaceAnimeStart(p);
                        w->animSpeed = 0.4f;
                        w->count = 0;
                    }
                    w->started++;
                    if (chaAnimStep(w->model, w->animSpeed, 1) == 1) {
                        w->count++;
                        if (w->count < 2) {
                            p->action = 0;
                            w->animSpeed = 0.4f;
                        } else if (w->count == 2) {
                            if (!(fadeRand(10) & 1)) {
                                p->action = 0x70;
                                w->animSpeed = 0.5f;
                            } else {
                                p->action = 0x71;
                                w->animSpeed = 0.3f;
                            }
                        } else {
                            p->action = 0;
                            w->animSpeed = 0.4f;
                            w->count = 0;
                        }
                        plAnimeSet(p, p->action);
                        plFaceAnimeStart(p);
                    }
                    break;
                case 1:
                    chaTalkStart(o);
                    break;
                case 2:
                    chaStandUp(o);
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x133)) {
                            DPRINTF("YAE CHANGE!!!\n");
                            w->done = 1;
                        }
                        chaSitDown(o);
                    }
                    break;
                case 4:
                    chaAnimPlay(o);
                    break;
                default:
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->x, w->y, w->z, 1.0f, NULL);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Steps a model's animation by speed. Mode 0 loops and 1 stops at the end, 2 loops and 3
 * stops at the start, going backwards; 1 when it wrapped or stopped. */
s32 chaAnimStep(AnimeFrame *f, f32 speed, s32 mode) {
    if (mode == 0 || mode == 1) {
        f->frame += speed;
        if (polAnimeLastFrame(f) < f->frame) {
            if (mode == 0) {
                f->frame -= polAnimeLastFrame(f);
            } else {
                f->frame = polAnimeLastFrame(f);
            }
            return 1;
        }
    } else if (mode == 2 || mode == 3) {
        f->frame -= speed;
        if (f->frame < 0.0f) {
            if (mode == 2) {
                f->frame += polAnimeLastFrame(f);
            } else {
                f->frame = 0.0f;
            }
            return 1;
        }
    }
    return 0;
}

/* A seated player to its seat. */
void chaSeatSet(Obj *o) {
    ChaW *w = CHA(o);
    f32 *seat;

    w->zone = 0;
    seat = chaSeats[w->set][w->chara];
    w->x = seat[0];
    w->y = seat[1];
    w->z = seat[2];
    w->rot[1] = seat[3];
}

/* A player in the game to its seat. */
void chaPlayerToSeat(Obj *player, s32 set, s32 chara) {
    Player *p = PL(player);
    f32 *seat;

    plSetAction(player, 0);
    seat = chaSeats[set][chara];
    p->place = seat[0];
    p->height = seat[1];
    p->side = seat[2];
    p->zone = 0;
}

/* The seated players' tasks, under the enemy manager's fourth child. */
Obj *chaGOE_create(Obj *o, s32 player, s32 set) {
    ChaW arg;

    arg.player = player;
    arg.set = set;
    return tskCreate(gEneManager->child->next->next->next, chaGOE_proc, o->file, &arg);
}

Obj *chaEBI_create(Obj *o, s32 player, s32 set) {
    ChaW arg;

    arg.player = player;
    arg.set = set;
    return tskCreate(gEneManager->child->next->next->next, chaEBI_proc, o->file, &arg);
}

Obj *chaSAS_create(Obj *o, s32 player, s32 set) {
    ChaW arg;

    arg.player = player;
    arg.set = set;
    return tskCreate(gEneManager->child->next->next->next, chaSAS_proc, o->file, &arg);
}

Obj *chaYAE_create(Obj *o, s32 player, s32 set) {
    ChaW arg;

    arg.player = player;
    arg.set = set;
    return tskCreate(gEneManager->child->next->next->next, chaYAE_proc, o->file, &arg);
}

/* A seated player's talker, once it is done. */
Obj *chaGetTalker(Obj *o) {
    ChaW *w = CHA(o);

    if (o != NULL && w->done != 0) {
        return w->talker;
    }
    return NULL;
}

void chamiseUnusedStub(void) {
}

/* "chamise_Player_create": a seated player for each character no one plays; with flag 0x134
 * set, the players in the game sit down too. */
void chamise_Player_create(Obj *o) {
    ChamiseW *w = CHAMISE(o);
    s32 i;
    s32 chara;
    s32 index;
    s32 set;
    s32 pad;

    w->players = 0;
    w->charas = 0;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            chara = plGetChara(gGameWork.players[i]);
            index = plGetIndex(gGameWork.players[i]);
            if (chara == 0) {
                w->charas |= 1;
            } else if (chara == 1) {
                w->charas |= 2;
            } else if (chara == 2) {
                w->charas |= 4;
            } else if (chara == 3) {
                w->charas |= 8;
            }
            if (index == 0) {
                w->players |= 1;
            } else if (index == 1) {
                w->players |= 2;
            } else if (index == 2) {
                w->players |= 4;
            } else if (index == 3) {
                w->players |= 8;
            }
        }
    }
    if (!scStageFlagBit7(1)) {
        w->charas |= 4;
    }
    if (!scStageFlagBit7(4)) {
        w->charas |= 8;
    }
    if (gGameWork.stage == 0x8D) {
        set = 1;
    } else {
        set = 0;
    }
    for (i = 0; i < 4; i++) {
        if (!plIsInPlay(i)) {
            if (!(w->charas & 1)) {
                DPRINTF("ゴエモン作ります\n");
                w->seated[i] = chaGOE_create(o, i, set);
                w->charas |= 1;
            } else if (!(w->charas & 2)) {
                DPRINTF("エビス丸作ります\n");
                w->seated[i] = chaEBI_create(o, i, set);
                w->charas |= 2;
            } else if (!(w->charas & 4)) {
                DPRINTF("サスケ作ります\n");
                w->seated[i] = chaSAS_create(o, i, set);
                w->charas |= 4;
            } else if (!(w->charas & 8)) {
                DPRINTF("ヤエちゃん作ります\n");
                w->seated[i] = chaYAE_create(o, i, set);
                w->charas |= 8;
            }
        } else if (swWorkTest(0x134)) {
            chaPlayerToSeat(gGameWork.players[i], set, plGetChara(gGameWork.players[i]));
        }
        if (w->seated[i] != NULL) {
            chaSeatSet(w->seated[i]);
        }
    }
    DPRINTF("chamise_Player_create: exist num[%x] kind[%x]\n", w->players, w->charas);
}

/* "chamise": the tea house. A player who talks a seated one into it (flags 0x130-0x133)
 * changes to that character: a white fade, a voice, and the stage again. */
s32 chamise_proc(Obj *o, s16 msg, void *arg) {
    Obj *talker;
    ChamiseW *w = CHAMISE(o);
    s32 pad;
    s32 i;
    s32 index;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "chamise";
            eneSetup(o, arg);
            chamise_Player_create(o);
            DPRINTF("new num[%d] kind[%d]\n", w->players, w->charas);
            w->timer = 0;
            if (!swWorkTest(0x134)) {
                fadeSetAll(25.0f, 255.0f, 0.0f, 0, 0, 0);
                w->fade = -25;
            } else {
                fadeSetAll(15.0f, 255.0f, 0.0f, 0xFF, 0xFF, 0xFF);
                w->fade = -17;
            }
            gGameWork.scenarioRunning = 2;
            break;
        case 0:
            if (w->timer == 8) {
                if (!swWorkTest(0x134)) {
                    if (gGameWork.stage == 0x8D) {
                        soundPlayAfterOne(0x8D);
                    }
                } else {
                    swWorkReset(0x134);
                }
            }
            if (swWorkTest(0x130)) {
                swWorkReset(0x130);
                fadeSetAll(15.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                w->chara = 0;
                w->fade = 17;
                sndPlay(0x161);
            } else if (swWorkTest(0x131)) {
                swWorkReset(0x131);
                fadeSetAll(15.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                w->chara = 1;
                w->fade = 17;
                sndPlay(0x161);
            } else if (swWorkTest(0x132)) {
                swWorkReset(0x132);
                fadeSetAll(15.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                w->chara = 2;
                w->fade = 17;
                sndPlay(0x161);
            } else if (swWorkTest(0x133)) {
                swWorkReset(0x133);
                fadeSetAll(15.0f, 0.0f, 255.0f, 0xFF, 0xFF, 0xFF);
                w->chara = 3;
                w->fade = 17;
                sndPlay(0x161);
            }
            if (w->fade != 0) {
                DPRINTF("デモモード突入\n");
                gGameWork.scenarioRunning = 2;
            }
            if (!scIsRunning()) {
                DPRINTF("会話開始\n");
                DPRINTF("talkNo[%d]\n", 0);
                gGameWork.scenarioRunning = 2;
            }
            if (w->fade > 0) {
                w->fade--;
                if (w->fade == 0) {
                    for (i = 0; i < 4; i++) {
                        if ((talker = chaGetTalker(w->seated[i])) != NULL) {
                            index = plGetIndex(talker);
                            break;
                        }
                    }
                    ((PlayerSlot *) &gPlayer1Chara)[index].chara = w->chara;
                    if (w->chara == 0) {
                        sndPlay(0x116);
                    } else if (w->chara == 1) {
                        sndPlay(0x115);
                    } else if (w->chara == 2) {
                        sndPlay(0x117);
                    } else if (w->chara == 3) {
                        sndPlay(0x118);
                    }
                    swWorkSet(0x134);
                    if (gGameWork.stage == 0x8D) {
                        stageGotoSpawnNoSave(gStageTask->parent, gGameWork.stage, 0, 0.0f, 10.0f, 0.0f,
                                             1.5707964f, 0);
                    } else {
                        stageGotoSpawnNoSave(gStageTask->parent, gGameWork.stage, 0, 0.0f, 10.0f, 0.0f,
                                             3.1415927f, 0);
                    }
                }
            } else if (w->fade < 0) {
                w->fade++;
                if (w->fade == 0 && gGameWork.stage == 0x8D) {
                    DPRINTF("[%d] P デモ解除\n", 0);
                    if (w->players & 1) {
                        plSetAction(gGameWork.players[0], 0);
                    }
                    if (w->players & 2) {
                        plSetAction(gGameWork.players[1], 0);
                    }
                    if (w->players & 4) {
                        plSetAction(gGameWork.players[2], 0);
                    }
                    if (w->players & 8) {
                        plSetAction(gGameWork.players[3], 0);
                    }
                }
            }
            DPRINTF("何人いるのかな？[%d]\n", 0);
            w->timer++;
            break;
        case 1:
            gGameWork.scenarioRunning = 0;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Message 2: its model is a player's ("struct DOLL_PLAYER"), with a talk box and a shadow. */
void chaDollInit(Obj *o, s32 index, s32 chara) {
    s32 pad[2];
    PlInit init;
    ChaW *w = CHA(o);

    plMakeInit(&init, index, chara);
    init.flags |= 0x90000040;
    init.flags2 |= 0xC;
    init.zone = 0;
    w->body = plCreate(o, &init);
    w->model = PL(w->body)->model;
    w->box = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 100, 100, 0, 0);
    w->box->kind = 0x65;
    w->shadow = shadowCreate();
}

/* State 0: it faces its way, its face moves; talked to, state 1. */
void chaWait(Obj *o) {
    s32 pad[2];
    ChaW *w = CHA(o);
    Player *p = PL(w->body);

    eneTurnToAngle(o, chaSeats[w->set][w->chara][3] * 57.29577637f, 18.0f);
    fadeAnimStep((Anim *) &p->anims[0]);
    fadeAnimStep((Anim *) &p->anims[0x10]);
    if (!scIsRunning() && demoTalkEndCheck(o)) {
        w->state = 1;
        w->started = 0;
    }
}

/* State 1: its talk starts. */
void chaTalkStart(Obj *o) {
    ChaW *w = CHA(o);
    s32 pad;

    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
    demoCamZoom_CL(w->talker);
    eneSetForward(o, 1);
    w->state = 2;
}

/* State 2: it stands up and turns to the talker. */
void chaStandUp(Obj *o) {
    ChaW *w = CHA(o);
    s32 pad;
    Vec3f pos;
    s32 pad2;

    if (w->started == 0) {
        w->animSpeed = 0.4f;
        plAnimeSet(PL(w->body), 0xD7);
    }
    w->started = 1;
    if (w->talker != NULL) {
        tskSendMsg(w->talker, 4, &pos);
        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
            w->state = 3;
            w->started = 0;
        }
    }
}

/* State 3: it sits back down. When the talk is over the talker is let go; with one of the
 * flags 0x130-0x133 set, every player gets flag 0x40 and it stands up (state 4). */
void chaSitDown(Obj *o) {
    ChaW *w = CHA(o);
    s32 index = plGetIndex(w->talker);
    Player *p;
    s32 i;

    if (w->started == 0) {
        w->model->frame = 0.0f;
        w->animSpeed = 0.2f;
    }
    w->started = 1;
    chaAnimStep(w->model, w->animSpeed, 0);
    plSetAction(gGameWork.players[index], 0);
    p = PL(gGameWork.players[index]);
    p->flags &= ~0x10000000;
    personTalkCamOff();
    shopDemoFinish(o);
    camAiSetHold(0);
    if (swWorkTest(0x130) || swWorkTest(0x131) || swWorkTest(0x132) || swWorkTest(0x133)) {
        for (i = 0; i < 4; i++) {
            if (plIsInPlay(i)) {
                p = PL(gGameWork.players[i]);
                p->flags |= 0x40;
            }
        }
        w->state = 4;
        w->started = 0;
    } else {
        w->talker = NULL;
        w->state = 0;
        w->started = 0;
    }
}

/* State 4: the model plays on. */
void chaAnimPlay(Obj *o) {
    chaAnimStep(CHA(o)->model, CHA(o)->animSpeed, 0);
}

/* Message 1: the shadow, the model task and the talker's work go. */
void chaDestroy(Obj *o) {
    ChaW *w = CHA(o);

    if (w->shadow != NULL) {
        frmDestroy(&w->shadow->f);
    }
    tskDestroy(w->body);
    demoTalkerFree(o);
}

s32 shopIdleProc(Obj *obj, s16 msg, void *arg) {
    return tskDefaultProc(obj, msg, arg);
}

/* The talk starts: the players stand, the camera closes in on the talker. */
void shopTalkStart(Obj *o) {
    demoPlStandAll();
    demoCamZoom_CL(CHA(o)->talker);
    eneSetForward(o, 1);
}

/* The talk ends: the players go, the camera back. */
void shopTalkEnd(Obj *o) {
    (*shopDemoFinish)(o);
    demoPlReleaseAll();
    personTalkCamOff();
    camAiSetHold(0);
}

/* The talk is over (demo.c's demoFinish): the talker is sent message 8, the talk box is on
 * again. */
void shopDemoFinish(Obj *o) {
    ChaW *p = CHA(o);
    s32 pad;
    struct {
        Obj *o;
        s32 one;
    } arg;

    arg.o = o;
    arg.one = 1;
    if (p->talker != NULL) {
        tskSendMsg(p->talker, 8, &arg);
    } else {
        DPRINTF("Please set playerD at damage frame. Please use taskSet\n");
    }
    p->flags &= ~2;
    p->flags &= ~1;
    p->box->flags |= 4;
    eneSetForward(o, 0);
}
