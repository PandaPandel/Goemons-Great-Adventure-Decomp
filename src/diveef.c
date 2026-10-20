#include "player.h"
#include "diveef.h"
#include "effect.h"
#include "fade.h"
#include "file.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "plmain.h"
#include "plpick.h"
#include "plshot.h"
#include "plutil.h"
#include "polygon.h"
#include "ride.h"
#include "tmanager.h"

/* The player's water effects (overlay 9, "diveef"): a sparkle of three stars ("plkira"),
 * underwater bubbles ("水中泡"), the ripple on the surface ("水面も"), the splash ring
 * ("diveef"), and the splashes and sounds of going in and out of water. */

/* The sparkle: three stars that grow over the player, spin, and fade. */
typedef struct PlKira {
    /* 0x00 */ AnimeFrame *frame[3];
    /* 0x0C */ s32 alpha;
    /* 0x10 */ f32 scale;
    /* 0x14 */ s32 index; /* the player's */
    /* 0x18 */ s32 state; /* 0 growing, 1 spinning, 2 fading */
    /* 0x1C */ s32 t;
} PlKira;

/* An underwater bubble. */
typedef struct PlAwa {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 x; /* where it rises from */
    /* 0x08 */ f32 z;
    /* 0x0C */ s32 unused0C;
    /* 0x10 */ s32 r;
    /* 0x14 */ s32 g;
    /* 0x18 */ s32 b;
    /* 0x1C */ s32 a;
    /* 0x20 */ f32 size;
    /* 0x24 */ s32 index; /* the player's */
    /* 0x28 */ s32 unused28; /* set to 0 when made, never read */
    /* 0x2C */ s32 t;
} PlAwa;

/* The ripple on the water's surface. */
typedef struct PlMoya {
    /* 0x00 */ MPolFrame *frame;
    /* 0x04 */ f32 scale;
    /* 0x08 */ s32 unused08;
    /* 0x0C */ Anim anim;
    /* 0x1C */ s32 unused1C; /* set to 0 when made, never read */
    /* 0x20 */ s32 unused20; /* likewise */
} PlMoya;

/* The splash ring. */
typedef struct PlDiveEf {
    /* 0x00 */ s32 index; /* the player's */
    /* 0x04 */ PolFrame *frame;
    /* 0x08 */ Anim anim;
    /* 0x18 */ s32 r;
    /* 0x1C */ s32 g;
    /* 0x20 */ s32 b;
    /* 0x24 */ s32 a;
    /* 0x28 */ f32 height; /* over the model */
    /* 0x2C */ s32 t;
    /* 0x30 */ s32 unused30; /* set to 0 when made, never read */
} PlDiveEf;

/* What the splash ring is made with. */
typedef struct PlDiveEfInit {
    /* 0x00 */ Player *p;
    /* 0x04 */ f32 height;
} PlDiveEfInit;

#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))
#define MOYA(obj) ((PlMoya *) FX(obj))

extern Obj *gEffectTask; /* the effect manager */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Frame *gFrmLayer8; /* the frame effects go under */
extern s16 gFrmKindMpol; /* "mpol" */
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "anime" */
/* diveef.c's .data (0x801A72B0-0x801A7300). */
u32 gPlKiraAnims[3] = { 0x0300012C, 0x0300025C, 0x0300037C }; /* the sparkle: its three animations */
AnimFrame gDiveRippleTexAnim[] = { /* the ripple: its animation */
    { 0x03003F60, 4 }, { 0x03004160, 4 }, { 0x03004360, 4 }, { 0x03004160, 4 }, { 0, 0 },
};
AnimFrame gDiveSplashRingTexAnim[] = { { 0x03000530, 1 }, { 0, 0 } }; /* the splash ring: its animation */
extern u8 diveSplashRingDl[]; /* the splash ring's list (file 0x123) */
extern u8 diveBubbleAnim[]; /* the bubble's animation (file 0x128) */
extern u8 diveRippleDl[]; /* the ripple's list (file 0x128) */



