#include "rs6060_app_porting.h"

#include "mt_fe_def.h"
#include "mt_fe_i2c.h"

#include "vs_fe_debug.h"

#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define _PRINTF_ _Print
#else
#define _PRINTF_(...)
#endif

#define RS6060_CHIP_COUNT (2)

#define RS6060_MULTI_SET_TS_INDEX (0)

/*Use plp_id field to store dvb_type, plp_id is not used now*/

MT_FE_RS6060_DEVICE_SETTINGS g_astRs6060Config[RS6060_CHIP_COUNT];

/*End customer define */

static _BOOL g_abNeedExitLockTuner[RS6060_CHIP_COUNT] = {_FALSE, _FALSE};

static _U8 g_BlindScanTunerIndex = 0;
static _BlindScanCallback g_pBlindScanCallback = _NULL;
static _BOOL g_bInBlindScanLoop = _FALSE;
static _U32 g_BlindScanBeginFreqKHz = 0, g_BlindScanEndFreqKHz = 0;

void RS6060_SetExitLockTuner(_U8 u8TunerIndex, _BOOL bExitLock)
{
    g_abNeedExitLockTuner[u8TunerIndex] = bExitLock;
}

_BOOL RS6060_GetExitLockTuner(_U8 u8TunerIndex)
{
    return g_abNeedExitLockTuner[u8TunerIndex];
}

MT_FE_RS6060_Device_Handle RS6060_GetHandleByFeIndex(_U8 u8TunerIndex)
{
    if(RS6060_CHIP_COUNT < u8TunerIndex)
    {
        return NULL;
    }

    return &g_astRs6060Config[u8TunerIndex];
}

