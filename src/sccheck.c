#include "object.h"
#include "game.h"
#include "audio.h"
#include "contpak.h"
#include "fade.h"
#include "plmain.h"
#include "sccheck.h"
#include "swwork.h"

/* The scenario's calls (op 11): checks and changes on the game data that the event scripts
 * make. Each takes its arguments from script variables 0x08000074/78/7C (and 0x04, the
 * character talking) and leaves its answer in 0x08000084.
 * - Passes (手形, 6 areas): how many are held, how many an area still needs, adding one.
 * - Collections: the mushrooms (キノコ, 7), the hide-and-seek finds (flags 30-35), and two
 *   more sets kept in the game state.
 * - The salmon rice ball (鮭おにぎり): taking one from the talker or anyone else.
 * - The other character in play, the players' money, armour, lives, food and costumes.
 * - Whether the game is finished, the hour, and the diary (Controller Pak) checks and saves.
 * All their prints are dead; their places here follow the strings' order in .rodata. */

/* sccheck.c's .data (0x8010AEB0-0x8010AEC8), in ROM order. */
u8 gScTegataNeedMin[6] = { 5, 10, 17, 24, 30, 41 }; /* passes needed, by area (two tables) */
u8 gScTegataNeedMax[6] = { 6, 14, 24, 32, 41, 44 };
u8 gScTegataPerArea[6] = { 6, 8, 10, 8, 9, 3 }; /* passes an area gives */
extern s32 *gScVarPlayer; /* the script variables (scenario.c) */
extern s32 *gScVar04;
extern s32 *gScVarChoice;
extern s32 *gScVar68;
extern s32 *gScVarInMenu1;
extern s32 *gScVarInMenu2;
extern s32 *gScVar74;
extern s32 *gScVar78;
extern s32 *gScVar7C;
extern s32 *gScVar84;


/* Passes still needed: by the first table. */
void scAskTegataLeftMin(void) {
    *gScVar84 = scTegataLeftMin(*gScVar74);
}

s32 scTegataLeftMin(s32 area) {
    s32 i;
    s32 sum = 0;

    for (i = 0; i < 6; i++) {
        sum += gSaveData.tegata[i];
    }
    PRINTF("*****   手形debug [%d]エリア  [%d]\n", area, sum);
    PRINTF(">>>>手形情報[%d]エリア   手形は現在 [%d]枚もってます。最小ノルマ[%d]枚。残り[%d]枚。\n",
           area, sum, gScTegataNeedMin[area], gScTegataNeedMin[area] - sum);
    return gScTegataNeedMin[area] - sum;
}

/* By the second. */
void scAskTegataLeftMax(void) {
    *gScVar84 = scTegataLeftMax(*gScVar74);
}

s32 scTegataLeftMax(s32 area) {
    s32 i;
    s32 sum = 0;

    for (i = 0; i < 6; i++) {
        sum += gSaveData.tegata[i];
    }
    PRINTF("*****   手形debug [%d]エリア  [%d]\n", area, sum);
    PRINTF(">>>>手形情報[%d]エリア   手形は現在 [%d]枚もってます。最大ノルマ[%d]枚。残り[%d]枚。\n",
           area, sum, gScTegataNeedMax[area], gScTegataNeedMax[area] - sum);
    return gScTegataNeedMax[area] - sum;
}

/* An area's own passes still to find. */
void scAskTegataLeftArea(void) {
    *gScVar84 = scTegataLeftArea(*gScVar74);
}

s32 scTegataLeftArea(s32 area) {
    PRINTF("*****   手形debug [%d]エリア  [%d]\n", area, gSaveData.tegata[area]);
    PRINTF(">>>>手形情報[%d]エリア   エリア手形は現在 [%d]枚もってます。最大[%d]枚。残り[%d]枚。\n",
           area, gSaveData.tegata[area], gScTegataPerArea[area],
           gScTegataPerArea[area] - gSaveData.tegata[area]);
    return gScTegataPerArea[area] - gSaveData.tegata[area];
}

/* Adds a pass in an area, up to its most. */
void scTegataAdd(void) {
    scTegataAddArea(*gScVar74);
}

