#include "avl68xx_app_porting.h"

#include "vs_fe_debug.h"

#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define _PRINTF_ _Print
#else
#define _PRINTF_(...)
#endif

static EXTERNAL_SEMAPHORE g_pSemphore68XXAccess[2];

_U32 AVL68XX_App_Init(_U8 u8TunerIndex, _U32 *pChipId)
{
    int return_code = AVL_EC_OK;
    AVL_int32 uiChipNo = u8TunerIndex ;
    _U32 systemType;

    if (!External_CreateSemaphore(&g_pSemphore68XXAccess[uiChipNo])) {
        _Print("Failed to create g_pSemphore68XXAccess!\n");
    }

    //I2C driver initialization
    //======================================================//
    // user defined functions should be customized according specific application platform.
    // Following implement is based on Availink I2CAgent.
    // Please refer to AVL6762 porting guide for user defined function porting.
    return_code = AVL_IBSP_Initialize(uiChipNo);
    if (AVL_EC_OK != return_code ) {
        _Print("Failed to initialize the BSP!\n");

        return 0;
    }
    //======================================================//

    // initialize demod and tuner
    //======================================================//
    systemType = AVL_Init(uiChipNo, pChipId);
    if (0 == systemType) {
        _Print("Failed to Init demod!\n");

        return 0;
    }
    //AVL_PrintVersion(uiChipNo);
    //======================================================//

    return systemType;
}

