#include "enemy.h"
#include "camera.h"
#include "game.h"
#include "enecommon.h"
#include "eneproc.h"
#include "fade.h"
#include "gfxmgr.h"
#include "mathutil.h"
#include "person.h"
#include "plmain.h"
#include "polygon.h"
#include "scenario.h"
#include "shadow.h"
#include "sndloc.h"
#include "stage.h"
#include "tmanager.h"

/* The townspeople (overlay 9): personInit, personTalk, personAct (walking their paths and
 * turning to talk), personTalkStart, personTalking, personPlCollCheck (players push them
 * aside and stop them), and the talk camera (demoCamZoom_CL). The debug prints are compiled
 * out; their strings are in gPersonDebugStrs. PersonObj is in enemy.h. */

/* The game's debug print, compiled to nothing but its arguments: a call in the arguments
 * still runs (pakfiler.c's form). */
#define DPRINTF

extern s8 gRetracesPerFrame; /* retraces per frame */
extern Frame *ene_atk_root_frame; /* the enemies' attack boxes' parent */



/* The functions' prints are DPRINTF (FAKE: the game's literals): each points at its
 * string in gPersonDebugStrs's block, so no second copy lands in .rodata. */

/* personInit: a person's setup from message 2 (its table entry in main RAM with mode 0, in
 * its file with 1): its path, whether it is out at this hour, its model (anims[1]), its place
 * on the ground, its shadow and its talk box; -1 if it is not made. */
s32 personInit(Obj *o, u32 *anims, EneArg *arg, u8 mode) {
    PersonObj *p = PERSON(o);
    u32 data = (*eneSetup)(o, arg);

    p->mode = mode;
    if (p->mode == 0) {
        p->data = (PersonData *) stageAddrToRam(data);
        p->path = (u16 *) stageAddrToRam(p->data->path);
    } else if (p->mode == 1) {
        p->data = (PersonData *) fadeSegToRam(data, o->file);
        p->path = (u16 *) fadeSegToRam(p->data->path, o->file);
    }
    if (p->path == NULL) {
        DPRINTF("超人に行動パターンはいってません\n");
        return -1;
    }
    DPRINTF("personInit: talkNo[%d] *actStep[%d] r[%d] h[%d] k[%d] ry[%f]\n", p->data->talk, *p->path, p->data->radiusHeight[0], p->data->radiusHeight[1],
            p->data->hours, p->data->rotY);
    if (personHoursCheck(o, p->data->hours) == 0) {
        DPRINTF("ねむいよ〜〜\n");
        (*eneReleaseSpawn)(p->spawn);
        return -1;
    }
    p->anims = anims;
    p->animSpeed = 0.5f;
    if (personFlagCheck(o, 0x40) == 0) {
        p->model = (Frame *) (*eneNewAnime)(o, p->anims[1]);
    } else {
        p->model = (Frame *) (*eneAnimeModelMake)(o, p->anims[1], NULL, 0);
    }
    p->rot[1] = p->data->rotY;
    if ((*eneSetOnEarth)(o) == -1) {
        DPRINTF("personInit: Error: 地面にセット失敗\n");
        return -1;
    }
    p->shadow = (*shadowCreate)();
    p->box = (*acollAtkAddYZ)(ene_atk_root_frame, p->model, 100, 100, 0, 0);
    p->box->kind = 0x65;
    return 0;
}

/* The same with its table in main RAM. */
s32 personInitMainTable(Obj *o, u32 *anims, EneArg *arg) {
    return personInit(o, anims, arg, 0);
}

/* personTalk: the talk messages. Start (0): unless already talking, it turns to the player
 * (the talk camera on it). 3: whether it is talking. Standby (2): marks it talked to. The
 * prints (FAKE: the game's literals) point at their strings in gPersonDebugStrs's block. */
s32 personTalk(Obj *o, PersonTalk *t) {
    PersonObj *p = PERSON(o);
    s32 pad[2];

    switch (t->msg) {
        case 0:
            DPRINTF("personTalk: ene kind[%d] がENEPL_TALK_START を受け取ったぜ！\n", p->kind->kind);
            p->talker = t->player;
            if (personFlagCheck(o, 2) == 0) {
                personFlagOn(o, 2);
                eneSetForward(o, 1);
                if (p->data->camera == 0) {
                    personTalkCam(o);
                } else {
                    demoCamZoom_CL(p->talker);
                }
                p->state = 1;
                return 1;
            }
            return 0;
        case 3:
            p->count = 0;
            if (personFlagCheck(o, 2) == 1) {
                return 1;
            }
            return 0;
        case 2:
            DPRINTF("personTalk: ENEPL_TALK_STANDBY を受け取ったぜ！\n");
            personFlagOn(o, 1);
            break;
    }
    return 1;
}

