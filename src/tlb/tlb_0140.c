#include "tlb.h"

/* File 140: the hikyaku bentou (courier's lunch) mission. "okami", the innkeeper who sends the
 * players after the courier and gives a pass (tegata) for it; "d_hikyaku", the courier, who runs
 * one of seven routes against the clock; "jizou", a talking jizou on the road. It begins with the
 * same helpers as file 127 (the Beat Mania mission). */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The innkeeper's work ("okami"), at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct OkamiW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ Camera *cam;
    /* 0x4C */ Obj *pass; /* the pass (kind 0x1C3) */
    /* 0x50 */ s32 timer; /* ticks in the state */
    /* 0x54 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x55 */ u8 tuneTimer;
    /* 0x58 */ Anim blink; /* its eyes' texture animation */
} OkamiW;

#define OKAMI(obj) ((OkamiW *) FX(obj))

/* The courier's work ("d_hikyaku"), at its task + 0x20: a person's (PersonObj) up to 0x48, his
 * route and the mission's counter. */
typedef struct HikyakuW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ u8 pad38[0xC];
    /* 0x44 */ u32 *anims; /* 0: standing, 1: running */
    /* 0x48 */ u16 *path; /* the route command he is at */
    /* 0x4C */ s32 timer; /* ticks in the state */
    /* 0x50 */ MPolFrame *shadow;
    /* 0x54 */ ACollFrame *dmg; /* the box the players stop him with */
    /* 0x58 */ u16 *route; /* the route's first command (one of seven, at random) */
    /* 0x5C */ Obj *counter; /* the goal time's display (kind 0x1C2) */
    /* 0x60 */ s32 daySeconds; /* gGameWork.daySeconds, kept while the mission runs */
    /* 0x64 */ Camera *cam;
} HikyakuW;

#define HIKYAKU(obj) ((HikyakuW *) FX(obj))

/* The road jizou's work ("jizou"), at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct JizouW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ s32 timer; /* ticks alive */
} JizouW;

#define JIZOU(obj) ((JizouW *) FX(obj))

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

/* The pass's work (kind 0x1C3, another file's), as far as this file sets it. */
typedef struct PassW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *pol;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *glow;
    /* 0x38 */ u8 rgb[3];
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ s16 alpha;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ s32 timer;
    /* 0x44 */ Obj *player; /* the one it goes to */
} PassW;

void hikyakuFadeOut(f32 speed);
void hikyakuPassToPlayer(Obj *o, Obj *player);

/* acollDmgAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);

extern u8 dtskHikyakuAnim0[];
AnimFrame hikyakuOkamiEyeAnim[] = {
    { 0x04002100, 15 },
    { 0x04002520, 15 },
    { 0, 0 },
}; /* the innkeeper's eyes: two textures, 15 ticks each */
u32 dtskHikyakuAnims[] = { (u32) dtskHikyakuAnim0, 0x03000AFC }; /* the courier's animations: standing (0x03000580), running */
u16 hikyakuRoute1[] = { 0x0000, 0x0658, 0x0146, 0x0014, 0x0000, 0x05A8, 0x0322, 0x0014, 0x0000, 0x0517, 0x0409, 0x0014, 0x0000, 0x03DB, 0x0521, 0x0014, 0x0000, 0x01F3, 0x0629, 0x0014, 0x0000, 0x0000, 0x067C, 0x0014, 0x0000, 0x0000, 0x0571, 0x0014, 0x0000, 0x0000, 0x041A, 0x0014, 0x0000, 0x0000, 0x02B4, 0x0014, 0x0000, 0x00A2, 0x028E, 0x0014, 0x0000, 0x0147, 0x024F, 0x0014, 0x0000, 0x01E1, 0x01C8, 0x0014, 0x0000, 0x0213, 0x01A8, 0x0014, 0x0000, 0x0296, 0x00A6, 0x0014, 0x0000, 0x0213, 0x01A8, 0x0014, 0x0000, 0x01E1, 0x01C8, 0x0014, 0x0000, 0x0147, 0x024F, 0x0014, 0x0000, 0x00A2, 0x028E, 0x0014, 0x0000, 0x0000, 0x02B4, 0x0014, 0x0000, 0x0000, 0x041A, 0x0014, 0x0000, 0x0000, 0x0571, 0x0014, 0x0000, 0x0000, 0x067C, 0x0014, 0x0000, 0x01F3, 0x0629, 0x0014, 0x0000, 0x03DB, 0x0521, 0x0014, 0x0000, 0x0517, 0x0409, 0x0014, 0x0000, 0x05A8, 0x0322, 0x0014, 0x0000, 0x0658, 0x0146, 0x0014, 0x0000, 0x0680, 0x002B, 0x0014, 0x0001, 0x0000 }; /* the courier's seven routes (person.c's path commands) */
u16 hikyakuRoute2[] = { 0x0000, 0xFE28, 0x0637, 0x0014, 0x0000, 0xFC12, 0x052F, 0x0014, 0x0000, 0xFA26, 0x02E1, 0x0014, 0x0000, 0xF9BC, 0x0170, 0x0014, 0x0000, 0xF998, 0x00A0, 0x0014, 0x0000, 0xFA92, 0x007C, 0x0014, 0x0000, 0xF998, 0x00A0, 0x0014, 0x0000, 0xF9BC, 0x0170, 0x0014, 0x0000, 0xFA26, 0x02E1, 0x0014, 0x0000, 0xFC12, 0x052F, 0x0014, 0x0000, 0xFE28, 0x0637, 0x0014, 0x0000, 0x0000, 0x067C, 0x0014, 0x0000, 0x0000, 0x0571, 0x0014, 0x0000, 0xFEF7, 0x055F, 0x0014, 0x0000, 0xFD57, 0x04C6, 0x0014, 0x0000, 0xFBE3, 0x0374, 0x0014, 0x0000, 0xFC4E, 0x0312, 0x0014, 0x0000, 0xFBE3, 0x0374, 0x0014, 0x0000, 0xFCAD, 0x044B, 0x0014, 0x0000, 0xFD57, 0x04C6, 0x0014, 0x0000, 0xFEF7, 0x055F, 0x0014, 0x0000, 0x0000, 0x0571, 0x0014, 0x0000, 0x0000, 0x067C, 0x0014, 0x0001, 0x0000 };
u16 hikyakuRoute3[] = { 0x0000, 0x0213, 0x01A8, 0x0014, 0x0000, 0x01E1, 0x01C8, 0x0014, 0x0000, 0x0147, 0x024F, 0x0014, 0x0000, 0x00A2, 0x028E, 0x0014, 0x0000, 0x0000, 0x02B4, 0x0014, 0x0000, 0x0000, 0x041A, 0x0014, 0x0000, 0x00F6, 0x03FC, 0x0014, 0x0000, 0x0158, 0x03C3, 0x0014, 0x0000, 0x01F5, 0x0385, 0x0014, 0x0000, 0x026B, 0x039A, 0x0014, 0x0000, 0x034E, 0x036D, 0x0014, 0x0000, 0x0397, 0x0326, 0x0014, 0x0000, 0x047D, 0x01A2, 0x0014, 0x0000, 0x04B7, 0x0072, 0x0014, 0x0000, 0x065F, 0x00A0, 0x0014, 0x0000, 0x0680, 0x002B, 0x0014, 0x0000, 0x065F, 0x00A0, 0x0014, 0x0000, 0x04B7, 0x0072, 0x0014, 0x0000, 0x047D, 0x01A2, 0x0014, 0x0000, 0x0397, 0x0326, 0x0014, 0x0000, 0x034E, 0x036D, 0x0014, 0x0000, 0x026B, 0x039A, 0x0014, 0x0000, 0x01F5, 0x0385, 0x0014, 0x0000, 0x0158, 0x03C3, 0x0014, 0x0000, 0x00F6, 0x03FC, 0x0014, 0x0000, 0x0000, 0x041A, 0x0014, 0x0000, 0x0000, 0x02B4, 0x0014, 0x0000, 0x00A2, 0x028E, 0x0014, 0x0000, 0x0147, 0x024F, 0x0014, 0x0000, 0x01E1, 0x01C8, 0x0014, 0x0000, 0x0213, 0x01A8, 0x0014, 0x0000, 0x0296, 0x00A6, 0x0014, 0x0001, 0x0000 };
u16 hikyakuRoute4[] = { 0x0000, 0xF998, 0x00A0, 0x0014, 0x0000, 0xF9BC, 0x0170, 0x0014, 0x0000, 0xFA26, 0x02E1, 0x0014, 0x0000, 0xFC12, 0x052F, 0x0014, 0x0000, 0xFE28, 0x0637, 0x0014, 0x0000, 0x0000, 0x067C, 0x0014, 0x0000, 0xFE28, 0x0637, 0x0014, 0x0000, 0xFC12, 0x052F, 0x0014, 0x0000, 0xFA26, 0x02E1, 0x0014, 0x0000, 0xF9BC, 0x0170, 0x0014, 0x0000, 0xF998, 0x00A0, 0x0014, 0x0000, 0xFA92, 0x007C, 0x0014, 0x0001, 0x0000 };
u16 hikyakuRoute5[] = { 0x0000, 0x0213, 0x01A8, 0x0014, 0x0000, 0x01E1, 0x01C8, 0x0014, 0x0000, 0x0147, 0x024F, 0x0014, 0x0000, 0x00A2, 0x028E, 0x0014, 0x0000, 0x0000, 0x02B4, 0x0014, 0x0000, 0x00A2, 0x028E, 0x0014, 0x0000, 0x0147, 0x024F, 0x0014, 0x0000, 0x01E1, 0x01C8, 0x0014, 0x0000, 0x0213, 0x01A8, 0x0014, 0x0000, 0x0296, 0x00A6, 0x0014, 0x0001, 0x0000 };
u16 hikyakuRoute6[] = { 0x0000, 0xFBE3, 0x0374, 0x0014, 0x0000, 0xFCAD, 0x044B, 0x0014, 0x0000, 0xFD57, 0x04C6, 0x0014, 0x0000, 0xFEF7, 0x055F, 0x0014, 0x0000, 0x0000, 0x0571, 0x0014, 0x0000, 0x0000, 0x067C, 0x0014, 0x0000, 0xFE28, 0x0637, 0x0014, 0x0000, 0xFC12, 0x052F, 0x0014, 0x0000, 0xFA26, 0x02E1, 0x0014, 0x0000, 0xF9BC, 0x0170, 0x0014, 0x0000, 0xF998, 0x00A0, 0x0014, 0x0000, 0xFA92, 0x007C, 0x0014, 0x0000, 0xF998, 0x00A0, 0x0014, 0x0000, 0xF9BC, 0x0170, 0x0014, 0x0000, 0xFA26, 0x02E1, 0x0014, 0x0000, 0xFC12, 0x052F, 0x0014, 0x0000, 0xFE28, 0x0637, 0x0014, 0x0000, 0x0000, 0x067C, 0x0014, 0x0000, 0x01F3, 0x0629, 0x0014, 0x0000, 0x03DB, 0x0521, 0x0014, 0x0000, 0x0517, 0x0409, 0x0014, 0x0000, 0x05A8, 0x0322, 0x0014, 0x0000, 0x0658, 0x0146, 0x0014, 0x0000, 0x0680, 0x002B, 0x0014, 0x0000, 0x065F, 0x00A0, 0x0014, 0x0000, 0x04B7, 0x0072, 0x0014, 0x0000, 0x047D, 0x01A2, 0x0014, 0x0000, 0x0397, 0x0326, 0x0014, 0x0000, 0x034E, 0x036D, 0x0014, 0x0000, 0x026B, 0x039A, 0x0014, 0x0000, 0x01F5, 0x0385, 0x0014, 0x0000, 0x0158, 0x03C3, 0x0014, 0x0000, 0x00F6, 0x03FC, 0x0014, 0x0000, 0x0000, 0x041A, 0x0014, 0x0000, 0x0000, 0x02B4, 0x0014, 0x0000, 0x00A2, 0x028E, 0x0014, 0x0000, 0x0147, 0x024F, 0x0014, 0x0000, 0x01E1, 0x01C8, 0x0014, 0x0000, 0x0213, 0x01A8, 0x0014, 0x0000, 0x0296, 0x00A6, 0x0014, 0x0000, 0x0213, 0x01A8, 0x0014, 0x0000, 0x01E1, 0x01C8, 0x0014, 0x0000, 0x0147, 0x024F, 0x0014, 0x0000, 0x00A2, 0x028E, 0x0014, 0x0000, 0x0000, 0x02B4, 0x0014, 0x0000, 0x0000, 0x041A, 0x0014, 0x0000, 0x0000, 0x0571, 0x0014, 0x0000, 0xFEF7, 0x055F, 0x0014, 0x0000, 0xFD57, 0x04C6, 0x0014, 0x0000, 0xFBE3, 0x0374, 0x0014, 0x0000, 0xFC4E, 0x0312, 0x0014, 0x0001, 0x0000, 0x0000, 0xFE92, 0x0257, 0x0012, 0x0000, 0xFF7A, 0x02AB, 0x0012, 0x0000, 0x0002, 0x02BC, 0x0012, 0x0000, 0xFFF4, 0x0677, 0x0012, 0x0000, 0x00E2, 0x0677, 0x0012, 0x0000, 0x02B0, 0x05DB, 0x0012, 0x0000, 0x046F, 0x04AB, 0x0012, 0x0000, 0x05B0, 0x032F, 0x0012, 0x0000, 0x0677, 0x009C, 0x0012, 0x0006, 0xFFFF, 0x0000, 0x05B0, 0x032F, 0x0012, 0x0000, 0x046F, 0x04AB, 0x0012, 0x0000, 0x02B0, 0x05DB, 0x0012, 0x0000, 0x00E2, 0x0677, 0x0012, 0x0000, 0xFFF4, 0x0677, 0x0012, 0x0000, 0xF982, 0x009C, 0x0012, 0x0000, 0xFA9D, 0x0079, 0x0012, 0x0006, 0xFFFF, 0x0000, 0x049E, 0x006C, 0x0012, 0x0000, 0x03DA, 0x02DB, 0x0012, 0x0000, 0x02AD, 0x03B7, 0x0012, 0x0000, 0x01E4, 0x038A, 0x0012, 0x0000, 0x00BE, 0x03EC, 0x0012, 0x0000, 0x0000, 0x040C, 0x0012, 0x0000, 0x0002, 0x02A5, 0x0012, 0x0000, 0x01E4, 0x01C9, 0x0012, 0x0000, 0x025F, 0x0149, 0x0012, 0x0000, 0x029F, 0x0091, 0x0012, 0x0006, 0xFFFF, 0x0000, 0xF982, 0x009C, 0x0012, 0x0000, 0xFFF4, 0x0677, 0x0012, 0x0000, 0xFFF7, 0x0446, 0x0012, 0x0000, 0xFBE5, 0x0374, 0x0012, 0x0000, 0xFC5E, 0x0301, 0x0012, 0x0006, 0xFFFF, 0x0000, 0xFE92, 0x0257, 0x0012, 0x0000, 0xFF7A, 0x02AB, 0x0012, 0x0000, 0x0002, 0x02BC, 0x0012, 0x0000, 0xFFF4, 0x0677, 0x0012, 0x0000, 0x00E2, 0x0677, 0x0012, 0x0000, 0x02B0, 0x05DB, 0x0012, 0x0000, 0x046F, 0x04AB, 0x0012, 0x0000, 0x05B0, 0x032F, 0x0012, 0x0000, 0x0677, 0x009C, 0x0012, 0x0006, 0xFFFF, 0x0000, 0x0000 };
extern u8 dtskOkamiAnim[]; /* the innkeeper's animation */
extern u8 hikyakuJizouAnim[]; /* the jizou's animation */