void scTegataAddArea(s32 area) {
    gSaveData.tegata[area]++;
    if (gScTegataNeedMax[area] < gSaveData.tegata[area]) {
        gSaveData.tegata[area] = gScTegataNeedMax[area];
    }
    PRINTF("手形は[%d]エリアで現在 [%d]枚もってます。最小ノルマ[%d]枚。最大ノルマ[%d]枚\n", area,
           gSaveData.tegata[area], gScTegataNeedMin[area], gScTegataNeedMax[area]);
}

/* All the passes held. */
void scAskTegataTotal(void) {
    s32 i;
    s32 sum = 0;

    for (i = 0; i < 6; i++) {
        sum += gSaveData.tegata[i];
    }
    PRINTF("*****   手形debug [%d]エリア  [%d]枚\n", i, sum);
    PRINTF("-----------------\n");
    PRINTF("*****   手形  累計[%d]枚\n", sum);
    *gScVar84 = sum;
}

/* The mushrooms found (bits 1-7), and whether all seven are. */
s32 scKinokoCheck(void) {
    s32 n = 0;
    s32 i;

    for (i = 1; i < 8; i++) {
        if (gGameWork.kinoko & (u8) (1 << (u8) (i % 8))) {
            n++;
        }
    }
    PRINTF("scKinokoCheck: キノコ[%d]個\n", n);
    return n;
}

void scAskKinokoAll(void) {
    if (scKinokoCheck() >= 7) {
        *gScVar84 = TRUE;
    } else {
        *gScVar84 = FALSE;
    }
}

void scKinokoClear(void) {
    gGameWork.kinoko = 0;
}

void scAskKinokoCount(void) {
    s32 v = gGameWork.kinokoCount;

    *gScVar84 = v;
}

void scClearKinokoCount(void) {
    gGameWork.kinokoCount = 0;
}

void scAskJizouCorrectFull(void) {
    if (gGameWork.jizouCorrect >= 7) {
        *gScVar84 = TRUE;
    } else {
        *gScVar84 = FALSE;
    }
}

void scClearJizouCorrect(void) {
    gGameWork.jizouCorrect = 0;
}

/* Two empty calls. */
void scDummy1(void) {
    PRINTF("dummy");
}

void scDummy2(void) {
    PRINTF("dummy");
}

/* The hide-and-seek finds (flags 30-35). */
s32 kakurenboCheck(void) {
    s32 n = 0;

    if (swWorkTest(30) != 0) {
        n++;
    }
    if (swWorkTest(31) != 0) {
        n++;
    }
    if (swWorkTest(32) != 0) {
        n++;
    }
    if (swWorkTest(33) != 0) {
        n++;
    }
    if (swWorkTest(34) != 0) {
        n++;
    }
    if (swWorkTest(35) != 0) {
        n++;
    }
    PRINTF(">>>> kakurenboCheck [%d]\n", n);
    return n;
}

void scAskKakurenbo(void) {
    *gScVar84 = kakurenboCheck();
}

/* Bits 1-6 of another set. */
s32 scCountKarasuShot(void) {
    s32 n = 0;
    s32 i;

    for (i = 1; i < 7; i++) {
        if (gGameWork.karasuShot & (u8) (1 << (u8) (i % 8))) {
            n++;
        }
    }
    return n;
}

void scAskKarasuShot(void) {
    *gScVar84 = scCountKarasuShot();
}

/* Two halfwords of the game data. */
u16 scGetDebtPaid(void) {
    u16 v = gSaveData.debtPaid;

    return v;
}

void scAskDebtPaid(void) {
    *gScVar84 = scGetDebtPaid();
}

u16 scGetDebt2Paid(void) {
    u16 v = gSaveData.debt2Paid;

    return v;
}

void scAskDebt2Paid(void) {
    *gScVar84 = scGetDebt2Paid();
}

void scAskHanabiShows(void) {
    *gScVar84 = gGameWork.hanabiShows;
}

