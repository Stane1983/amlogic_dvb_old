//////////////////////////////////////////////////////////////////////////
/////
// TH DTV MIDDLE WARE DEFINES, PLEASE DO NOT MAKE ANY CHANGE OF THIS FILE
/////
//////////////////////////////////////////////////////////////////////////

#ifndef __VS_FE_COMMON_H__
#define __VS_FE_COMMON_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "vs_fe_external.h"
#include "vs_fe_tuner.h"

//////////////////

typedef _BOOL (*_SAT_TUNE_FUNC)(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock);
typedef _BOOL (*_SAT_STOP_TUNE_FUNC)(_U8 u8TunerIndex);
typedef void (*_SAT_EXIT_TUNE_FUNC)(_U8 u8TunerIndex);
typedef void (*_SAT_BLIND_SCAN_START_FUNC)(_U8 u8TunerIndex, _BlindScanCallback pCallback, PBLIND_SCAN_PARAM_INFO pScanParams);
typedef void (*_SAT_BLIND_SCAN_STOP_FUNC)(_U8 u8TunerIndex);
typedef _BOOL (*_SAT_SET_LNB_POWER_FUNC)(_U8 u8TunerIndex, _BOOL bPowerOn);
typedef _BOOL (*_SAT_SET_POLARIZATION_FUNC)(_U8 u8TunerIndex, _BOOL bHigh);
typedef _BOOL (*_SAT_SET_DISEQC_CONTINUOUS_FUNC)(_U8 u8TunerIndex, _BOOL bOn);
typedef _BOOL (*_SAT_SEND_DISEQC_COMMAND_FUNC)(_U8 u8TunerIndex, _U8 *pu8Data, _U8 u8Length);
typedef _BOOL (*_SAT_SEND_DISEQC_TONEBURST_COMMAND_FUNC)(_U8 u8TunerIndex, _U8 u8Command);
typedef _BOOL (*_SAT_CHECK_LOCKED)(_U8 u8TunerIndex, _BOOL *pbLock);
typedef _BOOL (*_SAT_GET_BER)(_U8 u8TunerIndex, _U32 *pu32Ber);
typedef _BOOL (*_SAT_GET_STRENGTH)(_U8 u8TunerIndex, _U16 *pu16Strength);
typedef _BOOL (*_SAT_GET_SNR)(_U8 u8TunerIndex, _U16 *pu16Snr);
typedef void (*_SAT_STAND_BY_FUNC)(_U8 u8TunerIndex);

typedef struct
{
    _SAT_TUNE_FUNC tune_func;
    _SAT_STOP_TUNE_FUNC stop_tune_func;
    _SAT_EXIT_TUNE_FUNC exit_tune_func;
    _SAT_BLIND_SCAN_START_FUNC blind_scan_start_func;
    _SAT_BLIND_SCAN_STOP_FUNC blind_scan_stop_func;
    _SAT_SET_LNB_POWER_FUNC set_lnb_power_func;
    _SAT_SET_POLARIZATION_FUNC set_polarization_func;
    _SAT_SET_DISEQC_CONTINUOUS_FUNC set_diseqc_continuous_func;
    _SAT_SEND_DISEQC_COMMAND_FUNC send_diseqc_command_func;
    _SAT_SEND_DISEQC_TONEBURST_COMMAND_FUNC send_diseqc_toneburst_commond_func;
    _SAT_CHECK_LOCKED check_locked_func;
    _SAT_GET_BER get_ber_func;
    _SAT_GET_STRENGTH get_strength_func;
    _SAT_GET_SNR get_snr_func;
    _SAT_STAND_BY_FUNC standby_func;
} SAT_FUNCS, *PSAT_FUNCS;

extern SAT_FUNCS astSatFuncs[SYSTEM_TUNER_NUMBER];

#define SAT_TUNE(bRet, u8TunerIndex, pstLockInfo, bWaitLock) {\
if(astSatFuncs[u8TunerIndex].tune_func != _NULL)\
    bRet = astSatFuncs[u8TunerIndex].tune_func(u8TunerIndex, pstLockInfo, bWaitLock);\
else\
    bRet = _TRUE;\
}

#define SAT_STOP_TUNE(bRet, u8TunerIndex) {\
if(astSatFuncs[u8TunerIndex].stop_tune_func != _NULL)\
    bRet = astSatFuncs[u8TunerIndex].stop_tune_func(u8TunerIndex);\
else\
    bRet = _TRUE;\
}

#define SAT_EXIT_TUNE(u8TunerIndex) {\
if(astSatFuncs[u8TunerIndex].exit_tune_func!= _NULL)\
    astSatFuncs[u8TunerIndex].exit_tune_func(u8TunerIndex);\
}

