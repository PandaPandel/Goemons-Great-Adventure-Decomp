#include "tlb.h"
#include "tlb/tlb_0016.h"

void mapUnusedStub1(void) {
}

void mapUnusedStub2(void) {
}

/* The world map's tables (file data, 0x08007190): five areas of 0x28C bytes each: the area's
 * files, its stage markers (MapAnchor), its roads, gate and castle. A MapStage is a road between
 * two markers (its exits): its display list (NULL: none), a kind (0 its own clear flags, 1 the
 * flags of the stage at its first end, 2 always cleared; nonzero also the impact that carries
 * the players along it) and its two ends. */
typedef struct MapExit {
    /* 0x0 */ s8 stage; /* the stage it leads to (lb) */
    /* 0x1 */ u8 pad1[3];
    /* 0x4 */ f32 place; /* where along the road the walk to it ends */
} MapExit; /* size 0x8 */

typedef struct MapStage {
    /* 0x00 */ Gfx *road; /* its road's display list (in the area's file); NULL: no stage here */
    /* 0x04 */ u8 kind;
    /* 0x05 */ u8 roadKind; /* the road's texture and scroll speed: 0, 1 or 2 */
    /* 0x06 */ s16 zone; /* the road's course */
    /* 0x08 */ MapExit exits[2];
} MapStage; /* size 0x18 */

/* An area's castle on the map: the stage it stands for, its look while that stage is not
 * cleared, and where it stands. */
typedef struct MapCastle {
    /* 0x00 */ s8 stage;
    /* 0x01 */ u8 pad1[3];
    /* 0x04 */ Gfx *dl;
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ f32 rotY;
    /* 0x18 */ f32 scale;
} MapCastle; /* size 0x1C */

/* A stage's marker ("anchor") on an area's map: its name, where it stands, and where its
 * cleared flag stands (if it has one). */
typedef struct MapAnchor {
    /* 0x00 */ s16 id; /* 0: no stage */
    /* 0x02 */ s16 hasFlag; /* 1: a flag stands by it once it and the stages to it are cleared */
    /* 0x04 */ char *name;
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ f32 rot[3];
    /* 0x20 */ f32 flagPos[3];
} MapAnchor; /* size 0x2C */

/* An area's gate on the map: the stage that opens it, and where it stands. */
typedef struct MapGate {
    /* 0x00 */ s8 stage;
    /* 0x01 */ u8 pad1[3];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 rotY;
} MapGate; /* size 0x14 */

typedef struct MapArea {
    /* 0x000 */ s32 stageData; /* its stage data (stageSetData) */
    /* 0x004 */ u32 file; /* the file holding its models */
    /* 0x008 */ Gfx *title; /* its name's picture (in that file) */
    /* 0x00C */ MapAnchor anchors[8];
    /* 0x16C */ MapStage stages[10];
    /* 0x25C */ MapGate gate;
    /* 0x270 */ MapCastle castle;
} MapArea; /* size 0x28C */

extern MapArea mapAreas[];
extern s32 mapArea; /* the area the map shows */
extern s32 mapCursorStage; /* the stage the cursor is on */
extern s32 mapChosenStage; /* the stage chosen (-1: none yet) */
extern s32 mapPrevStage; /* the one the cursor was on before */

/* A stage's clear flags. */
u8 mapStageGetFlags(s32 area, s32 stage) {
    switch (mapAreas[area].stages[stage].kind) {
        case 0:
            return ((u8 (*)[16]) gSaveData.areaFlags)[area][stage];
        case 1:
            return ((u8 (*)[16]) gSaveData.stageFlags)[area][mapAreas[area].stages[stage].exits[0].stage];
        case 2:
            return 3;
        default:
            return 0;
    }
}

/* Sets bits in a stage's clear flags. */
void mapStageSetFlags(s32 area, s32 stage, u8 bits) {
    MapStage *s = &mapAreas[area].stages[stage];

    switch (s->kind) {
        case 0:
            ((u8 (*)[16]) gSaveData.areaFlags)[area][stage] |= bits;
            break;
        case 1:
            ((u8 (*)[16]) gSaveData.stageFlags)[area][s->exits[0].stage] |= bits;
            break;
        case 2:
            break;
    }
}

/* The stages (of the first 10 in the area shown) with an exit leading to a stage, into out;
 * returns how many. */
s32 mapGetRoadsTo(s32 stage, s32 *out) {
    s32 n;
    s32 i;
    s32 j;

    for (n = 0, i = 0; i < 10; i++) {
        for (j = 0; j < 2; j++) {
            if (mapAreas[mapArea].stages[i].road != NULL &&
                stage == mapAreas[mapArea].stages[i].exits[j].stage) {
                out[n++] = i;
            }
        }
    }
    return n;
}

/* Whether a stage of the area shown and every stage leading to it are cleared. */
s32 mapIsPathCleared(s32 stage) {
    s32 n;
    s32 i;
    s32 from[4];

    if (!(gSaveData.stageFlags[mapArea * 16 + stage] & 1)) {
        return 0;
    }
    n = mapGetRoadsTo(stage, from);
    for (i = 0; i < n; i++) {
        if (!(mapStageGetFlags(mapArea, from[i]) & 1)) {
            return 0;
        }
    }
    return 1;
}

/* Where the first kind-1 stage of an area leads (0: none). */
s32 mapGetKind1Stage(s32 area) {
    s32 i;

    for (i = 0; i < 10; i++) {
        if (mapAreas[area].stages[i].road != NULL && mapAreas[area].stages[i].kind == 1) {
            return mapAreas[area].stages[i].exits[0].stage;
        }
    }
    return 0;
}

/* Where the first kind-2 stage of an area leads (0: none). */
s32 mapGetKind2Stage(s32 area) {
    s32 i;

    for (i = 0; i < 10; i++) {
        if (mapAreas[area].stages[i].road != NULL && mapAreas[area].stages[i].kind == 2) {
            return mapAreas[area].stages[i].exits[0].stage;
        }
    }
    return 0;
}

/* Takes the players' models out of the frame tree. */
void mapPlayersHide(void) {
    s32 i;
    Player *pl;

    for (i = 0; i < 4; i++) {
        if (gGameWork.players[i] != NULL) {
            pl = PL(gGameWork.players[i]);
            if (pl->model != NULL) {
                frmUnlink(&pl->model->f);
            }
        }
    }
}

/* Puts the players' models back on layer 1. */
void mapPlayersShow(void) {
    s32 i;
    Player *pl;

    for (i = 0; i < 4; i++) {
        if (gGameWork.players[i] != NULL) {
            pl = PL(gGameWork.players[i]);
            if (pl->model != NULL && pl->model->f.parent == NULL) {
                frmLink(&pl->model->f, gFrmLayers[1]);
            }
        }
    }
}

/* The n-th player in play (from 1) says its line, panned by n. */
void mapPlayerSayLine(s32 n) {
    s32 count;
    s32 i;

    for (count = 0, i = 0; i < 4; i++) {
        if (gGameWork.players[i] != NULL && ++count == n) {
            switch (plGetChara(gGameWork.players[i])) {
                case 0:
                    sndPlayPan(0x393, (n & 1) | 0x40);
                    return;
                case 1:
                    sndPlayPan(0x372, (n & 1) | 0x40);
                    return;
                case 2:
                    sndPlayPan(0x3B0, (n & 1) | 0x40);
                    return;
                case 3:
                    sndPlayPan(0x3CF, (n & 1) | 0x40);
                    return;
            }
            return;
        }
    }
}

