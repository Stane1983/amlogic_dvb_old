//////////////////////////////////////////////////////////////////////////
/////
// TH DTV MIDDLE WARE DEFINES, PLEASE DO NOT MAKE ANY CHANGE OF THIS FILE
/////
//////////////////////////////////////////////////////////////////////////

#ifndef __VS_FE_TUNER_H__
#define __VS_FE_TUNER_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <linux/i2c.h>
#include <linux/types.h>
#include <uapi/linux/dvb/frontend.h>
#include "aml_dvb.h"

#define SAT_SCR

#define MAIN_TUNER_INDEX        0
#define SECOND_TUNER_INDEX      1
#define SYSTEM_TUNER_NUMBER     2

#define FRONTEND_ATTR_INFO_NUMBER 255

typedef enum {
    TUNER_ID_UNDEFINED = 0,
    TUNER_ID_COMBO_AVL68XX = 1,
    TUNER_ID_RS6060 = 2,
} TUNER_ID_E, *PTUNER_ID_E;

typedef enum {
    SYSTEM_TYPE_MIN = 0,
    SYSTEM_TYPE_DVB_TX = SYSTEM_TYPE_MIN,
    SYSTEM_TYPE_DVB_C,
    SYSTEM_TYPE_ISDB_T,
    SYSTEM_TYPE_DTMB,
    SYSTEM_TYPE_ATSC,
    SYSTEM_TYPE_TER_COUNT,
    SYSTEM_TYPE_SAT = SYSTEM_TYPE_TER_COUNT,
    SYSTEM_TYPE_COUNT,
    SYSTEM_TYPE_INVALID = SYSTEM_TYPE_COUNT,
    SYSTEM_TYPE_MAX
} SYSTEM_TYPE_E, *PSYSTEM_TYPE_E;

typedef enum {
    FRONTEND_TYPE_MIN = 0,
    FRONTEND_TYPE_Undef = FRONTEND_TYPE_MIN,
    FRONTEND_TYPE_TER,
    FRONTEND_TYPE_DVBT2,
    FRONTEND_TYPE_DVBSX,
    FRONTEND_TYPE_DVBS2_MIS,
    FRONTEND_TYPE_MAX
} FRONTEND_TYPE_E, *PFRONTEND_TYPE_E;

typedef struct
{
    _BOOL bSystemTypeDetect[SYSTEM_TUNER_NUMBER][SYSTEM_TYPE_COUNT];
    _BOOL bSystemDetect[SYSTEM_TUNER_NUMBER];
} SYSTEM_INFO_T, *PSYSTEM_INFO_T;

typedef struct
{
    FRONTEND_TYPE_E enFrontendType;
 
    _U32 u32MutiAttr; /* dvbc_j83b, 0: not_j83b, 1: is_j83b; \
                            * dvbt_profile, 0: low, 1: high; \
                            * dvbt2_channel_mode, 0: base, 1: lite; \
                            * atsc, 0: air; 1: cable, 2: cable_64QAM, 3: cable_256QAM; \
                            * dvbs2_mis_system_type */
    _U32 u32StreamId; /* dvbt2_plp_id; dvbs2_mis_isi */
} FRONTEND_ATTR_INFO_T, *PFRONTEND_ATTR_INFO_T; 

typedef struct
{
    FRONTEND_ATTR_INFO_T astFrontendAttrInfo[FRONTEND_ATTR_INFO_NUMBER];

    _U32 u32ChannelCount; 
} SCAN_CALLBACK_INFO_T, *PSCAN_CALLBACK_INFO_T;

typedef struct
{
    SYSTEM_TYPE_E enSystemType;

    _U32 u32Frequency; /* dvbsx: MHz, other: KHz */
    _U32 u32Symbolrate; /* KSps */
    _U32 u32Bandwidth; /* MHz */

    _BOOL bFrontendAttrInfoValid;
    FRONTEND_ATTR_INFO_T stFrontendAttrInfo;
} TUNER_PARAM_INFO_T, *PTUNER_PARAM_INFO_T;

typedef struct
{
    TUNER_PARAM_INFO_T stTunerParamInfo;

    PSCAN_CALLBACK_INFO_T pstScanCallbackInfo;
} LOCK_PARAM_INFO_T, *PLOCK_PARAM_INFO_T;

typedef struct
{
    _U8 u8TunerIndex;
    _U8 u8Process;

    _BOOL bLocked;
    
    _U32 u32Frequency; /* KHz */
    _U32 u32Symbolrate; /* KSps */

    _BOOL bFrontendAttrInfoValid;
    FRONTEND_ATTR_INFO_T stFrontendAttrInfo;
} BLIND_SCAN_CALLBACK_INFO, *PBLIND_SCAN_CALLBACK_INFO;

typedef struct
{
    _BOOL bBlindScan;
    
    struct dvbsx_blindscanpara stBlindScanParam;

    _BOOL bSupportMis;

#ifdef SAT_SCR
    _BOOL bScrOn;
    _U32 u32ScrNumber;
    _U32 u32ScrBandPassFrequency;
    _U32 u32ScrLnbNumber; 
#endif
} BLIND_SCAN_PARAM_INFO, *PBLIND_SCAN_PARAM_INFO;