#define SAT_BLIND_SCAN_START(u8TunerIndex, pfnBlindScanCallback, pstBlindScanParamInfo) {\
if(astSatFuncs[u8TunerIndex].blind_scan_start_func != _NULL)\
    astSatFuncs[u8TunerIndex].blind_scan_start_func(u8TunerIndex, pfnBlindScanCallback, pstBlindScanParamInfo);\
}

#define SAT_LIND_SCAN_STOP(u8TunerIndex) {\
if(astSatFuncs[u8TunerIndex].blind_scan_stop_func != _NULL)\
    astSatFuncs[u8TunerIndex].blind_scan_stop_func(u8TunerIndex);\
}

#define SAT_SET_LNB_POWER(bRet, u8TunerIndex, bPowerOn) {\
if(astSatFuncs[u8TunerIndex].set_lnb_power_func!= _NULL)\
    bRet = astSatFuncs[u8TunerIndex].set_lnb_power_func(u8TunerIndex, bPowerOn);\
else\
    bRet = _TRUE;\
}

#define SAT_SET_POLARIZATION(bRet, u8TunerIndex, bHigh) {\
if(astSatFuncs[u8TunerIndex].set_polarization_func!= _NULL)\
    bRet = astSatFuncs[u8TunerIndex].set_polarization_func(u8TunerIndex, bHigh);\
else\
    bRet = _TRUE;\
}

#define SAT_SET_DISEQC_CONTINUOUS(bRet, u8TunerIndex, bOn) {\
if(astSatFuncs[u8TunerIndex].set_diseqc_continuous_func!= _NULL)\
    bRet = astSatFuncs[u8TunerIndex].set_diseqc_continuous_func(u8TunerIndex, bOn);\
else\
    bRet = _TRUE;\
}

#define SAT_SEND_DISEQC_COMMAND(bRet, u8TunerIndex, pu8Data, u8Length) {\
if(astSatFuncs[u8TunerIndex].send_diseqc_command_func!= _NULL)\
    bRet = astSatFuncs[u8TunerIndex].send_diseqc_command_func(u8TunerIndex, pu8Data, u8Length);\
else\
    bRet = _TRUE;\
}

#define SAT_SEND_DISEQC_TONEBURST_COMMAND(bRet, u8TunerIndex, u8Command) {\
if(astSatFuncs[u8TunerIndex].send_diseqc_toneburst_commond_func!= _NULL)\
    bRet = astSatFuncs[u8TunerIndex].send_diseqc_toneburst_commond_func(u8TunerIndex, u8Command);\
else\
    bRet = _TRUE;\
}

#define SAT_CHECK_LOCKED(bRet, u8TunerIndex, pbLock) {\
if(astSatFuncs[u8TunerIndex].check_locked_func != _NULL)\
    bRet = astSatFuncs[u8TunerIndex].check_locked_func(u8TunerIndex, pbLock);\
else\
    bRet = _TRUE;\
}

#define SAT_GET_BER(bRet, u8TunerIndex, pu32Ber) {\
if(astSatFuncs[u8TunerIndex].get_ber_func != _NULL)\
    bRet = astSatFuncs[u8TunerIndex].get_ber_func(u8TunerIndex, pu32Ber);\
else\
    bRet = _TRUE;\
}

#define SAT_GET_STRENGTH(bRet, u8TunerIndex, pu16Strength) {\
if(astSatFuncs[u8TunerIndex].get_strength_func != _NULL)\
    bRet = astSatFuncs[u8TunerIndex].get_strength_func(u8TunerIndex, pu16Strength);\
else\
    bRet = _TRUE;\
}

#define SAT_GET_SNR(bRet, u8TunerIndex, pu16Snr) {\
if(astSatFuncs[u8TunerIndex].get_snr_func != _NULL)\
    bRet = astSatFuncs[u8TunerIndex].get_snr_func(u8TunerIndex, pu16Snr);\
else\
    bRet = _TRUE;\
}

#define SAT_STANDBY(u8TunerIndex) {\
if(astSatFuncs[u8TunerIndex].standby_func!= _NULL)\
    astSatFuncs[u8TunerIndex].standby_func(u8TunerIndex);\
}

//////////////////

typedef _BOOL (*_TER_TUNE_FUNC)(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock);
typedef _BOOL (*_TER_STOP_TUNE_FUNC)(_U8 u8TunerIndex);
typedef void (*_TER_EXIT_TUNE_FUNC)(_U8 u8TunerIndex);
typedef void (*_TER_SET_ANTENNA_POWER)(_U8 u8TunerIndex, _BOOL bPowerOn);
typedef _BOOL (*_TER_CHECK_LOCKED)(_U8 u8TunerIndex, _BOOL *pbLock);
typedef _BOOL (*_TER_GET_BER)(_U8 u8TunerIndex, _U32 *pu32Ber);
typedef _BOOL (*_TER_GET_STRENGTH)(_U8 u8TunerIndex, _U16 *pu16Strength);
typedef _BOOL (*_TER_GET_SNR)(_U8 u8TunerIndex, _U16 *pu16Snr);
typedef void (*_TER_STAND_BY_FUNC)(_U8 u8TunerIndex);

