#include "tlb.h"
#include "tlb/tlb_0033.h"

extern u8 demoPieceTex0[];
extern u8 demoPieceTex1[];
extern u8 demoPieceTex2[];
extern u8 demoPieceTex3[];
extern u8 demoPieceTex4[];
extern u8 demoPieceTex7[];
extern u8 demoPieceTex15[];
extern u8 demoPieceTex19[];
extern u8 demoPieceTex78[];
extern u8 demoFaceTex1[];
extern u8 demoFaceTex2[];
extern u8 utaSet0F3Tex0[];
extern u8 utaOmiFaceTex[];

/* demotool.c's characters' animation table, a copy nothing reads; it starts tlb_0033_1's data. */
AnimFrame utaOmiCharaAnims[] = {
    { (u32) demoFaceTex2, 8 },
    { 0x00000000, 0 },
    { 0x04002100, 8 },
    { 0x04002940, 6 },
    { 0x04003180, 5 },
    { (u32) demoFaceTex1, 5 },
    { 0x04001080, 6 },
    { (u32) demoFaceTex1, 8 },
    { 0x04003180, 6 },
    { 0x04002940, 7 },
    { 0x04002100, 8 },
    { 0x00000000, 0 },
    { 0x04002520, 13 },
    { 0x04002D60, 13 },
    { 0x00000000, 0 },
    { 0x040035A0, 13 },
    { 0x00000000, 0 },
    { 0x03005350, 13 },
    { 0x00000000, 0 },
    { 0x03005770, 13 },
    { 0x00000000, 0 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 6 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 7 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex0, 8 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 9 },
    { (u32) demoPieceTex3, 8 },
    { (u32) demoPieceTex4, 8 },
    { (u32) demoPieceTex1, 7 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex2, 8 },
    { (u32) demoPieceTex1, 9 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 7 },
    { (u32) demoPieceTex2, 6 },
    { 0x03001570, 7 },
    { (u32) demoPieceTex0, 7 },
    { 0x03001570, 7 },
    { 0x00000000, 0 },
    { 0x03002E30, 7 },
    { 0x03001990, 6 },
    { 0x030021D0, 7 },
    { (u32) demoPieceTex7, 7 },
    { 0x03002E30, 7 },
    { 0x03001990, 8 },
    { 0x030021D0, 9 },
    { 0x030025F0, 8 },
    { 0x03002A10, 8 },
    { 0x03002E30, 7 },
    { (u32) demoPieceTex7, 7 },
    { 0x030021D0, 8 },
    { (u32) demoPieceTex7, 9 },
    { 0x03002E30, 7 },
    { 0x03001990, 7 },
    { 0x030021D0, 6 },
    { 0x03002E30, 7 },
    { 0x03001990, 7 },
    { 0x03002E30, 7 },
    { 0x00000000, 0 },
    { 0x030046F0, 7 },
    { 0x03003250, 6 },
    { 0x030046F0, 7 },
    { 0x03003A90, 7 },
    { 0x03003670, 7 },
    { 0x03003250, 8 },
    { 0x030046F0, 7 },
    { 0x03003A90, 9 },
    { (u32) demoPieceTex15, 8 },
    { 0x030042D0, 8 },
    { 0x030046F0, 7 },
    { 0x03003670, 7 },
    { 0x03003A90, 8 },
    { 0x030046F0, 7 },
    { 0x03003670, 9 },
    { 0x03003250, 7 },
    { 0x030046F0, 7 },
    { 0x03003A90, 6 },
    { 0x03003250, 7 },
    { 0x030046F0, 7 },
    { 0x00000000, 0 },
    { 0x03001570, 2 },
    { (u32) demoPieceTex3, 7 },
    { 0x03001570, 3 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex2, 7 },
    { 0x03001570, 2 },
    { (u32) demoPieceTex0, 7 },
    { (u32) demoPieceTex2, 8 },
    { 0x00000000, 0 },
    { (u32) demoPieceTex4, 40 },
    { (u32) demoPieceTex2, 7 },
    { (u32) demoPieceTex1, 7 },
    { (u32) demoPieceTex0, 70 },
    { (u32) demoPieceTex4, 10 },
    { (u32) demoPieceTex4, 10 },
    { (u32) demoPieceTex0, 10 },
    { (u32) demoPieceTex3, 200 },
    { 0x00000000, 0 },
    { (u32) demoPieceTex19, 60 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 10 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 40 },
    { 0x03004B10, 7 },
    { (u32) demoPieceTex19, 40 },
    { 0x00000000, 0 },
    { (u32) utaSet0F3Tex0, 8 },
    { (u32) demoFaceTex1, 6 },
    { 0x04000C60, 5 },
    { 0x04001080, 5 },
    { 0x04001080, 8 },
    { 0x04000C60, 6 },
    { (u32) demoFaceTex1, 7 },
    { (u32) utaSet0F3Tex0, 8 },
    { 0x00000000, 0 },
    { 0x0300A5D0, 13 },
    { 0x00000000, 0 },
    { 0x040018C0, 13 },
    { 0x00000000, 0 },
    { 0x03007030, 2 },
    { 0x03006C10, 7 },
    { 0x03005FB0, 570 },
    { 0x00000000, 0 },
    { 0x03005B90, 7 },
    { 0x030063D0, 7 },
    { 0x03007030, 7 },
    { 0x03005FB0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x03007030, 7 },
    { 0x030063D0, 7 },
    { 0x030067F0, 7 },
    { 0x03006C10, 7 },
    { 0x03007030, 7 },
    { 0x03005FB0, 7 },
    { 0x030063D0, 7 },
    { 0x03005FB0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x030063D0, 7 },
    { 0x03007030, 7 },
    { 0x03005B90, 7 },
    { 0x03007030, 7 },
    { 0x00000000, 0 },
    { 0x03007450, 7 },
    { 0x03007C90, 7 },
    { 0x030088F0, 7 },
    { 0x03007870, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x030088F0, 7 },
    { 0x03007C90, 7 },
    { 0x030080B0, 7 },
    { 0x030084D0, 7 },
    { 0x030088F0, 7 },
    { 0x03007870, 7 },
    { 0x03007C90, 7 },
    { 0x03007870, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x03007C90, 7 },
    { 0x030088F0, 7 },
    { 0x03007450, 7 },
    { 0x030088F0, 7 },
    { 0x00000000, 0 },
    { 0x03008D10, 7 },
    { 0x03009550, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009130, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009550, 7 },
    { 0x03009970, 7 },
    { 0x03009D90, 7 },
    { 0x0300A1B0, 7 },
    { 0x03009130, 7 },
    { 0x03009550, 7 },
    { 0x03009130, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x03009550, 7 },
    { 0x0300A1B0, 7 },
    { 0x03008D10, 7 },
    { 0x0300A1B0, 7 },
    { 0x00000000, 0 },
    { 0x04000C60, 8 },
    { 0x04001080, 6 },
    { 0x04001CE0, 5 },
    { 0x04002520, 5 },
    { 0x04002520, 8 },
    { 0x04001CE0, 6 },
    { 0x04001080, 7 },
    { 0x04000C60, 8 },
    { 0x00000000, 0 },
    { 0x040018C0, 13 },
    { 0x00000000, 0 },
    { 0x040014A0, 13 },
    { 0x00000000, 0 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300AE10, 4 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300B650, 4 },
    { 0x0300BA70, 2 },
    { 0x0300AE10, 4 },
    { 0x0300B230, 4 },
    { 0x0300AE10, 4 },
    { 0x0300A9F0, 3 },
    { 0x0300B230, 4 },
    { 0x0300A9F0, 3 },
    { 0x00000000, 0 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300C6D0, 4 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300CF10, 4 },
    { 0x0300D330, 2 },
    { 0x0300C6D0, 4 },
    { 0x0300CAF0, 4 },
    { 0x0300C6D0, 4 },
    { 0x0300C2B0, 3 },
    { 0x0300CAF0, 4 },
    { 0x0300C2B0, 3 },
    { 0x00000000, 0 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300DF90, 4 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300E7D0, 4 },
    { 0x0300EBF0, 2 },
    { 0x0300DF90, 4 },
    { 0x0300E3B0, 4 },
    { 0x0300DF90, 4 },
    { 0x0300DB70, 3 },
    { 0x0300E3B0, 4 },
    { 0x0300DB70, 3 },
    { 0x00000000, 0 },
    { (u32) demoFaceTex1, 52 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 52 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 5 },
    { 0x04000C60, 1 },
    { 0x040014A0, 2 },
    { 0x04002100, 4 },
    { (u32) demoFaceTex1, 42 },
    { 0x00000000, 0 },
    { (u32) demoFaceTex1, 10 },
    { 0x04000C60, 10 },
    { 0x040014A0, 400 },
    { 0x040018C0, 3 },
    { 0x04001080, 3 },
    { (u32) demoFaceTex2, 3 },
    { 0x04001080, 3 },
    { 0x040018C0, 3 },
    { 0x04002520, 3 },
    { 0x04002520, 3 },
    { 0x00000000, 0 },
    { (u32) demoPieceTex78, 13 },
    { 0x00000000, 0 },
    { 0x030146B0, 13 },
    { 0x00000000, 0 },
    { 0x0300F850, 6 },
    { 0x03010CF0, 4 },
    { 0x03010090, 8 },
    { 0x03010CF0, 4 },
    { 0x0300FC70, 10 },
    { 0x03010CF0, 4 },
    { 0x0300F850, 12 },
    { 0x03010090, 8 },
    { 0x030104B0, 8 },
    { 0x03010CF0, 4 },
    { 0x030108D0, 8 },
    { 0x03010CF0, 3 },
    { 0x00000000, 0 },
    { 0x03011110, 4 },
    { 0x030125B0, 2 },
    { 0x03011950, 5 },
    { 0x030125B0, 2 },
    { 0x03011530, 6 },
    { 0x030125B0, 2 },
    { 0x03011110, 6 },
    { 0x03011950, 5 },
    { 0x03011D70, 5 },
    { 0x030125B0, 4 },
    { 0x03012190, 2 },
    { 0x030125B0, 2 },
    { 0x00000000, 0 },
    { 0x030125B0, 2 },
    { 0x03012190, 15 },
    { 0x03011950, 15 },
    { 0x03011530, 16 },
    { 0x03011950, 15 },
    { 0x03011530, 400 },
    { 0x00000000, 0 },
    { 0x030125B0, 2000 },
    { 0x00000000, 0 },
    { 0x030129D0, 4 },
    { 0x03013E70, 2 },
    { 0x03013210, 5 },
    { 0x03013E70, 2 },
    { 0x03012DF0, 6 },
    { 0x03013E70, 2 },
    { 0x030129D0, 6 },
    { 0x03013210, 5 },
    { 0x03013630, 5 },
    { 0x03013E70, 4 },
    { 0x03013A50, 2 },
    { 0x03013E70, 2 },
    { 0x00000000, 0 },
    { 0x03014AD0, 2000 },
    { 0x00000000, 0 },
};
AnimFrame utaOmiFaceAnim0[] = {
    { (u32) utaOmiFaceTex, 1 },
    { 0x030059C0, 1 },
    { 0x03005DE0, 1 },
    { 0x03006200, 1 },
    { 0x03006620, 1 },
    { 0x03006A40, 1 },
    { 0, 0 },
};
AnimFrame utaOmiFaceAnim[] = {
    { (u32) utaOmiFaceTex, 40 },
    { 0x03006E60, 3 },
    { 0x03007280, 3 },
    { 0x030076A0, 700 },
    { 0, 0 },
};
AnimFrame utaOmiFaceAnim2[] = {
    { 0x030076A0, 50 },
    { 0x03007280, 3 },
    { 0x03006E60, 3 },
    { (u32) utaOmiFaceTex, 700 },
    { 0, 0 },
};
AnimFrame utaOmiFaceAnim3[] = {
    { 0x03006A40, 16 },
    { 0x030059C0, 2 },
    { 0x03006620, 700 },
    { 0x00000000, 0 },
};

