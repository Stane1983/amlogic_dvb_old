#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <generated/utsrelease.h>
#include <generated/compile.h>

#include <linux/amlogic/aml_gpio_consumer.h>

#include "vs_fe_external.h"
#include "vs_fe_tuner.h"

//#define VS_FE_DEBUG_LEVEL DEBUG_LEVEL_INFO

#undef DEBUG_LEVEL
#undef DEBUG_TAG

#ifdef VS_FE_DEBUG_LEVEL
#define DEBUG_LEVEL VS_FE_DEBUG_LEVEL
#endif

#include "vs_fe_debug.h"

#define COMMON_NAME "vs_frontend"

#define CHECK_INIT(pstCtrl) \
do \
{ \
    if ((_NULL == pstCtrl) || (!pstCtrl->bInitFlag)) { \
        return _FAILURE; \
    } \
}while(0)

#define CHECK_SUSPEND(pstCtrl) \
do \
{ \
    if ((_NULL != pstCtrl) && (pstCtrl->bSuspendFlag)) { \
        return _SUCCESS; \
    } \
}while(0)

#define CHECK_POINTER(pPointer) \
do \
{ \
    if (_NULL == pPointer) { \
        return _FAILURE; \
    } \
}while(0)

#define CHECK_RANGE(s32Value, s32Min, s32Max) \
do \
{ \
    if (!((s32Min <= s32Value) && (s32Value <= s32Max))) { \
        return _FAILURE; \
    } \
}while(0)

#define CHECK_DELIVERY_SYSTEM(enDeliverySystem) \
do \
{ \
    if ((SYS_UNDEFINED == enDeliverySystem) || (SYS_ANALOG < enDeliverySystem)) { \
        return _FAILURE; \
    } \
}while(0)

#define GET_SYSTEM_TYPE(pstCtrl, enSystemType) ((_NULL != pstCtrl) ? pstCtrl->stSystemInfo.bSystemTypeDetect[pstCtrl->u8TunerIndex][enSystemType] : _FALSE)

#define GET_SYSTEM(pstCtrl) ((_NULL != pstCtrl) ? pstCtrl->stSystemInfo.bSystemDetect[pstCtrl->u8TunerIndex] : _FALSE)

PVS_FE_CTRL g_apstCtrl[SYSTEM_TUNER_NUMBER] = {_NULL, _NULL};
static SCAN_CALLBACK_INFO_T g_stScanCallbackInfo = {0};
static struct dtv_frontend_properties g_stDvb_fe_prop_bak;	// reudce tune one freq repeatly.

static int _get_fe_ops(PVS_FE_CTRL pstCtrl, enum fe_delivery_system enDeliverySystem);

void _dvbsx_blind_scan_callback(PBLIND_SCAN_CALLBACK_INFO pstBlindScanCallbackInfo)
{
    enum fe_status signal_status = FE_HAS_LOCK;
    enum fe_status scan_status = FE_HAS_LOCK;

    if ((_NULL == pstBlindScanCallbackInfo) || (SYSTEM_TUNER_NUMBER < pstBlindScanCallbackInfo->u8TunerIndex)) {
        return ;
    }

    DBG_VERBOSE("frequency: %d, symbolRate: %d, lock: %d, process: %d", \
        pstBlindScanCallbackInfo->u32Frequency, pstBlindScanCallbackInfo->u32Symbolrate, pstBlindScanCallbackInfo->bLocked, pstBlindScanCallbackInfo->u8Process);

    if (0 == pstBlindScanCallbackInfo->u32Frequency) {
        //return ; //TODO: Whether filter
    }

    if (100 < pstBlindScanCallbackInfo->u8Process) {
        pstBlindScanCallbackInfo->u8Process = 100;
    }

    if (!pstBlindScanCallbackInfo->bLocked) {
        //signal_status = FE_TIMEDOUT;
    }

    if (0 == pstBlindScanCallbackInfo->u8Process) {
        scan_status = BLINDSCAN_UPDATESTARTFREQ | signal_status;
        g_apstCtrl[pstBlindScanCallbackInfo->u8TunerIndex]->stTunerParamInfo.u32Frequency = pstBlindScanCallbackInfo->u32Frequency;
    } else if((100 == pstBlindScanCallbackInfo->u8Process) || (!pstBlindScanCallbackInfo->bLocked)) {
        scan_status = BLINDSCAN_UPDATEPROCESS | signal_status;
        g_apstCtrl[pstBlindScanCallbackInfo->u8TunerIndex]->stTunerParamInfo.u32Frequency = pstBlindScanCallbackInfo->u8Process;
    } else {
        scan_status = BLINDSCAN_UPDATERESULTFREQ | signal_status;
        g_apstCtrl[pstBlindScanCallbackInfo->u8TunerIndex]->stTunerParamInfo.u32Frequency = pstBlindScanCallbackInfo->u32Frequency;
        g_apstCtrl[pstBlindScanCallbackInfo->u8TunerIndex]->stTunerParamInfo.u32Symbolrate = pstBlindScanCallbackInfo->u32Symbolrate;
    }
    DBG_VERBOSE("scan_status: %d, frequency or process: %d", \
        scan_status, g_apstCtrl[pstBlindScanCallbackInfo->u8TunerIndex]->stTunerParamInfo.u32Frequency);
    dvb_frontend_add_event(&(g_apstCtrl[pstBlindScanCallbackInfo->u8TunerIndex]->stDvbFrontend), scan_status);

    if (100 == pstBlindScanCallbackInfo->u8Process) {
        g_apstCtrl[pstBlindScanCallbackInfo->u8TunerIndex]->stBlindScanParamInfo.bBlindScan = _FALSE;
    }

    return ;
}