_BOOL AVL68XX_App_Tune_DVBT2(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock)
{
    AVL_uint16 times_out_cnt = 0;
    AVL_uchar ucLockStatus = 0;
    AVL_uchar ucNosignal = 0;
    AVL_uint32 uiChipNo = u8TunerIndex;

    AVL68XX_SetExitLockTuner(uiChipNo, _TRUE);

    External_AcquireSemaphore(g_pSemphore68XXAccess[uiChipNo]);

    AVL68XX_SetExitLockTuner(uiChipNo, _FALSE);

    //Set demod work mode to DVB-Tx
    AVL_SetWorkMode(uiChipNo, AVL_DVBTX);

    if ((_TRUE == pstLockInfo->stTunerParamInfo.bFrontendAttrInfoValid) && (FRONTEND_TYPE_TER == pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.enFrontendType)) {
        _PRINTF_ (" u8TunerIndex = %d   Tune to DVB-T signal with the following parameters:\n", u8TunerIndex);
        _PRINTF_ ("  - Center Freq    : %uKHz\n", pstLockInfo->stTunerParamInfo.u32Frequency);
        _PRINTF_ ("  - Bandwidth      : %uMHz\n", pstLockInfo->stTunerParamInfo.u32Bandwidth);
        _PRINTF_ ("  - Profile        : %u\n", pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32MutiAttr);

        AVL_LockChannel_T(uiChipNo, pstLockInfo->stTunerParamInfo.u32Frequency, pstLockInfo->stTunerParamInfo.u32Bandwidth * 1000, pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32MutiAttr);
    } else if ((_TRUE == pstLockInfo->stTunerParamInfo.bFrontendAttrInfoValid) && (FRONTEND_TYPE_DVBT2 == pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.enFrontendType)) {
        _PRINTF_ (" u8TunerIndex = %d   Tune to DVB-T2 signal with the following parameters:\n", u8TunerIndex);
        _PRINTF_ ("  - Center Freq    : %uKHz\n", pstLockInfo->stTunerParamInfo.u32Frequency);
        _PRINTF_ ("  - Bandwidth      : %uMHz\n", pstLockInfo->stTunerParamInfo.u32Bandwidth);
        _PRINTF_ ("  - PLP ID         : %u\n", pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32StreamId);
        _PRINTF_ ("  - Profile        : %u\n", pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32MutiAttr);

        AVL_LockChannel_T2(uiChipNo, pstLockInfo->stTunerParamInfo.u32Frequency, pstLockInfo->stTunerParamInfo.u32Bandwidth * 1000, pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32MutiAttr, pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32StreamId);
    } else {
        _PRINTF_ (" u8TunerIndex = %d   Scan with the following parameters: \n", u8TunerIndex);
        _PRINTF_ ("  - Frequency       : %u KHz\n", pstLockInfo->stTunerParamInfo.u32Frequency);
        _PRINTF_ ("  - Bandwidth       : %u MHz\n", pstLockInfo->stTunerParamInfo.u32Bandwidth);

        //DVBTx Channel Scan Functions
        AVL_ScanChannel_Tx(uiChipNo, pstLockInfo->stTunerParamInfo.u32Frequency, pstLockInfo->stTunerParamInfo.u32Bandwidth * 1000);

        _PRINTF_ ("  - Channels Found : %u\n\n", g_nChannel_ts_total);

        pstLockInfo->pstScanCallbackInfo->u32ChannelCount = g_nChannel_ts_total;
        if (g_nChannel_ts_total > 0) {
            _U8 i;
            if (AVL_DVBTx_Standard_T == global_channel_ts_table[0].channel_type) {
                if (_NULL != pstLockInfo->pstScanCallbackInfo) {
                    pstLockInfo->pstScanCallbackInfo->u32ChannelCount = g_nChannel_ts_total;
                    for (i = 0 ; i < g_nChannel_ts_total ; i++) {
                        pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[i].enFrontendType = FRONTEND_TYPE_TER;
                        pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[i].u32MutiAttr = global_channel_ts_table[i].dvbt_hierarchy_layer;
                    }
                }

                _PRINTF_ (" Tune to DVB-T signal with the following parameters:\n");
                _PRINTF_ ("  - Center Freq    : %uKHz\n", global_channel_ts_table[0].channel_freq_khz);
                _PRINTF_ ("  - Bandwidth      : %uMHz\n", global_channel_ts_table[0].channel_bandwith_khz / 1000);
                _PRINTF_ ("  - Profile        : %u\n", global_channel_ts_table[0].dvbt_hierarchy_layer);

                //first global_channel_ts_table[0].dvbt_hierarchy_layer is HP as value '1'
                AVL_LockChannel_T(uiChipNo, global_channel_ts_table[0].channel_freq_khz, global_channel_ts_table[0].channel_bandwith_khz, global_channel_ts_table[0].dvbt_hierarchy_layer);
            }
            else if (AVL_DVBTx_Standard_T2 == global_channel_ts_table[0].channel_type) {
                if (_NULL != pstLockInfo->pstScanCallbackInfo) {
                    pstLockInfo->pstScanCallbackInfo->u32ChannelCount = g_nChannel_ts_total;
                    for (i = 0 ; i < g_nChannel_ts_total ; i++) {
                        pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[i].enFrontendType = FRONTEND_TYPE_DVBT2;
                        pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[i].u32MutiAttr = global_channel_ts_table[i].channel_profile;
                        pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[i].u32StreamId = global_channel_ts_table[i].data_plp_id;
                    }
                }

                _PRINTF_ (" Tune to DVB-T2 signal with the following parameters:\n");
                _PRINTF_ ("  - Center Freq    : %uKHz\n", global_channel_ts_table[0].channel_freq_khz);
                _PRINTF_ ("  - Bandwidth      : %uMHz\n", global_channel_ts_table[0].channel_bandwith_khz / 1000);
                _PRINTF_ ("  - PLP ID         : %u\n", global_channel_ts_table[0].data_plp_id);
                _PRINTF_ ("  - Profile        : %u\n", global_channel_ts_table[0].channel_profile);

                AVL_LockChannel_T2(uiChipNo, global_channel_ts_table[0].channel_freq_khz, global_channel_ts_table[0].channel_bandwith_khz, global_channel_ts_table[0].channel_profile, global_channel_ts_table[0].data_plp_id);
            }
        }
    }

    //this time-out window can be customized, 200*15 = 3000ms for this time
    while (bWaitLock && times_out_cnt < 100 && !AVL68XX_GetExitLockTuner(uiChipNo)) {
        AVL_IBSP_Delay(15);

        AVL_Demod_DVBTxSignalDetection(&ucNosignal, uiChipNo);
        //ucNosignal=1:signal exist 0:nosignal
        if (0 == ucNosignal) {
            break;//no signal, no need to wait any more
        }

        //get the lock status
        AVL_Demod_GetLockStatus(&ucLockStatus, uiChipNo);
        if (1 == ucLockStatus) {
            break;//locked, no need to wait any more
        }

        times_out_cnt++;
    }

    External_ReleaseSemaphore(g_pSemphore68XXAccess[uiChipNo]);

    return  _TRUE;
}

