#include "tlb.h"

/* The kkmaster's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct KkMasterObj {
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
    /* 0x34 */ s32 timer;
    /* 0x38 */ Vec3f home; /* where its kkoma are made */
    /* 0x44 */ s32 count; /* the next kkoma's number, 1 to 3 */
    /* 0x48 */ u8 on;
    /* 0x4C */ s32 calls; /* kkoma calls this step: its sound plays while any call */
} KkMasterObj;

#define KKMASTER(obj) ((KkMasterObj *) FX(obj))

/* What a kkoma is made with (eneMakeEx's c, mode 2): its kind (1-3) and where it rolls to. */
typedef struct KkomaArg {
    /* 0x00 */ s32 n;
    /* 0x04 */ Vec3f pos;
} KkomaArg;

/* A kkoma's work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct KkomaObj {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *unused0C;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *atk;
    /* 0x40 */ Obj *master; /* the kkmaster (message 10 with 0x15B) */
    /* 0x44 */ f32 speed;
    /* 0x48 */ f32 dir[3]; /* where it rolls; dir[1] is then the height it ends below */
    /* 0x54 */ f32 spin; /* its turn this step */
    /* 0x58 */ u8 pad58[2];
    /* 0x5A */ u8 ridden; /* a player stood on it this step (message 6) */
} KkomaObj;

#define KKOMA(obj) ((KkomaObj *) FX(obj))

extern Frame *gFrmLayer3;
extern Gfx kkomaN1Dl[];
extern Gfx kkomaN2Dl[];
extern Gfx kkomaN3Dl[];
extern u8 kkomaN1Ride[];
extern u8 kkomaN2Ride[];
extern u8 kkomaN3Ride[];
/* size 1's attack offsets: unused, size 1 takes no table (tab = NULL below) */
f32 kkomaAtkOfs1[5][3] = { 0.0f, 105.0f, -313.0f, 0.0f, -272.0f, -190.0f, 0.0f, -268.0f, 194.0f, 0.0f, 95.0f, 313.0f, 0.0f, 333.0f, 0.0f };
f32 kkomaAtkOfs2[5][3] = { 0.0f, 122.0f, -381.0f, 0.0f, -330.0f, -235.0f, 0.0f, -326.0f, 235.0f, 0.0f, 119.0f, 384.0f, 0.0f, 400.0f, 0.0f };
f32 kkomaAtkOfs3[5][3] = { 0.0f, 129.0f, -391.0f, 0.0f, -333.0f, -238.0f, 0.0f, -336.0f, 241.0f, 0.0f, 119.0f, 391.0f, 0.0f, 415.0f, 0.0f };

/* Message 6's argument (plmove.c's): a place on the course, moved by the object. */
typedef struct PlPlace {
    /* 0x0 */ s16 zone;
    /* 0x4 */ f32 place;
    /* 0x8 */ f32 side;
    /* 0xC */ f32 height;
} PlPlace;

void kkmasterCall(Obj *master);
void kkomaAtkHitSend(Obj *o, KkomaObj *w);
f32 kkomaGroundRay(Obj *o, f32 x, f32 y, f32 z, f32 r, f32 h);
void kkomaFall(Obj *o, KkomaObj *w, f32 r);

/* The "kkmaster" (kind proc): makes a kkoma (kind 0x82) at its home every 64 steps, numbers 1-3
 * in turn, while the camera looks along its course and is within 200; plays sound 0x15B while
 * any kkoma it can see calls it (kkmasterCall), and answers message 10 with 0x15B by itself. */
