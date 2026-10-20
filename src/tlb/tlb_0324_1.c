#include "tlb.h"
#include "tlb/tlb_0324.h"

/* "Btai" (the sea-bream boss, tlb_0324_0): its actions. Each takes the boss's task and
 * returns 0; tlb_0324_2's btaiRunAction calls the one its action (w->action) names. */

/* The boss's work (obj + 0x20). */
typedef struct BtaiW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ s32 target[3]; /* a place it heads for */
    /* 0x1C */ f32 count;
    /* 0x20 */ PolFrame *drop; /* the model that drops in after the death (tlb_0324_3) */
    /* 0x24 */ ACollFrame *atk;
    /* 0x28 */ ACollFrame *dmg;
    /* 0x2C */ Obj *cam; /* "dracam", the opening camera's (tlb_0324_3) */
    /* 0x30 */ u8 pad30[8];
    /* 0x38 */ CamView *view; /* tlb_0324_3's */
    /* 0x3C */ s32 invTimer; /* steps of invincibility left after a hit ("無敵timer") */
    /* 0x40 */ u8 pad40[8];
    /* 0x48 */ f32 speed;
    /* 0x4C */ u8 action;
    /* 0x4D */ u8 endPattern; /* 1: end the pattern now (set as each hit reaction ends) */
    /* 0x4E */ u8 step;
    /* 0x4F */ u8 unused4F;
    /* 0x50 */ s16 timer;
} BtaiW;

/* rand(n) (tlb_0324_3). This file calls its neighbours' functions far, through t9. */
#define btaiRand (*btaiRand)
/* tlb_0324_4's: moves the boss toward (x, y, z) at a speed; 0 once there. */
#define btaiBmoveX (*btaiBmoveX)
#define btaiBmoveXFast (*btaiBmoveXFast)
#define btaiRandSigned (*btaiRandSigned)
#define btaiBank (*btaiBank)
/* A bubble (tlb_0324_6). */
#define btaiTbubbleMake (*btaiTbubbleMake)
/* Three "T3bubble"s rising at x, y, z (tlb_0324_6). */
#define btaiT3bubbleMake (*btaiT3bubbleMake)
/* "U3bubble"s at x, y, z (tlb_0324_6). */
#define btaiTsbubbleMake (*btaiTsbubbleMake)
/* A "Tzanzou" (afterimage) under parent (tlb_0324_6), called near. */
/* tlb_0324_4's: pushes the Impact by btaiPushVelX/btaiPushVelZ. */
#define btaiEaseVel (*btaiEaseVel)
/* The barrier (tlb_0324_6). */
#define btaiBarriMake (*btaiBarriMake)
/* A sand-dust puff (tlb_0324_6). */
#define sunaEfMake (*sunaEfMake)
/* A "KOKEef" at x, y, z (tlb_0324_6). */
#define btaiKokeEfMake (*btaiKokeEfMake)
/* A "Ddmybomb" at x, y, z under obj (tlb_0324_5). */
#define btaiDdmybombCreate (*btaiDdmybombCreate)
/* tlb_0324_5's: at x, y, z under obj. */
#define btaiTMarbleCreate (*btaiTMarbleCreate)
/* tlb_0324_5's: a marble at x, y, z under obj, at an angle. */
#define btaiIkaBombCreate (*btaiIkaBombCreate)
/* tlb_0324_4's: steers the boss's velocity toward (a, b, c). */
#define btaiTurnTo (*btaiTurnTo)

/* libultra's, through their addresses. */
#define sinf (*sinf)
#define cosf (*cosf)

/* This file's own, called far as well. */
void btaiSway(Obj *obj);
#define btaiSway (*btaiSway)

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */

/* The file's .data (0x08019EC0, IDO_NOTES 343/349), in the game's order. IDO emits .data in
 * source order, so some of its words are defined between the functions below and some are a
 * function's statics (btaiBarrierAttack's bomb count, the last). */
s32 btaiNewAction = 0;
s32 btaiHitStun = 0;
s32 btaiEelInterval = 1;   /* steps between eels (200, down to 50) */
s32 btaiCannonStun = 1;
f32 btaiPushVelX = 1.0f;
f32 btaiPushVelY = 1.0f; /* nothing reads it */
f32 btaiPushVelZ = 1.0f;
f32 btaiPushStartVelX = 0.0f;
f32 btaiPushStartVelY = 0.0f; /* nothing reads it */
f32 btaiPushStartVelZ = 0.0f;
f32 btaiWhirl2StartSpeed = 1.0f; /* the second whirlpool's start's speed toward the cockpit */
f32 btaiWhirlStartUnusedX = 1.0f;
f32 btaiWhirlStartUnusedY = 1.0f;
f32 btaiWhirlStartUnusedZ = 1.0f;
s32 btaiSwervePositive = 1;   /* 1: the swerve goes to +90 degrees */
s32 btaiWhirlPiecesEnd = 1;   /* 1 ends the whirlpools' pieces (tlb_0324_6) */
s32 btaiWhirlStartZ = 1;   /* the boss's z when the whirlpool began */
extern s32 btaiInkGone; /* 1: the ink is gone (tlb_0324_6) */
extern s32 btaiEffectsEnd; /* 1: the battle's effects end (tlb_0324_6) */
extern u8 btaiNormalAnim[];
extern u8 btaiStraightHitAnim[];
extern u8 btaiBiribiriRodAnim[];
extern u8 btaiJabHitNegXAnim[];
extern u8 btaiJabHitPosXAnim[];
extern u8 btaiMarbleShotAnim[];
extern u8 btaiHeadbuttAnim[];
extern u8 btaiHeadbuttStep1Anim[];
extern u8 btaiHeadbuttStep2Anim[];

/* The boss's hits: its attack box landing makes a splash; its damage box takes the Impact's
 * vulcan (15), the other Impact's vulcan (16), the jab (1), the straight (3), the hundred-fist
 * punch (5), the other Impact's attack (0xCA), the kick (8) and the cannon (11). */
/* Matched 2026-10-16; it was parked for one t-register too many (the timer's reload for its
 * test), gone with the timer counted down into a local, invincible, and tested there. */
void btaiHit(Obj *obj) {
    static s32 btaiDizzyHits = 0; /* hits taken while dizzy (action 20); 5 knock it down */
    f32 size;
    s32 invincible; /* the invincibility timer, counted down */
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[9];

    if (acollAtkWasHit(w->atk) == 1) {
        btaiTsbubbleMake(w->frame->pos[0], -50.0f, 200.0f);
    }
    invincible = --w->invTimer;
    if (invincible < 0) {
        w->invTimer = 0;
        w->dmg->flags |= 2;
    }
    PRINTF("無敵timer--->%d\r", w->invTimer);
    if (acollDmgWasHit(w->dmg) && !(gBossFlags & 0x4000)) {
        switch (w->dmg->hitKind) {
            case 15:
                PRINTF("タイサンバはバルカンに当たりました。[%d]\n", gBossLife);
                if (gBossLife <= 0 || w->action == 6) {
                    break;
                }
                if (gBossFlags & 4) {
                    itexFlash(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 0.2f);
                } else {
                    SoundLocation2(0x22B, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                   w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                    bossDamage(3, 15, 1);
                    btaiTsbubbleMake(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2]);
                    itexFlash(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                }
                if (gBossLife <= 0) {
                    w->action = 19;
                    w->endPattern = 0;
                    w->timer = 800;
                    w->step = 0;
                    btaiHitStun = 1;
                    btaiEffectsEnd = 1;
                    gBossLife = 0;
                    w->dmg->flags &= ~2;
                    w->invTimer = 30;
                }
                break;
            case 16:
                PRINTF("タイサンバは外部バルカンに当たりました。\n");
                if (gBossLife <= 0) {
                    break;
                }
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                if (gBossFlags & 4) {
                } else {
                    itexFlash(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 0.1f);
                    SoundLocation2(0x22B, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                   w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                    bossDamage(3, 15, 1);
                }
                bossDamage(1, 16, 1);
                if (gBossLife <= 0) {
                    w->endPattern = 0;
                    w->action = 19;
                    w->timer = 800;
                    w->step = 0;
                    btaiHitStun = 1;
                    btaiEffectsEnd = 1;
                    gBossLife = 0;
                    w->invTimer = 30;
                    w->dmg->flags &= ~2;
                }
                break;
            case 1:
                PRINTF("タイサンバはジャブに当たりました。\n");
                w->endPattern = 0;
                btaiHitStun = 1;
                w->invTimer = 16;
                beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                if (w->action != 8) {
                    bossDamage(20, 1, 1);
                    w->dmg->flags &= ~2;
                    SoundLocation2(0x27C, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                   w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                    if (w->frame->pos[0] < w->dmg->hitPos[0]) {
                        w->action = 15;
                    } else {
                        w->action = 14;
                    }
                    w->step = 0;
                } else {
                    beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 3.0f);
                    btaiHitStun = 0;
                }
                if (gBossLife <= 0) {
                    w->invTimer = 30;
                    w->action = 19;
                    w->timer = 800;
                    w->step = 0;
                    btaiHitStun = 1;
                    btaiEffectsEnd = 1;
                    gBossLife = 0;
                    w->dmg->flags &= ~2;
                }
                break;
            case 3:
                PRINTF("タイサンバはストレートに当たりました。\n");
                w->endPattern = 0;
                bossDamage(50, 3, 2);
                SoundLocation2(0x27D, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2],
                               300.0f, 15000.0f, gBattleCamView->fovy);
                w->dmg->flags &= ~2;
                w->invTimer = 30;
                if (w->action == 20) {
                    btaiDizzyHits++;
                }
                if (btaiDizzyHits >= 5) {
                    btaiDizzyHits = 0;
                    w->action = 22;
                } else {
                    w->action = 13;
                }
                w->timer = 800;
                w->step = 0;
                btaiHitStun = 1;
                break;
            case 5:
                PRINTF("タイサンバは百烈パンチに当たりました。\n");
                bossDamage(10, 5, 1);
                beHitmarkCreate(w->dmg->hitPos[0] + fadeRandRange(-200, 200),
                                w->dmg->hitPos[1] + fadeRandRange(-200, 200),
                                w->dmg->hitPos[2] + fadeRandRange(-200, 200), 3.0f);
                beHitmarkCreate(w->dmg->hitPos[0] + fadeRandRange(-200, 200),
                                w->dmg->hitPos[1] + fadeRandRange(-200, 200),
                                w->dmg->hitPos[2] + fadeRandRange(-200, 200), 3.0f);
                beHitringCreate(w->dmg->hitPos[0] + fadeRandRange(-200, 200),
                                w->dmg->hitPos[1] + fadeRandRange(-200, 200),
                                w->dmg->hitPos[2] + fadeRandRange(-200, 200), 1.0f, 0);
                w->action = 16;
                w->step = 0;
                w->timer = 800;
                gBattleFlags2 |= 0x2000;
                btaiHitStun = 1;
                if (gBattleSndToggles & 2) {
                    sndPlay(0x50E);
                    gBattleSndToggles &= ~2;
                } else {
                    sndPlay(0x50F);
                    gBattleSndToggles |= 2;
                }
                break;
            case 0xCA:
                PRINTF("タイサンバは外部インパクト攻撃に当たりました。\n");
                if (gBossLife <= 0) {
                    break;
                }
                w->endPattern = 0;
                beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                SoundLocation2(0x27D, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                               w->dmg->hitPos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                bossDamage(0, 0xCA, 0);
                w->dmg->flags &= ~2;
                w->invTimer = 30;
                w->action = 20;
                w->timer = 800;
                w->step = 0;
                btaiHitStun = 1;
                break;
            case 8:
                PRINTF("タイサンバはキックに当たりました。\n");
                w->endPattern = 0;
                beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 5.0f);
                bossDamage(80, 8, 3);
                sndPlay(0x102);
                btaiHitStun = 1;
                w->timer = 800;
                w->dmg->flags &= ~2;
                w->invTimer = 30;
                if (w->action == 20) {
                    btaiDizzyHits++;
                }
                if (btaiDizzyHits >= 5) {
                    btaiDizzyHits = 0;
                    w->action = 22;
                } else {
                    w->action = 17;
                }
                w->step = 0;
                break;
            case 11:
                PRINTF("タイサンバはんが砲に当たりました。\n");
                if (gBossLife > 0) {
                    size = 5.0f;
                    w->endPattern = 0;
                    beHitmarkCreate(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], size);
                    if (gBattleFlags2 & 0x10000) {
                        bossDamage(800, 11, 0);
                    } else {
                        bossDamage(400, 11, 0);
                    }
                    w->dmg->flags &= ~2;
                    w->timer = 800;
                    w->action = 18;
                    w->step = 0;
                    btaiWhirlPiecesEnd = 1;
                    btaiCannonStun = 1;
                    btaiHitStun = 1;
                }
                break;
        }
        if (w->dmg->hitKind != 11) {
            btaiWhirlPiecesEnd = 0;
        }
        if (gBossLife < 2) {
            btaiInkGone = 1;
        }
    }
}

