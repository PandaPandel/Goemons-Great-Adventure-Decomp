#include "tlb.h"

/* File 105: the shops' display objects ("struct SHOP_OBJ_W"). Each kind's proc hands its model
 * to a manager: the general store's (Yorozu-ya, "managerYorozu_proc") or the restaurant's
 * (Manpuku, "managerManpuku_proc"), which picks by the placement's type either the plain
 * display "Normal" (a model on the course) or "comDisp" (the model under its own camera and
 * light, growing in and turning; switch 0x12D fades it out). "roomBG" is the shop room's
 * background model. */

extern Frame *gFrmLayer3;

extern u8 shopDeroderoTex2[];
extern u8 shopDeroderoTex3[];
extern u8 shopGinYoroiTex[];
AnimFrame shopGinYoroiAnim[] = {
    { (u32) shopGinYoroiTex, 1 },
    { 0, 0 },
};
AnimFrame shopKinYoroiAnim[] = {
    { 0x03002F60, 1 },
    { 0, 0 },
};
AnimFrame shopDeroderoAnim[] = {
    { 0x03001070, 3 },
    { 0x03001110, 3 },
    { (u32) shopDeroderoTex2, 3 },
    { (u32) shopDeroderoTex3, 3 },
    { 0, 0 },
    { 0, 0 },
};

extern Gfx shopRoomBGDl[];
extern Gfx shopYorozuItem4Dl[];
extern Gfx shopManpukuDish41aDl[];
extern Gfx shopManpukuDish40aDl[];
extern Gfx shopManpukuDish44aDl[];
extern Gfx shopManpukuDish42aDl[];
extern Gfx shopManpukuDish43aDl[];
extern Gfx shopYorozuItem1Dl[];
extern Gfx shopYorozuItem0Dl[];
extern Gfx shopManpukuDish40bDl[];
extern Gfx shopManpukuDish45aDl[];
extern Gfx shopManpukuDish43bDl[];
extern Gfx shopManpukuDish44bDl[];
extern Gfx shopManpukuDish41bDl[];
extern Gfx shopManpukuDeroderoDl[];
extern Gfx shopManpukuDish42bDl[];
extern Gfx shopYorozuItem2Dl[];
extern Gfx shopManpukuDish40cDl[];
extern Gfx shopYorozuGinYoroiDl[];
extern Gfx shopManpukuDish45bDl[];
extern Gfx shopManpukuDish43cDl[];
extern Gfx shopManpukuDish41cDl[];
extern Gfx shopManpukuDish40dDl[];
extern Gfx shopYorozuItem3Dl[];
extern Gfx shopManpukuDish40eDl[];
extern Gfx shopYorozuKinYoroiDl[];
extern Gfx shopManpukuDish40fDl[];
extern Gfx shopManpukuDish42cDl[];
extern Gfx shopManpukuDish45cDl[];
extern Gfx shopManpukuDish45dDl[];
extern Gfx shopManpukuDish45eDl[];
extern Gfx shopManpukuDish45fDl[];

/* A shop display's work (task + 0x20), "struct SHOP_OBJ_W": an enemy's (EneObj) up to 0x34. */
typedef struct ShopObjW {
    /* 0x00 */ u8 state; /* comDisp: 0 growing in and turning, 1 fading out */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ Anim anim; /* its texture's */
    /* 0x44 */ Frame *unused44; /* destroyed in message 1 if set, never made */
    /* 0x48 */ CamView *view; /* comDisp's own camera */
    /* 0x4C */ u16 count; /* steps */
    /* 0x4E */ u8 alpha; /* comDisp: the environment colour's alpha (25 a step as it fades out) */
    /* 0x4F */ u8 bright; /* comDisp: the environment colour's r, g and b */
    /* 0x50 */ u8 animOn; /* the texture is animated */
    /* 0x51 */ u8 unused51;
    /* 0x52 */ u8 type; /* its placement's type: 0 Normal, 1-3 comDisp */
} ShopObjW;

#define SHOP(obj) ((ShopObjW *) FX(obj))

s32 shopNormalProc(Obj *o, s16 msg, void *arg, Gfx *model, s32 file, f32 scale);
s32 shopComDispProc(Obj *o, s16 msg, void *arg, Gfx *model, s32 file, f32 scale);
s32 managerYorozu_proc(Obj *o, s16 msg, void *arg, Gfx *model);
s32 managerManpuku_proc(Obj *o, s16 msg, void *arg, Gfx *model, s32 file);
Gfx *shopDlEnvTexFood(Gfx *gfx, s32 a, s32 r, s32 g, s32 b, s32 noSetup);
Gfx *shopDlEnvTexArmor(Gfx *gfx, s32 a, s32 r, s32 g, s32 b, s32 noSetup);