static int _dvbsx_blind_scan_task(void *pCtrl)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = (PVS_FE_CTRL)pCtrl;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    bRet = VS_FE_Tuner_Sat_BlindScan_Start(pstCtrl, _dvbsx_blind_scan_callback, &(pstCtrl->stBlindScanParamInfo));
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

static int _dvbsx_blind_scan_start(PVS_FE_CTRL pstCtrl)
{
    _S32 s32Ret = _SUCCESS;

    struct task_struct *dvbsx_blind_scan_task = _NULL;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    pstCtrl->stBlindScanParamInfo.bBlindScan = _TRUE;
    dvbsx_blind_scan_task = kthread_create(_dvbsx_blind_scan_task, (void *)pstCtrl, "dvbsx_blind_scan_task");
    if (!dvbsx_blind_scan_task) {
        dvbsx_blind_scan_task = _NULL;
        pstCtrl->stBlindScanParamInfo.bBlindScan = _FALSE;

        return _FAILURE;
    }

    wake_up_process(dvbsx_blind_scan_task);

    return s32Ret;
}

static int _dvbsx_blind_scan_stop(PVS_FE_CTRL pstCtrl)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    pstCtrl->stBlindScanParamInfo.bBlindScan = _FALSE;
    bRet = VS_FE_Tuner_Sat_BlindScan_Stop(pstCtrl);
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

static _S32 _get_system_type(PVS_FE_CTRL pstCtrl, enum fe_delivery_system enDeliverySystem, PSYSTEM_TYPE_E penSystemType)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    CHECK_POINTER(pstCtrl);
    CHECK_DELIVERY_SYSTEM(enDeliverySystem);

    CHECK_INIT(pstCtrl);

    bRet |= VS_FE_Tuner_GetSystemInfo(enDeliverySystem, penSystemType);
    if ((!bRet) || (SYSTEM_TYPE_INVALID == (SYSTEM_TYPE_E)*penSystemType)) {
        return _FAILURE;
    }

    if (!GET_SYSTEM_TYPE(pstCtrl, *penSystemType)) {
        return _FAILURE;
    }

    return s32Ret;
}

static _S32 _set_dvbt2_plp_id(struct dvb_frontend *dvb_fe, struct dtv_property *tvp)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(tvp);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    CHECK_RANGE(tvp->u.data, 0, 255);

    if ((SYS_DVBT2 == pstCtrl->enDeliverySystem) \
        && ((_TRUE == pstCtrl->stTunerParamInfo.bFrontendAttrInfoValid) && (pstCtrl->stTunerParamInfo.stFrontendAttrInfo.u32StreamId == tvp->u.data))) {
        return 0;
    }

    pstCtrl->enDeliverySystem = SYS_DVBT2;
    pstCtrl->stTunerParamInfo.bFrontendAttrInfoValid =_TRUE;
    pstCtrl->stTunerParamInfo.stFrontendAttrInfo.u32StreamId = tvp->u.data;

    bRet = VS_FE_Tuner_StartLock(pstCtrl, &g_stScanCallbackInfo, _FALSE); //TODO:
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

static _S32 _set_frontend(struct dvb_frontend *dvb_fe)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    struct dtv_frontend_properties *properties_cache = _NULL;

    _BOOL bLocked = _TRUE;

    CHECK_POINTER(dvb_fe);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;
    properties_cache = &dvb_fe->dtv_property_cache;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    CHECK_POINTER(properties_cache);

    if (pstCtrl->stTunerParamInfo.u32Frequency != properties_cache->frequency / 1000) {
        pstCtrl->stTunerParamInfo.bFrontendAttrInfoValid = _FALSE;
    }

    pstCtrl->stTunerParamInfo.u32Frequency = properties_cache->frequency / 1000;
    pstCtrl->stTunerParamInfo.u32Symbolrate = properties_cache->symbol_rate / 1000;
    pstCtrl->stTunerParamInfo.u32Bandwidth = properties_cache->bandwidth_hz / 1000000;

    bRet = VS_FE_Tuner_StartLock(pstCtrl, &g_stScanCallbackInfo, _TRUE);
    if (!bRet) {
        return _FAILURE;
    }

    bRet = VS_FE_Tuner_CheckLocked(pstCtrl, &bLocked);
    if (!bRet) {
        return _FAILURE;
    }

    if (!bLocked) {
        return _FAILURE;
    }

    return s32Ret;
}

