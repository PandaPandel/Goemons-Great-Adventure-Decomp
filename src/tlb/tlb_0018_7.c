#include "tlb.h"
#include "tlb/tlb_0018.h"

/* The character select: four panels ("tate"), each with a character turning on a stand ("doll")
 * and its name plate; a cursor per player ("pscur") moves along them with left and right,
 * changes the costume with up and down and takes the character with A (a "kettei" box marks it);
 * B gives it back. */

/* Segment addresses: the stand's and the panel's display lists, the four name plates, the
 * "kettei" box, the cursor and the players' marks ("1P".."4P", 32x13 CI4 each). */
extern u8 pseleDollStandDl[];
extern u8 pseleTateDl[];
extern u8 pseleTateChara0Tex[];
extern u8 pseleTateChara1Tex[];
extern u8 pseleTateChara2Tex[];
extern u8 pseleTateChara3Tex[];
extern u8 pseleKetteiDl[];
extern u8 pseleCursorTex[];
extern u16 pselePlayerMarkTex[];

extern Gfx titlePanelDl[]; /* the panel's render setup */
extern Gfx titleKetteiBoxDl[]; /* the "kettei" box's render setup */
extern Gfx titleCursorDl[]; /* the cursor's render setup */
extern u16 titleCursorPalettes[5][16]; /* the cursor's palettes: 0 white, then one per player */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 titleStartTextEn[];
extern u8 titleStartTextFr[];
extern u8 titleStartTextDe[];
extern u8 titleOptionTextEn[];
extern u8 titleOptionTextFr[];
extern u8 titleOptionTextDe[];
extern u8 titleStereoTextEn[];
extern u8 titleStereoTextFr[];
extern u8 titleStereoTextDe[];
extern u8 titleMonauralTextEn[];
extern u8 titleMonauralTextFr[];
extern u8 titleMonauralTextDe[];
extern u8 titleLanguageTextEn[];
extern u8 titleLanguageTextFr[];
extern u8 titleLanguageTextDe[];
extern u8 titleSoundTextEn[];
extern u8 titleSoundTextFr[];
extern u8 titleSoundTextDe[];
extern u8 titleEnglishTextEn[];
extern u8 titleEnglishTextFr[];
extern u8 titleEnglishTextDe[];
extern u8 titleFrenchTextEn[];
extern u8 titleFrenchTextFr[];
extern u8 titleFrenchTextDe[];
extern u8 titleGermanTextEn[];
extern u8 titleGermanTextFr[];
extern u8 titleGermanTextDe[];
Gfx titleEmptyDl[] = {
    gsSPEndDisplayList(),
};
u32 titleTexFmt = 0x00000004;
u16 titleCursorPalettes[5][16] = { 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFF, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF801, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057E, 0x057F, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C1, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83E, 0xF83F }; /* the cursor's palettes: 0 white, then one per player */
Gfx titleCursorDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsSPEndDisplayList(),
}; /* the cursor's render setup */
u32 titleStartText[] = { (u32) titleStartTextEn, (u32) titleStartTextFr, (u32) titleStartTextDe };
u32 titleOptionText[] = { (u32) titleOptionTextEn, (u32) titleOptionTextFr, (u32) titleOptionTextDe };
u32 titleStereoText[] = { (u32) titleStereoTextEn, (u32) titleStereoTextFr, (u32) titleStereoTextDe };
u32 titleMonauralText[] = { (u32) titleMonauralTextEn, (u32) titleMonauralTextFr, (u32) titleMonauralTextDe };
u32 titleLanguageText[] = { (u32) titleLanguageTextEn, (u32) titleLanguageTextFr, (u32) titleLanguageTextDe };
u32 titleSoundText[] = { (u32) titleSoundTextEn, (u32) titleSoundTextFr, (u32) titleSoundTextDe };
u32 titleEnglishText[] = { (u32) titleEnglishTextEn, (u32) titleEnglishTextFr, (u32) titleEnglishTextDe };
u32 titleFrenchText[] = { (u32) titleFrenchTextEn, (u32) titleFrenchTextFr, (u32) titleFrenchTextDe };
u32 titleGermanText[] = { (u32) titleGermanTextEn, (u32) titleGermanTextFr, (u32) titleGermanTextDe, 0x00000000 };
Gfx titlePanelDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | 0x0000F9FA),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsSPEndDisplayList(),
}; /* the panel's render setup */
Gfx titleKetteiBoxDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
    gsSPEndDisplayList(),
}; /* the "kettei" box's render setup */