/* "Normal": the model on the course, at scale, its file's segments set. */
s32 shopNormalProc(Obj *o, s16 msg, void *arg, Gfx *model, s32 file, f32 scale) {
    ShopObjW *w = SHOP(o);

    switch (msg) {
        case 2:
            o->name = "Normal";
            eneSetup(o, arg);
            if (model == shopYorozuGinYoroiDl) {
                if ((w->model = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol)) == NULL) {
                    return -1;
                }
                w->model->flags |= 0x10;
                fadeAnimStart(&w->anim, shopGinYoroiAnim, 0, &w->model->segs[3], floadGetSetFile(0x128, 0));
                w->animOn = 1;
            } else if (model == shopYorozuKinYoroiDl) {
                if ((w->model = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol)) == NULL) {
                    return -1;
                }
                w->model->flags |= 0x10;
                fadeAnimStart(&w->anim, shopKinYoroiAnim, 0, &w->model->segs[3], floadGetSetFile(0x128, 0));
                w->animOn = 1;
            } else {
                if ((w->model = (PolFrame *) frmCreate(gFrmLayer2, gFrmKindPol)) == NULL) {
                    return -1;
                }
                w->model->flags |= 0x20;
            }
            w->model->dl2 = model;
            w->model->scale[2] = scale;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            floadGetSetRam((void **) w->model->segs, file);
            w->count = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->unused44 != NULL) {
                frmDestroy(w->unused44);
            }
            if (w->view != NULL) {
                frmDestroy(&w->view->f);
            }
            break;
        case 3:
            if (model == shopYorozuGinYoroiDl || model == shopYorozuKinYoroiDl) {
                w->model->dl = gGfxDlHead;
                gGfxDlHead = shopDlEnvTexArmor(gGfxDlHead, 0, 0, 0, 0, 0);
            } else {
                w->model->dl = eneDlEnvColorLit(o, 0, 0, 0, 0, 0);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (w->animOn) {
                fadeAnimStep(&w->anim);
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "comDisp": the model under its own camera and light, growing to its scale while it turns;
 * switch 0x12D lights it up and fades it out, and it ends. */
s32 shopComDispProc(Obj *o, s16 msg, void *arg, Gfx *model, s32 file, f32 scale) {
    ShopObjW *w = SHOP(o);
    LightFrame *light;

    switch (msg) {
        case 2:
            o->name = "comDisp";
            PRINTF("struct SHOP_OBJ_W size %d\n", sizeof(ShopObjW));
            if ((w->view = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera)) == NULL) {
                PRINTF("comDisp cam NULL\n");
                return -1;
            }
            w->view->eye[1] = 8.0f;
            w->view->at[1] = -100.0f;
            light = (LightFrame *) frmCreate(&w->view->f, gFrmKindLight);
            light->lightNum = 1;
            if ((w->model = (PolFrame *) frmCreate(&w->view->f, gFrmKindPol)) == NULL) {
                PRINTF("comDisp p NULL\n");
                return -1;
            }
            w->model->dl2 = model;
            floadGetSetRam((void **) w->model->segs, file);
            if (model == shopManpukuDeroderoDl) {
                PRINTF("comDisp model is food_derodero_nanika\n");
                w->model->flags |= 0x27;
                fadeAnimStart(&w->anim, shopDeroderoAnim, 0, &w->model->segs[3], floadGetSetFile(0x44, 0));
                w->animOn = 1;
                w->view->at[1] = -108.0f;
            } else if (model == shopYorozuGinYoroiDl) {
                PRINTF("comDisp model is yorozu_obj_gin_yoroi\n");
                w->model->flags |= 0x30;
                fadeAnimStart(&w->anim, shopGinYoroiAnim, 0, &w->model->segs[3], floadGetSetFile(0x128, 0));
                w->animOn = 1;
            } else if (model == shopYorozuKinYoroiDl) {
                PRINTF("comDisp model is yorozu_obj_kin_yoroi\n");
                w->model->flags |= 0x30;
                fadeAnimStart(&w->anim, shopKinYoroiAnim, 0, &w->model->segs[3], floadGetSetFile(0x128, 0));
                w->animOn = 1;
            }
            w->x = -4.0f;
            w->y = -2.0f;
            w->z = 370.0f;
            w->model->scale[2] = 0.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->alpha = 0;
            w->state = 0;
            PRINTF("comDisp CREATE\n");
            break;
        case 3:
            if (model == shopManpukuDeroderoDl) {
                w->model->dl = gGfxDlHead;
                gGfxDlHead = shopDlEnvTexFood(gGfxDlHead, w->alpha, w->bright, w->bright, w->bright, 0);
            } else if (model == shopYorozuGinYoroiDl || model == shopYorozuKinYoroiDl) {
                w->model->dl = gGfxDlHead;
                gGfxDlHead = shopDlEnvTexArmor(gGfxDlHead, w->alpha, w->bright, w->bright, w->bright, 0);
            } else {
                w->model->dl = eneDlEnvColorLit(o, w->alpha, w->bright, w->bright, w->bright, 1);
            }
            break;
        case 1:
            PRINTF("comDisp DESTROY\n");
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->unused44 != NULL) {
                frmDestroy(w->unused44);
            }
            if (w->view != NULL) {
                frmDestroy(&w->view->f);
            }
            break;
        case 0:
            if (w->animOn) {
                fadeAnimStep(&w->anim);
            }
            if (swWorkTest(0x12D)) {
                swWorkReset(0x12D);
                w->bright = 0xFF;
                w->state = 1;
            }
            switch (w->state) {
                case 0:
                    w->model->scale[2] += scale / 10;
                    w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    if (scale < w->model->scale[0]) {
                        w->model->scale[2] = scale;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                    }
                    w->rot[1] += 0.05235987902f;
                    break;
                case 1:
                    w->alpha += 25;
                    if (w->alpha == 250) {
                        return -1;
                    }
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModel(o, &w->model->f);
            }
            w->count++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The general store's manager: the type from the placement, the model's file, then Normal
 * (type 0) or comDisp (1-3). */
s32 managerYorozu_proc(Obj *o, s16 msg, void *arg, Gfx *model) {
    ShopObjW *w = SHOP(o);
    s32 file;

    if (msg == 2) {
        w->type = (u32) ((EneArg *) arg)->c;
    }
    if (model == shopYorozuItem1Dl || model == shopYorozuItem2Dl || model == shopYorozuItem3Dl) {
        file = 0x10D;
    } else if (model == shopYorozuItem4Dl) {
        file = 0x3F;
    } else {
        file = 0x3E;
    }
    PRINTF("managerYorozu_proc : prm is %d\n", w->type);
    switch (w->type) {
        case 0:
            return shopNormalProc(o, msg, arg, model, file, 0.1f);
        case 1:
        case 2:
        case 3:
            return shopComDispProc(o, msg, arg, model, file, 0.03f);
    }
    return -1;
}

/* The restaurant's manager: comDisp for every type, smaller for one model. */
s32 managerManpuku_proc(Obj *o, s16 msg, void *arg, Gfx *model, s32 file) {
    ShopObjW *w = SHOP(o);
    f32 scale;

    if (msg == 2) {
        w->type = (u32) ((EneArg *) arg)->c;
    }
    if (model == shopManpukuDish40dDl) {
        scale = 0.016f;
    } else {
        scale = 0.03f;
    }
    PRINTF("managerManpuku_proc : prm is %d\n", w->type);
    switch (w->type) {
        case 0:
        case 1:
        case 2:
        case 3:
            return shopComDispProc(o, msg, arg, model, file, scale);
    }
    return -1;
}

/* The general store's displays. */
s32 shopYorozuItem0Proc(Obj *o, s16 msg, void *arg) {
    return managerYorozu_proc(o, msg, arg, shopYorozuItem0Dl);
}

s32 shopYorozuGinYoroiProc(Obj *o, s16 msg, void *arg) {
    return managerYorozu_proc(o, msg, arg, shopYorozuGinYoroiDl);
}

s32 shopYorozuKinYoroiProc(Obj *o, s16 msg, void *arg) {
    return managerYorozu_proc(o, msg, arg, shopYorozuKinYoroiDl);
}

s32 shopYorozuItem1Proc(Obj *o, s16 msg, void *arg) {
    return managerYorozu_proc(o, msg, arg, shopYorozuItem1Dl);
}

s32 shopYorozuItem2Proc(Obj *o, s16 msg, void *arg) {
    return managerYorozu_proc(o, msg, arg, shopYorozuItem2Dl);
}

s32 shopYorozuItem3Proc(Obj *o, s16 msg, void *arg) {
    return managerYorozu_proc(o, msg, arg, shopYorozuItem3Dl);
}

s32 shopYorozuItem4Proc(Obj *o, s16 msg, void *arg) {
    return managerYorozu_proc(o, msg, arg, shopYorozuItem4Dl);
}

/* The restaurant's dishes, by file. */
s32 shopManpukuDish40aProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish40aDl, 0x40);
}