/* after btaiHit's static (0x08019F04): .data is in source order */
s32 btaiSweepNegX = 1;   /* 1: the sweep runs toward -x */

s32 btaiNormal(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[10];

    PRINTF("ノーマル\n");
    PRINTF("ゆらゆら....\n");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiBank(obj, 0.3f);
    w->count += 2.0f;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + -6.0) {
        w->frame->pos[1] += 3.0f;
    }
    if (sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + 6.0 < w->frame->pos[1]) {
        w->frame->pos[1] -= 3.0f;
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    if (gGameFrameCount % 70 == 0) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 3.0f);
    }
    if (gGameFrameCount % 60 == 30) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 2.0f);
    }
    switch (w->step) {
        case 0:
            w->timer = 300;
            bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
            w->step = 1;
            w->speed = 0.0f;
            w->dmg->flags |= 2;
            break;
        case 1:
            if (gGameFrameCount % 20 == 0) {
                btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                       btaiRandSigned(10), btaiRandSigned(10), 3.0f);
            }
            if (!bossDampVel(obj, 0.98f)) {
                w->step = 2;
                w->timer = 300;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bossStepAnim(w->frame, 0.42f, 0);
            break;
        case 2:
            if (btaiTurnTo(obj, 0.0f, 0.0f, 6.0f)) {
                w->step = 3;
                w->timer = 90;
            }
            bossStepAnim(w->frame, 0.42f, 0);
            break;
        case 3:
            bossStepAnim(w->frame, 0.42f, 0);
            break;
    }
    return 0;
}

s32 btaiNormalSwayLong(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[11];

    PRINTF("ノーマルゆらゆら\n");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiBank(obj, 0.3f);
    w->count += 1.0f;
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
            w->step = 1;
            w->timer = 200;
            w->speed = 0.0f;
            w->dmg->flags |= 2;
            break;
        case 1:
            btaiSway(obj);
            break;
    }
    if (gGameFrameCount % 70 == 0) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 3.0f);
    }
    if (gGameFrameCount % 60 == 30) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 2.0f);
    }
    bossStepAnim(w->frame, 0.84f, 0);
    return 0;
}

s32 btaiNormalSwayMid(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[11];

    PRINTF("ノーマルゆらゆら\n");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiBank(obj, 0.3f);
    w->count += 1.0f;
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
            w->step = 1;
            w->timer = 100;
            w->speed = 0.0f;
            w->dmg->flags |= 2;
            break;
        case 1:
            btaiSway(obj);
            break;
    }
    if (gGameFrameCount % 70 == 0) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 3.0f);
    }
    if (gGameFrameCount % 60 == 30) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 2.0f);
    }
    bossStepAnim(w->frame, 0.84f, 0);
    return 0;
}

s32 btaiNormalSwayShort(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[11];

    PRINTF("ノーマルゆらゆら\n");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiBank(obj, 0.3f);
    w->count += 1.0f;
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
            w->step = 1;
            w->timer = 50;
            w->speed = 0.0f;
            w->dmg->flags |= 2;
            break;
        case 1:
            btaiSway(obj);
            break;
    }
    if (gGameFrameCount % 70 == 0) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 3.0f);
    }
    if (gGameFrameCount % 60 == 30) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 2.0f);
    }
    bossStepAnim(w->frame, 0.84f, 0);
    return 0;
}

s32 btaiNeutral(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[10];

    PRINTF("ニュートラル\n");
    PRINTF("ゆらゆら....\n");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiBank(obj, 0.5f);
    w->count += 2.0f;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + -6.0) {
        w->frame->pos[1] += 3.0f;
    }
    if (sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + 6.0 < w->frame->pos[1]) {
        w->frame->pos[1] -= 3.0f;
    }
    if (gGameFrameCount % 70 == 0) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 3.0f);
    }
    if (gGameFrameCount % 60 == 30) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 2.0f);
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->step) {
        case 0:
            w->timer = 300;
            bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
            w->step = 1;
            w->speed = 0.0f;
            w->dmg->flags |= 2;
            break;
        case 1:
            if (gGameFrameCount % 20 == 0) {
                btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                       btaiRandSigned(10), btaiRandSigned(10), 3.0f);
            }
            if (!bossDampVel(obj, 0.97f)) {
                w->step = 2;
                w->timer = 300;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 5.0f);
            bossStepAnim(w->frame, 0.42f, 0);
            break;
        case 2:
            if (btaiTurnTo(obj, 0.0f, 0.0f, 5.0f)) {
                w->timer = 0;
            }
            bossStepAnim(w->frame, 0.42f, 0);
            break;
    }
    return 0;
}

s32 btaiMarbleShot(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[12];

    PRINTF("マーブル弾\n");
    btaiMoveLimit(obj);
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + -6.0) {
        w->frame->pos[1] += 3.0f;
    }
    if (sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + 6.0 < w->frame->pos[1]) {
        w->frame->pos[1] -= 3.0f;
    }
    if (gGameFrameCount % 30 == 0) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 3.0f);
    }
    switch (w->step) {
        case 0:
            w->timer = 800;
            w->step = 1;
            bossSetAnimAt(w->frame, btaiMarbleShotAnim, 0xF8, 0.0f);
            btaiTurnTo(obj, 0.0f, 0.0f, 4.0f);
            break;
        case 1:
            w->count += 2.0f;
            bossStepAnim(w->frame, 0.42f, 2);
            if (w->timer < 780) {
                btaiTMarbleCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2]);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                w->step = 2;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 4.0f);
            break;
        case 2:
            w->count += 2.0f;
            if (bossStepAnim(w->frame, 0.42f, 2)) {
                w->timer = 0;
                w->speed = 0.0f;
                w->frame->rot[0] = 0.0f;
                w->frame->rot[2] = 0.0f;
            }
            if (!(gGameFrameCount & 7)) {
                btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                       btaiRandSigned(20), btaiRandSigned(20), 4.0f);
            }
            break;
    }
    return 0;
}

/* Matched 2026-10-12; it was parked for one stack slot, gone with the second test's height read
 * into an f64 local. */
s32 btaiMarbleBarrage(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 height;
    s32 pad[9];

    PRINTF("マーブル連射\n");
    btaiMoveLimit(obj);
    if (btaiRand(40) == 3) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(20), btaiRandSigned(20), 2.0f);
    }
    if (gGameFrameCount % 60 == 5) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(20), btaiRandSigned(20), 3.0f);
    }
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->vel[1] *= 0.9;
    w->vel[2] *= 0.95;
    if (w->frame->pos[2] > -2800.0f) {
        w->frame->pos[2] -= 2.0f;
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->step) {
        case 0:
            w->timer = 800;
            w->step = 1;
            bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
            sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 128);
            if (w->frame->pos[0] > 0.0f) {
                btaiSweepNegX = 1;
            } else {
                btaiSweepNegX = 0;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 20.0f);
            break;
        case 1:
            btaiTurnTo(obj, 0.0f, 0.0f, 7.0f);
            bossStepAnim(w->frame, 1.26f, 0);
            if (btaiSweepNegX == 1) {
                w->vel[0] -= 1.0f;
                if (w->vel[0] <= -25.0f) {
                    w->vel[0] = -25.0f;
                }
                if (w->frame->pos[0] <= -1800.0f) {
                    w->step = 2;
                    sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 160);
                }
                w->frame->rot[2] += 0.0174532930038800785;
                if (w->frame->rot[2] >= 0.523598790116402313) {
                    w->frame->rot[2] = 0.5235987902f;
                }
                if (w->vel[0] == -25.0f && w->timer % (32 - gBossPhase * 4) == 0) {
                    btaiTMarbleCreate(obj, w->frame->pos[0] - 300.0f, w->frame->pos[1] - 50.0f,
                                           w->frame->pos[2]);
                    btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                    btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                }
            } else {
                w->vel[0] += 1.0f;
                if (w->vel[0] >= 25.0f) {
                    w->vel[0] = 25.0f;
                }
                if (w->frame->pos[0] >= 1800.0f) {
                    w->step = 2;
                    sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 160);
                }
                w->frame->rot[2] -= 0.0174532930038800785;
                if (w->frame->rot[2] <= -0.523598790116402313) {
                    w->frame->rot[2] = -0.5235987902f;
                }
                if (w->vel[0] == 25.0f && w->timer % (32 - gBossPhase * 4) == 0) {
                    btaiTMarbleCreate(obj, w->frame->pos[0] + 300.0f, w->frame->pos[1] - 50.0f,
                                           w->frame->pos[2]);
                    btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                    btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                }
            }
            if (w->timer % 5 == 0) {
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 48);
            }
            break;
        case 2:
            btaiTurnTo(obj, 0.0f, 0.0f, 7.0f);
            bossStepAnim(w->frame, 0.63f, 0);
            if (btaiSweepNegX == 1) {
                w->vel[0] += 1.0f;
                if (w->vel[0] >= 0.0f) {
                    w->vel[0] = 0.0f;
                }
            } else {
                w->vel[0] -= 1.0f;
                if (w->vel[0] <= 0.0f) {
                    w->vel[0] = 0.0f;
                }
            }
            w->frame->rot[2] *= 0.95;
            w->frame->rot[0] *= 0.95;
            if (w->vel[0] == 0.0f) {
                w->step = 3;
                w->timer = 0;
            }
            break;
    }
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0 + -6.0) {
        w->frame->pos[1] += 3.0f;
    }
    height = sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0;
    if (height + 6.0 < w->frame->pos[1]) {
        w->frame->pos[1] -= 3.0f;
    }
    return 0;
}

s32 btaiBiribiriRod(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[12];

    PRINTF("ビリビリ棒\n");
    btaiMoveLimit(obj);
    w->frame->pos[0] *= 0.98;
    w->frame->pos[2] *= 0.98;
    w->vel[1] *= 0.95;
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->step) {
        case 0:
            w->timer = 800;
            w->step = 1;
            bossSetAnimAt(w->frame, btaiBiribiriRodAnim, 0xF8, 0.0f);
            break;
        case 1:
            if (w->timer == 767) {
                btaiDdmybombCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 500.0f, 5, 10,
                                       52);
            }
            if (bossStepAnim(w->frame, 0.42f, 2)) {
                w->step = 2;
                w->timer = 100;
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
            } else {
                btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            }
            break;
        case 2:
            if (w->timer > 90) {
                bossStepAnim(w->frame, 0.14f, 3);
                w->timer = 0;
                w->count = 0.0f;
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
            }
            break;
    }
    return 0;
}

