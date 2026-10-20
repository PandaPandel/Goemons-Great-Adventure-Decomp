#include "tlb.h"

/* A boss scene: the director ("main"), two screens that flash ("do_vram1", "do_vram2"), the
 * Dochuki ("dochuki", kind 0x32), the Behimos ("behimos", kind 0x33), "chinko" (kind 0x36) and
 * the pieces they throw ("genki", "ntama", "effect", "beam", "CORE", "iki", "bat", "HANA"). The
 * director answers message 10 with argument MAIN_ID with itself. */

#define MAIN_ID 0x2DD59FE7

/* The director's work. */
typedef struct MainWork {
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
    /* 0x34 */ Obj *dochuki; /* kind 0x32 */
    /* 0x38 */ Obj *behimos; /* kind 0x33 */
    /* 0x3C */ Obj *chinko; /* kind 0x36 */
    /* 0x40 */ Obj *vram1; /* kind 0x34 */
    /* 0x44 */ Obj *vram2; /* kind 0x35 */
    /* 0x48 */ Obj *gauge;
    /* 0x4C */ s32 timer;
    /* 0x50 */ s32 unused50;
    /* 0x54 */ s32 heard;
    /* 0x58 */ s32 count; /* the gauge's pips */
} MainWork;

/* "do_vram1": a screen whose colour cycles. */
typedef struct Vram1Work {
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ u8 cycling;
    /* 0x39 */ u8 alpha;
    /* 0x3A */ u8 r;
    /* 0x3B */ u8 g;
    /* 0x3C */ u8 b;
    /* 0x3D */ u8 step; /* which target colour */
    /* 0x3E */ u8 wait;
    /* 0x3F */ u8 tr; /* the target colour */
    /* 0x40 */ u8 tg;
    /* 0x41 */ u8 tb;
} Vram1Work;

/* "do_vram2": a screen that brightens with smoke puffs (state 1), then blows the players back
 * and fades (state 2). */
typedef struct Vram2Work {
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ s32 puffWait;
    /* 0x3C */ s32 alpha;
    /* 0x40 */ s32 fade; /* added to alpha each frame */
    /* 0x44 */ u8 r;
    /* 0x45 */ u8 g;
    /* 0x46 */ u8 b;
    /* 0x47 */ u8 red;
} Vram2Work;

/* "dochuki" */
typedef struct DochukiWork {
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
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ Frame *ride; /* destroyed with it; nothing here makes it */
    /* 0x44 */ AnimeFrame *ghosts[5]; /* its after-images */
    /* 0x58 */ u8 ghostAlpha[5];
    /* 0x5D */ s8 ghostWait; /* frames until the next after-image */
    /* 0x5E */ u8 pad5E[2];
    /* 0x60 */ Obj *rings; /* the "effect" (kind 0x39) it makes before it goes */
    /* 0x64 */ s32 timer;
    /* 0x68 */ s32 swing; /* the swing's way: 0 forward, 1 back */
    /* 0x6C */ s32 sounded;
    /* 0x70 */ s32 atkHitTimer; /* frames after a hit on a player (sound 0x303 at 1) */
    /* 0x74 */ s32 tintAlpha; /* its tint's alpha */
    /* 0x78 */ s32 unused78;
    /* 0x7C */ s32 wait;
    /* 0x80 */ s32 steps;
    /* 0x84 */ s32 target; /* the player it goes for */
    /* 0x88 */ s32 retarget; /* frames until it picks a player again */
    /* 0x8C */ u8 pad8C[8];
    /* 0x94 */ s8 hurt; /* frames it cannot be hurt again (it flashes) */
    /* 0x95 */ u8 unused95;
    /* 0x96 */ s8 unused96; /* counted down to 0 each frame; nothing sets it */
    /* 0x97 */ s8 blinkWait;
    /* 0x98 */ u8 blink; /* its eyes' texture: 1 shut */
    /* 0x99 */ u8 toggle;
} DochukiWork;

/* "behimos" */
typedef struct BehimosWork {
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
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ u8 pad38[0xC];
    /* 0x44 */ s32 timer;
    /* 0x48 */ s32 walk; /* frames left to walk after the players */
    /* 0x4C */ f32 speed; /* its animation's */
    /* 0x50 */ s32 sub; /* a done flag, a count, or (state 55 on) where it shakes */
    /* 0x54 */ s32 sounded;
    /* 0x58 */ s32 hp; /* also its red tint's alpha */
    /* 0x5C */ u8 pad5C[4];
    /* 0x60 */ s32 target; /* the player it goes for */
    /* 0x64 */ s32 retarget; /* frames until it picks a player again */
    /* 0x68 */ s32 wait;
    /* 0x6C */ s32 steps;
    /* 0x70 */ u8 pad70[4];
    /* 0x74 */ s32 hits;
    /* 0x78 */ s8 flash;
    /* 0x79 */ u8 unused79;
    /* 0x7A */ s8 unused7A; /* counted down to 0 each frame; nothing sets it */
} BehimosWork;

/* "chinko" */
typedef struct ChinkoWork {
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
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ u8 pad3C[8];
    /* 0x44 */ s32 alpha; /* of its red flush */
    /* 0x48 */ u8 pad48[8];
    /* 0x50 */ s8 hurt; /* frames it cannot be hurt again (it flashes) */
} ChinkoWork;

/* "genki" */
typedef struct GenkiWork {
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
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vy;
    /* 0x30 */ f32 vz;
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ f32 size;
    /* 0x44 */ u8 alpha;
    /* 0x45 */ u8 r;
    /* 0x46 */ u8 g;
    /* 0x47 */ u8 b;
    /* 0x48 */ s16 timer; /* frames left once thrown */
    /* 0x4A */ s16 wait; /* frames until the next ring */
    /* 0x4C */ PolFrame *rings[5];
} GenkiWork;

/* "ntama" */
typedef struct NtamaWork {
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
} NtamaWork;

/* "bat" */
typedef struct BatWork {
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ s32 life;
} BatWork;

/* acollAtkAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

/* "beam" */
typedef struct BeamWork {
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
    /* 0x34 */ AnimeFrame *model;
    /* 0x38 */ u32 variant; /* what it was made with: 0 or not */
    /* 0x3C */ u8 fired;
} BeamWork;

/* "effect" */
typedef struct EffectWork {
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
    /* 0x34 */ PolFrame *rings[2];
    /* 0x3C */ f32 grow;
} EffectWork;

/* "CORE" */
typedef struct CoreWork {
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ s32 life;
    /* 0x44 */ u8 pad44[4];
    /* 0x48 */ u32 variant; /* 0 to the left, 10 stays, else to the right */
} CoreWork;

/* acollDmgAddYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* "iki" */
typedef struct IkiWork {
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ Anim anim; /* the puff's texture frames */
    /* 0x50 */ s32 life;
    /* 0x54 */ s32 alpha;
} IkiWork;

/* "HANA" */
typedef struct HanaWork {
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
    /* 0x34 */ PolFrame *model;
    /* 0x38 */ ACollFrame *atk;
    /* 0x3C */ ACollFrame *dmg;
    /* 0x40 */ s32 life;
} HanaWork;

#define HANA(obj) ((HanaWork *) FX(obj))
#define IKI(obj) ((IkiWork *) FX(obj))
#define CORE(obj) ((CoreWork *) FX(obj))
#define EFFECT(obj) ((EffectWork *) FX(obj))
#define BEAM(obj) ((BeamWork *) FX(obj))
#define BAT(obj) ((BatWork *) FX(obj))
#define MAIN(obj) ((MainWork *) FX(obj))
#define VRAM1(obj) ((Vram1Work *) FX(obj))
#define VRAM2(obj) ((Vram2Work *) FX(obj))
#define DOCHUKI(obj) ((DochukiWork *) FX(obj))
#define BEHIMOS(obj) ((BehimosWork *) FX(obj))
#define CHINKO(obj) ((ChinkoWork *) FX(obj))
#define GENKI(obj) ((GenkiWork *) FX(obj))
#define NTAMA(obj) ((NtamaWork *) FX(obj))

extern u8 dochukiBehimosSneezeAnim[];
extern u8 dochukiEyesOpenTex[];
extern u8 dochukiBlinkTex[];
extern Gfx dochukiXluEnvDl[];
extern u8 dochukiState8Anim[];
extern u8 dochukiState8Attach[];
extern Gfx dochukiGenkiRingDl[];
extern Gfx dochukiVram1Dl[];
extern Gfx dochukiEffectRingDl[];
extern Gfx dochukiVram2Dl[];
extern u8 dochukiChinkoState0Anim[];
extern u8 dochukiNtamaDl[]; /* the ntama's display list (kind 0x38's file set 0x122): the same address, another file */
extern u8 dochukiChinkoState2Anim[];
extern Gfx dochukiOpaEnvDl[];
extern Gfx dochukiIkiDl[];
extern u8 dochukiBeamAnim0[];
extern u8 dochukiBeamAnim1[];
extern Gfx beamDl[];
extern Gfx doVram1Dl[];

/* The file's .data, in address order (its externs above stay as declarations). */
Gfx doVram1Dl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineLERP(ENVIRONMENT, COMBINED, ENV_ALPHA, COMBINED, TEXEL0, 0, SHADE, 0, ENVIRONMENT, COMBINED, ENV_ALPHA, COMBINED, TEXEL0, 0, SHADE, 0),
    gsDPSetEnvColor(0x00, 0xFF, 0xFF, 0xFF),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};
Gfx dochukiOpaEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, TEXEL0, ENVIRONMENT, COMBINED, ENV_ALPHA, COMBINED, 0, 0, 0, TEXEL0),
    gsDPSetEnvColor(0x00, 0xFF, 0xFF, 0xFF),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};
Gfx dochukiXluEnvDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineLERP(TEXEL0, ENVIRONMENT, ENV_ALPHA, ENVIRONMENT, 0, 0, 0, PRIMITIVE, TEXEL0, ENVIRONMENT, ENV_ALPHA, ENVIRONMENT, 0, 0, 0, PRIMITIVE),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
};
Gfx beamDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetEnvColor(0x00, 0xFF, 0xFF, 0xFF),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
    gsDPNoOp(),
};
extern u8 dochukiBehimosState34Anim[];
extern u8 dochukiBehimosState34Attach[];
extern u8 dochukiBehimosState28Anim[];
extern u8 dochukiBehimosState28Attach[];

/* mdlAnimNodePos as this file calls it: the frame as a float. */
typedef void (*NodePosFunc)(u32 addr, f32 frame, u32 base, f32 *x, f32 *y, f32 *z);

/* libultra's, through their addresses (code at 0x08000000 cannot jal them). */
#define sinf (*sinf)
#define cosf (*cosf)

void dochukiVram1StartCycle(Obj *o);
void dochukiSetState19(Obj *o);
void dochukiMainShowGauge(void);
void dochukiVram1Show(Obj *o);
void dochukiVram1Hide(Obj *o);
void dochukiVram1StopCycle(Obj *o);
void dochukiVram2Start(Obj *o);
void dochukiVram2Stop(Obj *o);
void dochukiSetState1(Obj *o);
void dochukiSetState2(Obj *o);
void dochukiSetState3(Obj *o);
void dochukiSetState6(Obj *o);
void dochukiDestroy(Obj *o);
void dochukiBehimosSetState1(Obj *o);
void dochukiBehimosSetState3(Obj *o);
s32 dochukiBehimosIsState6(Obj *o);
s32 dochukiBehimosIsState59(Obj *o);
void dochukiBehimosSetState8(Obj *o);
void dochukiBehimosSetState7(Obj *o);
void dochukiBehimosSetState47(Obj *o);
void dochukiBehimosSetState55(Obj *o);
void dochukiChinkoShow(Obj *o);
void dochukiChinkoHide(Obj *o);

/* "main": makes the screens, the Dochuki, the Behimos, "chinko" and the gauge, then runs the
 * scenes: the players drop in and the camera finds the Dochuki (states 0-9, skipped with start),
 * the Dochuki's end and the Behimos's entrance (11-18), the walk to the Behimos's chamber (19-22)
 * and its end (23-24, which sets the stage's flags and queues demo 14).
 * The walk goal's 64.0 is a double: as 64.0f it would share case 1's camSetAt constant. */