/* The sparkle ("plkira"). */
s32 plkira(Obj *obj, s16 msg, void *arg) {
    PlKira *w = FX(obj);
    Player *pl;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "plkira";
            w->index = *(s32 *) arg;
            w->state = 0;
            w->alpha = 200;
            for (i = 0; i < 3; i++) {
                if ((w->frame[i] = (AnimeFrame *) frmCreate(gFrmLayer8,
                                                                         gFrmKindAnime)) == NULL) {
                    return -1;
                }
                w->frame[i]->anim = gPlKiraAnims[i];
                w->frame[i]->segs[0] = fileRamAddr(0x306);
                plPutFrameAt(w->index, w->frame[i]);
                w->frame[i]->scale[0] = w->frame[i]->scale[1] = w->frame[i]->scale[2] = 0.0f;
                w->frame[i]->rot[1] = 0.0f;
            }
            plSound(arg, 0x24A);
            break;
        case 0:
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            pl = PL(gGameWork.players[w->index]);
            switch (w->state) {
                case 0:
                    w->scale += 0.014f;
                    if (w->scale >= 0.12f) {
                        w->scale = 0.12f;
                    }
                    if (plIsCharged(pl)) {
                        w->state = 1;
                    }
                    if (pl->charge < 5) {
                        w->state = 2;
                    }
                    break;
                case 1:
                    w->frame[0]->rot[1] += 0.18849556f;
                    w->frame[1]->rot[1] += 0.14137168f;
                    w->frame[2]->rot[1] += 0.09424778f;
                    if (w->t % 12 == 0) {
                        plSound(pl, 0x24B);
                    }
                    if (pl->charge < 5) {
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->alpha -= 20;
                    w->scale -= 0.004f;
                    if (w->alpha <= 0 || w->scale <= 0.0f) {
                        return -1;
                    }
                    break;
            }
            for (i = 0; i < 3; i++) {
                plPutFrameAt(w->index, w->frame[i]);
                if (!(pl->flags & 0x30000)) {
                    w->frame[i]->pos[1] += pl->bodyHeight * 0.5f;
                }
                w->frame[i]->scale[0] = w->frame[i]->scale[1] = w->frame[i]->scale[2] = w->scale;
                plAnimeStepLoop(w->frame[i], i * 0.15f + 0.5f);
            }
            w->t++;
            break;
        case 3:
            for (i = 0; i < 3; i++) {
                w->frame[i]->dl = gGfxDlHead;
            }
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0,
                              0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 250, 230, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 250, 36, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                frmDestroy(&w->frame[i]->f);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An underwater bubble ("水中泡"): it rises 1 a step, wobbling, and pops at the surface. */
s32 plSuichuAwa(Obj *obj, s16 msg, void *arg) {
    PlAwa *w = FX(obj);
    s32 pad;
    AnimeFrame *f;
    AnimeFrame *m;
    s32 pad2[2];
    Vec3f pos;
    Vec3f push;

    switch (msg) {
        case 2:
            obj->name = "水中泡";
            f = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if ((w->frame = f) == NULL) {
                return -1;
            }
            w->index = ((Player *) arg)->index;
            m = ((Player *) arg)->model;
            f->anim = diveBubbleAnim;
            floadGetSetRam((void **) f->segs, 0x128);
            plFramePosCopy(w->frame, m);
            f->pos[0] += fadeRand(100) * 0.05f - 5.0f;
            f->pos[1] += fadeRand(100) * 0.05f - 5.0f;
            f->pos[2] += fadeRand(100) * 0.05f - 5.0f;
            w->x = f->pos[0];
            w->z = f->pos[2];
            f->scale[0] = f->scale[2] = f->scale[1] = 0.03f;
            w->r = 255;
            w->g = 255;
            w->b = 255;
            w->a = 128;
            w->size = fadeRand(100) * 0.0002f + 0.03f;
            w->unused28 = 0;
            w->t = 0;
            break;
        case 1:
            frmDestroy(&((PlAwa *) FX(obj))->frame->f);
            break;
        case 0:
            m = w->frame;
            if (plIsInPlay(w->index) &&
                fadeViewWidth(gGameWork.view) <
                    m->pos[1] - PL(gGameWork.players[w->index])->height) {
                return -1;
            }
            pos.x = m->pos[0];
            pos.y = m->pos[1];
            pos.z = m->pos[2];
            if (rideTree2PointIn(&pos, &push)) {
                m->scale[1] = mathSin(w->t) * 0.015f + w->size;
                m->scale[0] = m->scale[2] =
                    mathSin(w->t + 1.5707963f) * 0.02f + w->size;
                m->pos[0] = mathCos(w->t) + w->x;
                m->pos[2] = mathSin(w->t) + w->z;
                m->pos[1] += 1.0f;
                w->t++;
                break;
            }
            return -1;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCycleType(gGfxDlHead++, G_CYC_1CYCLE);
            gDPSetRenderMode(gGfxDlHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->r, w->g, w->b, w->a);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0,
                              PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Lets a bubble out of the player whose task this is. */
Obj *plSuichuAwaCreate(Obj *task) {
    return tskCreate(gEffectTask, plSuichuAwa, 0, FX(task));
}

/* The ripple ("水面も"). plMoyaPlace places it and shows it. */
s32 plMoya(Obj *obj, s16 msg, void *arg) {
    PlMoya *w = FX(obj);
    s32 pad;
    MPolFrame *f;

    switch (msg) {
        case 2:
            obj->name = "水面も";
            if ((w->frame = f = (MPolFrame *) frmCreate(gFrmLayer8, gFrmKindMpol)) ==
                NULL) {
                PRINTF("plMoya: create failed\n");
                return -1;
            }
            f->dl2 = (Gfx *) diveRippleDl;
            floadGetSetRam((void **) f->segs, 0x128);
            f->f.flags &= ~2;
            w->scale = 0.1f;
            fadeAnimStart(&w->anim, gDiveRippleTexAnim, 0, &f->segs[3], 0x306);
            w->unused1C = 0;
            w->unused20 = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (w->frame->f.flags & 2) {
                fadeAnimStep(&w->anim);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 100);
            gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0, 0, 0, 0,
                              PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes the player's ripple. */
Obj *plMoyaCreate(Obj *task) {
    return tskCreate(task, plMoya, 0, FX(task));
}

/* Lays the ripple on the water where the player meets it, sized by how deep the player is;
 * hidden out of water and on a steep surface. */
void plMoyaPlace(Obj *moya, Player *p) {
    PlMoya *w;
    MPolFrame *f;
    s32 pad[2];
    f32 len;
    f32 k;
    f32 x;
    f32 z;

    if (moya != NULL) {
        w = MOYA(moya);
        f = w->frame;
        if ((p == NULL || p->waterObj == 0) && (p->flags & 0x10040014)) {
            f->f.flags &= ~2;
            return;
        }
        if (p->waterObj != 0) {
            len = plVecLen(p->toSurface);
            if (!(len < 0.01f)) {
                k = 1.0f / len;
                x = p->toSurface[0] * k;
                z = p->toSurface[2] * k;
                if (x * x + z * z <= 0.5f) {
                    if (p->bodyHeight < len) {
                        w->scale = (p->bodyHeight * 1.4f - len) * 0.025f;
                    } else if (p->bodyHeight * 0.2f < len) {
                        w->scale = 0.25f;
                    } else {
                        w->scale = (len + 5.0f) * 0.025f;
                    }
                    if (w->scale <= 0.001f) {
                        w->scale = 0.0f;
                    }
                    mathMtxIdent(f->mtx.m);
                    mathMtxPostRotX(f->mtx.m, 1.5707963f);
                    mathMtxLookDir(f->mtx.m, p->toSurface[0], -p->toSurface[2], p->toSurface[1]);
                    mathMtxPostTrans(f->mtx.m, p->surfacePos[0], p->surfacePos[1], p->surfacePos[2]);
                    mathMtxScaleRot(f->mtx.m, w->scale);
                    f->f.flags |= 2;
                    return;
                }
            }
        }
        f->f.flags &= ~2;
    }
}

/* The splash ring ("diveef"): pink for Yae, blue for Sasuke. It follows the player's model and
 * fades in and out by a sine. */
s32 diveef(Obj *obj, s16 msg, void *arg) {
    PlDiveEf *w = FX(obj);
    s32 pad;
    PolFrame *f;
    Player *p;
    AnimeFrame *m;
    f32 c;
    s32 pad2[2];

    switch (msg) {
        case 2:
            obj->name = "diveef";
            f = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if ((w->frame = f) == NULL) {
                return -1;
            }
            p = ((PlDiveEfInit *) arg)->p;
            m = p->model;
            w->index = p->index;
            w->height = ((PlDiveEfInit *) arg)->height;
            f->dl2 = (Gfx *) diveSplashRingDl;
            floadGetSetRam((void **) f->segs, 0x123);
            f->flags |= 7;
            f->pos[0] = m->pos[0];
            f->pos[1] = m->pos[1];
            f->pos[2] = m->pos[2];
            f->rot[2] = fadeRand(6283) * 0.001f;
            w->a = 0;
            fadeAnimStart(&w->anim, gDiveSplashRingTexAnim, 0, &f->segs[3], 0x301);
            if (p->chara == 3) {
                w->r = 255;
                w->g = 120;
                w->b = 140;
                f->scale[0] = f->scale[1] = f->scale[2] = 0.8f;
            } else {
                w->r = 100;
                w->g = 255;
                w->b = 255;
                f->scale[0] = f->scale[1] = f->scale[2] = 0.6f;
            }
            w->unused30 = 0;
            w->t = 0;
            break;
        case 1:
            frmDestroy(&((PlDiveEf *) FX(obj))->frame->f);
            break;
        case 0:
            if ((w->a = mathSinCos(w->t * 9, &c) * 230.0f) < 0) {
                return -1;
            }
            if (plIsInPlay(w->index)) {
                AnimeFrame *model;
                f = w->frame;
                model = PL(gGameWork.players[w->index])->model;
                f->pos[0] = model->pos[0];
                f->pos[1] = model->pos[1] + w->height;
                f->pos[2] = model->pos[2];
            }
            w->t++;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->r, w->g, w->b, w->a);
            gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0, 0, 0, 0,
                              PRIMITIVE, PRIMITIVE, 0, TEXEL0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Makes a splash ring over the player, height above its model. */
void plDiveefCreate(Player *p, f32 height) {
    Obj *task = gGameWork.players[p->index];
    PlDiveEfInit init;

    init.p = p;
    init.height = height;
    tskCreate(task, diveef, 0, &init);
}

/* Each step: splashes and their sounds as the player goes into and out of water. */
void plWaterSplash(Player *p) {
    f32 d;

    if ((u32) gGameWork.frames >= 5) {
        if (p->waterObj != 0 && p->prevWaterObj == 0) {
            if (p->action == 0x13 && p->chara == 1) {
                effectSibukiCreateWhite(p->model->pos[0], p->surfacePos[1], p->model->pos[2], 0.03f,
                                       0.05f, 0.0075f, 0.015f);
                plSound(p, 0x299);
                p->waterSe = 0;
                p->waterSeWait = 0;
                p->splashWait = 5;
            } else {
                effectSibukiCreateWhite(p->model->pos[0], p->surfacePos[1], p->model->pos[2], 0.004f,
                                       0.005f, 0.008f, 0.003f);
                plWaterSoundLater(p, 0x298, 3);
            }
        }
        if (p->waterObj != 0 && p->splashWait == 0) {
            d = fabsf(p->prevWaterDepth - p->waterDepth);
            if (d > 1.2f) {
                if (p->prevWaterDepth <= 16.0f && p->waterDepth > 16.0f) {
                    effectSibukiCreateWhite(p->model->pos[0], p->surfacePos[1], p->model->pos[2], 0.03f,
                                           0.05f, 0.0075f, sqrtf(d) * 0.0075f);
                    plSound(p, 0x299);
                    p->waterSe = 0;
                    p->waterSeWait = 0;
                    p->splashWait = 5;
                } else if (p->waterDepth < 20.0f && p->prevWaterDepth >= 20.0f) {
                    effectSibukiCreateWhite(p->model->pos[0], p->surfacePos[1], p->model->pos[2], 0.02f,
                                           0.005f, 0.01f, 0.003f);
                }
            }
        }
        if (p->splashWait != 0) {
            p->splashWait--;
        }
        if (p->waterSeWait <= 0) {
            if (p->waterSe != 0 && p->waterObj != 0) {
                if (p->waterDepth >= 6.0f && p->waterDepth < 16.0f) {
                    plSound(p, p->waterSe);
                    p->splashWait = 5;
                }
                p->waterSe = 0;
            }
            p->waterSeWait = 0;
        } else {
            p->waterSeWait--;
        }
    }
}

/* A sound to play after wait steps (if the player is then 6 to 16 deep). */
void plWaterSoundLater(Player *p, u16 se, s16 wait) {
    p->waterSe = se;
    p->waterSeWait = wait;
}