MT_FE_RET RS6060_LockChannel(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    U32 freq_MHz = 0;
    U32 sym_rate_KSs = 0;
    MT_FE_TYPE dvbs_type = MtFeType_Undef;

    S8 lock_timeout_cnt = 0;

    MT_FE_LOCK_STATE enLockState = MtFeLockState_Undef;

    _U8 u8Index = 0;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return MtFeErr_Uninit;
    }

    RS6060_SetExitLockTuner(u8TunerIndex, _FALSE);

    freq_MHz = pstLockInfo->stTunerParamInfo.u32Frequency;
    sym_rate_KSs = pstLockInfo->stTunerParamInfo.u32Symbolrate;

    if(FRONTEND_TYPE_DVBS2_MIS == pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.enFrontendType)
    {
        dvbs_type = pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32MutiAttr; //MtFeType_DvbS2 or MtFeType_DTV_Unknown
        pstRs6060Handle->tp_cfg.ucCurTsId = pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32StreamId; //mt_fe_dmd_rs6060_get_lock_state
    }
    else
    {
        dvbs_type = MtFeType_DTV_Unknown;
    }

    enRet = mt_fe_dmd_rs6060_connect(pstRs6060Handle, freq_MHz, sym_rate_KSs, dvbs_type);
    if(MtFeErr_Ok != enRet)
    {
        return MtFeErr_UnLock;
    }

    if(bWaitLock)
    {
        if(dvbs_type == MtFeType_DvbS)
        {
            if(sym_rate_KSs >= 2000)        lock_timeout_cnt = 10;    //1s
            else                            lock_timeout_cnt = 60;    //6s
        }
        else if(dvbs_type == MtFeType_DvbS2)
        {
            if(sym_rate_KSs >= 10000)        lock_timeout_cnt = 8;  //800ms
            else if(sym_rate_KSs >= 5000)    lock_timeout_cnt = 15; //1.5s
            else if(sym_rate_KSs >= 2000)    lock_timeout_cnt = 30; //3s
            else                            lock_timeout_cnt = 60; //6s
        }
        else// if(dvbs_type == MtFeType_DTV_Unknown)
        {
            if(sym_rate_KSs >= 10000)        lock_timeout_cnt = 13; //1.3s
            else if(sym_rate_KSs >= 4000)    lock_timeout_cnt = 32; //3.2s
            else if(sym_rate_KSs >= 2000)    lock_timeout_cnt = 35; //3.5s
            else                            lock_timeout_cnt = 85; //8.5s
        }

        do
        {
            enRet = mt_fe_dmd_rs6060_get_lock_state(pstRs6060Handle, &enLockState);
            if((MtFeErr_Ok == enRet) && (MtFeLockState_Locked == enLockState))
            {
                _Print("RS6060_LockChannel, mt_fe_dmd_rs6060_get_lock_state success");

                break;
            }

            //MtFeLockState_Unlocked

            _mt_sleep(100); /* timeout = 100ms */
        }while(--lock_timeout_cnt && !RS6060_GetExitLockTuner(u8TunerIndex));

        if(lock_timeout_cnt <= 0)
        {
            _Print("RS6060_LockChannel, MtFeErr_TimeOut");

            return MtFeErr_TimeOut;
        }
        else if(!RS6060_GetExitLockTuner(u8TunerIndex))
        {
            if((FRONTEND_TYPE_Undef == pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.enFrontendType) && (NULL != pstLockInfo->pstScanCallbackInfo))
            {
                if(MtFeType_DvbS == pstRs6060Handle->tp_cfg.mCurrentType)
                {
                    _Print("RS6060_LockChannel, FRONTEND_TYPE_DVBSX");

                    pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[0].enFrontendType = FRONTEND_TYPE_DVBSX;
                    pstLockInfo->pstScanCallbackInfo->u32ChannelCount = 1;
                }
                else if(MtFeType_DvbS2 == pstRs6060Handle->tp_cfg.mCurrentType)
                {
                    if(0 == pstRs6060Handle->tp_cfg.iTsCnt)
                    {
                        _Print("RS6060_LockChannel, FRONTEND_TYPE_DVBSX");

                        pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[0].enFrontendType = FRONTEND_TYPE_DVBSX;
                        pstLockInfo->pstScanCallbackInfo->u32ChannelCount = 1;
                    }
                    else
                    {
                        _Print("RS6060_LockChannel, FRONTEND_TYPE_DVBS2_MIS");

                        for(u8Index = 0; u8Index < pstRs6060Handle->tp_cfg.iTsCnt; u8Index++)
                        {
                            pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[u8Index].enFrontendType = FRONTEND_TYPE_DVBS2_MIS;
                            pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[u8Index].u32MutiAttr = pstRs6060Handle->tp_cfg.mCurrentType;
                            pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[u8Index].u32StreamId = pstRs6060Handle->tp_cfg.ucTsId[u8Index];

                            if(0 == u8Index)
                            {
                                enRet = mt_fe_dmd_rs6060_set_ts(pstRs6060Handle, RS6060_MULTI_SET_TS_INDEX, pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[u8Index].u32StreamId);
                            }
                        }

                        pstLockInfo->pstScanCallbackInfo->u32ChannelCount = pstRs6060Handle->tp_cfg.iTsCnt;
                    }
                }
                else
                {
                    _Print("RS6060_LockChannel, Unsupported type: %d", pstRs6060Handle->tp_cfg.mCurrentType);
                }
            }
        }
    }
    else
    {
        if(FRONTEND_TYPE_DVBS2_MIS == pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.enFrontendType)
        {
            enRet = mt_fe_dmd_rs6060_set_ts(pstRs6060Handle, RS6060_MULTI_SET_TS_INDEX, pstRs6060Handle->tp_cfg.ucCurTsId);
        }
    }

    return enRet;
}