static int _read_status(struct dvb_frontend *dvb_fe, enum fe_status* status)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    _BOOL bLocked = _TRUE;

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(status);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    *status = FE_TIMEDOUT;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);
    CHECK_SUSPEND(pstCtrl);

    bRet = VS_FE_Tuner_CheckLocked(pstCtrl, &bLocked);
    if (!bRet) {
        return _FAILURE;
    }

    if (bLocked) {
        *status = FE_HAS_LOCK | FE_HAS_SIGNAL | FE_HAS_CARRIER | FE_HAS_VITERBI | FE_HAS_SYNC;
    }

    return s32Ret;
}

static _S32 _set_delivery_system(PVS_FE_CTRL pstCtrl, enum fe_delivery_system enDeliverySystem)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    CHECK_POINTER(pstCtrl);
    CHECK_DELIVERY_SYSTEM(enDeliverySystem);

    bRet = VS_FE_Tuner_Init(pstCtrl);
    if ((!bRet) || (!GET_SYSTEM(pstCtrl))) {
        return _FAILURE;
    }

    if (enDeliverySystem == pstCtrl->enDeliverySystem) {
        return 0;
    }

    s32Ret = _get_fe_ops(pstCtrl, enDeliverySystem);
    if (_SUCCESS != s32Ret) {
        return _FAILURE;
    }

    memset(&pstCtrl->stTunerParamInfo, 0, sizeof(pstCtrl->stTunerParamInfo));
    pstCtrl->enDeliverySystem = enDeliverySystem;
    pstCtrl->stTunerParamInfo.bFrontendAttrInfoValid = _FALSE;

    return s32Ret;
}

int vs_fe_tune(struct dvb_frontend *dvb_fe,
        bool re_tune,
        unsigned int mode_flags,
        unsigned int *delay,
        enum fe_status *status)
{
    _S32 s32Ret = _SUCCESS;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(delay);
    CHECK_POINTER(status);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    if (pstCtrl->stBlindScanParamInfo.bBlindScan) {
        return s32Ret;
    }

	if (memcmp(&g_stDvb_fe_prop_bak, &(dvb_fe->dtv_property_cache), sizeof(struct dtv_frontend_properties)) == 0)
	{
		re_tune = false;
	}

    if (re_tune) {
        s32Ret = _set_frontend(dvb_fe);
        if (_SUCCESS == s32Ret) {
            *status = FE_HAS_LOCK | FE_HAS_SIGNAL | FE_HAS_CARRIER
                | FE_HAS_VITERBI | FE_HAS_SYNC;
        } else {
            *status = FE_TIMEDOUT;
        }

        *delay = 3 * HZ;

		memcpy(&g_stDvb_fe_prop_bak, &(dvb_fe->dtv_property_cache), sizeof(struct dtv_frontend_properties));
    } else {
        s32Ret = _read_status(dvb_fe, status);
        *delay = HZ / 3;
    }

    return s32Ret;
}

enum dvbfe_algo vs_fe_get_frontend_algo(struct dvb_frontend *dvb_fe)
{
    DBG_INFO("");

    return DVBFE_ALGO_HW;
}

int vs_fe_set_frontend(struct dvb_frontend *dvb_fe)
{
    _S32 s32Ret = _SUCCESS;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);

    s32Ret = _set_frontend(dvb_fe);

    return s32Ret;
}

int vs_fe_get_tune_settings(struct dvb_frontend *dvb_fe, struct dvb_frontend_tune_settings* settings)
{
    _S32 s32Ret = _SUCCESS;

    PVS_FE_CTRL pstCtrl = _NULL;

    struct dtv_frontend_properties *properties_cache = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(settings);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;
    properties_cache = &dvb_fe->dtv_property_cache;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    CHECK_POINTER(properties_cache);

    if ((SYS_DVBS == properties_cache->delivery_system) || (SYS_DVBS2 == properties_cache->delivery_system)) {
        settings->min_delay_ms = 50;
        settings->step_size = 1000;
        settings->max_drift = 4000;
    } else {
        settings->min_delay_ms = 300;
        settings->step_size = 0;
        settings->max_drift = 0;
    }

    return s32Ret;
}


int vs_fe_get_frontend(struct dvb_frontend *dvb_fe,
        struct dtv_frontend_properties *props)
{
    _S32 s32Ret = _SUCCESS;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(props);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    switch (pstCtrl->enDeliverySystem) {
        case SYS_DVBS:
        case SYS_DVBS2:
            props->frequency = pstCtrl->stTunerParamInfo.u32Frequency;
            props->symbol_rate = pstCtrl->stTunerParamInfo.u32Symbolrate * 1000;
            props->delivery_system = pstCtrl->enDeliverySystem;
            break;
        default:
            break;
    }

