#ifndef __RS6060_APP_PORTING__
#define __RS6060_APP_PORTING__

#include "vs_fe_external.h"
#include "vs_fe_tuner.h"

_U32 RS6060_App_Init(_U8 u8TunerIndex);

_BOOL RS6060_App_Tune(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock);
_BOOL RS6060_App_Stop_Tune(_U8 u8TunerIndex);
void RS6060_App_ExitTune(_U8 u8TunerIndex);

void RS6060_App_BlindScanStart(_U8 u8TunerIndex, _BlindScanCallback pCallback, PBLIND_SCAN_PARAM_INFO pScanParams);
void RS6060_App_BlindScanStop(_U8 u8TunerIndex);

_BOOL RS6060_App_SetLNBPower(_U8 u8TunerIndex, _BOOL bPowerOn);
_BOOL RS6060_App_SetPolarization(_U8 u8TunerIndex, _BOOL bHigh);
_BOOL RS6060_App_SetDiseqcContinuous(_U8 u8TunerIndex, _BOOL bOn);
_BOOL RS6060_App_SendDiseqcCommand(_U8 u8TunerIndex, _U8 *pu8Data, _U8 u8Length);
_BOOL RS6060_App_SendDiseqcToneBurst(_U8 u8TunerIndex, _U8 u8Command);

_BOOL RS6060_App_CheckLocked(_U8 u8TunerIndex, _BOOL *pbLock);
_BOOL RS6060_App_GetBer(_U8 u8TunerIndex, _U32 *pu32Ber);
_BOOL RS6060_App_GetStrength(_U8 u8TunerIndex, _U16 *pu16Strength);
_BOOL RS6060_App_GetSnr(_U8 u8TunerIndex, _U16 *pu16Snr);

void RS6060_App_StandBy(_U8 u8TunerIndex);

#endif
