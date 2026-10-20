#include "tlb.h"
#include "tlb/tlb_0033.h"

/* File 33, part 4: the song demo ("UTADEMO"). The task "SONG" waits to be told (state 1) to
 * play sound 0xB9 and start scene 0x1B7; the task "RETU" loads the singers' sets, one every
 * third frame (Tai Samba, the fat oni "BOO", Kabuki 64), and builds the fat oni's anime frame. */

/* The "SONG" task's work. */
typedef struct SongWork {
    /* 0x00 */ s32 state; /* 1: play the jingle and start the next scene */
    /* 0x04 */ u8 pad04[8];
    /* 0x0C */ s32 timer;
} SongWork;

/* The "RETU" task's work ("L" in its print). */
typedef struct RetuWork {
    /* 0x00 */ s32 state;
    /* 0x04 */ s32 frames;
    /* 0x08 */ s32 unused08; /* written 0 at its start, never read */
    /* 0x0C */ u8 pad0C[8];
    /* 0x14 */ Frame *tai;
    /* 0x18 */ AnimeFrame *boo;
    /* 0x1C */ Frame *kabu;
    /* 0x20 */ u8 unused20; /* written 0 at its start, never read */
    /* 0x21 */ u8 unused21; /* written 0 at its start, never read */
    /* 0x22 */ u8 unused22; /* written 0 at its start, never read */
} RetuWork;

extern u8 utaRetuFatOniAnim[]; /* the fat oni's animation */

void utaSongStart(Obj *obj) {
    ((SongWork *) (obj + 1))->state = 1;
}

s32 utaSongProc(Obj *obj, s16 msg, void *arg) {
    SongWork *w = (SongWork *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "SONG";
            tskCreate(obj, scParentNoFilesProc, 0, NULL);
            w->timer = 120;
            break;
        case 0:
            switch (w->state) {
                case 0:
                    break;
                case 1:
                    sndPlay(0xB9);
                    scRunStart2(0x1B7, 0, 0);
                    w->state = 0;
                    break;
            }
            break;
        case 1:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Two empty functions nothing calls (splat folds them into the one above). */
void utaUnusedStub1(void) {
}

void utaUnusedStub2(void) {
}

/* The work pointer is set at its declaration (assigned in each case, IDO folds it into obj's
 * offsets); case 1's tests and calls are each on one line, as the game schedules them. */
s32 utaRetuProc(Obj *obj, s16 msg, void *arg) {
    RetuWork *w = (RetuWork *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "RETU";
            w->frames = 0;
            w->unused08 = 0;
            w->unused20 = 0;
            w->unused21 = 0;
            w->unused22 = 0;
            w->state = 0;
            break;
        case 1:
            if (w->tai != NULL) { frmDestroy(w->tai); }
            if (w->boo != NULL) { frmDestroy((Frame *) w->boo); }
            if (w->kabu != NULL) { frmDestroy(w->kabu); }
            break;
        case 0:
            w->frames++;
            switch (w->state) {
                case 0:
                    PRINTF("!! TAI trans !!\n");
                    floadLoadSet(0xF8);
                    w->state++;
                    break;
                case 3:
                    PRINTF("!! BOO trans !!\n");
                    floadLoadSet(0x27);
                    w->state++;
                    break;
                case 6:
                    PRINTF("!! KABU trans !!\n");
                    floadLoadSet(0x28);
                    w->state++;
                    break;
                case 1:
                case 2:
                case 4:
                case 5:
                case 7:
                case 8:
                    w->state++;
                    break;
                case 9:
                    w->boo = (AnimeFrame *) frmCreate(gFrmModelLayer, gFrmKindAnime);
                    floadGetSetRam((void **) w->boo->segs, 0x27);
                    PRINTF("!!!! L->boo = %x !!!!\n", w->boo);
                    w->boo->anim = utaRetuFatOniAnim;
                    w->boo->frame = 0.0f;
                    w->state++;
                    break;
                case 10:
                case 11:
                case 12:
                    break;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