extern u8 mapAreaTitleDl[];
extern u8 mapArea0Road0Dl[];
MapArea mapAreas[] = {
    { 12, 0x00000114, (Gfx*) mapAreaTitleDl, { { 130, 0, "Home of Wiseman", { 0.0f, 0.0f, 206.0f } }, { 20, 1, "Lost'n Road", { 70.0f, 0.0f, 141.0f }, { 0 }, { 69.0f, 10.0f, 124.0f } }, { 25, 0, "Lost'n Town", { -23.0f, 0.0f, 112.0f } }, { 21, 1, "Digadig Gold Mine", { 72.0f, 0.0f, 33.0f }, { 0 }, { 94.0f, 10.0f, 33.0f } }, { 22, 1, "Ringbell Pass", { -128.0f, 0.0f, 22.0f }, { 0 }, { -144.0f, 10.0f, 40.0f } }, { 29, 0, "Edo Checkpoint", { -12.0f, 0.0f, -61.0f } }, { 30, 0, "Edo Castle", { 12.0f, 40.0f, -207.0f }, { 0.45378563f, 0.0f, 0.0f } } }, { { (Gfx*) mapArea0Road0Dl, 0, 1, 1, { { 0, { 0 }, 40.0f }, { 1, { 0 }, 139.0f } } }, { (Gfx*) 0x030002F0, 0, 1, 1, { { 1, { 0 }, 139.0f }, { 2, { 0 }, 246.0f } } }, { (Gfx*) 0x03000430, 0, 1, 1, { { 2, { 0 }, 246.0f }, { 4, { 0 }, 403.0f } } }, { (Gfx*) 0x03000570, 0, 1, 2, { { 2, { 0 }, 80.0f }, { 3, { 0 }, 216.0f } } }, { (Gfx*) 0x030006B0, 0, 1, 2, { { 3, { 0 }, 216.0f }, { 5, { 0 }, 377.0f } } }, { (Gfx*) 0x030007D0, 0, 1, 1, { { 4, { 0 }, 403.0f }, { 5, { 0 }, 573.0f } } }, { (Gfx*) 0x030008E0, 0, 1, 1, { { 5, { 0 }, 573.0f }, { 6, { 0 }, 713.0f } } }, { (Gfx*) 0x03001AF0, 1, 1, 0, { { 6, { 0 }, 713.0f }, { 10, { 0 }, 790.0f } } } }, { 5, { 0 }, { -6.0f, 0.0f, -92.0f }, 0.0f }, { 6, { 0 }, (Gfx*) 0x03001750, { 9.72f, 45.84f, -266.1f }, 0.0f, 0.1141f } },
    { 33, 0x00000115, (Gfx*) mapAreaTitleDl, { { 40, 1, "Kappa Road", { 5.0f, -30.0f, 202.5f }, { 0 }, { -14.0f, -20.0f, 219.0f } }, { 46, 1, "Frog Mountain", { 155.0f, -60.0f, 25.0f }, { 0 }, { 174.0f, -50.0f, 10.0f } }, { 50, 0, "Otohime Town", { 0.0f, -17.5f, -180.0f } }, { 47, 1, "Naruto Road", { -157.5f, -60.0f, 25.0f }, { 0 }, { -173.0f, -50.0f, 10.0f } }, { 58, 0, "Ryugu Checkpoint", { 0.0f, 12.5f, 115.0f }, { 0.36651915f, 0.0f, 0.0f } }, { 51, 0, "Ryugu Castle", { 0.0f, 20.0f, 15.0f } } }, { { (Gfx*) 0x030002D0, 0, 2, 1, { { 0, { 0 }, 1124.0f }, { 3, { 0 }, 905.0f } } }, { (Gfx*) 0x03000630, 0, 1, 2, { { 0, { 0 }, 166.0f }, { 4, { 0 }, 258.0f } } }, { (Gfx*) 0x03000410, 0, 2, 1, { { 1, { 0 }, 491.0f }, { 2, { 0 }, 702.0f } } }, { (Gfx*) 0x03000550, 0, 2, 1, { { 2, { 0 }, 702.0f }, { 3, { 0 }, 905.0f } } }, { (Gfx*) 0x03000190, 0, 2, 1, { { 0, { 0 }, 249.0f }, { 1, { 0 }, 491.0f } } }, { (Gfx*) 0x030006D0, 0, 0, 2, { { 4, { 0 }, 258.0f }, { 5, { 0 }, 350.0f } } }, { (Gfx*) 0x030011F0, 2, 1, 2, { { 0, { 0 }, 166.0f }, { 10, { 0 }, 100.0f } } }, { (Gfx*) 0x03001300, 1, 1, 2, { { 5, { 0 }, 350.0f }, { 10, { 0 }, 450.0f } } } }, { 4, { 0 }, { 1.0f, 18.5f, 84.6f }, 0.0f }, { 5, { 0 }, (Gfx*) 0x03000F50, { -3.0f, 18.0f, -30.0f }, 0.0f, 0.08f } },
    { 69, 0x00000116, (Gfx*) mapAreaTitleDl, { { 60, 1, "Kokeshi Road", { -65.0f, 2.0f, 219.0f }, { 0 }, { -79.0f, 10.0f, 202.0f } }, { 61, 1, "Maneater Road", { -11.0f, 0.0f, 121.0f }, { 0 }, { -9.0f, 10.0f, 102.0f } }, { 69, 1, "Pochan Lake", { -207.0f, -12.0f, -63.0f }, { 0 }, { -190.0f, -2.0f, -78.0f } }, { 71, 0, "Spook Village", { -18.0f, 0.0f, -210.0f }, { 0 }, { -18.0f, 0.0f, -210.0f } }, { 65, 1, "Tomb Pass", { 129.0f, 0.0f, 23.0f }, { 0 }, { 115.0f, 10.0f, 2.0f } }, { 78, 0, "Mafu Checkpoint", { 35.0f, 22.0f, -30.0f }, { 0.0f, -0.33161256f, 0.0f }, { 35.0f, 22.0f, -30.0f } }, { 72, 0, "Majin Castle", { -71.0f, 48.0f, -57.0f }, { 0.10471976f, -0.13962634f, -0.6108653f }, { -71.0f, 48.0f, -57.0f } } }, { { (Gfx*) 0x03000150, 0, 1, 2, { { 0, { 0 }, 140.0f }, { 1, { 0 }, 252.0f } } }, { (Gfx*) 0x03000650, 0, 2, 2, { { 1, { 0 }, 1314.0f }, { 2, { 0 }, 1019.0f } } }, { (Gfx*) 0x03000260, 0, 1, 2, { { 1, { 0 }, 252.0f }, { 4, { 0 }, 435.0f } } }, { (Gfx*) 0x030004F0, 0, 2, 2, { { 2, { 0 }, 1019.0f }, { 3, { 0 }, 762.0f } } }, { (Gfx*) 0x03000770, 0, 1, 1, { { 4, { 0 }, 42.0f }, { 5, { 0 }, 152.0f } } }, { (Gfx*) 0x030003B0, 0, 2, 2, { { 3, { 0 }, 762.0f }, { 4, { 0 }, 435.0f } } }, { (Gfx*) 0x03000860, 0, 1, 1, { { 5, { 0 }, 152.0f }, { 6, { 0 }, 260.0f } } }, { (Gfx*) 0x03001960, 2, 1, 2, { { 0, { 0 }, 140.0f }, { 10, { 0 }, 50.0f } } }, { (Gfx*) 0x03001A70, 1, 1, 1, { { 6, { 0 }, 260.0f }, { 10, { 0 }, 360.0f } } } }, { 5, { 0 }, { 5.0f, 20.0f, -43.0f }, 1.1972959f }, { 6, { 0 }, (Gfx*) 0x03001550, { -137.7f, 55.0f, -71.6f }, -0.2617994f, 0.091f } },
    { 87, 0x00000117, (Gfx*) mapAreaTitleDl, { { 80, 1, "Kaiware Road", { 173.0f, 0.0f, -10.0f }, { 0 }, { 152.0f, 10.0f, -2.0f } }, { 81, 1, "Mokeke Forest", { 81.0f, -9.0f, 115.0f }, { 0 }, { 88.0f, 1.0f, 95.0f } }, { 84, 1, "Wonder Valley", { -104.0f, 46.0f, 32.0f }, { 0 }, { -126.0f, 60.0f, 31.0f } }, { 95, 0, "Tree Stump Village", { -101.0f, 66.0f, -128.0f }, { 0 }, { -101.0f, 66.0f, -128.0f } }, { 98, 0, "Underworld Checkpoint", { 48.0f, 68.0f, 19.0f }, { 0 }, { 48.0f, 68.0f, 19.0f } }, { 87, 0, "Underworld Castle", { 31.0f, 96.0f, -76.0f }, { 0.38397247f, 0.0f, 0.0f }, { 31.0f, 96.0f, -76.0f } }, { 138, 0, "Creep Village", { 26.0f, 8.0f, 208.0f }, { 0 }, { 26.0f, 8.0f, 208.0f } } }, { { (Gfx*) 0x03000170, 0, 1, 1, { { 0, { 0 }, 152.0f }, { 1, { 0 }, 320.0f } } }, { (Gfx*) 0x03000790, 0, 1, 3, { { 1, { 0 }, 45.0f }, { 6, { 0 }, 174.0f } } }, { (Gfx*) 0x030002C0, 0, 2, 1, { { 1, { 0 }, 320.0f }, { 2, { 0 }, 528.0f } } }, { (Gfx*) 0x03000420, 0, 1, 1, { { 2, { 0 }, 528.0f }, { 3, { 0 }, 688.0f } } }, { (Gfx*) 0x03000580, 0, 1, 2, { { 2, { 0 }, 75.0f }, { 4, { 0 }, 235.0f } } }, { (Gfx*) 0x03000680, 0, 1, 2, { { 4, { 0 }, 240.0f }, { 5, { 0 }, 326.0f } } }, { (Gfx*) 0x03001330, 2, 1, 1, { { 0, { 0 }, 152.0f }, { 10, { 0 }, 65.0f } } }, { (Gfx*) 0x03001440, 1, 1, 2, { { 5, { 0 }, 326.0f }, { 10, { 0 }, 420.0f } } } }, { 4, { 0 }, { 40.0f, 65.0f, -4.0f }, 0.0f }, { 5, { 0 }, (Gfx*) 0x030010C0, { 21.0f, 100.0f, -143.5f }, 0.0f, 0.0924f } },
    { 98, 0x00000118, (Gfx*) mapAreaTitleDl, { { 100, 1, "Cloud Fortress", { 0.0f, 0.0f, 197.0f }, { 0 }, { 15.0f, 10.0f, 170.0f } }, { 103, 1, "Sky Garden", { 202.0f, -3.0f, 7.0f }, { 0 }, { 181.0f, 8.0f, 20.0f } }, { 105, 1, "Wind Castle Wall", { -185.0f, -5.0f, -7.0f }, { 0 }, { -165.0f, 5.0f, 8.0f } }, { 110, 0, "Floating World Town", { 4.0f, 0.0f, -179.0f }, { 0 }, { 4.0f, 0.0f, -179.0f } }, { 119, 0, "Floating Checkpoint", { -1.0f, 1.0f, -116.0f }, { 0 }, { -1.0f, 1.0f, -116.0f } }, { 106, 0, "Dream Castle", { 0.0f, 23.0f, -21.0f }, { -0.2617994f, 0.0f, 0.0f }, { 0.0f, 23.0f, -21.0f } } }, { { (Gfx*) 0x030002D0, 0, 2, 1, { { 0, { 0 }, 43.0f }, { 2, { 0 }, 340.0f } } }, { (Gfx*) 0x03000190, 0, 2, 2, { { 0, { 0 }, 124.0f }, { 1, { 0 }, 414.0f } } }, { (Gfx*) 0x03000410, 0, 2, 2, { { 1, { 0 }, 414.0f }, { 3, { 0 }, 711.0f } } }, { (Gfx*) 0x03000630, 0, 0, 3, { { 3, { 0 }, 57.0f }, { 4, { 0 }, 122.0f } } }, { (Gfx*) 0x03000550, 0, 2, 1, { { 2, { 0 }, 340.0f }, { 3, { 0 }, 630.0f } } }, { (Gfx*) 0x030006F0, 0, 1, 3, { { 4, { 0 }, 122.0f }, { 5, { 0 }, 216.0f } } }, { (Gfx*) 0x03001E30, 2, 1, 2, { { 0, { 0 }, 124.0f }, { 10, { 0 }, 40.0f } } } }, { 4, { 0 }, { 0.0f, 0.0f, -98.0f }, 0.0f }, { 5, { 0 }, (Gfx*) 0x03001940, { -1.0f, 27.5f, 43.54f }, 0.0f, 0.125f } },
};

