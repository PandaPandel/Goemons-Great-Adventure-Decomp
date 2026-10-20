#include "player.h"
#include "camera.h"
#include "enemy.h"
#include "fade.h"
#include "file.h"
#include "fileload.h"
#include "fileres.h"
#include "gfxmgr.h"
#include "heap.h"
#include "mathutil.h"
#include "stage.h"
#include "tmanager.h"
#include "vram.h"

/* The enemies (overlay 9): the manager task ("Enemy", gEneManager) with five base tasks
 * ("E_Base") the enemies go under, the stage's kinds (loaded from file 0xC's table), and the
 * spawn list: each placement from the stage data appears when the camera comes within its
 * class's reach (and is heading towards it), in its hours of the day, and goes again when the
 * camera is 1.1 times that reach away. An enemy's task has an EneObj as its work. */

#define ENEW(obj) ((EneWork *) FX(obj))

/* enemy.c's .data (0x801B4500-0x801B4518, then IDO's 16-byte alignment), in address order. */
Obj *gEneManager = NULL; /* the manager */
s32 gEneSpawnMinDist = 100; /* how near the camera a spawn may be (0 at the start of a stage) */
f32 gEneClassReachTable[4] = { 150.0f, 250.0f, 400.0f, 30000.0f }; /* by class: the reach */
extern s16 gEneLastCamX; /* the camera's place last time (x, z) */
extern s16 gEneLastCamZ;
extern s32 gGameFrameCount; /* the frame counter */
extern Frame *gFrmModelLayer;
extern Frame *gFrmLayer2;
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "anime" */
extern s16 FRM_Dmg;

