#include "acoll.h"
#include "aniatk.h"
#include "gfxmgr.h"
#include "mathutil.h"

/* Attack and damage boxes ("acoll"): the frame kinds "atk " and "dmg ", and their tests.
 * - A box is a frame that follows a model frame (a "pol" or "mpol" frame, or an animated
 *   attack chain of the "aniAtk" kind). It takes the model's place, rotation and scale, plus
 *   its own offset turned with the model if flag 8 is set.
 * - A box is a sphere (height 0) or an upright cylinder, radius and height scaled by the
 *   model's x and y scale.
 * - Attacks live under the attack root in player, enemy and "uda" roots, and damage boxes
 *   the same under the damage root (attack.c tests them each step).
 * - On a hit, both boxes keep a record of the other (flag 4), and a damage box with a push
 *   point is pushed to the attack's edge. An attack on an animated chain marks every link. */

/* The kind makers' debug print, compiled to nothing but its arguments (as pakfiler.c's): the
 * strings stay in .rodata, and no dead branch is left to move the registers. */
#define DPRINTF

typedef struct PolFrame2 {
    /* 0x00 */ Frame frame;
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ f32 rot[3];
    /* 0x2C */ f32 scale[3];
} PolFrame2;

extern s16 gFrmKindCore; /* the "null" kind (roots) */
extern s16 gFrmKindPol; /* "pol" */
extern s16 gFrmKindAnime; /* "mpol" */
extern s16 FRM_Atk; /* "atk " */
extern ACollFrame *atk_root_frame; /* the attack root, and its player, enemy and uda roots */
extern ACollFrame *pl_atk_root_frame;
extern ACollFrame *ene_atk_root_frame;
extern ACollFrame *uda_atk_root_frame;
extern s16 FRM_Dmg; /* "dmg " */
extern ACollFrame *dmg_root_frame; /* the damage root, and its player, enemy and uda roots */
extern ACollFrame *pl_dmg_root_frame;
extern ACollFrame *ene_dmg_root_frame;
extern ACollFrame *uda_dmg_root_frame;
extern s16 gFrmKindAniAtk; /* "aniAtk " (aniatk.c) */
extern Frame *gAniAtkRoot;

ACollFrame *_atkLinkAdd(Frame *parent, Frame *model, f32 x, f32 y, f32 z, f32 sx,
                                   f32 sy, f32 sz, f32 rx, f32 ry, f32 rz, u16 radius, s16 ox,
                                   s16 oy, s16 oz, s32 unused);

/* Makes both kinds and their roots: 50 frames each, or the counts given. */
s32 acollLinkInit(void) {
    if (_atkLinkInit(50) == -1) {
        PRINTF("acollLinkInit: _atkLinkInit でエラー\n");
        return -1;
    }
    if (_dmgLinkInit(50) == -1) {
        PRINTF("acollLinkInit: _dmgLinkInit でエラー\n");
        return -1;
    }
    return 0;
}

s32 acollLinkInitCounts(s32 atk, s32 dmg) {
    if (_atkLinkInit(atk) == -1) {
        PRINTF("acollLinkInit: _atkLinkInit でエラー\n");
        return -1;
    }
    if (_dmgLinkInit(dmg) == -1) {
        PRINTF("acollLinkInit: _dmgLinkInit でエラー\n");
        return -1;
    }
    return 0;
}

/* Makes the "atk " kind (and the "dmg " kind) and its root with the player, enemy and uda roots
 * under it. Returns the kind, -1 if it could not be made, or 0 if a root could not. */