s32 dochukiMainProc(Obj *o, s16 msg, void *arg) {
    MainWork *w = MAIN(o);
    Camera *cam = camAiGetCam();
    Player *pl;
    s32 i;
    s32 c;
    s32 zone;
    Vec3f p;

    switch (msg) {
        case 2:
            o->name = "main";
            eneSetup(o, arg);
            w->vram1 = eneMakeUnder(o, ENE_DOCHUKI_VRAM1, 0, 0.0f, 0.0f, 0.0f);
            w->vram2 = eneMakeUnder(o, ENE_DOCHUKI_VRAM2, 0, 0.0f, 0.0f, 0.0f);
            w->dochuki = eneMakeUnder(o, ENE_DOCHUKI, 1, 457.0f, 110.0f, 0.0f);
            w->behimos = eneMakeUnder(o, ENE_DOCHUKI_BEHIMOS, 1, 340.0f, 0.0f, -97.0f);
            w->chinko = eneMakeUnder(o, ENE_DOCHUKI_CHINKO, 0, -282.0f, 80.0f, 96.0f);
            w->gauge = tskCreate(o, pipGauge_proc, 0, NULL);
            w->count = 50;
            eneCounterReward(4, 1, 520.0f, 69.0f, 0.0f, 1, 170.0f, 69.0f, 0.0f);
            w->state = 0;
            dochukiVram1Show(w->vram1);
            dochukiVram2Stop(w->vram2);
            dochukiChinkoHide(w->chinko);
            w->count = 50;
            gGameWork.scenarioRunning = 2;
            sndPlay(0x19);
            if (eneCounterGet(4) == 0) {
                w->state = 0;
            } else {
                w->state = 9;
            }
            eneCounterIncrement(4);
            break;
        case 10:
            if ((s32) arg == MAIN_ID) {
                return (s32) o;
            }
            break;
        case 0:
            if (w->state < 9) {
                if (fadeStartPressed() != 0) {
                    w->state = 9;
                }
            }
            switch (w->state) {
                case 0: {
                    s32 n;

                    if (fadeMoveLetterbox(45, 185, 1) != 0) {
                        w->state = 1;
                        w->heard = 0;
                    } else {
                        n = 0;
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                n++;
                            }
                        }
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                p.y = 200.0f;
                                switch (n) {
                                    case 1:
                                        p.z = 0.0f;
                                        break;
                                    case 2:
                                        p.z = 8 - i * 16;
                                        break;
                                    case 3:
                                        p.z = 8 - i * 8;
                                        break;
                                    case 4:
                                        p.z = 8.0 - i * 5.3;
                                        break;
                                }
                                plSetPos(gGameWork.players[i], zone, &p);
                            }
                        }
                    }
                }
                    /* fallthrough */
                case 1: {
                    f32 y;

                    y = 0.0f;
                    p.x = p.y = p.z = 0.0f;
                    for (i = 0, c = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            plGetModelPos(gGameWork.players[i], &p.x, &p.y, &p.z);
                            c++;
                            y += p.y;
                        }
                    }
                    y /= c;
                    y += 10.0f;
                    if (y > 160.0f) {
                        y = 160.0f;
                    }
                    camSetAt(cam, 64.0f, y, 97.0f);
                    if (w->heard == 0 && y <= 85.0f) {
                        w->heard = 1;
                        sndPlay(0x257);
                    }
                    if (w->heard == 1) {
                        if (y <= 80.0f) {
                            w->heard = 2;
                            for (i = 0, c = 0; i < 4; i++) {
                                if (plIsInPlay(i)) {
                                    c++;
                                }
                            }
                            if (c >= 2) {
                                sndPlay(0x257);
                            }
                        }
                    }
                    if (y <= 80.0f) {
                        w->timer = 40;
                        w->state = 2;
                    }
                    break;
                }
                case 2:
                    if (w->timer-- <= 0) {
                        camSetSpeed(cam, 100.0f);
                        camMoveAt(cam, 178.0f, 75.0f, 97.0f);
                        camMoveEye(cam, 10.0f, 95.0f, 97.0f);
                        w->state = 3;
                    }
                    break;
                case 3:
                    if (!camGetMoving(cam)) {
                        dochukiSetState1(w->dochuki);
                        camSetSpeed(cam, 80.0f);
                        camMoveAt(cam, 178.0f, 94.0f, 97.0f);
                        camMoveEye(cam, 10.0f, 95.0f, 97.0f);
                        w->state = 4;
                    }
                    break;
                case 4:
                    if (!camGetMoving(cam)) {
                        camSetSpeed(cam, 30.0f);
                        camMoveAt(cam, 178.0f, 116.0f, 97.0f);
                        camMoveEye(cam, 152.0f, 124.0f, 97.0f);
                        w->timer = 20;
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (!camGetMoving(cam) && w->timer-- <= 0) {
                        scRunStart(0x13, 0, 0);
                        dochukiSetState3(w->dochuki);
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        w->timer = 20;
                        dochukiSetState2(w->dochuki);
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (w->timer-- <= 0) {
                        scRunStart(11, 0, 0);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (!scIsRunning()) {
                        w->state = 9;
                    }
                    break;
                case 9: {
                    s32 n;

                    n = 0;
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            n++;
                        }
                    }
                    fadeSetLetterbox(320, 0, 0, 240);
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f q;
                            s32 zone2;

                            plSetAction(gGameWork.players[i], 0);
                            plAskPos(gGameWork.players[i], &q.x, &q.y, &q.z, &zone2);
                            if (q.y > 69.0) {
                                q.y = 69.0f;
                            }
                            switch (n) {
                                case 1:
                                    q.z = 0.0f;
                                    break;
                                case 2:
                                    q.z = 5 - i * 10;
                                    break;
                                case 3:
                                    q.z = 5 - i * 5;
                                    break;
                                case 4:
                                    q.z = 5.0 - i * 3.3;
                                    break;
                            }
                            plSetPos(gGameWork.players[i], zone2, &q);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = 0.0f;
                        }
                    }
                    dochukiSetState6(w->dochuki);
                    scRunEnd();
                    camAiSetHold(0);
                    dochukiMainShowGauge();
                    gGameWork.scenarioRunning = 1;
                    w->state = 10;
                    eneRumbleStopAll();
                    break;
                }
                case 11: {
                    s32 n;
                    Player *pl;

                    gGameWork.scenarioRunning = 2;
                    n = 0;
                    eneRumbleStopAll();
                    camAiSetHold(1);
                    camSetAt(cam, 110.0f, 105.0f, 54.0f);
                    camSetEye(cam, -18.0f, 74.0f, 154.0f);
                    eneEndPlayersPowerDown();
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            n++;
                        }
                    }
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f q;
                            s32 zone2;

                            plSetAction(gGameWork.players[i], 0xD7);
                            plAskPos(gGameWork.players[i], &q.x, &q.y, &q.z, &zone2);
                            q.x = 250.0f;
                            q.y = 69.0f;
                            switch (n) {
                                case 1:
                                    q.z = 0.0f;
                                    break;
                                case 2:
                                    q.z = 8 - i * 16;
                                    break;
                                case 3:
                                    q.z = 8 - i * 8;
                                    break;
                                case 4:
                                    q.z = 8.0 - i * 5.3;
                                    break;
                            }
                            pl = PL(gGameWork.players[i]);
                            pl->facing = 0.0f;
                            plSetPos(gGameWork.players[i], zone2, &q);
                        }
                    }
                    w->state = 12;
                }
                    /* fallthrough */
                case 12: {
                    Player *pl;

                    fadeMoveLetterbox(50, 190, 1);
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            pl = PL(gGameWork.players[i]);
                            pl->facing = 0.0f;
                        }
                    }
                    break;
                }
                case 13:
                    w->timer = 30;
                    fadeSetAll(30.0f, 0.0f, 255.0f, 255, 255, 255);
                    w->state = 14;
                    /* fallthrough */
                case 14:
                    if (w->timer-- < -9) {
                        w->timer = 60;
                        fadeSetAll(60.0f, 255.0f, 0.0f, 255, 255, 255);
                        dochukiBehimosSetState1(w->behimos);
                        dochukiDestroy(w->dochuki);
                        camSetAt(cam, 19.0f, 93.0f, 0.0f);
                        camSetEye(cam, -54.0f, 71.0f, 166.0f);
                        sndPlay(7);
                        w->state = 15;
                    }
                    break;
                case 15:
                    if (w->timer == 0) {
                        sndPlay(0x1A);
                    }
                    if (w->timer-- < -9) {
                        sndPlay(0x165);
                        camSetSpeed(cam, 100.0f);
                        camMoveAt(cam, 27.0f, 127.0f, 23.0f);
                        camMoveEye(cam, -54.0f, 71.0f, 166.0f);
                        w->state = 16;
                    }
                    break;
                case 16:
                    if (!camGetMoving(cam)) {
                        w->timer = 20;
                        dochukiBehimosSetState3(w->behimos);
                        w->state = 17;
                    }
                    break;
                case 17:
                    if (dochukiBehimosIsState6(w->behimos) && w->timer-- <= 0) {
                        w->state = 18;
                    }
                    break;
                case 18: {
                    s32 n;

                    n = 0;
                    fadeSetLetterbox(320, 0, 0, 240);
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            n++;
                        }
                    }
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f q;
                            s32 zone2;

                            plAskPos(gGameWork.players[i], &q.x, &q.y, &q.z, &zone2);
                            switch (n) {
                                case 1:
                                    q.z = 0.0f;
                                    break;
                                case 2:
                                    q.z = 5 - i * 10;
                                    break;
                                case 3:
                                    q.z = 5 - i * 5;
                                    break;
                                case 4:
                                    q.z = 5.0 - i * 3.3;
                                    break;
                            }
                            plSetPos(gGameWork.players[i], zone2, &q);
                            plSetAction(gGameWork.players[i], 0);
                        }
                    }
                    dochukiBehimosSetState8(w->behimos);
                    camAiSetHold(0);
                    w->count = 50;
                    dochukiMainShowGauge();
                    dochukiVram1StopCycle(w->vram1);
                    gGameWork.scenarioRunning = 1;
                    w->state = 10;
                    eneRumbleStopAll();
                    break;
                }
                case 19:
                    camAiSetHold(1);
                    camSetAt(cam, -233.0f, 72.0f, 71.0f);
                    camSetEye(cam, -172.0f, 80.0f, 219.0f);
                    dochukiVram1Hide(w->vram1);
                    dochukiVram2Start(w->vram2);
                    dochukiChinkoShow(w->chinko);
                    dochukiBehimosSetState7(w->behimos);
                    eneEndPlayersPowerDown();
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f q;
                            s32 zone2;

                            plAskPos(gGameWork.players[i], &q.x, &q.y, &q.z, &zone2);
                            q.x = 90.0f;
                            q.y = 60.0f;
                            plSetPos(gGameWork.players[i], zone2, &q);
                            plSetAction(gGameWork.players[i], 0xDC);
                            q.x = 64.0;
                            q.y = 55.0f;
                            plDemoWalkTo(gGameWork.players[i], q, 2.0f);
                        }
                    }
                    w->state = 20;
                    break;
                case 20:
                    for (i = 0, c = 0; i < 4; i++) {
                        if (plIsInPlay(i) && plIsDemoWalking(gGameWork.players[i])) {
                            c++;
                        }
                    }
                    if (c == 0) {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plSetAction(gGameWork.players[i], 0);
                            }
                        }
                        w->state = 21;
                    }
                    break;
                case 21:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f q;
                            s32 zone2;

                            plAskPos(gGameWork.players[i], &q.x, &q.y, &q.z, &zone2);
                            if (q.x >= 115.0f) {
                                w->state = 22;
                            }
                        }
                    }
                    break;
                case 22:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f q;
                            s32 zone2;

                            plSetAction(gGameWork.players[i], 0xF);
                            plAskPos(gGameWork.players[i], &q.x, &q.y, &q.z, &zone2);
                            q.x = 342.0f;
                            q.y = 120.0f;
                            plSetPos(gGameWork.players[i], zone2, &q);
                            pl = PL(gGameWork.players[i]);
                            pl->tilt = 0.0f;
                        }
                    }
                    camSetAt(cam, 63.0f, 84.0f, 97.0f);
                    camSetEye(cam, 63.0f, 107.0f, 255.0f);
                    dochukiVram1Show(w->vram1);
                    dochukiVram2Stop(w->vram2);
                    eneRumbleStopAll();
                    dochukiChinkoHide(w->chinko);
                    dochukiBehimosSetState47(w->behimos);
                    camAiSetHold(0);
                    w->state = 10;
                    break;
                case 23:
                    gGameWork.scenarioRunning = 2;
                    sndPlay(9);
                    camAiSetHold(1);
                    camSetAt(cam, 151.0f, 117.0f, 74.0f);
                    camSetEye(cam, 250.0f, 67.0f, 189.0f);
                    dochukiVram1Hide(w->vram1);
                    dochukiVram2Stop(w->vram2);
                    dochukiChinkoHide(w->chinko);
                    eneEndPlayersPowerDown();
                    dochukiBehimosSetState55(w->behimos);
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i)) {
                            Vec3f q;
                            s32 zone2;

                            plAskPos(gGameWork.players[i], &q.x, &q.y, &q.z, &zone2);
                            q.x = 520.0f;
                            q.y = 69.0f;
                            plSetPos(gGameWork.players[i], zone2, &q);
                            plSetAction(gGameWork.players[i], 0xD7);
                        }
                    }
                    w->state = 24;
                    break;
                case 24:
                    if (dochukiBehimosIsState59(w->behimos)) {
                        eneRumbleStopAll();
                        eneCounterSetFlag(4);
                        gSaveData.stageFlags[gGameWork.area * 16 + gGameWork.areaStage] |= 5;
                        dmenuQueueDemo(14);
                    }
                    break;
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

s32 dochukiMainGetCount(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    if (o != NULL) {
        return MAIN(o)->count;
    }
    return -1;
}

void dochukiMainShowGauge(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    MainWork *w;

    if (o != NULL) {
        w = MAIN(o);
        pipGaugeOn(w->gauge);
        pipGaugeSetTarget(w->gauge, w->count);
    }
}

s32 dochukiMainTakeCount(s32 n) {
    s32 pad;
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);
    MainWork *w;

    if (o != NULL) {
        w = MAIN(o);
        w->count -= n;
        if (w->count <= 0) {
            w->count = 0;
        }
        pipGaugeSetTarget(w->gauge, w->count);
        return w->count;
    }
    return -1;
}

void dochukiMainStartDochukiEnd(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);
    s32 pad;
    MainWork *w;

    if (o != NULL) {
        w = MAIN(o);
        w->state = 11;
        pipGaugeOff(w->gauge);
        dochukiSetState19(w->dochuki);
        gGameWork.scenarioRunning = 2;
        dochukiVram1StartCycle(w->vram1);
    }
}

void dochukiMainSetState13(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    if (o != NULL) {
        MAIN(o)->state = 13;
    }
}

