#include <linux/string.h>

#include "vs_fe_external.h"
#include "vs_fe_common.h"
#include "vs_fe_tuner.h"

//#define VS_FE_TUNER_DEBUG_LEVEL DEBUG_LEVEL_INFO

#undef DEBUG_LEVEL
#undef DEBUG_TAG

#ifdef VS_FE_TUNER_DEBUG_LEVEL
#define DEBUG_LEVEL VS_FE_TUNER_DEBUG_LEVEL
#endif

#include "vs_fe_debug.h"

static EXTERNAL_SEMAPHORE g_pSemphoreTunerAccess;

//TODO: check param

#define GET_PIN(x) \
extern int dtv_params_get_##x(unsigned char *);

#define SET_PIN(x) \
extern int dtv_params_set_##x(unsigned char);

GET_PIN(demod_reset_1)
GET_PIN(demod_reset_2)
GET_PIN(ant_power_1)
GET_PIN(ant_power_2)
GET_PIN(tuner_power_enable_1)
GET_PIN(tuner_power_enable_2)
GET_PIN(user_defined_1)
GET_PIN(user_defined_2)

SET_PIN(demod_reset_1)
SET_PIN(demod_reset_2)
SET_PIN(ant_power_1)
SET_PIN(ant_power_2)
SET_PIN(tuner_power_enable_1)
SET_PIN(tuner_power_enable_2)
SET_PIN(user_defined_1)
SET_PIN(user_defined_2)

SYSTEM_TYPE_E _Tuner_GetSystemType(enum fe_delivery_system enDeliverySystem)
{
    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;

    switch (enDeliverySystem) {
        case SYS_DVBT:
        case SYS_DVBT2:
            enSystemType = SYSTEM_TYPE_DVB_TX;
            break;

        case SYS_DVBC_ANNEX_A:
        case SYS_DVBC_ANNEX_B:
        case SYS_DVBC_ANNEX_C:
            enSystemType = SYSTEM_TYPE_DVB_C;
            break;

        case SYS_DVBS:
        case SYS_DVBS2:
            enSystemType = SYSTEM_TYPE_SAT;
            break;

        case SYS_ISDBT:
            enSystemType = SYSTEM_TYPE_DTMB;
            break;

        case SYS_ANALOG:
            enSystemType = SYSTEM_TYPE_DVB_TX;
            break;

        default:
            enSystemType = SYSTEM_TYPE_INVALID;
            break;
    }

    return enSystemType;
}

_BOOL _Tuner_CheckSat(enum fe_delivery_system enDeliverySystem)
{
    _BOOL bSat = _TRUE;

    switch (enDeliverySystem) {
        case SYS_DVBT:
        case SYS_DVBT2:
            bSat = _FALSE;
            break;

        case SYS_DVBC_ANNEX_A:
        case SYS_DVBC_ANNEX_B:
        case SYS_DVBC_ANNEX_C:
            bSat = _FALSE;
            break;

        case SYS_DVBS:
        case SYS_DVBS2:
            bSat = _TRUE;
            break;

        case SYS_ISDBT:
            bSat = _FALSE;
            break;

        default:
            bSat = _FALSE;
            break;
    }

    return bSat;
}

static _BOOL _Tuner_Init(PVS_FE_CTRL pstCtrl)
{
    _BOOL bRet = _TRUE;

    bRet |= _Tuner_Detect(pstCtrl);

    return bRet;
}

void VS_FE_SetDemodReset(_U8 u8TunerIndex, _U8 u8Value)
{
    int ret = 0;

    DBG_INFO("");

    if (0 == u8TunerIndex) {
        ret = dtv_params_set_demod_reset_1(u8Value);
    } else {
        ret = dtv_params_set_demod_reset_2(u8Value);
    }

    return ;
}

void VS_FE_SetAntPower(_U8 u8TunerIndex, _U8 u8Value)
{
    int ret = 0;

    DBG_INFO("");

    if (0 == u8TunerIndex) {
        ret = dtv_params_set_ant_power_1(u8Value);
    } else {
        ret = dtv_params_set_ant_power_2(u8Value);
    }

    return ;
}

void VS_FE_SetTunerPowerEnable(_U8 u8TunerIndex, _U8 u8Value)
{
    int ret = 0;

    DBG_INFO("");

    if (0 == u8TunerIndex) {
        ret = dtv_params_set_tuner_power_enable_1(u8Value);
    } else {
        ret = dtv_params_set_tuner_power_enable_2(u8Value);
    }

    return ;
}

