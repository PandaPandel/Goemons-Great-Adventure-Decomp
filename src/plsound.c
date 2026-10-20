#include "player.h"
#include "audio.h"
#include "mathutil.h"
#include "plsound.h"

/* The player's sounds (overlay 9): sounds and voices placed in the world (the volume falls off
 * between a near and a far distance from the camera, the pan follows the direction across the
 * view), footsteps by the ground's kind (Player+0x1A2; 5 in water), the attack voices and the
 * animations' event sounds. Sound 0x500 stands for "the footstep here". */


/* Per character, the animations event sounds (in the .data after plshot.c). */
extern u16 gGoemonEventSeTable[][4];
extern u16 gPlEbisuEventSeTable[][4];
extern u16 gPlSasukeEventSeTable[][4];
extern u16 gPlYaeEventSeTable[][4];

/* plsound.c's .data (0x801A87C0-0x801A8870). */
u16 gPlFootstepSeTable[7][2] = { /* footsteps: per ground kind, per player parity */
    { 0x0500, 0x050A }, { 0x0500, 0x050A }, { 0x0503, 0x0506 }, { 0x0502, 0x0507 },
    { 0x0501, 0x0508 }, { 0x0504, 0x0509 }, { 0x0505, 0x050B },
};
u16 gPlAttackSeTable[4] = { 0x020B, 0x020F, 0x0211, 0x0212 }; /* per character: the attack sound */
u16 gPlAttackSeVoiceTable[4] = { 0x053B, 0x052F, 0x0548, 0x0554 }; /* and voice */
u16 gPlAttackVoiceTable[4] = { 0x053D, 0x0531, 0x054A, 0x0556 };
u16 gPlLandSeTable[7][2] = { /* landing, per ground kind */
    { 0x025D, 0x025F }, { 0x025D, 0x025F }, { 0x0261, 0x026A }, { 0x0263, 0x026C },
    { 0x0265, 0x026E }, { 0x0267, 0x0270 }, { 0x0273, 0x0275 },
};
u16 gPlLandWaterSeTable[2] = { 0x0269, 0x0272 }; /* landing on water */
u16 gPlLandVoiceTable[4] = { 0x0541, 0x0535, 0x054E, 0x055A };
u16 gPlLandGroundSeTable[7][2] = {
    { 0x025E, 0x0260 }, { 0x025E, 0x0260 }, { 0x0262, 0x026B }, { 0x0264, 0x026D },
    { 0x0266, 0x026F }, { 0x0268, 0x0271 }, { 0x0274, 0x0276 },
};
u16 gPlActStartVoiceTable[4] = { 0x0543, 0x0537, 0x0550, 0x055C };
u16 gPlEatVoiceTable[4] = { 0x0545, 0x0539, 0x0552, 0x055E };
u16 gPlPushSeTable[4] = { 0x0397, 0x0379, 0x03B6, 0x03D3 };
u16 gPlRestartSdSetSeTable[4] = { 0x0398, 0x0376, 0x03BA, 0x03D4 };
u16 gPlJoinSdSetSeTable[4] = { 0x039C, 0x037B, 0x03B8, 0x03D8 };
u16 (*gPlAnimEventSeTable[4])[4] = { /* per character: the animations four event sounds */
    gGoemonEventSeTable, gPlEbisuEventSeTable, gPlSasukeEventSeTable, gPlYaeEventSeTable,
};



/* Character 2's humming (0x288). */
#define PL_HUM(p) (*(s32 *) &(p)->cloud)

/* A voice (another one, one higher, after a rice ball: 0x16C). */
void plVoice(Player *p, u16 se) {
    if (se != 0) {
        if (p->onigiriTimer != 0) {
            se++;
        }
        plSoundAtModel(p->model, se, p->index);
    }
}

void plSound(Player *p, u16 se) {
    if (se != 0) {
        plSoundAtModel(p->model, se, p->index);
    }
}