/* Takes a salmon rice ball (food 3): the talker's, or else anyone's. Then 0; 1 if someone
 * holds another rice ball; 2 if no one holds any.
 * The second search is a loop too: IDO unrolls it completely (the game's lui per element),
 * and with it there the first loop stays rolled with the game's sltu.
 * The two searches reuse i (the talker's index) as their counter. */
s32 scShakeOnigiriCheck(s32 pl) {
    s32 i = pl - 1;

    if (gSaveData.players[pl - 1].food == 3) {
        PRINTF(">>>>  scShakeOnigiriCheck: [%d]P の鮭おにぎり減らす\n", pl);
        gSaveData.players[i].food = 0;
        return 0;
    }
    PRINTF(">>>>  step1: scShakeOnigiriCheck: 話かけた人 [%d]P のしゃけおにぎりない\n", pl);
    for (i = 0; i < 4; i++) {
        if (gSaveData.players[i].chara != -1 && gSaveData.players[i].lives >= -1 &&
            gSaveData.players[i].food == 3) {
            PRINTF(">>>>  scShakeOnigiriCheck: [%d]P の鮭おにぎり減らす\n", i + 1);
            gSaveData.players[i].food = 0;
            return 0;
        }
    }
    PRINTF(">>>>  step2: scShakeOnigiriCheck: だれもしゃけおにぎりない\n");
    for (i = 0; i < 4; i++) {
        if (gSaveData.players[i].chara != -1 && gSaveData.players[i].lives > 0 &&
            gSaveData.players[i].food != 0) {
            PRINTF(">>>>  scShakeOnigiriCheck: [%d]P 鮭ではないおにぎりもっている\n", i + 1);
            return 1;
        }
    }
    PRINTF(">>>>  scShakeOnigiriCheck: だれもおにぎりもっていない\n");
    return 2;
}

void scAskShakeOnigiri(void) {
    *gScVar84 = scShakeOnigiriCheck(*gScVar78);
}

/* Bit 7 of stage flag 100 + n. */
s32 scStageFlagBit7(s32 n) {
    if (gSaveData.stageFlags[100 + n] & 0x80) {
        return TRUE;
    }
    return FALSE;
}

void scAskStageFlagBit7(void) {
    *gScVar84 = scStageFlagBit7(*gScVar74);
}

/* The other character in play (4 for none). */
void scAskPlWithoutMe(void) {
    *gScVar84 = plCheckWithoutMe(*gScVar04);
}

s32 plCheckWithoutMe(s32 me) {
    s32 i;
    s32 kind;

    for (i = 0; i < 2; i++) {
        if (plIsInPlay(i) != 0) {
            kind = plGetChara(gGameWork.players[i]);
            if (kind != me) {
                PRINTF("plCheckWithoutMe: plkind[%d]\n", kind);
                return kind;
            }
        }
    }
    PRINTF("plCheckWithoutMe: 相棒はいませんぜ\n");
    return 4;
}

/* The first character in play. */
void scAskPlKind(void) {
    Obj *player;

    player = plCheck();

    if (player != NULL) {
        *gScVar84 = plGetChara(player);
    }
}

Obj *plCheck(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) != 0) {
            PRINTF("plCheck: plkind[%d]\n", i);
            return gGameWork.players[i];
        }
    }
    return NULL;
}

/* Adds to a player's money (ryo): 1, or 0 if it would go below 0; -1 for no player.
 * The new sum reuses pl (it keeps pl's register, a1; a new local moves amount to a2). */
s32 moneyAdd(s32 amount, s32 pl) {
    s32 i = pl - 1;

    if (gSaveData.players[pl - 1].chara == -1 || gSaveData.players[i].lives <= 0) {
        PRINTF("*** moneyAdd プレーヤーいない plno[%d]\n", pl);
        return -1;
    }
    pl = gSaveData.players[i].money + amount; /* the new sum */
    if (pl >= 0) {
        PRINTF("*** moneyAdd 足りた  支払[%d]両 現在[%d]両\n", amount, pl);
        gSaveData.players[i].money = pl;
        return 1;
    }
    PRINTF("*** moneyAdd 足りた\n");
    return 0;
}