_BOOL AVL68XX_App_Tune_DVBC(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock)
{
    AVL_DVBC_Standard enStandard = AVL_DVBC_UNKNOWN;
    AVL_uint16 times_out_cnt = 0;
    AVL_uchar ucLockStatus = 0;
    AVL_uint32 uiChipNo = u8TunerIndex;

    AVL68XX_SetExitLockTuner(uiChipNo, _TRUE);

    External_AcquireSemaphore(g_pSemphore68XXAccess[uiChipNo]);

    AVL68XX_SetExitLockTuner(uiChipNo, _FALSE);

    //Set demod work mode to DVB-C
    AVL_SetWorkMode(uiChipNo, AVL_DVBC);
    if ((_TRUE == pstLockInfo->stTunerParamInfo.bFrontendAttrInfoValid) && (FRONTEND_TYPE_TER == pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.enFrontendType)) {
        enStandard = (0 == pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32MutiAttr) ? AVL_DVBC_J83A : AVL_DVBC_J83B;
    } else {
        enStandard = AVL_DVBC_J83A;
    }

    AVL_Demod_Set_DVBCStandard(enStandard, uiChipNo);

    _PRINTF_ (" u8TunerIndex = %d   Tune to DVB-C signal with the following parameters:\n",u8TunerIndex);
    _PRINTF_ ("  - Center Freq    : %uKHz\n", pstLockInfo->stTunerParamInfo.u32Frequency);
    _PRINTF_ ("  - Symbolrate     : %ukSps\n", pstLockInfo->stTunerParamInfo.u32Symbolrate);
    _PRINTF_ ("  - DVBC Standard  : %s\n", pstLockInfo->stTunerParamInfo.stFrontendAttrInfo.u32MutiAttr ? "J83B" : "J83A");

    //use bandwidth to store symbolrate
    AVL_LockChannel_DVBC(uiChipNo, pstLockInfo->stTunerParamInfo.u32Frequency, pstLockInfo->stTunerParamInfo.u32Symbolrate);

    if (_NULL != pstLockInfo->pstScanCallbackInfo) {
        pstLockInfo->pstScanCallbackInfo->u32ChannelCount = 1;
        pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[0].enFrontendType = FRONTEND_TYPE_TER;
    }
	
    //this time-out window can be customized, times_out_cnt (100) about 2500ms. 200 about 4.9s
    while (bWaitLock && times_out_cnt < 100 && !AVL68XX_GetExitLockTuner(uiChipNo)) {
        AVL_IBSP_Delay(15);
		
        //get the lock status
        AVL_Demod_GetLockStatus(&ucLockStatus, uiChipNo);
        if (1 == ucLockStatus) {
            break;//locked, no need to wait any more
        }

        times_out_cnt++;
    }
    _PRINTF_ (" done \n");
    External_ReleaseSemaphore(g_pSemphore68XXAccess[uiChipNo]);

    return  _TRUE;
}

_BOOL AVL68XX_App_Tune_ISDBT(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock)
{
    AVL_uint16 times_out_cnt = 0;
    AVL_uchar ucLockStatus = 0;
    AVL_uint32 uiChipNo = u8TunerIndex;

    AVL68XX_SetExitLockTuner(uiChipNo, _TRUE);

    External_AcquireSemaphore(g_pSemphore68XXAccess[uiChipNo]);

    AVL68XX_SetExitLockTuner(uiChipNo, _FALSE);

    //Set demod work mode to ISDBT
    AVL_SetWorkMode(uiChipNo, AVL_ISDBT);

    _PRINTF_ (" u8TunerIndex = %d   Tune to ISDB-T signal with the following parameters:\n",u8TunerIndex);
    _PRINTF_ ("  - Center Freq    : %uKHz\n", pstLockInfo->stTunerParamInfo.u32Frequency);
    _PRINTF_ ("  - Bandwidth      : %uMHz\n", pstLockInfo->stTunerParamInfo.u32Bandwidth);

    if (_NULL != pstLockInfo->pstScanCallbackInfo) {
        pstLockInfo->pstScanCallbackInfo->u32ChannelCount = 1;
        pstLockInfo->pstScanCallbackInfo->astFrontendAttrInfo[0].enFrontendType = FRONTEND_TYPE_TER;
    }

    AVL_LockChannel_ISDBT(uiChipNo, pstLockInfo->stTunerParamInfo.u32Frequency, pstLockInfo->stTunerParamInfo.u32Bandwidth * 1000);

    //this time-out window can be customized, 200*15 = 300ms for this time
    while (bWaitLock && times_out_cnt < 100 && !AVL68XX_GetExitLockTuner(uiChipNo)) {
        AVL_IBSP_Delay(15);

        //get the lock status
        AVL_Demod_GetLockStatus(&ucLockStatus, uiChipNo);
        if (1 == ucLockStatus) {
            break;//locked, no need to wait any more
        }

        times_out_cnt++;
    }

    External_ReleaseSemaphore(g_pSemphore68XXAccess[uiChipNo]);

    return  _TRUE;
}

