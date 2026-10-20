#include "tlb.h"

/* Three course objects: "water" (a scrolling water surface you can ride), "boze" (a sea monk
 * that surfaces near the players and lunges along the course) and "ubridge" (a bridge that
 * swings and falls into the water when the boze rams it). */

extern Frame *gFrmLayer3;

extern u8 bozeWaterTex[];
extern u8 bozeFace1Tex[];
extern u8 bozeFace2Tex[];
extern u8 bozeRiseAnim[];
extern u8 bozeSwimAnim[];
extern u8 bozeLungeAnim[];
extern u8 bozeLungeAtkChain[];
extern Gfx bozeWaterModelDl[];
extern u8 bozeWaterRide[];
extern Gfx bozeUbridgeDl[];
extern u8 bozeUbridgeRide[];

/* acoll.c defines the box maker void; it returns the box. */
typedef ACollFrame *(*DmgAddXYZ)(Frame *parent, Frame *model, u16 radius, u16 height, s16 ox, s16 oy, s16 oz,
                                 s32 a);

/* water's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct WaterWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ s32 scroll;
    /* 0x40 */ s32 speed; /* added to scroll each draw */
    /* 0x44 */ Gfx *(*draw)(Gfx *gfx, u16 scroll);
} WaterWork;

/* boze's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct BozeWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3]; /* the move's step, toward (tx, ty, tz) */
    /* 0x34 */ s32 (*act)(Obj *obj, struct BozeWork *w); /* its state; nonzero: it ends */
    /* 0x38 */ ACollFrame *atk; /* its attack (an animated chain) while it lunges */
    /* 0x3C */ f32 animStep;
    /* 0x40 */ s16 animTime; /* steps to the animation's and the move's end */
    /* 0x42 */ u16 count; /* steps in the phase */
    /* 0x44 */ s8 animEnd; /* the animation's last frame (-1: it loops) */
    /* 0x45 */ u8 phase; /* within the state */
    /* 0x46 */ u8 tick; /* its wake: a splash every 8 steps while it moves fast */
    /* 0x48 */ s16 tx;
    /* 0x4A */ s16 ty;
    /* 0x4C */ s16 tz;
    /* 0x50 */ f32 turnTo; /* the heading it turns to */
    /* 0x54 */ f32 turnStep;
    /* 0x58 */ f32 place; /* where along the course it went */
    /* 0x5C */ s16 placeMin; /* the stretch of course it works in */
    /* 0x5E */ s16 placeMax;
    /* 0x60 */ s16 height; /* up, out of the water */
    /* 0x62 */ s16 depth; /* down, under it */
} BozeWork;

#define BOZE(obj) ((BozeWork *) ((u32) (obj) + sizeof(Obj)))

/* ubridge's work (task + 0x20): an enemy's (EneObj) up to 0x34. */
typedef struct UbridgeWork {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3]; /* vel[1]: the fall's speed */
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ f32 home[3];
    /* 0x4C */ f32 drop; /* how far its end has fallen */
    /* 0x50 */ u16 count;
    /* 0x52 */ u8 fall; /* 0 standing, 1 and 2 falling one way or the other */
    /* 0x53 */ u8 splashed;
    /* 0x54 */ s16 sparkle; /* steps of sparkles after the hit */
} UbridgeWork;

s32 bozeProc(Obj *obj, s16 msg, void *arg);
void bozeTurnTo(Obj *obj, f32 angle);
s32 bozeActWait(Obj *obj, BozeWork *w);
s32 bozeActFight(Obj *obj, BozeWork *w);
void bozeItexSparkle(f32 x, f32 y, f32 z, f32 size);
s32 bozeUbridgeFallStep(Obj *obj, UbridgeWork *w);

/* A 32x32 RGBA16 texture loaded at tmem and drawn through tile rtile from (uls, ult): one macro,
 * its seven commands on one source line (tlb_0053's). */
#define gDPLoadScrollBlock(pkt, timg, tmem, rtile, uls, ult)                                               \
    {                                                                                                     \
        gDPSetTextureImage(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, timg);                                    \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, \
                   5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPLoadSync(pkt);                                                                                 \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES));            \
        gDPPipeSync(pkt);                                                                                 \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,      \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                                 \
        gDPSetTileSize(pkt, rtile, uls, ult, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC);     \
    }

