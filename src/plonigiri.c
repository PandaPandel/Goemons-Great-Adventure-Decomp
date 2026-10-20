#include "player.h"
#include "fade.h"
#include "file.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "itex.h"
#include "mathutil.h"
#include "plmain.h"
#include "plonigiri.h"
#include "plsound.h"
#include "plutil.h"
#include "polygon.h"
#include "tmanager.h"
#include "vram.h"

/* A bouncing jump, eating, the rice ball ("onigiri"), and two characters' own effects:
 * character 0's cloud ("goe雲") and character 3's bubble ("yae泡") (overlay 9). */

/* A bounce's work (Player.work). */
typedef struct PlHop {
    /* 0x0 */ f32 vx; /* across, as it went up */
    /* 0x4 */ f32 height;
    /* 0x8 */ f32 bounce; /* each bounce is 3/4 of the last */
    /* 0xC */ s32 count; /* steps since the last bounce */
} PlHop;

/* The shrink's work (Player.work). */
typedef struct PlShrink {
    /* 0x0 */ f32 scale;
} PlShrink;

/* The rice ball's task work. */
typedef struct PlOnigiri {
    /* 0x00 */ f32 vel[3];
    /* 0x0C */ PolFrame *frame;
    /* 0x10 */ ACollFrame *box;
    /* 0x14 */ s32 index; /* the player's */
    /* 0x18 */ s32 unused18;
    /* 0x1C */ s32 kind; /* the box's: 0x6D and up, by the food */
} PlOnigiri;

/* What an itex is made with (as itex.c's). */
typedef struct ItexInit {
    /* 0x00 */ s32 file;
    /* 0x04 */ Gfx *dl;
    /* 0x08 */ AnimFrame *frames;
    /* 0x0C */ Color3 prim;
    /* 0x0F */ Color3 env;
    /* 0x14 */ s32 flags;
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ f32 scale[3];
    /* 0x30 */ Vec3f vel;
    /* 0x3C */ Vec3f grow;
    /* 0x48 */ s32 alpha;
    /* 0x4C */ s32 fade;
    /* 0x50 */ s32 life;
} ItexInit;

/* Character 3's bubble. */
typedef struct PlBubble {
    /* 0x00 */ s32 index;
    /* 0x04 */ AnimeFrame *frame;
    /* 0x08 */ s32 unused08;
    /* 0x0C */ s32 r;
    /* 0x10 */ s32 g;
    /* 0x14 */ s32 b;
    /* 0x18 */ s32 a;
    /* 0x1C */ s32 t;
} PlBubble;

/* Character 0's cloud: eight puffs around the model. */
typedef struct PlCloud {
    /* 0x00 */ s32 index;
    /* 0x04 */ PolFrame *puff[8];
    /* 0x24 */ Anim anim[8];
    /* 0xA4 */ f32 angle[8];
    /* 0xC4 */ s32 unusedC4;
    /* 0xC8 */ s32 r;
    /* 0xCC */ s32 g;
    /* 0xD0 */ s32 b;
    /* 0xD4 */ s32 a;
    /* 0xD8 */ s32 t;
} PlCloud;

#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))

extern ACollFrame *ene_atk_root_frame; /* the item boxes */
extern Obj *gEffectTask; /* the effect manager */
extern Frame *gFrmModelLayer;
extern Frame *gFrmLayer8;
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "anime" */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern s8 gPlayerStats[]; /* per player, 10 bytes each; byte 4: the food held */
extern AnimFrame gItexPuffAnim[];
extern ItexInit gItexTemplate; /* the itex template */
extern u8 plGoeKumoPuffDl[]; /* the cloud puff's list */
extern u8 plYaeBubbleAnim[]; /* the bubble's animation */
/* plonigiri.c's .data (0x801A7220-0x801A7290). */
u16 gPlVoiceShrinkTable[] = { 0x0396, 0x0375, 0x03B5, 0x03D2 }; /* per character: voices */
u16 gPlVoiceDownFallTable[] = { 0x0561, 0x0560, 0x0562, 0x0563 };
u16 gPlVoiceDown2FTable[] = { 0x0561, 0x0560, 0x0562, 0x0563 };
u16 gPlVoiceDamageTable[] = { 0x053F, 0x0533, 0x054C, 0x0558 };
Gfx *gPlFoodDlTable[] = { NULL, (Gfx *) 0x03000828, (Gfx *) 0x03001270,
                               (Gfx *) 0x03001AC0 }; /* per food: its list */