void plSoundAtModel(AnimeFrame *model, u16 se, s32 index) {
    plSoundLocation(se, gGameWork.view, model->pos[0], model->pos[1], model->pos[2], 0.0f, 0.0f,
                           gGameWork.view->fovy, index);
}

void plSoundAtPoint(f32 x, f32 y, f32 z, u16 se, s32 index) {
    plSoundLocation(se, gGameWork.view, x, y, z, 0.0f, 0.0f, gGameWork.view->fovy, index);
}

/* SoundLocation: a sound at (x, y, z), fading out from near to far (far 0: heard everywhere).
 * (The x difference goes through k, the variable the fade reuses, before dx, which gives the
 * game's float registers.) */
void plSoundLocation(u16 se, CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far, f32 fov,
                            s32 index) {
    f32 t;
    s16 vol;
    f32 k;
    f32 dy;
    f32 dx;
    f32 dz;
    f32 *eye;

    near = fabsf(near);
    eye = view->eye;
    k = x - eye[0];
    dx = k;
    dy = y - eye[1];
    dz = z - eye[2];
    if (far == 0.0f) {
        PRINTF("SoundLocation: 0除算しました\n");
        vol = 0;
    } else {
        t = (sqrtf(dx * dx + dy * dy + dz * dz) - near) / (far - near);
        k = t;
        if (t > 1.0) {
            k = 1.0f;
        } else if (t < 0.0f) {
            k = 0.0f;
        }
        vol = (1.0f - k) * 256.0f;
        if (vol <= 0) {
            return;
        }
    }
    sndRequest(se, (plSoundPan(view, x, y, z, near, far, fov, index) & 0xFFFE) | (index & 1),
                       vol);
}

/* The pan (1-127, 64 the middle): how far across the view the sound is. (The frame is the
 * game's: the locals in this order, two pad words, and a dy that is never used; lz assigned
 * before lx keeps lx in memory and lz in a register, as the game does.) */
s16 plSoundPan(CamView *view, f32 x, f32 y, f32 z, f32 near, f32 far, f32 fov, s32 index) {
    s16 pan;
    s32 pad;
    f32 d;
    f32 dz;
    f32 dy;
    f32 dx;
    f32 lx;
    s32 pad2;
    f32 lz;

    dx = x - view->eye[0];
    dy = y - view->eye[1];
    dz = z - view->eye[2];
    lz = view->eye[2] - view->at[2];
    lx = view->at[0] - view->eye[0];
    d = sqrtf((lz * lz + lx * lx) * (dx * dx + dz * dz));

    if (d < 1e-7f) {
        return 64;
    }
    pan = (s16) ((lz * dx + lx * dz) / d / mathSin(fov) * 64.0f) + 64;
    if (pan <= 0) {
        pan = 1;
    } else if (pan >= 128) {
        pan = 127;
    }
    return pan;
}

/* Sound 0x500 is the footstep for the ground under the player. */
u16 plFootstepSe(Player *p, u16 se) {
    s32 g;
    u16 r = se;

    if (p->waterObj != 0) {
        g = 5;
    } else {
        g = p->groundSurface;
    }
    if (g >= 7 || g < 0) {
        g = 0;
    }
    if (se == 0x500) {
        r = gPlFootstepSeTable[g][p->index & 1];
    }
    return r;
}

/* An attack: character 2's weapon swings (in its three forms) sound 0x242; the others, their
 * character's sound and voice. */
void plAttackSound(Player *p) {
    s32 flags;
    s32 action;

    if (p->chara == 2) {
        flags = p->flags;
        action = p->action;
        if (((flags & 0x8000) && (action == 0x1F || action == 0x20 || action == 0x21)) ||
            ((flags & 0x4000) && (action == 0x38 || action == 0x3D || action == 0x42)) ||
            ((flags & 0x100000) && (action == 0x36 || action == 0x3B || action == 0x40)) ||
            ((flags & 0x800) && (action == 0xB7 || action == 0xBA || action == 0xBD)) ||
            action == 0x37 || action == 0x3C || action == 0x41) {
            plSound(p, 0x242);
            return;
        }
    }
    plSound(p, gPlAttackSeTable[p->chara]);
    plVoice(p, gPlAttackSeVoiceTable[p->chara]);
}