/* personAct: a person's step. */
void personAct(Obj *o) {
    PersonObj *p = PERSON(o);
    s32 pad;
    Vec3f pos;

    (*eneStepHookPre)(o);
    switch (p->state) {
        case 0:
            (*personNormal)(o);
            break;
        case 1:
            (*personTurnToPlayer)(o);
            break;
        case 2:
            (*personTalkStart)(o);
            p->prev = gGameWork.partake;
            gGameWork.partake = 1;
            break;
        case 3:
            (*personTalking)(o);
            break;
        case 4:
            (*personTalkEnd)(o);
            gGameWork.partake = p->prev;
            break;
    }
    if (personFlagCheck(o, 4)) {
        if (personFlagCheck(o, 0x10)) {
            if ((*eneAnimPlayOnce)(o, (AnimeFrame *) p->model, p->animSpeed * -1.0f)) {
                personFlagOn(o, 8);
            }
        } else if ((*eneAnimLoop)(o, (AnimeFrame *) p->model, p->animSpeed * -1.0f)) {
            personFlagOn(o, 8);
        }
    } else if (personFlagCheck(o, 0x10)) {
        if ((*eneAnimPlayOnce)(o, (AnimeFrame *) p->model, p->animSpeed)) {
            personFlagOn(o, 8);
        }
    } else if ((*eneAnimLoop)(o, (AnimeFrame *) p->model, p->animSpeed)) {
        personFlagOn(o, 8);
    }
    personPlBlockCheck(o);
    (*eneMove)((EneObj *) p);
    (*eneFall)(o, 0.4f);
    p->hit &= ~1;
    if ((*eneLand)((EneObj *) p, 10.0f) == 1) {
        p->hit |= 1;
        p->vel[1] = 0.0f;
    }
    if (p->model != NULL) {
        (*eneSetModelOnCourse)(o, p->model);
    }
    if (personFlagCheck(o, 2) == 1) {
        if (++p->count >= 4) {
            DPRINTF("personAct: time out....\n");
            p->state = 4;
            personFlagOff(o, 2);
        }
    }
    (*eneGetWorldPos)(o, &pos);
    (*shadowPut)(p->shadow, pos.x, pos.y, pos.z, 1.0f, 0);
    (*eneStepHookPost)(o);
}

/* A person's end: its model, talk box and shadow. */
void personDelete(Obj *o) {
    PersonObj *p = PERSON(o);
    s32 pad;

    if (PERSON(o)->model != NULL) {
        (*frmDestroy)(p->model);
    }
    if (p->box != NULL) {
        (*frmDestroy)((Frame *) p->box);
    }
    if (p->shadow != NULL) {
        (*frmDestroy)((Frame *) p->shadow);
    }
}

/* Walking (state 0): a hit on its talk box passes message 7 to the hitter, and the path's
 * next command runs. */
void personNormal(Obj *o) {
    PersonObj *p = PERSON(o);
    Obj *by;
    u16 *path;
    u16 *base;

    if ((*acollAtkWasHit)(p->box) == 1) {
        by = (Obj *) p->box->hitPower;
        if (by != NULL) {
            (*tskSendMsg)(by, 7, o);
        } else {
            DPRINTF("personNormal: Please set playerD at damage frame. Please use taskSet\n");
        }
    }
    path = p->path;
    if (p->mode == 0) {
        base = (u16 *) stageAddrToRam(p->data->path);
    } else if (p->mode == 1) {
        base = (u16 *) fadeSegToRam(p->data->path, o->file);
    }
    p->path = (*personActStepRun)(o, base, path, p->anims);
}