void VS_FE_SetUserDefined1(_U8 u8Value)
{
    int ret = 0;

    DBG_INFO("");

    ret = dtv_params_set_user_defined_1(u8Value);

    return ;
}

void VS_FE_SetUserDefined2(_U8 u8Value)
{
    int ret = 0;

    DBG_INFO("");

    ret = dtv_params_set_user_defined_2(u8Value);

    return ;
}

void VS_FE_GetDemodReset(_U8 u8TunerIndex, _U8 *pu8Value)
{
    int ret = 0;

    DBG_INFO("");

    if (0 == u8TunerIndex) {
        ret = dtv_params_get_demod_reset_1(pu8Value);
    } else {
        ret = dtv_params_get_demod_reset_2(pu8Value);
    }

    return ;
}

void VS_FE_GetAntPower(_U8 u8TunerIndex, _U8 *pu8Value)
{
    int ret = 0;

    DBG_INFO("");

    if (0 == u8TunerIndex) {
        ret = dtv_params_get_ant_power_1(pu8Value);
    } else {
        ret = dtv_params_get_ant_power_2(pu8Value);
    }

    return ;
}

void VS_FE_GetTunerPowerEnable(_U8 u8TunerIndex, _U8 *pu8Value)
{
    int ret = 0;

    DBG_INFO("");

    if (0 == u8TunerIndex) {
        ret = dtv_params_get_tuner_power_enable_1(pu8Value);
    } else {
        ret = dtv_params_get_tuner_power_enable_2(pu8Value);
    }

    return ;
}

void VS_FE_GetUserDefined1(_U8 *pu8Value)
{
    int ret = 0;

    DBG_INFO("");

    ret = dtv_params_get_user_defined_1(pu8Value);

    return ;
}

void VS_FE_GetUserDefined2(_U8 *pu8Value)
{
    int ret = 0;

    DBG_INFO("");

    ret = dtv_params_get_user_defined_2(pu8Value);

    return ;
}

_BOOL VS_FE_Tuner_GetSystemInfo(enum fe_delivery_system enDeliverySystem, PSYSTEM_TYPE_E penSystemType)
{
    _BOOL bRet = _TRUE;

    DBG_INFO("");

    *penSystemType = _Tuner_GetSystemType(enDeliverySystem);

    return bRet;
}

_BOOL VS_FE_Tuner_Init(PVS_FE_CTRL pstCtrl)
{
    _BOOL bRet = _TRUE;

    DBG_INFO("");

    if (!pstCtrl->bInitFlag) {
        if (!External_CreateSemaphore(&g_pSemphoreTunerAccess)) {
            return _FALSE;
        }

        memset((void*)astSatFuncs, 0, sizeof(astSatFuncs));
        memset((void*)astTerFuncs, 0, sizeof(astTerFuncs));

        External_AcquireSemaphore(g_pSemphoreTunerAccess);

        bRet |= _Tuner_Init(pstCtrl);

        External_ReleaseSemaphore(g_pSemphoreTunerAccess);

        bRet |= VS_FE_Tuner_SetVoltage(pstCtrl, SEC_VOLTAGE_OFF);

        pstCtrl->bInitFlag = 1;
    }

    return bRet;
}

_BOOL VS_FE_Tuner_Standby(PVS_FE_CTRL pstCtrl)
{
    _BOOL bRet = _TRUE;

    _U8 u8Index = 0;

    DBG_INFO("");

    if (!pstCtrl->bSuspendFlag) {
        External_AcquireSemaphore(g_pSemphoreTunerAccess);

        SAT_STANDBY(MAIN_TUNER_INDEX);
        SAT_STANDBY(SECOND_TUNER_INDEX);

        for (u8Index = 0 ; u8Index < SYSTEM_TYPE_TER_COUNT; u8Index++) {
            TER_STAND_BY(MAIN_TUNER_INDEX, u8Index);
        }
        for (u8Index = 0 ; u8Index < SYSTEM_TYPE_TER_COUNT; u8Index++) {
            TER_STAND_BY(SECOND_TUNER_INDEX, u8Index);
        }

        //TODO: VS_FE_Tuner_SetTunerPowerMode

        External_ReleaseSemaphore(g_pSemphoreTunerAccess);

        pstCtrl->bSuspendFlag = 1;
    }

    return bRet;
}