_BOOL AVL68XX_App_Tune_DVBS2(_U8 u8TunerIndex, PLOCK_PARAM_INFO_T pstLockInfo, _BOOL bWaitLock)
{
    AVL_uint16 times_out_cnt = 0;
    AVL_uchar ucLockStatus = 0;
    AVL_uint32 uiChipNo = u8TunerIndex;

    AVL68XX_SetExitLockTuner(uiChipNo, _TRUE);

    External_AcquireSemaphore(g_pSemphore68XXAccess[uiChipNo]);

    AVL68XX_SetExitLockTuner(uiChipNo, _FALSE);

   //Set demod work mode to DVB-SX
    AVL_SetWorkMode(uiChipNo, AVL_DVBSX);

    _PRINTF_ (" u8TunerIndex = %d   Tune to DVB-S/S2 signal with the following parameters:\n",u8TunerIndex);
    _PRINTF_ ("  - Center Freq    : %uMHz\n", pstLockInfo->stTunerParamInfo.u32Frequency);
    _PRINTF_ ("  - SymbbolRate    : %uKHz\n", pstLockInfo->stTunerParamInfo.u32Symbolrate);

    AVL_LockChannel_DVBSx(uiChipNo, pstLockInfo->stTunerParamInfo.u32Frequency * 1000, pstLockInfo->stTunerParamInfo.u32Symbolrate);

    //this time-out window can be customized, 200*15 = 300ms for this time
    while (bWaitLock && times_out_cnt < 100 && !AVL68XX_GetExitLockTuner(uiChipNo)) {
        AVL_IBSP_Delay(15);

        //get the lock status
        AVL_Demod_GetLockStatus(&ucLockStatus, uiChipNo);
        if (1 == ucLockStatus) {
            break;//locked, no need to wait any more
        }

        times_out_cnt++;
    }

    External_ReleaseSemaphore(g_pSemphore68XXAccess[uiChipNo]);

    return  _TRUE;
}

_BOOL AVL68XX_App_Stop_Tune(_U8 u8TunerIndex)
{
    return _TRUE;
}

void AVL68XX_App_ExitTune(_U8 u8TunerIndex)
{
    AVL68XX_SetExitLockTuner(u8TunerIndex, _TRUE);
}

void AVL68XX_App_BlindScanStart(_U8 u8TunerIndex, _BlindScanCallback pCallback, PBLIND_SCAN_PARAM_INFO pScanParams)
{
    AVL68XX_BlindScanStartLoop(u8TunerIndex, pCallback, pScanParams);
}

void AVL68XX_App_BlindScanStop(_U8 u8TunerIndex)
{
    AVL68XX_BlindScanStopLoop(u8TunerIndex);
}

void AVL68XX_App_SetAntennaPower(_U8 u8TunerIndex, _BOOL bPowerOn)
{
    VS_FE_SetAntPower(u8TunerIndex, bPowerOn ? 1 : 0);

    return ;
}

_BOOL AVL68XX_App_SetLNBPower(_U8 u8TunerIndex, _BOOL bPowerOn)
{
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_uint32 uiChipNo = u8TunerIndex;

    if (bPowerOn) {
        _Print("\n---------->AVL6862_Set_LNBPower : on\n");
        r = AVL_Demod_SetGPIO(AVL_Pin37,AVL_LOGIC_1,uiChipNo);
    } else {
        _Print("\n---------->AVL6862_Set_LNBPower : off\n");
        r = AVL_Demod_SetGPIO(AVL_Pin37,AVL_LOGIC_0,uiChipNo);
    }

    return r == AVL_EC_OK ? _TRUE : _FALSE;
}

_BOOL AVL68XX_App_SetPolarization(_U8 u8TunerIndex, _BOOL bHigh)
{
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_uint32 uiChipNo = u8TunerIndex;

    if (bHigh)//Horizontal
    {
        _Print("\n---------->AVL6862_SetPolarization : 18V\n");
        r = AVL_Demod_SetGPIO(AVL_Pin38,AVL_LOGIC_1,uiChipNo);
    }
    else  //Vertical
    {
        _Print("\n---------->AVL6862_SetPolarization : 13V\n");
        r = AVL_Demod_SetGPIO(AVL_Pin38,AVL_LOGIC_0,uiChipNo);
    }

    return r== AVL_EC_OK ? _TRUE : _FALSE;
}

_BOOL AVL68XX_App_SetDiseqcContinuous(_U8 u8TunerIndex, _BOOL bOn)
{
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_uint32 uiChipNo = u8TunerIndex;

    //Set demod work mode to DVB-SX
    AVL_SetWorkMode(uiChipNo, AVL_DVBSX);

    if (bOn) {
        r = AVL_Demod_DVBSx_Diseqc_StartContinuous(uiChipNo);
    } else {
        r = AVL_Demod_DVBSx_Diseqc_StopContinuous(uiChipNo);
    }

    if (AVL_EC_OK != r)
    {
        _Print("[AVL_SX_22K_Control] Err:0x%x\n",r);
    }

    return r == AVL_EC_OK ? _TRUE : _FALSE;
}