/* The charge: the boss runs at the Impact (step 1) until it is within 3800, keeps running (3)
 * to within 2600, swerves round it sideways (4, the side from its heading), comes in again (5)
 * to within 1100 and bites (6), then rolls over (7). btaiSwervePositive and btaiRushFeintCount are
 * function-local statics (a lui per access, read by no other function; their words sit in
 * tlb_0324_4.data.s). The game wraps the cockpit's head yaw into 0..2 pi and never reads it:
 * its test's branch stays, empty (lui, lwc1, c.le.s, bc1fl with the next test in its delay
 * slot). The local "yaw" read by the FAKE `if (yaw) {}` gives that (2026-10-12, from
 * 13 differing lines and 6 instructions short; IDO_NOTES 204's mechanism); without a reader,
 * uopt drops the test (an empty block, a dead PRINTF, an expression statement all vanish).
 * Misses: then-blocks `angle = angle;`, a label, `angle -= 0.0f` (13 each); the reader at
 * the function's end (282, yaw lives across the calls); the yaw load after the angle test (12,
 * one block late).
 * Parked again 2026-10-12: check matches, but its two statics then come out
 * together at the function, where the game's .data has four words between them (0x08019EF8,
 * 0x08019F0C; tlb_0324_1's .data 0x10 off in the build). Its .data order is open.
 * 2026-10-12 (second try), bsswrap's .data view: IDO emits .data in source order (globals where they
 * are defined, a function's initialised statics at the function), the same at 0 and 145 extra
 * symbol numbers, so .data is not hashed as .bss is. The game's order (EF4, this function's
 * EF8, two globals, btaiHit's dizzy count at F04, a global, this function's F0C) fits
 * no source order of these definitions: btaiHit is the file's first function. As file
 * statics at their places: real 169 (IDO holds their addresses).
 * SOLVED 2026-10-12: only F0C is this function's static. btaiSwervePositive (EF8) is a
 * file-scope global (with it global and F0C static, check matched at once: the 169 was F0C's),
 * and the marble words (F10, F14) are defined after this function, just before
 * btaiTorpedoAttack, which uses them. IDO emits .data in source order, so the layout is the game's
 * (a probe: declaration order, at the function's place). */
s32 btaiRushFeint(Obj *obj) {
    static f32 btaiRushFeintCount = 0.0f;
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad0;
    f32 angle;
    s32 pad1;
    f32 dist;
    s32 pad[10];
    f32 yaw;

    PRINTF("突っ込みフェイント\n");
    PRINTF("よける1 %d\n");
    PRINTF("よける2 %d\n");
    PRINTF("%d\n");
    PRINTF("すすむ2\n");
    PRINTF("\rtimer ->[%d]");
    PRINTF("がつん！\n");
    PRINTF("STEP %d\n");
    btaiMoveLimit(obj);
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    dist = w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[0] * w->frame->pos[0];
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    if ((s32) gGameFrameCount % 6 == 0) {
        if ((s32) w->frame->frame < 16) {
            btaiTzanzouMake(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 20.0f,
                                   w->frame->rot[0], w->frame->rot[1], w->frame->rot[2], w->frame->frame);
        }
    }
    w->vel[1] = 0.0f;
    angle = w->frame->rot[1];
    yaw = gCockpitHeadYaw;
    if (angle >= 6.2831855f) {
        angle -= 6.2831855f;
    }
    if (yaw >= 6.2831855f) {
        yaw -= 6.2831855f;
    }
    if (angle < 0.0f) {
        angle += 6.2831855f;
    }
    if (yaw) {} /* FAKE: yaw read once, so its test's branch stays and its store goes (IDO_NOTES 204) */
    btaiRushFeintCount += 3.0f;
    if (w->frame->frame < 7.0) {
        bossStepAnim(w->frame, 0.42f, 2);
    }
    switch (w->step) {
        case 0:
            w->timer = 0xAF0;
            w->count = 0.0f;
            btaiRushFeintCount = 0.0f;
            w->step = 1;
            w->speed = 0.0f;
            bossSetAnimAt(w->frame, btaiBiribiriRodAnim, 0xF8, 0.0f);
            btaiPushStartVelX = w->vel[0];
            btaiPushStartVelZ = w->vel[2];
            btaiTurnTo(obj, 0.0f, 0.0f, 10.0f);
            gBossFlags |= 4;
            break;
        case 1:
            w->count += 1.0f;
            if (dist < 14440000.0f) {
                w->step = 3;
                w->count = 0.0f;
                w->timer = 800;
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 10.0f, 240);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                btaiPushStartVelX = w->vel[0];
                btaiPushStartVelZ = w->vel[2];
                if (angle > 3.1415927f) {
                    btaiSwervePositive = 0;
                } else {
                    btaiSwervePositive = 1;
                }
            }
            w->speed += 0.8;
            if (w->speed > gBossPhase * 7 + 70) {
                w->speed = gBossPhase * 7 + 70;
            }
            btaiPushVelZ = -cosf(w->frame->rot[1] + 3.1415927f) * w->speed;
            btaiPushVelX = -sinf(w->frame->rot[1] + 3.1415927f) * w->speed;
            btaiEaseVel(obj);
            btaiBank(obj, 1.0f);
            btaiTurnTo(obj, 0.0f, 0.0f, 3.0f);
            if (w->timer % 20 == 0) {
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 8.0f, 64);
            }
            break;
        case 3:
            w->count += 1.0f;
            w->speed += 0.8;
            if (w->speed > gBossPhase * 7 + 70) {
                w->speed = gBossPhase * 7 + 70;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            btaiPushVelZ = -cosf(w->frame->rot[1] + 3.1415927f) * w->speed;
            btaiPushVelX = -sinf(w->frame->rot[1] + 3.1415927f) * w->speed;
            btaiEaseVel(obj);
            btaiBank(obj, 1.0f);
            if (dist < 6760000.0f) {
                w->timer = 800;
                w->count = 0.0f;
                w->step = 4;
                btaiPushStartVelX = w->vel[0];
                btaiPushStartVelZ = w->vel[2];
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 10.0f, 240);
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                if (angle > 3.1415927f) {
                    btaiSwervePositive = 1;
                } else {
                    btaiSwervePositive = 0;
                }
                SoundLocation2(0x2B5, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            if (w->timer % 10 == 0) {
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 8.0f, 64);
            }
            break;
        case 4:
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            w->speed = (gBossPhase * 7 + 70) / 1.5;
            if (w->timer < 793) {
                w->timer = 800;
                w->step = 5;
                btaiPushStartVelX = w->vel[0];
                btaiPushStartVelZ = w->vel[2];
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 10.0f, 240);
            }
            if (w->timer % 10 == 0) {
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 8.0f, 64);
            }
            if (btaiSwervePositive == 0) {
                w->vel[2] = -cosf(w->frame->rot[1] - 1.5707964f) * w->speed;
                w->vel[0] = -sinf(w->frame->rot[1] - 1.5707964f) * w->speed;
            } else {
                w->vel[2] = -cosf(w->frame->rot[1] + 1.5707964f) * w->speed;
                w->vel[0] = -sinf(w->frame->rot[1] + 1.5707964f) * w->speed;
            }
            btaiBank(obj, 1.0f);
            break;
        case 5:
            btaiBank(obj, 1.0f);
            w->count += 1.0f;
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            w->speed += 0.8;
            if (w->speed > gBossPhase * 7 + 70) {
                w->speed = gBossPhase * 7 + 70;
            }
            btaiPushVelZ = -cosf(w->frame->rot[1] + 3.1415927f) * w->speed;
            btaiPushVelX = -sinf(w->frame->rot[1] + 3.1415927f) * w->speed;
            btaiEaseVel(obj);
            if (dist < 1210000.0f) {
                w->timer = 800;
                w->count = 0.3f;
                w->step = 6;
                w->frame->rot[2] = 0.0f;
                w->frame->rot[0] = 0.0f;
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 10.0f, 240);
            }
            if (w->timer % 10 == 0) {
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 8.0f, 64);
            }
            break;
        case 6:
            if ((s32) w->frame->frame == 15) {
                btaiDdmybombCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 700.0f, 3, 10,
                                       45);
                w->frame->frame += 1.0f;
                w->count = 0.05f;
                gBossFlags &= ~4;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bossDampVel(obj, 0.93f);
            btaiBank(obj, 0.4f);
            w->frame->rot[2] = 0.0f;
            w->frame->rot[0] = 0.0f;
            if (bossStepAnim(w->frame, w->count, 2)) {
                w->speed = 0.0f;
                w->step = 7;
                w->timer = 0;
                w->frame->rot[0] += 0.00349065860077601596;
            }
            if (w->frame->pos[1]-- > -600.0f) {
                w->frame->pos[1] = -600.0f;
            }
            break;
        case 7:
            w->frame->rot[0] += 0.00349065860077601596;
            w->count = 0.0f;
            break;
    }
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.1415927f) * 20.0 + -400.0 + -6.0) {
        w->frame->pos[1] += 3.0f;
    }
    if (sinf(w->count / 180.0f * 3.1415927f) * 20.0 + -400.0 + 6.0 < w->frame->pos[1]) {
        w->frame->pos[1] -= 3.0f;
    }
    return 0;
}

/* tlb_0324_7's: an eel from a side. */
#define btaiUtsuboCreate (*btaiUtsuboCreate)
/* tlb_0324_6's whirlpools' pieces and their effects. */
#define btaiUzu1Make (*btaiUzu1Make)
#define btaiUzuInMake (*btaiUzuInMake)
#define btaiUzu2Make (*btaiUzu2Make)
#define btaiUzuSnaMake (*btaiUzuSnaMake)
#define btaiSunaEf2Make (*btaiSunaEf2Make)

extern s32 btaiEelInterval; /* steps between eels (200, down to 50) */
extern f32 btaiWhirl2StartSpeed; /* the second whirlpool's start's speed toward the cockpit */
extern f32 btaiWhirlStartUnusedX;
extern f32 btaiWhirlStartUnusedY;
extern f32 btaiWhirlStartUnusedZ;
extern s32 btaiWhirlStartZ; /* the boss's z when the whirlpool began */
extern s32 btaiWhirlBubbles;
extern s32 btaiWhirl1FadeCount; /* the first whirlpool's count where it fades out */
extern f32 btaiWhirl1Scale; /* its pieces' scale */
extern s32 btaiWhirl1Count; /* its count */
extern s32 btaiWhirl1Life; /* its pieces' life */
extern f32 btaiWhirl2Scale; /* the second whirlpool's pieces' scale */
extern f32 btaiWhirl2Turn; /* its turning speed, in degrees */
extern f32 btaiWhirl1Turn; /* the first's turning speed, in degrees */
extern f32 btaiWhirl1Grow; /* the first's pieces' growth */
extern s32 btaiWhirl3Count; /* the third's count */
extern s32 btaiWhirl2Count; /* the second's count */
extern s32 btaiWhirlAlphaMax; /* the most alpha a piece starts with */
extern s32 btaiWhirlAlphaMaxUnused;
extern f32 btaiWhirlPoint2; /* the second point's x, y, z */
extern f32 btaiWhirlPoint2Y;
extern f32 btaiWhirlPoint2Z;
extern f32 btaiWhirlStartCentre; /* the start's centre */
extern f32 btaiWhirlStartCentreY;
extern f32 btaiWhirlStartCentreZ;
extern f32 btaiWhirlEndCentre; /* the end's centre */
extern f32 btaiWhirlEndCentreY;
extern f32 btaiWhirlEndCentreZ;
extern s32 btaiEelsMade; /* eels made (tlb_0324_7) */
extern u8 btaiWhirlpoolAttackAnim[];
extern u8 btaiWhirlpoolAttackStep1Anim[];
extern u8 btaiWhirlpoolAttackStep6Anim[];