typedef struct
{
    _TER_TUNE_FUNC tune_func;
    _TER_STOP_TUNE_FUNC stop_tune_func;
    _TER_EXIT_TUNE_FUNC exit_tune_func;
    _TER_SET_ANTENNA_POWER set_antenna_power_func;
    _TER_CHECK_LOCKED check_locked_func;
    _TER_GET_BER get_ber_func;
    _TER_GET_STRENGTH get_strength_func;
    _TER_GET_SNR get_snr_func;
    _TER_STAND_BY_FUNC standby_func;
} TER_FUNCS, *PTER_FUNCS;

extern TER_FUNCS astTerFuncs[SYSTEM_TUNER_NUMBER][SYSTEM_TYPE_TER_COUNT];

#define TER_TUNE(bRet, u8TunerIndex, enSystemType, pstLockInfo, bWaitLock) {\
if(astTerFuncs[u8TunerIndex][enSystemType].tune_func != _NULL)\
    bRet = astTerFuncs[u8TunerIndex][enSystemType].tune_func(u8TunerIndex, pstLockInfo, bWaitLock);\
else\
    bRet = _TRUE;\
}

#define TER_STOP_TUNE(bRet, u8TunerIndex, enSystemType) {\
if(astTerFuncs[u8TunerIndex][enSystemType].stop_tune_func != _NULL)\
    bRet = astTerFuncs[u8TunerIndex][enSystemType].stop_tune_func(u8TunerIndex);\
else\
    bRet = _TRUE;\
}
#define TER_EXIT_TUNE(u8TunerIndex, enSystemType) {\
if(astTerFuncs[u8TunerIndex][enSystemType].exit_tune_func != _NULL)\
    astTerFuncs[u8TunerIndex][enSystemType].exit_tune_func(u8TunerIndex);\
}

#define TER_SET_ANTENNA_POWER(u8TunerIndex, enSystemType, bPowerOn) {\
if(astTerFuncs[u8TunerIndex][enSystemType].set_antenna_power_func != _NULL)\
    astTerFuncs[u8TunerIndex][enSystemType].set_antenna_power_func(u8TunerIndex, bPowerOn);\
}

#define TER_CHECK_LOCKED(bRet, u8TunerIndex, enSystemType, pbLock) {\
if(astTerFuncs[u8TunerIndex][enSystemType].check_locked_func != _NULL)\
    bRet = astTerFuncs[u8TunerIndex][enSystemType].check_locked_func(u8TunerIndex, pbLock);\
else\
    bRet = _TRUE;\
}

#define TER_GET_BER(bRet, u8TunerIndex, enSystemType, pu32Ber) {\
if(astTerFuncs[u8TunerIndex][enSystemType].get_ber_func != _NULL)\
    bRet = astTerFuncs[u8TunerIndex][enSystemType].get_ber_func(u8TunerIndex, pu32Ber);\
else\
    bRet = _TRUE;\
}

#define TER_GET_STRENGTH(bRet, u8TunerIndex, enSystemType, pu16Strength) {\
if(astTerFuncs[u8TunerIndex][enSystemType].get_strength_func != _NULL)\
    bRet = astTerFuncs[u8TunerIndex][enSystemType].get_strength_func(u8TunerIndex, pu16Strength);\
else\
    bRet = _TRUE;\
}

#define TER_GET_SNR(bRet, u8TunerIndex, enSystemType, pu16Snr) {\
if(astTerFuncs[u8TunerIndex][enSystemType].get_snr_func != _NULL)\
    bRet = astTerFuncs[u8TunerIndex][enSystemType].get_snr_func(u8TunerIndex, pu16Snr);\
else\
    bRet = _TRUE;\
}

#define TER_STAND_BY(u8TunerIndex, enSystemType) {\
if(astTerFuncs[u8TunerIndex][enSystemType].standby_func != _NULL)\
    astTerFuncs[u8TunerIndex][enSystemType].standby_func(u8TunerIndex);\
}

//////////////////

_BOOL _Tuner_Detect(PVS_FE_CTRL pCtrl);

//////////////////

#ifdef __cplusplus
}
#endif

#endif    // __VS_FE_COMMON_H__

//////////////////////////////////////////////////////////////////////////
/////
// TH DTV MIDDLE WARE DEFINES, PLEASE DO NOT MAKE ANY CHANGE OF THIS FILE
/////
//////////////////////////////////////////////////////////////////////////