void dochukiMainStartWalk(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    if (o != NULL) {
        MAIN(o)->state = 19;
    }
}

void dochukiMainStartBehimosEnd(void) {
    Obj *o = (Obj *) tskSendMsg(gEneManager, 10, (void *) MAIN_ID);

    if (o != NULL) {
        MAIN(o)->state = 23;
    }
}

void dochukiVram1Show(Obj *o) {
    VRAM1(o)->model->f.flags |= 2;
}

void dochukiVram1Hide(Obj *o) {
    VRAM1(o)->model->f.flags &= ~2;
}

void dochukiVram1StartCycle(Obj *o) {
    Vram1Work *w = VRAM1(o);

    w->cycling = 1;
    w->alpha = 170;
    w->r = 50; w->g = 70; w->b = 250;
    w->tr = 50; w->tg = 70; w->tb = 250;
    w->step = 0;
    w->wait = 1;
}

void dochukiVram1StopCycle(Obj *o) {
    VRAM1(o)->cycling = 0;
}

/* "do_vram1": fades in, then cycles its colour through four targets once cycling is on. */
s32 dochukiVram1Proc(Obj *o, s16 msg, void *arg) {
    Vram1Work *w = VRAM1(o);
    s32 pad;
    PolFrame *m;

    switch (msg) {
        case 2:
            o->name = "do_vram1";
            eneSetup(o, arg);
            w->model = eneNewPol2(o, dochukiVram1Dl, (Gfx *) fadeSegToRam(doVram1Dl, o->file));
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            frmMove(&m->f, gFrmLayer8);
            eneSetModel(o, &m->f);
            w->cycling = 0;
            w->alpha = 170;
            w->r = 50; w->g = 70; w->b = 250;
            w->tr = 50; w->tg = 70; w->tb = 250;
            w->step = 0;
            w->wait = 1;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 3:
            m = w->model;
            m->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(doVram1Dl, o->file));
            gDPSetEnvColor(gGfxDlHead++, w->r, w->g, w->b, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            m = w->model;
            if (w->cycling == 0) {
                if (w->alpha != 255) {
                    w->alpha += 5;
                }
            }
            if (w->alpha == 255) {
                if (w->wait-- == 0) {
                    switch (w->step) {
                        case 0:
                            w->tr = 250;
                            w->tg = 0;
                            w->tb = 0;
                            break;
                        case 1:
                            w->tr = 0;
                            w->tg = 250;
                            w->tb = 0;
                            break;
                        case 2:
                            w->tr = 250;
                            w->tg = 250;
                            w->tb = 0;
                            break;
                        case 3:
                            w->tr = 0;
                            w->tg = 0;
                            w->tb = 250;
                            break;
                    }
                    w->wait = 60;
                    w->step++; w->step &= 3;
                }
                if (w->r < w->tr) {
                    w->r += 10;
                }
                if (w->tr < w->r) {
                    w->r -= 10;
                }
                if (w->g < w->tg) {
                    w->g += 10;
                }
                if (w->tg < w->g) {
                    w->g -= 10;
                }
                if (w->b < w->tb) {
                    w->b += 10;
                }
                if (w->tb < w->b) {
                    w->b -= 10;
                }
            }
            eneSetModel(o, &m->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void dochukiVram2Start(Obj *o) {
    Vram2Work *w = VRAM2(o);
    PolFrame *m = w->model;

    w->state = 1;
    w->alpha = 0;
    w->fade = 1;
    m->f.flags |= 2;
}

void dochukiVram2Stop(Obj *o) {
    s32 pad;
    Vram2Work *w = VRAM2(o);
    PolFrame *m = w->model;

    w->state = 0;
    w->alpha = 0;
    w->fade = 1;
    tskDestroyChildren(o);
    m->f.flags &= ~2;
}

void dochukiPuffYellow(Obj *parent, f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexPuffAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xFF; init.prim.b = 0xA0;
    init.env.r = 0x60; init.env.g = 0x60; init.env.b = 0x50;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.vel.x = vx;
    init.vel.y = vy;
    init.vel.z = vz;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    init.grow.x = init.grow.y = init.grow.z = 0.98f;
    init.alpha = 0x80;
    tskCreate(parent, itex, 0, &init);
}

void dochukiPuffWhite(Obj *parent, f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x123;
    init.frames = gItexPuffAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xFF; init.prim.b = 0xFF;
    init.env.r = 0xC0; init.env.g = 0xC0; init.env.b = 0xC0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.vel.x = vx;
    init.vel.y = vy;
    init.vel.z = vz;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    init.grow.x = init.grow.y = init.grow.z = 1.02f;
    init.alpha = 0xFF;
    tskCreate(parent, itex, 0, &init);
}

/* "do_vram2" */
s32 dochukiVram2Proc(Obj *o, s16 msg, void *arg) {
    Vram2Work *w = VRAM2(o);
    s32 pad;
    PolFrame *m;

    switch (msg) {
        case 2:
            o->name = "do_vram2";
            eneSetup(o, arg);
            w->model = eneNewPol2(o, dochukiVram2Dl, (Gfx *) fadeSegToRam(dochukiOpaEnvDl, o->file));
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            eneSetModel(o, &m->f);
            w->alpha = 0;
            w->r = 255;
            w->g = 255;
            w->b = 255;
            w->red = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 3:
            m = w->model;
            m->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(dochukiOpaEnvDl, o->file));
            gDPSetEnvColor(gGfxDlHead++, w->r, w->g, w->b, (u8) w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            m = w->model;
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    if (w->puffWait-- <= 0) {
                        f32 x;
                        f32 y;
                        f32 z;
                        f32 size;
                        f32 vx;

                        if (w->alpha < 32) {
                            w->puffWait = 5;
                        } else if (w->alpha < 64) {
                            w->puffWait = 4;
                        } else if (w->alpha < 96) {
                            w->puffWait = 3;
                        } else if (w->alpha < 128) {
                            w->puffWait = 2;
                        } else {
                            w->puffWait = 0;
                        }
                        x = -160 - fadeRand(60);
                        y = fadeRand(30) + 55;
                        z = fadeRand(40) + 77;
                        vx = (fadeRand(20) + 20) / -10.0f;
                        size = (fadeRand(10) + 10) / 100.0f;
                        dochukiPuffYellow(o, x, y, z, vx, 0.0f, 0.0f, size);
                    }
                    w->fade = 1;
                    if (w->alpha >= 160) {
                        w->state = 2;
                        eneRumbleAll(0xBBBBBBBB, 100);
                        sndPlay(0x167);
                    }
                    break;
                case 2: {
                    f32 y2;
                    f32 z2;
                    f32 size2;
                    f32 vx2;
                    s32 i;
                    s32 zone;
                    Vec3f p;

                    y2 = fadeRand(30) + 55;
                    z2 = fadeRand(40) + 77;
                    vx2 = (fadeRand(20) + 40) / 10.0f;
                    size2 = (fadeRand(10) + 10) / 100.0f;
                    dochukiPuffWhite(o, -280.0f, y2, z2, vx2, 0.0f, 0.0f, size2);
                    w->fade = -8;
                    if (w->alpha <= 80) {
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i)) {
                                plAskPos(gGameWork.players[i], &p.x, &p.y, &p.z, &zone);
                                p.x += 4.0f;
                                plSetPos(gGameWork.players[i], zone, &p);
                            }
                        }
                    }
                    break;
                }
            }
            w->alpha += w->fade;
            if (w->alpha >= 255) {
                w->alpha = 255;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
            }
            if (w->red == 0) {
                w->r = 255;
                w->g = 255;
                w->b = 255;
            } else {
                w->r = 255;
                w->g = 0;
                w->b = 0;
            }
            eneSetModel(o, &m->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void dochukiEyesOpen(Obj *o) {
    DOCHUKI(o)->model->segs[3] = eneFileAddr(o, dochukiEyesOpenTex, 0);
}

/* An empty function: the game has its "jr ra; nop" between the two (splat counts it in the
 * one above). */
void dochukiUnusedStub(void) {
}

void dochukiBlink(Obj *o, s32 wait) {
    DochukiWork *w = DOCHUKI(o);

    if (w->blinkWait-- <= 0) {
        w->blinkWait = wait;
        if (w->blink ^= 1) {
            w->model->segs[3] = eneFileAddr(o, dochukiBlinkTex, 0);
        } else {
            w->model->segs[3] = eneFileAddr(o, dochukiEyesOpenTex, 0);
        }
    }
}

void dochukiStop(Obj *o) {
    DOCHUKI(o)->vel[0] = 0.0f;
    DOCHUKI(o)->vel[1] = 0.0f;
    DOCHUKI(o)->vel[2] = 0.0f;
}

void dochukiGhostAdd(Obj *o) {
    DochukiWork *w = DOCHUKI(o);
    AnimeFrame *m = w->model;
    AnimeFrame *g;
    s32 i;

    if (w->ghostWait == 0) {
        for (i = 0; i < 5; i++) {
            if (w->ghosts[i] == NULL) {
                w->ghosts[i] = eneNewAnime2(o, m->anim,
                                            (Gfx *) fadeSegToRam(dochukiXluEnvDl, o->file));
                if (w->ghosts[i] != NULL) {
                    w->ghostWait = 2;
                    g = w->ghosts[i];
                    g->frame = m->frame;
                    w->ghostAlpha[i] = 0x80;
                    eneSetModelOnCourse(o, &g->f);
                }
                return;
            }
        }
    }
}

void dochukiGhostFade(Obj *o) {
    DochukiWork *w = DOCHUKI(o);
    s32 i;

    if (w->ghostWait != 0) {
        w->ghostWait--;
    }
    for (i = 0; i < 5; i++) {
        if (w->ghostAlpha[i] != 0) {
            w->ghostAlpha[i] -= 16;
        }
        if (w->ghostAlpha[i] == 0 && w->ghosts[i] != NULL) {
            frmDestroy(&w->ghosts[i]->f);
            w->ghosts[i] = NULL;
        }
    }
}

void dochukiGhostClear(Obj *o) {
    DochukiWork *w = DOCHUKI(o);
    s32 i;

    for (i = 0; i < 5; i++) {
        if (w->ghosts[i] != NULL) {
            frmDestroy(&w->ghosts[i]->f);
        }
        w->ghosts[i] = NULL;
        w->ghostAlpha[i] = 0;
    }
}

s32 dochukiTurnToPlayer(Obj *o, s32 pl, f32 step) {
    f32 angle;
    f32 x;
    f32 y;
    f32 z;
    s32 pad;
    AnimeFrame *m = DOCHUKI(o)->model;

    step = fabsf(step);
    if (!plIsInPlay(pl)) {
        pl = eneRandomPlayerIndex();
        if (pl == 0) {
            return TRUE;
        }
    }
    plGetModelPos(gGameWork.players[pl], &x, &y, &z);
    angle = mathAtan2(x - m->pos[0], z - m->pos[2]);
    if (eneTurnToward(o, angle, step)) {
        return TRUE;
    }
    return FALSE;
}

void dochukiAccelAxis(f32 target, f32 max, f32 accel, f32 *pos, f32 *vel) {
    f32 d = target - *pos;

    if (d >= 0.0f) {
        d = (accel > 0.0f) ? accel : -accel;
        *vel += d;
        if (max <= *vel) {
            *vel = max;
        }
        *pos += *vel;
        d = target - *pos;
        if (d < 0.0f) {
            *vel = max;
        }
    } else {
        d = (accel > 0.0f) ? accel : -accel;
        *vel -= d;
        if (*vel <= -max) {
            *vel = -max;
        }
        *pos += *vel;
        d = target - *pos;
        if (d >= 0.0f) {
            *vel = -max;
        }
    }
}

void dochukiSetStateWait(Obj *o, s32 state, s32 wait) {
    DOCHUKI(o)->state = state;
    DOCHUKI(o)->wait = wait;
    DOCHUKI(o)->steps++;
}

void dochukiSetState1(Obj *o) {
    DOCHUKI(o)->state = 1;
}

void dochukiSetState2(Obj *o) {
    DOCHUKI(o)->state = 2;
}

void dochukiSetState3(Obj *o) {
    DOCHUKI(o)->state = 3;
}

void dochukiSetState6(Obj *o) {
    DOCHUKI(o)->state = 6;
    DOCHUKI(o)->y = 69.0f;
}

void dochukiSetState19(Obj *o) {
    DOCHUKI(o)->state = 19;
}

void dochukiOnPlayerHit(Obj *o) {
    DOCHUKI(o)->atkHitTimer = 30;
}

u8 dochukiFlipToggle(Obj *o) {
    return DOCHUKI(o)->toggle ^= 1;
}

f32 dochukiGetNodePos(Obj *o, f32 *x, f32 *y, f32 *z) {
    s32 pad;
    f32 nx = 0.0f;
    f32 ny = 0.0f;
    f32 nz = 0.0f;
    AnimeFrame *m = DOCHUKI(o)->model;

    if (m->anim == dochukiState8Anim) {
        ((NodePosFunc) mdlAnimNodePos)((u32) dochukiState8Attach, m->frame, eneFileAddr(o, 0, 0), &nx, &ny, &nz);
        *x = m->pos[0] + (nz * sinf(m->rot[1]) + cosf(m->rot[1]) * nx) / 10.0f;
        *z = m->pos[2] + (nz * cosf(m->rot[1]) - sinf(m->rot[1]) * nx) / 10.0f;
        *y = m->pos[1] + ny / 10.0f;
        return m->frame;
    }
    return -1.0f;
}

s32 dochukiGetTarget(Obj *o) {
    if (!plIsInPlay(DOCHUKI(o)->target)) {
        DOCHUKI(o)->target = eneRandomPlayerIndex();
    }
    return DOCHUKI(o)->target;
}

void dochukiDestroy(Obj *o) {
    tskDestroy(o);
}

void dochukiEffectSetGrow(Obj *o, f32 grow);

extern u8 dochukiState6Anim[];
extern u8 dochukiState13Anim[];
extern u8 dochukiState16Anim[];
extern u8 dochukiState19Anim[];

/* "dochuki", the first boss: drops in (state 1), waits for the director (2, 3), then hovers
 * after the players' centre with after-images (7), on a step count slamming (8-12: kind 0x37
 * where it lands) or spitting seven "ntama" (13, 14: kind 0x38); a hit costs the gauge twice its
 * strength and knocks it back (16, 17; at zero the director's dochukiMainStartDochukiEnd). Its
 * end (19-21) makes kind 0x39 and tells the director (dochukiMainSetState13). Message 6 hands a
 * rider its speed.
 * Matched 2026-10-16; it was parked for its frame and then for the ghost pointer's register
 * (the game's a3). */
s32 dochukiProc(Obj *o, s16 msg, void *arg);
s32 dochukiProc(Obj *o, s16 msg, void *arg) {
    DochukiWork *w = DOCHUKI(o);
    AnimeFrame *m;
    s32 pad;
    s32 i;

    switch (msg) {
        case 2:
            o->name = "dochuki";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) dochukiState6Anim);
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            eneSetModelOnCourse(o, &m->f);
            frmMove(&m->f, gFrmLayer2);
            dochukiEyesOpen(o);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &m->f, 50, 70, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            if (w->atk != NULL) {
                w->atk->kind = 120;
            }
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &m->f, 120, 500, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 101;
            w->rot[1] = eneCourseAngle(o, 4);
            w->target = eneRandomPlayerIndex();
            w->retarget = 100;
            w->wait = 180;
            w->tintAlpha = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->ride != NULL) {
                frmDestroy(w->ride);
            }
            dochukiGhostClear(o);
            break;
        case 6:
            ((f32 *) arg)[1] += w->vel[0];
            ((f32 *) arg)[3] += w->vel[1];
            break;
        case 3:
            m = w->model;
            m->dl = gGfxDlHead;
            if (w->hurt & 2) {
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexDl);
                gDPSetEnvColor(gGfxDlHead++, 0xD0, 0xD0, 0xD0, 0xA0);
            } else {
                m->dl = gGfxDlHead;
                gSPDisplayList(gGfxDlHead++, gDrawSetupEnvTintTexShadeDl);
                gDPSetEnvColor(gGfxDlHead++, 0x40, 0x40, 0x40, w->tintAlpha);
            }
            gSPEndDisplayList(gGfxDlHead++);
            for (i = 0; i < 5; i++) {
                s32 pad2;

                if (w->ghosts[i] != NULL) {
                    AnimeFrame *g = w->ghosts[i];

                    g->dl = gGfxDlHead;
                    gSPDisplayList(gGfxDlHead++, fadeSegToRam(dochukiXluEnvDl, o->file));
                    gDPSetEnvColor(gGfxDlHead++, 0x50, 0x32, 0xFF, w->ghostAlpha[i]);
                    gDPSetPrimColor(gGfxDlHead++, 0, 0, 0, 0, 0, w->ghostAlpha[i] / 2);
                    gSPEndDisplayList(gGfxDlHead++);
                }
            }
            break;
        case 0:
            m = w->model;
            if (w->hurt-- <= 0) {
                w->hurt = 0;
            }
            if (w->unused96-- <= 0) {
                w->unused96 = 0;
            }
            if (w->atkHitTimer-- <= 0) {
                w->atkHitTimer = 0;
            }
            dochukiGhostFade(o);
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                w->atkHitTimer = 30;
            }
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1 && w->state < 16 && w->hurt == 0) {
                dochukiMainTakeCount(eneHitStrength(w->dmg) * 2);
                w->hurt = 30;
                w->state = 16;
                eneHitSoundHeavy(w->dmg);
            }
            if (w->atkHitTimer == 1) {
                sndPlay(0x303);
            }
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    eneAnimLoop(o, w->model, 0.5f);
                    w->vel[1] = -0.5f;
                    eneMoveObj(o);
                    dochukiGhostAdd(o);
                    if (w->y <= 69.0f) {
                        w->y = 69.0f;
                        w->state = 2;
                        w->vel[1] = 0.0f;
                    }
                    break;
                case 2:
                    eneAnimLoop(o, w->model, 0.3f);
                    dochukiEyesOpen(o);
                    break;
                case 3:
                    eneAnimLoop(o, w->model, 0.3f);
                    dochukiBlink(o, fadeRand(4) + 2);
                    break;
                case 6:
                    dochukiEyesOpen(o);
                    m->anim = dochukiState6Anim;
                    m->frame = 0.0f;
                    w->state = 7;
                    /* fallthrough */
                case 7: {
                    s32 pad3;
                    f32 x;
                    f32 y;
                    f32 z;

                    if (w->retarget-- <= 0) {
                        w->retarget = 100;
                        w->target = eneRandomPlayerIndex();
                    }
                    w->y = 69.0f;
                    enePlayersCenterOnCourse(&x, &y, &z);
                    dochukiAccelAxis(x, 4.0f, 0.12f, &w->x, &w->vel[0]);
                    dochukiAccelAxis(0.0f, 3.0f, 0.08f, &w->z, &w->vel[2]);
                    dochukiGhostAdd(o);
                    dochukiGetTarget(o);
                    dochukiTurnToPlayer(o, w->target, 0.07853982f);
                    if (w->x < 150.0f) {
                        w->vel[0] = fabsf(w->vel[0]);
                    }
                    if (w->x > 530.0f) {
                        w->vel[0] = -fabsf(w->vel[0]);
                    }
                    eneAnimLoop(o, w->model, 0.5f);
                    if (w->wait-- <= 0) {
                        switch (w->steps) {
                            case 0:
                                dochukiSetStateWait(o, 13, 40);
                                break;
                            case 1:
                                dochukiSetStateWait(o, 13, 40);
                                break;
                            case 2:
                                dochukiSetStateWait(o, 13, 40);
                                break;
                            case 3:
                                dochukiSetStateWait(o, 13, 80);
                                break;
                            case 4:
                                dochukiSetStateWait(o, 8, 40);
                                w->steps = 0;
                                break;
                        }
                    }
                    break;
                }
                case 8: {
                    Vec3f p;
                    s32 pad4[9];

                    m->anim = dochukiState8Anim;
                    m->frame = 0.0f;
                    SoundLocation2(0x17D, gGameWork.view, m->pos[0], m->pos[1], m->pos[2], 0.0f, 300.0f,
                                   gGameWork.view->fovy);
                    eneMakeUnder(o, ENE_DOCHUKI_GENKI, 0, m->pos[0], m->pos[1], m->pos[2]);
                    w->swing = 0;
                    enePlayersCenterOnCourse(&p.x, &p.y, &p.z);
                    if (w->x < p.x) {
                        p.x -= 50.0f;
                    } else {
                        p.x += 50.0f;
                    }
                    w->vel[0] = (p.x - w->x) / 20.0f;
                    w->state = 9;
                    w->vel[1] = 0.0f;
                    w->vel[2] = (0.0f - w->z) / 20.0f;
                }
                    /* fallthrough */
                case 9:
                    dochukiGhostAdd(o);
                    dochukiGetTarget(o);
                    dochukiTurnToPlayer(o, w->target, 0.07853982f);
                    if (w->swing == 0) {
                        eneAnimPlayOnce(o, m, 0.4f);
                        if (m->frame > 10.0f) {
                            w->swing = 1;
                        }
                    } else {
                        eneAnimPlayOnce(o, m, -0.2f);
                        if (m->frame < 9.0f) {
                            w->swing = 0;
                        }
                    }
                    if (w->x < 150.0f) {
                        w->vel[0] = fabsf(w->vel[0]);
                    }
                    if (w->x > 530.0f) {
                        w->vel[0] = -fabsf(w->vel[0]);
                    }
                    if (w->vel[2] > 0.0f) {
                        eneMoveObj(o);
                        if (w->z > 0.0f) {
                            w->z = 0.0f;
                            dochukiStop(o);
                        }
                    } else if (w->vel[2] < 0.0f) {
                        eneMoveObj(o);
                        if (w->z < 0.0f) {
                            w->z = 0.0f;
                            dochukiStop(o);
                        }
                    } else {
                        w->timer = 1;
                        w->state = 10;
                    }
                    break;
                case 10:
                    dochukiGhostAdd(o);
                    dochukiGetTarget(o);
                    dochukiTurnToPlayer(o, w->target, 0.07853982f);
                    if (w->swing == 0) {
                        eneAnimPlayOnce(o, w->model, 0.5f);
                        if (m->frame > 10.0f) {
                            w->swing = 1;
                        }
                    } else {
                        eneAnimPlayOnce(o, w->model, -0.3f);
                        if (m->frame < 9.0f) {
                            w->swing = 0;
                        }
                    }
                    if (w->timer-- <= 0) {
                        w->state = 11;
                    }
                    break;
                case 11:
                    dochukiGhostAdd(o);
                    dochukiGetTarget(o);
                    dochukiTurnToPlayer(o, w->target, 0.07853982f);
                    if (eneAnimPlayOnce(o, w->model, 0.3f)) {
                        w->timer = 30;
                        w->state = 12;
                    }
                    break;
                case 12:
                    if (w->timer-- <= 0) {
                        w->state = 6;
                    }
                    break;
                case 13:
                    m->anim = dochukiState13Anim;
                    m->frame = 0.0f;
                    w->swing = 0;
                    w->state = 14;
                    /* fallthrough */
                case 14:
                    dochukiGhostAdd(o);
                    dochukiGetTarget(o);
                    dochukiTurnToPlayer(o, w->target, 0.07853982f);
                    if (m->frame >= 6.0f && w->swing == 0) {
                        w->swing = 1;
                        SoundLocation2(0x21C, gGameWork.view, m->pos[0], m->pos[1], m->pos[2], 0.0f, 1000.0f,
                                       gGameWork.view->fovy);
                        eneMakeEx(o, ENE_DOCHUKI_NTAMA, 0, m->pos[0], m->pos[1] + 40.0f, m->pos[2], 2,
                                  (u32 (*)(void *, void *, void *)) -90);
                        eneMakeEx(o, ENE_DOCHUKI_NTAMA, 0, m->pos[0], m->pos[1] + 40.0f, m->pos[2], 2,
                                  (u32 (*)(void *, void *, void *)) -60);
                        eneMakeEx(o, ENE_DOCHUKI_NTAMA, 0, m->pos[0], m->pos[1] + 40.0f, m->pos[2], 2,
                                  (u32 (*)(void *, void *, void *)) -30);
                        eneMakeEx(o, ENE_DOCHUKI_NTAMA, 0, m->pos[0], m->pos[1] + 40.0f, m->pos[2], 2,
                                  (u32 (*)(void *, void *, void *)) 0);
                        eneMakeEx(o, ENE_DOCHUKI_NTAMA, 0, m->pos[0], m->pos[1] + 40.0f, m->pos[2], 2,
                                  (u32 (*)(void *, void *, void *)) 30);
                        eneMakeEx(o, ENE_DOCHUKI_NTAMA, 0, m->pos[0], m->pos[1] + 40.0f, m->pos[2], 2,
                                  (u32 (*)(void *, void *, void *)) 60);
                        eneMakeEx(o, ENE_DOCHUKI_NTAMA, 0, m->pos[0], m->pos[1] + 40.0f, m->pos[2], 2,
                                  (u32 (*)(void *, void *, void *)) 90);
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.5f)) {
                        w->state = 6;
                    }
                    break;
                case 16:
                    m->anim = dochukiState16Anim;
                    m->frame = 0.0f;
                    w->vel[0] = eneKnockbackSpeed(o, w->dmg, 3.0f);
                    w->state = 17;
                    /* fallthrough */
                case 17:
                    w->vel[0] *= 0.9;
                    w->x += w->vel[0];
                    dochukiAccelAxis(0.0f, 3.0f, 0.08f, &w->z, &w->vel[2]);
                    if (eneAnimPlayOnce(o, m, 0.3f)) {
                        if (dochukiMainGetCount() == 0) {
                            dochukiMainStartDochukiEnd();
                        } else {
                            w->state = 6;
                        }
                    }
                    break;
                case 19:
                    m->anim = dochukiState19Anim;
                    m->frame = 0.0f;
                    w->z = 0.0f;
                    w->vel[0] = 0.0f;
                    w->vel[1] = 0.0f;
                    w->vel[2] = 0.0f;
                    w->x = 340.0f;
                    w->y = 69.0;
                    w->rot[1] = eneCourseAngle(o, 4);
                    w->sounded = 0;
                    w->state = 20;
                    /* fallthrough */
                case 20:
                    eneAnimPlayOnce(o, m, 0.3f);
                    if (w->sounded == 0 && m->frame >= 7.4f) {
                        w->sounded = 1;
                        sndPlay(0x21B);
                    }
                    if (m->frame >= 11.0f) {
                        w->timer = 100;
                        m->frame = 11.0f;
                        w->state = 21;
                    }
                    break;
                case 21:
                    if (w->timer == 40) {
                        sndPlay(0x188);
                        w->rings = eneMakeUnder(o, ENE_DOCHUKI_EFFECT, w->zone, w->x, w->y, w->z);
                    }
                    if (w->timer-- <= 0) {
                        dochukiEffectSetGrow(w->rings, 1.2f);
                        if (eneAnimPlayOnce(o, m, 0.3f)) {
                            dochukiMainSetState13();
                            w->state = 22;
                        }
                    }
                    break;
                case 22:
                    break;
            }
            eneSetModelOnCourse(o, &w->model->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void dochukiBehimosAddHp(Obj *o, s32 n) {
    BehimosWork *w = BEHIMOS(o);

    w->hp += n;
    if (w->hp >= 64) {
        w->hp = 64;
    }
    if (w->hp <= 0) {
        w->hp = 0;
    }
}

void dochukiBehimosSetStateWait(Obj *o, s32 state, s32 wait) {
    BEHIMOS(o)->state = state;
    BEHIMOS(o)->wait = wait;
    BEHIMOS(o)->steps++;
}

void dochukiBehimosSetState1(Obj *o) {
    BEHIMOS(o)->state = 1;
}

void dochukiBehimosSetState3(Obj *o) {
    BEHIMOS(o)->state = 3;
}

s32 dochukiBehimosIsState6(Obj *o) {
    if (BEHIMOS(o)->state == 6) {
        return TRUE;
    }
    return FALSE;
}

s32 dochukiBehimosIsState59(Obj *o) {
    if (BEHIMOS(o)->state == 59) {
        return TRUE;
    }
    return FALSE;
}

void dochukiBehimosSetState8(Obj *o) {
    BEHIMOS(o)->state = 8;
}

void dochukiBehimosSetState7(Obj *o) {
    BEHIMOS(o)->state = 7;
}

void dochukiBehimosSetState47(Obj *o) {
    BEHIMOS(o)->state = 47;
}

void dochukiBehimosTakeHit(Obj *o) {
    BehimosWork *w = BEHIMOS(o);

    w->flash = 30;
    w->hits++;
    if (w->hits >= 5) {
        w->hits = 0;
        w->state = 51;
    } else {
        w->state = 43;
    }
}

void dochukiBehimosSetState55(Obj *o) {
    BEHIMOS(o)->state = 55;
}

s32 dochukiBehimosIsSneezing(Obj *o) {
    AnimeFrame *m = BEHIMOS(o)->model;

    if (m->anim == dochukiBehimosSneezeAnim &&6.0f <= m->frame && m->frame <= 11.0f) {
        return TRUE;
    }
    return FALSE;
}

void dochukiBehimosGetPos(Obj *o, f32 *x, f32 *y, f32 *z) {
    AnimeFrame *m = BEHIMOS(o)->model;

    *x = m->pos[0];
    *z = m->pos[2];
    *y = m->pos[1];
}

f32 dochukiBehimosGetMouthPos(Obj *o, f32 *x, f32 *y, f32 *z) {
    s32 pad;
    f32 nx = 0.0f;
    f32 ny = 0.0f;
    f32 nz = 0.0f;
    AnimeFrame *m = BEHIMOS(o)->model;

    if (m->anim == dochukiBehimosState34Anim) {
        ((NodePosFunc) mdlAnimNodePos)((u32) dochukiBehimosState34Attach, m->frame, eneFileAddr(o, 0, 0), &nx, &ny, &nz);
    } else if (m->anim == dochukiBehimosState28Anim) {
        ((NodePosFunc) mdlAnimNodePos)((u32) dochukiBehimosState28Attach, m->frame, eneFileAddr(o, 0, 0), &nx, &ny, &nz);
    } else {
        return -1.0f;
    }
    *x = m->pos[0] + m->scale[0] * (nz * sinf(m->rot[1]) + cosf(m->rot[1]) * nx);
    *z = m->pos[2] + m->scale[1] * (nz * cosf(m->rot[1]) - sinf(m->rot[1]) * nx);
    *y = ny * m->scale[1] + m->pos[1];
    return m->frame;
}

void dochukiBehimosNextPattern(Obj *o) {
    BehimosWork *w = BEHIMOS(o);

    if (w->wait-- <= 0) {
        w->target = eneRandomPlayerIndex();
        switch (w->steps) {
            case 0:
                dochukiBehimosSetStateWait(o, 38, 1);
                break;
            case 1:
                dochukiBehimosSetStateWait(o, 38, 1);
                break;
            case 2:
                dochukiBehimosSetStateWait(o, 20, 1);
                break;
            case 3:
                dochukiBehimosSetStateWait(o, 38, 1);
                break;
            case 4:
                dochukiBehimosSetStateWait(o, 26, 1);
                break;
            case 5:
                dochukiBehimosSetStateWait(o, 38, 1);
                break;
            case 6:
                dochukiBehimosSetStateWait(o, 14, 1);
                break;
            case 7:
                dochukiBehimosSetStateWait(o, 38, 1);
                break;
            case 8:
                dochukiBehimosSetStateWait(o, 38, 1);
                break;
            case 9:
                dochukiBehimosSetStateWait(o, 32, 10);
                w->steps = 0;
                break;
        }
    }
}

extern u8 dochukiBehimosAnim[];
extern u8 dochukiBehimosState16Anim[];
extern u8 dochukiBehimosState22Anim[];
extern u8 dochukiBehimosState43Anim[];
extern u8 dochukiBehimosState51Anim[];
extern u8 dochukiBehimosState47Anim[];
extern u8 dochukiBehimosState10Anim[];

/* "behimos", the second boss: rises (states 1-6), then walks after the players' centre (8-11) and
 * on its step count (dochukiBehimosNextPattern) breathes "iki" (16-17, 22-23: kind 0x3A, its hp
 * swinging), swipes (28-29, 34-35: two kind 0x3C), walks to a player (14-15, 20-21, 26-27,
 * 32-33, 38-39) and sneezes (40: kind 0x3E, its nose "HANA"). A hit on the nose (43-44, 47-48,
 * 51-54) costs the gauge; at zero the director's dochukiMainStartBehimosEnd. Its end (55-58)
 * sinks it under a mist of puffs and makes kind 0x3F; state 59 is the director's "done".
 * Message 6 hands a rider its speed. */
s32 dochukiBehimosProc(Obj *o, s16 msg, void *arg);
s32 dochukiBehimosProc(Obj *o, s16 msg, void *arg) {
    BehimosWork *w = BEHIMOS(o);
    AnimeFrame *m;
    s32 pad; /* FAKE: a frame word (the game's eighth word above case 11's p); unused */

    switch (msg) {
        case 2:
            o->name = "behimos";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) dochukiBehimosAnim);
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            eneSetModelOnCourse(o, &m->f);
            frmMove(&m->f, gFrmLayer2);
            m->scale[0] = m->scale[1] = m->scale[2] = 0.5f;
            m->f.flags &= ~2;
            w->hp = 0;
            w->wait = 60;
            w->walk = 30;
            w->retarget = 10;
            w->target = eneRandomPlayerIndex();
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 6:
            ((f32 *) arg)[1] += w->vel[0];
            ((f32 *) arg)[3] += w->vel[1];
            break;
        case 3:
            m = w->model;
            m->dl = gGfxDlHead;
            if (w->flash & 2) {
                gSPDisplayList(gGfxDlHead++, fadeSegToRam(dochukiOpaEnvDl, o->file));
                gDPSetEnvColor(gGfxDlHead++, 0xD0, 0xD0, 0xD0, 0xA0);
            } else {
                gSPDisplayList(gGfxDlHead++, fadeSegToRam(dochukiOpaEnvDl, o->file));
                gDPSetEnvColor(gGfxDlHead++, 0xFF, 0x40, 0x40, (u8) w->hp);
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            m = w->model;
            if (w->flash-- <= 0) {
                w->flash = 0;
            }
            if (w->unused7A-- <= 0) {
                w->unused7A = 0;
            }
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    m->f.flags |= 2;
                    m->anim = dochukiBehimosAnim;
                    m->frame = 0.0f;
                    w->state = 2;
                    /* fallthrough */
                case 2:
                    eneAnimLoop(o, w->model, 0.3f);
                    break;
                case 3:
                    m->anim = dochukiBehimosSneezeAnim;
                    m->frame = 0.0f;
                    w->state = 4;
                    /* fallthrough */
                case 4:
                    eneAnimPlayOnce(o, w->model, 0.15f);
                    if (m->frame >= 4.0f) {
                        m->frame = 4.0f;
                        w->timer = 5;
                        w->state = 5;
                    }
                    break;
                case 5:
                    if (w->timer-- <= 0 && eneAnimPlayOnce(o, w->model, -0.07f)) {
                        m->anim = dochukiBehimosAnim;
                        m->frame = 0.0f;
                        w->state = 6;
                    }
                    break;
                case 6:
                    eneAnimLoop(o, w->model, 0.3f);
                    break;
                case 8:
                    m->anim = dochukiBehimosAnim;
                    m->frame = 0.0f;
                    w->state = 9;
                    /* fallthrough */
                case 9:
                    eneAnimLoop(o, w->model, 0.3f);
                    if (w->retarget-- <= 0) {
                        w->retarget = 100;
                        w->target = eneRandomPlayerIndex();
                    }
                    if (w->walk-- <= 0) {
                        w->walk = 100;
                        w->state = 10;
                    }
                    dochukiBehimosNextPattern(o);
                    break;
                case 10:
                    m->anim = dochukiBehimosState10Anim;
                    m->frame = 0.0f;
                    w->state = 11;
                    /* fallthrough */
                case 11: {
                    Vec3f p;
                    s32 near;

                    if (w->retarget-- <= 0) {
                        w->retarget = 100;
                        w->target = eneRandomPlayerIndex();
                    }
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    eneAnimLoop(o, w->model, 0.4f);
                    enePlayersCenterOnCourse(&p.x, &p.y, &p.z);
                    dochukiAccelAxis(p.x, 3.0f, 0.12f, &w->x, &w->vel[0]);
                    if (w->walk-- <= 0 || near == 1) {
                        w->walk = 20;
                        w->state = 8;
                    }
                    dochukiBehimosNextPattern(o);
                    break;
                }
                case 16:
                    m->frame = 0.0f;
                    m->anim = dochukiBehimosState16Anim;
                    SoundLocation2(0x2E5, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    dochukiPuffWhite(o, m->pos[0] + 10.0f, m->pos[1] + 100.0f, m->pos[2] + 50.0f, 0.2f, 0.2f,
                                           0.1f, 0.1f);
                    dochukiPuffWhite(o, m->pos[0] - 10.0f, m->pos[1] + 100.0f, m->pos[2] + 50.0f, -0.2f,
                                           0.2f, 0.1f, 0.1f);
                    w->sub = 0;
                    w->speed = 0.4f;
                    w->state = 17;
                    /* fallthrough */
                case 17:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    if (eneAnimLoop(o, m, w->speed)) {
                        w->state = 8;
                    }
                    if (m->frame >= 12.0f) {
                        dochukiBehimosAddHp(o, -10);
                    } else {
                        dochukiBehimosAddHp(o, 5);
                    }
                    if (m->frame >= 11.0f && w->sub == 0) {
                        w->speed = 0.3f;
                        w->sub = 1;
                        SoundLocation2(0x169, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                        eneMakeEx(o, ENE_DOCHUKI_BEAM, w->zone, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) 0);
                    }
                    break;
                case 22:
                    m->frame = 0.0f;
                    m->anim = dochukiBehimosState22Anim;
                    SoundLocation2(0x2E5, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    dochukiPuffWhite(o, m->pos[0] + 10.0f, m->pos[1] + 100.0f, m->pos[2] + 50.0f, 0.2f, 0.2f,
                                           0.1f, 0.1f);
                    dochukiPuffWhite(o, m->pos[0] - 10.0f, m->pos[1] + 100.0f, m->pos[2] + 50.0f, -0.2f,
                                           0.2f, 0.1f, 0.1f);
                    w->sub = 0;
                    w->speed = 0.4f;
                    w->state = 23;
                    /* fallthrough */
                case 23:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    if (eneAnimLoop(o, w->model, w->speed)) {
                        w->state = 8;
                    }
                    if (m->frame >= 12.0f) {
                        dochukiBehimosAddHp(o, -8);
                    } else {
                        dochukiBehimosAddHp(o, 4);
                    }
                    if (m->frame >= 11.0f && w->sub == 0) {
                        w->speed = 0.3f;
                        w->sub = 1;
                        SoundLocation2(0x169, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                        eneMakeEx(o, ENE_DOCHUKI_BEAM, w->zone, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) 1);
                    }
                    break;
                case 28:
                    m->anim = dochukiBehimosState28Anim;
                    m->frame = 0.0f;
                    w->sub = 0;
                    SoundLocation2(0x452, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    w->state = 29;
                    /* fallthrough */
                case 29:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    if (m->frame >= 5.0f) {
                        if (w->sub-- <= 0) {
                            w->sub = 2;
                            eneMakeEx(o, ENE_DOCHUKI_IKI, w->zone, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) 0);
                            eneMakeEx(o, ENE_DOCHUKI_IKI, w->zone, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) 1);
                        }
                    }
                    if (m->frame >= 8.0f) {
                        w->vel[0] = 3.0f;
                        eneMoveObj(o);
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.2f)) {
                        w->state = 8;
                    }
                    break;
                case 34:
                    m->anim = dochukiBehimosState34Anim;
                    m->frame = 0.0f;
                    w->sub = 0;
                    SoundLocation2(0x452, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    w->state = 35;
                    /* fallthrough */
                case 35:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    if (m->frame >= 5.0f) {
                        if (w->sub-- <= 0) {
                            w->sub = 2;
                            eneMakeEx(o, ENE_DOCHUKI_IKI, w->zone, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) 0);
                            eneMakeEx(o, ENE_DOCHUKI_IKI, w->zone, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) 1);
                        }
                    }
                    if (m->frame >= 8.0f) {
                        w->vel[0] = -3.0f;
                        eneMoveObj(o);
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.2f)) {
                        w->state = 8;
                    }
                    break;
                case 14:
                case 20:
                case 26:
                case 32:
                case 38: {
                    Vec3f p;
                    s32 zone;

                    m->anim = dochukiBehimosState10Anim;
                    m->frame = 0.0f;
                    if (!plIsInPlay(w->target)) {
                        if ((w->target = eneRandomPlayerIndex()) == 0) {
                            w->state = 8;
                        }
                    }
                    plAskPos(gGameWork.players[w->target], &p.x, &p.y, &p.z, &zone);
                    w->vel[0] = ((p.x < w->x) ? -1 : 1) * 4.0f;
                    w->state++;
                    break;
                }
                case 15:
                case 21:
                case 27:
                case 33: {
                    Vec3f p;
                    s32 zone;

                    eneTurnToward(o, 0.0f, 0.07853982f);
                    eneAnimLoop(o, w->model, 0.8f);
                    eneMoveObj(o);
                    if (!plIsInPlay(w->target)) {
                        if ((w->target = eneRandomPlayerIndex()) == 0) {
                            w->state = 8;
                        }
                    }
                    plAskPos(gGameWork.players[w->target], &p.x, &p.y, &p.z, &zone);
                    if (w->vel[0] < 0.0f && w->x <= p.x) {
                        w->vel[0] = 0.0f;
                    }
                    if (w->vel[0] > 0.0f && p.x <= w->x) {
                        w->vel[0] = 0.0f;
                    }
                    if (w->vel[0] == 0.0f) {
                        w->state++;
                    }
                    break;
                }
                case 39: {
                    Vec3f p;
                    s32 zone;

                    eneTurnToward(o, 0.0f, 0.07853982f);
                    eneAnimLoop(o, w->model, 0.8f);
                    eneMoveObj(o);
                    if (!plIsInPlay(w->target)) {
                        if ((w->target = eneRandomPlayerIndex()) == 0) {
                            w->state = 8;
                        }
                    }
                    plAskPos(gGameWork.players[w->target], &p.x, &p.y, &p.z, &zone);
                    if (w->vel[0] < 0.0f && w->x <= p.x) {
                        w->vel[0] = 0.0f;
                    }
                    if (w->vel[0] > 0.0f && p.x <= w->x) {
                        w->vel[0] = 0.0f;
                    }
                    if (w->vel[0] == 0.0f) {
                        m->anim = dochukiBehimosSneezeAnim;
                        m->frame = 0.0f;
                        w->sub = 0;
                        w->sounded = 0;
                        w->state = 40;
                    }
                    break;
                }
                case 40:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    if (eneAnimPlayOnce(o, m, 0.4f)) {
                        w->state = 8;
                    }
                    if (w->sounded == 0 && m->frame >= 2.4f) {
                        w->sounded = 1;
                        SoundLocation2(0x166, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                    }
                    if (w->sub == 0 && dochukiBehimosIsSneezing(o)) {
                        w->sub = 1;
                        eneMakeEx(o, ENE_DOCHUKI_HANA, w->zone, w->x, 69.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 0);
                    }
                    break;
                case 43:
                    SoundLocation2(0x2E6, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    m->anim = dochukiBehimosState43Anim;
                    m->frame = 0.0f;
                    w->state = 44;
                    /* fallthrough */
                case 44:
                    if (eneAnimPlayOnce(o, w->model, 0.2f)) {
                        if (dochukiMainGetCount() == 0) {
                            dochukiMainStartBehimosEnd();
                        } else {
                            w->state = 8;
                        }
                    }
                    break;
                case 47:
                    sndPlay(0x2E8);
                    m->anim = dochukiBehimosState47Anim;
                    m->frame = 0.0f;
                    w->timer = 20;
                    w->state = 48;
                    /* fallthrough */
                case 48:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    if (eneAnimPlayOnce(o, w->model, 0.2f)) {
                        if (w->timer-- <= 0) {
                            if (dochukiMainGetCount() == 0) {
                                dochukiMainStartBehimosEnd();
                            } else {
                                w->state = 8;
                            }
                        }
                    }
                    break;
                case 51:
                    SoundLocation2(0x2E7, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                   gGameWork.view->fovy);
                    m->anim = dochukiBehimosState51Anim;
                    m->frame = 0.0f;
                    w->timer = 20;
                    w->sounded = 0;
                    w->state = 52;
                    /* fallthrough */
                case 52:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    eneAnimPlayOnce(o, m, 0.2f);
                    if (w->sounded == 0 && m->frame >= 8.0) {
                        w->sounded = 1;
                        eneRumbleAll(0xBBBBBBBB, 10);
                        SoundLocation2(0x23E, gGameWork.view, m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.0f, 0.0f,
                                       gGameWork.view->fovy);
                    }
                    if (m->frame >= 13.0f) {
                        m->frame = 13.0f;
                        w->timer = 100;
                        eneMakeEx(o, ENE_DOCHUKI_BAT, w->zone, w->x, 69.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 0);
                        w->state = 53;
                    }
                    break;
                case 53:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    if (w->timer-- <= 0) {
                        w->state = 54;
                    }
                    break;
                case 54:
                    eneTurnToward(o, 0.0f, 0.07853982f);
                    if (eneAnimPlayOnce(o, w->model, 0.2f)) {
                        w->state = 8;
                    }
                    break;
                case 55:
                    m->anim = dochukiBehimosSneezeAnim;
                    w->sub = 340;
                    w->x = 340.0f;
                    w->y = 0.0f;
                    w->z = -97.0f;
                    m->frame = 0.0f;
                    w->sounded = 0;
                    /* 0.10f: its own .rodata constant in the game, not the function's 0.1f (IDO_NOTES 365) */
                    effectPompomCreate(61.0f, 160.0f, -10.0f, 70.0f, 70.0f, 70.0f, 10, 8000, 1, 0.4f, 0.10f);
                    sndPlay(0x168);
                    eneRumbleAll(0xBBBBBBBB, 400);
                    w->state = 56;
                    /* fallthrough */
                case 56:
                    eneAnimPlayOnce(o, w->model, 0.02f);
                    if (dochukiBehimosIsSneezing(o)) {
                        w->state = 57;
                    }
                    break;
                case 57:
                    if (gGameFrameCount & 1) {
                        w->x = w->sub + 1;
                    } else {
                        w->x = w->sub - 1;
                    }
                    w->y -= 1.0f;
                    if (w->y <= -80.0f) {
                        w->y = -80.0f;
                        sndPlay(0x11D);
                        eneRumbleAll(-1, 70);
                        w->timer = 70;
                        fadeSetAll(70.0f, 0.0f, 255.0f, 255, 255, 255);
                        eneMakeEx(o, ENE_BIG_BOMB, 0, m->pos[0], m->pos[1] + 120.0f, m->pos[2], 2,
                                  (u32 (*)(void *, void *, void *)) 5);
                        w->state = 58;
                    }
                    break;
                case 58:
                    if (--w->timer == -50) {
                        w->state = 59;
                    }
                    break;
                case 59:
                    break;
            }
            eneSetModelOnCourse(o, &w->model->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

void dochukiChinkoShow(Obj *o) {
    CHINKO(o)->model->f.flags |= 2;
}

void dochukiChinkoHide(Obj *o) {
    CHINKO(o)->model->f.flags &= ~2;
}

/* "chinko": the Behimos's tail; when hit it takes pips off the gauge, flashes and swells. */
s32 dochukiChinkoProc(Obj *o, s16 msg, void *arg) {
    ChinkoWork *w = CHINKO(o);
    AnimeFrame *m;
    s32 pad[6];
    s32 power;

    switch (msg) {
        case 2:
            o->name = "chinko";
            eneSetup(o, arg);
            w->model = eneNewAnime2(o, (u32) dochukiChinkoState0Anim, (Gfx *) fadeSegToRam(dochukiOpaEnvDl, o->file));
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            eneSetModel(o, &m->f);
            frmMove(&m->f, gFrmLayer2);
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &m->f, 70, 125, -125, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 101;
            w->alpha = 0;
            break;
        case 1:
            if (w->model != NULL) frmDestroy(&w->model->f);
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 3:
            m = w->model;
            m->dl = gGfxDlHead;
            if (w->hurt & 2) {
                gSPDisplayList(gGfxDlHead++, fadeSegToRam(dochukiOpaEnvDl, o->file));
                gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, 128);
            } else {
                gSPDisplayList(gGfxDlHead++, fadeSegToRam(dochukiOpaEnvDl, o->file));
                gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, w->alpha);
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            m = w->model;
            if (w->hurt-- <= 0) {
                w->hurt = 0;
            }
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1 && w->hurt == 0) {
                eneHitSoundHeavy(w->dmg);
                SoundLocation2(0x2E9, gGameWork.view, m->pos[0], m->pos[1], m->pos[2], 0.0f, 0.0f,
                               gGameWork.view->fovy);
                power = eneHitStrength(w->dmg);
                dochukiMainTakeCount(power);
                w->hurt = power * 10;
                w->state = 2;
            }
            switch (w->state) {
                case 0:
                    m->anim = dochukiChinkoState0Anim;
                    m->frame = 0.0f;
                    w->state = 1;
                case 1:
                    eneAnimLoop(o, w->model, 0.5f);
                    break;
                case 2:
                    m->anim = dochukiChinkoState2Anim;
                    m->frame = 0.0f;
                    w->alpha += 8;
                    if (w->alpha >= 128) {
                        w->alpha = 128;
                    }
                    m->scale[0] += 0.005f;
                    if (m->scale[0] >= 0.18) {
                        m->scale[0] = 0.18f;
                    }
                    m->scale[1] = m->scale[2] = m->scale[0];
                    w->state = 3;
                case 3:
                    eneAnimLoop(o, w->model, 0.8f);
                    if (w->hurt == 0) {
                        w->state = 0;
                    }
                    break;
            }
            eneSetModel(o, &w->model->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The game puts "i = 0" (move v0,zero) in the first branch's delay slot, before
 * "w->wait = 2". */
void dochukiGenkiRingAdd(Obj *o) {
    s32 pad;
    GenkiWork *w = GENKI(o);
    s32 i;

    if (w->wait == 0) {
        for (i = 0; i < 4; i++) {
            /* The store in the loop: IDO hoists it out after "i = 0" as the game has it;
             * written before the loop, it lands before "i = 0". */
            w->wait = 2;
            if (w->rings[i] == NULL) {
                w->rings[i] = eneNewPol(o, dochukiGenkiRingDl);
                if (w->rings[i] != NULL) {
                    frmMove(&w->rings[i]->f, gFrmLayer8);
                    eneSetModelOnCourse(o, &w->rings[i]->f);
                    w->rings[i]->flags |= 7;
                    w->rings[i]->scale[0] = w->rings[i]->scale[1] = w->rings[i]->scale[2] = w->size;
                }
                return;
            }
        }
    }
}

void dochukiGenkiRingShrink(Obj *o) {
    GenkiWork *w = GENKI(o);
    s32 i;
    s32 pad[2];

    if (w->wait != 0) {
        w->wait--;
    }
    for (i = 0; i < 5; i++) {
        if (w->rings[i] != NULL) {
            w->rings[i]->scale[0] -= 0.25;
            if (w->rings[i]->scale[0] <= 0.0f) {
                w->rings[i] = (PolFrame *) frmDestroy(&w->rings[i]->f);
                return;
            }
            w->rings[i]->scale[1] = w->rings[i]->scale[2] = w->rings[i]->scale[0];
        }
    }
}

void dochukiGenkiRingClear(Obj *o) {
    GenkiWork *w = GENKI(o);
    s32 i;

    for (i = 0; i < 5; i++) {
        if (w->rings[i] != NULL) {
            w->rings[i] = (PolFrame *) frmDestroy(&w->rings[i]->f);
        }
    }
}

/* "genki", the boss's bouncing fireball: a pol model on layer 8 with an attack box (kind 0x78)
 * and a damage box (kind 0x65). Held (state 0), it pulses from 1.3 to 2.0 at the boss's hand
 * (dochukiGetNodePos); thrown (13 or more), it falls toward the target player's side, bouncing at
 * height 78 between x -120 and 252 (sound 0x453) with a trail of up to five rings, and is gone
 * after 150 frames or below 70; let go (-1), it shrinks away (state 2). It flickers yellow and
 * white every other frame, at 1.1 times its size. Hitting a player, it sends message 7 to the
 * player's task and tells the boss (dochukiOnPlayerHit).
 * Matched 2026-10-17; it was parked for registers: the ring pointer took v0 instead of the
 * game's a3 until the ghost-loop pattern (an array tested directly, the ring copied into r
 * inside the if, as dochukiProc's ghost loop in this file) colours a local copy into v0 first,
 * which pushes the ring itself to a3. Two FAKE marks stay: the pad above m, and message 1's two
 * tests on one line (IDO_NOTES 347). */
s32 dochukiGenkiProc(Obj *o, s16 msg, void *arg);
s32 dochukiGenkiProc(Obj *o, s16 msg, void *arg) {
    GenkiWork *w = GENKI(o);
    s32 pad; /* FAKE: the game has one more word above m */
    PolFrame *m;
    PolFrame *r;
    s32 i;
    f32 d;
    s32 sign;

    switch (msg) {
        case 2:
            o->name = "genki";
            eneSetup(o, arg);
            if ((w->model = eneNewPol(o, dochukiGenkiRingDl)) == NULL) {
                return -1;
            }
            m = w->model;
            frmMove(&m->f, gFrmLayer8);
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &m->f, 7, 0, 0, 0)) == NULL) {
                return -1;
            }
            if (w->atk != NULL) {
                w->atk->kind = 0x78;
            }
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &m->f, 7, 14, -7, 0, 0)) == NULL) {
                return -1;
            }
            w->dmg->kind = 0x65;
            m->flags |= 7;
            w->alpha = 0xFF;
            w->r = 0xFF;
            w->g = 0xFF;
            w->b = 0xFF;
            w->size = 0.0f;
            break;
        case 1:
            /* FAKE: the first test and the next on one line (IDO_NOTES 347) */
            if (w->model != NULL) { frmDestroy(&w->model->f); } if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            dochukiGenkiRingClear(o);
            break;
        case 3:
            m = w->model;
            m->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, w->r, w->g, w->b, w->alpha);
            gDPSetCombineLERP(gGfxDlHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0,
                              TEXEL0, 0, ENVIRONMENT, 0);
            gSPEndDisplayList(gGfxDlHead++);
            for (i = 0; i < 5; i++) {
                if (w->rings[i] != NULL) {
                    r = w->rings[i];
                    r->dl = gGfxDlHead;
                    gDPSetEnvColor(gGfxDlHead++, w->r, w->g, w->b, w->alpha);
                    gDPSetCombineLERP(gGfxDlHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0,
                                      ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
                    gSPEndDisplayList(gGfxDlHead++);
                }
            }
            break;
        case 0:
            m = w->model;
            if (w->atk != NULL) {
                if (acollAtkWasHit(w->atk) == 1) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                    dochukiOnPlayerHit(o->parent);
                }
            }
            switch (w->state) {
                case 0:
                    w->size += 0.1;
                    if (w->size >= 2.0) {
                        w->size = 1.3f;
                    }
                    d = dochukiGetNodePos(o->parent, &w->x, &w->y, &w->z);
                    if (d >= 13.0f) {
                        f32 px;
                        f32 py;
                        f32 pz;
                        s32 n;
                        Obj *pl;

                        n = dochukiGetTarget(o->parent);
                        pl = gGameWork.players[n];
                        plGetModelPos(pl, &px, &py, &pz);
                        if (m->pos[0] <= px) {
                            w->vx = 1.0f;
                        } else {
                            w->vx = -1.0f;
                        }
                        w->vz = 0.0f;
                        w->timer = 150;
                        w->state = 1;
                        w->vy = -3.0f;
                    } else if (d == -1.0f) {
                        w->state = 2;
                    }
                    break;
                case 1:
                    dochukiGenkiRingShrink(o);
                    dochukiGenkiRingAdd(o);
                    eneFall(o, 0.4f);
                    eneMoveObj(o);
                    if (w->x >= -120.0f && w->x <= 252.0f && w->y <= 78.0f) {
                        soundIncamCam(0x453, w->model->pos[0], w->model->pos[1] + 5.0f, w->model->pos[2]);
                        w->y = 78.0f;
                        w->vy = 7.0f;
                        if (w->vx < 0.0f) {
                            sign = -1;
                        } else {
                            sign = 1;
                        }
                        w->vx = sign * 1.5f;
                    }
                    w->timer--;
                    if (w->y <= 70.0f || w->timer <= 0) {
                        return -1;
                    }
                    break;
                case 2:
                    w->size -= 0.2;
                    if (w->size <= 0.0f) {
                        w->size = 0.0f;
                        return -1;
                    }
                    break;
            }
            if (gGameFrameCount & 1) {
                m->scale[1] = m->scale[2] = m->scale[0] = w->size * 1.1;
                w->b = 0xA0;
                w->g = 0xFF;
                w->r = 0xFF;
            } else {
                m->scale[1] = m->scale[2] = m->scale[0] = w->size;
                w->b = 0xFF;
                w->g = 0xFF;
                w->r = 0xFF;
            }
            eneSetModel(o, &m->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "ntama": a ball the Dochuki throws at its target player; sparks when it hits or lands.
 * Was parked: 317 differing lines (345), mix distance 3: the game keeps eneSetup's result v
 * in s0 across case 2's calls (three saved registers, frame 0x78); here v stays in its stack
 * slot (two saved registers, frame 0x70), which moves every register after it. Measured: v as
 * u32 with a cast, as s32, as "register": all 317. Sweep 2026-10-11: message 1's first two
 * tests on one line (IDO_NOTES 347): 317 (the game's frmDestroy address is in s1 there).
 * 2026-10-12 (real 160): the game's three saved registers are v (s0), m (s1) and w (s2);
 * here m (s0) and w (s1), v in its slot. The frmDestroy address in s1 in the game's message 1
 * follows from that: a third saved register is already paid for there. So the one cause is v's
 * priority. A FAKE `if (v) {}` after the call: folded, 160.
 * 2026-10-12 (second try): `if (v) {}` before plGetModelPos: 160; the atk test through an s32 view
 * (iki's lever): 170.
 * 2026-10-12 (third try): in the game message 1's model pointer is s0 (v's register in message 2)
 * and m is s1 elsewhere, so message 1 was tried through v ("v = (s32) w->model; if (v != 0)
 * frmDestroy((Frame *) v)"): 142, length -1, but v is still spilled in message 2 and the frame
 * still 0x70 (reverted: a FAKE that does not reach the cause). v declared first (above pad):
 * 160, no change.
 * 2026-10-12 (fourth try): MATCH. The game's s0 is ONE variable: eneSetup's result in message 2, the
 * model in message 1 and the model read for message 0's four SoundLocation2 calls (this C read
 * those through a v0 temp). Its uses together give it a saved register, so v is no longer
 * spilled (160 to 10); then d.y before d.z (10 to 4) and message 1's "if ((s = w->model) !=
 * NULL)" (4 to MATCH). */
s32 dochukiNtamaProc(Obj *o, s16 msg, void *arg) {
    NtamaWork *w = NTAMA(o);
    /* One variable for eneSetup's result (message 2) and the model (messages 1 and 0's
     * sounds), as the game's s0; split in two, eneSetup's result loses its saved register */
    PolFrame *s;
    PolFrame *m;
    s32 pad;
    f32 px;
    f32 py;
    f32 pz;
    s32 pad2[2];
    Vec3f d;

    switch (msg) {
        case 2:
            o->name = "ntama";
            s = (PolFrame *) eneSetup(o, arg);
            w->model = eneNewPol(o, (Gfx *) dochukiNtamaDl);
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &m->f, 15, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            if (w->atk != NULL) {
                w->atk->kind = 120;
            }
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &m->f, 10, 20, -10, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 101;
            m->flags |= 7;
            plGetModelPos(gGameWork.players[dochukiGetTarget(o->parent)], &px, &py, &pz);
            d.x = px - m->pos[0] + (s32) s;
            d.y = 69.0f - m->pos[1];
            d.z = pz - m->pos[2];
            mathVecNormalizeB(&d);
            mathVecScale(&d, 4.0f);
            w->vel[0] = d.x;
            w->vel[1] = d.y;
            w->vel[2] = d.z;
            break;
        case 1:
            if ((s = w->model) != NULL) {
                frmDestroy(&s->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 0:
            m = w->model;
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                itexSparkleYellow(m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.1f);
                ptclSpawnSparksFall(m->pos[0], m->pos[1] + 5.0f, m->pos[2]);
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                if (dochukiFlipToggle(o->parent)) {
                    s = w->model;
                    SoundLocation2(0x229, gGameWork.view, s->pos[0], s->pos[1] + 5.0f,
                                   s->pos[2], 100.0f, 1000.0f, gGameWork.view->fovy);
                } else {
                    s = w->model;
                    SoundLocation2(0x22A, gGameWork.view, s->pos[0], s->pos[1] + 5.0f,
                                   s->pos[2], 100.0f, 1000.0f, gGameWork.view->fovy);
                }
                dochukiOnPlayerHit(o->parent);
                return -1;
            }
            eneMoveObj(o);
            if (w->y <= 70.0f) {
                eneSetModel(o, &m->f);
                if (dochukiFlipToggle(o->parent)) {
                    s = w->model;
                    SoundLocation2(0x229, gGameWork.view, s->pos[0], s->pos[1] + 5.0f,
                                   s->pos[2], 100.0f, 1000.0f, gGameWork.view->fovy);
                } else {
                    s = w->model;
                    SoundLocation2(0x22A, gGameWork.view, s->pos[0], s->pos[1] + 5.0f,
                                   s->pos[2], 100.0f, 1000.0f, gGameWork.view->fovy);
                }
                itexSparkleYellow(m->pos[0], m->pos[1] + 5.0f, m->pos[2], 0.1f);
                ptclSpawnSparksFall(m->pos[0], m->pos[1] + 5.0f, m->pos[2]);
                return -1;
            }
            eneSetModel(o, &m->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Called by the Dochuki on its kind 0x39 (an "effect"): a rumble, and the rings' size. */
void dochukiEffectSetGrow(Obj *o, f32 grow) {
    eneRumbleAll(0xBBBBBBBB, 100);
    EFFECT(o)->grow = grow;
}

/* "effect": two rings that spin opposite ways and grow (a rumble while it lives). */
s32 dochukiEffectProc(Obj *o, s16 msg, void *arg) {
    EffectWork *w = EFFECT(o);
    s32 pad;
    PolFrame *a;
    PolFrame *b;

    switch (msg) {
        case 2:
            o->name = "effect";
            eneSetup(o, arg);
            w->rings[0] = eneNewPol(o, dochukiEffectRingDl);
            if (w->rings[0] == NULL) {
                return -1;
            }
            a = w->rings[0];
            frmMove(&a->f, gFrmLayer8);
            w->rings[1] = eneNewPol(o, dochukiEffectRingDl);
            if (w->rings[1] == NULL) {
                return -1;
            }
            b = w->rings[1];
            frmMove(&b->f, gFrmLayer8);
            w->grow = 0.0f;
            a->scale[0] = 0.0f;
            a->scale[1] = 0.0f;
            a->scale[2] = 0.0f;
            b->scale[0] = 0.0f;
            b->scale[1] = 0.0f;
            b->scale[2] = 0.0f;
            eneSetModelOnCourse(o, &a->f);
            eneSetModelOnCourse(o, &b->f);
            eneRumbleAll(0x88888888, 150);
            break;
        case 3:
            a = w->rings[0];
            a->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupPrimEnvTexBlendDl);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, 128);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 128, 128, 255, 128);
            gSPEndDisplayList(gGfxDlHead++);
            b = w->rings[1];
            b->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupPrimEnvTexBlendDl);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 255, 128);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 128, 128, 255, 128);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->rings[0] != NULL) {
                frmDestroy(&w->rings[0]->f);
            }
            if (w->rings[1] != NULL) {
                frmDestroy(&w->rings[1]->f);
            }
            eneRumbleStopAll();
            break;
        case 0: {
            s32 pad2;

            if (w->grow <= 0.3) {
                w->grow += 0.01;
            }
            a = w->rings[0];
            a->rot[1] += 0.15707964f;
            b = w->rings[1];
            b->rot[1] -= 0.12566371f;
            a->scale[0] = 0.6f;
            a->scale[2] = 0.6f;
            a->scale[1] = w->grow * 0.8;
            b->scale[0] = 0.8f;
            b->scale[2] = 0.8f;
            b->scale[1] = w->grow * 0.6;
            break;
        }
    }
    return tskDefaultProc(o, msg, arg);
}

