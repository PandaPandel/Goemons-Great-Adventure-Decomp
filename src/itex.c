#include "object.h"
#include "fade.h"
#include "fileload.h"
#include "gfxmgr.h"
#include "itex.h"
#include "tmanager.h"

/* Animated-texture effects ("itex"), tasks under the effect manager (effect.c): a "pol" frame
 * drawing one quad whose texture steps through a frame-list animation, moving, growing and
 * fading. The effect textures are in file 0x123 (segment 3); the animation is in segment 6.
 * Each creator copies a template and sets a few fields. */

/* What an itex is made with. */
typedef struct ItexInit {
    /* 0x00 */ s32 file; /* the animation's file */
    /* 0x04 */ Gfx *dl;
    /* 0x08 */ AnimFrame *frames;
    /* 0x0C */ Color3 prim;
    /* 0x0F */ Color3 env;
    /* 0x14 */ s32 flags; /* the frame's; 4: no random turn */
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ f32 scale[3]; /* times 3 */
    /* 0x30 */ Vec3f vel;
    /* 0x3C */ Vec3f grow; /* the scale's factor each step */
    /* 0x48 */ s32 alpha;
    /* 0x4C */ s32 fade; /* added to alpha each step; 0x100: fade out over the life */
    /* 0x50 */ s32 life; /* in steps; 0: the animation's length */
} ItexInit; /* size 0x54 */

typedef struct ItexWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ Anim anim;
    /* 0x14 */ Color3 prim;
    /* 0x17 */ Color3 env;
    /* 0x1C */ Vec3f vel;
    /* 0x28 */ Vec3f grow;
    /* 0x34 */ s32 alpha;
    /* 0x38 */ s32 fade;
    /* 0x3C */ s32 life;
} ItexWork;