/* The screen fades to black at the given speed. The first call is on the brace's line: IDO
 * schedules by line, and only there does "sw ra" come after the call's address (file 127's
 * beatFadeOut, measured). */
void hikyakuFadeOut(f32 speed) { fadeSetSpeed(speed);
    fadeSetColor(0, 0, 0);
    fadeSetTarget(255.0f);
}

/* The pass goes to the player (state 3). */
void hikyakuPassToPlayer(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

/* The pass's sound ("eventTegataSdSet") and the clear's, emptied; their prints' strings remain
 * (file 127's beatEventTegataSdSet and beatClearSdSet). */
void hikyakuEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void hikyakuEventClearSd(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "okami": the innkeeper. She turns to the nearest player; talked to, she turns to the player and
 * talks (scenario 0x29D); if the player takes the errand (0x94) the screen fades and the game goes
 * to stage 0x6E; back from it (0x92), she gives the pass between them (kind 0x1C3, sparks and
 * kind 0x1C1), the pass goes to the player (scenario 0x20), and the sky's tunes change. */
s32 DtskOkami_proc(Obj *o, s16 msg, void *arg) {
    OkamiW *w = OKAMI(o);
    u32 data;
    CamView *view;
    Obj *pl;
    Vec3f pos;
    s32 i;
    Vec3f at;
    s32 pad[2];
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "okami";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) dtskOkamiAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("DtskOkami_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->blink, hikyakuOkamiEyeAnim, 0, &w->model->segs[3], eneGetFile(o, 1));
            if (swWorkTest(0x92)) {
                swWorkReset(0x92);
                gGameWork.scenarioRunning = 2;
                w->state = 5;
            }
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i) && !(gGameWork.players[i]->flags & 2)) {
                            tskSendMsg(gGameWork.players[i], 4, &pos);
                            eneTurnToPoint(o, pos.x, pos.z, 10.0f);
                            break;
                        }
                    }
                    if (demoTalkEndCheck(o)) {
                        demoCamZoom_CL(w->talker);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 18.0f) == 1) {
                            w->state = 2;
                        }
                    }
                    break;
                case 2:
                    scRunStart(0x29D, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 3;
                    break;
                case 3:
                    fadeAnimStep(&w->blink);
                    if (!scIsRunning()) {
                        if (swWorkTest(0x94)) {
                            swWorkReset(0x94);
                            gGameWork.scenarioRunning = 2;
                            w->state = 4;
                            w->timer = 0;
                        } else {
                            personTalkCamOff();
                            demoFinish(o);
                            w->state = 0;
                        }
                    }
                    break;
                case 4:
                    if (w->timer == 1) {
                        sndPlay(10);
                        hikyakuFadeOut(16 / gRetracesPerFrame);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        camAiSetHold(0);
                        stageGotoSpawn(gStageTask->parent, 0x6E, 0, 186.0f, 0.0f, 1000.0f, 0.0f, 0);
                    }
                    break;
                case 5:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (!swWorkTest(0x2A)) {
                            camSunClearTunes();
                            camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
                        }
                        personCamZoomUp(plCheck());
                        scRunStart(0x29D, 0, 0);
                        w->state = 6;
                    }
                    break;
                case 6:
                    fadeAnimStep(&w->blink);
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            w->state = 7;
                            swWorkReset(0x93);
                        } else {
                            demoPlReleaseAll();
                            w->state = 11;
                        }
                    }
                    break;
                case 7:
                    pl = plCheck();
                    if (pl == NULL) {
                        return -1;
                    }
                    plGetModelPos(pl, &x, &y, &z);
                    w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                        (w->model->pos[2] + z) / 2, 0, NULL);
                    w->timer = 0;
                    w->state = 8;
                    break;
                case 8:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        hikyakuPassToPlayer(w->pass, plCheck());
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        scRunStart(0x29D, 0, 0);
                        w->state = 11;
                    }
                    break;
                case 11:
                    fadeAnimStep(&w->blink);
                    if (!scIsRunning()) {
                        personTalkCamOff();
                        demoPlReleaseAll();
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
                }
            }
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.3f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "d_hikyaku": the courier, there from the mission's start (flag 0x29) until the pass is given
 * (0xB6). He runs one of seven routes, chosen at random, with the goal time's display (kind 0x1C2:
 * an hour on from now) and a 2400-second day. A player who hits him stops him: he turns to that
 * player and talks (scenario 0x29F, then 0x21); at the route's end, or when the counter's time is
 * up, he talks (0x263). Then the screen fades, the day's length comes back, flag 0x92 is set and
 * the game goes back to the innkeeper (stage 0xE0). A player's death (the mission's 2) does the
 * same.
 * Matched 2026-10-16; it was parked for three spare ugen integer temps in the goal-time block
 * and the 7th route's reload; a local for the time left after the hour (kept the frame) and the
 * model's null test spending the temp instead of a hidden reload closed it. */