/* The whirlpool attack: the boss turns to face the Impact (1), swims to (0, -600, -3500) (2),
 * sets the whirlpools going and sinks while eels come out (3), then comes back up behind the
 * cockpit (4-8). Its float literals are written as ints (-3500, -600, 0, `* 1`, `* 2`,
 * `- 1500`, case 3's 2s); cases 5-8 keep `2.0f`.
 * Matched 2026-10-17; it was parked for where the constants -3500 and 2.0 sat in the float
 * registers. */
s32 btaiWhirlpoolAttack(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f32 turn;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 pad[12];

    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * 1;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * 1;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiMoveLimit(obj);
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    switch (w->step) {
        case 0:
            PRINTF("渦巻き攻撃\n");
            bossSetAnimAt(w->frame, btaiWhirlpoolAttackAnim, 0xF8, 0.0f);
            w->timer = 900;
            w->step = 1;
            gBrotherFlags |= 0x20000;
            break;
        case 1:
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            if (bossStepAnim(w->frame, 0.21f, 2)) {
                bossSetAnimAt(w->frame, btaiWhirlpoolAttackStep1Anim, 0xF8, 0.0f);
                w->step = 2;
            }
            break;
        case 2:
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            dx = (0 - w->frame->pos[0]) / 60.0f;
            dy = (-600 - w->frame->pos[1]) / 60.0f;
            dz = (-3500 - w->frame->pos[2]) / 60.0f;
            if (dx > 0.0f) {
                w->vel[0] += 0.6;
            } else {
                w->vel[0] -= 0.6;
            }
            if (dy > 0.0f) {
                w->vel[1] += 0.3;
            } else {
                w->vel[1] -= 0.3;
            }
            if (dz > 0.0f) {
                w->vel[2] += 0.3;
            } else {
                w->vel[2] -= 0.3;
            }
            if (w->vel[0] > 30.0f) {
                w->vel[0] = 30.0f;
            }
            if (w->vel[0] < -30.0f) {
                w->vel[0] = -30.0f;
            }
            if (w->vel[2] > 8.0f) {
                w->vel[2] = 8.0f;
            }
            if (w->vel[2] < -8.0f) {
                w->vel[2] = -8.0f;
            }
            if (w->vel[1] > 8.0f) {
                w->vel[1] = 8.0f;
            }
            if (w->vel[1] < -8.0f) {
                w->vel[1] = -8.0f;
            }
            if (w->frame->rot[0] < w->vel[2] * 2 * 0.0174532930038800785 - 0.0698131720155203139) {
                w->frame->rot[0] += 0.034906586007760157;
            }
            if (w->frame->rot[0] > w->vel[2] * 2 * 0.0174532930038800785 + 0.0698131720155203139) {
                w->frame->rot[0] -= 0.034906586007760157;
            }
            if (w->frame->rot[2] < -w->vel[0] * 2 * 0.0174532930038800785 - 0.0698131720155203139) {
                w->frame->rot[2] += 0.034906586007760157;
            }
            if (w->frame->rot[2] > -w->vel[0] * 2 * 0.0174532930038800785 + 0.0698131720155203139) {
                w->frame->rot[2] -= 0.034906586007760157;
            }
            if (w->frame->rot[0] > 0.523598790116402313) {
                w->frame->rot[0] = 0.5235987902f;
            }
            if (w->frame->rot[0] < -0.523598790116402313) {
                w->frame->rot[0] = -0.5235987902f;
            }
            if (w->frame->rot[2] > 0.523598790116402313) {
                w->frame->rot[2] = 0.5235987902f;
            }
            if (w->frame->rot[2] < -0.523598790116402313) {
                w->frame->rot[2] = -0.5235987902f;
            }
            bossStepAnim(w->frame, 0.42f, 0);
            if (((w->frame->pos[0] > 0.0f) ? w->frame->pos[0] : -w->frame->pos[0]) < 200.0f
                && ((w->frame->pos[2] > -3500.0f) ? w->frame->pos[2] - -3500.0f
                                                   : -(w->frame->pos[2] - -3500.0f)) < 200.0f) {
                w->timer = 0xAF0;
                btaiWhirlPoint2 = w->frame->pos[0];
                btaiWhirlPoint2Z = w->frame->pos[2] + 300.0f;
                btaiWhirlPoint2Y = -800.0f;
                w->step = 3;
                w->count = 0.0f;
                btaiWhirl1Count = 0;
                btaiWhirl1FadeCount = 200;
                btaiWhirl2Count = 0;
                btaiWhirl1Turn = 0.0f;
                btaiWhirl2Turn = 0.0f;
                btaiWhirl1Scale = 3.0f;
                btaiWhirl2Scale = 0.01f;
                btaiWhirl1Life = 60;
                btaiWhirlBubbles = 0;
                btaiWhirl2StartSpeed = 0.0f;
                btaiWhirlStartCentre = w->frame->pos[0];
                btaiWhirlStartCentreY = 2000.0f;
                btaiWhirlStartCentreZ = w->frame->pos[2] - 600.0f;
                btaiWhirlStartZ = w->frame->pos[2];
                btaiWhirlEndCentre = 0.0f;
                btaiWhirlEndCentreZ = 0.0f;
                btaiWhirlEndCentreY = 0.0f;
                btaiWhirl3Count = 0;
                btaiWhirl1Grow = 0.005f;
                btaiWhirlAlphaMax = 255;
                btaiWhirlAlphaMaxUnused = 255;
                btaiEelsMade = 0;
                gBattleFlags2 |= 1;
                sndPlay(0x2F5);
                sndPlay(0x24E);
                btaiWhirlStartUnusedX = btaiWhirlStartUnusedY = btaiWhirlStartUnusedZ = 0.0f;
                btaiEelInterval = 200;
            }
            break;
        case 3:
            w->vel[0] *= 0.93;
            w->vel[2] *= 0.93;
            w->vel[1] *= 0.93;
            w->frame->pos[0] *= 0.98;
            w->count += 1.0f;
            gBrotherFlags |= 0x20000;
            w->frame->rot[2] *= 0.94;
            w->frame->rot[0] *= 0.94;
            turn = w->count / 600.0f + 0.1;
            if (turn > 0.4) {
                turn = 0.4f;
                w->frame->scale[0] -= 0.003;
                w->frame->scale[1] -= 0.003;
                w->frame->scale[2] -= 0.003;
                if (w->frame->scale[0] < 1.0) {
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.0f;
                }
                btaiWhirl1Scale -= 0.008;
                if (btaiWhirl1Scale < 0.1) {
                    btaiWhirl1Scale = 0.1f;
                }
                btaiWhirlBubbles = 0;
            }
            if (w->count == 300.0f) {
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.7f;
                w->frame->pos[2] = -3500.0f;
                w->frame->f.flags &= ~2;
            }
            if (w->frame->pos[2] > -3500.0f) {
                w->frame->pos[2] -= 2;
            }
            if (w->frame->pos[2] < -3500.0f) {
                w->frame->pos[2] += 2;
            }
            if (w->count > 300.0f) {
                btaiWhirlPoint2 *= 0.94;
                w->frame->pos[0] *= 0.94;
            }
            if (w->count > 480.0f) {
                if (w->timer % btaiEelInterval == 0 && w->timer > 1500) {
                    btaiUtsuboCreate(0);
                    if (btaiEelInterval > 50) {
                        btaiEelInterval -= 8;
                    }
                }
            }
            if (btaiWhirl1Count < 500) {
                if (w->count < 500.0f) {
                    bossStepAnim(w->frame, turn * 2.09999999999999964, 0);
                }
                btaiWhirl1Count++;
                btaiWhirl1Turn = turn * 20.0f;
                if (btaiWhirl1Count % 7 == 0) {
                    btaiUzu1Make();
                }
            }
            if (btaiWhirl1Count == 180) {
                sndPlay(0x2F6);
            }
            if (btaiWhirl1Count > 180) {
                btaiWhirl3Count++;
                if (btaiWhirl2StartSpeed < 12.0f) {
                    btaiWhirl2StartSpeed += 0.04;
                }
                btaiWhirlStartCentreY -= btaiWhirl2StartSpeed;
                btaiWhirlStartCentre *= 0.98;
                btaiWhirlStartCentreZ -= 2;
                if (btaiWhirlStartCentreZ < -4000.0f) {
                    btaiWhirlStartCentreZ = -4000.0f;
                }
                if (btaiWhirlStartCentreY < 0.0f) {
                    btaiWhirlStartCentreY = 0.0f;
                }
                if (btaiWhirl3Count % 14 == 0 && btaiWhirlStartCentreZ < gMegaShotPosZ) {
                    btaiUzuInMake();
                }
                if (w->timer - 1200 < 255) {
                    btaiWhirlAlphaMax = w->timer - 1200;
                }
            }
            if (w->timer == 1500) {
                sndPlay(0x2F7);
            }
            if (w->timer < 1500) {
                btaiWhirl2Count++;
                btaiWhirl2Turn = turn * 20.0f;
                if (btaiWhirl2Count % 7 == 0) {
                    btaiUzu2Make();
                }
                if (btaiWhirl2Scale < 3.0f) {
                    btaiWhirl2Scale += 0.003;
                }
                if (w->timer % 50 == 6) {
                    btaiUzuSnaMake();
                }
            }
            if (btaiWhirl1Count < 200) {
                if (w->timer % 50 == 6) {
                    btaiUzuSnaMake();
                }
                if (w->timer % 50 == 20) {
                    sunaEfMake(0.0f, 0.0f, -100.0f, 0.0f, 0.0f, 10.0f, 240);
                }
            }
            if (btaiWhirl1Count == 200) {
                btaiSunaEf2Make(0.0f, 0.0f, -2500.0f, 0.0f, 0.0f, 10.0f, 2000);
            }
            if (btaiWhirl1Count == 260) {
                btaiSunaEf2Make(0.0f, 0.0f, -2500.0f, 0.0f, 0.0f, 10.0f, 2000);
            }
            if (w->timer < 1200) {
                w->step = 4;
                w->timer = 2000;
                sndPlay(0x51A);
            }
            break;
        case 4:
            btaiWhirlBubbles = 1;
            w->timer = 200;
            w->step = 5;
            bossSetAnimAt(w->frame, btaiWhirlpoolAttackStep1Anim, 0xF8, 0.0f);
            w->count = 0.0f;
            w->frame->flags |= 0x20;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = -600.0f;
            w->frame->pos[2] = btaiWhirlStartZ - 1500;
            w->frame->f.flags |= 2;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.7f;
            gBrotherFlags &= ~0x20000;
            break;
        case 5:
            w->count += 2.0f;
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0;
            btaiWhirl2Count += 2;
            if (btaiWhirl2Count % 7 == 0) {
                btaiUzu2Make();
            }
            if (btaiWhirl2Count % 50 == 6) {
                btaiUzuSnaMake();
            }
            if (btaiWhirl2Count % 50 == 20) {
                sunaEfMake(0.0f, 0.0f, -100.0f, 0.0f, 0.0f, 10.0f, 240);
            }
            gBattleFlags2 &= ~1;
            bossStepAnim(w->frame, 0.42f, 0);
            if (w->timer < 80) {
                w->step = 6;
            }
            break;
        case 6:
            w->count += 2.0f;
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0;
            btaiWhirl2Count += 2;
            if (btaiWhirl2Count % 7 == 0) {
                btaiUzu2Make();
            }
            if (btaiWhirl2Count % 50 == 6) {
                btaiUzuSnaMake();
            }
            if (btaiWhirl2Count % 50 == 20) {
                sunaEfMake(0.0f, 0.0f, -100.0f, 0.0f, 0.0f, 10.0f, 240);
            }
            gBattleFlags2 &= ~1;
            if (bossStepAnim(w->frame, 0.42f, 2)) {
                bossSetAnimAt(w->frame, btaiWhirlpoolAttackStep6Anim, 0xF8, 0.0f);
                w->step = 7;
            }
            break;
        case 7:
            w->count += 2.0f;
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0;
            btaiWhirl2Count += 2;
            if (btaiWhirl2Count % 7 == 0) {
                btaiUzu2Make();
            }
            if (btaiWhirl2Count % 50 == 6) {
                btaiUzuSnaMake();
            }
            if (btaiWhirl2Count % 50 == 20) {
                sunaEfMake(0.0f, 0.0f, -100.0f, 0.0f, 0.0f, 10.0f, 240);
            }
            gBattleFlags2 &= ~1;
            if (bossStepAnim(w->frame, 0.21f, 2)) {
                bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
                w->step = 8;
            }
            break;
        case 8:
            w->count += 2.0f;
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 80.0 + -600.0;
            bossStepAnim(w->frame, 0.42f, 0);
            btaiWhirl2Count += 2;
            if (btaiWhirl2Count % 7 == 0) {
                btaiUzu2Make();
            }
            break;
    }
    return 0;
}