_BOOL VS_FE_Tuner_Resume(PVS_FE_CTRL pstCtrl)
{
    _BOOL bRet = _TRUE;

    DBG_INFO("");

    if (pstCtrl->bSuspendFlag) {
        External_AcquireSemaphore(g_pSemphoreTunerAccess);

        bRet |= _Tuner_Init(pstCtrl);

        External_ReleaseSemaphore(g_pSemphoreTunerAccess);

        pstCtrl->bSuspendFlag = 0;
    }

    return bRet;
}

_BOOL VS_FE_Tuner_StartLock(PVS_FE_CTRL pstCtrl, PSCAN_CALLBACK_INFO_T pstScanCallbackInfo, _BOOL bWaitLock)
{
    _BOOL bRet = _TRUE;

    LOCK_PARAM_INFO_T stLockInfo = {0};

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;
    _BOOL bSat = _TRUE;

    DBG_INFO("");

    enSystemType = _Tuner_GetSystemType(pstCtrl->enDeliverySystem);
    bSat = _Tuner_CheckSat(pstCtrl->enDeliverySystem);

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    stLockInfo.stTunerParamInfo = pstCtrl->stTunerParamInfo;
    stLockInfo.pstScanCallbackInfo = pstScanCallbackInfo;

    if (bSat) {
        SAT_TUNE(bRet, pstCtrl->u8TunerIndex, &stLockInfo, bWaitLock);
    } else {
        TER_TUNE(bRet, pstCtrl->u8TunerIndex, enSystemType, &stLockInfo, bWaitLock);
    }

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_StopLock(PVS_FE_CTRL pstCtrl)
{
    _BOOL bRet = _TRUE;

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;
    _BOOL bSat = _TRUE;

    DBG_INFO("");

    enSystemType = _Tuner_GetSystemType(pstCtrl->enDeliverySystem);
    bSat = _Tuner_CheckSat(pstCtrl->enDeliverySystem);

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    if (bSat) {
        SAT_STOP_TUNE(bRet, pstCtrl->u8TunerIndex);
    } else {
        TER_STOP_TUNE(bRet, pstCtrl->u8TunerIndex, enSystemType);
    }

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_ExitLock(PVS_FE_CTRL pstCtrl)
{
    _BOOL bRet = _TRUE;

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;
    _BOOL bSat = _TRUE;

    DBG_INFO("");

    enSystemType = _Tuner_GetSystemType(pstCtrl->enDeliverySystem);
    bSat = _Tuner_CheckSat(pstCtrl->enDeliverySystem);

//    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    if (bSat) {
        SAT_EXIT_TUNE(pstCtrl->u8TunerIndex);
    } else {
        TER_EXIT_TUNE(pstCtrl->u8TunerIndex, enSystemType);
    }

//    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_Sat_BlindScan_Start(PVS_FE_CTRL pstCtrl, _BlindScanCallback pfnBlindScanCallback, PBLIND_SCAN_PARAM_INFO pstBlindScanParamInfo)
{
    _BOOL bRet = _TRUE;

    DBG_INFO("");

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    SAT_BLIND_SCAN_START(pstCtrl->u8TunerIndex, pfnBlindScanCallback, pstBlindScanParamInfo);

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_Sat_BlindScan_Stop(PVS_FE_CTRL pstCtrl)
{
    _BOOL bRet = _TRUE;

    DBG_INFO("");

//    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    SAT_LIND_SCAN_STOP(pstCtrl->u8TunerIndex);

//    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_SetVoltage(PVS_FE_CTRL pstCtrl, enum fe_sec_voltage enVoltage)
{
    _BOOL bRet = _TRUE;

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;
    _BOOL bSat = _TRUE;

    DBG_INFO("");

    enSystemType = _Tuner_GetSystemType(pstCtrl->enDeliverySystem);
    bSat = _Tuner_CheckSat(pstCtrl->enDeliverySystem);

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    if (SEC_VOLTAGE_OFF == enVoltage) {
        SAT_SET_LNB_POWER(bRet, pstCtrl->u8TunerIndex, _FALSE);
        TER_SET_ANTENNA_POWER(pstCtrl->u8TunerIndex, enSystemType, (SEC_VOLTAGE_ON == enVoltage) ? _TRUE : _FALSE);
        pstCtrl->enCurAntVoltage = enVoltage;
    } else if(SEC_VOLTAGE_ON == enVoltage) {
        TER_SET_ANTENNA_POWER(pstCtrl->u8TunerIndex, enSystemType, (SEC_VOLTAGE_ON == enVoltage) ? _TRUE : _FALSE);
        pstCtrl->enCurAntVoltage = enVoltage;
    } else {
        SAT_SET_LNB_POWER(bRet, pstCtrl->u8TunerIndex, _TRUE);
        SAT_SET_POLARIZATION(bRet, pstCtrl->u8TunerIndex, (SEC_VOLTAGE_18 == enVoltage) ? _TRUE : _FALSE);
    }

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_Sat_SetDiseqcContinuous(PVS_FE_CTRL pstCtrl, _BOOL bOn)
{
    _BOOL bRet = _TRUE;

    DBG_INFO("");

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    SAT_SET_DISEQC_CONTINUOUS(bRet, pstCtrl->u8TunerIndex, bOn);

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_Sat_SendDiseqcCommand(PVS_FE_CTRL pstCtrl, _U8 *pu8Data,_U8 u8Length)
{
    _BOOL bRet = _TRUE;

    DBG_INFO("");

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    SAT_SEND_DISEQC_COMMAND(bRet, pstCtrl->u8TunerIndex, pu8Data, u8Length);

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return _TRUE;
}

_BOOL VS_FE_Tuner_Sat_SendDiseqcBurst(PVS_FE_CTRL pstCtrl, _U8 u8Command)
{
    _BOOL bRet = _TRUE;

    DBG_INFO("");

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    SAT_SEND_DISEQC_TONEBURST_COMMAND(bRet, pstCtrl->u8TunerIndex, u8Command);

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return _TRUE;
}

_BOOL VS_FE_Tuner_CheckLocked(PVS_FE_CTRL pstCtrl, _BOOL *pbLock)
{
    _BOOL bRet = _TRUE;

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;
    _BOOL bSat = _TRUE;

    DBG_INFO("");

    enSystemType = _Tuner_GetSystemType(pstCtrl->enDeliverySystem);
    bSat = _Tuner_CheckSat(pstCtrl->enDeliverySystem);

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    if (bSat) {
        SAT_CHECK_LOCKED(bRet, pstCtrl->u8TunerIndex, pbLock);
    } else {
        TER_CHECK_LOCKED(bRet, pstCtrl->u8TunerIndex, enSystemType, pbLock);
    }

    DBG_INFO("ret: %d, lock: %d", bRet, *pbLock);

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_GetBer(PVS_FE_CTRL pstCtrl, _U32 *pu32Ber)
{
    _BOOL bRet = _TRUE;

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;
    _BOOL bSat = _TRUE;

    DBG_INFO("");

    enSystemType = _Tuner_GetSystemType(pstCtrl->enDeliverySystem);
    bSat = _Tuner_CheckSat(pstCtrl->enDeliverySystem);

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    if (bSat) {
        SAT_GET_BER(bRet, pstCtrl->u8TunerIndex, pu32Ber);
    } else {
        TER_GET_BER(bRet, pstCtrl->u8TunerIndex, enSystemType, pu32Ber);
    }

    DBG_INFO("ret: %d, ber: %d", bRet, *pu32Ber);

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_GetStrength(PVS_FE_CTRL pstCtrl, _U16 *pu16Strength)
{
    _BOOL bRet = _TRUE;

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;
    _BOOL bSat = _TRUE;

    DBG_INFO("");

    enSystemType = _Tuner_GetSystemType(pstCtrl->enDeliverySystem);
    bSat = _Tuner_CheckSat(pstCtrl->enDeliverySystem);

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    if (bSat) {
        SAT_GET_STRENGTH(bRet, pstCtrl->u8TunerIndex, pu16Strength);
    } else {
        TER_GET_STRENGTH(bRet, pstCtrl->u8TunerIndex, enSystemType, pu16Strength);
    }

    DBG_INFO("ret: %d, strength: %d", bRet, *pu16Strength);

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}

_BOOL VS_FE_Tuner_GetSnr(PVS_FE_CTRL pstCtrl, _U16 *pu16Snr)
{
    _BOOL bRet = _TRUE;

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;
    _BOOL bSat = _TRUE;

    DBG_INFO("");

    enSystemType = _Tuner_GetSystemType(pstCtrl->enDeliverySystem);
    bSat = _Tuner_CheckSat(pstCtrl->enDeliverySystem);

    External_AcquireSemaphore(g_pSemphoreTunerAccess);

    if (bSat) {
        SAT_GET_SNR(bRet, pstCtrl->u8TunerIndex, pu16Snr);
    } else {
        TER_GET_SNR(bRet, pstCtrl->u8TunerIndex, enSystemType, pu16Snr);
    }

    DBG_INFO("ret: %d, snr: %d", bRet, *pu16Snr);

    External_ReleaseSemaphore(g_pSemphoreTunerAccess);

    return bRet;
}
