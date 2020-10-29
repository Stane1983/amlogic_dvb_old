#ifndef __AVL68XX_APP_PORTING__
#define __AVL68XX_APP_PORTING__

#include "vs_fe_external.h"
#include "vs_fe_tuner.h"

#include "AVL68XX_Porting.h"

#define AVL68XX_SYSTEM_TYPE_DVB_T 1
#define AVL68XX_SYSTEM_TYPE_DVB_C 2
#define AVL68XX_SYSTEM_TYPE_DVB_S 4
#define AVL68XX_SYSTEM_TYPE_ISDB_T 8

_U32 AVL68XX_App_Init(_U8 u8TunerIndex, _U32 *pChipId);

_BOOL AVL68XX_App_Tune_DVBT2(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock);
_BOOL AVL68XX_App_Tune_DVBC(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock);
_BOOL AVL68XX_App_Tune_ISDBT(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock);
_BOOL AVL68XX_App_Tune_DVBS2(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock);
_BOOL AVL68XX_App_Stop_Tune(_U8 u8TunerIndex);
void AVL68XX_App_ExitTune(_U8 u8TunerIndex);

void AVL68XX_App_BlindScanStart(_U8 u8TunerIndex, _BlindScanCallback pCallback, PBLIND_SCAN_PARAM_INFO pScanParams);
void AVL68XX_App_BlindScanStop(_U8 u8TunerIndex);

void AVL68XX_App_SetAntennaPower(_U8 u8TunerIndex, _BOOL bPowerOn);

_BOOL AVL68XX_App_SetLNBPower(_U8 u8TunerIndex, _BOOL bPowerOn);
_BOOL AVL68XX_App_SetPolarization(_U8 u8TunerIndex, _BOOL bHigh);
_BOOL AVL68XX_App_SetDiseqcContinuous(_U8 u8TunerIndex, _BOOL bOn);
_BOOL AVL68XX_App_SendDiseqcCommand(_U8 u8TunerIndex, _U8 *pu8Data, _U8 u8Length);
_BOOL AVL68XX_App_SendDiseqcToneBurst(_U8 u8TunerIndex, _U8 u8Command);

_BOOL AVL68XX_App_CheckLocked(_U8 u8TunerIndex, _BOOL *pbLock);
_BOOL AVL68XX_App_GetBer(_U8 u8TunerIndex, _U32 *pu32Ber);
_BOOL AVL68XX_App_GetStrength(_U8 u8TunerIndex, _U16 *pu16Strength);
_BOOL AVL68XX_App_GetSnr(_U8 u8TunerIndex, _U16 *pu16Snr);

void AVL68XX_App_StandBy(_U8 u8TunerIndex);

#endif