    return s32Ret;
}

int vs_fe_read_status(struct dvb_frontend *dvb_fe, enum fe_status *status)
{
    _S32 s32Ret = _SUCCESS;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(status);

    s32Ret = _read_status(dvb_fe, status);

    return s32Ret;
}

int vs_fe_read_ber(struct dvb_frontend *dvb_fe, u32 *ber)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(ber);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    bRet = VS_FE_Tuner_GetBer(pstCtrl, ber);
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

int vs_fe_read_signal_strength(struct dvb_frontend *dvb_fe, u16 *strength)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(strength);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    bRet = VS_FE_Tuner_GetStrength(pstCtrl, strength);
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

int vs_fe_read_snr(struct dvb_frontend *dvb_fe, u16* snr)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(snr);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    bRet = VS_FE_Tuner_GetSnr(pstCtrl, snr);
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

int vs_fe_read_ucblocks(struct dvb_frontend *dvb_fe, u32 *ucblocks)
{
    _S32 s32Ret = _SUCCESS;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(ucblocks);

    *ucblocks = 0;

    return s32Ret;
}

int vs_fe_diseqc_reset_overload(struct dvb_frontend *dvb_fe)
{
    _S32 s32Ret = _SUCCESS;

    DBG_INFO("");

    return s32Ret;
}

int vs_fe_diseqc_send_master_cmd(struct dvb_frontend *dvb_fe, struct dvb_diseqc_master_cmd *cmd)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    _U8 au8Data[6] = {0};
    _U8 u8Length = 0;

    _U8 u8Index = 0;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(cmd);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    u8Length = cmd->msg_len;
    for (u8Index = 0; u8Index < u8Length; u8Index++) {
        au8Data[u8Index] = cmd->msg[u8Index];
    }

    bRet = VS_FE_Tuner_Sat_SendDiseqcCommand(pstCtrl, au8Data, u8Length);
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

int vs_fe_diseqc_recv_slave_reply(struct dvb_frontend *dvb_fe, struct dvb_diseqc_slave_reply* reply)
{
    DBG_INFO("");

    return 0;
}

int vs_fe_diseqc_send_burst(struct dvb_frontend *dvb_fe,
        enum fe_sec_mini_cmd minicmd)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    bRet = VS_FE_Tuner_Sat_SendDiseqcBurst(pstCtrl, minicmd);
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

int vs_fe_set_tone(struct dvb_frontend *dvb_fe, enum fe_sec_tone_mode tone)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    bRet = VS_FE_Tuner_Sat_SetDiseqcContinuous(pstCtrl, (SEC_TONE_ON == tone) ? _TRUE : _FALSE);
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

int vs_fe_set_voltage(struct dvb_frontend *dvb_fe,
        enum fe_sec_voltage voltage)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    bRet = VS_FE_Tuner_SetVoltage(pstCtrl, voltage);
    if (!bRet) {
        return _FAILURE;
    }

    return s32Ret;
}

int vs_fe_enable_high_lnb_voltage(struct dvb_frontend *dvb_fe, long arg)
{
    DBG_INFO("");

    return 0;
}

int vs_fe_set_property(struct dvb_frontend *dvb_fe, struct dtv_property* tvp)
{
    _S32 s32Ret = _SUCCESS;

    PVS_FE_CTRL pstCtrl = _NULL;

    _S32 s32Value = 0;

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(tvp);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);
    CHECK_SUSPEND(pstCtrl);

    if ((!pstCtrl->bInitFlag) && (DTV_DELIVERY_SYSTEM != tvp->cmd)) {
        return _FAILURE;
    }

    s32Value = tvp->u.data;

    DBG_INFO("cmd: %d, data: %d", tvp->cmd, tvp->u.data);

    switch (tvp->cmd) {
        case DTV_DELIVERY_SYSTEM:
            s32Ret = _set_delivery_system(pstCtrl, s32Value);
            break;
        case DTV_DVBT2_PLP_ID:
            s32Ret = _set_dvbt2_plp_id(dvb_fe, tvp);
            break;
        case DTV_BLIND_SCAN_MIN_FRE:
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.minfrequency = ((850000 > s32Value) || (s32Value > 2300000)) ? 950000 : s32Value;
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.minfrequency -= 10000;
            break;
        case DTV_BLIND_SCAN_MAX_FRE:
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.maxfrequency = ((850000 > s32Value) || (s32Value > 2300000)) ? 2150000 : s32Value;
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.maxfrequency += 10000;
            break;
        case DTV_BLIND_SCAN_MIN_SRATE:
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.minSymbolRate = s32Value / 1000;
            break;
        case DTV_BLIND_SCAN_MAX_SRATE:
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.maxSymbolRate = s32Value / 1000;
            break;
        case DTV_BLIND_SCAN_FRE_RANGE:
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.frequencyRange = s32Value;
            break;
        case DTV_BLIND_SCAN_FRE_STEP:
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.frequencyStep = s32Value;
            break;
        case DTV_BLIND_SCAN_TIMEOUT:
            pstCtrl->stBlindScanParamInfo.stBlindScanParam.timeout = s32Value;
            break;
        case DTV_START_BLIND_SCAN:
            _dvbsx_blind_scan_start(pstCtrl);
            break;
        case DTV_CANCEL_BLIND_SCAN:
            _dvbsx_blind_scan_stop(pstCtrl);
            break;
        default:
            break;
    }

    return s32Ret;
}