void RS6060_BlindScanCallback(MT_FE_MSG msg, void *p_tp_info)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    BLIND_SCAN_CALLBACK_INFO stBsCbInfo = {0};

    MT_FE_TP_INFO* pstTpInfo = NULL;

    _U8 u8Index = 0;

    static U32 u32LastProcess = 0;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(g_BlindScanTunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return ;
    }

    if(NULL != g_pBlindScanCallback)
    {
        if((MtFeMsg_BSStart == msg) || (MtFeMsg_BSFinish == msg))
        {
            u32LastProcess = 0;
        }
        else if(((MtFeMsg_BSTpLocked == msg) || (MtFeMsg_BSTpUnlock == msg)) && (NULL != p_tp_info))
        {
            pstTpInfo = (MT_FE_TP_INFO *)p_tp_info;

            _Print("RS6060_BlindScanCallback [%dKHz, %dKSps]", pstTpInfo->freq_KHz, pstTpInfo->sym_rate_KSs);

            memset(&stBsCbInfo, 0, sizeof(BLIND_SCAN_CALLBACK_INFO));
            stBsCbInfo.u32Frequency = pstTpInfo->freq_KHz;
            stBsCbInfo.u32Symbolrate = pstTpInfo->sym_rate_KSs;
            if(g_BlindScanEndFreqKHz == g_BlindScanBeginFreqKHz)
            {
                stBsCbInfo.u8Process = u32LastProcess = 100;
            }
            else if((g_BlindScanBeginFreqKHz > pstTpInfo->freq_KHz) || (g_BlindScanEndFreqKHz < pstTpInfo->freq_KHz))
            {
                stBsCbInfo.u8Process = u32LastProcess;
            }
            else
            {
                stBsCbInfo.u8Process = u32LastProcess = (pstTpInfo->freq_KHz - g_BlindScanBeginFreqKHz) * 100 / (g_BlindScanEndFreqKHz - g_BlindScanBeginFreqKHz);
            }
            stBsCbInfo.bFrontendAttrInfoValid = (0 != pstTpInfo->iTsCnt) ? _TRUE : _FALSE;
            if(stBsCbInfo.bFrontendAttrInfoValid)
            {
                stBsCbInfo.bLocked = (MtFeMsg_BSTpLocked == msg) ? 1 : 0;

                for(u8Index = 0; u8Index < pstTpInfo->iTsCnt; u8Index++)
                {
                    stBsCbInfo.stFrontendAttrInfo.enFrontendType = FRONTEND_TYPE_DVBS2_MIS;
                    stBsCbInfo.stFrontendAttrInfo.u32MutiAttr = pstRs6060Handle->tp_cfg.mCurrentType;
                    stBsCbInfo.stFrontendAttrInfo.u32StreamId = pstRs6060Handle->tp_cfg.ucTsId[u8Index];

                    enRet = mt_fe_dmd_rs6060_set_ts(pstRs6060Handle, RS6060_MULTI_SET_TS_INDEX, stBsCbInfo.stFrontendAttrInfo.u32StreamId);
                    if(MtFeErr_Ok == enRet)
                    {
                        g_pBlindScanCallback(&stBsCbInfo);
                    }
                }
            }
            else
            {
                stBsCbInfo.bLocked = (MtFeMsg_BSTpLocked == msg) ? 1 : 0;

                g_pBlindScanCallback(&stBsCbInfo);
            }

        }
        else
        {
            //Do nothing
        }
    }

    return ;
}


_U32 RS6060_App_Init(_U8 u8TunerIndex)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _FAILURE;
    }

    enRet = mt_fe_dmd_rs6060_config_default(pstRs6060Handle);
    if(MtFeErr_Ok != enRet)
    {
        return _FAILURE;
    }

    pstRs6060Handle->dev_index = u8TunerIndex;

    enRet = mt_fe_dmd_rs6060_init(pstRs6060Handle);

    return enRet == MtFeErr_Ok ? _SUCCESS : _FAILURE;
}

_BOOL RS6060_App_Tune(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    enRet = RS6060_LockChannel(u8TunerIndex, pstLockInfo, bWaitLock);

    return enRet == MtFeErr_Ok ? _TRUE : _FALSE;
}

_BOOL RS6060_App_Stop_Tune(_U8 u8TunerIndex)
{
    return _TRUE;
}

void RS6060_App_ExitTune(_U8 u8TunerIndex)
{
    RS6060_SetExitLockTuner(u8TunerIndex, _TRUE);

    return ;
}

void RS6060_App_BlindScanStart(_U8 u8TunerIndex, _BlindScanCallback pCallback, PBLIND_SCAN_PARAM_INFO pScanParams)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    MT_FE_BS_TP_INFO stBsTpInfo = {0};

    /*alloc the memeory to store the scanned and locked TP info*/
	MT_FE_TP_INFO astTpInfo[1000] = {0};

    if(NULL == pScanParams)
    {
        return ;
    }

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return ;
    }

    g_BlindScanTunerIndex = u8TunerIndex;

    g_pBlindScanCallback = pCallback;

    g_BlindScanBeginFreqKHz = pScanParams->stBlindScanParam.minfrequency;
    g_BlindScanEndFreqKHz  = pScanParams->stBlindScanParam.maxfrequency;

    if(NULL != pCallback)
    {
        mt_fe_dmd_rs6060_register_notify(RS6060_BlindScanCallback);
    }

    g_bInBlindScanLoop = _TRUE;

    /*initialize the parameter*/
    stBsTpInfo.bs_algorithm = MT_FE_BS_ALGORITHM_A;
    stBsTpInfo.tp_num       = 0;
    stBsTpInfo.tp_max_num   = 1000;
    stBsTpInfo.p_tp_info    = astTpInfo;
    enRet = mt_fe_dmd_rs6060_blindscan(pstRs6060Handle, (U32)pScanParams->stBlindScanParam.minfrequency / 1000, (U32)pScanParams->stBlindScanParam.maxfrequency / 1000, &stBsTpInfo);

    if(_NULL != g_pBlindScanCallback)
    {
        g_pBlindScanCallback(_NULL);
    }

    g_bInBlindScanLoop = _FALSE;

    return ;
}