s16 _atkLinkInit(s32 count) {
    FRM_Atk = frmNewKind("atk ", NULL, 0x80, count, NULL);
    DPRINTF("FRM_Atk を作成\n");
    if (FRM_Atk == -1) {
        return -1;
    }
    if ((atk_root_frame = (ACollFrame *) frmCreateRoot(gFrmKindCore)) == NULL) {
        DPRINTF(">>>> atkLinkInit: atk_root_frame dekinai...\n");
        return 0;
    }
    if ((pl_atk_root_frame = (ACollFrame *) frmCreate((Frame *) atk_root_frame, FRM_Atk)) ==
        NULL) {
        DPRINTF(">>>> atkLinkInit: pl_atk_root_frame dekinai...\n");
        return 0;
    }
    if ((ene_atk_root_frame = (ACollFrame *) frmCreate((Frame *) atk_root_frame, FRM_Atk)) ==
        NULL) {
        DPRINTF(">>>> atkLinkInit: ene_atk_root_frame dekinai...\n");
        return 0;
    }
    if ((uda_atk_root_frame = (ACollFrame *) frmCreate((Frame *) atk_root_frame, FRM_Atk)) ==
        NULL) {
        DPRINTF(">>>> atkLinkInit: uda_atk_root_frame dekinai...\n");
        return 0;
    }
    DPRINTF("atkLinkInit OK\n");
    return FRM_Atk;
}

s16 _dmgLinkInit(s32 count) {
    FRM_Dmg = frmNewKind("dmg ", NULL, 0x84, count, NULL);
    DPRINTF("FRM_Dmg を作成\n");
    if (FRM_Dmg == -1) {
        return -1;
    }
    if ((dmg_root_frame = (ACollFrame *) frmCreateRoot(gFrmKindCore)) == NULL) {
        DPRINTF(">>>> atkLinkInit: dmg_root_frame dekinai...\n");
        return 0;
    }
    if ((pl_dmg_root_frame = (ACollFrame *) frmCreate((Frame *) dmg_root_frame, FRM_Dmg)) ==
        NULL) {
        DPRINTF(">>>> dmgLinkInit: pl_dmg_root_frame dekinai...\n");
        return 0;
    }
    if ((ene_dmg_root_frame = (ACollFrame *) frmCreate((Frame *) dmg_root_frame, FRM_Dmg)) ==
        NULL) {
        DPRINTF(">>>> dmgLinkInit: ene_dmg_root_frame dekinai...\n");
        return 0;
    }
    if ((uda_dmg_root_frame = (ACollFrame *) frmCreate((Frame *) dmg_root_frame, FRM_Dmg)) ==
        NULL) {
        DPRINTF(">>>> dmgLinkInit: uda_dmg_root_frame dekinai...\n");
        return 0;
    }
    DPRINTF("dmgLinkInit OK\n");
    return FRM_Dmg;
}

/* Deletes the roots and the three kinds. */
void acollManagerDelete(void) {
    frmDestroy((Frame *) atk_root_frame);
    frmDestroy((Frame *) dmg_root_frame);
    frmDestroy(gAniAtkRoot);
    if (FRM_Atk != 0) {
        frmDeleteKind(FRM_Atk);
        FRM_Atk = 0;
    }
    if (FRM_Dmg != 0) {
        frmDeleteKind(FRM_Dmg);
        FRM_Dmg = 0;
    }
    if (gFrmKindAniAtk != 0) {
        frmDeleteKind(gFrmKindAniAtk);
        gFrmKindAniAtk = 0;
    }
    PRINTF("acollManagerDelete: acoll マネージャー削除\n");
}

/* Places every attack (and every damage box) under a root after its model; a box with no
 * model is deleted. */
void acollAtkPlaceAll(ACollFrame *root) {
    ACollFrame *f;

    while (TRUE) {
        f = (ACollFrame *) root->frame.child;
        if (f != NULL && (ACollFrame *) f->frame.parent == root) {
            if (f->model != NULL) {
                if (f->model != NULL) {
                    acollAtkPlace(f);
                } else {
                    frmDestroy((Frame *) f);
                }
            }
            if ((f = (ACollFrame *) root->frame.child) != NULL) {
                acollAtkPlaceAll(f);
            }
        }
        f = (ACollFrame *) root->frame.next;
        if (f == NULL || (ACollFrame *) f->frame.prev != root) {
            break;
        }
        if (f->model != NULL) {
            if (f->model != NULL) {
                acollAtkPlace(f);
            } else {
                frmDestroy((Frame *) f);
            }
        }
        if ((f = (ACollFrame *) root->frame.next) == NULL) {
            break;
        }
        root = f;
    }
}

