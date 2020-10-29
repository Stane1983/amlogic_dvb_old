#ifndef __AVL68XX_PORTING__
#define __AVL68XX_PORTING__

#define MAX_CHANNEL_INFO 256

#include "AVL_Demod.h"
#include "AVL_Tuner.h"

#include "vs_fe_external.h"
#include "vs_fe_tuner.h"

typedef struct s_DVBTx_Channel_TS
{
    // number, example 474*1000 is RF frequency 474MHz.
    int channel_freq_khz;
    // number, example 8000 is 8MHz bandwith channel.
    int channel_bandwith_khz;

    AVL_DVBTx_Standard channel_type;
    // 0 - Low priority layer, 1 - High priority layer
    unsigned char dvbt_hierarchy_layer;
    // data PLP id, 0 to 255; for single PLP DVBT2 channel, this ID is 0; for DVBT channel, this ID isn't used.
    unsigned char data_plp_id;
    AVL_DVBT2_PROFILE channel_profile;
}s_DVBTx_Channel_TS;


extern s_DVBTx_Channel_TS global_channel_ts_table[MAX_CHANNEL_INFO];
extern AVL_uint16 g_nChannel_ts_total;

void AVL68XX_SetExitLockTuner(AVL_uint32 uiChipNo, _BOOL bExitLock);
_BOOL AVL68XX_GetExitLockTuner(AVL_uint32 uiChipNo);

_U32 AVL_Init(AVL_uint32 uiChipNo, _U32 *pChipId);
AVL_ErrorCode AVL_LockChannel_DVBSx(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint32 Symbol_Khz);
AVL_ErrorCode AVL_LockChannel_ISDBT(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz, AVL_uint16 BandWidth_Khz);
AVL_ErrorCode AVL_LockChannel_T(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint16 BandWidth_Khz, AVL_int32 DVBT_layer_info);
AVL_ErrorCode AVL_LockChannel_T2(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint16 BandWidth_Khz, AVL_uchar T2_Profile, AVL_int32 PLP_ID);
AVL_ErrorCode AVL_LockChannel_DVBC(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint32 symbolRate);
AVL_ErrorCode AVL_SetWorkMode(AVL_uint32 uiChipNo, AVL_DemodMode eDemodWorkMode);
AVL_ErrorCode AVL_ScanChannel_Tx(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint16 BandWidth_Khz);
AVL_uchar get_SSI_info(AVL_uint32 uiChipNo);
AVL_uchar get_SQI_info(AVL_uint32 uiChipNo);
AVL_uint32 get_SNR_info(AVL_uint32 uiChipNo);
AVL_uint32 get_PER_info(AVL_uint32 uiChipNo);
void AVL_PrintVersion(AVL_uint32 uiChipNo);

void AVL68XX_BlindScanStartLoop(AVL_uint32 uiChipNo, _BlindScanCallback pCallback,PBLIND_SCAN_PARAM_INFO pScanParams);
void AVL68XX_BlindScanStopLoop(AVL_uint32 uiChipNo);

#endif