/* The manager's handler. */
s32 eneManagerProc(Obj *obj, s16 msg, void *arg) {
    EneWork *w = ENEW(obj);
    s32 i;

    switch (msg) {
        case 2:
            if (gEneManager != NULL) {
                return -1;
            }
            gEneManager = obj;
            obj->name = "Enemy";
            for (i = 0; i < 5; i++) {
                w->bases[i] = tskCreate(obj, tskFreeBlockProc, 0, 0);
                w->bases[i]->name = "E_Base";
            }
            break;
        case 1:
            if (w->kinds != NULL) {
                heapFree(w->kinds);
            }
            if (w->spawns != NULL) {
                heapFree(w->spawns);
            }
            gEneManager = NULL;
            break;
        case 0:
            if (w->kinds != NULL && w->spawns != NULL) {
                if (w->mode == 1) {
                    if (w->forward != NULL) {
                        tskSendMsg(w->forward, msg, arg);
                    }
                    return 0;
                }
                switch ((u32) gGameFrameCount & 7) {
                    case 5:
                        eneSpawnInReach(w);
                        break;
                    case 6:
                        eneDespawnFarAll(obj);
                        break;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Base task i. */
Obj *eneGetBase(s32 i) {
    EneWork *w = ENEW(gEneManager);

    return w->bases[i];
}

/* Makes the manager. */
void eneMakeManager(Obj *parent) {
    tskCreate(parent, eneManagerProc, 0, 0);
}

/* Spawns what is around the camera now, at any distance (a stage's start). */
void eneSpawnStart(void) {
    EneWork *w = ENEW(gEneManager);

    if (w->kinds != NULL && w->spawns != NULL) {
        gEneSpawnMinDist = 0;
        gEneLastCamX = gGameWork.view->at[0];
        gEneLastCamZ = gGameWork.view->at[2];
        eneSpawnInReach(w);
        gEneSpawnMinDist = 100;
    }
}

/* Lets the enemies appear (on 1) or not. */
void eneEnableSpawn(s32 on) {
    EneWork *w = ENEW(gEneManager);

    if (w->kinds != NULL && w->spawns != NULL) {
        if (on == 1) {
            w->paused = 0;
            return;
        }
        w->paused = 1;
    }
}

/* Makes the placements that have come into reach: not made yet, their wait over, in their
 * hours (the hour is the time of day's high half), and (but for class 3, always) within their
 * class's reach and beyond gEneSpawnMinDist, with the camera heading towards them. A made enemy gets
 * its placement's number; the placement waits 56 turns before it can appear again. The
 * distance is written out from the spawn's fields, not from dx/dz: the game squares them
 * twice (no shared dx*dx) and keeps a copy of each difference. The view read before the test
 * is the slot above cy. */
void eneSpawnInReach(EneWork *w) {
    f32 cx;
    CamView *view;
    f32 cy;
    f32 cz;
    f32 dx;
    f32 dz;
    f32 mx;
    f32 mz;
    f32 heading;
    f32 dist;
    EneSpawn *s;
    s32 hour;
    s32 i;
    Obj *o;

    view = gGameWork.view;
    if (w->paused != 1 && view != NULL) {
        cx = (s16) view->at[0];
        cy = (s16) view->at[1];
        cz = (s16) view->at[2];
        for (i = 0; i < w->count; i++) {
            s = &w->spawns[i];
            if (s->flags & 0x80) {
                continue;
            }
            if (s->timer != 0) {
                s->timer--;
                continue;
            }
            if (s->to != s->from) {
                hour = gGameWork.time / 0x10000;
                if (s->from < s->to) {
                    if (hour < s->from || hour >= s->to) {
                        continue;
                    }
                } else if (hour >= s->to && hour < s->from) {
                    continue;
                }
            }
            dx = s->x - cx;
            dz = s->z - cz;
            mx = cx - gEneLastCamX;
            mz = cz - gEneLastCamZ;
            if (dx == 0.0f && dz == 0.0f) {
                heading = 0.0f;
            } else if (mx == 0.0f && mz == 0.0f) {
                heading = 0.0f;
            } else {
                heading = (dx * mx + dz * mz) / (sqrtf(dx * dx + dz * dz) * sqrtf(mx * mx + mz * mz));
            }
            dist = sqrtf((s->x - cx) * (s->x - cx) + (s->y - cy) * (s->y - cy) + (s->z - cz) * (s->z - cz));
            if ((s->flags & 3) == 3 || (dist < gEneClassReachTable[s->flags & 3] &&
                                        (f32) gEneSpawnMinDist < dist && heading >= 0.0f)) {
                o = eneMakeEx(NULL, s->def->kind, s->def->zone, s->def->x, s->def->y, s->def->z,
                                           s->def->mode, (u32 (*)(void *, void *, void *)) s->def->param);
                if (o != NULL) {
                    s->flags |= 0x80;
                    s->timer = 56;
                    ENE(o)->spawn = i + 1;
                }
            }
        }
        gEneLastCamX = cx;
        gEneLastCamZ = cz;
    }
}

/* Lets placement n (from 1) appear again. */
void eneReleaseSpawn(s32 n) {
    EneWork *w = ENEW(gEneManager);

    if (n > 0 && w->count >= n) {
        w->spawns[n - 1].flags &= ~0x80;
    }
}

/* And at once. */
void eneClearSpawnWait(s32 n) {
    EneWork *w = ENEW(gEneManager);

    if (n > 0 && w->count >= n) {
        w->spawns[n - 1].timer = 0;
    }
}

/* Ends the enemies (in o and under it) farther than 1.1 times their reach from the camera. */
void eneDespawnFar(Obj *o, Vec3f *cam) {
    EneObj *e = ENE(o);
    Obj *c;
    f32 d;
    f32 r;
    f32 x;
    f32 y;
    f32 z;

    if (eneIsEnemy(o) == 1 && (e->flags & 3) != 3) {
        if (e->zone != 0) {
            vramCoursePointXZ(e->zone, e->x, e->z, &x, &z);
        } else {
            x = e->x;
            z = e->z;
        }
        y = e->y;
        d = (x - cam->x) * (x - cam->x) + (y - cam->y) * (y - cam->y) + (z - cam->z) * (z - cam->z);
        r = gEneClassReachTable[e->flags & 3] * 1.1f;
        if (d > r * r) {
            eneReleaseSpawn(e->spawn);
            eneClearSpawnWait(e->spawn);
            tskDestroy(o);
            return;
        }
    }
    for (c = o->child; c != NULL; c = o) {
        o = c->next;
        eneDespawnFar(c, cam);
    }
}

void eneDespawnFarAll(Obj *obj) {
    CamView *view;
    Obj *c;
    Vec3f cam;
    Obj *next;

    if ((view = camAiGetView()) != NULL) {
        cam.x = view->at[0];
        cam.y = view->at[1];
        cam.z = view->at[2];
        for (c = obj->child; c != NULL; c = next) {
            next = c->next;
            eneDespawnFar(c, &cam);
        }
    }
}

void eneMakeBenchOne(EneDef *a, s32 na) {
    eneMakeBench(a, na, NULL, 0);
}

/* Makes the stage's kind table from its two placement lists: the kinds used (and 507-509 and
 * 567, always), each with its entry from file 0xC's table; loads their code files and file
 * sets; then the spawn list. The set in a local (declared first) drops the CSE temporary that
 * sat between the two loops' spilled bounds; the pad after file is the slot the game has. */
void eneMakeBench(EneDef *a, s32 na, EneDef *b, s32 nb) {
    s32 set;
    EneWork *w = ENEW(gEneManager);
    EneKindInfo *info = NULL;
    s16 *used;
    s32 i;
    s32 n;
    s32 file;
    s32 pad;

    if (a == NULL) {
        na = 0;
    }
    if (b == NULL) {
        nb = 0;
    }
    if (w->kinds != NULL) {
        w->kinds = heapFree(w->kinds);
    }
    used = heapMallocTop(1000 * sizeof(s16));
    if (used != NULL) {
        heapMemClear(used, 1000 * sizeof(s16));
        used[507]++;
        used[508]++;
        used[509]++;
        used[567]++;
        for (i = 0; i < na; i++) {
            if (a[i].kind > 1000) {
                goto fail;
            }
            used[a[i].kind]++;
        }
        for (i = 0; i < nb; i++) {
            if (b[i].kind > 1000) {
                goto fail;
            }
            used[b[i].kind]++;
        }
        n = 0;
        for (i = 0; i < 1000; i++) {
            if (used[i] != 0) {
                n++;
            }
        }
        w->nKinds = n;
        info = heapMallocTop(fileRamSize(0xC));
        if (info != NULL) {
            fileLoadNow(0xC, info);
            if ((w->kinds = heapMalloc(n * sizeof(EneKind))) != NULL) {
                n = 0;
                for (i = 0; i < 1000; i++) {
                    if (used[i] != 0) {
                        w->kinds[n].kind = i;
                        w->kinds[n].info = info[i];
                        n++;
                    }
                }
                resDrop(0xC);
                heapFree(info);
                heapFree(used);
                for (i = 0; i < n; i++) {
                    if ((file = w->kinds[i].info.file) != 0) {
                        fileLoadNow(file, NULL);
                    }
                }
                for (i = 0; i < n; i++) {
                    set = w->kinds[i].info.set;
                    if (set != 0) {
                        w->kinds[i].files = floadLoadSet(set);
                    }
                }
                geoEatEnemy(a, na, b, nb);
                return;
            }
        }
    }
    /* The game frees both here and again after the label (a double free when a later
     * allocation fails). */
    PRINTF("<eneMakeBench> メモリーが足りません \n");
    if (info != NULL) {
        heapFree(info);
    }
    if (used != NULL) {
        heapFree(used);
    }
fail:
    PRINTF("<eneMakeBench> 存在しないはずのKINDがあります。\n");
    if (info != NULL) {
        heapFree(info);
    }
    if (used != NULL) {
        heapFree(used);
    }
}

/* The stage's kind k (NULL: not in the stage). */
EneKind *eneFindKind(s32 kind) {
    EneWork *w = ENEW(gEneManager);
    s32 i;

    for (i = 0; i < w->nKinds; i++) {
        if (kind == w->kinds[i].kind) {
            return &w->kinds[i];
        }
    }
    return NULL;
}

/* Fills the spawn list from n on with the placements d (an all-zero one is dropped); how far
 * it got. "e = &d[i]" gives the original's walker plus a counter by 16 against nd * 16; x and
 * z declared before e give its frame and slots. n is copied into the local j as a statement
 * (not used in place): that puts the copy in the guard's delay slot. */
s32 eneFillSpawns(EneWork *w, s32 n, EneDef *d, s32 nd) {
    s32 i;
    EneKind *k;
    f32 x;
    f32 z;
    EneDef *e;
    s32 j;
    s32 pad;

    j = n;
    for (i = 0; i < nd; i++) {
        e = &d[i];
        if (e->zone == 0 && e->x == 0 && e->y == 0 && e->z == 0) {
            w->count--;
        } else {
            vramCoursePointXZ(e->zone, e->x, e->z, &x, &z);
            k = eneFindKind(e->kind);
            if (k != NULL) {
                w->spawns[j].flags = k->info.flags & 3;
            }
            w->spawns[j].from = k->info.from;
            w->spawns[j].to = k->info.to;
            w->spawns[j].x = x;
            w->spawns[j].y = e->y;
            w->spawns[j].z = z;
            w->spawns[j].def = e;
            j++;
        }
    }
    return j;
}

void eneEatEnemyOne(EneDef *a, s32 na) {
    geoEatEnemy(a, na, NULL, 0);
}

/* Makes the spawn list from the stage's two placement lists. */
void geoEatEnemy(EneDef *a, s32 na, EneDef *b, s32 nb) {
    EneWork *w = ENEW(gEneManager);
    s32 n;

    if (gEneManager != NULL) {
        w->count = na + nb;
        if (w->spawns != NULL) {
            heapFree(w->spawns);
        }
        if ((w->spawns = heapMalloc(w->count * sizeof(EneSpawn))) != NULL) {
            n = 0;
            if (a != NULL) {
                n = eneFillSpawns(w, 0, a, na);
            }
            if (b != NULL) {
                eneFillSpawns(w, n, b, nb);
            }
        } else {
            PRINTF("geoEatEnemy: メモリーが足りません \n");
        }
    }
}

/* Makes an enemy of a kind at a place, under its base. */
Obj *eneMake(s32 kind, s32 zone, f32 x, f32 y, f32 z) {
    return eneMakeEx(NULL, kind, zone, x, y, z, 0, NULL);
}

/* And under parent. */
Obj *eneMakeUnder(Obj *parent, s32 kind, s32 zone, f32 x, f32 y, f32 z) {
    return eneMakeEx(parent, kind, zone, x, y, z, 0, NULL);
}

Obj *eneMakeEx(Obj *parent, s32 kind, s32 zone, f32 x, f32 y, f32 z, s16 mode,
                            u32 (*c)(void *, void *, void *)) {
    EneWork *w = ENEW(gEneManager);
    s32 i;
    EneArg arg;
    s32 pad;
    Obj *p;
    s32 pad2[2];

    for (i = 0; i < w->nKinds; i++) {
        if (kind == w->kinds[i].kind) {
            arg.zone = zone;
            arg.x = x;
            arg.y = y;
            arg.z = z;
            arg.kind = &w->kinds[i];
            arg.mode = mode;
            arg.c = c;
            if (parent != NULL) {
                p = parent;
            } else {
                p = w->bases[w->kinds[i].info.base];
            }
            return fadeCreateRetry(p, w->kinds[i].info.proc, w->kinds[i].info.prio, w->kinds[i].info.file,
                                          &arg);
        }
    }
    PRINTF("敵カインド %d は、このステージにはセットされていません\n", kind);
    return NULL;
}

/* Makes a task under base i. */
Obj *eneMakeBaseTask(s32 i, s32 (*proc)(Obj *, s16, void *)) {
    EneWork *w = ENEW(gEneManager);

    return tskCreate(w->bases[i], proc, 0, 0);
}

/* An enemy's setup from its message 2. */
u32 eneSetup(Obj *o, EneArg *a) {
    u32 (*c)(void *, void *, void *);

    ENE(o)->zone = a->zone;
    ENE(o)->x = a->x;
    ENE(o)->y = a->y;
    ENE(o)->z = a->z;
    ENE(o)->vel[0] = ENE(o)->vel[1] = ENE(o)->vel[2] = 0.0f;
    ENE(o)->rot[0] = ENE(o)->rot[1] = ENE(o)->rot[2] = 0.0f;
    ENE(o)->kind = a->kind;
    ENE(o)->flags |= a->kind->info.flags & 3;
    switch (a->mode) {
        case 1:
            fileMapTlb(stageAddrToRam(0));
            c = a->c;
            c(FX(o), a, o);
            if (o->file != 0) {
                fileMapTlb(o->file);
            }
            break;
        case 2:
            return (u32) a->c;
        case 3:
            return stageAddrToRam((u32) a->c);
    }
    return 0;
}

/* An enemy's model: an "anime" frame at its place, in its kind's files; and three more of the
 * same (the second frame pool, a "pol" frame, both). */
AnimeFrame *eneNewAnime(Obj *o, u32 anim) {
    EneObj *e = ENE(o);
    EneKind *k = e->kind;
    AnimeFrame *m;
    s32 pad[2];

    if ((m = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime)) != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &m->pos[0], &m->pos[2]);
        m->pos[1] = e->y;
        m->scale[0] = m->scale[1] = m->scale[2] = 0.1f;
        m->anim = anim;
        m->flags |= 0x20;
        m->segs[0] = fileRamAddr(k->files.file[0]);
        m->segs[1] = fileRamAddr(k->files.file[1]);
        m->segs[2] = fileRamAddr(k->files.file[2]);
        m->segs[3] = fileRamAddr(k->files.file[3]);
    }
    return m;
}

/* (These three are written as eneNewAnime: e at its declaration, two pad words, pos[1]
 * first, the scale chain [0] = [1] = [2], and flags last.) */

/* The same in the second pool, with a list. */
AnimeFrame *eneNewAnime2(Obj *o, u32 anim, Gfx *dl) {
    EneObj *e = ENE(o);
    EneKind *k = e->kind;
    AnimeFrame *m;
    s32 pad[2];

    if ((m = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime)) != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &m->pos[0], &m->pos[2]);
        m->pos[1] = e->y;
        m->scale[0] = m->scale[1] = m->scale[2] = 0.1f;
        m->dl = dl;
        m->anim = anim;
        m->flags |= 0x20;
        m->segs[0] = fileRamAddr(k->files.file[0]);
        m->segs[1] = fileRamAddr(k->files.file[1]);
        m->segs[2] = fileRamAddr(k->files.file[2]);
        m->segs[3] = fileRamAddr(k->files.file[3]);
    }
    return m;
}