extern Frame *gFrmEffectLayer; /* the frame effects go under */
extern s16 gFrmKindPol; /* "pol" */
extern Gfx *gGfxDlHead; /* the display-list buffer */
extern Gfx gDrawSetupCldSurfDl[]; /* the itex draw setup */
extern Obj *gEffectTask; /* the effect manager */
extern Gfx itexSparkleDl[]; /* the quad, in file 0x123 */
/* itex.c's .data (0x8010C290-0x8010C5D4), in ROM order. */
AnimFrame gItexSparkleAnim[] = {
    { 0x03000530, 3 }, { 0x03000D30, 3 }, { 0x03001530, 3 }, { 0x03001D30, 3 }, { 0x03002530, 3 },
    { 0x03002D30, 3 }, { 0x03003530, 3 }, { 0x03003D30, 3 }, { 0x03004530, 3 }, { 0x03004D30, 3 },
    { 0x03005530, 3 }, { 0x03005D30, 3 }, { 0x00000000, 0 },
};
AnimFrame gItexPuffAnim[] = {
    { 0x03000530, 2 }, { 0x03000D30, 2 }, { 0x03001530, 2 }, { 0x03001D30, 2 }, { 0x03002530, 2 },
    { 0x03002D30, 2 }, { 0x03003530, 2 }, { 0x03003D30, 2 }, { 0x03004530, 2 }, { 0x03004D30, 2 },
    { 0x03005530, 2 }, { 0x03005D30, 2 }, { 0x00000000, 0 },
};
AnimFrame gItexGlowAnim[] = {
    { 0x030000D0, 2 }, { 0x030008D0, 2 }, { 0x030010D0, 2 }, { 0x030018D0, 2 }, { 0x030020D0, 2 },
    { 0x030028D0, 2 }, { 0x030030D0, 2 }, { 0x030038D0, 2 }, { 0x030040D0, 2 }, { 0x030048D0, 2 },
    { 0x030050D0, 2 }, { 0x030058D0, 2 }, { 0x030060D0, 2 }, { 0x030068D0, 2 }, { 0x030070D0, 2 },
    { 0x030078D0, 2 }, { 0x00000000, 0 },
};
AnimFrame gItexFlashAnim[] = {
    { 0x030000D0, 1 }, { 0x030008D0, 1 }, { 0x030010D0, 1 }, { 0x030018D0, 1 }, { 0x030020D0, 1 },
    { 0x030028D0, 1 }, { 0x030030D0, 1 }, { 0x030038D0, 1 }, { 0x030040D0, 1 }, { 0x030048D0, 1 },
    { 0x030050D0, 1 }, { 0x030058D0, 1 }, { 0x030060D0, 1 }, { 0x030068D0, 1 }, { 0x030070D0, 1 },
    { 0x030078D0, 1 }, { 0x00000000, 0 },
};
AnimFrame gItexBombAnim[] = {
    { 0x030000D0, 2 }, { 0x030008D0, 2 }, { 0x030010D0, 2 }, { 0x030018D0, 2 }, { 0x030020D0, 2 },
    { 0x030028D0, 2 }, { 0x030030D0, 2 }, { 0x030038D0, 2 }, { 0x030040D0, 2 }, { 0x030048D0, 2 },
    { 0x030050D0, 2 }, { 0x030058D0, 2 }, { 0x030060D0, 2 }, { 0x030068D0, 2 }, { 0x030070D0, 2 },
    { 0x030078D0, 2 }, { 0x00000000, 0 },
};
AnimFrame gItexTimedYellowAnim[] = {
    { 0x03000580, 2 }, { 0x03000D80, 2 }, { 0x03001580, 2 }, { 0x03001D80, 2 }, { 0x03002580, 2 },
    { 0x03002D80, 2 }, { 0x03003580, 2 }, { 0x03003D80, 2 }, { 0x03004580, 2 }, { 0x03004D80, 2 },
    { 0x03005580, 2 }, { 0x03005D80, 2 }, { 0x03006580, 2 }, { 0x03006D80, 2 }, { 0x03007580, 2 },
    { 0x03007D80, 2 }, { 0x00000000, 0 },
};
ItexInit gItexTemplate = { /* the template */
    291, (Gfx *) 0x030003A0, NULL, { 0xFF, 0xFF, 0xFF }, { 0x00, 0x00, 0x00 }, 7,
    { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 255,
    256, 0,
};


#define FX(obj) ((void *) ((u32) (obj) + sizeof(Obj)))

/* An animation's length in ticks. */
s32 itexAnimLength(AnimFrame *f) {
    s32 n = 0;

    for (; f->addr != 0; f++) {
        n += f->duration;
    }
    return n;
}

/* The itex handler. */
s32 itex(Obj *obj, s16 msg, void *arg) {
    ItexWork *w = FX(obj);
    ItexInit *init = arg;

    switch (msg) {
        case 2:
            obj->name = "itex";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = init->dl;
            floadGetSetRam((void **) &w->frame->segs[0], 0x123);
            fadeAnimStart(&w->anim, init->frames, 0, &w->frame->segs[3],
                                   floadGetSetFile(init->file, 0));
            w->frame->flags |= init->flags;
            if (!(init->flags & 4)) {
                w->frame->rot[2] = fadeRand(360) * 3.1415927f / 180.0f;
            }
            w->frame->pos[0] = init->pos[0];
            w->frame->pos[1] = init->pos[1];
            w->frame->pos[2] = init->pos[2];
            w->frame->scale[0] = init->scale[0] * 3.0f;
            w->frame->scale[1] = init->scale[1] * 3.0f;
            w->frame->scale[2] = init->scale[2] * 3.0f;
            w->vel = init->vel;
            w->grow = init->grow;
            w->prim = init->prim;
            w->env = init->env;
            if (init->life == 0) {
                w->life = itexAnimLength(init->frames);
            } else {
                w->life = init->life;
            }
            w->alpha = init->alpha;
            if (init->fade == 0x100) {
                w->fade = -(w->alpha + w->life - 1) / w->life;
            } else {
                w->fade = init->fade;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (--w->life <= 0) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            w->frame->scale[0] *= w->grow.x;
            w->frame->scale[1] *= w->grow.y;
            w->frame->scale[2] *= w->grow.z;
            w->frame->pos[0] += w->vel.x;
            w->frame->pos[1] += w->vel.y;
            w->frame->pos[2] += w->vel.z;
            w->alpha += w->fade;
            if (w->alpha < 0) {
                w->alpha = 0;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gSPDisplayList(gGfxDlHead++, gDrawSetupCldSurfDl);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0,
                              ENVIRONMENT, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0,
                              0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->prim.r, w->prim.g, w->prim.b, 0);
            gDPSetEnvColor(gGfxDlHead++, w->env.r, w->env.g, w->env.b, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A sparkle rising from (x, y, z), of a size. */
void itexSparkle(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.dl = itexSparkleDl;
    init.frames = gItexSparkleAnim;
    init.env = init.prim;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.grow.x = 1.001f;
    init.grow.z = 1.001f;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    init.vel.y = 0.1f;
    init.grow.y = 1.01f;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A rising puff (animation C2F8). */
void itexPuff(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexPuffAnim;
    init.flags = 3;
    init.env = init.prim;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.grow.x = 1.02f;
    init.grow.y = 1.02f;
    init.grow.z = 1.02f;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    init.vel.y = 1.0f;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A grey rising puff. */
void itexPuffGrey(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexPuffAnim;
    init.flags = 3;
    init.prim.r = init.prim.g = init.prim.b = 0x60;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.grow.x = init.grow.y = init.grow.z = 1.02f;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    init.vel.y = 1.0f;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A dark puff, grey inside. */
void itexPuffDark(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexPuffAnim;
    init.flags = 3;
    init.env.r = init.env.g = init.env.b = 0x80;
    init.prim.r = init.prim.g = init.prim.b = 0x40;
    init.pos[0] = x;
    init.pos[1] = y;
    init.grow.x = init.grow.y = init.grow.z = 1.02f;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A yellow and orange puff.
 * Here and in the next three each colour is set on one line: the original stores b, g, r (and
 * env g before r) with the constants loaded r, g, b, which only same-line statements give. */
void itexPuffYellow(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexPuffAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xFF; init.prim.b = 0x80;
    init.env.r = 0xB4; init.env.g = 0x28; init.env.b = 0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A blue glow (animation C360, file 0x121). */
void itexGlowBlue(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x121;
    init.frames = gItexGlowAnim;
    init.flags = 3;
    init.env.r = init.env.g = 0;
    init.env.b = 0xFF;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A yellow sparkle that does not fade (animation C470, file 0x11F). */
void itexSparkleYellow(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x11F;
    init.frames = gItexBombAnim;
    init.prim.r = 0xFF; init.prim.g = 0xFF; init.prim.b = 0x80;
    init.env.r = 0xFF; init.env.g = 0x28; init.env.b = 0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.fade = 0;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A white and blue sparkle that does not fade (the burst's, effect.c). */
void itexSparkleBlue(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x11F;
    init.frames = gItexBombAnim;
    init.prim.b = init.prim.g = init.prim.r = 0xFF;
    init.env.b = 0xFF;
    init.env.r = 0;
    init.env.g = 0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.fade = 0;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A yellow and red flash that does not grow or fade (animation C3E8, file 0x121). */
void itexFlash(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x121;
    init.frames = gItexFlashAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xFF; init.prim.b = 0x80;
    init.env.r = 0xFF;
    init.env.g = 0;
    init.env.b = 0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.fade = 0;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A yellow and orange effect for a time (animation C4F8, file 0x125). */
void itexTimedYellow(f32 x, f32 y, f32 z, f32 size, s32 life) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x125;
    init.frames = gItexTimedYellowAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xFF; init.prim.b = 0x80;
    init.env.r = 0xFF; init.env.g = 0x28; init.env.b = 0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.life = life;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* A white puff, light grey inside. */
void itexPuffWhite(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexPuffAnim;
    init.flags = 3;
    init.prim.b = init.prim.g = init.prim.r = 0xFF;
    init.env.b = init.env.g = init.env.r = 0xBF;
    init.pos[0] = x;
    init.pos[1] = y;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, 0, &init);
}