/* "beam": plays its animation once; near its end it makes kind 0x3B under its parent. */
s32 dochukiBeamProc(Obj *o, s16 msg, void *arg) {
    s32 pad;
    BeamWork *w = BEAM(o);
    AnimeFrame *m;

    switch (msg) {
        case 2:
            o->name = "beam";
            w->variant = eneSetup(o, arg);
            if (w->variant == 0) {
                w->model = eneNewAnime(o, (u32) dochukiBeamAnim0);
            } else {
                w->model = eneNewAnime(o, (u32) dochukiBeamAnim1);
            }
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            frmMove(&m->f, gFrmLayer8);
            m->scale[0] = 0.5f;
            m->scale[1] = 0.5f;
            m->scale[2] = 0.5f;
            w->fired = 0;
            eneSetModelOnCourse(o, &m->f);
            break;
        case 3:
            m = w->model;
            m->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(beamDl, o->file));
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 128, 128);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0: {
            f32 end;

            m = w->model;
            if (eneAnimLoop(o, m, 0.5f)) {
                return -1;
            }
            end = polAnimeLastFrame(m) - 2.0f;
            if (end <= m->frame) {
                if (w->fired == 0) {
                    w->fired = 1;
                    eneMakeEx(o->parent, ENE_DOCHUKI_CORE, w->zone, w->x, 69.0f, 0.0f, 2,
                              (u32 (*)(void *, void *, void *)) w->variant);
                }
            }
            eneSetModelOnCourse(o, &m->f);
            break;
        }
    }
    return tskDefaultProc(o, msg, arg);
}