/* "cursor" (its proc mapCursorProc): its frame, whether it is hidden, the stage it stands on, and
 * the choices from there (the stages with a road to it) and the one it points at. */
typedef struct CursorW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 hidden;
    /* 0x05 */ u8 pad05[0x3];
    /* 0x08 */ s32 stage; /* its create argument */
    /* 0x0C */ s32 choice; /* -1: none */
    /* 0x10 */ s32 count;
    /* 0x14 */ s32 values[20];
} CursorW;

extern Gfx mapCursorArrowDl[]; /* the cursor's arrow (file 0x113) */

/* "cursor": an arrow on the stage the players stand on, pointing along the road to the chosen
 * next stage (first a cleared one or the one after the stage they came from, else the first
 * open one); left and right on the D-pad turn it through the open ones. */
s32 mapCursorProc(Obj *obj, s16 msg, void *arg) {
    CursorW *w = (CursorW *) FX(obj);
    s32 pad;
    s32 i;
    s32 j;
    s32 old;
    MapStage *s;
    f32 place;
    Vec3f pos;

    switch (msg) {
        case 2:
            obj->name = "cursor";
            w->stage = (s32) arg;
            w->frame = (PolFrame *) frmCreate(gFrmLayers[1], gFrmKindPol);
            w->frame->dl2 = mapCursorArrowDl;
            floadGetSetRam((void **) w->frame->segs, 0x113);
            w->count = mapGetRoadsTo(w->stage, w->values);
            w->frame->pos[0] = mapAreas[mapArea].anchors[w->stage].pos[0];
            w->frame->pos[1] = mapAreas[mapArea].anchors[w->stage].pos[1] + 30.0f;
            w->frame->pos[2] = mapAreas[mapArea].anchors[w->stage].pos[2];
            w->frame->scale[2] = 0.05999999866f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->choice = -1;
            for (i = 0; i < w->count; i++) {
                if ((mapStageGetFlags(mapArea, w->values[i]) & 0x83) == 1) {
                    w->choice = i;
                    break;
                }
            }
            for (i = 0; w->choice == -1 && i < w->count; i++) {
                for (j = 0; j < 2; j++) {
                    if (mapPrevStage ==
                        mapAreas[mapArea].stages[w->values[i]].exits[j].stage) {
                        w->choice = i + 1;
                        if (w->choice >= w->count) {
                            w->choice = 0;
                        }
                    }
                }
                if (w->choice != -1 &&
                    (mapStageGetFlags(mapArea, w->values[w->choice]) & 0x81) != 1) {
                    w->choice = -1;
                }
            }
            for (i = 0; w->choice == -1 && i < w->count; i++) {
                if ((mapStageGetFlags(mapArea, w->values[i]) & 0x81) == 1) {
                    w->choice = i;
                }
            }
            if (w->choice == -1) {
                w->frame->f.flags &= ~2;
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            if (w->hidden) {
                break;
            }
            if (w->choice < 0) {
                break;
            }
            old = w->choice;
            for (i = 0; i < 4; i++) {
                if (!plIsInPlay(i)) {
                    continue;
                }
                if (joyGetHeldTime(i, L_JPAD) == 1) {
                    do {
                        if (--w->choice < 0) {
                            w->choice = w->count - 1;
                        }
                    } while ((mapStageGetFlags(mapArea, w->values[w->choice]) & 0x81) != 1 &&
                             old != w->choice);
                }
                if (joyGetHeldTime(i, R_JPAD) == 1) {
                    do {
                        if (++w->choice >= w->count) {
                            w->choice = 0;
                        }
                    } while ((mapStageGetFlags(mapArea, w->values[w->choice]) & 0x81) != 1 &&
                             old != w->choice);
                }
            }
            if (old != w->choice) {
                sndPlay(0x119);
            }
            s = &mapAreas[mapArea].stages[w->values[w->choice]];
            if (w->stage == s->exits[0].stage) {
                place = s->exits[1].place;
                mapChosenStage = s->exits[1].stage;
            } else {
                place = s->exits[0].place;
                mapChosenStage = s->exits[0].stage;
            }
            vramCoursePointXZ(s->zone, place, 0.0f, &pos.x, &pos.z);
            w->frame->rot[0] += 12.73239517f;
            w->frame->rot[1] = mathAtan2(pos.x - w->frame->pos[0], pos.z - w->frame->pos[2]) - 1.570796371f;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 0x80, 0x80, 0xFF, 0xFF);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The value of the choice it is on (-1: no cursor or no choice). */
s32 mapCursorGetChoice(Obj *obj) {
    CursorW *w = (CursorW *) FX(obj);

    if (obj == NULL) {
        return -1;
    }
    if (w->choice < 0) {
        return -1;
    }
    return w->values[w->choice];
}

/* Hides it. */
void mapCursorHide(Obj *obj) {
    CursorW *w = (CursorW *) FX(obj);

    if (obj != NULL) {
        w->frame->f.flags &= ~2;
        w->hidden = 1;
    }
}

/* Shows it. */
void mapCursorShow(Obj *obj) {
    CursorW *w = (CursorW *) FX(obj);

    if (obj != NULL) {
        w->frame->f.flags |= 2;
        w->hidden = 0;
    }
}

/* "fplace" (its proc mapFplaceProc): the name plate of the stage under the cursor, a box that
 * slides to the stage's place on the screen and grows (cur, 0-1) or shrinks; x, y and cur each
 * move to a target, their step the distance over the steps. */
typedef struct FplaceW {
    /* 0x00 */ DLFrame *frame; /* the box */
    /* 0x04 */ TextFrame *text; /* the stage's name */
    /* 0x08 */ s32 stage; /* its create argument */
    /* 0x0C */ f32 curX;
    /* 0x10 */ f32 curY;
    /* 0x14 */ f32 halfW; /* half the box's size */
    /* 0x18 */ f32 halfH;
    /* 0x1C */ f32 targetX;
    /* 0x20 */ f32 stepX;
    /* 0x24 */ f32 targetY;
    /* 0x28 */ f32 stepY;
    /* 0x2C */ f32 steps;
    /* 0x30 */ f32 cur;
    /* 0x34 */ f32 step;
    /* 0x38 */ f32 target;
} FplaceW;

void mapFplaceScaleTo(Obj *o, f32 v);
void mapFplaceMoveTo(Obj *o, f32 x, f32 y);

extern Gfx mapBoxFillDl[]; /* the box's fill setup */
extern Gfx mapBoxTexDl[]; /* its texture setup */

s32 mapFplaceProc(Obj *obj, s16 msg, void *arg) {
    FplaceW *w = (FplaceW *) FX(obj);
    f32 *pos;
    s32 pad[2];
    f32 sx;
    f32 sy;

    switch (msg) {
        case 2:
            obj->name = "fplace";
            w->stage = (s32) arg;
            w->frame = (DLFrame *) frmCreate(gFrmLayers[10], gFrmKindDlist);
            w->text = frmFontCreate(gFrmLayers[10], 0, 0,
                                    mapAreas[mapArea].anchors[w->stage].name);
            if (w->frame == NULL || w->text == NULL) {
                return -1;
            }
            w->halfW = (frmFontGetWidth(w->text) + 2) / 2 + 1;
            w->halfH = 8.0f;
            w->steps = 8.0f;
            w->cur = 0.0f;
            mapFplaceScaleTo(obj, 1.0f);
            w->steps = 1.0f;
            break;
        case 1:
            frmFontDestroy(w->text);
            frmDestroy(&w->frame->f);
            break;
        case 0:
            pos = mapAreas[mapArea].anchors[w->stage].pos;
            fadeProjectScreen(pos[0], pos[1], pos[2], &sx, &sy);
            mapFplaceMoveTo(obj, (s32) sx, (s32) sy + 8);
            w->steps = 4.0f;
            if (9.999999747e-06f <w->stepX) {
                if (fabsf(w->targetX - w->curX) < w->stepX) {
                    w->curX = w->targetX;
                    w->stepX = 0.0f;
                } else if (w->targetX < w->curX) {
                    w->curX = w->curX - w->stepX;
                } else {
                    w->curX = w->curX + w->stepX;
                }
            }
            if (9.999999747e-06f <w->stepY) {
                if (fabsf(w->targetY - w->curY) < w->stepY) {
                    w->curY = w->targetY;
                    w->stepY = 0.0f;
                } else if (w->targetY < w->curY) {
                    w->curY = w->curY - w->stepY;
                } else {
                    w->curY = w->curY + w->stepY;
                }
            }
            if (9.999999747e-06f <w->step) {
                if (fabsf(w->target - w->cur) < w->step) {
                    w->cur = w->target;
                    w->step = 0.0f;
                } else if (w->target < w->cur) {
                    w->cur = w->cur - w->step;
                } else {
                    w->cur = w->cur + w->step;
                }
            }
            w->text->x = w->curX - w->halfW + 2.0f;
            w->text->y = w->curY - w->halfH + 2.0f;
            if (0.9900000095f < w->cur) {
                w->text->frame.flags |= 2;
            } else {
                w->text->frame.flags &= ~2;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(mapBoxFillDl, obj->file));
            gDPFillRectangle(gGfxDlHead++, (s32) (w->curX - w->halfW * w->cur - 1.0f),
                             (s32) (w->curY - w->halfH * w->cur - 1.0f), (s32) (w->curX + w->halfW * w->cur + 1.0f),
                             (s32) (w->curY + w->halfH * w->cur + 1.0f));
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(mapBoxTexDl, obj->file));
            gSPTextureRectangle(gGfxDlHead++, (s32) ((w->curX - w->halfW * w->cur) * 4.0f),
                                (s32) ((w->curY - w->halfH * w->cur) * 4.0f),
                                (s32) ((w->curX + w->halfW * w->cur) * 4.0f),
                                (s32) ((w->curY + w->halfH * w->cur) * 4.0f), 0, 0, 0, 0x400, 0x400);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

void mapFplaceScaleTo(Obj *o, f32 v) {
    ((FplaceW *) FX(o))->target = v;
    ((FplaceW *) FX(o))->step = fabsf(v - ((FplaceW *) FX(o))->cur) / ((FplaceW *) FX(o))->steps;
}

/* The same for x and y, over the same steps. */
void mapFplaceMoveTo(Obj *o, f32 x, f32 y) {
    FplaceW *w = (FplaceW *) FX(o);
    f32 steps = w->steps;

    w->targetX = x;
    w->targetY = y;
    w->stepX = fabsf(x - w->curX) / steps;
    w->stepY = fabsf(y - w->curY) / steps;
}

s32 mapFplaceProc(Obj *obj, s16 msg, void *arg);

/* Starts a mapFplaceProc task under obj. */
Obj *mapFplaceMake(Obj *obj, void *arg) {
    return tskCreate(obj, mapFplaceProc, obj->file, arg);
}

/* "manual": the help box in the map's corner, "Move" and "Enter stage" with the A and B button
 * pictures, in the language of the settings. */
typedef struct ManualW {
    /* 0x00 */ DLFrame *frame;
    /* 0x04 */ TextFrame *move;
    /* 0x08 */ TextFrame *enter;
    /* 0x0C */ s32 x;
    /* 0x10 */ s32 y;
    /* 0x14 */ s32 w;
    /* 0x18 */ s32 h;
} ManualW;

/* The settings at 0x800881F8 (fade.c's Settings); byte 8 is the language: 0 English, 1 French,
 * 2 German. */
typedef struct MapSettings {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ s8 language;
    /* 0x09 */ u8 pad9[7];
} MapSettings;
extern MapSettings gSaveSettings;

extern char * mapMoveText[3]; /* "Move", by language */
extern char * mapEnterStageText[]; /* "Enter stage", by language */
extern Gfx mapBoxCopyTexDl[]; /* the box's texture setup */
extern u8 mapManualButtonsTex[]; /* the A and B buttons, 20x10 RGBA16 (file 0x113) */

s32 mapManualProc(Obj *obj, s16 msg, void *arg) {
    ManualW *w = (ManualW *) FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "manual";
            w->x = 203;
            w->y = 10;
            w->w = 93;
            w->h = 25;
            w->frame = (DLFrame *) frmCreate(gFrmLayers[10], gFrmKindDlist);
            w->move = frmFontCreate(&w->frame->f, w->x + 15, w->y + 2, mapMoveText[gSaveSettings.language]);
            w->enter = frmFontCreate(&w->frame->f, w->x + 15, w->y + 13, mapEnterStageText[gSaveSettings.language]);
            if (w->frame == NULL || w->move == NULL || w->enter == NULL) {
                return -1;
            }
            frmFontSetCell(w->move, 8, 10, 1, 0);
            frmFontSetCell(w->enter, 8, 10, 1, 0);
            w->w = frmFontGetWidth(w->enter) + 16;
            w->x = 294 - w->w;
            w->move->x = w->x + 16;
            w->enter->x = w->x + 16;
            break;
        case 1:
            frmFontDestroy(w->move);
            frmFontDestroy(w->enter);
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(mapBoxFillDl, obj->file));
            gDPFillRectangle(gGfxDlHead++, w->x - 1, w->y - 1, w->x + w->w + 1, w->y + w->h + 1);
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(mapBoxTexDl, obj->file));
            gSPTextureRectangle(gGfxDlHead++, w->x << 2, w->y << 2, (w->x + w->w) << 2, (w->y + w->h) << 2, 0, 0,
                                0, 0x400, 0x400);
            gSPDisplayList(gGfxDlHead++, fadeSegToRam(mapBoxCopyTexDl, obj->file));
            gDPPipeSync(gGfxDlHead++);
            gDPLoadTextureBlock(gGfxDlHead++, floadSegToRam((u32) mapManualButtonsTex, 0x113, 0), G_IM_FMT_RGBA, G_IM_SIZ_16b, 20,
                                10, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(gGfxDlHead++, (w->x + 2) << 2, (w->y + 2) << 2, (w->x + 11) << 2, (w->y + 11) << 2,
                                0, 0, 0, 0x1000, 0x400);
            gSPTextureRectangle(gGfxDlHead++, (w->x + 2) << 2, (w->y + 13) << 2, (w->x + 11) << 2,
                                (w->y + 22) << 2, 0, 10 << 5, 0, 0x1000, 0x400);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 mapManualProc(Obj *obj, s16 msg, void *arg);

Obj *mapManualMake(Obj *obj) {
    return tskCreate(obj, mapManualProc, obj->file, NULL);
}

s32 mapArea = 0; /* the area the map shows */
s32 mapCursorStage = 0; /* the stage the cursor is on */
s32 mapChosenStage = 0; /* the stage chosen (-1: none yet) */
s32 mapPrevStage = 0; /* the one the cursor was on before */
s32 mapPlayerCount = 0; /* the players in play on the map (0-2) */
u32 mapUnusedDl[] = { 0xE7000000, 0x00000000, 0xE3000A01, 0x00000000, 0xD9D0F9FA, 0x00000000, 0xD9FFFFFF, 0x00220404, 0xE200001C, 0x00552048, 0xE2001E01, 0x00000000, 0xDF000000, 0x00000000, 0x03030303, 0x03030300, 0x03030303, 0x03030300 };
Gfx mapBoxFillDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0x00),
    gsSPEndDisplayList(),
}; /* the box's fill setup */
Gfx mapBoxTexDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0xA0),
    gsSPEndDisplayList(),
}; /* its texture setup */
Gfx mapBoxCopyTexDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_COPY),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
}; /* the box's texture setup */
char *mapMoveText[3] = { "Move", "MOVE", "move" }; /* "Move", by language */
char *mapEnterStageText[3] = { "Enter stage", "ENTER STAGE", "enter stage" }; /* "Enter stage", by language */