/* The marble volley's words (.data 0x08019F10, 0x08019F14): defined here, after
 * btaiRushFeint's static (0x08019F0C), as IDO emits .data in source order (IDO_NOTES 374). */
s32 btaiMarblesShot = 0; /* marbles shot in this volley */
s32 btaiMarbleOut = 0;   /* 1: this animation loop's marble is out */

s32 btaiTorpedoAttack(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[13];

    PRINTF("イカス魚雷\n");
    btaiMoveLimit(obj);
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 60.0 + -600.0;
    if (gGameFrameCount % 30 == 0) {
        btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                               btaiRandSigned(10), btaiRandSigned(10), 3.0f);
    }
    switch (w->step) {
        case 0:
            btaiMarblesShot = 0;
            btaiMarbleOut = 0;
            w->timer = 799;
            w->step = 1;
            bossSetAnimAt(w->frame, btaiMarbleShotAnim, 0xF8, 0.0f);
            break;
        case 1:
            w->count += 2.0f;
            w->vel[0] *= 0.97;
            w->vel[2] *= 0.97;
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            if (gBossPhase * 2 + 4 < btaiMarblesShot) {
                w->step = 2;
            }
            if (bossStepAnim(w->frame, (0.26 - gBossPhase / 10) * 2.09999999999999964, 2)) {
                bossSetAnimAt(w->frame, btaiMarbleShotAnim, 0xF8, 0.0f);
                btaiMarbleOut = 0;
            }
            if ((s32) (w->frame->frame * 10.0f) >= 31 && btaiMarbleOut == 0) {
                btaiIkaBombCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                       180 / (gBossPhase * 2 + 4) * btaiMarblesShot);
                SoundLocation2(0x21C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                btaiMarblesShot++;
                btaiMarbleOut = 1;
                btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
                w->frame->frame += 0.1;
            }
            break;
        case 2:
            w->count += 2.0f;
            if (bossStepAnim(w->frame, 0.42f, 2)) {
                w->timer = 0;
                w->speed = 0.0f;
                w->frame->rot[0] = 0.0f;
                w->frame->rot[2] = 0.0f;
            }
            if (!(gGameFrameCount & 7)) {
                btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                       btaiRandSigned(20), btaiRandSigned(20), 4.0f);
            }
            break;
    }
    return 0;
}

/* The rush: the boss moves by its velocity, bobs (btaiRushBob, a function static: IDO_NOTES
 * 343's lui at every use) and walks its steps. Only z is a local; x is read through w->frame
 * in the distance and taken again by the "+=", which gives the game's x + vel in f2. */
s32 btaiHeadbutt(Obj *obj) {
    static s32 btaiRushBob = 0;
    s32 pad2;
    f32 dist;
    BtaiW *w = (BtaiW *) (obj + 1);
    f32 z;
    s32 pad[10];

    PRINTF("頭突き\n");
    PRINTF("dist %d\n");
    btaiMoveLimit(obj);
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    z = w->frame->pos[2];
    dist = sqrtf(w->frame->pos[0] * w->frame->pos[0] + z * z);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    if ((s32) gGameFrameCount % 5 == 0 && w->step == 2) {
        btaiTzanzou2Make(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2] - 20.0f, w->frame->rot[0],
                               w->frame->rot[1], w->frame->rot[2], w->frame->frame, w->step);
    }
    btaiRushBob = btaiRushBob + 3;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->step) {
        case 0:
            w->timer = 2800;
            w->step = 1;
            w->speed = 0.0f;
            bossSetAnimAt(w->frame, btaiHeadbuttAnim, 0xF8, 0.0f);
            btaiPushStartVelX = w->vel[0];
            btaiPushStartVelZ = w->vel[2];
            gBossFlags |= 4;
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            break;
        case 1:
            btaiTurnTo(obj, 0.0f, 0.0f, 4.0f);
            if (bossStepAnim(w->frame, 0.126f, 2)) {
                bossSetAnimAt(w->frame, btaiHeadbuttStep1Anim, 0xF8, 0.0f);
                w->step = 2;
                SoundLocation2(0x2B4, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 500.0f,
                               18000.0f, gBattleCamView->fovy);
            }
            break;
        case 2:
            w->count += 1.0f;
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            w->speed += 3.0f;
            if ((f32) (gBossPhase * 12 + 150) < w->speed) {
                w->speed = gBossPhase * 12 + 150;
            }
            btaiPushVelZ = -cosf(w->frame->rot[1] + 3.1415927f) * w->speed;
            btaiPushVelX = -sinf(w->frame->rot[1] + 3.1415927f) * w->speed;
            btaiEaseVel(obj);
            if (w->timer % 20 == 0) {
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 8.0f, 64);
            }
            bossStepAnim(w->frame, 1.05f, 0);
            if (dist < 1000.0f) {
                w->step = 3;
                btaiDdmybombCreate(obj, 0.0f, 0.0f, -500.0f, 600.0f, 5, 10, 50);
                bossSetAnimAt(w->frame, btaiHeadbuttStep2Anim, 0xF8, 0.0f);
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 7.0f, 240);
                w->vel[0] = -w->vel[0];
                w->vel[2] = -w->vel[2];
                gBossFlags &= ~4;
            }
            break;
        case 3:
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bossDampVel(obj, 0.96f);
            if (bossStepAnim(w->frame, 0.063f, 2)) {
                w->timer = 0;
            }
            break;
    }
    if (w->frame->pos[1] < sinf((f32) (btaiRushBob / 180) * 3.1415927f) * 40.0 + -500.0) {
        w->frame->pos[1] += 12.0f;
    } else {
        w->frame->pos[1] -= 12.0f;
    }
    return 0;
}

/* The file's .data after btaiHeadbutt's static (declaration order, IDO_NOTES 343). */
s32 btaiRush2Places[4][2] = { { -2500, -7000 }, { 800, -7500 }, { 2500, -7000 }, { -400, -7500 } }; /* four places (x, z) for the other rush */
/* btaiRush2Index, the last of them, is btaiMoveBack's static (defined there, next in .data); the
 * words after it are defined after that function (.data is in definition order, IDO_NOTES 367). */
extern s32 btaiRush2FarBack;

/* Matched 2026-10-12; it was parked for btaiRush2Index's address held in a register, gone with
 * the static counted in place and the targets read back from w for the distance. */
s32 btaiMoveBack(Obj *obj) {
    static s32 btaiRush2Index = 0;
    BtaiW *w = (BtaiW *) (obj + 1);
    f32 x;
    f32 z;
    s32 pad[12];

    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    x = w->frame->pos[0] - sinf(w->frame->rot[1] - 3.1415927f) * 50.0f;
    z = w->frame->pos[2] - cosf(w->frame->rot[1] - 3.1415927f) * 50.0f;
    if (gGameFrameCount % 60 == 0) {
        btaiTbubbleMake(obj, x, w->frame->pos[1] + 510.0f, z, btaiRandSigned(20),
                               btaiRandSigned(20), 2.0f);
    }
    PRINTF("奥へ移動\n");
    switch (w->step) {
        case 0:
            btaiRush2Index++;
            w->timer = 500;
            w->speed = 0.0f;
            w->count = 0.0f;
            if (btaiRush2Index == 4) {
                btaiRush2Index = 0;
            }
            w->target[0] = btaiRush2Places[btaiRush2Index][0];
            w->target[2] = btaiRush2Places[btaiRush2Index][1];
            if (bossDistSqXZ(w->target[0], w->target[2], w->frame->pos[0], w->frame->pos[2]) < 1000000.0f) {
                w->timer = 0;
            }
            w->target[0] += btaiRandSigned(100);
            w->target[2] += btaiRandSigned(100);
            bossSetAnimAt(w->frame, btaiHeadbuttAnim, 0xF8, 0.0f);
            if (w->frame->pos[2] < -5000.0f) {
                btaiRush2FarBack = 1;
            } else {
                btaiRush2FarBack = 0;
            }
            w->step = 1;
            break;
        case 1:
            if (bossStepAnim(w->frame, 0.21f, 2) == 1) {
                SoundLocation2(0x2B6, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                w->step = 2;
                bossSetAnimAt(w->frame, btaiHeadbuttStep1Anim, 0xF8, 0.0f);
                btaiTurnTo(obj, w->target[0], w->target[2], 6.0f);
            } else {
                btaiTurnTo(obj, w->target[0], w->target[2], 4.0f);
                if (btaiRand(20) == 3) {
                    btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                           btaiRandSigned(20), btaiRandSigned(20), 2.5f);
                }
            }
            break;
        case 2:
            if (btaiRush2FarBack == 0) {
                if (!btaiBmoveX(obj, w->target[0], w->target[1] - 600, w->target[2], 1.3f)) {
                    w->step = 3;
                    bossSetAnimAt(w->frame, btaiHeadbuttStep2Anim, 0xF8, 0.0f);
                    sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 4.0f, 160);
                    w->count = 0.0f;
                } else {
                    bossStepAnim(w->frame, (25.0f - w->speed / 3.0f) / 7 * 0.7, 0);
                }
            } else if (!btaiBmoveXFast(obj, w->target[0], w->target[1] - 600, w->target[2], 1.3f)) {
                w->step = 3;
                bossSetAnimAt(w->frame, btaiHeadbuttStep2Anim, 0xF8, 0.0f);
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, (25.0f - w->speed / 3.0f) / 7 * 0.7, 0);
            }
            break;
        case 3:
            if (bossStepAnim(w->frame, 0.21f, 2)) {
                w->step = 4;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 2.0f);
            bossDampVel(obj, 0.97f);
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 60.0 + -600.0;
            w->count += 2.0f;
            break;
        case 4:
            w->timer = 0;
            break;
    }
    if (w->frame->pos[1] < -606.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > -594.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    return 0;
}