void acollDmgPlaceAll(ACollFrame *root) {
    ACollFrame *f;

    while (TRUE) {
        f = (ACollFrame *) root->frame.child;
        if (f != NULL && (ACollFrame *) f->frame.parent == root) {
            if (f->model != NULL) {
                if (f->model != NULL) {
                    acollDmgPlace(f);
                } else {
                    frmDestroy((Frame *) f);
                }
            }
            if ((f = (ACollFrame *) root->frame.child) != NULL) {
                acollDmgPlaceAll(f);
            }
        }
        f = (ACollFrame *) root->frame.next;
        if (f == NULL || (ACollFrame *) f->frame.prev != root) {
            break;
        }
        if (f->model != NULL) {
            if (f->model != NULL) {
                acollDmgPlace(f);
            } else {
                frmDestroy((Frame *) f);
            }
        }
        if ((f = (ACollFrame *) root->frame.next) == NULL) {
            break;
        }
        root = f;
    }
}

/* Moves the boxes that were hit this step to the end of their lists, last first. */
void acollAtkHitsToEnd(ACollFrame *root) {
    ACollFrame *f = (ACollFrame *) root->frame.child;
    ACollFrame *prev;

    if (f == NULL) {
        return;
    }
    while (f->frame.next != NULL) {
        f = (ACollFrame *) f->frame.next;
    }
    for (; f != NULL; f = prev) {
        prev = (ACollFrame *) f->frame.prev;
        if (f->flags & 1) {
            frmMove((Frame *) f, f->frame.parent);
        }
    }
}

void acollDmgHitsToEnd(ACollFrame *root) {
    ACollFrame *f = (ACollFrame *) root->frame.child;
    ACollFrame *prev;

    if (f == NULL) {
        return;
    }
    while (f->frame.next != NULL) {
        f = (ACollFrame *) f->frame.next;
    }
    for (; f != NULL; f = prev) {
        prev = (ACollFrame *) f->frame.prev;
        if (f->flags & 1) {
            frmMove((Frame *) f, f->frame.parent);
        }
    }
}

/* Places a box after its model. */
void acollAtkPlace(ACollFrame *f) {
    Frame *model;
    s32 pad;
    f32 x, y, z;
    f32 rx, ry, rz;
    f32 sx, sy, sz;
    f32 outx, outy, outz;
    s16 ox, oy, oz;

    model = f->model;
    ox = f->ofs[0];
    oy = f->ofs[1];
    oz = f->ofs[2];
    _FrameToPara(model, &x, &y, &z, &rx, &ry, &rz, &sx, &sy, &sz);
    acollBoxPos(x, y, z, rx, ry, rz, sx, sy, sz, ox, oy, oz, &outx, &outy, &outz,
                           f->flags & 8);
    f->pos[0] = outx;
    f->pos[1] = outy;
    f->pos[2] = outz;
    f->scale[0] = sx;
    f->scale[1] = sy;
    f->scale[2] = sz;
    f->rot[0] = rx;
    f->rot[1] = ry;
    f->rot[2] = rz;
}

/* The same as acollAtkPlace (its twin, instruction for instruction). */
void acollDmgPlace(ACollFrame *f) {
    Frame *model;
    s32 pad;
    f32 x, y, z;
    f32 rx, ry, rz;
    f32 sx, sy, sz;
    f32 outx, outy, outz;
    s16 ox, oy, oz;

    model = f->model;
    ox = f->ofs[0];
    oy = f->ofs[1];
    oz = f->ofs[2];
    _FrameToPara(model, &x, &y, &z, &rx, &ry, &rz, &sx, &sy, &sz);
    acollBoxPos(x, y, z, rx, ry, rz, sx, sy, sz, ox, oy, oz, &outx, &outy, &outz,
                           f->flags & 8);
    f->pos[0] = outx;
    f->pos[1] = outy;
    f->pos[2] = outz;
    f->scale[0] = sx;
    f->scale[1] = sy;
    f->scale[2] = sz;
    f->rot[0] = rx;
    f->rot[1] = ry;
    f->rot[2] = rz;
}