/* "gate": the area's gate, an animated model. */
typedef struct GateW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 speed; /* its animation's step: 0 shut, rising as it opens */
} GateW;

extern u8 mapGateAnim[]; /* the gate's animation (file 0x113) */

/* "gate": shut until its stage is cleared; the first time after that (bit 2 not yet set) it
 * opens, else it stands open at frame 1. */
s32 mapGateProc(Obj *obj, s16 msg, void *arg) {
    GateW *w = (GateW *) FX(obj);
    s32 pad;
    MapGate *g;
    f32 last;
    u8 flags;

    switch (msg) {
        case 2:
            obj->name = "gate";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayers[3], gFrmKindAnime);
            w->frame->anim = mapGateAnim;
            floadGetSetRam((void **) w->frame->segs, 0x113);
            g = &mapAreas[mapArea].gate;
            w->frame->pos[0] = g->pos[0];
            w->frame->pos[1] = g->pos[1];
            w->frame->pos[2] = g->pos[2];
            w->frame->rot[1] = g->rotY;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            flags = ((u8 (*)[16]) gSaveData.stageFlags)[mapArea][g->stage];
            if (flags & 1) {
                if (flags & 2) {
                    w->frame->frame = 1.0f;
                    w->speed = 0.0f;
                } else {
                    ((u8 (*)[16]) gSaveData.stageFlags)[mapArea][g->stage] |= 2;
                    w->speed += 0.05;
                }
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->frame += w->speed;
            last = polAnimeLastFrame(w->frame);
            if (last < w->frame->frame) {
                w->frame->frame = last;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "castle" and the map's other single models: a "pol" frame. */
typedef struct CastleW {
    /* 0x00 */ PolFrame *frame;
} CastleW;

extern Gfx mapCastleClearedDl[]; /* the cleared castle's look (file 0x113) */

/* "castle": the area's castle, in its cleared look once its stage is cleared (which also sets
 * the stage's bit 2). */
s32 mapCastleProc(Obj *obj, s16 msg, void *arg) {
    CastleW *w = (CastleW *) FX(obj);
    s32 pad;
    MapCastle *c;

    switch (msg) {
        case 2:
            obj->name = "castle";
            c = &mapAreas[mapArea].castle;
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayers[3], gFrmKindPol)) == NULL) {
                return -1;
            }
            if (((u8 (*)[16]) gSaveData.stageFlags)[mapArea][c->stage] & 1) {
                ((u8 (*)[16]) gSaveData.stageFlags)[mapArea][c->stage] |= 2;
                w->frame->dl2 = mapCastleClearedDl;
                floadGetSetRam((void **) w->frame->segs, 0x113);
                w->frame->pos[0] = c->pos[0];
                w->frame->pos[1] = c->pos[1];
                w->frame->pos[2] = c->pos[2];
                w->frame->rot[1] = c->rotY;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = c->scale;
            } else {
                w->frame->dl2 = c->dl;
                floadGetSetRam((void **) w->frame->segs, mapAreas[mapArea].file);
                w->frame->scale[2] = 0.1f;
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "anchor": a stage's marker, its colour pulsing (blue-white on the stage the cursor is on, red
 * on a cleared one), with a flag by it once it and the stages to it are cleared. */
typedef struct AnchorW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *flag;
    /* 0x08 */ s32 stage; /* its create argument */
    /* 0x0C */ s32 pulse; /* 0-255 */
    /* 0x10 */ s32 pulseStep;
    /* 0x14 */ u8 flags; /* the stage's save flags when it was made */
} AnchorW;

extern Gfx mapAnchorMarkerDl[]; /* the marker (file 0x113) */
extern Gfx mapAnchorFlagDl[]; /* the flag (file 0x113) */

s32 mapAnchorProc(Obj *obj, s16 msg, void *arg) {
    AnchorW *w = (AnchorW *) FX(obj);
    s32 pad;
    MapAnchor *a;
    s32 fade;

    switch (msg) {
        case 2:
            obj->name = "anchor";
            w->stage = (s32) arg;
            a = &mapAreas[mapArea].anchors[(s32) arg];
            if (a->id == 0) {
                return -1;
            }
            w->flags = ((u8 (*)[16]) gSaveData.stageFlags)[mapArea][w->stage];
            if ((w->frame = (PolFrame *) frmCreateNext(gFrmLayers[6], gFrmKindPol)) == NULL) {
                return -1;
            }
            w->frame->dl2 = mapAnchorMarkerDl;
            floadGetSetRam((void **) w->frame->segs, 0x113);
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->frame->rot[0] = a->rot[0];
            w->frame->rot[1] = a->rot[1];
            w->frame->rot[2] = a->rot[2];
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            if (a->hasFlag == 1 && mapIsPathCleared(w->stage) == 1) {
                if ((w->flag = (PolFrame *) frmCreate(gFrmLayers[1], gFrmKindPol)) != NULL) {
                    floadGetSetRam((void **) w->flag->segs, 0x113);
                    w->flag->dl2 = mapAnchorFlagDl;
                    w->flag->pos[0] = a->flagPos[0];
                    w->flag->pos[1] = a->flagPos[1];
                    w->flag->pos[2] = a->flagPos[2];
                    w->flag->scale[2] = 0.1f;
                    w->flag->scale[0] = w->flag->scale[1] = w->flag->scale[2];
                    w->flag->flags |= 7;
                }
            }
            w->pulse = 0;
            w->pulseStep = 8;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            fade = 255 - w->pulse;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0, 0, 0, 0, COMBINED, 0, 0, 0,
                              COMBINED);
            if ((w->flags & 1) && !(w->flags & 0x80)) {
                if (mapChosenStage == w->stage) {
                    gDPSetEnvColor(gGfxDlHead++, fade, fade, 255, 255);
                } else {
                    gDPSetEnvColor(gGfxDlHead++, w->pulse, 0, 0, 255);
                }
            } else {
                if (mapChosenStage == w->stage) {
                    gDPSetEnvColor(gGfxDlHead++, fade, fade, 255, 255);
                } else {
                    gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
                }
            }
            gSPEndDisplayList(gGfxDlHead++);
            w->pulse += w->pulseStep;
            if (w->pulse >= 256) {
                w->pulse = 255;
                w->pulseStep = -8;
            } else if (w->pulse < 0) {
                w->pulse = 0;
                w->pulseStep = 8;
            }
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "road": a stage's road on the map, shown once the stage is cleared (and not hidden by bit
 * 0x80); its texture scrolls, red, or blue and four times as fast while it joins the stage the
 * cursor is on and the one chosen. The first time it is shown it unrolls (opening). */
typedef struct RoadW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 stage; /* its create argument */
    /* 0x08 */ s16 scroll;
    /* 0x0A */ s16 speed;
    /* 0x0C */ u8 opening;
    /* 0x0D */ u8 lit;
} RoadW;