/* Then plActCharaSpecial's static place (0x801A7250), and after that function the speed and the
 * cloud puff's animation: IDO lays .data out by each variable's FIRST declaration, so these two
 * are declared no earlier than the static. */

/* These actions' default: an empty switch on messages 0, 1 and 2. IDO keeps only the first
 * compare, the game's branch on msg to the next instruction (beqz a1); with cases 0 and 1 alone
 * nothing is left, and with 0 to 3 a second compare (li at,1; beq) stays. */
void plActNoneDefault(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
    }
}

/* A bouncing jump, 10 high: each bounce is 3/4 of the last, the stick steers once it has
 * been in the air 6 steps, and A leaves it.
 * The side test reads the hit box's 0x70 float into its own local d (the original's f0; inline
 * it took a rotation temp and shifted every float register after it); d takes one of pad's
 * slots, so the frame stays 0x48. */
void plActDamage(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;
    PlHop *w = (PlHop *) p->work;
    s32 pad[2];
    f32 sx;
    f32 d;

    switch (msg) {
        case 0:
            if (plDamage(p, p->damage, p->weaponDamage)) {
                if (plIsInWater(p)) {
                    next = 0x2C;
                } else {
                    next = 0x2B;
                }
                plActChange(p, next);
                return;
            }
            w->height = 10.0f;
            w->bounce = 1.0f;
            if (p->hitBy != 0) {
                d = *(f32 *) &p->dmg->hitInfo;
                p->pushX = w->vx =
                    ((p->model->pos[0] <= p->dmg->hitPos[0] - d) ? -60.0f : 60.0f) - p->velX;
            } else {
                w->vx = 0.0f;
            }
            p->jumpPush = plJumpSpeed(w->height) - p->velY;
            w->count = 0;
            plVoiceDamage(p);
            plInvincibleSet(p, 100);
            break;
        case 1:
            if (w->count >= 5 && plIsOnGround(p)) {
                if (p->actionSteps >= 6 && 1.7f < fabsf(sx = plStickSpeedX(p))) {
                    p->pushX = sx * 0.5f;
                    if (p->actionSteps >= 6 && 1.7f < fabsf(sx = plStickSpeedX(p))) {
                        p->pushX = sx * 0.75f;
                    }
                } else if (w->bounce >= 0.1f) {
                    p->pushX = w->vx * w->bounce;
                }
                w->bounce *= 0.75f;
                p->jumpPush += plJumpSpeed(w->height * w->bounce) - p->velY;
                w->count = 0;
            }
            if (p->held & A_BUTTON) {
                plInvincibleSet(p, 0x4B);
                plBlinkSet(p, 0x4B);
                if (plActChange(p, 0xE)) {
                    return;
                }
            }
            if (plAnimEnded(p)) {
                next = 0x31;
            }
            w->count++;
            plActChange(p, next);
            return;
        case 2:
            break;
        case 3:
            break;
    }
    plActNoneDefault(p, msg, a, b);
}

void plActDamageEnd(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            plInvincibleSet(p, 0x4B);
            plBlinkSet(p, 0x4B);
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = 0;
            }
            plActChange(p, next);
            return;
        case 2:
            break;
        case 3:
            break;
    }
    plActNoneDefault(p, msg, a, b);
}