/* personTalkStart (state 1): stops, and turns to the player (18 degrees a step); then 2. */
void personTurnToPlayer(Obj *o) {
    PersonObj *p = PERSON(o);
    s32 pad;
    Vec3f pos;

    p->vel[0] = 0.0f;
    p->vel[2] = 0.0f;
    if (p->talker != NULL) {
        tskSendMsg(p->talker, 4, &pos);
        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
            p->state = 2;
        }
    } else {
        DPRINTF("personTurnToPlayer: Please set playerD at damage frame. Please use taskSet\n");
    }
}

/* personTalkStart (state 2), once talked to: faces the player, and opens its talk
 * (scRunStart, with the player's two numbers) with its standing animation; then 3. No
 * scenario: 4. Not yet facing comes first: its string leads the function's in .rodata (IDO
 * emits strings in source order). The empty else's print is what places the talker's load at
 * the join. */
void personTalkStart(Obj *o) {
    PersonObj *p = PERSON(o);
    s32 pad;
    Vec3f pos;

    if (!personFlagCheck(o, 1)) {
        DPRINTF("personTalkStart: プレーヤーがまだ向いてません。\n");
    } else {
        if (p->data->talk == 0) {
            DPRINTF("personTalkStart: シナリオ番号がおかしいです。\n");
            p->state = 4;
            return;
        }
        if (p->talker != NULL) {
            tskSendMsg(p->talker, 4, &pos);
            if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 0) {
                DPRINTF("personTalkStart: プレーヤーに向いてません\n");
                return;
            }
        } else {
            DPRINTF("personTalkStart: Please set playerD at damage frame. Please use taskSet\n");
        }
        DPRINTF("personTalkStart: scNo[%d] ctrlPl[%d] ctrlKind[%d]\n", p->data->talk, plGetIndex(p->talker),
                plGetChara(p->talker));
        (*scRunStart)(p->data->talk, plGetIndex(p->talker) + 1,
                                  plGetChara(p->talker));
        if (p->data->camera == 0) {
            (*camSunSetClockStop)(1);
        }
        personAnimeSet(o, p->anims[0]);
        p->state = 3;
    }
}

/* personTalking (state 3): once the talk is over, the player is sent message 8; then 4. */
void personTalking(Obj *o) {
    PersonObj *p = PERSON(o);
    s32 pad;
    struct {
        Obj *o;
        s32 one;
    } arg;

    if ((*scIsRunning)() == 0) {
        arg.o = o;
        arg.one = 1;
        if (p->talker != NULL) {
            (*tskSendMsg)(p->talker, 8, &arg);
        } else {
            DPRINTF("personTalking: Please set playerD at damage frame. Please use taskSet\n");
        }
        p->state = 4;
    }
}

/* State 4: ends the talk: the camera back, its talk box on again, the walking animation;
 * then 0. */
void personTalkEnd(Obj *o) {
    PersonObj *p = PERSON(o);

    personFlagOff(o, 2);
    personFlagOff(o, 1);
    p->box->flags |= 4;
    eneSetForward(o, 0);
    (*camAiSetHold)(0);
    if (p->data->camera == 0) {
        (*camSunSetClockStop)(0);
    }
    if (p->data->camera == 1) {
        personTalkCamOff();
    }
    personAnimeSet(o, p->anims[1]);
    p->state = 0;
}

/* Sets its animation (from its end when backwards) and starts it over. */
void personAnimeSet(Obj *o, u32 anim) {
    PersonObj *p = PERSON(o);
    s32 pad;
    f32 frame;

    if (personFlagCheck(o, 4)) {
        frame = polAnimeLastFrame((AnimeFrame *) p->model);
    } else if (personFlagCheck(o, 0x20) == 0) {
        frame = 0.0f;
    } else {
        return;
    }
    ((AnimeFrame *) p->model)->anim = anim;
    ((AnimeFrame *) p->model)->frame = frame;
    personFlagOff(o, 0x10);
}

void personFlagOn(Obj *o, u8 mask) {
    PERSON(o)->flags |= mask;
}

void personFlagOff(Obj *o, u8 mask) {
    PERSON(o)->flags &= ~mask;
}

s32 personFlagCheck(Obj *o, u8 mask) {
    if (PERSON(o)->flags & mask) {
        return 1;
    }
    return 0;
}

/* Its talk box's kind. */
u8 personBoxKindGet(Obj *o) {
    return PERSON(o)->box->kind;
}