extern u8 mapRoadKind1Tex[]; /* the road textures (file 0x113) */
extern u8 mapRoadKind0Tex[];
extern u8 mapRoadKind2Tex[];

s32 mapRoadProc(Obj *obj, s16 msg, void *arg) {
    RoadW *w = (RoadW *) FX(obj);
    u32 tex;
    s32 pad;
    u8 flags;
    MapStage *s;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "road";
            w->stage = (s32) arg;
            s = &mapAreas[mapArea].stages[(s32) arg];
            if (s->road == NULL) {
                return -1;
            }
            if ((w->frame = (PolFrame *) frmCreateNext(gFrmLayers[6], gFrmKindPol)) == NULL) {
                return -1;
            }
            flags = mapStageGetFlags(mapArea, w->stage);
            if (!(flags & 1) || (flags & 0x80)) {
                break;
            }
            w->frame->dl2 = s->road;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            floadGetSetRam((void **) w->frame->segs, mapAreas[mapArea].file);
            switch (mapAreas[mapArea].stages[w->stage].roadKind) {
                case 0:
                    w->speed = -4;
                    break;
                case 1:
                default:
                    w->speed = -2;
                    break;
                case 2:
                    w->speed = -1;
                    break;
            }
            if (!(flags & 2)) {
                mapStageSetFlags(mapArea, w->stage, 2);
                w->opening = 1;
                if (mapCursorStage == mapAreas[mapArea].stages[w->stage].exits[1].stage) {
                    w->scroll = 0;
                    w->speed = -w->speed;
                } else {
                    w->scroll = 0x200;
                }
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            s = &mapAreas[mapArea].stages[w->stage];
            if ((mapChosenStage == s->exits[0].stage || mapChosenStage == s->exits[1].stage) &&
                (mapCursorStage == s->exits[0].stage || mapCursorStage == s->exits[1].stage)) {
                w->lit = 1;
            } else {
                w->lit = 0;
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gSPClearGeometryMode(gGfxDlHead++, G_FOG);
            gDPSetRenderMode(gGfxDlHead++, G_RM_PASS, G_RM_AA_ZB_XLU_SURF2);
            switch (mapAreas[mapArea].stages[w->stage].roadKind) {
                default:
                    tex = (u32) mapRoadKind1Tex;
                    break;
                case 0:
                    tex = (u32) mapRoadKind0Tex;
                    break;
                case 1:
                    tex = (u32) mapRoadKind1Tex;
                    break;
                case 2:
                    tex = (u32) mapRoadKind2Tex;
                    break;
            }
            tex = floadSegToRam(tex, 0x113, 0);
            if (!w->opening) {
                gDPSetTextureImage(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_16b, 1, tex);
                gDPSetTile(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR,
                           8, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 3, G_TX_NOLOD);
                gDPSetTileSize(gGfxDlHead++, G_TX_RENDERTILE, 0, w->scroll << 2, 7 << 2, (w->scroll + 255) << 2);
                gDPLoadSync(gGfxDlHead++);
                gDPLoadBlock(gGfxDlHead++, G_TX_LOADTILE, 0, 0, 1023, 2048);
                gDPPipeSync(gGfxDlHead++);
                gDPSetTile(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_8b, 1, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR,
                           8, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 3, G_TX_NOLOD);
            } else {
                gDPSetTextureImage(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_16b, 1, tex);
                gDPSetTile(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_CLAMP, 8, G_TX_NOLOD,
                           G_TX_CLAMP, 3, G_TX_NOLOD);
                gDPSetTileSize(gGfxDlHead++, G_TX_RENDERTILE, 0, w->scroll << 2, 7 << 2, (w->scroll + 255) << 2);
                gDPLoadSync(gGfxDlHead++);
                gDPLoadBlock(gGfxDlHead++, G_TX_LOADTILE, 0, 0, 1023, 2048);
                gDPPipeSync(gGfxDlHead++);
                gDPSetTile(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_8b, 1, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 8, G_TX_NOLOD,
                           G_TX_CLAMP, 3, G_TX_NOLOD);
                if ((w->scroll & 0x1F0) == 0x100) {
                    w->opening = 0;
                }
            }
            if (mapAreas[mapArea].stages[w->stage].kind == 0) {
                gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0,
                                  0, 0, COMBINED, 0, 0, 0, COMBINED);
            } else {
                gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0,
                                  0, 0, COMBINED, SHADE, 0, COMBINED, 0);
            }
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            if (w->lit) {
                gDPSetEnvColor(gGfxDlHead++, 0, 0, 255, 255);
                w->scroll += w->speed * 4;
            } else {
                gDPSetEnvColor(gGfxDlHead++, 255, 0, 0, 255);
                w->scroll += w->speed;
            }
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "title": the area's name in the map's corner, under its own orthographic camera. */
typedef struct TitleW {
    /* 0x00 */ PolFrame *frame; /* the name's backing */
    /* 0x04 */ PolFrame *name; /* the area's name */
    /* 0x08 */ CamView *cam;
} TitleW;

extern Gfx mapTitleBackingDl[]; /* the backing (file 0x113) */
extern Gfx mapTitleNameDl[]; /* the name's setup */

s32 mapTitleProc(Obj *obj, s16 msg, void *arg) {
    TitleW *w = (TitleW *) FX(obj);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "title";
            w->cam = (CamView *) frmCreate(gFrmLayers[10], gFrmKindCamera);
            w->frame = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
            w->name = (PolFrame *) frmCreate(&w->cam->f, gFrmKindPol);
            if (w->cam == NULL || w->frame == NULL || w->name == NULL) {
                return -1;
            }
            w->cam->ortho |= 1;
            w->cam->zNear = -200.0f;
            w->cam->zFar = 200.0f;
            w->frame->dl = gDrawSetupLitTexEdgeDl;
            w->frame->dl2 = mapTitleBackingDl;
            floadGetSetRam((void **) w->frame->segs, 0x113);
            w->frame->rot[0] = 3.141592741f;
            w->frame->pos[0] = 96.0f;
            w->frame->pos[1] = 32.0f;
            w->frame->scale[2] = 0.75f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->rot[2] = 1.570796371f;
            w->name->dl = (Gfx *) fadeSegToRam(mapTitleNameDl, obj->file);
            w->name->dl2 = mapAreas[mapArea].title;
            floadGetSetRam((void **) w->name->segs, mapAreas[mapArea].file);
            w->name->rot[0] = 3.141592741f;
            w->name->pos[0] = 98.0f;
            w->name->pos[1] = 30.0f;
            w->name->scale[2] = 0.8000000119f;
            w->name->scale[0] = w->name->scale[1] = w->name->scale[2];
            break;
        case 1:
            frmDestroy(&w->cam->f);
            break;
        case 0:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "atbgm": after 8 frames it plays sound 0x8B (or 0x8C) unless that sound already plays, then
 * ends. */
typedef struct AtbgmW {
    /* 0x00 */ u32 count;
    /* 0x04 */ s32 which; /* its create argument */
} AtbgmW;

s32 mapAtbgmProc(Obj *obj, s16 msg, void *arg) {
    AtbgmW *w = (AtbgmW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "atbgm";
            w->which = (s32) arg;
            break;
        case 0:
            if (++w->count >= 8) {
                if (w->which == 0) {
                    if (sndGetVolume(0x8B, 0) == 0) {
                        sndPlay(0x8B);
                    }
                } else {
                    if (sndGetVolume(0x8C, 0) == 0) {
                        sndPlay(0x8C);
                    }
                }
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a mapAtbgmProc task under obj. */
Obj *mapAtbgmMake(Obj *obj, void *arg) {
    return tskCreate(obj, mapAtbgmProc, obj->file, arg);
}

/* The work the map's step functions get (mapStepStart, mapStepWaitScenario, mapStepChoose...):
 * the next step and a timer the steps reset. */
typedef struct MapStepW MapStepW;
typedef s32 (*MapStep)(Obj *obj, MapStepW *w);
typedef s32 (*MapAreaCam)(MapStepW *w);

struct MapStepW {
    /* 0x00 */ Camera *camera;
    /* 0x04 */ CamView *view;
    /* 0x08 */ Obj *sun; /* camSunCreate's */
    /* 0x0C */ Obj *manual; /* its mapManualProc task */
    /* 0x10 */ Obj *cursor; /* its mapCursorProc task */
    /* 0x14 */ Obj *fplace; /* its mapFplaceProc task */
    /* 0x18 */ MapStep step;
    /* 0x1C */ MapAreaCam areaCam; /* the area's camera (mapCamArea0...) */
    /* 0x20 */ u8 toEnding; /* nonzero: a stage is entered without A */
    /* 0x21 */ u8 chara; /* the character of the player who entered */
    /* 0x22 */ u8 partner; /* and of the other player (0xFF: none) */
    /* 0x23 */ u8 hold; /* 1: the map stands still, 2: its camera (mapSetHold, mapSetCamHold) */
    /* 0x24 */ u8 enterState;
    /* 0x25 */ u8 pad25;
    /* 0x26 */ s16 timer;
    /* 0x28 */ s16 stage; /* the stage the cursor starts on */
};

s32 mapCursorProc(Obj *obj, s16 msg, void *arg);
Obj *mapFplaceMake(Obj *obj, void *arg);
s32 mapCursorGetChoice(Obj *obj);
void mapCursorHide(Obj *obj);


extern s32 mapPlayerCount; /* the players in play on the map (0-2) */

/* The file's .data, in address order (its externs above stay as declarations). */
extern MapArea mapAreas[];
extern u32 mapUnusedDl[];
extern Gfx mapBoxFillDl[];
extern Gfx mapBoxTexDl[];
extern Gfx mapBoxCopyTexDl[];
extern char *mapMoveText[3];
extern char *mapEnterStageText[3];
/* a display list nothing in the file names (splat gave it no label) */
Gfx mapUnusedEnvAlphaDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0),
    gsSPEndDisplayList(),
};
Gfx mapTitleNameDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0),
    gsDPSetEnvColor(0x00, 0x00, 0x00, 0xFF),
    gsSPEndDisplayList(),
}; /* the name's setup */
extern u16 gScenarioFiles[]; /* the scenario files (sctable.c) */

s32 mapStepStart(Obj *obj, MapStepW *w);
s32 mapStepWaitScenario(Obj *obj, MapStepW *w);
s32 mapStepChoose(Obj *obj, MapStepW *w);
s32 mapStepWaitWalk(Obj *obj, MapStepW *w);
s32 mapStepEnter(Obj *obj, MapStepW *w);
s32 mapCamNone(s32 arg);
s32 mapCamArea0(MapStepW *w);
s32 mapCamArea1(MapStepW *w);
s32 mapCamArea2(MapStepW *w);
s32 mapCamArea3(MapStepW *w);
s32 mapCamArea4(MapStepW *w);
void mapCursorShow(Obj *obj);

/* "atlas": the world map. Made: checks some player is left, loads the map's files and tasks,
 * puts the players (one or two) on the stage they left (or the area's first), the stage markers,
 * cursor, roads, gate, castle, title and help box, and the area's scenery and camera; each frame
 * runs the step (w->step) and the area's camera. */
s32 mapAtlasProc(Obj *obj, s16 msg, void *arg) {
    s32 i;
    f32 *pos;
    s32 pad[2];
    PlInit init;
    PlInit one;
    MapStepW *w = (MapStepW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "atlas";
            gGameWork.partake = 1;
            if ((gSaveData.players[0].chara < 0 || gSaveData.players[0].lives <= 0) &&
                (gSaveData.players[1].chara < 0 || gSaveData.players[1].lives <= 0)) {
                PRINTF("Caution: プレイヤーが全滅しています in atlas \n");
                stageAllOut();
                break;
            }
            gRetracesPerFrame = 2;
            for (i = 0; i < 4; i++) {
                joySetStickDpad(i, 1);
            }
            mapArea = gGameWork.area;
            mapCursorStage = gGameWork.areaStage;
            mapChosenStage = -1;
            if (mapCursorStage == -1) {
                mapCursorStage = mapGetKind1Stage(mapArea);
            } else if (mapCursorStage == -2) {
                mapCursorStage = mapGetKind2Stage(mapArea);
            } else if (mapCursorStage == -3) {
                w->toEnding = 1;
                mapCursorStage = 0;
            } else if (mapCursorStage == -4) {
                mapCursorStage = mapGetKind2Stage(mapArea);
            }
            floadLoadSet(0xBD);
            floadLoadSet(0x113);
            floadLoadSet(mapAreas[mapArea].file);
            tskCreate(obj, effect, 0, NULL);
            tskCreate(obj, Geography, 0, NULL);
            geoSetCacheSize(40);
            vramCreate(obj);
            stageSetData(mapAreas[mapArea].stageData);
            geoSetMode(1);
            tskCreate(obj, Ride, 0, NULL);
            tskCreate(obj, attack_Proc, 0, NULL);
            tskCreate(obj, scParentProc, 0, NULL);
            frmFontNewKind();
            resModesOn();
            fileLoadNowKeep(gScenarioFiles[46], NULL);
            w->camera = (Camera *) tskCreate(obj, dracam, 0, NULL);
            gWorldOriginX = gWorldOriginY = gWorldOriginZ = 0.0f;
            camSetEye(w->camera, 124.0f, 112.0f, 400.0f);
            camSetAt(w->camera, 60.0f, 85.0f, 256.0f);
            w->view = camGetView(w->camera);
            w->view->zNear = 20.0f;
            w->view->zFar = 800.0f;
            w->view->fovy = 0.6981317401f;
            w->sun = camSunCreate(obj);
            for (mapPlayerCount = 0, i = 0; i < 2; i++) {
                if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].chara < 4 &&
                    gSaveData.players[i].lives > 0) {
                    mapPlayerCount++;
                }
            }
            for (i = 0; i < 2; i++) {
                if (gSaveData.players[i].chara >= 0 && gSaveData.players[i].chara < 4 &&
                    gSaveData.players[i].lives > 0) {
                    if (stageCountChara(gSaveData.players[i].chara) >= 2) {
                        gSaveData.players[i].chara = gaugeNextChara(gSaveData.players[i].chara, 1);
                    }
                    plMakeInit(&init, i, gSaveData.players[i].chara);
                    init.zone = 0;
                    if (mapPlayerCount == 1) {
                        init.place = mapAreas[mapArea].anchors[mapCursorStage].pos[0];
                    } else {
                        init.place =
                            mapAreas[mapArea].anchors[mapCursorStage].pos[0] + 5.0f - i * 10;
                    }
                    init.height = mapAreas[mapArea].anchors[mapCursorStage].pos[1];
                    init.side = mapAreas[mapArea].anchors[mapCursorStage].pos[2];
                    plCreate(obj, &init);
                    plEnterDemo(gGameWork.players[i]);
                }
            }
            if (mapPlayerCount == 0) {
                plMakeInit(&one, 0, 0);
                one.zone = 0;
                one.place = mapAreas[mapArea].anchors[mapCursorStage].pos[0];
                one.height = mapAreas[mapArea].anchors[mapCursorStage].pos[1];
                one.side = mapAreas[mapArea].anchors[mapCursorStage].pos[2];
                plCreate(obj, &one);
                plEnterDemo(gGameWork.players[0]);
            }
            if (w->toEnding == 0) {
                if (gSaveData.areaFlags[0] & 2) {
                    mapAtbgmMake(obj, (void *) 0);
                } else {
                    mapAtbgmMake(obj, (void *) 1);
                }
            }
            for (i = 0; i < 8; i++) {
                tskCreate(obj, mapAnchorProc, obj->file, (void *) i);
            }
            w->cursor = tskCreate(obj, mapCursorProc, obj->file, (void *) mapCursorStage);
            w->fplace = mapFplaceMake(obj, (void *) mapCursorStage);
            for (i = 0; i < 10; i++) {
                tskCreate(obj, mapRoadProc, obj->file, (void *) i);
            }
            tskCreate(obj, mapGateProc, obj->file, NULL);
            tskCreate(obj, mapCastleProc, obj->file, NULL);
            if (w->toEnding == 0) {
                tskCreate(obj, mapTitleProc, obj->file, NULL);
                w->manual = mapManualMake(obj);
            }
            w->step = mapStepStart;
            switch (mapArea) {
                default:
                    w->areaCam = (MapAreaCam) mapCamNone;
                    break;
                case 0:
                    w->areaCam = mapCamArea0;
                    break;
                case 1:
                    mapSetupSeaFalls(obj);
                    w->areaCam = mapCamArea1;
                    break;
                case 2:
                    mapSetupLava(obj);
                    w->areaCam = mapCamArea2;
                    break;
                case 3:
                    mapSetupSea(obj);
                    w->areaCam = mapCamArea3;
                    break;
                case 4:
                    mapSetupObisu2(obj);
                    w->areaCam = mapCamArea4;
                    break;
            }
            tskSendMsg(w->sun, 0, NULL);
            camSunRefreshHold(1);
            camSunSetColor(255, 255, 255);
            if (!(w->hold & 2)) {
                w->areaCam(w);
                camSnapToTargets(w->camera);
            }
            if (gGameWork.areaStage < 0) {
                pos = mapAreas[mapArea].anchors[mapCursorStage].pos;
                mapImpactArriveMake(obj, 0, pos[0], pos[1], pos[2]);
                mapCursorHide(w->cursor);
            }
            break;
        case 1:
            for (i = 0; i < 4; i++) {
                joySetStickDpad(i, 0);
            }
            frmFontDeleteKind();
            while (gfxGetPendingSwaps()) {
            }
            resModesOff();
            resFree(gScenarioFiles[46]);
            break;
        case 0:
            if (w->hold & 1) {
                break;
            }
            if (w->cursor != NULL) {
                mapCursorShow(w->cursor);
            }
            w->timer++;
            if (w->step != NULL && w->step(obj, w) != 0) {
                return -1;
            }
            if (!(w->hold & 2) && w->areaCam != NULL) {
                w->areaCam(w);
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The players' lines at timer 4 and 8; then, after a boss stage (areaStage below -2) with a save
 * in use, the area-clear scenario (0x51; -3 the last area: the game is cleared), else the next
 * step. */
s32 mapStepStart(Obj *obj, MapStepW *w) {
    if (w->timer == 4) {
        if (gGameWork.areaStage < 0) {
            mapPlayerSayLine((gGameWork.entryFrom & 3) + 1);
        }
    } else if (w->timer == 8) {
        if (gGameWork.areaStage < 0) {
            mapPlayerSayLine(((gGameWork.entryFrom & 3) ^ 1) + 1);
        }
        if (gGameWork.areaStage < -2 && gGameWork.diary >= 0 && gGameWork.diary < 3) {
            if (gGameWork.areaStage == -3) {
                gSaveData.allClear++;
                gSaveData.savePoint = 7;
            } else {
                gSaveData.savePoint = gGameWork.area + 7;
            }
            scRunStart(0x51, 0, 0);
            w->timer = 0;
            w->step = mapStepWaitScenario;
        } else {
            w->timer = 0;
            w->step = mapStepChoose;
        }
    }
    return 0;
}

/* Waits for the scenario to end, then goes to mapStepChoose. */
s32 mapStepWaitScenario(Obj *obj, MapStepW *w) {
    if (scIsRunning() == 0) {
        w->timer = 0;
        w->step = mapStepChoose;
    }
    return 0;
}

/* The map's choosing step. A on an open stage (or toEnding) enters it (mapStepEnter);
 * B on the stage under the cursor walks there: through an impact if the stage has one, else
 * along the road to the exit away from the stage the cursor was on (the other player too,
 * unless mapPlayerCount), then waits for the walk (mapStepWaitWalk). */
s32 mapStepChoose(Obj *obj, MapStepW *w) {
    s32 i;
    s32 n;
    s32 choice;
    MapStage *s;
    Vec3f pos;
    f32 target;
    Vec3f from;
    Vec3f dir;
    Vec3f at;
    f32 t;

    for (i = 0; i < 4; i++) {
        if (!plIsInPlay(i)) {
            continue;
        }
        if (joyGetHeldTime(i, A_BUTTON) == 1 || w->toEnding != 0) {
            if (((u8 (*)[16]) gSaveData.stageFlags)[mapArea][mapCursorStage] & 0x84) {
                continue;
            }
            w->chara = gSaveData.players[i].chara;
            if (plIsInPlay(i ^ 1)) {
                w->partner = gSaveData.players[i ^ 1].chara;
            } else {
                w->partner = 0xFF;
            }
            w->enterState = 0;
            w->step = mapStepEnter;
            w->areaCam = (MapAreaCam) mapCamNone;
            return 0;
        }
        if (joyGetHeldTime(i, B_BUTTON) != 1) {
            continue;
        }
        if ((choice = mapCursorGetChoice(w->cursor)) < 0) {
            continue;
        }
        s = &mapAreas[mapArea].stages[choice];
        plAskPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z, &n);
        if (s->kind != 0) {
            plGetModelPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z);
            mapImpactLeaveMake(obj, s->kind, pos.x, pos.y, pos.z);
            mapPlayersHide();
            mapCursorHide(w->cursor);
            gGameWork.entryFrom = i;
            return 0;
        }
        if (mapCursorStage == s->exits[0].stage) {
            pos.x = s->exits[0].place;
            pos.y = mapAreas[mapArea].anchors[s->exits[0].stage].pos[1];
            target = s->exits[1].place;
            w->stage = s->exits[1].stage;
        } else {
            pos.x = s->exits[1].place;
            pos.y = mapAreas[mapArea].anchors[s->exits[1].stage].pos[1];
            target = s->exits[0].place;
            w->stage = s->exits[0].stage;
        }
        vramCoursePointXZ(s->zone, pos.x, pos.z, &from.x, &from.z);
        from.y = pos.y + 20.0f;
        dir.x = 0.0f;
        dir.z = 0.0f;
        dir.y = -40.0f;
        if (vramRayGround(&from, &dir, &at, &t)) {
            pos.y = at.y;
        }
        if (mapPlayerCount == 1) {
            pos.z = 0.0f;
            plSetPos(gGameWork.players[i], s->zone, &pos);
            pos.x = target;
            plDemoWalkToPlace(gGameWork.players[i], target, 2.5f);
        } else {
            n = (i + 1) & 1;
            pos.z = 5 - i * 10;
            plSetPos(gGameWork.players[i], s->zone, &pos);
            pos.z = 5 - n * 10;
            plSetPos(gGameWork.players[n], s->zone, &pos);
            pos.x = target;
            plDemoWalkToPlace(gGameWork.players[i], target, 2.5f);
            plDemoWalkToPlace(gGameWork.players[n], target, 1.8f);
        }
        w->step = mapStepWaitWalk;
        w->cursor = tskDestroy(w->cursor);
        w->fplace = tskDestroy(w->fplace);
        return 0;
    }
    return 0;
}