/* Steps an animation by `speed` and returns 1 when it passes an end. Modes: 0 forward, looping;
 * 2 forward, wrapping at the last frame itself; 1 backward, looping; 3 backward, stopping at 0. */
s32 utaAnimeStep(AnimeFrame *a, f32 speed, s32 mode) {
    s32 last;

    if (mode == 0 || mode == 2) {
        a->frame += speed;
        last = (mode == 0) ? polAnimeLastFrame(a) + 1 : polAnimeLastFrame(a);
        if (last < a->frame) {
            a->frame -= last;
            return 1;
        }
    } else if (mode == 1 || mode == 3) {
        a->frame -= speed;
        if (a->frame < 0.0f) {
            if (mode == 1) {
                a->frame += polAnimeLastFrame(a);
            } else {
                a->frame = 0.0f;
            }
            return 1;
        }
    }
    return 0;
}

/* A demo character's work (tlb_0029_2's DemoPlW). */
typedef struct DemoPlW {
    /* 0x00 */ Obj *pl;
    /* 0x04 */ AnimeFrame *model;
    /* 0x08 */ PolFrame *shadow;
    /* 0x0C */ Anim eyes;
    /* 0x1C */ Anim mouth;
    /* 0x2C */ s32 chara;
    /* 0x30 */ s32 frames; /* counted each message */
    /* 0x34 */ s32 state;
} DemoPlW;