s32 shopManpukuDish40bProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish40bDl, 0x40);
}

s32 shopManpukuDish40cProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish40cDl, 0x40);
}

s32 shopManpukuDish40dProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish40dDl, 0x40);
}

s32 shopManpukuDish40eProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish40eDl, 0x40);
}

s32 shopManpukuDish40fProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish40fDl, 0x40);
}

s32 shopManpukuDish41aProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish41aDl, 0x41);
}

s32 shopManpukuDish41bProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish41bDl, 0x41);
}

s32 shopManpukuDish41cProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish41cDl, 0x41);
}

s32 shopManpukuDish42aProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish42aDl, 0x42);
}

s32 shopManpukuDish42bProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish42bDl, 0x42);
}

s32 shopManpukuDish42cProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish42cDl, 0x42);
}

s32 shopManpukuDish43aProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish43aDl, 0x43);
}

s32 shopManpukuDish43bProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish43bDl, 0x43);
}

s32 shopManpukuDish43cProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish43cDl, 0x43);
}

s32 shopManpukuDish44aProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish44aDl, 0x44);
}

s32 shopManpukuDish44bProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish44bDl, 0x44);
}

s32 shopManpukuDeroderoProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDeroderoDl, 0x44);
}