static int vs_fe_get_property(struct dvb_frontend *dvb_fe, struct dtv_property* tvp)
{
    _S32 s32Ret = _SUCCESS;

    PVS_FE_CTRL pstCtrl = _NULL;

    _U32 u32Index = 0;
    _U32 u32PlpNum = 0;
    _U8 au8PlpId[FRONTEND_ATTR_INFO_NUMBER] = {0};

    DBG_INFO("");

    CHECK_POINTER(dvb_fe);
    CHECK_POINTER(tvp);

    pstCtrl = (PVS_FE_CTRL)dvb_fe->demodulator_priv;

    CHECK_POINTER(pstCtrl);

    switch (tvp->cmd) {
        case DTV_TS_INPUT:
            tvp->u.data = pstCtrl->u8TsId;
            break;

        case DTV_DELIVERY_SYSTEM:
            tvp->u.data = pstCtrl->enDeliverySystem;
            break;

        case DTV_DVBT2_PLP_ID:
            u32PlpNum = 0;
            for (u32Index = 0; u32Index < g_stScanCallbackInfo.u32ChannelCount; u32Index++) {
                if (FRONTEND_TYPE_DVBT2 == g_stScanCallbackInfo.astFrontendAttrInfo[u32Index].enFrontendType) {
                    au8PlpId[u32PlpNum] = g_stScanCallbackInfo.astFrontendAttrInfo[u32Index].u32StreamId;
                    u32PlpNum++;
                }
            }

            tvp->u.buffer.reserved1[0] = u32PlpNum;
            if (u32PlpNum > 0 && copy_to_user(tvp->u.buffer.reserved2, au8PlpId, u32PlpNum * sizeof(_U8))) {
                tvp->u.buffer.reserved1[0] = 0;
            }
            break;

        default:
            break;
    }

    return s32Ret;
}