/* Runs a path command; the next one. The commands (u16 words):
 * 0 x z speed: walk there, turning (13 degrees a step), in its walking animation;
 * 1 n: go to command word n; 2 deg: stop and turn to deg; 3 i: animation anims[i];
 * 4 n: stand for n frames; 5 se: a sound at it; 6: stop;
 * 7 i: play anims[i] through once; 8 b: backwards (b 1) or not; 9 s: animation speed s / 10.
 * Each command's operands are locals of its case's block; the pads place them as the game's
 * frame has them. */
u16 *personActStepRun(Obj *o, u16 *base, u16 *path, u32 *anims) {
    PersonObj *p = PERSON(o);
    s32 pad[2];

    switch (path[0]) {
        case 0: {
            f32 x = (s16) path[1];
            f32 z = (s16) path[2];
            f32 speed = (s16) path[3];

            if (p->step == 0) {
                personAnimeSet(o, p->anims[1]);
                p->step++;
            }
            if (eneMoveToPos(o, x, z, speed) == 1) {
                p->step = 0;
                path += 4;
            }
            eneTurnToPoint(o, x, z, 13.0f);
            break;
        }
        case 1:
            path = base + (s16) path[1];
            break;
        case 2: {
            s32 pad2;
            f32 deg = (s16) path[1];

            p->vel[0] = 0.0f;
            p->vel[2] = 0.0f;
            if (eneTurnToAngle(o, deg, 13.0f) == 1) {
                path += 2;
            }
            break;
        }
        case 3: {
            u8 i = path[1];

            personAnimeSet(o, PERSON(o)->anims[i]);
            path += 2;
            break;
        }
        case 7: {
            u8 i = path[1];

            if (p->step == 0) {
                personFlagOff(o, 8);
                personAnimeSet(o, p->anims[i]);
                personFlagOn(o, 0x10);
            }
            if (++p->step >= 2 && personFlagCheck(o, 8)) {
                p->step = 0;
                path += 2;
            }
            break;
        }
        case 8: {
            u8 back = path[1];

            if (back == 0) {
                personFlagOff(o, 4);
                path += 2;
            } else {
                personFlagOn(o, 4);
                path += 2;
            }
            break;
        }
        case 9: {
            f32 s = (s16) path[1];

            path += 2;
            PERSON(o)->animSpeed = s / 10.0f;
            break;
        }
        case 4: {
            s16 n = path[1];

            if (p->step == 0) {
                personAnimeSet(o, p->anims[0]);
                p->vel[0] = 0.0f;
                p->vel[2] = 0.0f;
            }
            if (n < p->step++) {
                p->step = 0;
                path += 2;
            }
            break;
        }
        case 5: {
            s16 se = path[1];

            DPRINTF("SoundLocation [%f] [%f] [%f]\n", p->x, p->y, p->z);
            soundLocationCam(se, p->x, p->y, p->z);
            path += 2;
            break;
        }
        case 6: {
            s32 pad3;

            path += 1;
            p->vel[2] = 0.0f;
            p->vel[1] = 0.0f;
            p->vel[0] = 0.0f;
            break;
        }
        default:
            DPRINTF("actStep がへんです");
            break;
    }
    return path;
}

/* personPlCollCheck: pushes a player's move (vel) out of the person's cylinder: its radius + 5
 * across, from 20 under its feet to its height. The three tests are early returns (as one
 * && chain IDO keeps the player's y in a register instead of loading it again). */
void personPlCollCheck(Obj *o, Player *pl, Vec3f *vel) {
    s32 pad[2];
    Vec3f pos;
    Vec3f to;
    f32 r;
    f32 h;
    f32 d;
    f32 k;
    f32 dx;
    s32 pad2[3];

    if (PERSON(o)->data != NULL) {
        (*eneGetWorldPos)(o, &pos);
        r = PERSON(o)->data->radiusHeight[0];
        h = PERSON(o)->data->radiusHeight[1];
        to.x = pl->model->pos[0] + vel->x;
        to.y = pl->model->pos[1] + vel->y;
        to.z = pl->model->pos[2] + vel->z;
        if (r != 0.0f && h != 0.0f) {
            d = sqrtf((pos.z - to.z) * (pos.z - to.z) + (pos.x - to.x) * (pos.x - to.x));
            if (d == 0.0f) {
                d = 1.0f;
            }
            if (5.0f + r <= d) {
                return;
            }
            if (to.y + 20.0f <= pos.y) {
                return;
            }
            if (pos.y + h <= to.y) {
                return;
            }
            k = (5.0f + r - d) / d;
            dx = (to.x - pos.x) * k;
            vel->z += (to.z - pos.z) * k;
            vel->x += dx;
        }
    }
}