/* The .data after btaiMoveBack's static. */
s32 btaiRush2FarBack = 0;   /* 1: that rush started far back (z < -5000) */
s32 btaiRushPlaces[4][2] = { { -300, -850 }, { 0, -900 }, { 300, -850 }, { 0, -850 } }; /* the four places (x, z) the boss rushes to in turn */
s32 btaiUnused1 = 0;   /* two words nothing reads */
s32 btaiUnused2 = 0;
/* btaiRushIndex, the last of them, is btaiMoveToCockpit's static (defined there, next in .data). */

/* Matched 2026-10-12; it was parked for btaiRushIndex's address held in a register, gone with
 * the static counted in place and the targets read back from w for the distance. */
s32 btaiMoveToCockpit(Obj *obj) {
    static s32 btaiRushIndex = 0;
    BtaiW *w = (BtaiW *) (obj + 1);
    f32 x;
    f32 z;
    s32 pad[12];

    x = w->frame->pos[0] - sinf(w->frame->rot[1] - 3.1415927f) * 50.0f;
    z = w->frame->pos[2] - cosf(w->frame->rot[1] - 3.1415927f) * 50.0f;
    if (btaiRand(20) == 3) {
        btaiTbubbleMake(obj, x, w->frame->pos[1] + 510.0f, z, btaiRandSigned(20),
                               btaiRandSigned(20), 2.0f);
    }
    if (gGameFrameCount % 30 == 5) {
        btaiTbubbleMake(obj, x, w->frame->pos[1] + 510.0f, z, btaiRandSigned(20),
                               btaiRandSigned(20), 3.0f);
    }
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->step) {
        case 0:
            PRINTF("コックピット前へ移動\n");
            btaiRushIndex++;
            w->timer = 500;
            w->speed = 0.0f;
            w->count = 0.0f;
            if (btaiRushIndex == 4) {
                btaiRushIndex = 0;
            }
            w->target[0] = btaiRushPlaces[btaiRushIndex][0];
            w->target[2] = btaiRushPlaces[btaiRushIndex][1];
            if (bossDistSqXZ(w->target[0], w->target[2], w->frame->pos[0], w->frame->pos[2]) < 250000.0f) {
                w->timer = 0;
            }
            bossSetAnimAt(w->frame, btaiHeadbuttAnim, 0xF8, 0.0f);
            w->step = 1;
            btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
            break;
        case 1:
            if (bossStepAnim(w->frame, 0.21f, 2) == 1) {
                w->step = 2;
                bossSetAnimAt(w->frame, btaiHeadbuttStep1Anim, 0xF8, 0.0f);
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 128);
                btaiTurnTo(obj, w->target[0], w->target[2], 6.0f);
                SoundLocation2(0x2B6, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            } else {
                btaiTurnTo(obj, w->target[0], w->target[2], 4.0f);
                if (btaiRand(20) == 3) {
                    btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                           btaiRandSigned(20), btaiRandSigned(20), 2.5f);
                }
            }
            break;
        case 2:
            btaiBmoveX(obj, w->target[0], w->target[1] - 600, w->target[2], 1.3f);
            if (w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[0] * w->frame->pos[0] < 1210000.0f) {
                w->step = 3;
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 4.0f, 160);
                bossSetAnimAt(w->frame, btaiHeadbuttStep2Anim, 0xF8, 0.0f);
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, (25.0f - w->speed / 3.0f) / 8 * 0.7, 0);
            }
            break;
        case 3:
            if (bossStepAnim(w->frame, 0.42f, 2)) {
                w->step = 4;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 2.0f);
            bossDampVel(obj, 0.92f);
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 60.0 + -600.0;
            w->count += 2.0f;
            break;
        case 4:
            w->timer = 0;
            break;
    }
    if (w->frame->pos[1] < -606.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > -594.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    return 0;
}

s32 btaiMoveToBase(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f32 x;
    f32 z;
    s32 pad[12];

    x = w->frame->pos[0] - sinf(w->frame->rot[1] - 3.1415927f) * 50.0f;
    z = w->frame->pos[2] - cosf(w->frame->rot[1] - 3.1415927f) * 50.0f;
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    if (gGameFrameCount % 20 == 0) {
        btaiTbubbleMake(obj, x, w->frame->pos[1] + 510.0f, z, btaiRandSigned(20),
                               btaiRandSigned(20), 2.0f);
    }
    if (btaiRand(30) == 5) {
        btaiTbubbleMake(obj, x, w->frame->pos[1] + 510.0f, z, btaiRandSigned(20),
                               btaiRandSigned(20), 3.0f);
    }
    switch (w->step) {
        case 0:
            PRINTF("攻撃基本位置に移動\n");
            w->timer = 500;
            w->target[0] = 0;
            w->target[2] = -2900;
            w->speed = 0.0f;
            w->count = 0.0f;
            if (bossDistSqXZ(0, -2900.0f, w->frame->pos[0], w->frame->pos[2]) < 90000.0f) {
                w->timer = 0;
            }
            bossSetAnimAt(w->frame, btaiHeadbuttAnim, 0xF8, 0.0f);
            btaiT3bubbleMake(w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2]);
            w->step = 1;
            break;
        case 1:
            if (bossStepAnim(w->frame, 0.21f, 2) == 1) {
                w->step = 2;
                bossSetAnimAt(w->frame, btaiHeadbuttStep1Anim, 0xF8, 0.0f);
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 128);
                btaiTurnTo(obj, w->target[0], w->target[2], 6.0f);
                SoundLocation2(0x2B6, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            } else {
                btaiTurnTo(obj, w->target[0], w->target[2], 4.0f);
                if (btaiRand(20) == 3) {
                    btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                           btaiRandSigned(20), btaiRandSigned(20), 2.5f);
                }
            }
            break;
        case 2:
            if (!btaiBmoveX(obj, w->target[0], w->target[1] - 600, w->target[2], 1.3f)) {
                w->step = 3;
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 128);
                bossSetAnimAt(w->frame, btaiHeadbuttStep2Anim, 0xF8, 0.0f);
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, (25.0f - w->speed / 3.0f) / 12.0f * 0.7, 0);
            }
            break;
        case 3:
            if (bossStepAnim(w->frame, 0.126f, 2)) {
                w->step = 4;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 2.0f);
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 60.0 + -600.0;
            w->count += 2.0f;
            bossDampVel(obj, 0.93f);
            break;
        case 4:
            w->timer = 0;
            break;
    }
    if (w->frame->pos[1] < -606.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > -594.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    return 0;
}

s32 btaiMoveToEdge(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f32 x;
    f32 z;
    s32 pad[12];

    x = w->frame->pos[0] - sinf(w->frame->rot[1] - 3.1415927f) * 50.0f;
    z = w->frame->pos[2] - cosf(w->frame->rot[1] - 3.1415927f) * 50.0f;
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    if (w->frame->pos[2] > -2800.0f) {
        w->frame->pos[2] -= 16.0f;
    }
    if (gGameFrameCount % 20 == 0) {
        btaiTbubbleMake(obj, x, w->frame->pos[1] + 510.0f, z, btaiRandSigned(20),
                               btaiRandSigned(20), 2.0f);
    }
    if (btaiRand(20) == 5) {
        btaiTbubbleMake(obj, x, w->frame->pos[1] + 510.0f, z, btaiRandSigned(20),
                               btaiRandSigned(20), 3.0f);
    }
    switch (w->step) {
        case 0:
            PRINTF("右端に移動\n");
            w->timer = 500;
            w->speed = 0.0f;
            w->count = 0.0f;
            if (w->frame->pos[0] > 0.0f) {
                w->target[0] = 2000;
            } else {
                w->target[0] = -2000;
            }
            w->target[2] = -2000;
            bossSetAnimAt(w->frame, btaiHeadbuttAnim, 0xF8, 0.0f);
            w->step = 1;
            break;
        case 1:
            if (bossStepAnim(w->frame, 0.21f, 2) == 1) {
                w->step = 2;
                SoundLocation2(0x2B6, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                bossSetAnimAt(w->frame, btaiHeadbuttStep1Anim, 0xF8, 0.0f);
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 128);
                btaiTurnTo(obj, w->target[0], w->target[2], 6.0f);
            } else {
                btaiTurnTo(obj, w->target[0], w->target[2], 4.0f);
                if (btaiRand(20) == 3) {
                    btaiTbubbleMake(obj, w->frame->pos[0], w->frame->pos[1] + 510.0f, w->frame->pos[2],
                                           btaiRandSigned(20), btaiRandSigned(20), 2.5f);
                }
            }
            break;
        case 2:
            if (!btaiBmoveX(obj, w->target[0], w->target[1] - 600, w->target[2], 1.3f)) {
                w->step = 3;
                sunaEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 128);
                bossSetAnimAt(w->frame, btaiHeadbuttStep2Anim, 0xF8, 0.0f);
                w->count = 0.0f;
            } else {
                bossStepAnim(w->frame, (25.0f - w->speed / 3.0f) / 12.0f * 0.7, 0);
            }
            break;
        case 3:
            if (bossStepAnim(w->frame, 0.21f, 2)) {
                w->step = 4;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 4.0f);
            w->frame->pos[1] = sinf(w->count / 180.0f * 3.1415927f) * 60.0 + -600.0;
            w->count += 2.0f;
            bossDampVel(obj, 0.95f);
            break;
        case 4:
            w->timer = 0;
            break;
    }
    if (w->frame->pos[1] < -606.0f) {
        w->frame->pos[1] += 3.0f;
    }
    if (w->frame->pos[1] > -594.0f) {
        w->frame->pos[1] -= 3.0f;
    }
    return 0;
}

s32 btaiStraightHit(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;
    s32 pad[8];

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 0.0f);
            w->timer = 800;
            w->speed = gCockpitHeadPitch / 0.0174532930038800785 * 1.5 / 3.0 + 30.0;
            w->vel[0] = sinf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[1] = 10.0 - gCockpitHeadPitch / 0.0174532930038800785 * 2 / 3.0;
            w->frame->pos[0] += w->vel[0] * 5.0f;
            w->frame->pos[2] += w->vel[2] * 5.0f;
            w->step = 1;
            btaiHitStun = 1;
            w->vel[0] *= 0.5;
            gBossFlags &= ~4;
            break;
        case 1:
            btaiTurnTo(obj, 0.0f, 0.0f, 60.0f);
            w->vel[1] -= 0.4;
            w->speed -= 0.4;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            w->vel[0] = sinf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.1415927f) * w->speed;
            if (gGameFrameCount % 5 == 0) {
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
            }
            bossStepAnim(w->frame, 0.14f, 2);
            if (w->frame->pos[1] < -1100.0f) {
                w->frame->pos[1] = -1100.0f;
                btaiKokeEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2] + 300.0f);
                w->step = 2;
                w->vel[1] = 9.0f;
                w->count = 94.0f;
                bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 1.0f);
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            break;
        case 2:
            w->vel[1] -= 0.2;
            w->vel[0] *= 0.9;
            bossStepAnim(w->frame, 0.014f, 3);
            if (w->frame->pos[1] <= -600.0f) {
                w->step = 3;
                w->count = 90.0f;
            }
            break;
        case 3:
            w->vel[1] -= 0.05;
            if (w->vel[1] < 0.0f) {
                w->vel[1] = 0.0f;
            }
            if (w->frame->pos[1] > -600.0f) {
                w->vel[1] = 0.0f;
            }
            if (w->count-- < 0.0f) {
                w->step = 4;
            }
            break;
        case 4:
            w->vel[1] -= 0.2;
            if (w->frame->pos[1] > -600.0f) {
                w->vel[1] = 0.0f;
            }
            if (bossStepAnim(w->frame, 0.28f, 3)) {
                w->timer = 0;
                w->endPattern = 1;
                btaiHitStun = 0;
            }
            break;
    }
    return 0;
}