/* A "pol" frame instead. */
PolFrame *eneNewPol(Obj *o, Gfx *dl) {
    EneObj *e = ENE(o);
    EneKind *k = e->kind;
    PolFrame *m;
    s32 pad[2];

    if ((m = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol)) != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &m->pos[0], &m->pos[2]);
        m->pos[1] = e->y;
        m->scale[0] = m->scale[1] = m->scale[2] = 0.1f;
        m->dl2 = dl;
        m->flags |= 0x20;
        m->segs[0] = fileRamAddr(k->files.file[0]);
        m->segs[1] = fileRamAddr(k->files.file[1]);
        m->segs[2] = fileRamAddr(k->files.file[2]);
        m->segs[3] = fileRamAddr(k->files.file[3]);
    }
    return m;
}

PolFrame *eneNewPol2(Obj *o, Gfx *dl2, Gfx *dl) {
    EneObj *e = ENE(o);
    EneKind *k = e->kind;
    PolFrame *m;
    s32 pad[2];

    if ((m = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol)) != NULL) {
        vramCoursePointXZ(e->zone, e->x, e->z, &m->pos[0], &m->pos[2]);
        m->pos[1] = e->y;
        m->scale[0] = m->scale[1] = m->scale[2] = 0.1f;
        m->dl = dl;
        m->dl2 = dl2;
        m->flags |= 0x20;
        m->segs[0] = fileRamAddr(k->files.file[0]);
        m->segs[1] = fileRamAddr(k->files.file[1]);
        m->segs[2] = fileRamAddr(k->files.file[2]);
        m->segs[3] = fileRamAddr(k->files.file[3]);
    }
    return m;
}