s32 shopManpukuDish45aProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish45aDl, 0x45);
}

s32 shopManpukuDish45bProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish45bDl, 0x45);
}

s32 shopManpukuDish45cProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish45cDl, 0x45);
}

s32 shopManpukuDish45dProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish45dDl, 0x45);
}

s32 shopManpukuDish45eProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish45eDl, 0x45);
}

s32 shopManpukuDish45fProc(Obj *o, s16 msg, void *arg) {
    return managerManpuku_proc(o, msg, arg, shopManpukuDish45fDl, 0x45);
}

/* "roomBG": the shop room's background model, at scale 1 on its parent frame. */
/* History (was NON_MATCHING: 23 differing lines, all but 3 the section's own addresses, mix
 * distance 1): frame and slots were the game's; only message 0 differed: the game computes w straight into a0 for eneMove
 * ("addiu a0,s0,32; sw a0,32(sp)"), here into a2 then "move a0,a2" (w's range there coloured
 * a2, as at the top). Measured (one check each): the chain on "w->model->scale[0] = ..[1] =
 * ..[2] = 1.0f" reloads w->model and scale[2] each link (60 lines, frame 0x30); a pointer m with
 * the chain (function scope, either order, or block scope): the game's single load and stores,
 * 37 (spill at 0x24, the game's 0x20); m with three separate stores [2], [1], [0]: 39 (frame
 * 0x30, spill at 0x20), with a pad first or last 37; SHOP(o)/ENE(o) at each use, no w: 80 (o's
 * offsets folded, frame 0x20). The pads sweep: the chain plus a 1-word pad (any position): 23,
 * here (FAKE pad). Then the splits sweep (60 FAKE boundaries): none below 23; "eneMove(ENE(o))"
 * with w kept for the rest: 23, the same code (IDO shares the o + 0x20). 2026-10-11,
 * no change (23): "w = SHOP(o);" again in message 0 after eneStepHookPre; message 0 through
 * a second variable (m) set from SHOP(o). With a FAKE "^ 0": "w = ..FX(o) ^ 0" in message 0
 * 46 (message 0 the game's, w then v1 at the top), "eneMove(w ^ 0)" 59. 2026-10-12 (second try), one
 * check each: `results` (every call's result kept / void cast as s32, one at a time and per
 * function) 23 at best (frmMove int 8 real); FAKE "if (w) {}" after eneMove 23; w declared bare,
 * "w = SHOP(o);" above the switch 23; w set at the start of cases 2, 1 and 0 (none at the top):
 * 30 lines, length right (109) but case 1 then reads 44(s0)/48(s0) directly and message 0 still
 * "addiu a2; move a0,a2" -- so the move is not from w's web reaching message 0 from the top;
 * "eneMove((EneObj *) (w = SHOP(o)))" 23. 2026-10-12 (third try): a second pointer e (in the
 * pad's place) set "((u32) o + sizeof(Obj)) ^ 0" in message 0 for eneMove and the model test:
 * real 33 (message 0 the game's, w v1 at the top, as with w itself); e for eneMove only, w for
 * the model test: real 22, length +1. So the game's w web does reach message 0.
 * 2026-10-12 (fourth try): the empty case 3 moved after case 0 (IDO_NOTES 378 E2: an empty case
 * moves allocation): real 4, the same. m reused in message 0 for the model test: 11.
 * 2026-10-12 (fifth try): "eneMove((EneObj *) (o + 1))" (pointer arithmetic, not FX's u32 add):
 * 4, the same (uopt still shares it with w); w a block local of each case: 18. The permuter
 * (--stack-diffs, ~1,900 iterations, base 135 to 90): an empty "if (w->model == NULL) {}"
 * between eneStepHookPre and eneMove: MATCH (its other change, shopRoomBGDl through a local, not
 * needed). Without the pad then: 8. */