s32 btaiJabHitNegX(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiJabHitNegXAnim, 0xF8, 0.0f);
            w->step = 1;
            if (w->frame->pos[2] > -900.0f) {
                w->frame->pos[2] -= 50.0f;
            }
            w->timer = 200;
            w->vel[2] -= 20.0f;
            w->vel[0] *= 0.5;
            w->count = 20.0f;
            w->vel[1] = 0.0f;
            gBossFlags &= ~4;
            break;
        case 1:
            w->count -= 1.0f;
            w->vel[0] *= 0.95;
            w->vel[2] *= 0.8;
            if (gGameFrameCount % 5 == 0) {
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 420.0f, w->frame->pos[2]);
            }
            if (bossStepAnim(w->frame, 0.7f, 2)) {
                bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
                w->timer = 0;
                w->endPattern = 1;
                btaiHitStun = 0;
            }
            break;
    }
    return 0;
}

s32 btaiJabHitPosX(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiJabHitPosXAnim, 0xF8, 0.0f);
            w->step = 1;
            if (w->frame->pos[2] > -900.0f) {
                w->frame->pos[2] -= 50.0f;
            }
            w->timer = 200;
            w->vel[2] -= 20.0f;
            w->vel[0] *= 0.5;
            w->count = 20.0f;
            w->vel[1] = 0.0f;
            gBossFlags &= ~4;
            break;
        case 1:
            w->vel[0] *= 0.95;
            w->vel[2] *= 0.8;
            w->count -= 1.0f;
            if (gGameFrameCount % 5 == 0) {
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 420.0f, w->frame->pos[2]);
            }
            if (bossStepAnim(w->frame, 0.7f, 2)) {
                bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
                w->timer = 0;
                w->endPattern = 1;
                btaiHitStun = 0;
            }
            break;
    }
    return 0;
}

s32 btaiHundredPunchHit(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[8];
    f32 x = 0.0f;
    s32 pad2;
    f32 z = 0.0f;
    s32 pad3;

    PRINTF("ひゃくれつおわり\n");
    PRINTF("timer -> %d \n");
    btaiMoveLimit(obj);
    z = 0.0f;
    x = 0.0f;
    bossHeadingOffset(gCockpitHeadYaw, 0.0f, 1000.0f, &x, &z);
    if (x < w->frame->pos[0]) {
        w->frame->pos[0] -= 10.0f;
    } else {
        w->frame->pos[0] += 10.0f;
    }
    w->frame->pos[1] += 1.0f;
    if ((gBattleFlags2 & 0x4000) && gImpactMoveTime < 81) {
        w->timer = 800;
        w->action = 19;
        w->step = 0;
        return 0;
    }
    bossHeadingOffset(gCockpitHeadYaw, 0.0f, 1000.0f, &x, &z);
    if (x < w->frame->pos[0]) {
        w->frame->pos[0] -= 10.0f;
    } else {
        w->frame->pos[0] += 10.0f;
    }
    if (w->frame->pos[2] > -900.0f) {
        w->frame->pos[2] -= 20.0f;
    } else {
        w->frame->pos[2] += 20.0f;
    }
    if (w->frame->pos[1] < 300.0f) {
        w->frame->pos[1] += 1.0f;
    } else {
        w->frame->pos[1] -= 1.0f;
    }
    switch (w->step) {
        case 0:
            w->vel[2] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[0] = 0.0f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[1] = 0.0f;
            w->frame->rot[2] = 0.0f;
            bossSetAnimAt(w->frame, btaiJabHitNegXAnim, 0xF8, 0.0f);
            w->step = 1;
            w->timer = 2000;
            w->frame->pos[1] = -800.0f;
            btaiHitStun = 1;
            gBossFlags &= ~4;
            break;
        case 1:
            if (bossStepAnim(w->frame, 2.8f, 2)) {
                bossSetAnimAt(w->frame, btaiJabHitPosXAnim, 0xF8, 0.0f);
                w->step = 2;
            }
            break;
        case 2:
            if (bossStepAnim(w->frame, 2.8f, 2)) {
                bossSetAnimAt(w->frame, btaiJabHitNegXAnim, 0xF8, 0.0f);
                w->step = 3;
            }
            break;
        case 3:
            if (bossStepAnim(w->frame, 0.42f, 2) && !(gBattleFlags2 & 0x4000)) {
                bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
                w->action = 19;
                w->timer = 800;
                w->step = 0;
            }
            break;
    }
    return 0;
}

s32 btaiKickHit(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;
    s32 pad[8];

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 0.0f);
            w->timer = 800;
            w->speed = 50.0f;
            w->vel[0] = sinf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[1] = 10.0f;
            w->frame->pos[0] += w->vel[0] * 5.0f;
            w->frame->pos[2] += w->vel[2] * 5.0f;
            gBossFlags &= ~4;
            w->step = 1;
            w->count = 0.0f;
            break;
        case 1:
            if (w->timer == 770) {
                SoundLocation2(0x2B4, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 600.0f,
                               10000.0f, gBattleCamView->fovy);
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 60.0f);
            w->vel[1] -= 0.2;
            w->speed -= 0.3;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            w->vel[0] = sinf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.1415927f) * w->speed;
            if (gGameFrameCount % 5 == 0) {
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
            }
            if (gGameFrameCount % 5 == 0) {
                sunaEfMake(w->frame->pos[0], -650.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 128);
            }
            bossStepAnim(w->frame, 0.14f, 2);
            if (w->frame->pos[1] < -1100.0f) {
                w->vel[2] = 0.0f;
                w->frame->pos[1] = -1100.0f;
                btaiKokeEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2] + 300.0f);
                w->step = 2;
                w->vel[1] = 4.0f;
                w->count = 34.0f;
                bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 1.0f);
                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            break;
        case 2:
            w->vel[1] -= 0.03;
            w->vel[0] *= 0.9;
            w->count -= 1.0f;
            bossStepAnim(w->frame, 0.014f, 3);
            if (w->count == 0.0f) {
                w->step = 3;
            }
            break;
        case 3:
            w->vel[1] -= 0.03;
            if (bossStepAnim(w->frame, 0.28f, 3)) {
                w->timer = 0;
                w->endPattern = 1;
                btaiHitStun = 0;
                btaiCannonStun = 0;
            }
            break;
    }
    return 0;
}

s32 btaiCannonKnockback(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 0.0f);
            w->timer = 800;
            w->speed = 50.0f;
            w->vel[0] = sinf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[1] = 10.0f;
            w->frame->pos[0] += w->vel[0] * 5.0f;
            w->frame->pos[2] += w->vel[2] * 5.0f;
            gBossFlags &= ~4;
            w->step = 1;
            w->count = 0.0f;
            break;
        case 1:
            btaiTurnTo(obj, 0.0f, 0.0f, 60.0f);
            w->vel[1] -= 0.2;
            w->speed -= 0.3;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            w->vel[0] = sinf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.1415927f) * w->speed;
            if (gGameFrameCount % 5 == 0) {
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
            }
            if (gGameFrameCount % 5 == 0) {
                sunaEfMake(w->frame->pos[0], -650.0f, w->frame->pos[2], w->vel[0], w->vel[2], 6.0f, 128);
            }
            bossStepAnim(w->frame, 0.14f, 2);
            if (w->frame->pos[1] < -1100.0f) {
                w->vel[2] = 0.0f;
                w->frame->pos[1] = -1100.0f;
                btaiKokeEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2] + 300.0f);
                w->step = 2;
                w->vel[1] = 4.0f;
                w->count = 34.0f;
                bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 1.0f);
                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            break;
        case 2:
            w->vel[1] -= 0.03;
            w->vel[0] *= 0.9;
            w->count -= 1.0f;
            bossStepAnim(w->frame, 0.014f, 3);
            if (w->count == 0.0f) {
                w->step = 3;
            }
            break;
        case 3:
            w->vel[1] -= 0.03;
            if (bossStepAnim(w->frame, 0.28f, 3)) {
                w->timer = 0;
                w->endPattern = 1;
                btaiHitStun = 0;
                btaiCannonStun = 0;
            }
            break;
    }
    return 0;
}

s32 btaiDizzyKnockdown(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    if (w->step == 0) {
        bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 0.0f);
        w->timer = 800;
        w->speed = 60.0f;
        w->vel[0] = sinf(w->frame->rot[1] - 3.1415927f) * w->speed;
        w->vel[2] = cosf(w->frame->rot[1] - 3.1415927f) * w->speed;
        w->vel[1] = 17.0f;
        w->frame->pos[0] += w->vel[0] * 5.0f;
        w->frame->pos[2] += w->vel[2] * 5.0f;
        w->step = 1;
        w->action = 17;
        gBossFlags &= ~4;
        btaiHitStun = 1;
    }
    return 0;
}

s32 btaiCannonHit(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiTurnTo(obj, 0.0f, 0.0f, 60.0f);
    gMegaBurstY = 0.0f;
    switch (w->step) {
        case 0:
            w->frame->f.flags |= 2;
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[2] = 0.0f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[1] = 0.0f;
            w->frame->rot[2] = 0.0f;
            btaiInkGone = 1;
            gBossFlags &= ~4;
            w->frame->pos[1] = -600.0f;
            if (w->frame->pos[0] > 0.0f) {
                bossSetAnimAt(w->frame, btaiJabHitNegXAnim, 0xF8, 0.0f);
            } else {
                bossSetAnimAt(w->frame, btaiJabHitPosXAnim, 0xF8, 0.0f);
            }
            w->timer = 800;
            w->step = 1;
            w->count = 240.0f;
            gBrotherFlags &= ~0x8000;
            btaiHitStun = 1;
            break;
        case 1:
            bossStepAnim(w->frame, 2.8f, 0);
            if (gGameFrameCount % 10 == 0) {
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 420.0f, w->frame->pos[2]);
            }
            if ((s32) w->count-- < 1) {
                mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                btaiInkGone = 0;
                w->timer = 800;
                w->invTimer = 20;
                w->action = 21;
                w->step = 0;
                btaiHitStun = 0;
                btaiWhirlPiecesEnd = 0;
                btaiEffectsEnd = 0;
                gBrotherFlags &= ~0x20000;
                sndPlay(0x2B4);
            }
            break;
    }
    return 0;
}

s32 btaiImpactChange(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[10];

    PRINTF("インパクト交代\n");
    PRINTF("ゆらゆら....\n");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiBank(obj, 0.3f);
    w->count += 2.0f;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.1415927f) * 40.0 + -500.0) {
        w->frame->pos[1] += 12.0f;
    }
    if (sinf(w->count / 180.0f * 3.1415927f) * 40.0 + -500.0 < w->frame->pos[1]) {
        w->frame->pos[1] -= 12.0f;
    }
    switch (w->step) {
        case 0:
            w->timer = 300;
            bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
            w->step = 1;
            w->speed = 0.0f;
            btaiHitStun = 0;
            gBossFlags &= ~0x4000;
            gBossFlags |= 4;
            break;
        case 1:
            if (!bossDampVel(obj, 0.97f)) {
                w->step = 2;
                w->timer = 600;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 1.0f);
            bossStepAnim(w->frame, 0.42f, 0);
            break;
        case 2:
            if (btaiTurnTo(obj, 0.0f, 0.0f, 1.0f)) {
                w->step = 3;
                w->timer = 90;
            }
            bossStepAnim(w->frame, 0.42f, 0);
            break;
        case 3:
            gBossFlags &= ~4;
            bossStepAnim(w->frame, 0.42f, 0);
            w->endPattern = 1;
            break;
    }
    return 0;
}