s32 pseleDollProc(Obj *obj, s16 msg, void *arg);
Obj *pseleDollMake(Obj *parent, s32 chara, f32 x, f32 y, f32 z);
f32 pseleDollGetRot(Obj *obj);
void pseleDollSetRot(Obj *obj, f32 rot);
s32 pseleTateProc(Obj *obj, s16 msg, void *arg);
s32 pseleKetteiProc(Obj *obj, s16 msg, void *arg);
Obj *pseleKetteiMake(Obj *parent, s32 chara, f32 x, f32 y);
Gfx *pseleDrawCursorSprite(Gfx *gfx, u32 file, f32 x, f32 y, s32 pal, u32 tex, s32 width,
                            s32 height);
s32 pselePscurProc(Obj *obj, s16 msg, void *arg);

/* "doll": a character turning on a stand in its panel, the player itself (plCreate) or, if that
 * cannot be made, a bare "anime" frame. Drawn through a scissor of its panel. */
typedef struct DollWork {
    /* 0x00 */ s32 chara;
    /* 0x04 */ Obj *player;
    /* 0x08 */ AnimeFrame *model;
    /* 0x0C */ PolFrame *stand;
} DollWork;

#define DOLL(obj) ((DollWork *) FX(obj))

s32 pseleDollProc(Obj *obj, s16 msg, void *arg) {
    DollWork *w = DOLL(obj);
    s32 pad;
    PlInit init;
    Player *pl;

    switch (msg) {
        case 2:
            obj->name = "doll";
            w->chara = (s32) arg;
            w->stand = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->stand == NULL) {
                return -1;
            }
            w->stand->dl2 = (Gfx *) pseleDollStandDl;
            w->stand->rot[0] = -1.570796371f;
            floadGetSetRam((void **) w->stand->segs, 0x10A);
            w->stand->scale[2] = 0.5f;
            w->stand->scale[0] = w->stand->scale[1] = w->stand->scale[2];
            plMakeInit(&init, w->chara, w->chara);
            init.flags |= 0x90000000;
            init.action = 0xD8;
            if ((w->player = plCreate(obj->parent->parent, &init)) != NULL) {
                pl = PL(w->player);
                w->model = pl->model;
            } else {
                w->model = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                if (w->model == NULL) {
                    return -1;
                }
            }
            w->model->dl = gDrawSetupLitTexEdgeDl;
            frmMove(&w->model->f, gFrmLayer8);
            w->model->scale[2] = 0.25f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            break;
        case 1:
            frmDestroy(&w->stand->f);
            if (w->player != NULL) {
                tskDestroy(w->player);
            } else {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            w->model->rot[1] += 0.01745329238f;
            w->stand->rot[1] = w->model->rot[1];
            w->stand->pos[0] = w->model->pos[0];
            w->stand->pos[1] = w->model->pos[1];
            w->stand->pos[2] = w->model->pos[2];
            break;
        case 3:
            w->stand->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, gDrawSetupLitTexEdgeDl);
            gDPSetScissor(gGfxDlHead++, G_SC_NON_INTERLACE, w->chara * 70 + 30, 62,
                          w->chara * 70 + 82, 172);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *pseleDollMake(Obj *parent, s32 chara, f32 x, f32 y, f32 z) {
    Obj *o;

    if ((o = tskCreate(parent, pseleDollProc, parent->file, (void *) chara)) != NULL) {
        DOLL(o)->model->pos[0] = x;
        DOLL(o)->model->pos[1] = y;
        DOLL(o)->model->pos[2] = z;
    }
    return o;
}

f32 pseleDollGetRot(Obj *obj) {
    if (obj == NULL) {
        return 0.0f;
    }
    return DOLL(obj)->model->rot[1];
}

void pseleDollSetRot(Obj *obj, f32 rot) {
    if (obj != NULL) {
        DOLL(obj)->model->rot[1] = rot;
    }
}

/* "tate": a character's panel, sliding to its place, with the character's name plate and its
 * doll. The doll is made again when the costume changes ("L->wear"). */
typedef struct TateWork {
    /* 0x00 */ s32 chara;
    /* 0x04 */ s32 wear; /* the costume the doll was made in */
    /* 0x08 */ f32 steps; /* how many steps a move takes */
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 targetX;
    /* 0x18 */ f32 targetY;
    /* 0x1C */ f32 velX; /* per step */
    /* 0x20 */ f32 velY;
    /* 0x24 */ PolFrame *frame;
    /* 0x28 */ DLFrame *name; /* the name plate */
    /* 0x2C */ u32 file;
    /* 0x30 */ Obj *doll;
} TateWork; /* size 0x34 */

#define TATE(obj) ((TateWork *) FX(obj))

s32 pseleTateProc(Obj *obj, s16 msg, void *arg) {
    TateWork *w = TATE(obj);
    TateWork *init = arg;
    s32 pad;
    u32 tex;
    s32 width;
    s32 height;
    f32 rot;

    switch (msg) {
        case 2:
            obj->name = "tate";
            w->chara = init->chara;
            w->x = init->x;
            w->y = init->y;
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl = (Gfx *) fadeSegToRam(titlePanelDl, obj->file);
            w->frame->dl2 = (Gfx *) pseleTateDl;
            floadGetSetRam((void **) w->frame->segs, 0x10A);
            w->frame->rot[0] = 3.141592741f;
            w->name = (DLFrame *) frmCreate(gFrmLayer2, gFrmKindDlist);
            if (w->name == NULL) {
                return -1;
            }
            w->name->f.flags &= ~2;
            w->file = floadGetSetFile(0x10A, 0);
            w->steps = 16.0f;
            break;
        case 1:
            frmDestroy(&w->name->f);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (w->velX != 0.0f) {
                if (fabsf(w->x - w->targetX) <= fabsf(w->velX)) {
                    w->x = w->targetX;
                    w->velX = 0.0f;
                } else {
                    w->x += w->velX;
                }
            }
            if (w->velY != 0.0f) {
                if (fabsf(w->y - w->targetY) <= fabsf(w->velY)) {
                    w->y = w->targetY;
                    w->velY = 0.0f;
                } else {
                    w->y += w->velY;
                }
            }
            w->frame->pos[0] = w->x;
            w->frame->pos[1] = w->y;
            if (w->doll != NULL && w->wear != gSaveData.costume[w->chara][0]) {
                PRINTF("L->wear = %d dw.char[%d].type = %d \n", w->wear, w->chara,
                       gSaveData.costume[w->chara][0]);
                rot = pseleDollGetRot(w->doll);
                pseleTateHide(obj);
                pseleTateShow(obj);
                pseleDollSetRot(w->doll, rot);
            }
            break;
        case 3:
            w->name->dl = gGfxDlHead;
            /* Each case on one line: the game stores height, width, then the plate (IDO
             * schedules by line). */
            switch (w->chara) {
                case 0:
                    tex = (u32) pseleTateChara0Tex; width = 48; height = 16; break;
                case 1:
                    tex = (u32) pseleTateChara1Tex; width = 55; height = 16; break;
                case 2:
                    tex = (u32) pseleTateChara2Tex; width = 48; height = 16; break;
                case 3:
                    tex = (u32) pseleTateChara3Tex; width = 32; height = 16; break;
            }
            gGfxDlHead = titleDrawRgba16(gGfxDlHead, obj->file, w->x - width / 2,
                                                w->y - height / 2 + 62.0f, 1.0f, 255, width,
                                                height, fadeSegToRam(tex, w->file));
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *pseleTateMake(Obj *parent, s32 chara, f32 x, f32 y) {
    TateWork init;

    init.chara = chara;
    init.x = x;
    init.y = y;
    return tskCreate(parent, pseleTateProc, parent->file, &init);
}

void pseleTateSetSteps(Obj *obj, s32 steps) {
    TATE(obj)->steps = steps;
}

/* Shows the name plate and makes the doll, in the costume the character wears. */
void pseleTateShow(Obj *obj) {
    TateWork *w = TATE(obj);
    s32 pad;

    if (w->name != NULL) {
        w->name->f.flags |= 2;
    }
    if (w->doll == NULL) {
        w->doll = pseleDollMake(obj, w->chara, (w->x - 160.0f) * 0.6f,
                                         (120.0f - w->y - 38.0f) * 0.6f, 0.0f);
    }
    w->wear = gSaveData.costume[w->chara][0];
}

/* Hides the name plate and ends the doll. */
void pseleTateHide(Obj *obj) {
    TateWork *w = TATE(obj);
    s32 pad;

    if (w->name != NULL) {
        w->name->f.flags &= ~2;
    }
    /* One line: the call's address is loaded before the work pointer (IDO schedules by line). */
    if (w->doll != NULL) { w->doll = tskDestroy(w->doll); }
}

/* The move's target, reached in its steps. */
void pseleTateMoveTo(Obj *obj, f32 x, f32 y) {
    TateWork *w = TATE(obj);

    w->targetX = x;
    w->targetY = y;
    w->velX = (x - w->x) / w->steps;
    w->velY = (y - w->y) / w->steps;
}

/* "kettei" (decided): the box over a taken character, with its voice a few frames on. */
typedef struct KetteiWork {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ s32 halfW;
    /* 0x10 */ s32 halfH;
    /* 0x14 */ s32 chara;
    /* 0x18 */ s32 timer;
} KetteiWork;

#define KETTEI(obj) ((KetteiWork *) FX(obj))

s32 pseleKetteiProc(Obj *obj, s16 msg, void *arg) {
    KetteiWork *w = KETTEI(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "kettei";
            w->frame = (PolFrame *) frmCreate(gFrmEffectLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->frame->dl2 = (Gfx *) pseleKetteiDl;
            floadGetSetRam((void **) w->frame->segs, 0x10A);
            w->frame->rot[0] = 3.141592741f;
            w->frame->scale[2] = 0.8f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->halfW = 27;
            w->halfH = 55;
            w->timer = 8;
            sndPlay(0x137);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (w->timer > 0) {
                w->timer--;
            }
            if (w->timer == 1) {
                switch (w->chara) {
                    case 0:
                        sndPlay(0x185);
                        break;
                    case 1:
                        sndPlay(0x184);
                        break;
                    case 2:
                        sndPlay(0x186);
                        break;
                    case 3:
                        sndPlay(0x187);
                        break;
                }
            }
            w->frame->pos[0] = w->x;
            w->frame->pos[1] = w->y;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(titleKetteiBoxDl, obj->file));
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 128);
            gDPFillRectangle(gGfxDlHead++, (s32) (w->x - w->halfW), (s32) (w->y - w->halfH),
                             (s32) (w->x + w->halfW), (s32) (w->y + w->halfH));
            gSPDisplayList(gGfxDlHead++, gDrawSetupXluSurfDl);
            gDPSetCombine(gGfxDlHead++, 0xFF9BFF, 0xFF36FF7F);
            gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, 255);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *pseleKetteiMake(Obj *parent, s32 chara, f32 x, f32 y) {
    Obj *o;

    if ((o = tskCreate(parent, pseleKetteiProc, parent->file, NULL)) != NULL) {
        KETTEI(o)->chara = chara;
        KETTEI(o)->x = x;
        KETTEI(o)->y = y;
    }
    return o;
}

/* A cursor sprite in a player's colours: its shadow, then the sprite through palette pal. */
Gfx *pseleDrawCursorSprite(Gfx *gfx, u32 file, f32 x, f32 y, s32 pal, u32 tex, s32 width,
                            s32 height) {
    gfx = titleSetTint(gfx, file, 0, 0, 0, 255);
    gfx = titleDrawImage4b(gfx, x, y, 1.0f, width, height, tex);
    gSPDisplayList(gfx++, fadeSegToRam(titleCursorDl, file));
    gDPLoadTLUT_pal16(gfx++, 0, fadeSegToRam(titleCursorPalettes[pal], file));
    gDPSetEnvColor(gfx++, 0, 0, 0, 255);
    return titleDrawImageCI4(gfx, x, y, 1.0f, width, height, tex);
}

/* "pscur": a player's cursor over the panels. */
typedef struct PscurWork {
    /* 0x00 */ s32 port;
    /* 0x04 */ s32 count; /* the players */
    /* 0x08 */ s32 sel; /* the character it is on */
    /* 0x0C */ u32 file;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ DLFrame *frame;
    /* 0x1C */ u8 state; /* 1 once the character is taken */
} PscurWork; /* size 0x20 */

#define PSCUR(obj) ((PscurWork *) FX(obj))

s32 pselePscurProc(Obj *obj, s16 msg, void *arg) {
    PscurWork *w = PSCUR(obj);
    s32 wear;
    s32 pad[5]; /* the game's frame has five more words, never touched */

    switch (msg) {
        case 2:
            obj->name = "pscur";
            w->port = ((PscurWork *) arg)->port;
            w->count = ((PscurWork *) arg)->count;
            w->frame = (DLFrame *) frmCreate(gFrmEffectLayer, gFrmKindDlist);
            if (w->frame == NULL) {
                return -1;
            }
            w->file = floadGetSetFile(0x10A, 0);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (joyGetHeldTime(w->port, 0x200) == 1) {
                        if (--w->sel < 0) {
                            w->sel = 3;
                        }
                        sndPlay(0x203);
                    }
                    if (joyGetHeldTime(w->port, 0x100) == 1) {
                        if (++w->sel >= 4) {
                            w->sel = 0;
                        }
                        sndPlay(0x203);
                    }
                    if (pseleGetTaker(obj->parent, w->sel) == 0) {
                        if (joyGetHeldTime(w->port, 0x400) == 1) {
                            wear = gSaveData.costume[w->sel][0];
                            do {
                                wear = (wear + 1) & 3;
                            } while (wear != 0 && gSaveData.costume[w->sel][wear] == 0);
                            gSaveData.costume[w->sel][0] = wear;
                        } else if (joyGetHeldTime(w->port, 0x800) == 1) {
                            wear = gSaveData.costume[w->sel][0];
                            do {
                                wear = (wear - 1) & 3;
                            } while (wear != 0 && gSaveData.costume[w->sel][wear] == 0);
                            gSaveData.costume[w->sel][0] = wear;
                        }
                        if (joyGetHeldTime(w->port, 0x8000) == 1) {
                            pseleSetTaker(obj->parent, w->sel, w->port);
                            pseleKetteiMake(obj, w->sel, w->sel * 70 + 56, 117.0f);
                            w->state = 1;
                        }
                    }
                    break;
                case 1:
                    if (joyGetHeldTime(w->port, 0x4000) == 1 &&
                        pseleGetTaker(obj->parent, w->sel) == w->port + 1) {
                        pseleSetTaker(obj->parent, w->sel, 0xFFFF);
                        tskDestroyChildren(obj);
                        w->state = 0;
                    }
                    break;
            }
            w->x = (w->port + 1) * 64 / (w->count + 1) + w->sel * 70 + 14;
            w->y = 194.0f;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gGfxDlHead = pseleDrawCursorSprite(gGfxDlHead, obj->file, w->x, w->y, w->port + 1,
                                                fadeSegToRam(pseleCursorTex, w->file), 32, 13);
            /* The player's mark: 32x13 at 4 bits, one after another ("(port << 5) * 13": the
             * game shifts first; "port * 32 * 13" is folded into one multiply). */
            gGfxDlHead = pseleDrawCursorSprite(
                gGfxDlHead, obj->file, w->x, w->y + 14.0f, w->port + 1,
                fadeSegToRam(&pselePlayerMarkTex[(w->port << 5) * 13 / 4], w->file), 32, 13);
            gGfxDlHead++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *pselePscurMake(Obj *parent, s32 port, s32 count) {
    PscurWork init;

    init.port = port;
    init.count = count;
    return tskCreate(parent, pselePscurProc, parent->file, &init);
}