/* Once no player in play is walking, puts up the cursor and the fplace on w->stage, goes to
 * mapStepChoose, and sets every player in play to action 0xD7. */
s32 mapStepWaitWalk(Obj *obj, MapStepW *w) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) && plIsDemoWalking(gGameWork.players[i])) {
            return 0;
        }
    }
    mapPrevStage = mapCursorStage;
    mapChosenStage = -1;
    mapCursorStage = w->stage;
    w->step = mapStepChoose;
    w->cursor = tskCreate(obj, mapCursorProc, obj->file, (void *) mapCursorStage);
    w->fplace = mapFplaceMake(obj, (void *) mapCursorStage);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plSetAction(gGameWork.players[i], 0xD7);
        }
    }
    return 0;
}

/* Entering the stage under the cursor: records it in gGameWork, clears the last area's low flag
 * bits, fades out with the camera closing in on its marker while the players call out, then
 * goes to the stage (the area's boss battle for its castle once that is open, the ending for
 * toEnding). */
s32 mapStepEnter(Obj *obj, MapStepW *w) {
    MapAnchor *a;
    s32 i;

    switch (w->enterState) {
        case 0:
            gGameWork.area = mapArea;
            gGameWork.areaStage = mapCursorStage;
            gGameWork.tunePart = 0;
            for (i = 0; i < 7; i++) {
                gSaveData.stageFlags[88 + i] &= ~0xF;
            }
            w->cursor = tskDestroy(w->cursor);
            w->fplace = tskDestroy(w->fplace);
            w->enterState = 1;
            w->timer = 0;
            break;
        case 1:
            if (w->timer == 1) {
                tskDestroy(fadeCreate(obj));
                fadeCreate(obj);
                fadeSetLevel(0.0f);
                fadeSetSpeed(40.0f);
                fadeSetColor(0, 0, 0);
                fadeSetTarget(255.0f);
                a = &mapAreas[mapArea].anchors[mapCursorStage];
                camSetSpeed(w->camera, 30.0f);
                camMoveAt(w->camera, a->pos[0], a->pos[1], a->pos[2]);
                camSetSpeed(w->camera, 30.0f);
                camMoveFov(w->camera, 0.34906587f);
                if (w->toEnding == 0) {
                    sndPlay(0x162);
                }
            } else if (w->timer == 15) {
                if (w->toEnding == 0) {
                    switch (w->chara) {
                        case 0:
                            sndPlay(0x116);
                            break;
                        case 1:
                            sndPlay(0x115);
                            break;
                        case 2:
                            sndPlay(0x117);
                            break;
                        case 3:
                            sndPlay(0x118);
                            break;
                    }
                } else {
                    sndPlay(10);
                }
            } else if (w->timer == 21) {
                if (w->toEnding == 0) {
                    switch (w->partner) {
                        case 0:
                            sndPlay(0x116);
                            break;
                        case 1:
                            sndPlay(0x115);
                            break;
                        case 2:
                            sndPlay(0x117);
                            break;
                        case 3:
                            sndPlay(0x118);
                            break;
                    }
                }
            } else if (w->timer == 45) {
                if (w->toEnding != 0) {
                    stageGotoEnding();
                    return 1;
                }
                if (mapCursorStage == mapAreas[mapArea].castle.stage &&
                    gSaveData.stageFlags[95 + mapArea] > 0) {
                    stageGotoBattle(mapArea + 1);
                    return 1;
                }
                stageGoto(obj->parent, mapAreas[mapArea].anchors[mapCursorStage].id);
                return 1;
            }
            break;
    }
    return 0;
}