typedef struct {
    _BOOL bInitFlag;
    _BOOL bSuspendFlag;

    //DemodI2cAddr, TunerI2cAddr
    TUNER_ID_E enTunerId;
    _U8 u8TunerIndex;
    _U8 u8TsId;
    struct i2c_adapter *pstI2cAdap;

    struct dvb_frontend stDvbFrontend;

    SYSTEM_INFO_T stSystemInfo;
    TUNER_PARAM_INFO_T stTunerParamInfo;

    BLIND_SCAN_PARAM_INFO stBlindScanParamInfo;

    enum fe_delivery_system enDeliverySystem; /* SYS_DVBT SYS_DVBS... */
    enum fe_sec_voltage enCurAntVoltage; /* SEC_VOLTAGE_OFF SEC_VOLTAGE_ON */
    enum fe_sec_voltage enCurSecVoltage; /* SEC_VOLTAGE_13 SEC_VOLTAGE_18 */

    struct class stClass;
} VS_FE_CTRL, *PVS_FE_CTRL;

typedef void (*_BlindScanCallback)(PBLIND_SCAN_CALLBACK_INFO pstBlindScanCallbackInfo);

void VS_FE_SetDemodReset(_U8 u8TunerIndex, _U8 u8Value);
void VS_FE_SetAntPower(_U8 u8TunerIndex, _U8 u8Value);
void VS_FE_SetTunerPowerEnable(_U8 u8TunerIndex, _U8 u8Value);
void VS_FE_SetUserDefined1(_U8 u8Value);
void VS_FE_SetUserDefined2(_U8 u8Value);
void VS_FE_GetDemodReset(_U8 u8TunerIndex, _U8 *pu8Value);
void VS_FE_GetAntPower(_U8 u8TunerIndex, _U8 *pu8Value);
void VS_FE_GetTunerPowerEnable(_U8 u8TunerIndex, _U8 *pu8Value);
void VS_FE_GetUserDefined1(_U8 *pu8Value);
void VS_FE_GetUserDefined2(_U8 *pu8Value);

_BOOL VS_FE_Tuner_GetSystemInfo(enum fe_delivery_system enDeliverySystem, PSYSTEM_TYPE_E penSystemType);

_BOOL VS_FE_Tuner_Init(PVS_FE_CTRL pstCtrl);
_BOOL VS_FE_Tuner_Standby(PVS_FE_CTRL pstCtrl);
_BOOL VS_FE_Tuner_Resume(PVS_FE_CTRL pstCtrl);

_BOOL VS_FE_Tuner_StartLock(PVS_FE_CTRL pstCtrl, PSCAN_CALLBACK_INFO_T pstScanCallbackInfo, _BOOL bWaitLock);
_BOOL VS_FE_Tuner_StopLock(PVS_FE_CTRL pstCtrl);
_BOOL VS_FE_Tuner_ExitLock(PVS_FE_CTRL pstCtrl);

_BOOL VS_FE_Tuner_Sat_BlindScan_Start(PVS_FE_CTRL pstCtrl, _BlindScanCallback pfnBlindScanCallback, PBLIND_SCAN_PARAM_INFO pstBlindScanParamInfo);
_BOOL VS_FE_Tuner_Sat_BlindScan_Stop(PVS_FE_CTRL pstCtrl);

_BOOL VS_FE_Tuner_SetVoltage(PVS_FE_CTRL pstCtrl, enum fe_sec_voltage enVoltage);

_BOOL VS_FE_Tuner_Sat_SetDiseqcContinuous(PVS_FE_CTRL pstCtrl, _BOOL bOn);
_BOOL VS_FE_Tuner_Sat_SendDiseqcCommand(PVS_FE_CTRL pstCtrl, _U8 *pu8Data,_U8 u8Length);
_BOOL VS_FE_Tuner_Sat_SendDiseqcBurst(PVS_FE_CTRL pstCtrl, _U8 u8Command);

_BOOL VS_FE_Tuner_CheckLocked(PVS_FE_CTRL pstCtrl, _BOOL *pbLock);
_BOOL VS_FE_Tuner_GetBer(PVS_FE_CTRL pstCtrl, _U32 *pu32Ber);
_BOOL VS_FE_Tuner_GetStrength(PVS_FE_CTRL pstCtrl, _U16 *pu16Strength);
_BOOL VS_FE_Tuner_GetSnr(PVS_FE_CTRL pstCtrl, _U16 *pu16Snr);

#ifdef __cplusplus
}
#endif

#endif    // __VS_FE_TUNER_H__

//////////////////////////////////////////////////////////////////////////
/////
// TH DTV MIDDLE WARE DEFINES, PLEASE DO NOT MAKE ANY CHANGE OF THIS FILE
/////
//////////////////////////////////////////////////////////////////////////