/* Tests every attack under a root against the damage boxes under another. Written recursive on
 * both links (as acollAtkClearHits): IDO -O2 turns the tail call on the sibling into the loop, and
 * the saved registers come out in the game's order. */
void acollCheckAll(ACollFrame *atks, ACollFrame *dmgs, s32 depth) {
    if (atks->frame.child != NULL && (ACollFrame *) atks->frame.child->parent == atks) {
        acollCheckAtk((ACollFrame *) atks->frame.child, dmgs, 0);
        acollCheckAll((ACollFrame *) atks->frame.child, dmgs, depth + 1);
    }
    if (depth == 0) {
        return;
    }
    if (atks->frame.next != NULL && (ACollFrame *) atks->frame.next->prev == atks) {
        acollCheckAtk((ACollFrame *) atks->frame.next, dmgs, 0);
        acollCheckAll((ACollFrame *) atks->frame.next, dmgs, depth + 1);
    }
}

/* Tests one attack against every damage box under a root. Written recursive on both links, as
 * acollCheckAll. */
void acollCheckAtk(ACollFrame *atk, ACollFrame *dmgs, s32 depth) {
    if (dmgs->frame.child != NULL && (ACollFrame *) dmgs->frame.child->parent == dmgs) {
        acollCheckPair(atk, (ACollFrame *) dmgs->frame.child);
        acollCheckAtk(atk, (ACollFrame *) dmgs->frame.child, depth + 1);
    }
    if (depth == 0) {
        return;
    }
    if (dmgs->frame.next != NULL && (ACollFrame *) dmgs->frame.next->prev == dmgs) {
        acollCheckPair(atk, (ACollFrame *) dmgs->frame.next);
        acollCheckAtk(atk, (ACollFrame *) dmgs->frame.next, depth + 1);
    }
}

/* Tests one attack against one damage box, and records a hit on both. The scales are read as
 * Vec3f fields (an array element is loaded before the radius's conversion), and the walk's count
 * is a local (read inline in the loop test, IDO leaves the walk rolled). */
void acollCheckPair(ACollFrame *atk, ACollFrame *dmg) {
    s32 hit;
    ACollFrame *f;
    s32 n;
    s32 pad[2];
    s32 i;
    u8 kind;
    s32 power;
    f32 dx, dy, dz;

    if (atk->model == dmg->model || !(atk->flags & 2) || !(dmg->flags & 2)) {
        return;
    }
    acollRelPos(atk, dmg, &dx, &dy, &dz);
    if ((atk->flags & 0x10) || (dmg->flags & 0x10)) {
        hit = acollHitSimple(dmg->radius * ((Vec3f *) dmg->scale)->x,
                                     dmg->height * ((Vec3f *) dmg->scale)->y, dx, dy, dz,
                                     atk->radius * ((Vec3f *) atk->scale)->x);
    } else {
        hit = acollHitTest(dmg->radius * ((Vec3f *) dmg->scale)->x,
                                     dmg->height * ((Vec3f *) dmg->scale)->y, dx, dy, dz,
                                     atk->radius * ((Vec3f *) atk->scale)->x);
    }
    if (hit != 1) {
        return;
    }
    acollSetPushPoint(dmg, dx, dy, dz);
    if (atk->model->kind == gFrmKindAniAtk) {
        f = atk;
        n = ((AniAtkFrame *) atk->model)->link;
        for (i = 0; i < n; i++) {
            f = (ACollFrame *) f->frame.parent;
        }
        kind = f->kind;
        power = f->power;
        for (i = 0; f != NULL; i++) {
            acollAtkRecordHit(f, dmg);
            f->index = i;
            f->kind = kind;
            f->flags |= 1;
            f->power = power;
            f = (ACollFrame *) f->frame.child;
        }
    } else if (!(atk->flags & 1)) {
        acollAtkRecordHit(atk, dmg);
        atk->index = 0;
        atk->flags |= 1;
    }
    if (!(dmg->flags & 1)) {
        acollDmgRecordHit(dmg, atk);
        dmg->flags |= 1;
    }
}

