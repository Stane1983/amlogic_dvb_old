#include "vs_fe_common.h"

//#define VS_FE_COMMON_DEBUG_LEVEL DEBUG_LEVEL_INFO

#undef DEBUG_LEVEL
#undef DEBUG_TAG

#ifdef VS_FE_COMMON_DEBUG_LEVEL
#define DEBUG_LEVEL VS_FE_COMMON_DEBUG_LEVEL
#endif

#include "vs_fe_debug.h"

#include "avl68xx_app_porting.h"

#include "rs6060_app_porting.h"

SAT_FUNCS astSatFuncs[SYSTEM_TUNER_NUMBER];
TER_FUNCS astTerFuncs[SYSTEM_TUNER_NUMBER][SYSTEM_TYPE_TER_COUNT];

_BOOL _Tuner_Detect(PVS_FE_CTRL pstCtrl)
{
    _BOOL tunerMatched = _FALSE;
    _BOOL needCheck = _FALSE;
    _U8 retryCount = 0;

    pstCtrl->stSystemInfo.bSystemDetect[pstCtrl->u8TunerIndex] = _FALSE;

RETRY_INIT:

    needCheck = _FALSE;
    tunerMatched = _TRUE;

    //TODO: Waste time for dual tuner
    if(0 < retryCount){
        VS_FE_SetTunerPowerEnable(pstCtrl->u8TunerIndex, 0);

        External_Delay(200);
    }

    VS_FE_SetTunerPowerEnable(pstCtrl->u8TunerIndex, 1);

    External_Delay(50);

    VS_FE_SetDemodReset(pstCtrl->u8TunerIndex, 0);

    External_Delay(100);

    VS_FE_SetDemodReset(pstCtrl->u8TunerIndex, 1);

    External_Delay(100);

    if (pstCtrl->stSystemInfo.bSystemDetect[pstCtrl->u8TunerIndex]) {
        tunerMatched = _FALSE;
        needCheck = _FALSE;
    }

    if (tunerMatched && (TUNER_ID_COMBO_AVL68XX == pstCtrl->enTunerId)) {
        _U32 systemType;
        _U32 chipId;

        DBG_VERBOSE("Try Demod[%d] [ Combo AVL68XX ]..\n", pstCtrl->u8TunerIndex);

        systemType = AVL68XX_App_Init(pstCtrl->u8TunerIndex, &chipId);
        if (0 != systemType) {
            if ((systemType & AVL68XX_SYSTEM_TYPE_DVB_T) == AVL68XX_SYSTEM_TYPE_DVB_T) {
                TER_FUNCS funcs = {AVL68XX_App_Tune_DVBT2, AVL68XX_App_Stop_Tune, AVL68XX_App_ExitTune, \
                    AVL68XX_App_SetAntennaPower, \
                    AVL68XX_App_CheckLocked, AVL68XX_App_GetBer, AVL68XX_App_GetStrength, AVL68XX_App_GetSnr, \
                    AVL68XX_App_StandBy};
                                            
                astTerFuncs[pstCtrl->u8TunerIndex][SYSTEM_TYPE_DVB_TX] = funcs;
                pstCtrl->stSystemInfo.bSystemTypeDetect[pstCtrl->u8TunerIndex][SYSTEM_TYPE_DVB_TX] = _TRUE;
                pstCtrl->stSystemInfo.bSystemDetect[pstCtrl->u8TunerIndex] = _TRUE;
            }

            if ((systemType & AVL68XX_SYSTEM_TYPE_DVB_C) == AVL68XX_SYSTEM_TYPE_DVB_C) {
                TER_FUNCS funcs = {AVL68XX_App_Tune_DVBC, AVL68XX_App_Stop_Tune, AVL68XX_App_ExitTune, \
                    AVL68XX_App_SetAntennaPower, \
                    AVL68XX_App_CheckLocked, AVL68XX_App_GetBer, AVL68XX_App_GetStrength, AVL68XX_App_GetSnr, \
                    AVL68XX_App_StandBy};
                                            
                astTerFuncs[pstCtrl->u8TunerIndex][SYSTEM_TYPE_DVB_C] = funcs;
                pstCtrl->stSystemInfo.bSystemTypeDetect[pstCtrl->u8TunerIndex][SYSTEM_TYPE_DVB_C] = _TRUE;
                pstCtrl->stSystemInfo.bSystemDetect[pstCtrl->u8TunerIndex] = _TRUE;
            } 
            
            if ((systemType & AVL68XX_SYSTEM_TYPE_ISDB_T) == AVL68XX_SYSTEM_TYPE_ISDB_T) {
                TER_FUNCS funcs = {AVL68XX_App_Tune_ISDBT, AVL68XX_App_Stop_Tune, AVL68XX_App_ExitTune, \
                    AVL68XX_App_SetAntennaPower, \
                    AVL68XX_App_CheckLocked, AVL68XX_App_GetBer, AVL68XX_App_GetStrength, AVL68XX_App_GetSnr, \
                    AVL68XX_App_StandBy};
                                            
                astTerFuncs[pstCtrl->u8TunerIndex][SYSTEM_TYPE_ISDB_T] = funcs;
                pstCtrl->stSystemInfo.bSystemTypeDetect[pstCtrl->u8TunerIndex][SYSTEM_TYPE_ISDB_T] = _TRUE;
                pstCtrl->stSystemInfo.bSystemDetect[pstCtrl->u8TunerIndex] = _TRUE;
            }        

            if ((systemType & AVL68XX_SYSTEM_TYPE_DVB_S) == AVL68XX_SYSTEM_TYPE_DVB_S) {
                SAT_FUNCS funcs = {AVL68XX_App_Tune_DVBS2, AVL68XX_App_Stop_Tune, AVL68XX_App_ExitTune, \
                    AVL68XX_App_BlindScanStart, AVL68XX_App_BlindScanStop, \
                    AVL68XX_App_SetLNBPower, AVL68XX_App_SetPolarization, \
                    AVL68XX_App_SetDiseqcContinuous, AVL68XX_App_SendDiseqcCommand, AVL68XX_App_SendDiseqcToneBurst, \
                    AVL68XX_App_CheckLocked, AVL68XX_App_GetBer, AVL68XX_App_GetStrength, AVL68XX_App_GetSnr, \
                    AVL68XX_App_StandBy};                                            
                                                    
                astSatFuncs[pstCtrl->u8TunerIndex] = funcs;
                pstCtrl->stSystemInfo.bSystemTypeDetect[pstCtrl->u8TunerIndex][SYSTEM_TYPE_SAT] = _TRUE;
                pstCtrl->stSystemInfo.bSystemDetect[pstCtrl->u8TunerIndex] = _TRUE;
            }

            switch(chipId) {
                case 6862:
                    DBG_VERBOSE("Detect demod type [ Combo AVL6862 ] !\n");
                    break;
                case 6812:
                    DBG_VERBOSE("Detect demod type [ Combo AVL6812 ] !\n");
                    break;
                case 6882:
                    DBG_VERBOSE("Detect demod type [ Combo AVL6882 ] !\n");
                    break;                         
                case 6762:
                    DBG_VERBOSE("Detect demod type [ Combo AVL6762 ] !\n");
                    break;
                default:
                    DBG_VERBOSE("Detect unknown demod type [ %d ] !\n", chipId);
                    break;
            }
        } else {
            needCheck = _TRUE;
        }
    }

    if (tunerMatched && (TUNER_ID_RS6060 == pstCtrl->enTunerId)) {
        _U32 ret;

        DBG_VERBOSE("Try Demod[%d] [ DVBS/S2/S2X RS6060 ]..\n", pstCtrl->u8TunerIndex);

        ret = RS6060_App_Init(pstCtrl->u8TunerIndex);
        if (_SUCCESS == ret) {
            SAT_FUNCS funcs = {RS6060_App_Tune, RS6060_App_Stop_Tune, RS6060_App_ExitTune, \
                RS6060_App_BlindScanStart, RS6060_App_BlindScanStop, \
                RS6060_App_SetLNBPower, RS6060_App_SetPolarization, \
                RS6060_App_SetDiseqcContinuous, RS6060_App_SendDiseqcCommand, RS6060_App_SendDiseqcToneBurst, \
                RS6060_App_CheckLocked, RS6060_App_GetBer, RS6060_App_GetStrength, RS6060_App_GetSnr, \
                RS6060_App_StandBy};

            astSatFuncs[pstCtrl->u8TunerIndex] = funcs;
            pstCtrl->stSystemInfo.bSystemTypeDetect[pstCtrl->u8TunerIndex][SYSTEM_TYPE_SAT] = _TRUE;
            pstCtrl->stSystemInfo.bSystemDetect[pstCtrl->u8TunerIndex] = _TRUE;

            DBG_VERBOSE("Detect demod type [ DVBS/S2/S2X RS6060 ] !\n");
        } else {
            needCheck = _TRUE;
        }
    }

    if (needCheck) {
        if (retryCount++ < 1) {
            DBG_VERBOSE("Detect demod type failed, retry !\n");
            goto RETRY_INIT;
        }

        return _FALSE;
    }

    return _TRUE;
}
