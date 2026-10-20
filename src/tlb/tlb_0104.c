#include "tlb.h"

/* "wiseman": the wise man, a townsperson who walks a path of commands, turns to the player who
 * talks to him and runs his scenario (demo.c's talker, as tlb_0109's clerks: its first three
 * functions are theirs, at this work's offsets). His room's two tasks ("Etsk_wise_room_main")
 * only name themselves, the first starting tune 0x80. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The wise man's work, at his task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct WisemanW {
    /* 0x00 */ u8 state; /* 0 walking, 1 talked to, 2 turning, 3 talking */
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
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker;
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ u16 timer;
    /* 0x4A */ u16 step; /* frames into the command */
    /* 0x4C */ u16 *path; /* the command it is at */
} WisemanW;

#define WISEMAN(obj) ((WisemanW *) FX(obj))

/* The talk is over (demo.c's demoFinish): the talker is sent message 8, the talk box is on
 * again. */
void wisemanTalkFinish(Obj *o) {
    WisemanW *p = WISEMAN(o);
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

/* Its path, from its table entry. */
void wisemanSetPath(Obj *o, PersonData *d) {
    WISEMAN(o)->path = (u16 *) stageAddrToRam(d->path);
}

/* Runs a path command (personActStepRun's, without the animations); the next one.
 * 0 x z speed: walk there; 1 n: go to command word n; 2 deg: stop and turn to deg;
 * 4 n: stand for n frames; 5 se: a sound at it; 6: stop. */
u16 *wisemanActStep(Obj *o, u16 *base, u16 *path) {
    WisemanW *p = WISEMAN(o);
    s32 pad[2];

    switch (path[0]) {
        case 0: {
            f32 x = (s16) path[1];
            f32 z = (s16) path[2];
            f32 speed = (s16) path[3];

            if (eneMoveToPos(o, x, z, speed) == 1) {
                path += 4;
            }
            eneTurnToPoint(o, x, z, 13.0f);
            break;
        }
        case 1:
            path = base + (s16) path[1];
            break;
        case 2: {
            f32 deg = (s16) path[1];

            p->vel[0] = 0.0f;
            p->vel[2] = 0.0f;
            if (eneTurnToAngle(o, deg, 13.0f) == 1) {
                path += 2;
            }
            break;
        }
        case 3:
            DPRINTF("超人に行動パターンはいってません\n");
            break;
        case 4: {
            s16 n = path[1];

            if (p->step == 0) {
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

            soundLocationCam(se, p->x, p->y, p->z);
            path += 2;
            break;
        }
        case 6:
            path += 1;
            p->vel[2] = 0.0f;
            p->vel[1] = 0.0f;
            p->vel[0] = 0.0f;
            break;
        default:
            DPRINTF("actStep がへんです");
            break;
    }
    return path;
}

/* The file's own demoCamZoom_CL (person.c's): the talk camera closing in on the player, 30-230
 * away, framing narrower the nearer. */
void wisemanCamZoom(Obj *player) {
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

    camAiSetHold(1);
    cam = camAiGetCam();
    v = camGetView(cam);
    dx = v->eye[0] - v->at[0];
    dz = v->eye[2] - v->at[2];
    d = (*sqrtf)(dx * dx + dz * dz);
    if (d > 230.0f) {
        d = 230.0f;
    } else if (d < 30.0f) {
        d = 30.0f;
    }
    camSetSpeed(cam, 20 / gRetracesPerFrame);
    t = (d - 30.0f) / 2;
    camMoveFov(cam, -0.003f * t + 0.6f);
    DPRINTF("demoCamZoom_CL: dist[%f] target_view [%f]\n", d, t);
    plGetModelPos(player, &x, &y, &z);
    camMoveAt(cam, x, 15.0f, z);
}

/* The talk camera's view back to 40 degrees (the camera stays held). */
void wisemanCamFovReset(void) {
    s32 pad;
    Camera *cam;

    camAiSetHold(1);
    cam = camAiGetCam();
    camGetView(cam);
    camSetSpeed(cam, 20 / gRetracesPerFrame);
    camMoveFov(cam, 0.698132f);
}

/* The procs call these through their addresses (lui/addiu + jalr), wisemanCamFovReset with a jal. */
#define wisemanTalkFinish (*wisemanTalkFinish)
#define wisemanSetPath (*wisemanSetPath)
#define wisemanActStep (*wisemanActStep)
#define wisemanCamZoom (*wisemanCamZoom)

/* The talk starts: the players stand, the camera closes in on the talker, pausing is off. */
void wisemanTalkStart(Obj *o) {
    demoPlStandAll();
    wisemanCamZoom(WISEMAN(o)->talker);
    eneSetForward(o, 1);
    gGameWork.scenarioRunning = 2;
}

/* The talk ends: the players go, the camera's view back, pausing on. */
void wisemanTalkEnd(Obj *o) {
    wisemanTalkFinish(o);
    demoPlReleaseAll();
    wisemanCamFovReset();
    camAiSetHold(0);
    gGameWork.scenarioRunning = 0;
}

#define wisemanTalkStart (*wisemanTalkStart)
#define wisemanTalkEnd (*wisemanTalkEnd)

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern u8 etskWisemanState3Anim[];
extern u8 etskWisemanState2Anim[];

/* "wiseman" ("Etsk_wiseman_main"). */
s32 Etsk_wiseman_main(Obj *o, s16 msg, void *arg) {
    WisemanW *w = WISEMAN(o);
    s32 pad;
    u32 data;
    Vec3f pos;
    s32 pad2;
    u16 *path;
    u16 *base;

    switch (msg) {
        case 2:
            o->name = "wiseman";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) etskWisemanState3Anim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            wisemanSetPath(o, w->data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("Etsk_wiseman_main: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->path != NULL) {
                        path = w->path;
                        base = (u16 *) stageAddrToRam(w->data->path);
                        w->path = wisemanActStep(o, base, path);
                    }
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    wisemanTalkStart(o);
                    w->state = 2;
                    break;
                case 2:
                    w->vel[0] = 0.0f;
                    w->vel[2] = 0.0f;
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &pos);
                        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
                            w->model->anim = etskWisemanState2Anim;
                            w->model->frame = 0.0f;
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    if (!scIsRunning()) {
                        wisemanTalkEnd(o);
                        w->state = 0;
                        w->model->anim = etskWisemanState3Anim;
                        w->model->frame = 0.0f;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.8f);
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "wiseman" ("Etsk_wise_room_main"): his room; tune 0x80 unless it is on. */
s32 wisemanRoomProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "wiseman";
            DPRINTF("Etsk_wise_room_main START\n");
            if (sndGetMusic() != 0x80) {
                sndPlay(0x80);
            }
            break;
        case 1:
            break;
        case 0:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "wiseman" ("Etsk_wise_room_main"): the same, no tune. */
s32 wisemanRoom2Proc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "wiseman";
            DPRINTF("Etsk_wise_room_main START\n");
            break;
        case 1:
            break;
        case 0:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