/* Sphere against sphere (h 0) or cylinder, for an offset (dx, dy, dz) and radius r2. */
s32 acollHitSimple(f32 r, f32 h, f32 dx, f32 dy, f32 dz, f32 r2) {
    f32 sum;

    if (h == 0.0f) {
        sum = r + r2;
        if (sum * sum < dx * dx + dy * dy + dz * dz) {
            return 0;
        }
        return 1;
    }
    sum = r + r2;
    if (sum * sum < dz * dz + dx * dx) {
        return 0;
    }
    if (h < dy - r2 || dy + r2 < 0.0f) {
        return 0;
    }
    return 1;
}

/* Clears the hit flags of every box under a root. Written recursive on both links: IDO -O2
 * turns the tail call on the sibling into the loop. */
void acollAtkClearHits(ACollFrame *root) {
    if (root->frame.child != NULL && (ACollFrame *) root->frame.child->parent == root) {
        if (((ACollFrame *) root->frame.child)->model != NULL) {
            ((ACollFrame *) root->frame.child)->flags &= ~1;
        }
        acollAtkClearHits((ACollFrame *) root->frame.child);
    }
    if (root->frame.next != NULL && (ACollFrame *) root->frame.next->prev == root) {
        if (((ACollFrame *) root->frame.next)->model != NULL) {
            ((ACollFrame *) root->frame.next)->flags &= ~1;
        }
        acollAtkClearHits((ACollFrame *) root->frame.next);
    }
}

/* Clears the hit flags of every box under a root (the same as acollAtkClearHits, written the same
 * way). */
void acollDmgClearHits(ACollFrame *root) {
    if (root->frame.child != NULL && (ACollFrame *) root->frame.child->parent == root) {
        if (((ACollFrame *) root->frame.child)->model != NULL) {
            ((ACollFrame *) root->frame.child)->flags &= ~1;
        }
        acollDmgClearHits((ACollFrame *) root->frame.child);
    }
    if (root->frame.next != NULL && (ACollFrame *) root->frame.next->prev == root) {
        if (((ACollFrame *) root->frame.next)->model != NULL) {
            ((ACollFrame *) root->frame.next)->flags &= ~1;
        }
        acollDmgClearHits((ACollFrame *) root->frame.next);
    }
}

/* Records a hit by another box. */
void acollAtkRecordHit(ACollFrame *f, ACollFrame *other) {
    if (f->flags & 4) {
        f->lastHitKind = f->hitKind;
        f->hitKind = other->kind;
        f->hitPos[0] = other->pos[0];
        f->hitPos[1] = other->pos[1];
        f->hitPos[2] = other->pos[2];
        f->hitRotY = other->rot[1];
        f->hitPower = other->power;
        f->hitInfo = other->info;
    }
}

void acollDmgRecordHit(ACollFrame *f, ACollFrame *other) {
    if (f->flags & 4) {
        f->lastHitKind = f->hitKind;
        f->hitKind = other->kind;
        f->hitPos[0] = other->pos[0];
        f->hitPos[1] = other->pos[1];
        f->hitPos[2] = other->pos[2];
        f->hitRotY = other->rot[1];
        f->hitPower = other->power;
        f->hitInfo = other->info;
    }
}