void scMoneyAdd(void) {
    *gScVar84 = moneyAdd(*gScVar74, *gScVar78);
}

/* The same for everyone in play. */
void scMoneyAddAll(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i) != 0) {
            moneyAdd(*gScVar74, i + 1);
        }
    }
}

/* The hour. */
s32 scGetHour(void) {
    s32 hour = gGameWork.time / 0x10000;

    return hour;
}

void scAskHour(void) {
    *gScVar84 = scGetHour();
}

/* Whether the game has been finished. */
void scAskGameClear(void) {
    if (gSaveData.allClear) {
        PRINTF(">>>>>>>>   scAskGameClear: ゲームクリアしている！！ dw.all_clear[%d]\n",
               gSaveData.allClear);
        *gScVar84 = TRUE;
    } else {
        PRINTF(">>>>>>>>   scAskGameClear: ゲームクリアまだ！ dw.all_clear[%d]\n",
               gSaveData.allClear);
        *gScVar84 = FALSE;
    }
}

/* Plays a sound. */
void scSdSet(void) {
    PRINTF("scSdSet: SdSet します[%d]\n", *gScVar74);
    (*sndPlay)(*gScVar74);
}

/* A player's menu choice (1 or 2). The switch value val0 is reused for the choice read, and
 * the choice itself stays in its stack slot (declared first, so 4(sp)). */
void scMenuCheck(void) {
    s32 choice;
    s32 val0 = *gScVar74;

    switch (val0) {
        case 1:
            if (*gScVarInMenu1 == 0) {
                val0 = *gScVarChoice;
            } else {
                val0 = *gScVarChoice;
            }
            choice = val0;
            break;
        case 2:
            if (*gScVarInMenu2 == 0) {
                val0 = *gScVar68;
            } else {
                val0 = *gScVar68;
            }
            choice = val0;
            break;
        default:
            PRINTF("scMenuCheck: Warning: val0 がおかしい\n");
            break;
    }
    *gScVar84 = choice;
}

/* Whether both players have chosen. */
void scMenuCheckBoth(void) {
    s32 choice[2];
    s32 i;

    *gScVar74 = 1;
    scMenuCheck();
    choice[0] = *gScVar84;
    *gScVar74 = 2;
    scMenuCheck();
    choice[1] = *gScVar84;
    for (i = 0; i < 2; i++) {
        if (choice[i] == -1) {
            *gScVar84 = FALSE;
            return;
        }
    }
    *gScVar84 = TRUE;
}

void scSetArgPlayer(void) {
    *gScVar78 = *gScVarPlayer;
}

s32 scGetMenuChoice1(void) {
    return *gScVarChoice;
}

/* Whether no diary is in use. */
void scCheckDiary(void) {
    PRINTF("scCheckDiary : gw.diaryNo [%d]\n", gGameWork.diary);
    if (gGameWork.diary == -1) {
        *gScVar84 = TRUE;
    } else {
        *gScVar84 = FALSE;
    }
}

/* Repairs the diary: 0 done, 2 no pak, 3 wrong device, 5 no controller, 7 CONTRFAIL,
 * 8 ID_FATAL, 4 anything else. */
void scRepairDiary(void) {
    s32 err = contPakInitPfsRepair(0);

    contPakProbe(0);
    switch (err) {
        case 0:
            PRINTF("scRepairDiary : No error\n");
            *gScVar84 = 0;
            break;
        case 1:
            PRINTF("scRepairDiary : No pak\n");
            *gScVar84 = 2;
            break;
        case 2:
        case 11:
        case 0x200:
            PRINTF("scRepairDiary : Wrong device\n");
            *gScVar84 = 3;
            break;
        case 0x101:
            PRINTF("scRepairDiary : No controler\n");
            *gScVar84 = 5;
            break;
        case 4:
            PRINTF("scRepairDiary : PFS_ERR_CONTRFAIL\n");
            *gScVar84 = 7;
            break;
        case 10:
            PRINTF("scRepairDiary : PFS_ERR_ID_FATAL\n");
            *gScVar84 = 8;
            break;
        default:
            PRINTF("scRepairDiary : Unknown error %d\n", err);
            *gScVar84 = 4;
            break;
    }
}