s32 kkmasterProc(Obj *o, s16 msg, void *arg) {
    KkMasterObj *w = KKMASTER(o);
    s32 *d;
    s32 pad;
    f32 x;
    f32 dy;
    f32 z;
    f32 hx;
    f32 hz;
    f32 c;
    f32 dist;
    KkomaArg a;
    CamView *v;

    switch (msg) {
        case 2:
            o->name = "kkmaster";
            d = (s32 *) eneSetup(o, arg);
            w->on = d[0];
            w->home.x = d[1];
            w->home.y = d[2];
            w->home.z = d[3];
            vramCoursePointXZ(w->zone, w->x, w->z, &x, &z);
            w->zone = 0;
            w->x = x;
            w->z = z;
            w->count = 1;
            break;
        case 1:
            sndPlay(0x815B);
            break;
        case 10:
            if (w->on != 0 && (s32) arg == 0x15B) {
                return (s32) o;
            }
            break;
        case 0:
            v = gGameWork.view;
            if (v != NULL) {
                if (w->on != 0) {
                    if (w->calls != 0) {
                        if (sndGetVolume(0x15B, 0) == 0) {
                            sndPlay(0x15B);
                        }
                    } else if (sndGetVolume(0x15B, 0) != 0) {
                        sndPlay(0x815B);
                    }
                    w->calls = 0;
                }
                x = v->at[0] - w->x;
                dy = v->at[1] - w->y;
                z = v->at[2] - w->z;
                hx = w->home.x - w->x;
                hz = w->home.z - w->z;
                c = (x * hx + z * hz) / sqrtf((x * x + z * z) * (hx * hx + hz * hz));
                dist = sqrtf(x * x + dy * dy + z * z);
                if (c < 0.7 && dist > 200.0f) {
                    tskDestroyChildren(o);
                } else if ((++w->timer & 0x3F) == 1) {
                    a.n = w->count;
                    a.pos = w->home;
                    eneMakeEx(o, ENE_KKOMA, w->zone, w->x, w->y, w->z, 2,
                              (u32 (*)(void *, void *, void *)) &a);
                    w->count++;
                    if (w->count >= 4) {
                        w->count = 1;
                    }
                }
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A kkoma in view calls its kkmaster this step. */
void kkmasterCall(Obj *master) {
    KKMASTER(master)->calls++;
}

/* The "kkoma" (kind 0x82): a model of three kinds that rolls from where it is made toward its
 * KkomaArg point, speeding up on the ground, carrying a player who stands on it (message 6),
 * rumbling the pads where it first lands, and ending below the point's height. */
s32 kkomaProc(Obj *o, s16 msg, void *arg) {
    KkomaObj *w = KKOMA(o);
    Gfx *dl;
    u8 *ride;
    s32 i;
    ACollFrame *b;
    s32 port;
    s32 pad;
    s32 r;
    f32 (*tab)[3];
    s32 n;
    KkomaArg *d;
    f32 x;
    f32 y;
    f32 z;
    s32 pad3;
    s32 pad4;
    CamView view;

    switch (msg) {
        case 2:
            o->name = "kkoma";
            d = (KkomaArg *) eneSetup(o, arg);
            switch (d->n) {
                default:
                case 1:
                    dl = kkomaN1Dl;
                    ride = kkomaN1Ride;
                    r = 100;
                    tab = NULL;
                    break;
                case 2:
                    dl = kkomaN2Dl;
                    ride = kkomaN2Ride;
                    r = 120;
                    tab = kkomaAtkOfs2; n = 5;
                    break;
                case 3:
                    dl = kkomaN3Dl;
                    ride = kkomaN3Ride;
                    r = 120;
                    tab = kkomaAtkOfs3; n = 5;
                    break;
            }
            if ((w->model = eneNewPol(o, dl)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmLayer3);
            if (ride != NULL) {
                w->ride = rideCreate(gRideTree1, (s32) o, &w->model->f,
                                     (RideData *) eneFileAddr(o, ride, 0));
            }
            if (tab != NULL) {
                w->atk = ((ACollFrame * (*) (Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)(
                    &ene_atk_root_frame->frame, &w->model->f, 0x154, 0, 0, 0);
            }
            if (w->atk != NULL) {
                w->atk->kind = 0x7D;
                for (i = 0; i < n; i++) {
                    b = ((ACollFrame * (*) (Frame *, Frame *, u16, s16, s16, s16, s32)) acollAtkAddXYZ)(
                        &w->atk->frame, &w->model->f, r, tab[i][0], tab[i][1], tab[i][2], 0);
                    if (b != NULL) {
                        b->kind = 0x7D;
                    }
                }
            }
            w->dir[0] = d->pos.x - w->x;
            w->dir[1] = 0.0f;
            w->dir[2] = d->pos.z - w->z;
            mathVecNormalizeB((Vec3f *) w->dir);
            w->rot[1] = mathAtan2(w->dir[0], w->dir[2]);
            w->speed = 0.0f;
            w->dir[1] = d->pos.y;
            w->master = (Obj *) tskSendMsg(gEneManager, 10, (void *) 0x15B);
            break;
        case 1:
            frmDestroy((Frame *) w->ride);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->model->f);
            break;
        case 6:
            if (arg != NULL) {
                vramCoursePointXZ(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place, 0.0f, &x, &z);
                x -= w->model->pos[0];
                z -= w->model->pos[2];
                y = ((PlPlace *) arg)->height - w->model->pos[1];
                mathVecRotateY(-(vramCourseHeading(((PlPlace *) arg)->zone, ((PlPlace *) arg)->place) - 1.5707964f),
                               &x, &y, &z);
                ((PlPlace *) arg)->place -= y * w->spin + w->speed;
                ((PlPlace *) arg)->height += x * w->spin + w->vel[1];
                w->ridden = 1;
            }
            break;
        case 0:
            if (w->master != NULL && gGameWork.view != NULL) {
                view = *gGameWork.view;
                view.fovy = 0.87266463f;
                if (soundInFovCheck(&view, w->model->pos[0], w->model->pos[1], w->model->pos[2])) {
                    kkmasterCall(w->master);
                }
            }
            if (w->y < w->dir[1]) {
                return -1;
            }
            if (w->hit & 1) {
                if (w->speed == 0.0f) {
                    if (soundIncamCam(0x10A, w->model->pos[0], w->model->pos[1],
                                      w->model->pos[2])) {
                        for (port = 0; port < 4; port++) {
                            if (gGameWork.players[port] != NULL) {
                                contPakRumbleStart(port, 0xAAAAAAAA, 15);
                            }
                        }
                    }
                }
                w->vel[1] = 0.0f;
                if (w->speed < 2.4) {
                    w->speed += 0.1;
                }
            }
            eneFall(o, 0.4f);
            kkomaFall(o, w, 60.0f);
            w->vel[0] = w->dir[0] * w->speed;
            w->vel[2] = w->dir[2] * w->speed;
            w->x += w->vel[0];
            w->z += w->vel[2];
            w->spin = w->speed / 60.0f;
            w->rot[0] += w->spin;
            if (w->ridden == 0) {
                kkomaAtkHitSend(o, w);
            }
            w->ridden = 0;
            eneSetModelOnCourse(o, &w->model->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Message 7 to whatever its attack boxes hit. */
void kkomaAtkHitSend(Obj *o, KkomaObj *w) {
    ACollFrame *b;
    Obj *by;

    if (w->atk != NULL) {
        if (acollAtkWasHit(w->atk) == 1 && (by = (Obj *) w->atk->hitPower) != NULL) {
            tskSendMsg(by, 7, o);
        }
        for (b = (ACollFrame *) w->atk->frame.child; b != NULL; b = (ACollFrame *) b->frame.next) {
            if (acollAtkWasHit(b) == 1 && (by = (Obj *) b->hitPower) != NULL) {
                tskSendMsg(by, 7, o);
            }
        }
    }
}

/* A ray down from r above (x, y, z) by the fall h: the fall cut to the ground if it is met
 * (and the on-ground bit set). */
f32 kkomaGroundRay(Obj *o, f32 x, f32 y, f32 z, f32 r, f32 h) {
    KkomaObj *w = KKOMA(o);
    s32 pad;
    Vec3f dir;
    Vec3f from;
    Vec3f at;
    f32 t;
    s32 pad2[2];

    dir.x = 0.0f;
    dir.y = h - r;
    dir.z = 0.0f;
    from.x = x;
    from.y = y + r;
    from.z = z;
    if (vramRayGround(&from, &dir, &at, &t)) {
        w->hit |= 1;
        h = at.y - (from.y - r);
    }
    return h;
}

/* Falling: the ground under its middle and 20 ahead and 30 behind on a round body of radius r. */
void kkomaFall(Obj *o, KkomaObj *w, f32 r) {
    f32 dy = w->vel[1];
    f32 x;
    f32 y;
    f32 z;

    w->hit &= ~1;
    if (dy <= 0.0f) {
        x = w->x;
        y = w->y;
        z = w->z;
        dy = kkomaGroundRay(o, x, y - r, z, r, dy);
        dy = kkomaGroundRay(o, w->dir[0] * 20.0f + x, y - r * r / sqrtf(r * r + 400.0f),
                                    w->dir[2] * 20.0f + z, r, dy);
        dy = kkomaGroundRay(o, x - w->dir[0] * 30.0f, y - r * r / sqrtf(r * r + 900.0f),
                                    z - w->dir[2] * 30.0f, r, dy);
    }
    w->y += dy;
}