/* personPlCollCheck: whether player pl stands in the person's way: within its radius + 5 and
 * within 60 degrees of the way it moves. The pads place the locals as the game's frame has
 * them. */
s32 personPlInWay(Obj *o, Obj *pl) {
    PersonObj *p = PERSON(o);
    s32 pad;
    Vec3f pos;
    Vec3f to;
    f32 hi;
    f32 lo;
    f32 r;
    f32 h;
    f32 d;
    f32 m;
    s32 pad2;
    f32 a;
    s32 pad3;

    if (PERSON(o)->data == NULL) {
        return 0;
    }
    (*eneGetWorldPos)(o, &pos);
    r = p->data->radiusHeight[0];
    h = p->data->radiusHeight[1];
    plGetModelPos(pl, &to.x, &to.y, &to.z);
    if (r == 0.0f || h == 0.0f) {
        DPRINTF("personPlCollCheck: radius ZERO\n");
        return 0;
    }
    DPRINTF("personPlCollCheck: Warning eneR の値が異常[%d]\n", (s32) r);
    d = sqrtf((pos.z - to.z) * (pos.z - to.z) + (pos.x - to.x) * (pos.x - to.x));
    if (d == 0.0f) {
        DPRINTF("personPlCollCheck: dist is ZERO \n");
        d = 1.0f;
    }
    if (5.0f + r < d - 1.0f) {
        DPRINTF("***personPlCollCheck: Warning eneR の値が異常[%d]\n", (s32) r);
        return 0;
    }
    a = mathAtan2(to.x - pos.x, to.z - pos.z);
    if (a < 0.0f) {
        a += 6.2831855f;
    }
    m = mathAtan2(p->vel[0], p->vel[2]);
    if (m < 0.0f) {
        m += 6.2831855f;
    }
    hi = m + 1.0471976f;
    if (hi >= 6.2831855f) {
        hi -= 6.2831855f;
    }
    lo = m - 1.0471976f;
    if (m < 1.0471976f) {
        lo += 6.2831855f;
    }
    if (hi < m || m < lo) {
        if ((lo < a && a < 6.2831855f) || (a > 0.0f && a < hi)) {
            return 1;
        }
    } else if (lo < a && a < hi) {
        return 1;
    }
    return 0;
}

/* personPlCollCheck for each player in play: one in its way stops it. */
void personPlBlockCheck(Obj *o) {
    PersonObj *p = PERSON(o);
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) == 1) {
            if (personPlInWay(o, gGameWork.players[i])) {
                p->vel[0] = 0.0f;
                p->vel[2] = 0.0f;
            }
        }
    }
}

/* The talk camera between the person and its talker, 80 back. */
void personTalkCam(Obj *o) {
    personTalkCamAt(o, PERSON(o)->talker, 80);
}

/* The talk camera: at the middle of the person and the player, dist back the way the camera
 * already looks. dist is u16: its conversion to float takes the unsigned fix-up. */
void personTalkCamAt(Obj *o, Obj *player, u16 dist) {
    s32 pad[3];
    Vec3f pos;
    Vec3f mid;
    Vec3f dir;
    f32 dx;
    f32 dz;
    s32 pad2;
    CamView *v;
    Camera *cam;
    f32 d;

    (*plGetModelPos)(player, &pos.x, &pos.y, &pos.z);
    mid.x = (((AnimeFrame *) ENE(o)->model)->pos[0] + pos.x) / 2;
    mid.y = (((AnimeFrame *) ENE(o)->model)->pos[1] + pos.y) / 2;
    mid.z = (((AnimeFrame *) ENE(o)->model)->pos[2] + pos.z) / 2;
    (*camAiSetHold)(1);
    cam = (*camAiGetCam)();
    v = (*camGetView)(cam);
    dx = v->eye[0] - mid.x;
    dz = v->eye[2] - mid.z;
    d = sqrtf(dx * dx + dz * dz);
    dir.x = dx / d;
    dir.z = dz / d;
    (*camSetSpeed)(cam, 20 / gRetracesPerFrame);
    (*camMoveEye)(cam, dist * dir.x + mid.x, v->eye[1], dist * dir.z + mid.z);
    (*camMoveAt)(cam, mid.x, mid.y + 10.0f, mid.z);
}