_BOOL AVL68XX_App_SendDiseqcCommand(_U8 u8TunerIndex, _U8 *pu8Data, _U8 u8Length)
{
    AVL_ErrorCode r = AVL_EC_OK;
    struct AVL_Diseqc_TxStatus TxStatus;
    AVL_uint32 uiChipNo = u8TunerIndex;

    //Set demod work mode to DVB-SX
    AVL_SetWorkMode(uiChipNo, AVL_DVBSX);

    r = AVL_Demod_DVBSx_Diseqc_SendModulationData(pu8Data, u8Length, uiChipNo);
    if (AVL_EC_OK != r) {
        _Print("AVL_SX_DiseqcSendCmd failed !\n");
    } else {
        do {
            AVL_IBSP_Delay(5);
            r |= AVL_Demod_DVBSx_Diseqc_GetTxStatus(&TxStatus, uiChipNo);
        } while (TxStatus.m_TxDone != 1);

        if (AVL_EC_OK != r) {
            _Print("AVL_SX_DiseqcSendCmd Err. !\n");
        }
    }

    return r == AVL_EC_OK ? _TRUE : _FALSE;
}

_BOOL AVL68XX_App_SendDiseqcToneBurst(_U8 u8TunerIndex, _U8 u8Command)
{
    AVL_ErrorCode r = AVL_EC_OK;
    struct AVL_Diseqc_TxStatus TxStatus;
    AVL_uint32 uiChipNo = u8TunerIndex;

    //follow api_fend
    if (u8Command == 0) //SEC_MINI_A
       u8Command = 1;
    else if (u8Command == 1)//SEC_MINI_B
       u8Command = 0;
    else
        return _FALSE;

    //Set demod work mode to DVB-SX
    AVL_SetWorkMode(uiChipNo, AVL_DVBSX);

    r = AVL_Demod_DVBSx_Diseqc_SendTone(u8Command, 1, uiChipNo);
    if (AVL_EC_OK != r) {
        _Print("AVL_Demod_DVBSx_Diseqc_SendTone failed !\n");
    } else {
        do {
            AVL_IBSP_Delay(5);
            r |= AVL_Demod_DVBSx_Diseqc_GetTxStatus(&TxStatus, uiChipNo);
        } while (TxStatus.m_TxDone != 1);

        if (AVL_EC_OK != r) {
            _Print("AVL_Demod_DVBSx_Diseqc_SendTone Err. !\n");
        }
    }

    return r == AVL_EC_OK ? _TRUE : _FALSE;
}

_BOOL AVL68XX_App_CheckLocked(_U8 u8TunerIndex, _BOOL *pbLock)
{
    _BOOL bRet = _TRUE;

    AVL_ErrorCode r = AVL_EC_OK;
    AVL_uchar ucLockStatus = 0;
    AVL_uint32 uiChipNo = u8TunerIndex;

    *pbLock = _FALSE;
    r = AVL_Demod_GetLockStatus(&ucLockStatus, uiChipNo);
    if (AVL_EC_OK == r) {
        if (1 == ucLockStatus) {
            *pbLock = _TRUE;
        }
    }
    else if ((r & AVL_EC_I2C_FAIL) == AVL_EC_I2C_FAIL) {
        return _FALSE;
    }

    return bRet;
}

_BOOL AVL68XX_App_GetBer(_U8 u8TunerIndex, _U32 *pu32Ber)
{
    _BOOL bRet = _TRUE;

    AVL_uint32 uiChipNo = u8TunerIndex;

    *pu32Ber = get_PER_info(uiChipNo);

    return bRet;
}

_BOOL AVL68XX_App_GetStrength(_U8 u8TunerIndex, _U16 *pu16Strength)
{
    _BOOL bRet = _TRUE;

    AVL_uint32 uiChipNo = u8TunerIndex;

    *pu16Strength = get_SSI_info(uiChipNo);

    return bRet;
}

_BOOL AVL68XX_App_GetSnr(_U8 u8TunerIndex, _U16 *pu16Snr)
{
    _BOOL bRet = _TRUE;

    AVL_uint32 uiChipNo = u8TunerIndex;

    *pu16Snr = get_SQI_info(uiChipNo);

    return bRet;
}

void AVL68XX_App_StandBy(_U8 u8TunerIndex)
{
    return ;
}