void plActNull(Player *p, s32 msg, s32 a, s32 b) {
    plActNoneDefault(p, msg, a, b);
}

void plActDamageWater(Player *p, s32 msg, s32 a, s32 b) {
    s32 next = -1;

    switch (msg) {
        case 0:
            if (plDamage(p, p->damage, p->weaponDamage)) {
                plActChange(p, plIsInWater(p) ? 0x2C : 0x2B);
                return;
            }
            plVoiceDamage(p);
            plInvincibleSet(p, 0x4B);
            break;
        case 1:
            if (plAnimEnded(p)) {
                next = 0x19;
            }
            plActChange(p, next);
            return;
        case 2:
            plInvincibleSet(p, 0x4B);
            plBlinkSet(p, 0x4B);
            break;
        case 3:
            break;
    }
    plActNoneDefault(p, msg, a, b);
}

/* The eating actions' default. */
void plActDownDefault(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            p->onigiri = NULL;
            p->flags |= 0x200;
            p->flags2 |= 4;
            break;
        case 2:
            p->flags &= ~0x200;
            p->flags2 &= ~4;
            p->onigiri = NULL;
            break;
    }
    plBaseDefault(p, msg, a, b);
}

/* Eating: when the animation ends, the rice ball is thrown up. */
void plActDown(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            switch (p->state) {
                case 0:
                    if (plAnimEnded(p)) {
                        p->state = 1;
                        p->onigiri = plOnigiriCreate(p);
                    }
                    break;
                case 1:
                    break;
            }
            return;
        case 0:
            plInvincibleSet(p, 0x708);
            plBlinkSet(p, 0);
            plEatVoice(p);
            plSound(p, 0x114);
            break;
        case 2:
            plInvincibleSet(p, 0x4B);
            break;
    }
    plActDownDefault(p, msg, a, b);
}

void plActDownWater(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            if (p->actionSteps == 60) {
                p->onigiri = plOnigiriCreate(p);
            }
            return;
        case 0:
            plBlinkSet(p, 0);
            plFormSizeSet(p, 2);
            plEatVoice(p);
            plSound(p, 0x114);
            break;
        case 2:
            plFormSizeSet(p, 0);
            break;
    }
    plActDownDefault(p, msg, a, b);
}

/* Shrinking down (y) by 0.1 a step, then throwing the rice ball. */
void plActDownShrink(Player *p, s32 msg, s32 a, s32 b) {
    PlShrink *w = (PlShrink *) p->work;

    switch (msg) {
        case 1:
            if (p->state == 0) {
                p->model->scale[1] = w->scale * 0.1f;
                w->scale -= 0.1f;
                if (w->scale <= 0.2f) {
                    p->state = 1;
                    p->onigiri = plOnigiriCreate(p);
                }
            }
            return;
        case 0:
            plDamage(p, -1, -1);
            p->model->frame = polAnimeLastFrame(p->model);
            plBlinkSet(p, 0);
            plFormSizeSet(p, 2);
            w->scale = 1.0f;
            plVoice(p, gPlVoiceShrinkTable[p->chara]);
            break;
        case 2:
            plFormSizeSet(p, 0);
            p->model->scale[1] = 0.1f;
            break;
    }
    plActDownDefault(p, msg, a, b);
}

void plActDownFall(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            plDamage(p, -1, -1);
            p->flags |= 0x12000000;
            p->flags2 |= 1;
            if (!(p->flags & 0x200)) {
                plVoice(p, gPlVoiceDownFallTable[p->chara]);
            }
            break;
        case 2:
            p->flags &= ~0x12000000;
            p->flags2 &= ~1;
            break;
    }
    plActDownDefault(p, msg, a, b);
}

void plActDown2F(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 1:
            return;
        case 0:
            p->flags |= 0x20;
            plInvincibleSet(p, -1);
            plDamage(p, -1, -1);
            plVoice(p, gPlVoiceDown2FTable[p->chara]);
            break;
        case 2:
            p->flags &= ~0x10000020;
            plInvincibleSet(p, 0);
            break;
    }
    plActDownDefault(p, msg, a, b);
}

