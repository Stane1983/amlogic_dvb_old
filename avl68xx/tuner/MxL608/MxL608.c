
#include "MxL608.h"

AVL_uchar devId_608[2] = {0, 0};

AVL_uint32 MxL608_Initialize(AVL_Tuner *pTuner)
{
    //AVL_uint32 r = 0;
    MXL608_STATUS status;
    MXL608_BOOL singleSupply_3_3V;
    MXL608_XTAL_SET_CFG_T xtalCfg;
    MXL608_IF_OUT_CFG_T ifOutCfg;
    MXL608_AGC_CFG_T agcCfg;
    MXL608_BOOL enableLoopThrough;

	devId_608[pTuner->tunerIndex] = (AVL_uchar)pTuner->usTunerI2CAddr;

    //Step 1 : Soft Reset MxL608
    status = MxLWare608_API_CfgDevSoftReset(devId_608[pTuner->tunerIndex]);
    if (status != MXL608_SUCCESS)
    {
        //printf("Error! MxLWare608_API_CfgDevSoftReset\n");
        return 1;
    }

    //Step 2 : Overwrite Default
    singleSupply_3_3V = MX608_XTAL_SINGLE_SUPPLY_3_3_V_ENABLE;
    status = MxLWare608_API_CfgDevOverwriteDefaults(devId_608[pTuner->tunerIndex], singleSupply_3_3V);
    if (status != MXL608_SUCCESS)
    {
        //printf("Error! MxLWare608_API_CfgDevOverwriteDefaults\n");    
        return 1;
    }
    //Step 3 : XTAL Setting
    xtalCfg.xtalFreqSel = MX608_XTAL_VALUE;
    //xtalCfg.xtalCap = 12;
    xtalCfg.xtalCap = 17;//old
    xtalCfg.clkOutEnable = MX608_XTAL_CLOCK_OUTPUT_ENABLE;
    xtalCfg.clkOutDiv = MXL608_DISABLE;
    xtalCfg.clkOutExt = MXL608_DISABLE;

    xtalCfg.singleSupply_3_3V = MX608_XTAL_SINGLE_SUPPLY_3_3_V_ENABLE;
    
    xtalCfg.XtalSharingMode = MXL608_DISABLE;
    status = MxLWare608_API_CfgDevXtal(devId_608[pTuner->tunerIndex], xtalCfg);

    enableLoopThrough = MX608_LOOP_THROUGH_ENABLE;
   
    MxLWare608_API_CfgTunerLoopThrough(devId_608[pTuner->tunerIndex],enableLoopThrough,NULL,NULL);//ÁéÃô¶ÈÌáÉý1dB
    if (status != MXL608_SUCCESS)
    {
        //printf("Error! MxLWare608_API_CfgDevXtal\n");    
        return 1;
    }

    //Step 4 : IF Out setting
    //ifOutCfg.ifOutFreq = MXL608_IF_4_1MHz;
/*
	if(36000000 == pTuner->uiIFHz)
	{
		ifOutCfg.ifOutFreq = MXL608_IF_36MHz;//old
	}
	else if(36150000 == pTuner->uiIFHz)
	{
       ifOutCfg.ifOutFreq = MXL608_IF_36_15MHz;
	}
*/

		ifOutCfg.ifOutFreq = MXL608_IF_5MHz;
	
    ifOutCfg.ifInversion = MX608_IF_OUT_INVERSION_ENABLE;
    ifOutCfg.gainLevel = 11;
    ifOutCfg.manualFreqSet = MXL608_DISABLE;
    //ifOutCfg.manualIFOutFreqInKHz = 0;
/*
	if(36000000 == pTuner->uiIFHz)
	{
      ifOutCfg.manualIFOutFreqInKHz = 36000; //old
	}
	else if(36150000 == pTuner->uiIFHz)
	{
      ifOutCfg.manualIFOutFreqInKHz = 36150; 
	}

*/
      ifOutCfg.manualIFOutFreqInKHz = 5000; 
	
    status = MxLWare608_API_CfgTunerIFOutParam(devId_608[pTuner->tunerIndex], ifOutCfg);
    if (status != MXL608_SUCCESS)
    {
        //printf("Error! MxLWare608_API_CfgTunerIFOutParam\n");    
        return 1;
    }
    
    //Step 5 : AGC Setting
    agcCfg.agcType = MXL608_AGC_EXTERNAL;
    agcCfg.setPoint = 66;
    agcCfg.agcPolarityInverstion = MXL608_DISABLE;
    status = MxLWare608_API_CfgTunerAGC(devId_608[pTuner->tunerIndex], agcCfg);
    if (status != MXL608_SUCCESS)
    {
        //printf("Error! MxLWare608_API_CfgTunerAGC\n");    
        return 1;
    }
    return 0;
}

