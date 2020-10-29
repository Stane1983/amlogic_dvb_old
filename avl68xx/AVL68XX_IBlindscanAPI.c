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


///$Date: 2012-3-8 21:47 $
///
#include "AVL68XX_IBlindScan.h"
#include "AVL68XX_IBlindscanAPI.h"

#ifdef SAT_SCR
extern _BOOL	g_68xx_use_unicable_device;
extern _U16	g_68xx_ub_freq_MHz;
//extern _BOOL SCR_ODU_ChannelChange(_U16 channel_freq_MHz, _U16 *real_freq_MHz);
#endif

AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_Initialize(struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting)
{		
	pBSsetting->m_uiScan_Start_Freq_MHz = 950;     //Default Set Blind scan start frequency
	pBSsetting->m_uiScan_Stop_Freq_MHz = 2150;     //Default Set Blind scan stop frequency
	pBSsetting->m_uiScan_Next_Freq_100KHz = 340+10*pBSsetting->m_uiScan_Start_Freq_MHz;

	pBSsetting->m_uiScan_Max_Symbolrate_MHz = 45;  //Set MAX symbol rate
	pBSsetting->m_uiScan_Min_Symbolrate_MHz = 2;   //Set MIN symbol rate
	
	pBSsetting->m_uiTuner_MaxLPF_100kHz = 340;//440;

	pBSsetting->m_uiScan_Bind_No = 0;
	pBSsetting->m_uiScan_Progress_Per = 0;
	pBSsetting->m_uiScaning_Flag = 0;
	pBSsetting->m_uiChannelCount = 0;

	pBSsetting->m_eSpectrumMode = AVL_Spectrum_Normal;  //Set spectrum mode

	//pBSsetting->BS_Mode = AVL68XX_DVBSx_BS_Slow_Mode; //1: Freq Step forward is 10MHz        0: Freq Step firmware is 20.7MHz
	//pBSsetting->m_uiScan_Center_Freq_Step_100KHz = 100;  //only valid when scan_algorithmic set to 1 and would be ignored when scan_algorithmic set to 0.

	return AVL_EC_OK;
}
AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_SetSpectrumMode(struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting, AVL_BlindSanSpectrumPolarity SpectrumMode)
{
	pBSsetting->m_eSpectrumMode = SpectrumMode;
	pBSsetting->bsPara.m_enumBSSpectrumPolarity = SpectrumMode;

	return AVL_EC_OK;
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_SetScanMode(struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting, enum AVL68XX_DVBSx_BlindScanAPI_Mode Scan_Mode)
{
	//pBSsetting->BS_Mode = Scan_Mode;
	return AVL_EC_OK;
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_SetFreqRange(struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting,AVL_uint16 StartFreq_MHz,AVL_uint16 EndFreq_MHz)
{
	pBSsetting->m_uiScan_Start_Freq_MHz = StartFreq_MHz;     //Change default start frequency
	pBSsetting->m_uiScan_Stop_Freq_MHz = EndFreq_MHz;        //Change default end frequency
	pBSsetting->m_uiScan_Next_Freq_100KHz = 340+ 10*pBSsetting->m_uiScan_Start_Freq_MHz;
	
	return AVL_EC_OK;
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_SetMaxLPF(struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting ,AVL_uint16 MaxLPF)
{
	pBSsetting->m_uiTuner_MaxLPF_100kHz = MaxLPF;
	
	return AVL_EC_OK;
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_Start(AVL_uint32 uiChipNo,AVL_Tuner * pTuner, struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting)
{
	AVL_ErrorCode r = AVL_EC_OK;
#ifdef SAT_SCR
	//_U16 real_freq_MHz;
#endif

	AVL_BlindScanPara * pbsPara = &pBSsetting->bsPara;
	
#if 0
	if(pBSsetting->BS_Mode)
	{
		pTuner->uiRFFrequencyHz = (10*pBSsetting->m_uiScan_Start_Freq_MHz + 340 + (pBSsetting->m_uiScan_Bind_No) * pBSsetting->m_uiScan_Center_Freq_Step_100KHz)*100*1000;
		pbsPara->m_uiStartFreq_100kHz = pTuner->uiRFFrequencyHz/(100*1000) - 340;
		pbsPara->m_uiStopFreq_100kHz =  pTuner->uiRFFrequencyHz/(100*1000) + 340;
	}
	else
	{
		pbsPara->m_uiStartFreq_100kHz = pBSsetting->m_uiScan_Next_Freq_100KHz;
		pbsPara->m_uiStopFreq_100kHz = pBSsetting->m_uiScan_Next_Freq_100KHz + 340*2;
		pTuner->uiRFFrequencyHz = (pbsPara->m_uiStartFreq_100kHz + pbsPara->m_uiStopFreq_100kHz)/2*100*1000;
	}
#else
		//#add
		pTuner->uiRFFrequencyHz = pBSsetting->m_uiScan_Next_Freq_100KHz*100*1000;//(10*pBSsetting->m_uiScan_Start_Freq_MHz + 340 + (pBSsetting->m_uiScan_Bind_No) * pBSsetting->m_uiScan_Center_Freq_Step_100KHz)*100*1000;
		pbsPara->m_uiStartFreq_100kHz = pTuner->uiRFFrequencyHz/(100*1000) - 340;
		pbsPara->m_uiStopFreq_100kHz =  pTuner->uiRFFrequencyHz/(100*1000) + 340;
#endif

	pTuner->uiLPFHz =  pBSsetting->m_uiTuner_MaxLPF_100kHz*100*1000;
#ifdef SAT_SCR
	if(g_68xx_use_unicable_device){
		//SCR_ODU_ChannelChange(pTuner->m_uiFrequency_100kHz/10, &real_freq_MHz);
		//pTuner->m_uiFrequency_100kHz = g_ub_freq_MHz*10;
	}
#endif
	
	AVL_Demod_I2CBypassOn(uiChipNo);
	r |= pTuner->fpLockFunc(pTuner);	//Lock the tuner. 
	
	AVL_IBSP_Delay(50);		//wait a while for tuner to lock in certain frequency.
	
	r |= pTuner->fpGetLockStatusFunc(pTuner);	 //Check the lock status of the tuner.
	AVL_Demod_I2CBypassOff(uiChipNo);

	if (AVL_EC_OK != r)		 
	{
		return r;
	}

	pbsPara->m_uiMaxSymRate_kHz = 1000*pBSsetting->m_uiScan_Max_Symbolrate_MHz;
	pbsPara->m_uiMinSymRate_kHz = 1000*pBSsetting->m_uiScan_Min_Symbolrate_MHz;

	r |= AVL_Demod_DVBSx_SetFunctionalMode(AVL_FuncMode_BlindScan,uiChipNo);
	r |= AVL68XX_DVBSx_IBlindScan_Reset(uiChipNo);
	r |= AVL68XX_DVBSx_IBlindScan_Scan(pbsPara,pBSsetting->m_uiTuner_MaxLPF_100kHz, uiChipNo);
	pBSsetting->m_uiScaning_Flag = 1;
	
	return r;
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_GetCurrentScanStatus(AVL_uint32 uiChipNo,struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting)
{
	AVL_ErrorCode r = AVL_EC_OK;
	AVL_BSInfo * pbsInfo = &(pBSsetting->bsInfo);
	//AVL_BlindScanPara * pbsPara = &(pBSsetting->bsPara);	
    
	r |= AVL68XX_DVBSx_IBlindScan_GetScanStatus(pbsInfo, uiChipNo);  //Query the internal blind scan procedure information.

	if(100 == pbsInfo->m_uiProgress)
	{
		pBSsetting->m_uiScan_Progress_Per = AVL_min(100,((10*(pbsInfo->m_uiNextStartFreq_100kHz - 10*pBSsetting->m_uiScan_Start_Freq_MHz))/(pBSsetting->m_uiScan_Stop_Freq_MHz - pBSsetting->m_uiScan_Start_Freq_MHz)));
		pBSsetting->m_uiScan_Next_Freq_100KHz = pbsInfo->m_uiNextStartFreq_100kHz+340;
		pBSsetting->m_uiScan_Bind_No++;
		pBSsetting->m_uiScaning_Flag = 0;

//		r |= AVL68XX_DVBSx_IBlindScanAPI_Adjust(uiChipNo,pBSsetting);
//		r |= AVL_Demod_DVBSx_SetFunctionalMode(AVL_FuncMode_Demod,uiChipNo);
	}
	if( r != AVL_EC_OK)
		return AVL_EC_GENERAL_FAIL;

	if(100 == pbsInfo->m_uiProgress)
		return AVL_EC_OK;
	else
		return AVL_EC_RUNNING;
}


AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_Adjust(AVL_uint32 uiChipNo,struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting)
{
	
	AVL_ErrorCode r = AVL_EC_OK;	
	AVL_BSInfo * pbsInfo = &pBSsetting->bsInfo;
	AVL_uint16 Indext = pBSsetting->m_uiChannelCount;
	AVL_uint16 i,j,flag;
	AVL_ChannelInfo *pTemp;
	AVL_ChannelInfo *pValid;
	AVL_uint32 uiSymbolRate_Hz;
	AVL_uint32 ui_SR_offset;
	
	if(pbsInfo->m_uiChannelCount>0)
	{
		r |= AVL68XX_Demod_DVBSx_BlindScan_ReadChannelInfo(0, &(pbsInfo->m_uiChannelCount), pBSsetting->channels_Temp, uiChipNo);
	}

	for(i=0; i<pbsInfo->m_uiChannelCount; i++)
	{
		pTemp = &(pBSsetting->channels_Temp[i]);
		flag =0;
		for(j=0; j<pBSsetting->m_uiChannelCount; j++)
		{
			pValid = &(pBSsetting->channels[j]);
			if( AVL_abssub(pValid->m_uiFrequency_kHz,pTemp->m_uiFrequency_kHz) < AVL_min(pValid->m_uiSymbolRate_Hz,pTemp->m_uiSymbolRate_Hz)/2000)
			//if( (AVL_abssub(pValid->m_uiFrequency_kHz,pTemp->m_uiFrequency_kHz)*833) < AVL_min(pValid->m_uiSymbolRate_Hz,pTemp->m_uiSymbolRate_Hz) )
			{
				flag = 1;
				break;
			}				
		}

		if(0 == flag)
		{
			pBSsetting->channels[Indext].m_Flags = pTemp->m_Flags;
			pBSsetting->channels[Indext].m_uiSymbolRate_Hz = pTemp->m_uiSymbolRate_Hz;
			//pBSsetting->channels[Indext].m_uiFrequency_kHz = 1000*((pTemp->m_uiFrequency_kHz+500)/1000);
			pBSsetting->channels[Indext].m_uiFrequency_kHz = pTemp->m_uiFrequency_kHz;


			uiSymbolRate_Hz = pBSsetting->channels[Indext].m_uiSymbolRate_Hz;
			//----------------------------adjust symbol rate offset------------------------------------------------------------
			ui_SR_offset = ((uiSymbolRate_Hz%10000)>5000)?(10000-(uiSymbolRate_Hz%10000)):(uiSymbolRate_Hz%10000);
			if( ((uiSymbolRate_Hz>10000000) && (ui_SR_offset<3500)) || ((uiSymbolRate_Hz>5000000) && (ui_SR_offset<2000)) )
				uiSymbolRate_Hz = (uiSymbolRate_Hz%10000<5000)?(uiSymbolRate_Hz - ui_SR_offset):(uiSymbolRate_Hz + ui_SR_offset);

			ui_SR_offset = ((uiSymbolRate_Hz%1000)>500)?(1000-(uiSymbolRate_Hz%1000)):(uiSymbolRate_Hz%1000);
			if( (uiSymbolRate_Hz<5000000) && (ui_SR_offset< 500) )
				uiSymbolRate_Hz = (uiSymbolRate_Hz%1000<500)?(uiSymbolRate_Hz - ui_SR_offset):(uiSymbolRate_Hz + ui_SR_offset);
	
			pBSsetting->channels[Indext].m_uiSymbolRate_Hz = 1000*(uiSymbolRate_Hz/1000);
			//----------------------------------------------------------------------------------------------------------------
			Indext++;
		}
	}
	
	pBSsetting->m_uiChannelCount = Indext;

	r |= AVL_Demod_DVBSx_SetFunctionalMode(AVL_FuncMode_Demod,uiChipNo);

	return r;
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindScanAPI_Exit(AVL_uint32 uiChipNo, struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting)
{
	AVL_ErrorCode r = AVL_EC_OK;
	AVL_BSInfo * pbsInfo = &pBSsetting->bsInfo;

	if(pBSsetting->m_uiScaning_Flag == 1)
	{
		do
		{
			AVL_IBSP_Delay(50);			
			r |= AVL68XX_DVBSx_IBlindScan_GetScanStatus(pbsInfo, uiChipNo);  //Query the internal blind scan procedure information.
			if(AVL_EC_OK !=r)
			{
				return r;			
			}
		}while(100 != pbsInfo->m_uiProgress);
		pBSsetting->m_uiScaning_Flag = 0;
	}

	r |= AVL_Demod_DVBSx_SetFunctionalMode(AVL_FuncMode_Demod,uiChipNo);
	AVL_IBSP_Delay(10);
	
	return r;
}

AVL_uint16 AVL68XX_DVBSx_IBlindscanAPI_GetProgress(struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting)
{
	return pBSsetting->m_uiScan_Progress_Per;
}

AVL_uint16 AVL68XX_DVBSx_IBlindscanAPI_GetTPCount(struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting)
{
	return pBSsetting->m_uiChannelCount;
}

AVL_ErrorCode AVL68XX_DVBSx_IBlindscanAPI_GetTP(struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting, AVL_uint16 uiStartIndex, AVL_puint16 uiNum,AVL_ChannelInfo *pChannel)
{
	AVL_ErrorCode r = AVL_EC_OK;
	AVL_ChannelInfo *pTemp;
	AVL_uint16 i;

	if(pBSsetting->m_uiChannelCount>=uiStartIndex)
	{
		*uiNum = AVL_min((pBSsetting->m_uiChannelCount - uiStartIndex),*uiNum);
	}
	else
	{
		return AVL_EC_GENERAL_FAIL;
	}
		
	for(i= 0; i< *uiNum; i++)
	{
		pTemp = &pBSsetting->channels[i+uiStartIndex];
		pChannel[i].m_uiFrequency_kHz = pTemp->m_uiFrequency_kHz;
		pChannel[i].m_uiSymbolRate_Hz = pTemp->m_uiSymbolRate_Hz;
		pChannel[i].m_Flags = pTemp->m_Flags;
	}

	return r;
}