s32 mapCamNone(s32 arg) {
    return 0;
}

/* Area 0's camera: three set views by how far along z the last player in play is. */
s32 mapCamArea0(MapStepW *w) {
    Vec3f pos;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z);
        }
    }
    camSetSpeed(w->camera, 20.0f);
    if (0.0f < pos.z) {
        camMoveAtIfChanged(w->camera, -100.0f, -50.0f, -250.0f);
        camMoveEyeIfChanged(w->camera, pos.x + 100.0f, 112.0f, pos.z * 100.0f / 200.0f + 300.0f);
    } else if (-50.0f < pos.z) {
        camMoveAtIfChanged(w->camera, -100.0 - pos.z * 2.4, -50.0f - pos.z + pos.y / 2, pos.z * 2 + -250.0f);
        camMoveEyeIfChanged(w->camera, pos.x + 100.0f + pos.z * 2, 112.0f, 300.0f);
    } else {
        camMoveAtIfChanged(w->camera, 20.0f, pos.y / 2, -350.0f);
        camMoveEyeIfChanged(w->camera, pos.x - 50.0f, 112.0f, pos.z + 350.0f);
    }
    return 0;
}

/* Area 1's camera: as area 4's, but within 100 of the centre it looks from 180 along x. */
s32 mapCamArea1(MapStepW *w) {
    Vec3f pos;
    Vec3f dir;
    s32 i;
    f32 dist;

    w->view->fovy = 0.8726646304f;
    w->view->zFar = 600.0f;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z);
        }
    }
    dir = pos;
    dir.y = 0.0f;
    dist = sqrtf(pos.x * pos.x + pos.z * pos.z);
    mathVecNormalizeB(&dir);
    camSetSpeed(w->camera, 8.0f);
    camMoveAtIfChanged(w->camera, pos.x, pos.y, pos.z);
    if (dist < 100.0f) {
        camMoveEyeIfChanged(w->camera, pos.x + 180.0f, pos.y + 100.0f, pos.z);
    } else {
        camMoveEyeIfChanged(w->camera, dir.x * 180.0f + pos.x, pos.y + 100.0f, dir.z * 180.0f + pos.z);
    }
    return 0;
}