void scProbePak(void) {
    contPakProbe(0);
}

void scSetupPak(void) {
    fadePakInit();
}

/* Saves the diary at this stage's save point (stage 0x99, 0xB4, 0xCA, 0xD9, 0xE9/0xEB, 0xFA
 * are points 1-6; 1 for anywhere else). The answers are as the repair's, with 6 for no GOEMON
 * NOTE file. */
void scSaveDiary(void) {
    s8 point;
    s32 err;

    switch (gGameWork.stage) {
        case 0x99:
            point = 1;
            break;
        case 0xB4:
            point = 2;
            break;
        case 0xCA:
            point = 3;
            break;
        case 0xE9:
        case 0xEB:
            point = 4;
            break;
        case 0xD9:
            point = 5;
            break;
        case 0xFA:
            point = 6;
            break;
        default:
            PRINTF("scSaveDiary : unknown SAVE_POINT\n");
            *gScVar84 = 1;
            return;
    }
    gSaveData.savePoint = point;
    err = fadeWriteSave(&gSaveData, gGameWork.diary);
    switch (err) {
        case 0:
            PRINTF("scSaveDiary : No error\n");
            *gScVar84 = 0;
            break;
        case 1:
            PRINTF("scSaveDiary : No pak\n");
            *gScVar84 = 2;
            break;
        case 11:
        case 0x200:
            PRINTF("scSaveDiary : Wrong device\n");
            *gScVar84 = 3;
            break;
        case 2:
        case 6:
        case 7:
        case 8:
        case 9:
        case 0x102:
            PRINTF("scSaveDiary : File error?\n");
            *gScVar84 = 4;
            break;
        case 0x100:
        case 0x101:
            PRINTF("scSaveDiary : No controler\n");
            *gScVar84 = 5;
            break;
        case 5:
            PRINTF("scSaveDiary : No GOEMON NOTE\n");
            *gScVar84 = 6;
            break;
        case 3:
        case 4:
            PRINTF("scSaveDiary : PFS_ERR_CONTRFAIL\n");
            *gScVar84 = 7;
            break;
        case 10:
            PRINTF("scSaveDiary : PFS_ERR_ID_FATAL\n");
            *gScVar84 = 8;
            break;
        default:
            PRINTF("scSaveDiary : Unknown error %d\n", err);
            *gScVar84 = 4;
            break;
    }
}

/* Saves the diary where it is. */
void scSaveDiaryKeepPoint(void) {
    s32 err = fadeWriteSave(&gSaveData, gGameWork.diary);

    switch (err) {
        case 0:
            PRINTF("scSaveDiary : No error\n");
            *gScVar84 = 0;
            break;
        case 1:
            PRINTF("scSaveDiary : No pak\n");
            *gScVar84 = 2;
            break;
        case 11:
        case 0x200:
            PRINTF("scSaveDiary : Wrong device\n");
            *gScVar84 = 3;
            break;
        case 2:
        case 6:
        case 7:
        case 8:
        case 9:
        case 0x102:
            PRINTF("scSaveDiary : File error?\n");
            *gScVar84 = 4;
            break;
        case 0x100:
        case 0x101:
            PRINTF("scSaveDiary : No controler\n");
            *gScVar84 = 5;
            break;
        case 5:
            PRINTF("scSaveDiary : No GOEMON NOTE\n");
            *gScVar84 = 6;
            break;
        case 3:
        case 4:
            PRINTF("scSaveDiary : PFS_ERR_CONTRFAIL\n");
            *gScVar84 = 7;
            break;
        case 10:
            PRINTF("scSaveDiary : PFS_ERR_ID_FATAL\n");
            *gScVar84 = 8;
            break;
        default:
            PRINTF("scSaveDiary : Unknown error %d\n", err);
            *gScVar84 = 4;
            break;
    }
}

