/*
 *           Copyright 2012 Availink, Inc.
 *
 *  This software contains Availink proprietary information and
 *  its use and disclosure are restricted solely to the terms in
 *  the corresponding written license agreement. It shall not be 
 *  disclosed to anyone other than valid licensees without
 *  written permission of Availink, Inc.
 *
 */


///$Date: 2012-2-9 17:36 $
///
#include "AVL68XX_IBlindScan.h"

#ifdef SAT_SCR
extern _BOOL	g_68xx_use_unicable_device;
extern _U16	g_68xx_ub_freq_MHz;
extern _BOOL SCR_ODU_ChannelChange(_U16 channel_freq_MHz, _U16 *real_freq_MHz);
#endif

AVL_ErrorCode AVL68XX_DVBSx_IBlindScan_Scan(AVL_BlindScanPara * pBSPara, AVL_uint16 uiTunerLPF_100kHz,const AVL_uint32 uiChipNo)
{
	AVL_ErrorCode r;
	r = AVL_Demod_DVBSx_BlindScan_Start(pBSPara ,uiTunerLPF_100kHz, uiChipNo);
	return(r);
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScan_GetScanStatus(AVL_BSInfo * pBSInfo, AVL_uint32 uiChipNo)
{
	AVL_ErrorCode r = AVL_EC_OK;

	r = AVL_Demod_DVBSx_BlindScan_GetStatus(pBSInfo,uiChipNo);
	return(r);
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScan_Cancel(AVL_uint32 uiChipNo)
{
	AVL_ErrorCode r;
	r = AVL_Demod_DVBSx_BlindScan_Cancel(uiChipNo);

	return(r);
}

AVL_ErrorCode AVL68XX_Demod_DVBSx_BlindScan_ReadChannelInfo(AVL_uint16 uiStartIndex, AVL_puint16 pChannelCount, AVL_ChannelInfo * pChannel, const AVL_uint32 uiChipNo)
{
	AVL_ErrorCode r;

	r = AVL_Demod_DVBSx_BlindScan_ReadChannelInfo(uiStartIndex,pChannelCount,pChannel,uiChipNo);
	return(r);
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScan_Reset(AVL_uint32 uiChipNo)
{
	AVL_ErrorCode r;

	r = AVL_Demod_DVBSx_BlindScan_Reset(uiChipNo);

	return(r);
}