AVL_uint32 MxL608_Lock(AVL_Tuner *pTuner)
{
    //AVL_uint32 r = 0;
    MXL608_STATUS status; 
    MXL608_TUNER_MODE_CFG_T tunerModeCfg;
    MXL608_CHAN_TUNE_CFG_T chanTuneCfg;

    //Step 6 : Application Mode setting
	if (pTuner->eDTVMode == DTVMode_DVBC)
    {
      tunerModeCfg.signalMode = MXL608_DIG_DVB_C;
    }
    else if (pTuner->eDTVMode == DTVMode_DVBTX)
    {
      tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
    }
	else if(pTuner->eDTVMode == DTVMode_ISDBT)
	{
       tunerModeCfg.signalMode = MXL608_DIG_ISDBT_ATSC;
	}
    //tunerModeCfg.ifOutFreqinKHz = 4100;
/*
	if(36000000 == pTuner->uiIFHz)
	{
       tunerModeCfg.ifOutFreqinKHz = 36000;//old
	}
    else if(36150000 == pTuner->uiIFHz)
	{
      tunerModeCfg.ifOutFreqinKHz = 36150; 
	}
*/

	tunerModeCfg.ifOutFreqinKHz = 5000;
	
    tunerModeCfg.xtalFreqSel = MX608_XTAL_VALUE;
    tunerModeCfg.ifOutGainLevel = 11;
    status = MxLWare608_API_CfgTunerMode(devId_608[pTuner->tunerIndex], tunerModeCfg);
    if (status != MXL608_SUCCESS)
    {
        return 1;
        //printf("Error! MxLWare608_API_CfgTunerMode\n");    
    }

    if (pTuner->eDTVMode == DTVMode_DVBC)//AVL_DEMOD_MODE_DVBC)
    {
        chanTuneCfg.signalMode = MXL608_DIG_DVB_C;
        if (pTuner->uiBandwidthHz <= 6000000)
        {
            chanTuneCfg.bandWidth = MXL608_CABLE_BW_6MHz;
        }
        else if (pTuner->uiBandwidthHz > 6000000 && pTuner->uiBandwidthHz <= 7000000)
        {
            chanTuneCfg.bandWidth = MXL608_CABLE_BW_7MHz;
        }
        else if (pTuner->uiBandwidthHz > 7000000 && pTuner->uiBandwidthHz <= 8000000)
        {
            chanTuneCfg.bandWidth = MXL608_CABLE_BW_8MHz;
        }
        else
        {
            chanTuneCfg.bandWidth = MXL608_CABLE_BW_8MHz;
        }
    }
    else if (pTuner->eDTVMode == DTVMode_ISDBT)//AVL_DEMOD_MODE_ISDBT)
    {
        chanTuneCfg.signalMode = MXL608_DIG_ISDBT_ATSC;
        if (pTuner->uiBandwidthHz <= 6000000)
        {
            chanTuneCfg.bandWidth = MXL608_TERR_BW_6MHz;
        }
        else if (pTuner->uiBandwidthHz > 6000000 && pTuner->uiBandwidthHz <= 7000000)
        {
            chanTuneCfg.bandWidth = MXL608_TERR_BW_7MHz;
        }
        else if (pTuner->uiBandwidthHz > 7000000 && pTuner->uiBandwidthHz <= 8000000)
        {
            chanTuneCfg.bandWidth = MXL608_TERR_BW_8MHz;
        }
        else
        {
            chanTuneCfg.bandWidth = MXL608_TERR_BW_8MHz;
        }
    }
    else
    {
        chanTuneCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
        if (pTuner->uiBandwidthHz <= 6000000)
        {
            chanTuneCfg.bandWidth = MXL608_TERR_BW_6MHz;
        }
        else if (pTuner->uiBandwidthHz > 6000000 && pTuner->uiBandwidthHz <= 7000000)
        {
            chanTuneCfg.bandWidth = MXL608_TERR_BW_7MHz;
        }
        else if (pTuner->uiBandwidthHz > 7000000 && pTuner->uiBandwidthHz <= 8000000)
        {
            chanTuneCfg.bandWidth = MXL608_TERR_BW_8MHz;
        }
        else
        {
            chanTuneCfg.bandWidth = MXL608_TERR_BW_8MHz;
        }
    }

	chanTuneCfg.freqInHz = pTuner->uiRFFrequencyHz;        //Unit:Hz
    chanTuneCfg.xtalFreqSel = MX608_XTAL_VALUE;
    chanTuneCfg.startTune = MXL608_START_TUNE;
    status = MxLWare608_API_CfgTunerChanTune(devId_608[pTuner->tunerIndex], chanTuneCfg);
    if (status != MXL608_SUCCESS)
    {
        return 1;
        //printf("Error! MxLWare608_API_CfgTunerChanTune\n");    
    }
    MxLWare608_OEM_Sleep(15);//new
    
    return 0;
}

AVL_uint32 MxL608_GetLockStatus(AVL_Tuner *pTuner)
{
    //AVL_uint32 r = 0;
    MXL608_STATUS status; 
    MXL608_BOOL refLockPtr = MXL608_UNLOCKED;
    MXL608_BOOL rfLockPtr = MXL608_UNLOCKED;        

    status = MxLWare608_API_ReqTunerLockStatus(devId_608[pTuner->tunerIndex], &rfLockPtr, &refLockPtr);
    if (status == MXL608_TRUE)
    {
        if (MXL608_LOCKED == rfLockPtr && MXL608_LOCKED == refLockPtr)
        {
            pTuner->ucTunerLocked = 1;
            return 0;
        }
        else
        {
            pTuner->ucTunerLocked = 0;
            return 1;
        }
    }
    return 1;
}

AVL_uint32 MxL608_GetRFStrength(AVL_Tuner *pTuner, AVL_int32 *power)
{
    MXL608_STATUS status; 
    AVL_int16 RSSI = 0;

    status = MxLWare608_API_ReqTunerRxPower(devId_608[pTuner->tunerIndex], &RSSI);
    *power = (AVL_int32)RSSI/100;

    if (status != MXL608_SUCCESS)
    {
        return 1;
    }

    return 0;
}


