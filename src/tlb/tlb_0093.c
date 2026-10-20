#include "tlb.h"

/* File 93: "mimi" (an ear), a turret you can stand on (a ride) that starts at half size and grows
 * when a player comes near (type 1) or stands full grown (types 2, 3), turns a quarter round
 * toward the nearest player's side and fires a "tama" (kind 0xDE, a ball) along the course (type
 * 3 never turns); and "komimi", a small one with attack boxes on its animation that only grows
 * and stands. */

/* acoll.c defines its adders void (they fall off their end with the box in v0); the callers
 * take the box, as main's own files declare them. */
#define ATK_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, s16, s16, s32)) acollAtkAddYZ)
#define DMG_ADD_YZ ((ACollFrame *(*)(Frame *, Frame *, u16, u16, s16, s16, s32)) acollDmgAddYZ)

extern u8 mimiAnim[];
extern u8 mimiTamaDl[];
extern u8 mimiTurnAnim[];
extern u8 mimiHalf0Dl[];
extern u8 mimiHalf1Dl[];
extern u8 etskKomimiAtkChain[];

extern u8 mimiTamaTex0[];
extern u8 mimiTamaTex2[];
extern u8 mimiTamaTex5[];
RideData mimiRideData[] = { { 1, 386.52295f, { 0 }, { { 0x0000, { 50.0f, 190.0f, 50.0f }, { 0.0f, 190.0f, 0.0f } } } } }; /* the mimi's ride box (file 0x5D's copy) */
AnimFrame mimiTamaAnim[] = {
    { (u32) mimiTamaTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) mimiTamaTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) mimiTamaTex5, 2 },
    { 0x03003580, 2 },
    { 0x03003D80, 2 },
    { 0x03004580, 2 },
    { 0x03004D80, 2 },
    { 0x03005580, 2 },
    { 0x03005D80, 2 },
    { 0x03006580, 2 },
    { 0x03006D80, 2 },
    { 0x03007580, 2 },
    { 0x03007D80, 2 },
    { 0x00000000, 0 },
}; /* the tama's texture animation */

/* A mimi's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct Mimi {
    /* 0x00 */ EneObj e;
    /* 0x4C */ f32 turn;  /* how far it has turned (a quarter: pi / 2) */
    /* 0x50 */ u8 type;   /* 1 rises from the ground; 2, 3 stand (3 never fires) */
    /* 0x51 */ s8 flash;  /* steps of white flashing after a hit (its damage box off) */
    /* 0x52 */ s8 wait;   /* steps before it looks for a player */
    /* 0x53 */ s8 way;    /* the player's side (enePlDirX: 1 or 4) */
    /* 0x54 */ RideFrame *ride;
} Mimi;

#define MIMI(obj) ((Mimi *) FX(obj))

/* A tama's work, at its task + 0x20: an enemy's (EneObj) and its own. */
typedef struct MimiTama {
    /* 0x00 */ EneObj e;
    /* 0x4C */ Anim anim; /* its texture's */
} MimiTama;

#define TAMA(obj) ((MimiTama *) FX(obj))

void mimiWait(Obj *o);
void mimiGrow(Obj *o);
void mimiStand(Obj *o);
void mimiTurn(Obj *o);
void mimiFire(Obj *o);
void komimiWait(Obj *o);
void komimiGrow(Obj *o);