s32 DtskHikyaku_proc(Obj *o, s16 msg, void *arg) {
    HikyakuW *w = HIKYAKU(o);
    u32 data;
    CamView *view;
    s32 route;
    s32 hour;
    s32 rest;
    s32 pad[1];
    Vec3f at;

    switch (msg) {
        case 2:
            o->name = "d_hikyaku";
            if (!swWorkTest(0x29)) {
                DPRINTF(">>>>    飛脚弁当ミッションたってません  終了\n");
                return -1;
            }
            if (swWorkTest(0xB6)) {
                DPRINTF(">>>>    手形もらってたら  終了\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, dtskHikyakuAnims[1]);
            if (w->model == NULL) {
                DPRINTF("でまへんで\n");
                return -1;
            }
            DPRINTF("でました！\n");
            w->anims = dtskHikyakuAnims;
            route = fadeRandRange(1, 120);
            if (route < 20) {
                DPRINTF(">>>>   hRoute1へ\n");
                w->x = 1664.0f;
                w->z = 43.0f;
                w->path = w->route = (u16 *) fadeSegToRam(hikyakuRoute1, o->file);
            } else if (route < 40) {
                DPRINTF(">>>>   hRoute2へ\n");
                w->x = 0.0f;
                w->z = 1660.0f;
                w->path = w->route = (u16 *) fadeSegToRam(hikyakuRoute2, o->file);
            } else if (route < 60) {
                DPRINTF(">>>>   hRoute3へ\n");
                w->x = 662.0f;
                w->z = 166.0f;
                w->path = w->route = (u16 *) fadeSegToRam(hikyakuRoute3, o->file);
            } else if (route < 80) {
                DPRINTF(">>>>   hRoute4へ\n");
                w->x = -1390.0f;
                w->z = 124.0f;
                w->path = w->route = (u16 *) fadeSegToRam(hikyakuRoute4, o->file);
            } else if (route < 100) {
                DPRINTF(">>>>   hRoute5へ\n");
                w->x = 662.0f;
                w->z = 166.0f;
                w->path = w->route = (u16 *) fadeSegToRam(hikyakuRoute5, o->file);
            } else if (route < 120) {
                DPRINTF(">>>>   hRoute6へ\n");
                w->x = -946.0f;
                w->z = 786.0f;
                w->path = w->route = (u16 *) fadeSegToRam(hikyakuRoute6, o->file);
            } else {
                DPRINTF(">>>>   hRoute6へ\n");
                w->x = -946.0f;
                w->z = 786.0f;
                w->path = w->route = (u16 *) fadeSegToRam(hikyakuRoute6, o->file);
            }
            w->y = 2.0f;
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("DtskHikyaku_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->data = (PersonData *) stageAddrToRam(data);
            w->shadow = shadowCreate();
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 100, 100, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            hour = gGameWork.time / 0x10000;
            rest = gGameWork.time - (hour << 16);
            DPRINTF(">>>>>  目標(時間、分) [%d]時[%d]分\n");
            /* FAKE: the (s8) cast and the double shift stand for three ugen temporaries the game
             * spends here with no instruction (as1 deletes the sll/sra pair under the & 0xFF and
             * folds the two shifts into one): two before "hour + 1", one before the minutes. */
            w->counter = eneMakeEx(NULL, ENE_DEMO_TIMER, 0, 0.0f, 0.0f, 0.0f, 2,
                                   (u32 (*)(void *, void *, void *)) ((((s8) hour + 1) & 0xFF) | ((((rest / 1092) << 4) << 4) & 0xFF00)));
            if (w->counter == NULL) {
                DPRINTF("カウンターできまへん\n");
                return -1;
            }
            w->daySeconds = gGameWork.daySeconds;
            gGameWork.daySeconds = 2400;
            gGameWork.mission = 1;
            DPRINTF(">>>>    飛脚弁当セットＯＫ  終了\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 0:
            eneStepHookPre(o);
            DPRINTF("******** HIKYAKU  1\n");
            switch (w->state) {
                case 0:
                    w->path = personActStepRun(o, w->route, w->path, (u32 *) dtskHikyakuAnims[0]);
                    if ((s16) *w->path == -1) {
                        w->state = 10;
                    }
                    if (acollDmgWasHit(w->dmg) == 1) {
                        DPRINTF(">>>>   飛脚いて〜〜\n");
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        w->counter = tskDestroy(w->counter);
                        w->dmg->flags &= ~4;
                        eneHitSoundAlt(w->dmg);
                        w->vel[2] = 0.0f;
                        w->vel[0] = 0.0f;
                        demoPlStandAll();
                        w->state = 5;
                    }
                    if (w->counter != NULL && demoEneState2Check(w->counter)) {
                        w->vel[0] = w->vel[1] = w->vel[2] = 0.0f;
                        w->dmg->flags &= ~2;
                        w->state = 11;
                    }
                    break;
                case 5:
                    if ((Obj *) w->dmg->hitPower != NULL) {
                        tskSendMsg((Obj *) w->dmg->hitPower, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 18.0f) == 1) {
                            w->state = 6;
                        }
                    }
                    break;
                case 6:
                    scRunStart(0x29F, plGetIndex((Obj *) w->dmg->hitPower) + 1, plGetChara((Obj *) w->dmg->hitPower));
                    w->model->anim = dtskHikyakuAnims[1];
                    w->model->frame = 0.0f;
                    w->state = 7;
                    break;
                case 7:
                    scRunStart(0x21, 0, 0);
                    w->state = 8;
                    break;
                case 8:
                    if (!scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        if (!swWorkTest(0x2A)) {
                            sndPlay(10);
                        }
                        w->timer = 0;
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (w->timer == 1) {
                        hikyakuFadeOut(16 / gRetracesPerFrame);
                        if (w->counter != NULL) {
                            w->counter = tskDestroy(w->counter);
                        }
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        gGameWork.daySeconds = w->daySeconds;
                        if (!swWorkTest(0x2A)) {
                            sndPlay(0x45F);
                        }
                    }
                    if (w->timer == 20) {
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        swWorkSet(0x92);
                        stageGotoSpawn(gStageTask->parent, 0xE0, 0, -20.0f, 10.0f, 3.0f, 3.141592741f, 0xD7);
                    }
                    break;
                case 10:
                    if (acollDmgWasHit(w->dmg) == 1) {
                        DPRINTF(">>>>   飛脚いて〜〜\n");
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                        w->counter = tskDestroy(w->counter);
                        w->dmg->flags &= ~4;
                        w->vel[2] = 0.0f;
                        w->vel[0] = 0.0f;
                        eneHitSoundAlt(w->dmg);
                        demoPlStandAll();
                        w->state = 5;
                    }
                    if (demoEneState2Check(w->counter)) {
                        w->state = 11;
                    }
                    break;
                case 11:
                    demoPlStandAll();
                    scRunStart(0x263, 0, 0);
                    w->state = 8;
                    break;
            }
            if (gGameWork.mission == 2 && w->state != 9) {
                DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                gGameWork.scenarioRunning = 2;
                sndPlay(10);
                demoPlStandAll();
                w->timer = 0;
                w->state = 9;
            }
            if (w->timer % 3 == 0) {
                itexPuffWhite(w->model->pos[0] + fadeRandRange(1, 3), w->model->pos[1] + 3.0f,
                              w->model->pos[2] + fadeRandRange(1, 3), 0.025f);
            }
            DPRINTF("******** HIKYAKU  2\n");
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            DPRINTF("******** HIKYAKU  3\n");
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            } else {
                DPRINTF("******** FRAME naide \n");
            }
            DPRINTF("******** HIKYAKU  4\n");
            if (w->model != NULL) {
                eneAnimLoop(o, w->model, 1.0f);
            }
            DPRINTF("******** HIKYAKU  5\n");
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "jizou": a talking jizou on the road, there from the mission's start (flag 0x29) until the
 * pass is given (0xB6). Talked to, it talks (scenario 0x265). */
s32 hikyakuJizouProc(Obj *o, s16 msg, void *arg) {
    JizouW *w = JIZOU(o);
    u32 data;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "jizou";
            if (!swWorkTest(0x29) || swWorkTest(0xB6)) {
                DPRINTF("****  jizou: stopper stop....\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) hikyakuJizouAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("jizou_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->model->scale[0] = 0.4f;
            w->model->scale[2] = 0.6f;
            w->model->scale[1] = 0.2f;
            w->box->radius = 150;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        gGameWork.partake = 1;
                        personTalkCamAt(o, w->talker, 80);
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(0x265, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 2;
                    break;
                case 2:
                    if (!scIsRunning()) {
                        gGameWork.partake = 0;
                        demoFinish(o);
                        camAiSetHold(0);
                        w->state = 0;
                    }
                    break;
            }
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.5f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