void dochukiItexBomb(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x11F;
    init.frames = gItexBombAnim;
    init.flags = 3;
    init.prim.r = 0xFF; init.prim.g = 0xFF; init.prim.b = 0x80;
    init.env.r = 0xFF; init.env.g = 0x28; init.env.b = 0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    init.fade = 0;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    tskCreate(gEffectTask, itex, 0, &init);
}

/* "CORE": flies out sideways (or stays, variant 10), throwing sparks, explosions and kind 0x3B
 * every other frame, for 15 frames. */
s32 dochukiCoreProc(Obj *o, s16 msg, void *arg) {
    CoreWork *w = CORE(o);
    s32 pad;
    u32 v;
    Vec3f p;

    switch (msg) {
        case 2:
            o->name = "CORE";
            v = eneSetup(o, arg);
            w->model = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->model == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 8, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 120;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->model->f, 10, 20, -10, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 101;
            w->variant = v;
            if (v == 10) {
                w->vel[0] = 0.0f;
            } else if (v == 0) {
                w->x -= 60.0f;
                w->vel[0] = 9.0f;
            } else {
                w->x += 60.0f;
                w->vel[0] = -9.0f;
            }
            w->life = 15;
            eneSetModelOnCourse(o, &w->model->f);
            eneRumbleAll(-1, 100);
            break;
        case 1:
            if (w->model != NULL) frmDestroy(&w->model->f);
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            eneRumbleStopAll();
            break;
        case 0:
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            if (w->variant != 10 && !(w->life & 1)) {
                vramCoursePointXZ(w->zone, w->x, w->z, &p.x, &p.z);
                dochukiItexBomb(p.x, w->y + 5.0f, p.z, 0.15f);
                ptclSpawnSparksFall(p.x, w->y + 5.0f, p.z);
                eneMakeEx(o->parent, ENE_DOCHUKI_CORE, w->zone, p.x, w->y, p.z, 2, (u32 (*)(void *, void *, void *)) 10);
                SoundLocation2(0x11B, gGameWork.view, p.x, w->y, p.z, 0.0f, 0.0f, gGameWork.view->fovy);
            }
            if (w->life-- <= 0) {
                return -1;
            }
            eneMoveObj(o);
            eneSetModelOnCourse(o, &w->model->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "iki": a breath puff from the Behimos's mouth (its parent), flying toward the front.
 * Matched 2026-10-15 (the last entry below). History, when it was NON_MATCHING:
 * (real 20 from case 3 on: the 2026-10-15 pass below). First draft: 126 differing lines
 * (305), mix distance 0: frame and code are the game's; from
 * the box constants (-10 and 120) in case 2 on, every t-register is one behind the game's (the
 * game spends one more temporary before them). Measured: the scale as a chain 150 (one compiler
 * temp word too many); "(s16) -10": no change.
 * 2026-10-12 (real 68): v as s32 68; the scale chain again: frame 0x78 and the t-registers
 * still one behind (98); `results` (each call's result kept, each callee as int): 68 or worse.
 * Lines 0-68 are the game's instruction for instruction, so the game's extra temporary is
 * one that emits nothing.
 * 2026-10-12 (second try, real 68 to 40): the atk test read through an s32 view (IDO_NOTES 274: ugen
 * reloads the stored pointer, as1 forwards and deletes the reload, its t-register stays spent)
 * puts every t-register through instruction 112 on the game's. The two debug strings in this
 * function's .rodata ("!!!!! IKI ..." and "L->work.x = %f ...") are now PRINTFs (their places
 * are guesses; the build must check the .rodata). Left: from v's load (113) on, the game has
 * spent one more t-register between o's load for dochukiBehimosGetMouthPos (t8) and v (t2, here t1). The
 * same view on the dmg test spends it too early (first difference 85, 28 lines). Measured,
 * no change (40): the L->work print as an empty-macro comma expression; floadGetSetFile's
 * result read into pad first; "*(s32 *) &v != 0"; `results` (every call both ways).
 * 2026-10-12 (third try, 40, no change): v as s32 again (with the atk view); the L->work PRINTF
 * left out (so the prints cost nothing, either place).
 * 2026-10-12 (fourth try, 40, no change): o->parent read into a local L (in pad's word; the print
 * names "L->work") before dochukiBehimosGetMouthPos; match_function.py reloads: nothing better.
 * 2026-10-12 (fifth try, 40, both taken back): floadGetSetFile through a (u32 (*)(s32, s32))
 * cast (IDO_NOTES 375): no change; the atk call through an s32-returning cast with the plain
 * "w->atk == NULL" test in place of the FAKE: 68 (uopt still forwards the store); a
 * diagnostic goto label before dochukiBehimosGetMouthPos's call: 40 (v still t1).
 * 2026-10-12 (sixth try, 40): the permuter (~15 min): no output
 * better than the base. Read from the objects: from o's t8 the game's next temporary is t2 and
 * this C's t1, so the game holds one more temporary invisible across that point.
 * 2026-10-12 (seventh try, 40): match_function.py splits (144 places, one boundary and eight):
 * nothing under 94 lines.
 * 2026-10-12 (eighth try, trace, 40): no uopt range differs; in ugen's listing the o reload for
 * dochukiBehimosGetMouthPos takes $24 and v's load $9, so ugen's queue passes $25 and $8 between them
 * (none of t0-t9 is a uopt range in case 2). The game's v is $10: ugen handed out one more
 * temporary there, as in spirit (tlb_0040), whose model test needed the same FAKE. The two
 * spent temporaries sit where this function's two debug strings are; "(void) "L->work..";"
 * in the print's place: 40, no change. Taken back.
 * 2026-10-12 (ninth try, 40): not a uopt register choice (trace: no uopt range differs), so uopt's
 * globalcolor (read for dochuki above) does not reach it; the difference is ugen's temporary
 * queue. No try made.
 * 2026-10-15 (40 to 20), ugen's queue read in ugen.c: get_one_free_reg (80781) takes the
 * head of the free list (0x100213d4) and appends the register to the used list; free_reg
 * (81664) returns it to the TAIL of the free list once its usage count reaches 0. So the
 * t-registers are a FIFO of frees. From the listing the queue here is t6 t7 t8 t1..t5 (t9 is
 * la's, t0 never handed out), and replaying it predicts every register of this C. Case 2: a
 * never-true "(u32) w + 1 < 0" break (IDO_NOTES 284) after dochukiBehimosGetMouthPos's call spends t1:
 * case 2 is then the game's to the last register (40 to 20). Left: case 3 starts with the queue
 * t6 t7 t8 t1 here; the game's model load takes t1 second (queue t6 t1 t7 t8 t2..t5), and t1
 * is never handed out again in case 3. Replayed, the game's case 2 needs t1 taken before
 * 120's li and given back between the flags statement's lhu (t6) and its sh (t7): held across
 * five statements, which no ugen temporary is; a uopt range in t1 ($9, uopt's r8) over those
 * blocks would remove it from ugen's list and return it when the range ends. Case 3's t1 after
 * the model load fits the same (a uopt range in t1 from there to the end of case 3, its code
 * deleted by as1). Tried: "m = w->model; m->dl = .." in case 3 (58: m's range moves).
 * MATCHED 2026-10-15 (docs/ido-tools/ugen-trace):
 * - The trace shows gen_regs taking v0, v1 and t0 off ugen's free list for the whole function:
 *   t0 is uopt's (case 3's fifth gfx pointer, r7). The list starts [t6 t7 t8 t9 t1 t2 t3 t4 t5].
 * - The game never hands t1 to a ugen temporary anywhere. Its one t1 is case 3's model load, so
 *   t1 is uopt's too (r8, the next free register after the five gfx pointers' a0-t0 in that
 *   block), and every "spent" t-register in case 2 was t1 simply missing from the list.
 * - So case 3 reads the model through m ("m = w->model; m->dl = gGfxDlHead;"): m is a uopt range
 *   in t1. Both FAKEs (the atk test's s32 view and the "(u32) w + 1 < 0" break) go: MATCH.
 *   The 2026-10-15 pass's 58 for this m was measured with those FAKEs still in, which spent registers twice. */
s32 dochukiIkiProc(Obj *o, s16 msg, void *arg) {
    IkiWork *w = IKI(o);
    s32 pad;
    PolFrame *m;
    Vec3f p;
    u32 v;

    switch (msg) {
        case 2:
            o->name = "iki";
            v = eneSetup(o, arg);
            w->model = eneNewPol(o, dochukiIkiDl);
            if (w->model == NULL) {
                return -1;
            }
            m = w->model;
            frmMove(&m->f, gFrmLayer8);
            m->scale[2] = 0.5f;
            m->scale[1] = 0.5f;
            m->scale[0] = 0.5f;
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &m->f, 5, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            if (w->atk != NULL) {
                w->atk->kind = 120;
            }
            PRINTF("!!!!! IKI IKI IKI IKI IKI !!!!!!!\n");
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &m->f, 10, 20, -10, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 101;
            w->model->flags |= 7;
            fadeAnimStart(&w->anim, gItexPuffAnim, 0, &w->model->segs[3], floadGetSetFile(0x123, 0));
            dochukiBehimosGetMouthPos(o->parent, &w->x, &w->y, &w->z);
            PRINTF("L->work.x = %f, L->work.y = %f, L->work.z = %f\n", w->x, w->y, w->z);
            if (v) {
                w->x += 10.0f;
            } else {
                w->x -= 10.0f;
            }
            dochukiBehimosGetPos(o->parent, &p.x, &p.y, &p.z);
            w->vel[0] = (w->x - p.x) * 0.05;
            w->vel[1] = (74.0f - w->y) / 20.0f;
            w->vel[2] = (97.0f - w->z) / 20.0f;
            eneSetModel(o, &m->f);
            w->life = 60;
            w->alpha = 128;
            break;
        case 3:
            m = w->model;
            m->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombine(gGfxDlHead++, 0x30D261, 0x55A6FF7F);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 0);
            gDPSetEnvColor(gGfxDlHead++, 255, 255, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 0:
            m = w->model;
            eneMoveObj(o);
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            if (w->dmg != NULL) {
                acollDmgWasHit(w->dmg);
            }
            if (w->y <= 74.0f) {
                eneRumbleAll(0xBBBBBBBB, 4);
                w->y = 74.0f;
                w->vel[1] = 0.0f;
            }
            if (w->life-- <= 0) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            eneSetModel(o, &m->f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "bat"
 * Matched 2026-10-17; it was parked for registers: the game rematerialises the work pointer
 * from o (s1) in each case, but o's own uses here are too few for uopt to give it a saved
 * register; one more use (the FAKE empty "if (o) {}" after the switch) gives o s1, as the
 * matched dochukiHanaProc's same shape does naturally (it reads o->parent five times). */
s32 dochukiBatProc(Obj *o, s16 msg, void *arg) {
    BatWork *w = BAT(o);

    switch (msg) {
        case 2:
            o->name = "bat";
            eneSetup(o, arg);
            w->model = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->model == NULL) {
                return -1;
            }
            w->x -= 8.0f;
            w->y = 70.0f;
            w->z = 0.0f;
            eneSetModelOnCourse(o, &w->model->f);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 8, 0, 0, 0);
            if (w->atk != NULL) {
                w->atk->kind = 100;
            }
            w->life = 100;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            break;
        case 0:
            eneSetModelOnCourse(o, &w->model->f);
            if (acollAtkWasHit(w->atk) == 1) {
                dochukiMainStartWalk();
                return -1;
            }
            if (w->life-- <= 0) {
                return -1;
            }
            break;
    }
    if (o) {} /* FAKE: one more use of o gives it a saved register (s1), as the game */
    return tskDefaultProc(o, msg, arg);
}

/* "HANA": the Behimos's nose, its weak spot; a hit there costs it (dochukiBehimosTakeHit). Puffs of
 * smoke when it appears; it goes when its parent's sneeze frames end. */
s32 dochukiHanaProc(Obj *o, s16 msg, void *arg) {
    HanaWork *w = HANA(o);
    PolFrame *m;

    switch (msg) {
        case 2:
            o->name = "HANA";
            eneSetup(o, arg);
            w->model = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->model == NULL) {
                return -1;
            }
            w->y = 70.0f;
            w->z = 0.0f;
            eneSetModelOnCourse(o, &w->model->f);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 18, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            if (w->atk != NULL) {
                w->atk->kind = 120;
            }
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->model->f, 20, 20, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            if (w->dmg != NULL) {
                w->dmg->kind = 101;
            }
            w->life = 100;
            m = w->model;
            dochukiPuffYellow(o->parent, m->pos[0], m->pos[1], m->pos[2], 2.0f, 0.0f, 0, 0.1f);
            m = w->model;
            dochukiPuffYellow(o->parent, m->pos[0], m->pos[1], m->pos[2], 1.7f, 0, 1.0f, 0.1f);
            m = w->model;
            dochukiPuffYellow(o->parent, m->pos[0], m->pos[1], m->pos[2], 1.0f, 0, 1.7f, 0.1f);
            m = w->model;
            dochukiPuffYellow(o->parent, m->pos[0], m->pos[1], m->pos[2], -1.0f, 0, 1.7f, 0.1f);
            m = w->model;
            dochukiPuffYellow(o->parent, m->pos[0], m->pos[1], m->pos[2], -1.7f, 0, 1.0f, 0.1f);
            m = w->model;
            dochukiPuffYellow(o->parent, m->pos[0], m->pos[1], m->pos[2], -2.0f, 0.0f, 0, 0.1f);
            eneRumbleAll(-1, 100);
            break;
        case 1:
            m = w->model;
            if (m != NULL) {
                frmDestroy(&m->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            eneRumbleStopAll();
            break;
        case 0:
            eneSetModelOnCourse(o, &w->model->f);
            if (w->atk != NULL && acollAtkWasHit(w->atk) == 1) {
                tskSendMsg((Obj *) w->atk->hitPower, 7, o);
            }
            if (w->dmg != NULL && acollDmgWasHit(w->dmg) == 1) {
                dochukiBehimosTakeHit(o->parent);
                eneHitSoundHeavy(w->dmg);
                return -1;
            }
            if (!dochukiBehimosIsSneezing(o->parent)) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