/* An enemy's kind's file n (0-3). */
s32 eneGetFile(Obj *o, s32 n) {
    EneKind *k = ENE(o)->kind;

    switch (n) {
        default:
            return 0;
        case 0:
            return k->files.file[0];
        case 1:
            return k->files.file[1];
        case 2:
            return k->files.file[2];
        case 3:
            return k->files.file[3];
    }
}

/* An address in the enemy's file n. */
u32 eneFileAddr(Obj *o, u32 addr, s32 n) {
    return fadeSegToRam(addr, eneGetFile(o, n));
}

/* The RAM of the enemy's four files. */
void eneGetSegs(Obj *o, u32 *segs) {
    EneKind *k = ENE(o)->kind;

    segs[0] = fileRamAddr(k->files.file[0]);
    segs[1] = fileRamAddr(k->files.file[1]);
    segs[2] = fileRamAddr(k->files.file[2]);
    segs[3] = fileRamAddr(k->files.file[3]);
}

/* Whether o is an enemy (its kind is one of the stage's). */
s32 eneIsEnemy(Obj *o) {
    EneWork *w = ENEW(gEneManager);

    if ((u32) ENE(o)->kind >= (u32) w->kinds && (u32) ENE(o)->kind < (u32) &w->kinds[w->nKinds]) {
        return 1;
    }
    return 0;
}