/* An attack box under parent, following model. */
ACollFrame *_atkLinkAdd(Frame *parent, Frame *model, f32 x, f32 y, f32 z, f32 sx,
                                   f32 sy, f32 sz, f32 rx, f32 ry, f32 rz, u16 radius, s16 ox,
                                   s16 oy, s16 oz, s32 unused) {
    ACollFrame *f = (ACollFrame *) frmCreate(parent, FRM_Atk);

    if (f == NULL) {
        PRINTF("_atkLinkAdd: Warning: atk frame が生成されませんでした\n");
        return NULL;
    }
    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
    f->scale[0] = sx;
    f->scale[1] = sy;
    f->scale[2] = sz;
    f->rot[0] = rx;
    f->rot[1] = ry;
    f->rot[2] = rz;
    f->radius = radius;
    f->ofs[0] = ox;
    f->ofs[1] = oy;
    f->ofs[2] = oz;
    f->model = model;
    f->flags |= 2 | 4 | 8;
    f->kind = 1;
    return f;
}

/* A damage box under parent, following model. */
ACollFrame *_dmgLinkAdd(Frame *parent, Frame *model, f32 x, f32 y, f32 z, f32 sx,
                                   f32 sy, f32 sz, f32 rx, f32 ry, f32 rz, u16 radius,
                                   u16 height, s16 ox, s16 oy, s16 oz, s32 unused) {
    ACollFrame *f = (ACollFrame *) frmCreate(parent, FRM_Dmg);

    if (f == NULL) {
        PRINTF("_dmgLinkAdd: Warning: dmg frame が生成されませんでした\n");
        return NULL;
    }
    f->pos[0] = x;
    f->pos[1] = y;
    f->pos[2] = z;
    f->scale[0] = sx;
    f->scale[1] = sy;
    f->scale[2] = sz;
    f->rot[0] = rx;
    f->rot[1] = ry;
    f->rot[2] = rz;
    f->radius = radius;
    f->height = height;
    f->ofs[0] = ox;
    f->ofs[1] = oy;
    f->ofs[2] = oz;
    f->model = model;
    f->flags |= 2 | 4 | 8;
    f->kind = 1;
    return f;
}

/* A model frame's place, rotation and scale. */
void _FrameToPara(Frame *model, f32 *x, f32 *y, f32 *z, f32 *rx, f32 *ry, f32 *rz,
                            f32 *sx, f32 *sy, f32 *sz) {
    f32 lx, ly, lz;
    f32 lrx, lry, lrz;
    f32 lsx, lsy, lsz;
    s16 kind = model->kind;

    if (gFrmKindAnime == kind) {
        lx = ((PolFrame2 *) model)->pos[0];
        ly = ((PolFrame2 *) model)->pos[1];
        lz = ((PolFrame2 *) model)->pos[2];
        lrx = ((PolFrame2 *) model)->rot[0];
        lry = ((PolFrame2 *) model)->rot[1];
        lrz = ((PolFrame2 *) model)->rot[2];
        lsx = ((PolFrame2 *) model)->scale[0];
        lsy = ((PolFrame2 *) model)->scale[1];
        lsz = ((PolFrame2 *) model)->scale[2];
    } else if (gFrmKindPol == kind) {
        lx = ((PolFrame2 *) model)->pos[0];
        ly = ((PolFrame2 *) model)->pos[1];
        lz = ((PolFrame2 *) model)->pos[2];
        lrx = ((PolFrame2 *) model)->rot[0];
        lry = ((PolFrame2 *) model)->rot[1];
        lrz = ((PolFrame2 *) model)->rot[2];
        lsx = ((PolFrame2 *) model)->scale[0];
        lsy = ((PolFrame2 *) model)->scale[1];
        lsz = ((PolFrame2 *) model)->scale[2];
    } else if (gFrmKindAniAtk == kind) {
        aniAtkSyncLinkTime((AniAtkFrame *) model, ((AniAtkFrame *) model)->link);
        aniAtkLinkTransform((AniAtkFrame *) model, ((AniAtkFrame *) model)->link,
                               ((AniAtkFrame *) model)->time, &lx, &ly, &lz, &lrx, &lry, &lrz,
                               &lsx, &lsy, &lsz);
    } else {
        PRINTF("_FrameToPara: 該当フレームなし  新設するように森脇に申請して〜\n");
        return;
    }
    *x = lx;
    *y = ly;
    *z = lz;
    *rx = lrx;
    *ry = lry;
    *rz = lrz;
    *sx = lsx;
    *sy = lsy;
    *sz = lsz;
}