/* Area 2's camera: the same as area 1's. */
s32 mapCamArea2(MapStepW *w) {
    Vec3f pos;
    Vec3f dir;
    s32 i;
    f32 dist;

    w->view->fovy = 0.8726646304f;
    w->view->zFar = 600.0f;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z);
        }
    }
    dir = pos;
    dir.y = 0.0f;
    dist = sqrtf(pos.x * pos.x + pos.z * pos.z);
    mathVecNormalizeB(&dir);
    camSetSpeed(w->camera, 8.0f);
    camMoveAtIfChanged(w->camera, pos.x, pos.y, pos.z);
    if (dist < 100.0f) {
        camMoveEyeIfChanged(w->camera, pos.x + 180.0f, pos.y + 100.0f, pos.z);
    } else {
        camMoveEyeIfChanged(w->camera, dir.x * 180.0f + pos.x, pos.y + 100.0f, dir.z * 180.0f + pos.z);
    }
    return 0;
}

/* Area 3's camera: as area 4's, but along the direction from (0, 0, -400). dist is area 1's
 * distance from the centre, computed and left unused here (it decides the eye's float registers
 * and takes its stack word). */
s32 mapCamArea3(MapStepW *w) {
    Vec3f pos;
    Vec3f dir;
    s32 i;
    f32 dist;

    w->view->fovy = 0.8726646304f;
    w->view->zFar = 600.0f;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z);
        }
    }
    dir = pos;
    dir.y = 0.0f;
    dir.z += 400.0f;
    dist = sqrtf(pos.x * pos.x + pos.z * pos.z);
    mathVecNormalizeB(&dir);
    camSetSpeed(w->camera, 8.0f);
    camMoveAtIfChanged(w->camera, pos.x, pos.y, pos.z);
    camMoveEyeIfChanged(w->camera, dir.x * 180.0f + pos.x, pos.y + 100.0f, dir.z * 180.0f + pos.z);
    return 0;
}

/* Area 4's camera (the map proc's per-area function, w->areaCam): looks at the last player in
 * play from 180 out along its direction from the origin, 100 up. dist: as area 3's, unused. */
s32 mapCamArea4(MapStepW *w) {
    Vec3f pos;
    Vec3f dir;
    s32 i;
    f32 dist;

    w->view->fovy = 0.8726646304f;
    w->view->zFar = 600.0f;
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plGetModelPos(gGameWork.players[i], &pos.x, &pos.y, &pos.z);
        }
    }
    dir = pos;
    dir.y = 0.0f;
    dist = sqrtf(pos.x * pos.x + pos.z * pos.z);
    mathVecNormalizeB(&dir);
    camSetSpeed(w->camera, 8.0f);
    camMoveAtIfChanged(w->camera, pos.x, pos.y, pos.z);
    camMoveEyeIfChanged(w->camera, dir.x * 180.0f + pos.x, pos.y + 100.0f, dir.z * 180.0f + pos.z);
    return 0;
}

/* The map task's work: its camera first; byte 0x23 holds the hold flags (1 the map, 2 its
 * camera). */
typedef struct MapW {
    /* 0x00 */ Camera *camera;
    /* 0x04 */ u8 pad04[0x1F];
    /* 0x23 */ u8 hold;
} MapW;

void mapSetHold(Obj *obj, s32 on) {
    MapW *w = (MapW *) FX(obj);

    if (on == 1) {
        w->hold |= 1;
    } else {
        w->hold &= ~1;
    }
}

void mapSetCamHold(Obj *obj, s32 on) {
    MapW *w = (MapW *) FX(obj);

    if (on == 1) {
        w->hold |= 2;
    } else {
        w->hold &= ~2;
    }
}

/* The map task's camera, the first word of its work (tlb_0016_1 holds it while it scrolls). */
Camera *mapGetCamera(Obj *obj) {
    return *(Camera **) FX(obj);
}

void mapUnusedStub3(void) {
}

extern u8 mapSeaTex[]; /* the sea's texture (file 0x113) */
extern u8 mapWaterfallTex[]; /* the waterfall's (file 0x115) */

/* A 32x32 RGBA16 texture loaded into TMEM at tmem for tile rtile, as gDPLoadMultiBlock, with its
 * tile size at (uls, ult) (as tlb_0016_1.c's and tlb_0050.c's: one source line). */
#define gDPLoadScrollBlock(pkt, timg, tmem, rtile, uls, ult)                                          \
    {                                                                                                  \
        gDPSetTextureImage(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, timg);                                 \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0,                        \
                   G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD); \
        gDPLoadSync(pkt);                                                                              \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES));        \
        gDPPipeSync(pkt);                                                                              \
        gDPSetTile(pkt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,  \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPSetTileSize(pkt, rtile, uls, ult, (32 - 1) << 2, (32 - 1) << 2);                            \
    }

/* The sea's texture, two layers blended by the environment colour, scrolling against each other. */
Gfx *mapSeaDL(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, TEXEL0, TEXEL1, ENVIRONMENT, TEXEL1, 0, 0, 0, ENVIRONMENT, 0, 0, 0, COMBINED, 0, 0,
                      0, COMBINED);
    gDPSetEnvColor(gfx++, 128, 128, 128, 144);
    gDPLoadScrollBlock(gfx++, mapSeaTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock(gfx++, mapSeaTex, 0x100, 1, scroll + 10, -scroll);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* The waterfall's texture, its two layers falling at two speeds (as tlb_0050.c's waterFallDL). */
Gfx *mapTaki2DL(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, TEXEL0, TEXEL1, ENVIRONMENT, TEXEL1, 0, 0, 0, ENVIRONMENT, 0, 0, 0, COMBINED, 0, 0,
                      0, COMBINED);
    gDPSetEnvColor(gfx++, 128, 128, 128, 160);
    gDPLoadScrollBlock(gfx++, mapWaterfallTex, 0, G_TX_RENDERTILE, 0, scroll);
    gDPLoadScrollBlock(gfx++, mapWaterfallTex, 0x100, 1, 10, scroll * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}
