#ifndef SCCHECK_H
#define SCCHECK_H

#include "object.h"
#include "game.h"

void scAskTegataLeftMin(void);
s32 scTegataLeftMin(s32 area);
void scAskTegataLeftMax(void);
s32 scTegataLeftMax(s32 area);
void scAskTegataLeftArea(void);
s32 scTegataLeftArea(s32 area);
void scTegataAdd(void);
void scTegataAddArea(s32 area);
void scAskTegataTotal(void);
s32 scKinokoCheck(void);
void scAskKinokoAll(void);
void scKinokoClear(void);
void scAskKinokoCount(void);
void scClearKinokoCount(void);
void scAskJizouCorrectFull(void);
void scClearJizouCorrect(void);
void scDummy1(void);
void scDummy2(void);
s32 kakurenboCheck(void);
void scAskKakurenbo(void);
s32 scCountKarasuShot(void);
void scAskKarasuShot(void);
u16 scGetDebtPaid(void);
void scAskDebtPaid(void);
u16 scGetDebt2Paid(void);
void scAskDebt2Paid(void);
void scAskHanabiShows(void);
s32 scShakeOnigiriCheck(s32 pl);
void scAskShakeOnigiri(void);
s32 scStageFlagBit7(s32 n);
void scAskStageFlagBit7(void);
void scAskPlWithoutMe(void);
s32 plCheckWithoutMe(s32 me);
void scAskPlKind(void);
Obj *plCheck(void);
s32 moneyAdd(s32 amount, s32 pl);
void scMoneyAdd(void);
void scMoneyAddAll(void);
s32 scGetHour(void);
void scAskHour(void);
void scAskGameClear(void);
void scSdSet(void);
void scMenuCheck(void);
void scMenuCheckBoth(void);
void scSetArgPlayer(void);
s32 scGetMenuChoice1(void);
void scCheckDiary(void);
void scRepairDiary(void);
void scProbePak(void);
void scSetupPak(void);
void scSaveDiary(void);
void scSaveDiaryKeepPoint(void);
s32 scArmorCheck(s32 armor, s32 pl);
void scAskArmor(void);
void scArmorBuy(void);
s32 LifeCheck(s32 pl);
void scAskLife(void);
void scLifeAdd(void);
void scRestAdd(void);
s32 scOnigiriCheck(s32 food, s32 pl);
void scAskOnigiri(void);
void scOnigiriBuy(void);
void scKosupureBuy(void);
void scAskTegata44(void);

#endif