/* The offset (ox, oy, oz) turned and scaled by the model's matrix, from its place. */
void acollTurnOffset(f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz,
                            s16 ox, s16 oy, s16 oz, f32 *outx, f32 *outy, f32 *outz) {
    f32 m[4][4];

    mathMtxTransRotScale(m, x, y, z, rx, ry, rz, sx, sy, sz);
    guMtxXFMF(m, ox, oy, oz, outx, outy, outz);
}

/* The box's place: the model's plus the offset, scaled, or turned too (turned set). */
void acollBoxPos(f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz,
                            s16 ox, s16 oy, s16 oz, f32 *outx, f32 *outy, f32 *outz, u8 turned) {
    if (!turned) {
        *outx = ox * sx + x;
        *outy = oy * sy + y;
        *outz = oz * sz + z;
    } else {
        acollTurnOffset(x, y, z, rx, ry, rz, sx, sy, sz, ox, oy, oz, outx, outy, outz);
    }
}

/* Sphere (h 0) or cylinder against a sphere of radius r2, including the cylinder's rims. (The
 * first test's int 0 keeps its zero apart from the others'; the sphere case returns a ternary.) */
s32 acollHitTest(f32 r, f32 h, f32 dx, f32 dy, f32 dz, f32 r2) {
    f32 d;
    f32 c;
    f32 flat;
    s32 pad[2];

    if (h == 0) {
        return (dx * dx + dy * dy + dz * dz <= (r2 + r) * (r2 + r)) ? 1 : 0;
    }
    flat = dz * dz + dx * dx;
    if ((r2 + r) * (r2 + r) < flat) {
        return 0;
    }
    d = sqrtf(flat);
    if (-r <= d && d <= r && 0.0f <= dy && dy <= h) {
        return 1;
    }
    if (h < dy + r2 && dy < r2) {
        return 1;
    }
    if (h < dy - r2 || dy + r2 < 0.0f) {
        return 0;
    }
    c = (r - d) / r2;
    if (c >= -1.0f && c <= 1.0f && acollSliceInSpan((r - d) / r2, r2, dy, h, 0.0f) == 1) {
        return 1;
    }
    c = (-r - d) / r2;
    if (c >= -1.0f && c <= 1.0f && acollSliceInSpan(c, r2, dy, h, 0.0f) == 1) {
        return 1;
    }
    return 0;
}

/* Whether a sphere's slice at c (-1 to 1) reaches the span lo to h. */
s32 acollSliceInSpan(f32 c, f32 r, f32 y, f32 h, f32 lo) {
    f32 s;
    f32 ns;

    if (c >= -1.0f && c <= 1.0f) {
        s = sqrtf(1 - c * c);
        ns = -s;
        if (r * s + y < lo || h < r * ns + y) {
            return 0;
        }
        return 1;
    }
    return 0;
}

/* The attack's place relative to the damage box, turned into the box's frame. */
void acollRelPos(ACollFrame *atk, ACollFrame *dmg, f32 *dx, f32 *dy, f32 *dz) {
    *dx = atk->pos[0] - dmg->pos[0];
    *dy = atk->pos[1] - dmg->pos[1];
    *dz = atk->pos[2] - dmg->pos[2];
    mathVecRotateInv(dmg->rot[0], dmg->rot[1], dmg->rot[2], dx, dy, dz);
}

/* Moves a damage box's push point to where the attack meets its edge. The point is a Vec3f in
 * memory (no separate copies, so r and h stay in caller-saved registers), the scales are read as
 * Vec3f fields, and only the dy tests' zeros are 0.0f: the game shares that one constant (f0)
 * and makes the others afresh, which int zeros do. */