static int _get_fe_ops(PVS_FE_CTRL pstCtrl, enum fe_delivery_system enDeliverySystem)
{
    _S32 s32Ret = _SUCCESS;

    int index = 0;

    SYSTEM_TYPE_E enSystemType = SYSTEM_TYPE_INVALID;

    CHECK_POINTER(pstCtrl);
    CHECK_DELIVERY_SYSTEM(enDeliverySystem);

    DBG_INFO("delivery_system: %d", enDeliverySystem);

    s32Ret = _get_system_type(pstCtrl, enDeliverySystem, &enSystemType);
    if (_SUCCESS != s32Ret) {
        return _FAILURE;
    }

    DBG_INFO("system_type: %d", enSystemType);

    memcpy(pstCtrl->stDvbFrontend.ops.info.name, COMMON_NAME, strlen(COMMON_NAME));

    switch (enSystemType) {
        case SYSTEM_TYPE_DVB_TX:
            pstCtrl->stDvbFrontend.ops.info.type = FE_OFDM;
            break;

        case SYSTEM_TYPE_DVB_C:
            pstCtrl->stDvbFrontend.ops.info.type = FE_QAM;
            break;

        case SYSTEM_TYPE_ISDB_T:
            pstCtrl->stDvbFrontend.ops.info.type = FE_ISDBT;
            break;

        case SYSTEM_TYPE_SAT:
            pstCtrl->stDvbFrontend.ops.info.type = FE_QPSK;
            break;

        default:
            return -EINVAL;
    }

    pstCtrl->stDvbFrontend.ops.info.caps =
        FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
        FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
        FE_CAN_TRANSMISSION_MODE_AUTO |
        FE_CAN_GUARD_INTERVAL_AUTO |
        FE_CAN_HIERARCHY_AUTO |
        FE_CAN_RECOVER |
        FE_CAN_MUTE_TS;

    if (SYSTEM_TYPE_SAT == enSystemType) {
        pstCtrl->stDvbFrontend.ops.info.frequency_min = 950000;
        pstCtrl->stDvbFrontend.ops.info.frequency_max = 2150000UL;
        pstCtrl->stDvbFrontend.ops.info.caps |=
            FE_CAN_QPSK;
    } else {
        pstCtrl->stDvbFrontend.ops.info.frequency_min = 42000000;
        pstCtrl->stDvbFrontend.ops.info.frequency_max = 1002000000UL;
        pstCtrl->stDvbFrontend.ops.info.caps |=
            FE_CAN_QAM_16 | FE_CAN_QAM_32 | FE_CAN_QAM_64 | FE_CAN_QAM_128 | FE_CAN_QAM_256 | FE_CAN_QAM_AUTO;
    }

    //pstCtrl->stDvbFrontend.ops.release = vs_fe_release;
    //pstCtrl->stDvbFrontend.ops.release_sec = vs_fe_release_sec;

    //pstCtrl->stDvbFrontend.ops.init = vs_fe_init;
    //pstCtrl->stDvbFrontend.ops.sleep = vs_fe_sleep;

    //pstCtrl->stDvbFrontend.ops.write = vs_fe_write;

    pstCtrl->stDvbFrontend.ops.tune = vs_fe_tune;

    pstCtrl->stDvbFrontend.ops.get_frontend_algo = vs_fe_get_frontend_algo;

    pstCtrl->stDvbFrontend.ops.set_frontend = vs_fe_set_frontend;
    pstCtrl->stDvbFrontend.ops.get_tune_settings = vs_fe_get_tune_settings;

    pstCtrl->stDvbFrontend.ops.get_frontend = vs_fe_get_frontend;

    pstCtrl->stDvbFrontend.ops.read_status = vs_fe_read_status;
    pstCtrl->stDvbFrontend.ops.read_ber = vs_fe_read_ber;
    pstCtrl->stDvbFrontend.ops.read_signal_strength = vs_fe_read_signal_strength;
    pstCtrl->stDvbFrontend.ops.read_snr = vs_fe_read_snr;
    pstCtrl->stDvbFrontend.ops.read_ucblocks = vs_fe_read_ucblocks;

    pstCtrl->stDvbFrontend.ops.diseqc_reset_overload = vs_fe_diseqc_reset_overload;
    pstCtrl->stDvbFrontend.ops.diseqc_send_master_cmd = vs_fe_diseqc_send_master_cmd;
    pstCtrl->stDvbFrontend.ops.diseqc_recv_slave_reply = vs_fe_diseqc_recv_slave_reply;
    pstCtrl->stDvbFrontend.ops.diseqc_send_burst = vs_fe_diseqc_send_burst;
    pstCtrl->stDvbFrontend.ops.set_tone = vs_fe_set_tone;
    pstCtrl->stDvbFrontend.ops.set_voltage = vs_fe_set_voltage;
    pstCtrl->stDvbFrontend.ops.enable_high_lnb_voltage = vs_fe_enable_high_lnb_voltage;
    //pstCtrl->stDvbFrontend.ops.dishnetwork_send_legacy_command = vs_fe_dishnetwork_send_legacy_command;
    //pstCtrl->stDvbFrontend.ops.i2c_gate_ctrl = vs_fe_i2c_gate_ctrl;
    //pstCtrl->stDvbFrontend.ops.ts_bus_ctrl = vs_fe_ts_bus_ctrl;
    //pstCtrl->stDvbFrontend.ops.set_lna = vs_fe_set_lna;

    //pstCtrl->stDvbFrontend.ops.search = vs_fe_search;

    //pstCtrl->stDvbFrontend.ops.tuner_ops = vs_fe_tuner_ops;
    //pstCtrl->stDvbFrontend.ops.analog_ops = vs_fe_analog_ops;

    pstCtrl->stDvbFrontend.ops.set_property = vs_fe_set_property;
    pstCtrl->stDvbFrontend.ops.get_property = vs_fe_get_property;

    if (GET_SYSTEM_TYPE(pstCtrl, SYSTEM_TYPE_DVB_TX)) {
        pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_DVBT;
        pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_DVBT2;
    }

    if (GET_SYSTEM_TYPE(pstCtrl, SYSTEM_TYPE_DVB_C)) {
        pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_DVBC_ANNEX_A;
        pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_DVBC_ANNEX_B;
        pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_DVBC_ANNEX_C;
    }

    if (GET_SYSTEM_TYPE(pstCtrl, SYSTEM_TYPE_ISDB_T)) {
        pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_ISDBT;
    }

    if (GET_SYSTEM_TYPE(pstCtrl, SYSTEM_TYPE_SAT)) {
        pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_DVBS;
        pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_DVBS2;
    }

    pstCtrl->stDvbFrontend.ops.delsys[index++] = SYS_ANALOG;

    return s32Ret;
}