s32 btaiKnockedBack(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;
    s32 pad[8];

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->step) {
        case 0:
            bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 0.0f);
            w->timer = 800;
            w->speed = 27.0f;
            w->vel[0] = 0.0f;
            w->vel[2] = 40.0f;
            w->vel[1] = 13.0f;
            w->frame->f.flags |= 2;
            w->step = 1;
            gBattleFlags2 &= ~1;
            gBossFlags &= ~4;
            break;
        case 1:
            btaiTurnTo(obj, 0.0f, 0.0f, 60.0f);
            w->vel[1] -= 0.4;
            w->speed -= 0.4;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            w->vel[0] = sinf(w->frame->rot[1] - 3.1415927f) * w->speed;
            w->vel[2] = cosf(w->frame->rot[1] - 3.1415927f) * w->speed;
            if (gGameFrameCount % 5 == 0) {
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
            }
            bossStepAnim(w->frame, 0.14f, 2);
            if (w->frame->pos[1] < -1100.0f) {
                w->frame->pos[1] = -1100.0f;
                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                btaiKokeEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2] + 300.0f);
                w->step = 2;
                w->vel[1] = 15.0f;
                w->count = 94.0f;
                bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 1.0f);
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
                btaiTsbubbleMake(w->frame->pos[0], w->frame->pos[1] + 300.0f, w->frame->pos[2] - 140.0f);
            }
            break;
        case 2:
            w->vel[1] -= 0.2;
            w->vel[0] *= 0.9;
            if (w->count > 0.0f) {
                w->count -= 1.0f;
            }
            bossStepAnim(w->frame, 0.014f, 3);
            if (w->frame->pos[1] <= -600.0f) {
                w->step = 3;
            }
            break;
        case 3:
            w->vel[1] -= 0.2;
            if (w->frame->pos[1] > -600.0f) {
                w->vel[1] = 0.0f;
            }
            if (bossStepAnim(w->frame, 0.28f, 3)) {
                w->timer = 0;
                w->endPattern = 1;
                btaiHitStun = 0;
            }
            break;
    }
    return 0;
}

/* Matched 2026-10-11; it was parked for its bomb count's address, gone with btaiBarrierBombs a
 * function static (the last word of this file's .data). */
s32 btaiBarrierAttack(Obj *obj) {
    static s32 btaiBarrierBombs = 0; /* bombs dropped in the barrier attack */
    BtaiW *w = (BtaiW *) (obj + 1);
    s32 pad[11];

    PRINTF("----->バリア\n");
    if (btaiNewAction == 1) {
        btaiNewAction = 0;
    }
    btaiMoveLimit(obj);
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    btaiBank(obj, 0.3f);
    w->count += 2.0f;
    if (w->frame->pos[1] < sinf(w->count / 180.0f * 3.1415927f) * 40.0 + -500.0) {
        w->frame->pos[1] += 12.0f;
    }
    if (sinf(w->count / 180.0f * 3.1415927f) * 40.0 + -500.0 < w->frame->pos[1]) {
        w->frame->pos[1] -= 12.0f;
    }
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[2] *= 0.94;
    w->frame->rot[0] *= 0.94;
    switch (w->step) {
        case 0:
            w->timer = 3200;
            bossSetAnimAt(w->frame, btaiNormalAnim, 0xF8, 0.0f);
            w->step = 1;
            w->speed = 0.0f;
            w->count = 0.0f;
            btaiBarrierBombs = 0;
            gBossFlags |= 0x4000;
            gBossFlags |= 4;
            SoundLocation2(0x2B4, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 600.0f,
                           10000.0f, gBattleCamView->fovy);
            soundActpanCreate(obj, 0x176, gBattleCamView, 500.0f, 11000.0f, gBattleCamView->fovy);
            btaiBarriMake();
            break;
        case 1:
            w->count += 1.0f;
            bossDampVel(obj, 0.97f);
            if (w->frame->pos[2] < -2790.0f) {
                w->step = 2;
                w->count = 0.0f;
                btaiBarrierBombs = 0;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 1.0f);
            bossStepAnim(w->frame, 0.42f, 0);
            if (w->frame->pos[2] > -2800.0f) {
                w->frame->pos[2] -= 20.0f;
            }
            if (w->frame->pos[2] < -2811.0f) {
                w->frame->pos[2] += 20.0f;
            }
            break;
        case 2:
            w->count += 1.0f;
            bossDampVel(obj, 0.97f);
            if (w->count > 700.0f) {
                w->step = 3;
                w->count = 0.0f;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 1.0f);
            bossStepAnim(w->frame, 0.42f, 0);
            break;
        case 3:
            bossDampVel(obj, 0.97f);
            if (btaiBarrierBombs >= 16) {
                w->step = 4;
                w->count = 0.0f;
            }
            btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
            bossStepAnim(w->frame, 0.42f, 0);
            w->vel[0] = -w->frame->pos[0] / 240.0f;
            w->vel[2] = -w->frame->pos[2] / 240.0f;
            if (sqrtf(w->frame->pos[2] * w->frame->pos[2] + w->frame->pos[0] * w->frame->pos[0]) < 900.0f &&
                gGameFrameCount % 15 == 0) {
                btaiDdmybombCreate(obj, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 400.0f, 5, 10,
                                       53);
                btaiBarrierBombs++;
            }
            break;
        case 4:
            w->vel[2] = -10.0f;
            w->vel[0] = 0.0f;
            if (w->count++ > 300.0f) {
                w->timer = 0;
                gBossFlags &= ~0x4000;
                gBossFlags &= ~4;
                sndPlay(0x8176);
                w->endPattern = 1;
            }
            break;
    }
    return 0;
}

s32 btaiExternalImpactHit(Obj *obj) {
    f32 angle;
    BtaiW *w = (BtaiW *) (obj + 1);
    f64 tilt = 1.0;

    btaiMoveLimit(obj);
    w->frame->rot[0] = w->vel[2] * 0.0174532930038800785 * tilt;
    w->frame->rot[2] = -w->vel[0] * 0.0174532930038800785 * tilt;
    w->frame->pos[0] += w->vel[0];
    w->frame->pos[1] += w->vel[1];
    w->frame->pos[2] += w->vel[2];
    switch (w->step) {
        case 0:
            w->timer = 800;
            bossSetAnimAt(w->frame, btaiStraightHitAnim, 0xF8, 0.0f);
            w->speed = 70.0f;
            angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
            w->vel[0] = sinf(angle + 3.1415927f) * w->speed;
            w->vel[2] = cosf(angle + 3.1415927f) * w->speed;
            w->vel[1] = 30.0f;
            btaiHitStun = 1;
            gBossFlags &= ~4;
            w->step = 1;
            w->count = 0.0f;
            break;
        case 1:
            w->vel[1] -= 0.9;
            w->speed -= 0.6;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
            w->vel[0] = sinf(angle + 3.1415927f) * w->speed;
            w->vel[2] = cosf(angle + 3.1415927f) * w->speed;
            bossStepAnim(w->frame, 0.28f, 2);
            if (w->frame->pos[1] < -1100.0f) {
                w->frame->pos[1] = -1100.0f;
                btaiHitStun = 0;
                btaiKokeEfMake(w->frame->pos[0], 0.0f, w->frame->pos[2] + 300.0f);
                w->step = 2;
                w->vel[1] = 33.0f;
                w->count = 34.0f;
                SoundLocation2(0x10C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
            }
            break;
        case 2:
            w->vel[1] -= 1.0f;
            angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
            w->vel[0] = sinf(angle + 3.1415927f) * w->speed;
            w->vel[2] = cosf(angle + 3.1415927f) * w->speed;
            w->speed -= 0.8;
            if (w->count > 0.0f) {
                w->count -= 1.0f;
            }
            bossStepAnim(w->frame, 0.28f, 2);
            if (w->frame->pos[1] <= -600.0f) {
                w->step = 3;
            }
            break;
        case 3:
            btaiTurnTo(obj, 0.0f, 0.0f, 10.0f);
            w->vel[1] -= 0.4;
            angle = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
            w->vel[0] = sinf(angle + 3.1415927f) * w->speed;
            w->vel[2] = cosf(angle + 3.1415927f) * w->speed;
            w->speed -= 0.7;
            if (w->speed < 0.0f) {
                w->speed = 0.0f;
            }
            if (w->frame->pos[1] > -600.0f) {
                w->vel[1] = 0.0f;
            }
            if (bossStepAnim(w->frame, 0.28f, 3)) {
                w->timer = 0;
                w->endPattern = 1;
            }
            break;
    }
    return 0;
}

/* The sway: steers the boss toward a point that circles with w->count (x on a 1300 swing,
 * y bobbing about -400, z about -2500), its velocity clamped, and tilts it with its velocity
 * by at most 30 degrees. */
#undef btaiSway
void btaiSway(Obj *obj) {
    BtaiW *w = (BtaiW *) (obj + 1);
    f32 y;
    f32 z;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 pad;

    btaiTurnTo(obj, 0.0f, 0.0f, 6.0f);
    z = sinf(w->count / 180.0f * 3.1415927f * 2.4f) * 400.0f + -2500.0f;
    y = sinf(2.0f * (w->count / 180.0f * 3.1415927f)) * 300.0f + -400.0f;
    dx = (sinf(w->count / 180.0f * 3.1415927f) * 1300.0f - w->frame->pos[0]) / 60.0f;
    dy = (y - w->frame->pos[1]) / 60.0f;
    dz = (z - w->frame->pos[2]) / 60.0f;
    if (dx > 0.0f) {
        w->vel[0] += 0.6;
    } else {
        w->vel[0] -= 0.6;
    }
    if (dy > 0.0f) {
        w->vel[1] += 0.3;
    } else {
        w->vel[1] -= 0.3;
    }
    if (dz > 0.0f) {
        w->vel[2] += 0.3;
    } else {
        w->vel[2] -= 0.3;
    }
    if (w->vel[0] > 30.0f) {
        w->vel[0] = 30.0f;
    }
    if (w->vel[0] < -30.0f) {
        w->vel[0] = -30.0f;
    }
    if (w->vel[2] > 8.0f) {
        w->vel[2] = 8.0f;
    }
    if (w->vel[2] < -8.0f) {
        w->vel[2] = -8.0f;
    }
    if (w->vel[1] > 8.0f) {
        w->vel[1] = 8.0f;
    }
    if (w->vel[1] < -8.0f) {
        w->vel[1] = -8.0f;
    }
    if (w->frame->rot[0] < w->vel[2] * 2 * 0.0174532930038800785 - 0.0698131720155203139) {
        w->frame->rot[0] += 0.034906586007760157;
    }
    if (w->vel[2] * 2 * 0.0174532930038800785 + 0.0698131720155203139 < w->frame->rot[0]) {
        w->frame->rot[0] -= 0.034906586007760157;
    }
    if (w->frame->rot[2] < -w->vel[0] * 2 * 0.0174532930038800785 - 0.0698131720155203139) {
        w->frame->rot[2] += 0.034906586007760157;
    }
    if (-w->vel[0] * 2 * 0.0174532930038800785 + 0.0698131720155203139 < w->frame->rot[2]) {
        w->frame->rot[2] -= 0.034906586007760157;
    }
    if (w->frame->rot[0] > 0.523598790116402313) {
        w->frame->rot[0] = 0.5235987902f;
    }
    if (w->frame->rot[0] < -0.523598790116402313) {
        w->frame->rot[0] = -0.5235987902f;
    }
    if (w->frame->rot[2] > 0.523598790116402313) {
        w->frame->rot[2] = 0.5235987902f;
    }
    if (w->frame->rot[2] < -0.523598790116402313) {
        w->frame->rot[2] = -0.5235987902f;
    }
}
