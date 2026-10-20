#ifndef SNDCMD_H
#define SNDCMD_H

#include "snd.h"

void sndCmdTempo(void);
void sndCmdTempoSlide(void);
void sndCmdTempoSlideTick(void);
void sndCmdInstrument(void);
void sndCmdInstrumentSetup(s8 *inst);
void sndCmdDrumInstrument(void);
void sndCmdVolInstrument(void);
void sndCmdVolInstrumentPan(void);
void sndCmdVolume(void);
void sndCmdVolumeSlide(void);
void sndCmdVolumeSlideTick(void);
void sndCmdEnvAttackDecay(void);
void sndCmdEnvSustain(void);
void sndCmdEnvRelease(void);
void sndCmdFxParams(void);
void sndCmdFadeHold(void);
void sndCmdPan(void);
void sndCmdPanSlide(void);
void sndCmdPanSlideTick(void);
void sndCmdTranspose(void);
void sndCmdDetune(void);
void sndCmdVibrato(void);
void sndCmdVibratoReset(void);
void sndCmdVibratoTick(void);
void sndCmdVibratoStep(void);
void sndCmdVibratoRise(void);
void sndCmdWobble(void);
void sndCmdWobbleTick(void);
void sndCmdPortamentoSkip(void);
void sndCmdPortamentoCalc(void);
void sndCmdPortamentoCheck(void);
void sndCmdPortamentoTick(void);
void sndCmdPortamentoTickSe(void);
void sndCmdPitchBend(void);
void sndCmdPitchBendNote(void);
void sndCmdGlide(void);
void sndCmdGlideTick(void);
void sndCmdLoopStart(void);
void sndCmdLoopEnd(void);
void sndCmdLoop2Start(void);
void sndCmdLoop2End(void);
void sndCmdMark(void);
void sndCmdGotoMark(void);
void sndCmdRepeatStart(void);
void sndCmdRepeat(void);
void sndCmdFxMix(void);
void sndCmdBank(void);
void sndCmdStateMute(void);
void sndCmdRest(void);
void sndCmdTie(void);
void sndCmdScaleBase(void);
void sndCmdRandomVariation(void);
void sndCmdEvent(void);
void sndCmdEventArg(void);
void sndCmdEventKind2(void);
void sndCmdEventKind3(void);
void sndCmdEventKind4(void);
void sndCmdQueueEvent(u8 kind, u8 id, u8 arg, s16 value);
void sndCmdEventValue(u8 kind);
void sndCmdEnd(void);
void sndCmdSkip1(void);
void sndCmdSkip2(void);
void sndCmdSkip3(void);

#endif