void acollSetPushPoint(ACollFrame *dmg, f32 dx, f32 dy, f32 dz) {
    f32 r;
    f32 h;
    Vec3f p;
    f32 d;
    f32 t;
    f32 lim;
    s32 pad[2];

    if (dmg == NULL || dmg->push == NULL) {
        return;
    }
    r = dmg->radius * ((Vec3f *) dmg->scale)->x;
    h = dmg->height * ((Vec3f *) dmg->scale)->y;
    if (h == 0) {
        t = r / sqrtf(dx * dx + dy * dy + dz * dz);
        p.x = t * dx;
        p.y = t * dy;
        p.z = t * dz;
    } else {
        if (dy >= 0.0f && dy <= h) {
            p.y = dy;
            d = sqrtf(dz * dz + dx * dx);
            p.z = dz / d * r;
            p.x = dx / d * r;
        } else {
            lim = (dy > 0.0f) ? h : 0;
            p.y = dy;
            d = sqrtf(dz * dz + dx * dx);
            t = (lim - dy) / (h / 2 - dy);
            p.z = (0 - dz) * t + dz;
            p.x = (0 - dx) * t + dx;
            if (r * r < p.z * p.z + p.x * p.x) {
                p.z = p.z / d * r;
                p.x = p.x / d * r;
            }
        }
        mathVecRotate(dmg->rot[0], dmg->rot[1], dmg->rot[2], &p.x, &p.y, &p.z);
    }
    dmg->push[0] = dmg->pos[0] + p.x;
    dmg->push[1] = dmg->pos[1] + p.y;
    dmg->push[2] = dmg->pos[2] + p.z;
}

/* Attack and damage boxes after a model's own place (sphere; cylinder), offset only in y/z or
 * in all three. */
ACollFrame *acollAtkAddYZ(Frame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a) {
    f32 x, y, z;
    f32 sx, sy, sz;
    f32 rx, ry, rz;

    _FrameToPara(model, &x, &y, &z, &rx, &ry, &rz, &sx, &sy, &sz);
    return _atkLinkAdd(parent, model, x, y, z, sx, sy, sz, rx, ry, rz, radius, 0, oy, oz, a);
}

void acollAtkAddXYZ(Frame *parent, Frame *model, u16 radius, s16 ox, s16 oy, s16 oz,
                            s32 a) {
    f32 x, y, z;
    f32 sx, sy, sz;
    f32 rx, ry, rz;

    _FrameToPara(model, &x, &y, &z, &rx, &ry, &rz, &sx, &sy, &sz);
    _atkLinkAdd(parent, model, x, y, z, sx, sy, sz, rx, ry, rz, radius, ox, oy, oz, a);
}

ACollFrame *acollDmgAddYZ(Frame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                            s32 a) {
    f32 x, y, z;
    f32 sx, sy, sz;
    f32 rx, ry, rz;

    _FrameToPara(model, &x, &y, &z, &rx, &ry, &rz, &sx, &sy, &sz);
    return _dmgLinkAdd(parent, model, x, y, z, sx, sy, sz, rx, ry, rz, radius, height, 0, oy,
                           oz, a);
}

void acollDmgAddXYZ(Frame *parent, Frame *model, u16 radius, u16 height, s16 ox, s16 oy,
                            s16 oz, s32 a) {
    f32 x, y, z;
    f32 sx, sy, sz;
    f32 rx, ry, rz;

    _FrameToPara(model, &x, &y, &z, &rx, &ry, &rz, &sx, &sy, &sz);
    _dmgLinkAdd(parent, model, x, y, z, sx, sy, sz, rx, ry, rz, radius, height, ox, oy,
                           oz, a);
}

/* Whether an attack (a damage box) was hit this step. */
s32 acollAtkWasHit(ACollFrame *f) {
    if (f == NULL) {
        return FALSE;
    }
    if (f->flags & 1) {
        return TRUE;
    }
    return FALSE;
}

s32 acollDmgWasHit(ACollFrame *f) {
    if (f == NULL) {
        return FALSE;
    }
    if (f->flags & 1) {
        return TRUE;
    }
    return FALSE;
}