/* "mimi_main". */
s32 mimiProc(Obj *o, s16 msg, void *arg) {
    Mimi *w = MIMI(o);

    switch (msg) {
        case 2:
            o->name = "mimi_main";
            w->type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) eneAnimeModelMake(o, (u32) mimiAnim, NULL, 0)) == NULL) {
                return -1;
            }
            if ((w->e.dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->e.model, 75, 380, 0, 0, 0)) == NULL) {
                return -1;
            }
            if ((w->ride = rideCreate(gRideTree1, (s32) o, w->e.model,
                                      (RideData *) fileRamAddrAt(0x5D, (u32) mimiRideData))) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                return -1;
            }
            w->e.halves[0] = (u32) mimiHalf0Dl;
            w->e.halves[1] = (u32) mimiHalf1Dl;
            w->e.behave = 0x10802;
            w->e.hp = 2;
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->flash = 0;
            if (w->type == 1) {
                ((AnimeFrame *) w->e.model)->scale[2] = 0.05f;
                ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
                    ((AnimeFrame *) w->e.model)->scale[2];
                w->e.state = 0;
            } else if (w->type == 2 || w->type == 3) {
                ((AnimeFrame *) w->e.model)->scale[2] = 0.1f;
                ((AnimeFrame *) w->e.model)->scale[0] = ((AnimeFrame *) w->e.model)->scale[1] =
                    ((AnimeFrame *) w->e.model)->scale[2];
                w->e.state = 2;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->e.state) {
                case 0:
                    (*mimiWait)(o);
                    break;
                case 1:
                    (*mimiGrow)(o);
                    break;
                case 2:
                    (*mimiStand)(o);
                    break;
                case 3:
                    (*mimiTurn)(o);
                    break;
                case 4:
                    (*mimiFire)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                eneSetSd(o, 0x519);
                PRINTF("Etsk_mimi_main: 死亡\n");
                return -1;
            }
            if (w->e.hit & 0x10) {
                w->flash = 16;
            }
            w->flash--;
            if (w->flash > 0) {
                w->e.dmg->flags &= ~2;
            } else {
                w->flash = 0;
                w->e.dmg->flags |= 2;
            }
            /* fallthrough */
        case 3:
            ((AnimeFrame *) w->e.model)->dl = eneDlEnvColorLit(o, (w->flash & 2) ? 0x80 : 0, 0xFF, 0xFF, 0xFF,
                                                               (w->flash & 2) ? 0 : 1);
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.dmg != NULL) { frmDestroy(&w->e.dmg->frame); }
            if (w->ride != NULL) { frmDestroy(&w->ride->f); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* State 0: waits at half size; a player within 70 makes it grow (1). */
void mimiWait(Obj *o) {
    eneAnimLoop(o, (AnimeFrame *) MIMI(o)->e.model, 0.7f);
    if (eneNearestPlInX(o, 70.0f)) {
        MIMI(o)->e.state = 1;
    }
}

/* State 1: grows to its full size (0.1), then stands (2). */
void mimiGrow(Obj *o) {
    Mimi *w = MIMI(o);

    eneAnimLoop(o, (AnimeFrame *) w->e.model, 0.7f);
    if (((AnimeFrame *) w->e.model)->scale[0] < 0.1) {
        ((AnimeFrame *) w->e.model)->scale[0] += 0.003;
    } else {
        ((AnimeFrame *) w->e.model)->scale[0] = 0.1f;
        w->e.state = 2;
    }
    ((AnimeFrame *) w->e.model)->scale[1] = ((AnimeFrame *) w->e.model)->scale[2] =
        ((AnimeFrame *) w->e.model)->scale[0];
}

/* State 2: stands, its sound at frames 3 and 13; when its wait runs out it looks for the
 * nearest player's side and turns (3), unless it is type 3. */
void mimiStand(Obj *o) {
    Mimi *w = MIMI(o);
    s32 pad;
    f32 speed = 0.7f; /* a local: the game adds it to 3.0f at run time (literals would fold) */

    eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
    w->wait--;
    if (w->wait <= 0 && w->type != 3) {
        w->way = enePlDirX(o, 0);
        w->e.state = 3;
    }
    if (3.0f <= ((AnimeFrame *) w->e.model)->frame && ((AnimeFrame *) w->e.model)->frame < 3.0f + speed) {
        eneSetSd(o, 0x510);
    }
    if (13.0f <= ((AnimeFrame *) w->e.model)->frame && ((AnimeFrame *) w->e.model)->frame < 13.0f + speed) {
        eneSetSd(o, 0x510);
    }
}

/* State 3: turns a quarter round toward its way, then starts its firing animation (4). */
void mimiTurn(Obj *o) {
    Mimi *w = MIMI(o);
    s32 pad[2];
    f32 speed = 0.7f;

    eneAnimLoop(o, (AnimeFrame *) w->e.model, speed);
    if (w->way == 4) {
        w->e.rot[1] -= 0.1f;
    } else {
        w->e.rot[1] += 0.1f;
    }
    w->turn += 0.1f;
    if (w->turn >= 1.5707964f) {
        w->e.rot[1] = eneCourseAngle(o, w->way);
        if (1.0 <= ((AnimeFrame *) w->e.model)->frame && ((AnimeFrame *) w->e.model)->frame < 1.0 + speed) {
            w->e.state = 4;
            w->turn = 0.0f;
            ((AnimeFrame *) w->e.model)->anim = mimiTurnAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
            eneSetSd(o, 0x511);
        }
        if (10.0 <= ((AnimeFrame *) w->e.model)->frame && ((AnimeFrame *) w->e.model)->frame < 10.0 + speed) {
            w->e.state = 4;
            w->turn = 0.0f;
            ((AnimeFrame *) w->e.model)->anim = mimiTurnAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
            eneSetSd(o, 0x511);
        }
    }
}

/* State 4: fires a tama its way at frame 5; after the animation turns back and stands (2) for
 * 65 steps. */
void mimiFire(Obj *o) {
    Mimi *w = MIMI(o);
    s32 pad[2];
    f32 speed = 0.3f;

    if (eneAnimPlayOnce(o, (AnimeFrame *) w->e.model, speed)) {
        if (w->way == 4) {
            w->e.rot[1] += 0.1f;
        } else {
            w->e.rot[1] -= 0.1f;
        }
        w->turn += 0.1f;
        if (w->turn >= 1.5707964f) {
            w->e.rot[1] = eneCourseAngle(o, 8);
            w->e.state = 2;
            w->wait = 65;
            w->turn = 0.0f;
            ((AnimeFrame *) w->e.model)->anim = mimiAnim;
            ((AnimeFrame *) w->e.model)->frame = 0.0f;
        }
    }
    if (5.0 <= ((AnimeFrame *) w->e.model)->frame && ((AnimeFrame *) w->e.model)->frame < 5.0 + speed) {
        eneMakeEx(NULL, ENE_MIMI_TAMA, w->e.zone, w->e.x, w->e.y + 35.0f - 2.0f, w->e.z, 2,
                  (u32 (*)(void *, void *, void *)) (s32) w->way);
    }
}

/* "tama": the mimi's ball, flying along the course (back for way 4) and down, gone when it hits
 * a wall or a player (eneAfterProc). */
s32 mimiTamaProc(Obj *o, s16 msg, void *arg) {
    MimiTama *w = TAMA(o);
    s32 file;
    s32 type;
    Vec3f v;

    switch (msg) {
        case 2:
            o->name = "tama";
            type = eneSetup(o, arg);
            if ((w->e.model = (Frame *) enePolModelMake(o, (Gfx *) mimiTamaDl, NULL, 1)) == NULL) {
                return -1;
            }
            if ((w->e.atk = ATK_ADD_YZ((Frame *) ene_atk_root_frame, w->e.model, 20, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->e.atk->kind = 123;
            ((PolFrame *) w->e.model)->flags |= 7;
            ((PolFrame *) w->e.model)->scale[2] = 0.1f;
            ((PolFrame *) w->e.model)->scale[0] = ((PolFrame *) w->e.model)->scale[1] =
                ((PolFrame *) w->e.model)->scale[2];
            file = floadGetSetFile(0x125, 0);
            fadeAnimStart(&w->anim, mimiTamaAnim, 0, &((PolFrame *) w->e.model)->segs[3], file);
            w->e.behave = 4;
            /* a ternary of doubles: converted in each arm (an if/else of literals folds them) */
            w->e.vel[0] = (type == 4) ? -1.8 : 1.8;
            w->e.vel[1] = -1.0f;
            v.x = w->e.vel[0];
            v.y = w->e.vel[1];
            v.z = w->e.vel[2];
            w->e.atk->info = *(ACollInfo *) &v;
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            fadeAnimStep(&w->anim);
            if (w->e.hit & 0xE) {
                w->e.behave |= 0x4000000;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_mimi_tama: 死亡\n");
                return -1;
            }
            /* fallthrough */
        case 3:
            ((PolFrame *) w->e.model)->dl = eneDlEnvPrimColor(o, 0xFF, 0xBF, 0x3F, 0x7F, 0, 0, 0xFF);
            break;
        case 1:
            if (w->e.model != NULL) { frmDestroy(w->e.model); }
            if (w->e.atk != NULL) { frmDestroy(&w->e.atk->frame); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "komimi": a small mimi, its attack boxes on its animation; tiny until a player comes near
 * (type 1) or full grown (type 2). */
s32 Etsk_komimi_main(Obj *o, s16 msg, void *arg) {
    EneObj *w = ENE(o);
    s32 file;
    s32 type;

    switch (msg) {
        case 2:
            o->name = "komimi";
            type = eneSetup(o, arg);
            if ((w->model = (Frame *) eneAnimeModelMake(o, (u32) mimiAnim, NULL, 0)) == NULL) {
                return -1;
            }
            file = eneGetFile(o, 0);
            if ((w->atk = aniAtkCreate((Frame *) ene_atk_root_frame, w->model, (u32) etskKomimiAtkChain, file, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            if ((w->dmg = DMG_ADD_YZ((Frame *) ene_dmg_root_frame, w->model, 75, 380, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("Etsk_komimi_main: 地面にセット失敗\n");
                return -1;
            }
            w->halves[0] = (u32) mimiHalf0Dl;
            w->halves[1] = (u32) mimiHalf1Dl;
            w->rot[1] = eneCourseAngle(o, 8);
            w->behave = 0x10800;
            if (type == 1) {
                ((AnimeFrame *) w->model)->scale[2] = 0.01f;
                ((AnimeFrame *) w->model)->scale[0] = ((AnimeFrame *) w->model)->scale[1] =
                    ((AnimeFrame *) w->model)->scale[2];
                w->state = 0;
            } else if (type == 2) {
                ((AnimeFrame *) w->model)->scale[2] = 0.05f;
                ((AnimeFrame *) w->model)->scale[0] = ((AnimeFrame *) w->model)->scale[1] =
                    ((AnimeFrame *) w->model)->scale[2];
                w->state = 1;
            }
            /* fallthrough */
        case 0:
            if (eneHitValidCheck(o) == -1) {
                break;
            }
            switch (w->state) {
                case 0:
                    (*komimiWait)(o);
                    break;
                case 1:
                    (*komimiGrow)(o);
                    break;
            }
            if (eneAfterProc(o) == -1) {
                PRINTF("Etsk_komimi_main: 死亡\n");
                return -1;
            }
            /* fallthrough */
        case 3:
            ((AnimeFrame *) w->model)->dl = eneDlEnvColorLit(o, 0, 0xFF, 0xFF, 0xFF, 1);
            break;
        case 1:
            if (w->model != NULL) { frmDestroy(w->model); }
            if (w->atk != NULL) { aniAtkDelete(w->atk); }
            if (w->dmg != NULL) { frmDestroy(&w->dmg->frame); }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Komimi state 0: waits tiny (0.01); a player within 70 makes it grow (1). */
void komimiWait(Obj *o) {
    EneObj *w = ENE(o);
    s32 pad[2];

    eneAnimLoop(o, (AnimeFrame *) w->model, 0.7f);
    ((AniAtkFrame *) w->atk->model)->time = ((AnimeFrame *) w->model)->frame;
    if (eneNearestPlInX(o, 70.0f)) {
        w->state = 1;
    }
}

/* Komimi state 1: grows to its full size (0.05) and stays. */
void komimiGrow(Obj *o) {
    EneObj *w = ENE(o);

    eneAnimLoop(o, (AnimeFrame *) w->model, 0.7f);
    ((AniAtkFrame *) w->atk->model)->time = ((AnimeFrame *) w->model)->frame;
    if (((AnimeFrame *) w->model)->scale[0] < 0.05) {
        ((AnimeFrame *) w->model)->scale[0] += 0.0025;
    } else {
        ((AnimeFrame *) w->model)->scale[0] = 0.05f;
    }
    ((AnimeFrame *) w->model)->scale[1] = ((AnimeFrame *) w->model)->scale[2] =
        ((AnimeFrame *) w->model)->scale[0];
}