/* The water's two scrolling texture layers. */
Gfx *bozeWaterDl(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 0x80, 0x80, 0x80, 0x60);
    gDPLoadScrollBlock(gfx++, bozeWaterTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock(gfx++, bozeWaterTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "water": eneSetup's data: the model in the low byte (1: the only one, with its ride), the
 * heading in 512ths of a half turn in the high half. */
s32 bozeWaterProc(Obj *obj, s16 msg, void *arg) {
    WaterWork *w = (WaterWork *) ((u32) obj + sizeof(Obj));
    s32 pad[2];
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "water";
            data = eneSetup(obj, arg);
            w->draw = bozeWaterDl;
            w->speed = -1;
            if ((data & 0xFF) == 1) {
                w->pol = eneNewPol(obj, bozeWaterModelDl);
                w->ride = rideCreate(gRideTree2, (s32) obj, &w->pol->f,
                                     (RideData *) eneFileAddr(obj, bozeWaterRide, 0));
            }
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 0.1f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->rot[1] = (data >> 16) * 3.1415927f / 512;
            break;
        case 1:
            if (w->ride != NULL) {
                frmDestroy(&w->ride->f);
            }
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = w->draw(gGfxDlHead, w->scroll);
            w->scroll += w->speed;
            eneSetModelOnCourse(obj, &w->pol->f);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The boze's next animation and move take n steps. */
void bozeSetAnimTime(Obj *obj, s16 n) {
    BOZE(obj)->animTime = n;
}

/* Its face texture: the first (bozeFace1Tex) or the second (bozeFace2Tex). */
void bozeSetFace1(Obj *obj) {
    BOZE(obj)->model->segs[3] = eneFileAddr(obj, bozeFace1Tex, 0);
}

void bozeSetFace2(Obj *obj) {
    BOZE(obj)->model->segs[3] = eneFileAddr(obj, bozeFace2Tex, 0);
}

/* Its rise (bozeRiseAnim) to the animation's last frame; drops its attack. */
void bozeAnimRise(Obj *obj) {
    BozeWork *w = BOZE(obj);

    w->model->anim = bozeRiseAnim;
    w->model->frame = 0.0f;
    w->animEnd = polAnimeLastFrame(w->model);
    w->animStep = (w->animEnd - w->model->frame) / w->animTime;
    if (w->atk != NULL) {
        w->atk = (ACollFrame *) aniAtkDelete(w->atk);
    }
}

/* Its swim (bozeSwimAnim), looped; drops its attack. */
void bozeAnimSwim(Obj *obj) {
    BozeWork *w = BOZE(obj);

    w->model->anim = bozeSwimAnim;
    w->model->frame = 0.0f;
    w->animEnd = -1;
    w->animStep = (f32) polAnimeLastFrame(w->model) / w->animTime;
    if (w->atk != NULL) {
        w->atk = (ACollFrame *) aniAtkDelete(w->atk);
    }
}

/* Its lunge (bozeLungeAnim) from frame 0 to 9, with its attack. */
void bozeAnimLunge(Obj *obj) {
    BozeWork *w = BOZE(obj);

    w->model->anim = bozeLungeAnim;
    w->model->frame = 0.0f;
    w->animEnd = 9;
    w->animStep = (w->animEnd - w->model->frame) / w->animTime;
    if (w->atk != NULL) {
        w->atk = (ACollFrame *) aniAtkDelete(w->atk);
    }
    w->atk = aniAtkCreate(&uda_atk_root_frame->frame, &w->model->f, (u32) bozeLungeAtkChain, eneGetFile(obj, 0), 0);
    if (w->atk != NULL) {
        w->atk->kind = 0x78;
        w->atk->power = (s32) obj;
    }
}

/* Its splash sound (0x217) at its attack box. */
void bozeSplashSound(Obj *obj) {
    SoundLocation2(0x217, gGameWork.view, BOZE(obj)->atk->pos[0], BOZE(obj)->atk->pos[1],
                   BOZE(obj)->atk->pos[2], 100.0f, 0.0f, 1.5707964f);
}

/* Its crash (0x10A) at its attack box; in view, every player's pak rumbles. */
void bozeCrash(Obj *obj) {
    BozeWork *w = BOZE(obj);
    s32 i;

    SoundLocation2(0x10A, gGameWork.view, w->atk->pos[0], w->atk->pos[1], w->atk->pos[2], 100.0f, 0.0f,
                   1.5707964f);
    if (soundInFovCheck(gGameWork.view, w->atk->pos[0], w->atk->pos[1], w->atk->pos[2])) {
        for (i = 0; i < 4; i++) {
            if (gGameWork.players[i] != NULL) {
                contPakRumbleStart(i, 0xAAAAAAAA, 15);
            }
        }
    }
}

/* Its lunge's end (bozeLungeAnim) from frame 9 to 19, with its attack. */
void bozeAnimLungeEnd(Obj *obj) {
    BozeWork *w = BOZE(obj);

    w->model->anim = bozeLungeAnim;
    w->model->frame = 9.0f;
    w->animEnd = 19;
    w->animStep = (w->animEnd - w->model->frame) / w->animTime;
    if (w->atk != NULL) {
        w->atk = (ACollFrame *) aniAtkDelete(w->atk);
    }
    w->atk = aniAtkCreate(&uda_atk_root_frame->frame, &w->model->f, (u32) bozeLungeAtkChain, eneGetFile(obj, 0), 0);
    if (w->atk != NULL) {
        w->atk->kind = 0x78;
        w->atk->power = (s32) obj;
    }
}

/* Turns it to the heading `angle` in its animTime steps, the short way round. (The step's
 * difference, written once after the wrap, is computed again on each path by IDO.) */
void bozeTurnTo(Obj *obj, f32 angle) {
    BozeWork *w = BOZE(obj);

    w->turnTo = angle;
    if (w->turnTo - w->rot[1] > 3.1415927f) {
        w->rot[1] += 6.2831855f;
    } else if (w->turnTo - w->rot[1] < -3.1415927f) {
        w->rot[1] -= 6.2831855f;
    }
    w->turnStep = (w->turnTo - w->rot[1]) / w->animTime;
}

/* Moves it to (x, y, z) in its animTime steps. */
void bozeMoveTo(Obj *obj, s16 x, s16 y, s16 z) {
    BOZE(obj)->tx = x;
    BOZE(obj)->ty = y;
    BOZE(obj)->tz = z;
    BOZE(obj)->vel[0] = (BOZE(obj)->tx - BOZE(obj)->x) / BOZE(obj)->animTime;
    BOZE(obj)->vel[1] = (BOZE(obj)->ty - BOZE(obj)->y) / BOZE(obj)->animTime;
    BOZE(obj)->vel[2] = (BOZE(obj)->tz - BOZE(obj)->z) / BOZE(obj)->animTime;
}

/* "boze": eneSetup's data: the stretch of course in the low byte (0: 620-1570, else 534-2046),
 * the heading in 512ths of a half turn in the high half. Runs its state (act) each step, then
 * its turn, its move and its animation; message 7 to whatever its attack hits. */
s32 bozeProc(Obj *obj, s16 msg, void *arg) {
    BozeWork *w = BOZE(obj);
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "boze";
            data = eneSetup(obj, arg);
            if ((data & 0xFF) == 0) {
                w->height = -23;
                w->depth = -125;
                w->placeMin = 620;
                w->placeMax = 1570;
            } else {
                w->height = -93;
                w->depth = -195;
                w->placeMin = 534;
                w->placeMax = 2046;
            }
            if ((w->model = eneNewAnime(obj, (u32) bozeRiseAnim)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmLayer3);
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->rot[1] = (data >> 16) * 3.1415927f / 512;
            bozeSetAnimTime(obj, 60);
            bozeSetFace1(obj);
            w->act = bozeActWait;
            break;
        case 1:
            if (w->atk != NULL) { w->atk = (ACollFrame *) aniAtkDelete(w->atk); }
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            w->count++;
            if (w->act != NULL && w->act(obj, w) != 0) {
                return -1;
            }
            if (w->turnStep != 0.0f) {
                if (fabsf(w->turnTo - w->rot[1]) < fabsf(w->turnStep)) {
                    w->rot[1] = w->turnTo;
                    w->turnStep = 0.0f;
                } else {
                    w->rot[1] += w->turnStep;
                }
            }
            if (w->vel[0] != 0.0f) {
                if (fabsf(w->tx - w->x) < fabsf(w->vel[0])) {
                    w->x = w->tx;
                    w->vel[0] = 0.0f;
                } else {
                    w->x += w->vel[0];
                }
            }
            if (w->vel[1] != 0.0f) {
                if (fabsf(w->ty - w->y) < fabsf(w->vel[1])) {
                    w->y = w->ty;
                    w->vel[1] = 0.0f;
                } else {
                    w->y += w->vel[1];
                }
            }
            if (w->vel[2] != 0.0f) {
                if (fabsf(w->tz - w->z) < fabsf(w->vel[2])) {
                    w->z = w->tz;
                    w->vel[2] = 0.0f;
                } else {
                    w->z += w->vel[2];
                }
            }
            eneSetModelOnCourse(obj, &w->model->f);
            w->model->frame += w->animStep;
            if (w->animEnd < 0) {
                if (polAnimeLastFrame(w->model) + 1 <= w->model->frame) {
                    w->model->frame -= polAnimeLastFrame(w->model) + 1;
                }
            } else if (w->animEnd < w->model->frame) {
                w->model->frame = w->animEnd;
            }
            if (w->atk != NULL) {
                ((AniAtkFrame *) w->atk->model)->time = w->model->frame;
                if (acollAtkWasHit(w->atk) == 1 && (Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, obj);
                }
            }
            eneStepHookPost(obj);
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* How far along the course the farthest player in play is moving this step (plGetSpeed's x). */
f32 bozePlayersMaxSpeedX(void) {
    s32 i;
    f32 max = 0.0f;
    Vec3f v;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetSpeed(gGameWork.players[i], &v);
            if (max < v.x) {
                max = v.x;
            }
        }
    }
    return max;
}

/* How far along the course the farthest player in play is. */
f32 bozePlayersMaxPlace(void) {
    s32 i;
    s32 zone;
    f32 max = 0.0f;
    f32 pos[3];

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plAskPos(gGameWork.players[i], &pos[0], &pos[1], &pos[2], &zone);
            if (max < pos[0]) {
                max = pos[0];
            }
        }
    }
    return max;
}

/* The point on course 1 `ahead` (plus 55) past the farthest player, 280 to the side, and the
 * course's heading there; returns the place. */
f32 bozePointAhead(f32 ahead, f32 *x, f32 *z, f32 *heading) {
    f32 place = bozePlayersMaxPlace() + ahead + 55.0f;

    vramCoursePointXZ(1, place, -280.0f, x, z);
    *heading = vramCourseHeading(1, place);
    return place;
}

/* Its first state: under the water until a player is within 60 of its stretch, then it rises
 * (with a splash if it breaks the surface), lunges and crashes, and fights (bozeActFight). */
s32 bozeActWait(Obj *obj, BozeWork *w) {
    Vec3f push;

    switch (w->phase) {
        case 0:
            if (w->placeMin - 60 < bozePlayersMaxPlace()) {
                w->count = 0;
                w->phase = 1;
            }
            break;
        case 1:
            if (w->count == 1) {
                bozeSetAnimTime(obj, 60);
                bozeSetFace1(obj);
                bozeAnimRise(obj);
                bozeMoveTo(obj, w->x, w->height, w->z);
                if (rideTree2PointIn((Vec3f *) &w->x, &push)) {
                    effectSibukiCreateWhite(w->x + push.x, w->y + push.y, w->z + push.z, 1.5f, 0.5f, 0.05f, 0.08f);
                    sndPlay(0x14C);
                }
            } else if (w->count == 60) {
                bozeSetAnimTime(obj, 30);
                bozeAnimLunge(obj);
            } else if (w->count == 75) {
                bozeSplashSound(obj);
            } else if (w->count == 90) {
                bozeCrash(obj);
                bozeAnimLungeEnd(obj);
            } else if (w->count == 120) {
                w->phase = 0;
                w->count = 0;
                w->act = bozeActFight;
            }
            break;
    }
    return 0;
}

/* The fight, along its stretch: a player in the near part brings it up ahead of him (1) or, in
 * the last 150, from behind (4): it surfaces 150 ahead (or at the player), lunges at 20 and
 * crashes at 50, and dives (2); under the water (3) it waits for the players to pass or stop.
 * A player out of the stretch sends it back under to wait (bozeActWait's phase 0). */
s32 bozeActFight(Obj *obj, BozeWork *w) {
    f32 x;
    f32 y; /* unused: the game's frame word between x and z */
    f32 z;
    f32 heading;
    Vec3f pos;
    Vec3f push;

    switch (w->phase) {
        case 0:
            x = bozePlayersMaxPlace();
            if (w->placeMin <= x && x < w->placeMax - 150) {
                w->count = 0;
                w->phase = 1;
            } else if (w->placeMax - 150 <= x && x < w->placeMax) {
                w->count = 0;
                w->phase = 4;
            }
            break;
        case 1:
            if (w->count == 1) {
                bozeSetAnimTime(obj, 30);
                w->place = bozePointAhead(150.0f, &x, &z, &heading);
                if (w->placeMax < w->place) {
                    w->place = bozePointAhead(0.0f, &x, &z, &heading);
                }
                bozeMoveTo(obj, x, w->height, z);
                bozeTurnTo(obj, heading - 1.5707964f);
                if (sqrtf(w->vel[2] * w->vel[2] + w->vel[0] * w->vel[0]) > 2.0f) {
                    sndPlay(0x2B7);
                }
            } else if (w->count == 20) {
                bozeSetAnimTime(obj, 30);
                bozeSetFace2(obj);
                bozeAnimLunge(obj);
            } else if (w->count == 35) {
                bozeSplashSound(obj);
            } else if (w->count == 50) {
                bozeCrash(obj);
                w->count = 0;
                w->phase = 2;
            }
            break;
        case 2:
            bozeSetAnimTime(obj, 30);
            bozeSetFace1(obj);
            bozeAnimLungeEnd(obj);
            w->count = 0;
            if (bozePlayersMaxPlace() < w->place) {
                w->phase = 3;
            } else {
                w->phase = 1;
            }
            break;
        case 3:
            if (w->count == 1) {
                bozeSetAnimTime(obj, 30);
                bozeAnimLungeEnd(obj);
            } else if (w->count == 30) {
                if (w->place < bozePlayersMaxPlace()) {
                    w->count = 0;
                    w->phase = 1;
                }
            } else if (w->count == 40) {
                if (bozePlayersMaxSpeedX() < 0.5f) {
                    w->count = 0;
                    w->phase = 4;
                }
            } else if (w->count == 50) {
                bozeSetAnimTime(obj, 30);
                bozeSetFace2(obj);
                bozeAnimLunge(obj);
            } else if (w->count == 65) {
                bozeSplashSound(obj);
            } else if (w->count == 80) {
                bozeCrash(obj);
                w->count = 0;
                w->phase = 2;
            }
            break;
        case 4:
            if (w->count == 1) {
                bozeSetAnimTime(obj, 30);
                w->place = bozePointAhead(0.0f, &x, &z, &heading);
                bozeMoveTo(obj, x, w->height, z);
                bozeTurnTo(obj, heading - 1.5707964f);
                if (sqrtf(w->vel[2] * w->vel[2] + w->vel[0] * w->vel[0]) > 2.0f) {
                    sndPlay(0x2B7);
                }
            } else if (w->count == 20) {
                bozeSetAnimTime(obj, 30);
                bozeSetFace2(obj);
                bozeAnimLunge(obj);
            } else if (w->count == 35) {
                bozeSplashSound(obj);
            } else if (w->count == 50) {
                bozeCrash(obj);
                w->count = 0;
                w->phase = 2;
            }
            break;
    }
    w->tick++;
    if ((w->tick & 7) == 1 && sqrtf(w->vel[2] * w->vel[2] + w->vel[0] * w->vel[0]) > 2.0f) {
        pos.x = w->x;
        pos.y = w->y - 100.0f;
        pos.z = w->z;
        if (rideTree2PointIn(&pos, &push)) {
            effectSibukiCreateWhite(pos.x + push.x, pos.y + push.y, pos.z + push.z, 1.2f, 0.2f, 0.02f, 0.02f);
        }
    }
    if (w->phase != 0) {
        x = bozePlayersMaxPlace();
        if (x < w->placeMin || w->placeMax < x) {
            bozeSetAnimTime(obj, 60);
            bozeAnimSwim(obj);
            w->count = 0;
            w->phase = 0;
        }
    }
    return 0;
}

void bozeItexSparkle(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexSparkleAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xE0; init.prim.b = 0xA0;
    init.env.r = 0x60; init.env.g = 0x60; init.env.b = 0x50;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* "ubridge": a bridge you can ride, its damage box hit only by the boze; eneSetup's data is its
 * heading in 512ths of a half turn. */
s32 bozeUbridgeProc(Obj *obj, s16 msg, void *arg) {
    UbridgeWork *w = (UbridgeWork *) ((u32) obj + sizeof(Obj));
    s32 pad;
    s32 data;

    switch (msg) {
        case 2:
            obj->name = "ubridge";
            data = eneSetup(obj, arg);
            if ((w->pol = eneNewPol(obj, bozeUbridgeDl)) == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayer3);
            if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->pol->f,
                                      (RideData *) eneFileAddr(obj, bozeUbridgeRide, 0))) == NULL) {
                return -1;
            }
            w->dmg = ((DmgAddXYZ) acollDmgAddXYZ)(&ene_dmg_root_frame->frame, &w->pol->f, 250, 80, 0, -40, 0, 0);
            w->rot[1] = data * 3.1415927f / 512;
            w->home[0] = w->x;
            w->home[1] = w->y;
            w->home[2] = w->z;
            w->drop = 0.0f;
            break;
        case 1:
            frmDestroy(&w->dmg->frame);
            frmDestroy(&w->ride->f);
            frmDestroy(&w->pol->f);
            break;
        case 0:
            w->count++;
            if (bozeUbridgeFallStep(obj, w)) {
                return -1;
            }
            eneSetModelOnCourse(obj, &w->pol->f);
            eneStepHookPost(obj);
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Its step: standing (0), a hit from a boze (bozeProc) tips it the way the boze came from
 * (1 or 2), with sparkles for 16 steps; it falls 20 long about its end until it is 80 down
 * (nonzero: it ends), with a splash when it reaches the water. */
s32 bozeUbridgeFallStep(Obj *obj, UbridgeWork *w) {
    f32 x;
    f32 y;
    f32 z;
    f32 k;
    f32 s;
    f32 c;
    Vec3f d;
    Vec3f p;
    Vec3f push;

    s = mathSinCos(w->rot[1] / 3.1415927f * 512.0f, &c);
    switch (w->fall) {
        case 0:
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1 && (Obj *) w->dmg->hitPower != NULL) {
                if (((Obj *) w->dmg->hitPower)->proc == bozeProc) {
                    d.x = 0.0f;
                    d.y = 0.0f;
                    d.z = 1.0f;
                    mathVecRotateY(w->rot[1], &d.x, &d.y, &d.z);
                    x = w->dmg->hitPos[0] - w->x;
                    z = w->dmg->hitPos[2] - w->z;
                    c = (x * d.x + z * d.z) / sqrtf((x * x + z * z) * (d.x * d.x + d.z * d.z));
                    w->sparkle = 16;
                    w->count = 0;
                    if (0.0f < c) {
                        w->fall = 2;
                    } else {
                        w->fall = 1;
                    }
                    soundSuionCreateCam(obj, 0x251, w->dmg->hitPos[0], w->y, w->dmg->hitPos[2]);
                }
            }
            break;
        case 1:
            w->vel[1] += -0.1;
            w->drop += w->vel[1];
            w->rot[0] = mathAtan2(w->drop, 20.0f);
            k = 1.0f - 20.0f / sqrtf(w->drop * w->drop + 400.0f);
            x = k * 20.0f;
            y = w->drop * k;
            w->x = w->home[0] - x * s;
            w->z = w->home[2] - x * c;
            w->y = w->home[1] + y;
            if (w->drop < -80.0f) {
                return 1;
            }
            break;
        case 2:
            w->vel[1] += -0.1;
            w->drop += w->vel[1];
            w->rot[0] = -mathAtan2(w->drop, 20.0f);
            k = 1.0f - 20.0f / sqrtf(w->drop * w->drop + 400.0f);
            x = k * 20.0f;
            y = w->drop * k;
            w->x = w->home[0] + x * s;
            w->z = w->home[2] + x * c;
            w->y = w->home[1] + y;
            if (w->drop < -80.0f) {
                return 1;
            }
            break;
    }
    if (!w->splashed) {
        p.x = w->x;
        p.z = w->z;
        p.y = w->home[1] + w->drop;
        if (rideTree2PointIn(&p, &push)) {
            soundIncamCam(0x29C, p.x, p.y, p.z);
            effectSibukiCreateWhite(p.x, p.y, p.z, 0.1f, 0.2f, 0.02f, 0.02f);
            w->splashed = 1;
        }
    }
    if (w->sparkle > 0) {
        if ((w->sparkle & 0xF) == 1) {
            x = fadeRand(40) - 20;
            y = 0.0f;
            z = 20.0f;
            mathVecRotate(w->rot[0], w->rot[1], w->rot[2], &x, &y, &z);
            bozeItexSparkle(w->x + x, w->y + y, w->z + z, 0.15f);
        } else if ((w->sparkle & 0xF) == 5) {
            x = fadeRand(40) - 20;
            y = 0.0f;
            z = -20.0f;
            mathVecRotate(w->rot[0], w->rot[1], w->rot[2], &x, &y, &z);
            bozeItexSparkle(w->x + x, w->y + y, w->z + z, 0.15f);
        }
        w->sparkle--;
    }
    return 0;
}