static _S32 _do_fe_upload(PVS_FE_CTRL pstCtrl)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    enum fe_delivery_system enDeliverySystem = SYS_UNDEFINED;

    struct aml_dvb *aml_dvb_device = aml_get_dvb_device();

    DBG_INFO("");

    CHECK_POINTER(pstCtrl);

    CHECK_POINTER(aml_dvb_device);

    if (TUNER_ID_UNDEFINED == pstCtrl->enTunerId) {
        return _FAILURE;
    }

    if (SYS_UNDEFINED != pstCtrl->enDeliverySystem) {
        dvb_unregister_frontend(&pstCtrl->stDvbFrontend);
    }

    bRet = VS_FE_Tuner_Init(pstCtrl);
    if ((!bRet) || (!GET_SYSTEM(pstCtrl))) {
        return _FAILURE;
    }

    if (SYS_UNDEFINED == enDeliverySystem) {
        if (GET_SYSTEM_TYPE(pstCtrl, SYSTEM_TYPE_DVB_TX)) {
            enDeliverySystem = SYS_DVBT;
        } else if (GET_SYSTEM_TYPE(pstCtrl, SYSTEM_TYPE_DVB_C)) {
            enDeliverySystem = SYS_DVBC_ANNEX_A;
        } else if (GET_SYSTEM_TYPE(pstCtrl, SYSTEM_TYPE_ISDB_T)) {
            enDeliverySystem = SYS_ISDBT;
        } else if (GET_SYSTEM_TYPE(pstCtrl, SYSTEM_TYPE_SAT)) {
            enDeliverySystem = SYS_DVBS;
        }
    }

    if (SYS_UNDEFINED == enDeliverySystem) {
        return _FAILURE;
    }

    s32Ret = _get_fe_ops(pstCtrl, enDeliverySystem);
    if (_SUCCESS != s32Ret) {
        return _FAILURE;
    }

    pstCtrl->stDvbFrontend.demodulator_priv = pstCtrl;
    s32Ret = dvb_register_frontend(&aml_dvb_device->dvb_adapter, &pstCtrl->stDvbFrontend);
    if (s32Ret) {
        return s32Ret;
    }

    pstCtrl->enDeliverySystem = enDeliverySystem;

    return s32Ret;
}

static ssize_t store_tuner_type(struct class *class, struct class_attribute *attr, const char *buf, size_t size)
{
    _S32 s32Ret = _SUCCESS;

    TUNER_ID_E enTunerId = TUNER_ID_UNDEFINED;

    CHECK_POINTER(buf);
    CHECK_POINTER(g_apstCtrl[0]);

    if (!strcasecmp(buf, "avl68xx")) {
        enTunerId = TUNER_ID_COMBO_AVL68XX;
    } else if (!strcasecmp(buf, "rs6060")) {
        enTunerId = TUNER_ID_RS6060;
    } else {
        return _FAILURE;
    }

    if ((TUNER_ID_UNDEFINED != enTunerId) && (g_apstCtrl[0]->enTunerId != enTunerId)) {
        g_apstCtrl[0]->enTunerId = enTunerId;
        (void)_do_fe_upload(g_apstCtrl[0]);
    }

    return s32Ret;
}

static struct class_attribute vs_frontend_attrs[] = {
    __ATTR(tuner_type, S_IRUGO | S_IWUSR, _NULL, store_tuner_type),
    __ATTR_NULL
};

static int vs_frontend_probe(struct platform_device *pdev)
{
    _S32 s32Ret = _SUCCESS;

    _S8 as8Buf[128];
    _U32 u32Value;
    const _S8 *ps8Str = _NULL;

    struct platform_device *p_dvb_platform_device = _NULL;
    struct device_node *p_device_node = _NULL;

    PVS_FE_CTRL pstCtrl = _NULL;

    struct aml_dvb *aml_dvb_device = aml_get_dvb_device();

    DBG_INFO("");

    CHECK_POINTER(aml_dvb_device);

    p_dvb_platform_device = aml_dvb_device->pdev;

    CHECK_POINTER(p_dvb_platform_device);

    g_apstCtrl[0] = pstCtrl = kzalloc(sizeof(VS_FE_CTRL), GFP_KERNEL);
    if (_NULL == pstCtrl) {
        return _FAILURE;
    }

    platform_set_drvdata(pdev, pstCtrl);
    memset(pstCtrl, 0, sizeof(VS_FE_CTRL));

    pstCtrl->bInitFlag = 0;
    pstCtrl->bSuspendFlag = 0;

    pstCtrl->u8TunerIndex = 0;

    pstCtrl->enTunerId = TUNER_ID_UNDEFINED;

    memset(as8Buf, 0, sizeof(as8Buf));
    snprintf(as8Buf, sizeof(as8Buf), "dtv_demod%d", pstCtrl->u8TunerIndex);
    s32Ret = of_property_read_string(p_dvb_platform_device->dev.of_node, as8Buf, &ps8Str);
    if (!s32Ret) {
        if (!strcasecmp(ps8Str, "avl68xx")) {
            pstCtrl->enTunerId = TUNER_ID_COMBO_AVL68XX;
        } else if (!strcasecmp(ps8Str, "rs6060")) {
            pstCtrl->enTunerId = TUNER_ID_RS6060;
        }
    }

    memset(as8Buf, 0, sizeof(as8Buf));
    snprintf(as8Buf, sizeof(as8Buf), "dtv_demod%d_ts_id", pstCtrl->u8TunerIndex);
    s32Ret = of_property_read_u32(p_dvb_platform_device->dev.of_node, as8Buf, &u32Value);
    if (!s32Ret) {
        pstCtrl->u8TsId = u32Value;
    } else {
        pstCtrl->u8TsId = 0;
    }

    DBG_INFO("TS id: %d", pstCtrl->u8TsId);

    memset(as8Buf, 0, sizeof(as8Buf));
    snprintf(as8Buf, sizeof(as8Buf), "dtv_demod%d_i2c_adap", pstCtrl->u8TunerIndex);
    p_device_node = of_parse_phandle(p_dvb_platform_device->dev.of_node, as8Buf, 0);
    if (p_device_node) {
        pstCtrl->pstI2cAdap = of_find_i2c_adapter_by_node(p_device_node);
        of_node_put(p_device_node);
    } else {
        kfree(pstCtrl);

        return _FAILURE;
    }

    (void)_do_fe_upload(pstCtrl);

    pstCtrl->stClass.name = COMMON_NAME;
    pstCtrl->stClass.owner = THIS_MODULE;
    pstCtrl->stClass.class_attrs = vs_frontend_attrs;

    s32Ret = class_register(&pstCtrl->stClass);
    if (s32Ret) {
        if (SYS_UNDEFINED != pstCtrl->enDeliverySystem) {
            dvb_unregister_frontend(&pstCtrl->stDvbFrontend);
        }

        kfree(pstCtrl);

        return s32Ret;
    }

    return s32Ret;
}