#define DPRINTF

extern s32 utaDemoFrame; /* the song's frame count */
extern f32 utaGoeUnusedF;
extern Player *utaGoePlayer; /* Goemon's player */
extern u8 utaShadowDl[]; /* the shadow */
extern u8 gDemoCamHold;
extern s32 gObjMoveSel;
extern f32 gObjMoveDx;
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern f32 gObjMovePosX;
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;

/* "UTAGOE", Goemon in the singing demo (his Player kept in utaGoePlayer): OBJMOVE's model 0 moves
 * him and his shadow follows him. He starts out of sight; hidden to 303, dancing to 335, waiting
 * below to 512, shown with actions 0x117-0x119, his faces changing on his animation's frames 4
 * and 8; at 980 he drops out of sight, at 1069 he returns with action 0x11A and dances to 1160;
 * at 1850 he is set at (-109, 0, 136) and drifts back 2 a frame, dropping away at 2086; at 2647
 * he returns at the origin with action 0x11C, then 0x11D at 2745 and 0x11E at 2810 (his shadow
 * shown, moved off to x 10000), and keeps dancing. */
s32 utaGoeProc(Obj *obj, s16 msg, void *arg);
s32 utaGoeProc(Obj *obj, s16 msg, void *arg) {
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Obj *pl;
    Player *p;
    PlInit init;

    w->frames++;
    switch (msg) {
        case 2:
            obj->name = "UTAGOE";
            plMakeInit(&init, 0, 0);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 0.0f;
            init.side = 0.0f;
            init.height = -10000.0f;
            utaGoeUnusedF = 0.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            PRINTF("デモモード突入\n");
            /* One line (as a macro's expansion would be; IDO_NOTES 12): apart, as1 orders them otherwise. */
            pl = w->pl; utaGoePlayer = p; plEnterDemo(pl);
            DPRINTF("wao\n");
            plAnimeSet(PL(w->pl), 0x116);
            DPRINTF("ani %x\n", 0x116);
            demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
            demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
            p = utaGoePlayer;
            w->model = p->model;
            PRINTF("*** p->model_buff=%x\n", p->model);
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("ゴエモン作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) utaShadowDl, 0xBC);
            w->shadow->scale[2] = 1.0f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->state = 0;
            w->chara = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = -10000.0f;
            w->model->pos[2] = 0.0f;
            w->model->rot[1] = 0.0f;
            DPRINTF("ゴエモン作りました\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 0);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 0) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 1.0f;
            w->shadow->f.flags &= ~2;
            switch (w->state) {
                case 0:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 303) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->model->f.flags |= 2;
                    w->model->pos[1] = 0.0f;
                    demoAnimeStep(w->model, 1.0f, 2);
                    if (utaDemoFrame == 335) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->model->pos[1] = -5000.0f;
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 512) {
                        plAnimeSet(PL(w->pl), 0x117);
                        w->state++;
                        w->model->pos[1] = 0.0f;
                    }
                    break;
                case 3:
                    (*utaAnimeStep)(w->model, 0.4f, 0);
                    if (utaDemoFrame == 702) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0x118);
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.4f, 2);
                    switch ((s32) w->model->frame) {
                        case 4:
                            demoTexAnimK0(&w->eyes, 8, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 8, &PL(w->pl)->model->segs[4]);
                            break;
                        case 8:
                            demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                            demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                            break;
                    }
                    if (utaDemoFrame == 800) {
                        w->state++;
                        w->model->scale[2] = 5.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        plAnimeSet(PL(w->pl), 0x119);
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 2);
                    if (utaDemoFrame == 980) {
                        w->state++;
                        w->model->pos[1] = -10000.0f;
                    }
                    break;
                case 6:
                    if (utaDemoFrame == 1069) {
                        w->model->pos[0] = 0.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 0.0f;
                        plAnimeSet(PL(w->pl), 0x11A);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 9, &PL(w->pl)->model->segs[4]);
                        w->model->scale[2] = 1.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->state++;
                    }
                    break;
                case 7:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (utaDemoFrame == 1160) {
                        w->state++;
                    }
                    break;
                case 8:
                    w->model->pos[1] = -10000.0f;
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 1850) {
                        plAnimeSet(PL(w->pl), 0x11B);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 5, &PL(w->pl)->model->segs[4]);
                        w->model->f.flags |= 2;
                        w->model->pos[0] = -109.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 136.0f;
                        w->model->rot[1] = 0.34906587f;
                        w->state++;
                    }
                    break;
                case 9:
                    w->model->pos[2] -= 2.0f;
                    if (utaDemoFrame == 2086) {
                        w->model->pos[1] = -10000.0f;
                        w->state++;
                    }
                    break;
                case 10:
                    if (utaDemoFrame == 2647) {
                        plAnimeSet(PL(w->pl), 0x11C);
                        demoTexAnimK0(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 0.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 0.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.14f, 2);
                    if (utaDemoFrame == 2745) {
                        plAnimeSet(PL(w->pl), 0x11D);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 9, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 12:
                    demoAnimeStep(w->model, 0.05f, 2);
                    if (utaDemoFrame == 2810) {
                        w->shadow->f.flags |= 2;
                        w->model->pos[0] = 10000.0f;
                        plAnimeSet(PL(w->pl), 0x11E);
                        demoTexAnimK0(&w->eyes, 5, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK0(&w->mouth, 10, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 13:
                    demoAnimeStep(w->model, 0.2f, 2);
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 utaGoeProc(Obj *obj, s16 msg, void *arg);

Obj *utaGoeCreate(Obj *obj) {
    return tskCreate(obj, utaGoeProc, obj->file, NULL);
}

extern f32 utaEbiX; /* Ebisumaru's x and z, each frame */
extern f32 utaEbiZ;

/* "UTAEBI", Ebisumaru in the singing demo: OBJMOVE's model 1 moves him and his shadow follows
 * him. He shows and hides with the song (hidden to 366, shown to 393, hidden to 512, shown with
 * actions 0x117-0x119 to 1030, hidden to 1049, shown again); at 1850 he is set at (197, 90,
 * -332) and drifts back 2 a frame, at 2086 he drops out of sight, at 2647 he returns at the
 * origin with action 0x11C and his face animations, and at 2715 he is gone. */
s32 utaEbiProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    w->frames++;
    switch (msg) {
        case 2:
            obj->name = "UTAEBI";
            plMakeInit(&init, 0, 1);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 0.0f;
            init.height = 0.0f;
            init.side = 0.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0x116);
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) utaShadowDl, 0xBC);
            w->shadow->scale[2] = 1.0f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 1;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 0.0f;
            w->model->rot[1] = 0.0f;
            DPRINTF("エビス丸作りました\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 128);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 1) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 1.0f;
            utaEbiX = w->model->pos[0];
            utaEbiZ = w->model->pos[2];
            w->shadow->f.flags &= ~2;
            switch (w->state) {
                case 0:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 366) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->model->f.flags |= 2;
                    if (utaDemoFrame == 393) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 512) {
                        plAnimeSet(PL(w->pl), 0x117);
                        w->state++;
                    }
                    break;
                case 3:
                    w->model->f.flags |= 2;
                    (*utaAnimeStep)(w->model, 0.4f, 0);
                    if (utaDemoFrame == 712) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0x118);
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (utaDemoFrame == 800) {
                        w->state++;
                        w->model->scale[2] = 5.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        plAnimeSet(PL(w->pl), 0x119);
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 2);
                    if (utaDemoFrame == 980) {
                        w->state++;
                    }
                    break;
                case 6:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 1030) {
                        plAnimeSet(PL(w->pl), 0x11A);
                        w->model->scale[2] = 1.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->state++;
                    }
                    break;
                case 7:
                    w->model->f.flags |= 2;
                    if (utaDemoFrame == 1049) {
                        w->state++;
                    }
                    break;
                case 8:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 1850) {
                        plAnimeSet(PL(w->pl), 0x11B);
                        w->model->f.flags |= 2;
                        w->model->pos[0] = 197.0f;
                        w->model->pos[1] = 90.0f;
                        w->model->pos[2] = -332.0f;
                        w->model->rot[1] = 0.40142575f;
                        w->state++;
                    }
                    break;
                case 9:
                    w->model->pos[2] -= 2.0f;
                    if (utaDemoFrame == 2086) {
                        w->model->pos[1] = -10000.0f;
                        w->state++;
                    }
                    break;
                case 10:
                    if (utaDemoFrame == 2647) {
                        plAnimeSet(PL(w->pl), 0x11C);
                        demoTexAnimK1(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK1(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 0.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 0.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.14f, 2);
                    if (utaDemoFrame == 2715) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 utaEbiProc(Obj *obj, s16 msg, void *arg);

Obj *utaEbiCreate(Obj *obj) {
    return tskCreate(obj, utaEbiProc, obj->file, NULL);
}

extern f32 utaSasX; /* Sasuke's x and z, each frame */
extern f32 utaSasZ;

/* "SAS", Sasuke in the singing demo: OBJMOVE's model 2 moves him and his shadow follows him. He
 * shows and hides with the song (hidden to 393, shown with actions 0x117-0x119, his faces
 * changing on his animation's frame 4 and at 980, hidden at 1011, shown from 1030); at 1850 he
 * is set at (775, 0, -219) and drifts back 2 a frame, at 2086 he drops out of sight, at 2647 he
 * returns at the origin with action 0x11C and his faces, and at 2715 he is gone. */
s32 utaSasProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    w->frames++;
    switch (msg) {
        case 2:
            obj->name = "SAS";
            plMakeInit(&init, 0, 2);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 0.0f;
            init.height = 0.0f;
            init.side = 0.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0x116);
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("サスケ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) utaShadowDl, 0xBC);
            w->shadow->scale[2] = 1.0f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 2;
            w->state = 0;
            w->model->rot[1] = 0.0f;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 0.0f;
            w->model->rot[1] = 0.0f;
            DPRINTF("サスケ作りました\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 128);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 2) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->shadow->f.flags &= ~2;
            utaSasX = w->model->pos[0];
            utaSasZ = w->model->pos[2];
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 1.0f;
            switch (w->state) {
                case 0:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 393) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->model->f.flags |= 2;
                    if (utaDemoFrame == 512) {
                        plAnimeSet(PL(w->pl), 0x117);
                        w->state++;
                    }
                    break;
                case 2:
                    w->model->f.flags |= 2;
                    (*utaAnimeStep)(w->model, 0.52f, 0);
                    if (utaDemoFrame == 702) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0x118);
                    }
                    break;
                case 3:
                    w->shadow->f.flags |= 2;
                    if ((s32) w->model->frame == 4) {
                        demoTexAnimK2(&w->eyes, 3, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 3, &PL(w->pl)->model->segs[4]);
                    }
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (utaDemoFrame == 800) {
                        w->state++;
                        w->model->scale[2] = 5.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        plAnimeSet(PL(w->pl), 0x119);
                    }
                    break;
                case 4:
                    demoAnimeStep(w->model, 0.2f, 2);
                    if (utaDemoFrame == 980) {
                        w->state++;
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 5:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 1011) {
                        w->model->scale[2] = 1.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        plAnimeSet(PL(w->pl), 0x11A);
                        w->model->f.flags |= 2;
                        w->state++;
                    }
                    break;
                case 6:
                    w->model->f.flags |= 2;
                    if (utaDemoFrame == 1030) {
                        w->state++;
                    }
                    break;
                case 7:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 1850) {
                        plAnimeSet(PL(w->pl), 0x11B);
                        w->model->f.flags |= 2;
                        w->model->pos[0] = 775.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = -219.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 8:
                    w->model->pos[2] -= 2.0f;
                    if (utaDemoFrame == 2086) {
                        w->model->pos[1] = -10000.0f;
                        w->state++;
                    }
                    break;
                case 9:
                    if (utaDemoFrame == 2647) {
                        plAnimeSet(PL(w->pl), 0x11C);
                        demoTexAnimK2(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK2(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 0.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 0.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 10:
                    demoAnimeStep(w->model, 0.14f, 2);
                    if (utaDemoFrame == 2715) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 utaSasProc(Obj *obj, s16 msg, void *arg);

Obj *utaSasCreate(Obj *obj) {
    return tskCreate(obj, utaSasProc, obj->file, NULL);
}

/* "YAE", Yae in the singing demo: OBJMOVE's model 3 moves her and her shadow follows her. She
 * shows and hides with the song (hidden to 335, dancing to 366, hidden to 512, shown with actions
 * 0x117-0x119, her faces changing on her animation's frame 4 and at 980; action 0x11A with her
 * mouth's tenth animation at 1049, shown at 1069); at 1850 she is set at (494, -4, -86) and
 * drifts back 2 a frame, at 2086 she drops out of sight, at 2647 she returns at the origin with
 * action 0x11C, and at 2715 she is gone. */
s32 utaYaeProc(Obj *obj, s16 msg, void *arg) {
    s32 pad[2];
    DemoPlW *w = (DemoPlW *) (obj + 1);
    Player *p;
    PlInit init;

    w->frames++;
    switch (msg) {
        case 2:
            obj->name = "YAE";
            plMakeInit(&init, 0, 3);
            init.flags |= 0x90000040;
            init.zone = 0;
            init.place = 0.0f;
            init.height = 0.0f;
            init.side = 0.0f;
            w->pl = plCreate(gDmenuLastObj, &init);
            p = PL(w->pl);
            plEnterDemo(w->pl);
            DPRINTF("デモモード突入\n");
            plAnimeSet(PL(w->pl), 0x116);
            w->model = p->model;
            ((Frame *) p->shadow)->flags &= ~2;
            DPRINTF("ヤエ作ります\n");
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoPolSetDL(w->shadow, (Gfx *) utaShadowDl, 0xBC);
            w->shadow->scale[2] = 1.0f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 1.0f;
            w->chara = 3;
            w->state = 0;
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->model->pos[0] = 0.0f;
            w->model->pos[1] = 0.0f;
            w->model->pos[2] = 0.0f;
            w->model->rot[1] = 0.0f;
            demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
            demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
            DPRINTF("ヤエ作りました\n");
            break;
        case 1:
            frmDestroy(&w->model->f);
            frmDestroy(&w->shadow->f);
            tskDestroy(w->pl);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 128);
            break;
        case 0:
            w->model->f.flags |= 2;
            if (gObjMoveSel == 3) {
                w->model->pos[0] += gObjMoveDx;
                w->model->pos[1] += gObjMoveDy;
                w->model->pos[2] += gObjMoveDz;
                w->model->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->model->pos[0];
                gObjMovePosY = w->model->pos[1];
                gObjMovePosZ = w->model->pos[2];
                gObjMoveRot = w->model->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->model->f.flags |= 2;
                } else {
                    w->model->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->shadow->pos[0] = w->model->pos[0];
            w->shadow->pos[2] = w->model->pos[2];
            w->shadow->pos[1] = w->model->pos[1] + 1.0f;
            w->shadow->f.flags &= ~2;
            switch (w->state) {
                case 0:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 335) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->model->f.flags |= 2;
                    if (utaDemoFrame == 366) {
                        w->state++;
                    }
                    demoAnimeStep(w->model, 1.0f, 2);
                    break;
                case 2:
                    w->model->f.flags &= ~2;
                    if (utaDemoFrame == 512) {
                        plAnimeSet(PL(w->pl), 0x117);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->state++;
                    }
                    break;
                case 3:
                    w->model->f.flags |= 2;
                    (*utaAnimeStep)(w->model, 0.4f, 0);
                    if (utaDemoFrame == 712) {
                        w->state++;
                        plAnimeSet(PL(w->pl), 0x118);
                    }
                    break;
                case 4:
                    if ((s32) w->model->frame == 4) {
                        demoTexAnimK3(&w->eyes, 1, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 1, &PL(w->pl)->model->segs[4]);
                    }
                    demoAnimeStep(w->model, 0.4f, 2);
                    if (utaDemoFrame == 800) {
                        w->state++;
                        w->model->scale[2] = 5.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        plAnimeSet(PL(w->pl), 0x119);
                    }
                    break;
                case 5:
                    demoAnimeStep(w->model, 0.2f, 2);
                    if (utaDemoFrame == 980) {
                        w->state++;
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                    }
                    break;
                case 6:
                    if (utaDemoFrame == 1049) {
                        plAnimeSet(PL(w->pl), 0x11A);
                        demoTexAnimK3(&w->mouth, 10, &PL(w->pl)->model->segs[4]);
                        w->model->scale[2] = 1.0f;
                        w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                        w->model->f.flags |= 2;
                        w->state++;
                    }
                    w->model->f.flags &= ~2;
                    break;
                case 7:
                    w->model->f.flags |= 2;
                    if (utaDemoFrame == 1069) {
                        w->state++;
                    }
                    break;
                case 8:
                    w->model->pos[1] = -10000.0f;
                    if (utaDemoFrame == 1850) {
                        plAnimeSet(PL(w->pl), 0x11B);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->f.flags |= 2;
                        w->model->pos[0] = 494.0f;
                        w->model->pos[1] = -4.0f;
                        w->model->pos[2] = -86.0f;
                        w->model->rot[1] = -1.2915437f;
                        w->state++;
                    }
                    break;
                case 9:
                    w->model->pos[2] -= 2.0f;
                    if (utaDemoFrame == 2086) {
                        w->model->pos[1] = -10000.0f;
                        w->state++;
                    }
                    break;
                case 10:
                    if (utaDemoFrame == 2647) {
                        plAnimeSet(PL(w->pl), 0x11C);
                        demoTexAnimK3(&w->eyes, 0, &PL(w->pl)->model->segs[3]);
                        demoTexAnimK3(&w->mouth, 0, &PL(w->pl)->model->segs[4]);
                        w->model->pos[0] = 0.0f;
                        w->model->pos[1] = 0.0f;
                        w->model->pos[2] = 0.0f;
                        w->model->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 11:
                    demoAnimeStep(w->model, 0.14f, 2);
                    if (utaDemoFrame == 2715) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 utaYaeProc(Obj *obj, s16 msg, void *arg);

Obj *utaYaeCreate(Obj *obj) {
    return tskCreate(obj, utaYaeProc, obj->file, NULL);
}

/* "omi"'s work (Omitsu): her model, her shadow and her face's texture animation. */
typedef struct OmiW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ PolFrame *shadow;
    /* 0x08 */ Anim anim;
    /* 0x18 */ u8 pad18[0x14];
    /* 0x2C */ s32 state;
} OmiW;

extern u8 gDemoCamHold;
extern s32 gObjMoveSel;
extern f32 gObjMovePosX;
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx;
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern s32 utaDemoFrame; /* the demo's frame count */
extern u8 utaShadowDl[]; /* the shadow */
extern u8 utaOmiAnim[]; /* her animations, in file 0xC7 */
extern u8 utaOmiState0Anim[];
extern u8 utaOmiState2Anim[];
extern u8 utaOmiState3Anim[];
extern AnimFrame utaOmiFaceAnim[]; /* her face's animations (tlb_0033_3's data) */
extern AnimFrame utaOmiFaceAnim2[];
extern AnimFrame utaOmiFaceAnim0[];
extern AnimFrame utaOmiFaceAnim3[];

/* "omi" (Omitsu, file 0xC7) in the demo, built as tlb_0023_1's "kuro": OBJMOVE's model 4 moves
 * her and her shadow follows her. On the demo's frame count she changes animation and face at
 * 2086, is put out of sight at 2397, comes back at the origin at 2715, changes again at 2780,
 * and ends at 2810. */
s32 utaOmiProc(Obj *obj, s16 msg, void *arg) {
    OmiW *w = (OmiW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "omi";
            PRINTF("おみっちゃん作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->shadow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) utaOmiAnim, 0xC7, 0.0f);
            demoPolSetDL(w->shadow, (Gfx *) utaShadowDl, 0xBC);
            w->shadow->scale[2] = 0.6f;
            w->shadow->scale[0] = w->shadow->scale[2];
            w->shadow->scale[1] = 0.6f;
            w->state = 0;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->frame = 3.0f;
            w->frame->rot[1] = 0.0f;
            fadeAnimStart(&w->anim, utaOmiFaceAnim, 0, &w->frame->segs[3], floadGetSetFile(0xC7, 0));
            PRINTF("おみっちゃん作りました\n");
            break;
        case 1:
            frmDestroy((Frame *) w->frame);
            frmDestroy((Frame *) w->shadow);
            break;
        case 3:
            w->shadow->dl = demoColorDL(1, 1, 1, 1, 1, 1, 100);
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 4) {
                w->frame->pos[0] += gObjMoveDx;
                w->frame->pos[1] += gObjMoveDy;
                w->frame->pos[2] += gObjMoveDz;
                w->frame->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->frame->pos[0];
                gObjMovePosY = w->frame->pos[1];
                gObjMovePosZ = w->frame->pos[2];
                gObjMoveRot = w->frame->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->frame->f.flags |= 2;
                } else {
                    w->frame->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            w->shadow->f.flags &= ~2;
            w->shadow->pos[0] = w->frame->pos[0];
            w->shadow->pos[2] = w->frame->pos[2];
            w->shadow->pos[1] = w->frame->pos[1] + 1.0f;
            switch (w->state) {
                case 0:
                    demoAnimeStep(w->frame, 0.06f, 2);
                    fadeAnimStep(&w->anim);
                    if (utaDemoFrame == 2086) {
                        w->state++;
                        demoAnimeSet(w->frame, (u32) utaOmiState0Anim, 0xC7, 0.0f);
                        fadeAnimStart(&w->anim, utaOmiFaceAnim2, 0, &w->frame->segs[3],
                                      floadGetSetFile(0xC7, 0));
                    }
                    break;
                case 1:
                    demoAnimeStep(w->frame, 0.1f, 2);
                    fadeAnimStep(&w->anim);
                    if (utaDemoFrame == 2397) {
                        w->frame->pos[1] = -10000.0f;
                        w->state++;
                    }
                    break;
                case 2:
                    if (utaDemoFrame == 2715) {
                        demoAnimeSet(w->frame, (u32) utaOmiState2Anim, 0xC7, 0.0f);
                        fadeAnimStart(&w->anim, utaOmiFaceAnim0, 2, &w->frame->segs[3],
                                      floadGetSetFile(0xC7, 0));
                        w->frame->pos[0] = 0.0f;
                        w->frame->pos[1] = 0.0f;
                        w->frame->pos[2] = 0.0f;
                        w->frame->rot[1] = 0.0f;
                        w->state++;
                    }
                    break;
                case 3:
                    demoAnimeStep(w->frame, 0.2f, 2);
                    if (utaDemoFrame == 2780) {
                        demoAnimeSet(w->frame, (u32) utaOmiState3Anim, 0xC7, 0.0f);
                        fadeAnimStart(&w->anim, utaOmiFaceAnim3, 0, &w->frame->segs[3],
                                      floadGetSetFile(0xC7, 0));
                        w->state++;
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->anim);
                    demoAnimeStep(w->frame, 0.2f, 2);
                    if (utaDemoFrame == 2810) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaOmiCreate(Obj *obj) {
    return tskCreate(obj, utaOmiProc, obj->file, NULL);
}

#define DPRINTF

extern u8 gDemoCamHold;
extern s32 gObjMoveSel; /* OBJMOVE: the model chosen, -1 for none */
extern f32 gObjMovePosX; /* the chosen model's position and angle, as shown */
extern f32 gObjMovePosY;
extern f32 gObjMovePosZ;
extern f32 gObjMoveRot;
extern f32 gObjMoveDx; /* the moves this frame: x, y, z, angle */
extern f32 gObjMoveDy;
extern f32 gObjMoveDz;
extern f32 gObjMoveDrot;
extern s32 utaDemoFrame; /* the demo's frame count */
extern u8 utaKamomeAnim[];          /* the gull's animation */

/* "kamome"'s work: one anime frame and its state. */
typedef struct KamomeW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad04[0x10];
    /* 0x14 */ s32 state; /* 0 waiting, 1 flying */
} KamomeW;

/* "kamome": a gull (anime of file 0xD2) at (0, 1264, 1023); OBJMOVE's model 6 moves it. From
 * demo frame 0x8A3 it flaps, and at 0x95D it goes. tlb_0027_1's "MACH12" (dochukiDemoMach12Proc)
 * is its twin. */
s32 utaKamomeProc(Obj *obj, s16 msg, void *arg) {
    KamomeW *w = (KamomeW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "kamome";
            w->frame = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            demoAnimeSet(w->frame, (u32) utaKamomeAnim, 0xD2, 0.0f);
            w->state = 0;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 1264.0f;
            w->frame->pos[2] = 1023.0f;
            w->frame->rot[1] = 0.0f;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            break;
        case 0:
            w->frame->f.flags |= 2;
            if (gObjMoveSel == 6) {
                w->frame->pos[0] += gObjMoveDx;
                w->frame->pos[1] += gObjMoveDy;
                w->frame->pos[2] += gObjMoveDz;
                w->frame->rot[1] += gObjMoveDrot;
                gObjMovePosX = w->frame->pos[0];
                gObjMovePosY = w->frame->pos[1];
                gObjMovePosZ = w->frame->pos[2];
                gObjMoveRot = w->frame->rot[1];
                if (gGameFrameCount % 20 >= 10) {
                    w->frame->f.flags |= 2;
                } else {
                    w->frame->f.flags &= ~2;
                }
            }
            if (gDemoCamHold) {
                return tskDefaultProc(obj, msg, arg);
            }
            switch (w->state) {
                case 0:
                    if (utaDemoFrame >= 0x8A3) {
                        w->state++;
                    }
                    break;
                case 1:
                    demoAnimeStep(w->frame, 0.21315789f, 2);
                    DPRINTF("kamome anistep --> %f\r");
                    if (utaDemoFrame == 0x95D) {
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

Obj *utaKamomeCreate(Obj *obj) {
    DPRINTF("かもめ作ります\n");
    return tskCreate(obj, utaKamomeProc, obj->file, NULL);
}