s32 shopRoomBGProc(Obj *o, s16 msg, void *arg) {
    s32 pad; /* FAKE: the pads sweep's */
    ShopObjW *w = SHOP(o);
    PolFrame *m;

    switch (msg) {
        case 2:
            o->name = "roomBG";
            eneSetup(o, arg);
            if ((w->model = eneNewPol(o, shopRoomBGDl)) == NULL) {
                return -1;
            }
            frmMove(&w->model->f, gFrmLayer3);
            m = w->model;
            m->scale[0] = m->scale[1] = m->scale[2] = 1.0f;
            w->count = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->unused44 != NULL) {
                frmDestroy(w->unused44);
            }
            if (w->view != NULL) {
                frmDestroy(&w->view->f);
            }
            break;
        case 3:
            break;
        case 0:
            eneStepHookPre(o);
            if (w->model == NULL) { /* FAKE: an empty test (the permuter's find); IDO_NOTES 378 E5 */
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The setup comDisp's food draws with: one cycle, point-sampled, the environment colour
 * blended over the texture by its alpha (unless noSetup), then the colour (r, g, b, a). */
Gfx *shopDlEnvTexFood(Gfx *gfx, s32 a, s32 r, s32 g, s32 b, s32 noSetup) {
    if (!noSetup) {
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_1CYCLE);
        gDPSetTexturePersp(gfx++, G_TP_PERSP);
        gDPSetColorDither(gfx++, G_CD_DISABLE);
        gDPSetTextureConvert(gfx++, G_TC_FILT);
        gDPSetTextureFilter(gfx++, G_TF_POINT);
        gSPClearGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                                        G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH);
        gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH);
        gDPSetRenderMode(gfx++, G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2);
        gDPSetCombineLERP(gfx++, ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, ENVIRONMENT, TEXEL0,
                          ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0);
        gDPSetAlphaCompare(gfx++, G_AC_NONE);
    }
    gDPSetEnvColor(gfx++, r, g, b, a);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* The same for the armour: bilinear, opaque surface, the geometry mode left alone. */
Gfx *shopDlEnvTexArmor(Gfx *gfx, s32 a, s32 r, s32 g, s32 b, s32 noSetup) {
    if (!noSetup) {
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_1CYCLE);
        gDPSetTexturePersp(gfx++, G_TP_PERSP);
        gDPSetColorDither(gfx++, G_CD_DISABLE);
        gDPSetTextureConvert(gfx++, G_TC_FILT);
        gDPSetTextureFilter(gfx++, G_TF_BILERP);
        gDPSetRenderMode(gfx++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
        gDPSetCombineLERP(gfx++, ENVIRONMENT, TEXEL0, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, ENVIRONMENT, TEXEL0,
                          ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0);
        gDPSetAlphaCompare(gfx++, G_AC_NONE);
    }
    gDPSetEnvColor(gfx++, r, g, b, a);
    gSPEndDisplayList(gfx++);
    return gfx;
}