/* Whether the person is out now, by the time of day (hours): 0 from 4:45 to 18:30, 1 outside
 * that, 3 outside 4:45-12:00, 4 outside 12:00-18:30, 5 outside 20:00-20:52, else always. */
s32 personHoursCheck(Obj *o, u8 hours) {
    switch (hours) {
        case 0:
            if (gGameWork.time >= 0x4C000 && gGameWork.time <= 0x128000) {
                return 1;
            }
            break;
        case 1:
            if (gGameWork.time < 0x4C000 || gGameWork.time >= 0x128000) {
                return 1;
            }
            break;
        case 3:
            if (gGameWork.time < 0x4C000 || gGameWork.time >= 0xC0000) {
                return 1;
            }
            break;
        case 4:
            if (gGameWork.time < 0xC0000 || gGameWork.time >= 0x128000) {
                return 1;
            }
            break;
        case 5:
            if (gGameWork.time < 0x140000 || gGameWork.time >= 0x14E000) {
                return 1;
            }
            break;
        default:
            return 1;
    }
    return 0;
}

/* demoCamZoom_CL: the talk camera closing in on the player: 30-230 away, framing narrower the
 * nearer. The game calls the library's sqrtf here, through t9 (`(*sqrtf)`); the quotient t is
 * a local (inline, the multiply takes the constant second). */
void demoCamZoom_CL(Obj *player) {
    s32 pad;
    Camera *cam;
    f32 d;
    CamView *v;
    f32 dx;
    f32 dz;
    f32 t;
    f32 x;
    f32 y;
    f32 z;
    s32 pad2[2];

    (*camAiSetHold)(1);
    cam = (*camAiGetCam)();
    v = (*camGetView)(cam);
    dx = v->eye[0] - v->at[0];
    dz = v->eye[2] - v->at[2];
    d = (*sqrtf)(dx * dx + dz * dz);
    DPRINTF("demoCamZoom_CL: dist[%f] target_view [%f]\n", d, v->fovy);
    if (d > 230.0f) {
        d = 230.0f;
    } else if (d < 30.0f) {
        d = 30.0f;
    }
    (*camSetSpeed)(cam, 20 / gRetracesPerFrame);
    t = (d - 30.0f) / 2;
    (*camMoveFov)(cam, -0.003f * t + 0.6f);
    (*plGetModelPos)(player, &x, &y, &z);
    (*camMoveAt)(cam, x, 15.0f, z);
}

/* The talk camera off: its view back to 40 degrees. */
void personTalkCamOff(void) {
    s32 pad;
    Camera *cam;

    (*camAiSetHold)(0);
    cam = (*camAiGetCam)();
    (*camGetView)(cam);
    (*camSetSpeed)(cam, 20 / gRetracesPerFrame);
    (*camMoveFov)(cam, 0.698132f);
}

/* demoCamZoom_CL, looking 25 up. */
void personCamZoomUp(Obj *player) {
    s32 pad;
    Camera *cam;
    f32 d;
    CamView *v;
    f32 dx;
    f32 dz;
    f32 t;
    f32 x;
    f32 y;
    f32 z;
    s32 pad2[2];

    (*camAiSetHold)(1);
    cam = (*camAiGetCam)();
    v = (*camGetView)(cam);
    dx = v->eye[0] - v->at[0];
    dz = v->eye[2] - v->at[2];
    d = (*sqrtf)(dx * dx + dz * dz);
    DPRINTF("demoCamZoom_CL: dist[%f] target_view [%f]\n", d, v->fovy);
    if (d > 230.0f) {
        d = 230.0f;
    } else if (d < 30.0f) {
        d = 30.0f;
    }
    (*camSetSpeed)(cam, 20 / gRetracesPerFrame);
    t = (d - 30.0f) / 2;
    (*camMoveFov)(cam, -0.003f * t + 0.6f);
    (*plGetModelPos)(player, &x, &y, &z);
    (*camMoveAt)(cam, x, 25.0f, z);
}