void RS6060_App_BlindScanStop(_U8 u8TunerIndex)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return ;
    }

    if(_TRUE == g_bInBlindScanLoop)
    {
        enRet = mt_fe_dmd_rs6060_blindscan_abort(pstRs6060Handle, MtFe_True);

        while(_TRUE == g_bInBlindScanLoop)
        {
            _mt_sleep(100);
        }
    }

    return ;
}

_BOOL RS6060_App_SetLNBPower(_U8 u8TunerIndex, _BOOL bPowerOn)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _FALSE;
    }

    if(pstRs6060Handle->lnb_cfg.bUnicable)
    {
        //Do nothing
    }
    else
    {
        pstRs6060Handle->lnb_cfg.bLnbOn = (BOOL)bPowerOn;

        enRet = mt_fe_dmd_rs6060_set_LNB(pstRs6060Handle, pstRs6060Handle->lnb_cfg.bLnbOn, pstRs6060Handle->lnb_cfg.b22K, pstRs6060Handle->lnb_cfg.mLnbVoltage, pstRs6060Handle->lnb_cfg.bEnvelopMode);
    }

    return enRet == MtFeErr_Ok ? _TRUE : _FALSE;
}

_BOOL RS6060_App_SetPolarization(_U8 u8TunerIndex, _BOOL bHigh)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _FALSE;
    }

    if(pstRs6060Handle->lnb_cfg.bUnicable)
    {
        if(_TRUE == bHigh)
        {
            pstRs6060Handle->lnb_cfg.iBankIndex |= 0x01;         // Horizontal    18V
        }
        else
        {
            pstRs6060Handle->lnb_cfg.iBankIndex &= ~0x01;        // Vertical      13V
        }
    }
    else
    {
        if(_TRUE == bHigh)
        {
            pstRs6060Handle->lnb_cfg.mLnbVoltage = MtFeLNB_18V;
        }
        else
        {
            pstRs6060Handle->lnb_cfg.mLnbVoltage = MtFeLNB_13V;
        }

        enRet = mt_fe_dmd_rs6060_set_LNB(pstRs6060Handle, pstRs6060Handle->lnb_cfg.bLnbOn, pstRs6060Handle->lnb_cfg.b22K, pstRs6060Handle->lnb_cfg.mLnbVoltage, pstRs6060Handle->lnb_cfg.bEnvelopMode);
    }

    return enRet == MtFeErr_Ok ? _TRUE : _FALSE;
}

_BOOL RS6060_App_SetDiseqcContinuous(_U8 u8TunerIndex, _BOOL bOn)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _FALSE;
    }

    if(pstRs6060Handle->lnb_cfg.bUnicable)
    {
        if(_TRUE == bOn)
        {
            pstRs6060Handle->lnb_cfg.iBankIndex |= 0x02;         // High Band    22K ON
        }
        else
        {
            pstRs6060Handle->lnb_cfg.iBankIndex &= ~0x02;        // Low Band     22K OFF
        }
    }
    else
    {
        pstRs6060Handle->lnb_cfg.b22K = (BOOL)bOn;

        enRet = mt_fe_dmd_rs6060_set_LNB(pstRs6060Handle, pstRs6060Handle->lnb_cfg.bLnbOn, pstRs6060Handle->lnb_cfg.b22K, pstRs6060Handle->lnb_cfg.mLnbVoltage, pstRs6060Handle->lnb_cfg.bEnvelopMode);
    }

    return enRet == MtFeErr_Ok ? _TRUE : _FALSE;
}

_BOOL RS6060_App_SendDiseqcCommand(_U8 u8TunerIndex, _U8 *pu8Data, _U8 u8Length)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    MT_FE_DiSEqC_MSG stDiseqcMsg = {0};

    if(NULL == pu8Data)
    {
        return _FALSE;
    }

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _FALSE;
    }

    memset(&stDiseqcMsg, 0, sizeof(MT_FE_DiSEqC_MSG));
    stDiseqcMsg.size_send = (u8Length > 8) ? 8 : u8Length;
    memcpy(stDiseqcMsg.data_send, pu8Data, stDiseqcMsg.size_send);
    /* only send data to device */
    stDiseqcMsg.is_enable_receive = FALSE;
    stDiseqcMsg.is_envelop_mode = FALSE;
    enRet = mt_fe_dmd_rs6060_DiSEqC_send_receive_msg(pstRs6060Handle, &stDiseqcMsg);

    return enRet == MtFeErr_Ok ? _TRUE : _FALSE;
}