void plVoiceDamage(Player *p) {
    plVoice(p, gPlVoiceDamageTable[p->chara]);
}

/* Throws a rice ball: a task under the player's. */
Obj *plOnigiriCreate(Player *p) {
    return fadeCreateRetry(gGameWork.players[p->index], onigiriProc, 1, 0,
                                  p);
}

/* The rice ball: the food the player holds, 100 above it, falling 1.5 a step and turning
 * pi/8. Whatever it hits gets message 7; below -500 it goes. */
s32 onigiriProc(Obj *obj, s16 msg, void *arg) {
    PlOnigiri *w = FX(obj);
    Player *p = arg;
    s32 pad[2];
    PolFrame *f;
    s32 food;
    PolFrame *g;
    Obj *hit;

    switch (msg) {
        case 2:
            obj->name = "onigiri";
            w->frame = NULL;
            w->box = NULL;
            food = gPlayerStats[p->index * 10 + 4];
            if (food == 0) {
                return -1;
            }
            if ((w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) == NULL) {
                return -1;
            }
            f = w->frame;
            w->index = p->index;
            f->segs[0] = fileRamAddr(0x2E7);
            f->dl2 = gPlFoodDlTable[food];
            f->flags |= 0x20;
            f->scale[0] = 0.15f;
            f->scale[1] = 0.15f;
            f->scale[2] = 0.15f;
            f->pos[0] = p->model->pos[0];
            f->pos[2] = p->model->pos[2];
            f->pos[1] = p->model->pos[1] + 100.0f;
            if ((w->box = acollAtkAddYZ((Frame *) ene_atk_root_frame, &w->frame->f, 50, 0, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->box->kind = w->kind = food + 0x6D;
            w->vel[1] = -1.5f;
            w->vel[0] = 0.0f;
            gPlayerStats[p->index * 10 + 4] = 0;
            break;
        case 0:
            g = w->frame;
            g->pos[0] += w->vel[0];
            g->pos[1] += w->vel[1];
            g->pos[2] += w->vel[2];
            g->rot[1] += 0.39269909f;
            if (acollAtkWasHit(w->box) == 1 && (hit = (Obj *) w->box->hitPower) != NULL) {
                tskSendMsg(hit, 7, obj);
                return -1;
            }
            if (g->pos[1] < -500.0f) {
                return -1;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->box->frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* __plOnigiri(): only its prints, compiled out. */
void __plOnigiri(void) {
    PRINTF("__plOnigiri()\n");
    PRINTF("__plOnigiri(): %08x\n");
}

/* The characters' own mode (120 steps; A, B, Z or R end it after 4): character 0 rides its
 * cloud, 3 its bubble, and 2 leaves a trail of effects. The place is a function-local static:
 * as1 loads its fields through one "lui at" (D+0/D+8/D+4). As a global, or a file-scope
 * static, IDO keeps its address in v1 (IDO_NOTES 38). */
void plActCharaSpecial(Player *p, s32 msg, s32 a, s32 b) {
    static Vec3f gPlTrailPuffPos = { -3.0f, 7.0f, 0.0f }; /* a place */
    extern f32 gPlTrailPuffVel[3]; /* a speed (x, y, z) */
    s32 next;
    f32 dir;

    switch (msg) {
        case 0:
            p->flags |= 0x400000;
            plInvincibleSet(p, 120);
            plBlinkSet(p, 120);
            if (p->flags3 & 1) {
                plJoinSdSet(p);
                p->flags3 &= ~1;
            } else {
                plRestartSdSet(p);
            }
            switch (p->chara) {
                case 0:
                    p->cloud = plGoeKumoCreate(gGameWork.players[p->index]);
                    break;
                case 3:
                    p->bubble = plYaeAwaCreate(gGameWork.players[p->index]);
                    break;
            }
            break;
        case 1:
            next = -1;
            p->pushX = plStickSpeedX(p);
            p->pushY = plStickSpeedY(p);
            if ((p->actionSteps >= 4 && (p->held & (A_BUTTON | B_BUTTON | Z_TRIG | R_TRIG))) ||
                p->actionSteps >= 121) {
                next = 0xF;
            }
            if (plActChange(p, next)) {
                plInvincibleSet(p, 60);
                plBlinkSet(p, 60);
                return;
            }
            switch (p->chara) {
                case 0:
                    p->targetFacing = p->facing = -1.5707962f;
                    return;
                case 1:
                    p->targetFacing = p->facing = -1.5707962f;
                    return;
                case 2:
                    plFaceMoveDir(p, 0.5235988f);
                    if (!(p->actionSteps & 3)) {
                        dir = vramCourseHeading(p->zone, p->place);
                        mathVecRotate(p->lean, p->facing + dir, p->roll,
                                               &gPlTrailPuffPos.x, &gPlTrailPuffPos.y,
                                               &gPlTrailPuffPos.z);
                        mathVecRotate(p->lean, p->facing + dir, p->roll,
                                               &gPlTrailPuffVel[0], &gPlTrailPuffVel[1],
                                               &gPlTrailPuffVel[2]);
                        plItexPuff(p->model->pos[0] + gPlTrailPuffPos.x,
                                               p->model->pos[1] + gPlTrailPuffPos.y,
                                               p->model->pos[2] + gPlTrailPuffPos.z,
                                               (Vec3f *) gPlTrailPuffVel, 0.03f);
                    }
                    return;
                case 3:
                    p->targetFacing = p->facing = -1.5707962f;
                    return;
            }
            return;
        case 2:
            switch (p->chara) {
                case 0:
                    p->cloud = tskDestroy(p->cloud);
                    break;
                case 3:
                    p->bubble = tskDestroy(p->bubble);
                    break;
            }
            p->flags &= ~0x400000;
            if (p->pushX >= 0.0f) {
                p->targetFacing = 0.0f;
            } else {
                p->targetFacing = 3.1415927f;
            }
            break;
    }
    plBaseDefault(p, msg, a, b);
}

f32 gPlTrailPuffVel[3] = { -0.3f, -1.0f, 0.0f };
AnimFrame gPlCloudPuffAnim[] = { /* the cloud puff's animation */
    { 0x03000530, 4 }, { 0x03000D30, 4 }, { 0x03001530, 4 }, { 0x03000D30, 4 }, { 0, 0 },
};

void plActToStand(Player *p, s32 msg, s32 a, s32 b) {
    if (msg == 1) {
        plActChange(p, 0);
        return;
    }
    plBaseDefault(p, msg, a, b);
}

/* An itex puff (file 0x123) at (x, y, z), moving by v, growing by 1.02 a step.
 * The float registers (1.02 in f0, z in f4, size in f6) need pos[2] written before the three
 * scale stores, each on its own line; the scheduler still stores pos[2] after them. */
void plItexPuff(f32 x, f32 y, f32 z, Vec3f *v, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexPuffAnim;
    init.flags = 3;
    init.env = init.prim;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    init.vel = *v;
    init.grow.x = init.grow.y = init.grow.z = 1.02f;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* Character 3's bubble ("yae泡"): an animation following the model, wobbling in height, drawn
 * translucent without z (G_RM_XLU_SURF). w is set at its declaration; each case has its own
 * frame and model locals (the game spills them in two separate pairs of slots). */
s32 plYaeAwa(Obj *obj, s16 msg, void *arg) {
    PlBubble *w = FX(obj);
    s32 pad;

    switch (msg) {
        case 2: {
            AnimeFrame *f;
            AnimeFrame *m;

            obj->name = "yae泡";
            if ((f = w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime)) ==
                NULL) {
                return -1;
            }
            w->index = ((Player *) arg)->index;
            m = ((Player *) arg)->model;
            f->anim = plYaeBubbleAnim;
            floadGetSetRam((void **) f->segs, 0x128);
            plFramePosCopy(w->frame, m);
            f->scale[0] = f->scale[2] = 0.16f;
            f->scale[1] = 0.1f;
            w->r = 255;
            w->g = 255;
            w->b = 255;
            w->a = 128;
            w->t = 0;
            break;
        }
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0: {
            AnimeFrame *m;
            AnimeFrame *f;

            if (!plIsInPlay(w->index)) {
                return -1;
            }
            f = w->frame;
            m = PL(gGameWork.players[w->index])->model;
            plFramePosCopy(f, m);
            f->scale[1] = mathSin(w->t >> 1) * 0.01f + 0.12f;
            f->pos[1] += 1.0f - mathSin(w->t >> 1);
            f->f.flags = m->f.flags;
            w->t++;
            break;
        }
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

Obj *plYaeAwaCreate(Obj *task) {
    return tskCreate(task, plYaeAwa, 0, FX(task));
}

/* Character 0's cloud ("goe雲"): eight animated puffs going round the model, 7.5 out, pi/10
 * a step. The angle in int literals (a real mul.s by 2 and div.s by 8), the model's place read as
 * Vec3f fields (loaded after the product), and the clear as "puff[i++] = NULL". w is set at its
 * declaration (IDO_NOTES 132): IDO sinks obj + 0x20 into each case from the home slot, and in
 * case 0 keeps it in s6 for the first read (lw a0,0(s6)) instead of folding it. */
s32 plGoeKumo(Obj *obj, s16 msg, void *arg) {
    PlCloud *w = FX(obj);
    PolFrame *f;
    AnimeFrame *m;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "goe雲";
            for (i = 0; i < 8;) {
                w->puff[i++] = NULL;
            }
            w->index = ((Player *) arg)->index;
            for (i = 0; i < 8; i++) {
                f = w->puff[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                /* The game tests the array, not the frame it made. */
                if (w->puff == NULL) {
                    return -1;
                }
                f->dl2 = (Gfx *) plGoeKumoPuffDl;
                floadGetSetRam((void **) f->segs, 0x123);
                fadeAnimStart(&w->anim[i], gPlCloudPuffAnim, fadeRand(4),
                                       &f->segs[3], 0x301);
                f->scale[0] = f->scale[1] = f->scale[2] = 0.15f;
                f->flags |= 7;
                w->angle[i] = (f32) i * 3.1415927f * 2 / 8;
                w->r = 230;
                w->g = 230;
                w->b = 230;
                w->a = 200;
            }
            w->t = 0;
            break;
        case 1:
            for (i = 0; i < 8; i++) {
                frmDestroy(&w->puff[i]->f);
            }
            break;
        case 0:
            if (!plIsInPlay(w->index)) {
                return -1;
            }
            m = PL(gGameWork.players[w->index])->model;
            for (i = 0; i < 8; i++) {
                f = w->puff[i];
                fadeAnimStep(&w->anim[i]);
                f->pos[0] = mathCos(w->angle[i]) * 7.5f + ((Vec3f *) m->pos)->x;
                f->pos[1] = m->pos[1];
                f->pos[2] = mathSin(w->angle[i]) * 7.5f + ((Vec3f *) m->pos)->z;
                f->f.flags = m->f.flags;
                w->angle[i] += 0.31415927f;
            }
            w->t++;
            break;
        case 3:
            for (i = 0; i < 8; i++) {
                w->puff[i]->dl = gGfxDlHead;
            }
            gDPPipeSync(gGfxDlHead++);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->r, w->g, w->b, w->a);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0,
                              PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *plGoeKumoCreate(Obj *task) {
    return fadeCreateRetry(task, plGoeKumo, 2, 0, FX(task));
}