void plAttackVoice(Player *p) {
    plVoice(p, gPlAttackVoiceTable[p->chara]);
}

/* Landing. */
void plLandSound(Player *p) {
    s32 g;

    if (p->waterObj != 0) {
        g = 5;
    } else {
        g = p->groundSurface;
    }
    if (g >= 7 || g < 0) {
        g = 0;
    }
    if (plIsInWater(p)) {
        plSound(p, gPlLandWaterSeTable[p->index & 1]);
    } else {
        plSound(p, gPlLandSeTable[g][p->index & 1]);
    }
    plVoice(p, gPlLandVoiceTable[p->chara]);
}

void plLandGroundSound(Player *p) {
    s32 g;

    if (p->waterObj != 0) {
        g = 5;
    } else {
        g = p->groundSurface;
    }
    if (g >= 7 || g < 0) {
        g = 0;
    }
    if ((u32) gGameWork.frames >= 20) {
        plSound(p, gPlLandGroundSeTable[g][p->index & 1]);
    }
}

void plRequestLandSound(Player *p) {
    p->eventBits |= 0x10;
}

void plActStartVoice(Player *p) {
    if ((u32) gGameWork.frames >= 5) {
        plVoice(p, gPlActStartVoiceTable[p->chara]);
    }
}

void plEatVoice(Player *p) {
    plVoice(p, gPlEatVoiceTable[p->chara]);
}

void plPushSound(Player *p) {
    plSound(p, gPlPushSeTable[p->chara]);
}

void plRestartSdSet(Player *p) {
    PRINTF("plRestartSdSet(%d)\n", p->chara);
    plSound(p, gPlRestartSdSetSeTable[p->chara]);
}

void plJoinSdSet(Player *p) {
    PRINTF("plJoinSdSet(%d)\n", p->chara);
    plSound(p, gPlJoinSdSetSeTable[p->chara]);
}

/* Character 2 hums (sound 0x252) while carried, except in actions 0x5B and 0x54. */
void plHumStart(Player *p) {
    if (p->chara == 2) {
        if (p->flags & 0x10000) {
            if (p->action != 0x5B && p->action != 0x54 && PL_HUM(p) == 0) {
                if (!sndGetVolume(0x252, 1)) {
                    plSound(p, 0x252);
                } else {
                    PRINTF(" (skip)\n");
                }
                PL_HUM(p) = 1;
            }
        } else {
            plHumStop(p);
        }
    }
}

void plHumStop(Player *p) {
    if (p->chara == 2) {
        sndPlay(0x8252);
        PL_HUM(p) = 0;
    }
}

/* The animation's event sounds (0x1F0 bits 0-3), from the character's table by the model's
 * set (0x205); in water or on ground 4 some footsteps get the ground's sound as well. */
void plAnimEventSounds(Player *p) {
    u16 (*tbl)[4] = gPlAnimEventSeTable[p->chara];
    s32 i;
    u16 se;

    for (i = 0; i < 4; i++) {
        if (p->eventBits & (1 << i)) {
            se = plFootstepSe(p, tbl[p->eventSeSet][i]);
            if (!(p->seAltBits & 1) || p->action != 0x79) {
                plSound(p, se);
            }
            if ((p->groundSurface == 4 || p->waterObj != 0) && (se == 0x387 || se == 0x368 || se == 0x3C4)) {
                plSound(p, plFootstepSe(p, 0x500));
            }
        }
    }
    PRINTF("\n");
    p->seAltBits = (p->seAltBits >> 1) | (p->seAltBits << 31);
}