_BOOL RS6060_App_SendDiseqcToneBurst(_U8 u8TunerIndex, _U8 u8Command)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    MT_FE_DiSEqC_TONE_BURST enToneBurst = MtFeDiSEqCToneBurst_Moulated;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _FALSE;
    }

    if(0 == u8Command)
    {
        enToneBurst = MtFeDiSEqCToneBurst_Moulated;
    }
    else
    {
        enToneBurst = MtFeDiSEqCToneBurst_Unmoulated;
    }

    enRet = mt_fe_dmd_rs6060_DiSEqC_send_tone_burst(pstRs6060Handle, enToneBurst, pstRs6060Handle->lnb_cfg.bEnvelopMode);

    return enRet == MtFeErr_Ok ? _TRUE : _FALSE;
}


_BOOL RS6060_App_CheckLocked(_U8 u8TunerIndex, _BOOL *pbLock)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    MT_FE_LOCK_STATE enLockState = MtFeLockState_Undef;

    if(NULL == pbLock)
    {
        return _FALSE;
    }

    *pbLock = _FALSE;
    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _FALSE;
    }

    enRet = mt_fe_dmd_rs6060_get_lock_state(pstRs6060Handle, &enLockState);
    if(MtFeErr_Ok == enRet)
    {
        if(MtFeLockState_Locked == enLockState)
        {
            *pbLock = _TRUE;

            return _TRUE;
        }
    }

    return _FALSE;
}

_BOOL RS6060_App_GetBer(_U8 u8TunerIndex, _U32 *pu32Ber)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    _U32 u32Total = 0, u32Err = 0;

    if(NULL == pu32Ber)
    {
        return _FALSE;
    }

    *pu32Ber = 0;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _FALSE;
    }

    enRet = mt_fe_dmd_rs6060_get_per(pstRs6060Handle, &u32Total, &u32Err);
    u32Err = u32Err * 1000 * 1000 * 1000;
    if(0 != u32Total)
    {
        *pu32Ber = u32Err / u32Total;
    }
    else
    {
        *pu32Ber = 0;
    }

    return _TRUE;
}

_BOOL RS6060_App_GetStrength(_U8 u8TunerIndex, _U16 *pu16Strength)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    MT_FE_LOCK_STATE enLockState = MtFeLockState_Undef;

    _S8 s8Strength = 0;

    if(NULL == pu16Strength)
    {
        return _FALSE;
    }

    *pu16Strength = 0;
    s8Strength = 0;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _TRUE;
    }

    enRet = mt_fe_dmd_rs6060_get_pure_lock(pstRs6060Handle, &enLockState);
    if((MtFeErr_Ok == enRet) && (MtFeLockState_Locked == enLockState))
    {
        enRet = mt_fe_dmd_rs6060_get_strength(pstRs6060Handle, &s8Strength);
        if(MtFeErr_Ok != enRet)
        {
            *pu16Strength = 0;
        }

        *pu16Strength = (100 < s8Strength) ? 100 : s8Strength;
    }

    return _TRUE;
}

_BOOL RS6060_App_GetSnr(_U8 u8TunerIndex, _U16 *pu16Snr)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    MT_FE_LOCK_STATE enLockState = MtFeLockState_Undef;

    _S8 s8SNR = 0;

    if(NULL == pu16Snr)
    {
        return _FALSE;
    }

    *pu16Snr = 0;
    s8SNR = 0;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return _TRUE;
    }

    enRet = mt_fe_dmd_rs6060_get_pure_lock(pstRs6060Handle, &enLockState);
    if((MtFeErr_Ok == enRet) && (MtFeLockState_Locked == enLockState))
    {
        enRet = mt_fe_dmd_rs6060_get_snr(pstRs6060Handle, &s8SNR);
        if(MtFeErr_Ok != enRet)
        {
            *pu16Snr = 0;
        }

        *pu16Snr = s8SNR;
    }

    return _TRUE;
}

void RS6060_App_StandBy(_U8 u8TunerIndex)
{
    MT_FE_RET enRet = MtFeErr_Ok;

    MT_FE_RS6060_Device_Handle pstRs6060Handle = NULL;

    pstRs6060Handle = RS6060_GetHandleByFeIndex(u8TunerIndex);
    if(NULL == pstRs6060Handle)
    {
        return ;
    }

    enRet = mt_fe_dmd_rs6060_sleep(pstRs6060Handle);

    return ;
}