void eneSetMode(s32 mode) {
    EneWork *w = ENEW(gEneManager);

    w->mode = mode;
    w->forward = NULL;
}

/* Sends the manager's messages to forward (in mode 1). */
void eneSetForward(Obj *forward, s32 mode) {
    EneWork *w = ENEW(gEneManager);

    w->mode = mode;
    w->forward = forward;
}

/* Where the enemy is: the middle of its first hit box (a frame of kind FRM_Dmg among the
 * frames in its work from word 13 on: half its height up, turned by its rotation), else its
 * place on its course. */
void eneGetPos(Obj *o, Vec3f *out) {
    EneObj *e = ENE(o);
    u32 i;
    ACollFrame *f;
    s32 pad;
    f32 z;
    f32 y;
    f32 x;

    for (i = 13; i < (u32) ((o->blocks << 7) - sizeof(Obj)) / 4; i++) {
        if (frmIsValid(((Frame **) FX(o))[i]) == 1) {
            if (FRM_Dmg == ((ACollFrame **) FX(o))[i]->frame.kind) {
                f = ((ACollFrame **) FX(o))[i];
                x = 0.0f;
                y = f->height * ((Vec3f *) f->scale)->y / 2;
                z = 0.0f;
                mathVecRotate(f->rot[0], f->rot[1], f->rot[2], &x, &y, &z);
                out->x = f->pos[0] + x;
                out->y = f->pos[1] + y;
                out->z = f->pos[2] + z;
                return;
            }
        }
    }
    vramCoursePointXZ(e->zone, e->x, e->z, &out->x, &out->z);
    out->y = e->y;
}