/* A player's armour against a level: 0 below, 1 above, 2 the same; -1 for no player. */
s32 scArmorCheck(s32 armor, s32 pl) {
    Unk88282 *p;
    s32 i = pl - 1;

    if (gSaveData.players[pl - 1].chara == -1 ||
        (p = &gSaveData.players[i])->lives <= 0) {
        return -1;
    }
    PRINTF("now arm[%d] new arm[%d]\n", p->armor, armor);
    if (p->armor < armor) {
        return 0;
    }
    if (armor < p->armor) {
        return 1;
    }
    return 2;
}

void scAskArmor(void) {
    *gScVar84 = scArmorCheck(*gScVar7C, *gScVar78);
}

/* Buys armour. */
void scArmorBuy(void) {
    s32 i = *gScVar78 - 1;

    PRINTF("*** ArmorCheck プレーヤーいない plno[%d]\n", *gScVar78);
    PRINTF("Armor BUY! arm[%d]\n", *gScVar7C);
    gSaveData.players[i].armor = *gScVar7C;
}

/* Whether a player's life is full (3); -1 for no player. */
s32 LifeCheck(s32 pl) {
    Unk88282 *p;
    s32 i = pl - 1;

    if (gSaveData.players[pl - 1].chara == -1 ||
        (p = &gSaveData.players[i])->lives <= 0) {
        PRINTF("*** LifeCheck プレーヤーいない plno[%d]\n", pl);
        return -1;
    }
    if (p->life < 3) {
        return 0;
    }
    return 1;
}

void scAskLife(void) {
    *gScVar84 = LifeCheck(*gScVar78);
}

/* Adds life, up to 3. */
void scLifeAdd(void) {
    s32 i = *gScVar78 - 1;
    Unk88282 *p = &gSaveData.players[i];

    p->life += *gScVar7C;
    if (p->life >= 4) {
        p->life = 3;
    }
    PRINTF("scLifeAdd : food[%d] pl[%d].life[%d]\n", *gScVar7C, *gScVar78, p->life);
}

/* Adds a life, up to 100. */
void scRestAdd(void) {
    s32 i = *gScVar78 - 1;
    Unk88282 *p = &gSaveData.players[i];

    p->lives++;
    if (p->lives >= 100) {
        p->lives = 100;
    }
}

/* A player's food against a kind: 0 below, 1 above, 2 the same; -1 for no player. */
s32 scOnigiriCheck(s32 food, s32 pl) {
    Unk88282 *p;
    s32 i = pl - 1;

    if (gSaveData.players[pl - 1].chara == -1 ||
        (p = &gSaveData.players[i])->lives <= 0) {
        return -1;
    }
    PRINTF("now food[%d] new food[%d]\n", p->food, food);
    if (p->food < food) {
        return 0;
    }
    if (food < p->food) {
        return 1;
    }
    return 2;
}

void scAskOnigiri(void) {
    *gScVar84 = scOnigiriCheck(*gScVar7C, *gScVar78);
}

/* Buys food. */
void scOnigiriBuy(void) {
    s32 i = *gScVar78 - 1;

    PRINTF("*** OnigiriCheck プレーヤーいない plno[%d]\n", *gScVar78);
    PRINTF("Onigiri BUY! food[%d]\n", *gScVar7C);
    gSaveData.players[i].food = *gScVar7C;
}

/* Buys a costume for the character talking, and wears it. */
void scKosupureBuy(void) {
    gSaveData.costume[*gScVar04][0] = *gScVar74;
    gSaveData.costume[*gScVar04][*gScVar74] = 1;
    PRINTF("KOSUPURE BUY! kind[%d] type[%d] wear[%d]\n", *gScVar04, *gScVar74, 1);
}

/* Whether 44 passes are held. */
void scAskTegata44(void) {
    s32 i;
    s32 sum = 0;

    for (i = 0; i < 6; i++) {
        sum += gSaveData.tegata[i];
    }
    PRINTF("*****   手形debug [%d]エリア  [%d]枚\n", i, sum);
    PRINTF("-----------------\n");
    PRINTF("*****   手形  累計[%d]枚\n", sum);
    if (sum >= 44) {
        *gScVar84 = TRUE;
    } else {
        *gScVar84 = FALSE;
    }
}