static int vs_frontend_remove(struct platform_device *pdev)
{
    _S32 s32Ret = _SUCCESS;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(pdev);

    pstCtrl = (PVS_FE_CTRL)platform_get_drvdata(pdev);

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    class_unregister(&pstCtrl->stClass);

    if (SYS_UNDEFINED != pstCtrl->enDeliverySystem) {
        dvb_unregister_frontend(&pstCtrl->stDvbFrontend);
    }

    kfree(pstCtrl);

    return s32Ret;
}

static int vs_frontend_suspend(struct platform_device *pdev, pm_message_t state)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    enum fe_sec_voltage enCurAntVoltage = SEC_VOLTAGE_OFF;

    DBG_INFO("");

    CHECK_POINTER(pdev);

    pstCtrl = (PVS_FE_CTRL)platform_get_drvdata(pdev);

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    enCurAntVoltage = pstCtrl->enCurAntVoltage;
    bRet |= VS_FE_Tuner_SetVoltage(pstCtrl, SEC_VOLTAGE_OFF);
    pstCtrl->enCurAntVoltage = enCurAntVoltage;

    bRet |= VS_FE_Tuner_Standby(pstCtrl);

    (void)VS_FE_SetDemodReset(pstCtrl->u8TunerIndex, 0);

    return s32Ret;
}

static int vs_frontend_resume(struct platform_device *pdev)
{
    _S32 s32Ret = _SUCCESS;
    _BOOL bRet = _TRUE;

    PVS_FE_CTRL pstCtrl = _NULL;

    DBG_INFO("");

    CHECK_POINTER(pdev);

    pstCtrl = (PVS_FE_CTRL)platform_get_drvdata(pdev);

    CHECK_POINTER(pstCtrl);
    CHECK_INIT(pstCtrl);

    bRet |= VS_FE_Tuner_Resume(pstCtrl);

    bRet |= VS_FE_Tuner_SetVoltage(pstCtrl, pstCtrl->enCurAntVoltage);

    bRet |= VS_FE_Tuner_StartLock(pstCtrl, &g_stScanCallbackInfo, _FALSE);

    return s32Ret;
}

static struct platform_driver vs_frontend_driver = {
    .probe   = vs_frontend_probe,
    .remove  = vs_frontend_remove,
    .suspend = vs_frontend_suspend,
    .resume  = vs_frontend_resume,
    .driver  = {
        .name = COMMON_NAME,
        .owner = THIS_MODULE,
    }
};

static void vs_frontend_release(struct device *dev)
{
    DBG_INFO("");

    return ;
}

static struct platform_device vs_frontend_device = {
    .name = COMMON_NAME,
    .id   = 0,
    .dev  = {
        .release  = vs_frontend_release,
    },
};

static int __init vs_frontend_init(void)
{
    DBG_INFO("");

    platform_driver_register(&vs_frontend_driver);
    platform_device_register(&vs_frontend_device);

    return 0;
}

static void __exit vs_frontend_exit(void)
{
    DBG_INFO("");

    platform_driver_unregister(&vs_frontend_driver);
    platform_device_unregister(&vs_frontend_device);
}

fs_initcall(vs_frontend_init);
module_exit(vs_frontend_exit);

MODULE_DESCRIPTION("Driver for vs_frontend demodulator");
MODULE_AUTHOR("VS");
MODULE_LICENSE("GPL");
