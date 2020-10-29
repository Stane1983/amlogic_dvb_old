/*
 *           Copyright 2007-2014 Availink, Inc.
 *
 *  This software contains Availink proprietary information and
 *  its use and disclosure are restricted solely to the terms in
 *  the corresponding written license agreement. It shall not be 
 *  disclosed to anyone other than valid licensees without
 *  written permission of Availink, Inc.
 *
 */

#include "AVL68XX_Porting.h"

#include "avl68xx_app_porting.h"

#include "vs_fe_debug.h"

#include "R848_API.h"
#include "MxL608.h"

//#include <string.h>

static _BOOL s_bNeedExitLockTuner[2] = {_FALSE, _FALSE};

#ifdef R848_I2C_ADDR_OVERWRITE
#define R848_I2C_ADDR R848_I2C_ADDR_OVERWRITE
#else
#define R848_I2C_ADDR 0x7A
#endif

#ifdef MXL608_I2C_ADDR_OVERWRITE
#define MXL608_I2C_ADDR MXL608_I2C_ADDR_OVERWRITE
#else
#define MXL608_I2C_ADDR 0x60
#endif

static AVL_Tuner global_tuner_t_c[2] = 
{
    {
        0x00,                     //this value will set later
        AVL_STATUS_UNLOCK,

        DTVMode_DVBTX,
        666*1000*1000,
        5*1000*1000,
        8000*1000,
        340*1000*1000,            //LPF setting, not used for T/C tuner

        NULL,

        NULL,                     //tuner initialization function           //this value will set later
        NULL,                     //tuner lock function                     //this value will set later
        NULL,                     //tuner check lock status function        //this value will set later
        NULL,                                                               //this value will set later

        0, //tuner index
    },
    
    {
        (0x00 | AVL68XX_CHIP_1),  //this value will set later
        AVL_STATUS_UNLOCK,

        DTVMode_DVBTX,
        666*1000*1000,
        5*1000*1000,
        8000*1000,
        340*1000*1000,            //LPF setting, not used for T/C tuner

        NULL,

        NULL,                     //tuner initialization function           //this value will set later
        NULL,                     //tuner lock function                     //this value will set later
        NULL,                     //tuner check lock status function        //this value will set later
        NULL,                                                               //this value will set later

        1, //tuner index
    },
};

static AVL_Tuner global_tuner_s[2] = 
{
    {
        //tuner configuration
        0x00,                     //this value will set later
        AVL_STATUS_UNLOCK,

        DTVMode_DVBSX,
        666*1000*1000,
        5000*1000,//IF frequency setting, not used for S tuner
        8000*1000,//BW setting, not used for S tuner
        340*1000*1000,

        NULL,
          
        NULL,                     //tuner initialization function           //this value will set later
        NULL,                     //tuner lock function                     //this value will set later
        NULL,                     //tuner check lock status function        //this value will set later
        NULL,                                                               //this value will set later

        0, //tuner index
    }, 

    {
        //tuner configuration
        (0x00 | AVL68XX_CHIP_1),  //this value will set later
        AVL_STATUS_UNLOCK,

        DTVMode_DVBSX,
        666*1000*1000,
        5000*1000,//IF frequency setting, not used for S tuner
        8000*1000,//BW setting, not used for S tuner
        340*1000*1000,

        NULL,
          
        NULL,                     //tuner initialization function           //this value will set later
        NULL,                     //tuner lock function                     //this value will set later
        NULL,                     //tuner check lock status function        //this value will set later
        NULL,                                                               //this value will set later

        1, //tuner index
    }, 
};

void AVL68XX_SetExitLockTuner(AVL_uint32 uiChipNo, _BOOL bExitLock)
{
    s_bNeedExitLockTuner[uiChipNo] = bExitLock;
}

_BOOL AVL68XX_GetExitLockTuner(AVL_uint32 uiChipNo)
{
    return s_bNeedExitLockTuner[uiChipNo];
}

static void AVL_Check_LockStatus(AVL_uint32 uiChipNo, AVL_uchar *pLockFlag)
{
    AVL_uint16 times_out_cnt = 0;
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_uchar ucLockStatus = 0;
    AVL_uchar ucNosignal = 0;
    AVL_DemodMode eCurrentDemodMode;
	*pLockFlag = 0;

    r = GetMode_Demod(&eCurrentDemodMode,uiChipNo);
    // check the lock status of the demod
    while(times_out_cnt<200 && !AVL68XX_GetExitLockTuner(uiChipNo))//this time-out window can be customized
    {
        AVL_IBSP_Delay(15);
        // No signal Detection
        if(eCurrentDemodMode ==  AVL_DVBTX)
        { 
          
          r |= AVL_Demod_DVBTxSignalDetection(&ucNosignal, uiChipNo);
          if(ucNosignal==0)//ucNosignal=1:signal exist 0:nosignal
          {
             _Print("[AVL_Check_LockStatus] ---- NoSignal Channel! ---- \n");
             break;
          }
        }  
		else if(eCurrentDemodMode ==  AVL_ISDBT)
		{
		   r |= ISDBT_GetSignalDetection(&ucNosignal, uiChipNo);
		   if(ucNosignal == 0)//ucNosignal=1:signal exist 0:nosignal
           {
             _Print("[AVL_Check_LockStatus] ----ISDBT NoSignal Channel! ---- \n");
             break;
           }
		}
		else if(eCurrentDemodMode ==  AVL_DVBC)
		{
			AVL_Demod_DVBCSignalDetection(&ucNosignal, uiChipNo);
		   if(ucNosignal == 0)//ucNosignal=1:signal exist 0:nosignal
           {
             _Print("[AVL_Check_LockStatus] ----ISDBT NoSignal Channel! ---- \n");
             break;
           }
		}
        //get the lock status
        r |= AVL_Demod_GetLockStatus(&ucLockStatus, uiChipNo);
        if(r != AVL_EC_OK)
        {
            _Print("[AVL_Check_LockStatus] AVL_GetLockStatus Failed!\n");
            return;
        }
        if(ucLockStatus == 1)
        {
            _Print("[AVL_Check_LockStatus] --- Channel locked! ---\n");
            *pLockFlag = 1;
            return;
        }
        times_out_cnt++;
    }
    _Print("[AVL_Check_LockStatus] channel unlocked!\n");
}

static AVL_ErrorCode DVB_Sx_tuner_Lock(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint32 Symbol_Khz)
{ 
    AVL_Tuner *pTuner = &global_tuner_s[uiChipNo];
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_uchar tuner_lock_retry = 0;

    pTuner->uiRFFrequencyHz = Freq_Khz*1000;
    pTuner->uiLPFHz = (Symbol_Khz * 1000/200)*135+2000000;///should be set  different
    pTuner->eDTVMode = DTVMode_DVBSX;

    //Open the I2C bus for Sx tuner
    AVL_Demod_I2CBypassOn(uiChipNo);
    
    if(pTuner->fpLockFunc != NULL)
    {
        return_code = pTuner->fpLockFunc(pTuner);
        if(return_code!=AVL_EC_OK)
        {
            _Print("[DVB_Sx_tuner_Lock] Tuner lock function Failed!\n");
			AVL_Demod_I2CBypassOff(uiChipNo);
            return (return_code);
        }
    }
    else
    {
        _Print("[DVB_Sx_tuner_Lock] Tuner lock function is NULL,............WARNING.!\n");
    }
    
    //check the tuner is locked
    if(pTuner->fpGetLockStatusFunc != NULL)
    {
        while(tuner_lock_retry <= 5 && !AVL68XX_GetExitLockTuner(uiChipNo))
        {
            AVL_IBSP_Delay(20);
            return_code = pTuner->fpGetLockStatusFunc(pTuner);
            if(return_code!=AVL_EC_OK)
            {
                _Print("[DVB_Sx_tuner_Lock] Tuner error or I2C error!\n");
                AVL_Demod_I2CBypassOff(uiChipNo);
                return return_code;
            }

            if(pTuner->ucTunerLocked == 1)
            {
                _Print("[DVB_Sx_tuner_Lock] Tuner locked!\n");      
                break;
            }
            else
            {
                _Print("[DVB_Sx_tuner_Lock] Tuner unlock!\n");
                AVL_Demod_I2CBypassOff(uiChipNo);
                return return_code;
            }
        }
    }
    else
    {
        _Print("[DVB_Sx_tuner_Lock] Tuner check status function is NULL,............WARNING.!\n");
    }
    
    //Close the I2C bus to avoid interference
    AVL_Demod_I2CBypassOff(uiChipNo);
    
    return return_code;
}

static AVL_ErrorCode DVB_C_tuner_Lock(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint32 BandWidth_Khz)
{
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_Tuner *pTuner = &global_tuner_t_c[uiChipNo];
    AVL_uchar tuner_lock_retry = 0;

    pTuner->uiRFFrequencyHz = Freq_Khz*1000;
    pTuner->uiBandwidthHz = BandWidth_Khz*1150;///should be set different, the rolloff  is 0.15.
    pTuner->eDTVMode = DTVMode_DVBC;
    
     // Open the I2C bus for Tx/C tuner
     AVL_Demod_I2CBypassOn(uiChipNo);
     
     if(pTuner->fpLockFunc != NULL)
     {
         return_code = pTuner->fpLockFunc(pTuner);
         if(return_code!=AVL_EC_OK)
         {
             _Print("[DVB_C_tuner_Lock] Tuner lock function Failed!\n");
		// Close the I2C bus to avoid interference
		AVL_Demod_I2CBypassOff(uiChipNo);
             return (return_code);
         }
     }
     else
     {
         _Print("[DVB_C_tuner_Lock] Tuner lock function is NULL,............WARRING.!\n");
     }
     
     //check the tuner is locked
     if(pTuner->fpGetLockStatusFunc != NULL)
     {
         while(tuner_lock_retry <= 5 && !AVL68XX_GetExitLockTuner(uiChipNo))
         {
             AVL_IBSP_Delay(20);
             return_code = pTuner->fpGetLockStatusFunc(pTuner);
             if(return_code != AVL_EC_OK)
             {
                 _Print("[DVB_C_tuner_Lock] Tuner error or I2C error!\n");
                 AVL_Demod_I2CBypassOff(uiChipNo);
                 return return_code;
             }
    
             if(pTuner->ucTunerLocked == 1)
             {
                 _Print("[DVB_C_tuner_Lock] Tuner locked!\n");
                 break;
             }
             else
             {
                 _Print("[DVB_C_tuner_Lock] Tuner unlock!\n");
                 AVL_Demod_I2CBypassOff(uiChipNo);
                 return return_code;
             }
         }
     }
     else
     {
         _Print("[DVB_C_tuner_Lock] Tuner check status function is NULL,............WARRING.!\n");
     }
     
     // Close the I2C bus to avoid interference
     AVL_Demod_I2CBypassOff(uiChipNo);
     return return_code;
}

static AVL_ErrorCode DVB_Tx_tuner_Lock(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint16 BandWidth_Khz)
{
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_Tuner *pTuner = &global_tuner_t_c[uiChipNo];
    AVL_uchar tuner_lock_retry = 0;
    
    pTuner->uiRFFrequencyHz = Freq_Khz*1000;
    pTuner->uiBandwidthHz = BandWidth_Khz*1000;
    pTuner->eDTVMode = DTVMode_DVBTX;
    
    // Open the I2C bus for Tx/C tuner
    AVL_Demod_I2CBypassOn(uiChipNo);
    
    if(pTuner->fpLockFunc != NULL)
    {
        return_code = pTuner->fpLockFunc(pTuner);
        if(return_code != AVL_EC_OK)
        {
            _Print("[DVB_Tx_tuner_Lock] Tuner lock function Failed!\n");
		// Close the I2C bus to avoid interference
		AVL_Demod_I2CBypassOff(uiChipNo);
            return (return_code);
        }
    }
    else
    {
        _Print("[DVB_Tx_tuner_Lock] Tuner lock function is NULL,............WARNING.!\n");
    }
    
    //check the tuner is locked
    if(pTuner->fpGetLockStatusFunc != NULL)
    {
        while(tuner_lock_retry <= 5 && !AVL68XX_GetExitLockTuner(uiChipNo))
        {
             AVL_IBSP_Delay(10);
            return_code = pTuner->fpGetLockStatusFunc(pTuner);
            if(return_code!=AVL_EC_OK)
            {
                _Print("[DVB_Tx_tuner_Lock] Tuner error or I2C error!\n");
                AVL_Demod_I2CBypassOff(uiChipNo);
                return return_code;
            }
    
            if(pTuner->ucTunerLocked == 1)
            {
                 _Print("[DVB_Tx_tuner_Lock] Tuner locked!\n");
                break;
            }
            else
            {
                tuner_lock_retry++;
                _Print("[DVB_Tx_tuner_Lock] Tuner unlock!\n");
            }
             //AVL_Demod_I2CBypassOff(uiChipNo);
        }
    }
    else
    {
        _Print("[DVB_Tx_tuner_Lock] Tuner check status function is NULL,............WARNING.!\n");
    }
    
    // Close the I2C bus to avoid interference
    AVL_Demod_I2CBypassOff(uiChipNo);
    
    return(return_code);
}

static AVL_ErrorCode ISDBT_tuner_Lock(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint16 BandWidth_Khz)
{
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_Tuner *pTuner = &global_tuner_t_c[uiChipNo];
    AVL_uchar tuner_lock_retry = 0;
    
    pTuner->uiRFFrequencyHz = Freq_Khz*1000;
    pTuner->uiBandwidthHz = BandWidth_Khz*1000;
    pTuner->eDTVMode = DTVMode_ISDBT;
    
    // Open the I2C bus for Tx/C tuner
    AVL_Demod_I2CBypassOn(uiChipNo);
    
    if(pTuner->fpLockFunc != NULL)
    {
        return_code = pTuner->fpLockFunc(pTuner);
        if(return_code != AVL_EC_OK)
        {
            _Print("[ISDBT_tuner_Lock] Tuner lock function Failed!\n");
		// Close the I2C bus to avoid interference
		AVL_Demod_I2CBypassOff(uiChipNo);
            return (return_code);
        }
    }
    else
    {
        _Print("[ISDBT_tuner_Lock] Tuner lock function is NULL,............WARNING.!\n");
    }
    
    //check the tuner is locked
    if(pTuner->fpGetLockStatusFunc != NULL)
    {
        while(tuner_lock_retry <= 5 && !AVL68XX_GetExitLockTuner(uiChipNo))
        {
             AVL_IBSP_Delay(10);
            return_code = pTuner->fpGetLockStatusFunc(pTuner);
            if(return_code!=AVL_EC_OK)
            {
                _Print("[ISDBT_tuner_Lock] Tuner error or I2C error!\n");
                AVL_Demod_I2CBypassOff(uiChipNo);
                return return_code;
            }
    
            if(pTuner->ucTunerLocked == 1)
            {
                 _Print("[ISDBT_tuner_Lock] Tuner locked!\n");
                break;
            }
            else
            {
                tuner_lock_retry++;
                _Print("[ISDBT_tuner_Lock] Tuner unlock!\n");
            }
             AVL_Demod_I2CBypassOff(uiChipNo);
        }
    }
    else
    {
        _Print("[ISDBT_tuner_Lock] Tuner check status function is NULL,............WARNING.!\n");
    }
    
    // Close the I2C bus to avoid interference
    AVL_Demod_I2CBypassOff(uiChipNo);
    
    return(return_code);
}

_U32 AVL_Init(AVL_uint32 uiChipNo, _U32 *pChipId)
{
    AVL_ErrorCode return_code = AVL_EC_OK;

    AVL_uint32 uiChipID = 0;    
    AVL_DemodMode eStartupMode = AVL_DVBTX;
    _U32 systemType = 0, extendSystemType = 0;
    _BOOL bDVBS = _FALSE;

    return_code = AVL_Demod_GetChipID(&uiChipID,uiChipNo);
    if(return_code == AVL_EC_OK)
	{
		if(uiChipID == AVL6882_CHIP_ID) uiChipID = 6882;
		else if(uiChipID == AVL6862_CHIP_ID) uiChipID = 6862;
		else if(uiChipID == AVL6812_CHIP_ID) uiChipID = 6812;
		else if(uiChipID == AVL6762_CHIP_ID) uiChipID = 6762;
        _Print("[GetChipId] chip id:AVL%d\n",uiChipID);
	}
    else{
        _Print("[GetChipId] get chip id Fail.\n");  
        return 0; 
    }

    switch(uiChipID){
        case 6882:
            eStartupMode = AVL_ISDBT;
            extendSystemType = (AVL68XX_SYSTEM_TYPE_DVB_C | AVL68XX_SYSTEM_TYPE_DVB_T | AVL68XX_SYSTEM_TYPE_ISDB_T);
            bDVBS = _TRUE;
            break;

        case 6812:
            eStartupMode = AVL_ISDBT;
            extendSystemType = (AVL68XX_SYSTEM_TYPE_DVB_C | AVL68XX_SYSTEM_TYPE_ISDB_T);
            break;

        case 6762:
            eStartupMode = AVL_DVBTX;
            extendSystemType = (AVL68XX_SYSTEM_TYPE_DVB_C | AVL68XX_SYSTEM_TYPE_DVB_T);
            break;

        case 6862:
            eStartupMode = AVL_DVBTX;
            extendSystemType = (AVL68XX_SYSTEM_TYPE_DVB_C | AVL68XX_SYSTEM_TYPE_DVB_T);
            bDVBS = _TRUE;
            break;

        default:
            _Print("[AVL_Init] unsupportted chip id\n");
            return 0;
    }

    if((0 == extendSystemType) && (_TRUE != bDVBS))
    {
        _Print("[AVL_Init] unvalid type\n");
        return 0;
    }

    return_code = AVL_Demod_Initialize(eStartupMode, uiChipNo);
    if(return_code != AVL_EC_OK)
    {
        _Print("[AVL_Init] AVL_Demod_Initialize Failed!\n");
        return 0;
    }
    else
    {
        _Print("[AVL_Init] AVL_Demod_Initialize Booted!\n");
    }

    // Open the I2C bus for tuner
    AVL_Demod_I2CBypassOn(uiChipNo);
    if(_TRUE == bDVBS)
    {
        global_tuner_t_c[uiChipNo].usTunerI2CAddr = R848_I2C_ADDR;
        if(uiChipNo == 1)
            global_tuner_t_c[uiChipNo].usTunerI2CAddr |= AVL68XX_CHIP_1;

        return_code = R848_Initialize(&global_tuner_t_c[uiChipNo]);
        if(return_code==AVL_EC_OK)
        {
            global_tuner_t_c[uiChipNo].fpInitializeFunc = R848_Initialize;
            global_tuner_t_c[uiChipNo].fpLockFunc = R848_Lock;
            global_tuner_t_c[uiChipNo].fpGetLockStatusFunc = R848_GetLockStatus;
            global_tuner_t_c[uiChipNo].fpGetRFStrength = _NULL;

            systemType |= extendSystemType;
            _Print("[AVL_Init] T/C Tuner Init OK [R848]!\n");
        }
        else
        {
            _Print("[AVL_Init] T/C Tuner Init Failed! [R848] \n");
        }
    }
    else
    {
        global_tuner_t_c[uiChipNo].usTunerI2CAddr = MXL608_I2C_ADDR;
        if(uiChipNo == 1)
            global_tuner_t_c[uiChipNo].usTunerI2CAddr |= AVL68XX_CHIP_1;
    
        return_code = MxL608_Initialize(&global_tuner_t_c[uiChipNo]);
        if(return_code==AVL_EC_OK)
        {
            global_tuner_t_c[uiChipNo].fpInitializeFunc = MxL608_Initialize;
            global_tuner_t_c[uiChipNo].fpLockFunc = MxL608_Lock;
            global_tuner_t_c[uiChipNo].fpGetLockStatusFunc = MxL608_GetLockStatus;
            global_tuner_t_c[uiChipNo].fpGetRFStrength = MxL608_GetRFStrength;

            _Print("[AVL_Init] T/C Tuner Init OK [MXL608]!\n");
            systemType |= extendSystemType;
        }
        else
        {
            _Print("[AVL_Init] T/C Tuner Init Failed! [MXL608]\n");
        }
    }
    // Close the I2C bus to avoid interference
    AVL_Demod_I2CBypassOff(uiChipNo);

    //if Sx tuner isn't initialized
    if(_TRUE == bDVBS)
    {
        // Open the I2C bus for tuner
        AVL_Demod_I2CBypassOn(uiChipNo);
        global_tuner_s[uiChipNo].usTunerI2CAddr = R848_I2C_ADDR;
        if(uiChipNo == 1)
            global_tuner_s[uiChipNo].usTunerI2CAddr |= AVL68XX_CHIP_1;

        return_code = R848_Initialize(&global_tuner_s[uiChipNo]);
        if(return_code==AVL_EC_OK)
        {
            global_tuner_s[uiChipNo].fpInitializeFunc = R848_Initialize;
            global_tuner_s[uiChipNo].fpLockFunc = R848_Lock;
            global_tuner_s[uiChipNo].fpGetLockStatusFunc = R848_GetLockStatus;
            global_tuner_s[uiChipNo].fpGetRFStrength = _NULL;

            systemType |= AVL68XX_SYSTEM_TYPE_DVB_S;
            _Print("[AVL_Init] S Tuner Init OK [R848]!\n");
        }
        // Close the I2C bus to avoid interference
        AVL_Demod_I2CBypassOff(uiChipNo);
    }   

    _Print("[AVL_Init] ok\n");

    *pChipId = uiChipID;
    return systemType;
}

AVL_ErrorCode AVL_LockChannel_DVBSx(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint32 Symbol_Khz) 
{
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_DemodMode eCurrentDemodMode;

    GetMode_Demod(&eCurrentDemodMode,uiChipNo);
    if(eCurrentDemodMode != AVL_DVBSX)
    {
        _Print("[AVL_LockChannel_DVBSx] demod mode is not DVB-Sx,Err.\n");
        return AVL_EC_GENERAL_FAIL;
    }

    _Print("[AVL_LockChannel_DVBSx] Freq:%d Mhz,sym:%d Khz\n",Freq_Khz/1000,Symbol_Khz);

    DVB_Sx_tuner_Lock(uiChipNo, Freq_Khz, Symbol_Khz);
    
    return_code = AVL_Demod_DVBSxAutoLock(Symbol_Khz*1000, uiChipNo);
    if(return_code != AVL_EC_OK)
    {
        _Print("[AVL_LockChannel_DVBSx] Failed to lock the channel!\n");
        return return_code;
    }

    return return_code;
}

AVL_ErrorCode AVL_LockChannel_ISDBT(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz, AVL_uint16 BandWidth_Khz)
{
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_DemodMode eCurrentDemodMode;

    GetMode_Demod(&eCurrentDemodMode,uiChipNo);
    if(eCurrentDemodMode != AVL_ISDBT)
    {
        _Print("[AVL_LockChannel_ISDBT] demod mode is not ISDBT,Err.\n");
        return AVL_EC_GENERAL_FAIL;
    }

    _Print("[AVL_LockChannel_ISDBT] Freq:%d Mhz,sym:%d Khz\n",Freq_Khz/1000,BandWidth_Khz);
    
    ISDBT_tuner_Lock(uiChipNo,Freq_Khz, BandWidth_Khz);
    
    return_code = AVL_Demod_ISDBTAutoLock(uiChipNo);
    if(return_code != AVL_EC_OK)
    {
        _Print("[AVL_LockChannel_ISDBT] Failed to lock the channel!\n");
        return return_code;
    }

    return return_code; 
}

static AVL_DVBTxBandWidth Convert2DemodBand(AVL_uint16 BandWidth_Khz)
{
    AVL_DVBTxBandWidth nBand = AVL_DVBTx_BW_8M;

    if(BandWidth_Khz == 1700)
     {
         nBand = AVL_DVBTx_BW_1M7;
     }
     else if(BandWidth_Khz == 5000)
     {
         nBand = AVL_DVBTx_BW_5M;
     }
     else if(BandWidth_Khz == 6000)
     {
         nBand = AVL_DVBTx_BW_6M;
     }
     else if(BandWidth_Khz == 7000)
     {
         nBand = AVL_DVBTx_BW_7M;
     }
     else if(BandWidth_Khz == 8000)
     {
         nBand = AVL_DVBTx_BW_8M;
     }
     else
     {
         _Print("[Convert2DemodBand] BandWidth_Khz is Err\n");
         nBand = AVL_DVBTx_BW_8M;
     }
     return nBand;
}

AVL_ErrorCode AVL_LockChannel_T(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint16 BandWidth_Khz, AVL_int32 DVBT_layer_info)
{
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_DVBTxBandWidth nBand = AVL_DVBTx_BW_8M;
    AVL_DemodMode eCurrentDemodMode;

    if(Freq_Khz < 100)
        Freq_Khz = 100; //don't set too small, otherwise will cause demod drivers i2c repeater always running, and can not accept new tune params
    GetMode_Demod(&eCurrentDemodMode,uiChipNo);
    if(eCurrentDemodMode != AVL_DVBTX)
    {
        _Print("[AVL_LockChannel_T] demod mode is not DVB-Tx,Err.\n");
        return AVL_EC_GENERAL_FAIL;
    }
    
    return_code = DVB_Tx_tuner_Lock(uiChipNo,Freq_Khz,BandWidth_Khz);
    
    _Print("[AVL_LockChannel_T] Freq is %d MHz, Bandwide is %d MHz, Layer Info is %d (0 : LP; 1 : HP)\n",
                       Freq_Khz/1000, BandWidth_Khz/1000, DVBT_layer_info);

    nBand = Convert2DemodBand(BandWidth_Khz);

    return_code = AVL_Demod_DVBTAutoLock(nBand, DVBT_layer_info, uiChipNo);
    
    if(return_code != AVL_EC_OK)
    {
        _Print("[AVL_LockChannel_T] Failed to lock the channel!\n");
    }

    return return_code;
}

AVL_ErrorCode AVL_LockChannel_T2(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint16 BandWidth_Khz, AVL_uchar T2_Profile, AVL_int32 PLP_ID)
{
    
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_DVBTxBandWidth nBand = AVL_DVBTx_BW_8M;
    AVL_DemodMode eCurrentDemodMode;
    if(Freq_Khz < 100)
        Freq_Khz = 100; //don't set too small, otherwise will cause demod drivers i2c repeater always running, and can not accept new tune params

    GetMode_Demod(&eCurrentDemodMode,uiChipNo);
    if(eCurrentDemodMode != AVL_DVBTX)
    {
        _Print("[AVL_LockChannel_T2] demod mode is not DVB-Tx,Err.\n");
        return AVL_EC_GENERAL_FAIL;
    }

    return_code = DVB_Tx_tuner_Lock(uiChipNo,Freq_Khz,BandWidth_Khz);

    _Print("[AVL_LockChannel_T2] Freq is %d MHz, Bandwide is %d MHz, DATA PLP ID is %d \n",
                       Freq_Khz/1000, BandWidth_Khz/1000, PLP_ID);

    nBand = Convert2DemodBand(BandWidth_Khz);

    return_code = AVL_Demod_DVBT2AutoLock(nBand, (AVL_DVBT2_PROFILE)T2_Profile, PLP_ID, uiChipNo);

    if(return_code != AVL_EC_OK)
    {
        _Print("[AVL_LockChannel_T2] Failed to lock the channel!\n");
    }
    
    return return_code;
}

AVL_ErrorCode AVL_LockChannel_DVBC(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint32 symbolRate)
{
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_DemodMode eCurrentDemodMode;
    if(Freq_Khz < 100)
        Freq_Khz = 100; //don't set too small, otherwise will cause demod drivers i2c repeater always running, and can not accept new tune params
    if(symbolRate < 100)
        symbolRate = 100; //don't set too small, otherwise will cause demod drivers i2c repeater always running, and can not accept new tune params

    GetMode_Demod(&eCurrentDemodMode,uiChipNo);
    if(eCurrentDemodMode !=  AVL_DVBC)
    {
        _Print("[AVL_LockChannel_C] demod mode is not DVB-C,Err.\n");
        return AVL_EC_GENERAL_FAIL;
    }

    DVB_C_tuner_Lock(uiChipNo, Freq_Khz, symbolRate);

    return_code = DVBC_SetSymbolRate_Demod(symbolRate*1000, uiChipNo);
    if(return_code!=AVL_EC_OK)
    {
        _Print("[AVL_LockChannel_C] Failed to set symbolrate = %d\n", symbolRate*1000);
        return return_code;
    }
    
    return_code = AVL_Demod_DVBCAutoLock(uiChipNo);
//	AVL_Demod_DVBCManualLock (5217*1000,AVL_DVBC_256QAM, uiChipNo);
    if(return_code!=AVL_EC_OK)
    {
        _Print("[AVL_LockChannel_C] Failed to lock the channel!\n");
        return return_code;
    }

    return return_code; 
}


s_DVBTx_Channel_TS global_channel_ts_table[MAX_CHANNEL_INFO];
AVL_uint16 g_nChannel_ts_total = 0;

AVL_ErrorCode AVL_SetWorkMode(AVL_uint32 uiChipNo, AVL_DemodMode eDemodWorkMode)
{
    AVL_ErrorCode return_code = AVL_EC_OK;

    return_code = AVL_Demod_SetMode(eDemodWorkMode, uiChipNo);

    if(return_code != AVL_EC_OK)
    {
        _Print("Failed to set work mode!\n");
        return (return_code);
    }
       
    return (return_code);
}

AVL_ErrorCode AVL_ScanChannel_Tx(AVL_uint32 uiChipNo, AVL_uint32 Freq_Khz,AVL_uint16 BandWidth_Khz)
{
    AVL_ErrorCode return_code = AVL_EC_OK;
    AVL_DVBTxScanInfo stDVBTxScanInfo;
    AVL_DVBTxBandWidth nBand = AVL_DVBTx_BW_8M;
    AVL_DemodMode eCurrentDemodMode;
    AVL_uint16 cur_index = 0;
    AVL_uchar ucLockFlag = 0;
    AVL_DVBT2_PROFILE ucT2Profile = AVL_DVBT2_PROFILE_UNKNOWN;
    AVL_uchar ucTemp = 0;
    AVL_uchar ucDataPLPArray[255] = {0};
    AVL_uchar ucDataPLPNumber = 0;
    if(Freq_Khz < 100)
        Freq_Khz = 100; //don't set too small, otherwise will cause demod drivers i2c repeater always running, and can not accept new tune params

    g_nChannel_ts_total = 0;

    GetMode_Demod(&eCurrentDemodMode,uiChipNo);
    if(eCurrentDemodMode != AVL_DVBTX)
    {
        _Print("[AVL_ChannelScan_Tx] demod mode is not DVB-Tx,Err.\n");
        return AVL_EC_GENERAL_FAIL;     
    }
    //=====Tuner Lock=====//
    _Print("[AVL_ChannelScan_Tx] Lock Tuner : \n===  Freq is %d MHz \n===  Bandwide is %d MHz \n",
                       Freq_Khz/1000, BandWidth_Khz/1000);
    return_code = DVB_Tx_tuner_Lock(uiChipNo, Freq_Khz, BandWidth_Khz);

    nBand = Convert2DemodBand(BandWidth_Khz);
    
    //=====Demod Lock=====//
    return_code = AVL_Demod_DVBTxChannelScan(nBand, AVL_DVBTx_LockMode_ALL, uiChipNo);
    _Print("[AVL_ChannelScan_Tx] Freq is %d MHz, Bandwide is %d MHz \n",
                       Freq_Khz/1000, BandWidth_Khz/1000);
    //=====Check Lock Status =====//   
    AVL_Check_LockStatus(uiChipNo, &ucLockFlag);

    if(ucLockFlag == 1 && !AVL68XX_GetExitLockTuner(uiChipNo))//DVBTx is locked
    {
        _Print("[AVL_ChannelScan_Tx] Freq is %d MHz, Bandwide is %d MHz \n",
                       Freq_Khz/1000, BandWidth_Khz/1000);
        return_code |= AVL_Demod_DVBTxGetScanInfo(&stDVBTxScanInfo, uiChipNo);
    
        if(stDVBTxScanInfo.eTxStandard == AVL_DVBTx_Standard_T2)//get PLP ID list only for DVBT2 signal, not for DVBT
        {
            cur_index = 0;
            return_code = AVL_Demod_DVBT2GetPLPList(ucDataPLPArray, &ucDataPLPNumber, uiChipNo);

            for (ucTemp = 0; ucTemp < ucDataPLPNumber; ucTemp++)
            {
                _Print("[DVB-T2_Scan_Info] DATA PLP ID is %d, profile = %d\n",ucDataPLPArray[ucTemp], stDVBTxScanInfo.ucTxInfo); 

                //save channel RF frequency
                global_channel_ts_table[cur_index].channel_freq_khz = Freq_Khz;
                // save channel bandwidth
                global_channel_ts_table[cur_index].channel_bandwith_khz = BandWidth_Khz;
                // save data plp id
                global_channel_ts_table[cur_index].data_plp_id = ucDataPLPArray[ucTemp];
                // 0 - DVBT; 1 - DVBT2.
                global_channel_ts_table[cur_index].channel_type = AVL_DVBTx_Standard_T2;
                // 0 - Base profile; 1 - Lite profile.
                global_channel_ts_table[cur_index].channel_profile = (AVL_DVBT2_PROFILE)stDVBTxScanInfo.ucTxInfo;

                cur_index++;
            }
            g_nChannel_ts_total = cur_index%MAX_CHANNEL_INFO;
            
            
            if (stDVBTxScanInfo.ucFEFInfo == 1)
            {  
                ucT2Profile = (AVL_DVBT2_PROFILE) stDVBTxScanInfo.ucTxInfo;
                
                if (ucT2Profile == AVL_DVBT2_PROFILE_BASE)//profile is base
                {         
                    //If T2 base is locked, try to lock T2 lite 
                    AVL_Demod_DVBTxChannelScan(nBand, AVL_DVBTx_LockMode_T2LITE, uiChipNo);
                    ucT2Profile = AVL_DVBT2_PROFILE_LITE;
                }
                else
                {         
                    //If T2 lite is locked, try to lock T2 base 
                    AVL_Demod_DVBTxChannelScan(nBand, AVL_DVBTx_LockMode_T2BASE, uiChipNo);                    
                    ucT2Profile = AVL_DVBT2_PROFILE_BASE;
                }
                AVL_Check_LockStatus(uiChipNo, &ucLockFlag);
                if(ucLockFlag == 1)//DVBTx is locked
                { 
//                    cur_index = g_nChannel_ts_total;
                    ucDataPLPNumber = 0;
                    return_code = AVL_Demod_DVBT2GetPLPList(ucDataPLPArray, &ucDataPLPNumber, uiChipNo);

                    // data PLP ID and common PLP ID pairing
                    for (ucTemp = 0; ucTemp < ucDataPLPNumber; ucTemp++)
                    {
                        _Print("[DVB-T2_Scan_Info] DATA PLP ID is %d, profile = %d\n",ucDataPLPArray[ucTemp], ucT2Profile); 

                        //save channel RF frequency
                        global_channel_ts_table[cur_index].channel_freq_khz = Freq_Khz;
                        // save channel bandwidth
                        global_channel_ts_table[cur_index].channel_bandwith_khz = BandWidth_Khz;
                        // save data plp id
                        global_channel_ts_table[cur_index].data_plp_id = ucDataPLPArray[ucTemp];
                        // 0 - DVBT; 1 - DVBT2.
                        global_channel_ts_table[cur_index].channel_type = AVL_DVBTx_Standard_T2;
                        // 0 - Base profile; 1 - Lite profile.
                        global_channel_ts_table[cur_index].channel_profile = ucT2Profile;

                        cur_index++;
                    }
                    g_nChannel_ts_total = cur_index%MAX_CHANNEL_INFO;
                }
            }
            else
            {
                _Print("No FEF \n");
            }
        }
        else // DVBT
        {
            cur_index = 0;
            // save channel RF frequency
            global_channel_ts_table[cur_index].channel_freq_khz = Freq_Khz;
            // save channel bandwidth
            global_channel_ts_table[cur_index].channel_bandwith_khz = BandWidth_Khz;
            // save data plp id(not used for DVBT, set to 0xff)
            global_channel_ts_table[cur_index].data_plp_id = 0;
            // 0 - DVBT; 1 - DVBT2.
            global_channel_ts_table[cur_index].channel_type = AVL_DVBTx_Standard_T;
            // 0 - Low priority layer, 1 - High priority layer
            global_channel_ts_table[cur_index].dvbt_hierarchy_layer = 1; 
            cur_index++;

            if(stDVBTxScanInfo.ucTxInfo == 1)// for hierarchy
            {
                // save channel RF frequency
                global_channel_ts_table[cur_index].channel_freq_khz = Freq_Khz;
                // save channel bandwidth
                global_channel_ts_table[cur_index].channel_bandwith_khz = BandWidth_Khz;
                // save data plp id(not used for DVBT, set to 0xff)
                global_channel_ts_table[cur_index].data_plp_id = 0;
                // 0 - DVBT; 1 - DVBT2.
                global_channel_ts_table[cur_index].channel_type = AVL_DVBTx_Standard_T;
                // 0 - Low priority layer, 1 - High priority layer
                global_channel_ts_table[cur_index].dvbt_hierarchy_layer = 0; 
                
                cur_index++;
            }
            g_nChannel_ts_total = cur_index%MAX_CHANNEL_INFO;
        }
        
    }
    else // return for unlock
    {
        _Print("[DVBTx_Channel_ScanLock_Example] DVBTx channel scan is fail,Err.\n");
    }
    return return_code; 
}

#ifdef MAKE_DVB_TX_RF_POWER_SMOOTH
#define DVB_TX_RF_POWER_LOW -107
#define DVB_TX_RF_POWER_HIGH 10
#endif

static AVL_uchar AVL_R848_get_RFPower_info(AVL_uint32 uiChipNo)
{
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_Tuner    *pTuner;
    AVL_uint32 uiRFGain;
	AVL_uint16 usSSI;
	AVL_int16  uiRFPower=0;
	AVL_int16  iRFPower=0;
    AVL_DemodMode current_mode;
    AVL_uchar ucSSI = 0;// range 0 - 100  

    r = GetMode_Demod(&current_mode,uiChipNo);
    switch(current_mode)
    {
        case AVL_DVBTX:
            pTuner = &global_tuner_t_c[uiChipNo];
			break;
		case AVL_DVBSX:
            pTuner = &global_tuner_s[uiChipNo];
			break;
        case AVL_ISDBT:
            pTuner = &global_tuner_t_c[uiChipNo];
			break;
        case AVL_DVBC: 
            _Print("can not calculate the RF Power\n");  
			return r;
			break;
		default:
            pTuner = &global_tuner_t_c[uiChipNo];
            break;
		
	}
    /******** get RFPower *******/
    
	AVL_Demod_I2CBypassOn(uiChipNo);
	R848_GetGain(pTuner, &uiRFGain);//get gain
	AVL_Demod_I2CBypassOff(uiChipNo);
	AVL_Demod_GetSSI(&usSSI,uiChipNo);// get SSI					
	uiRFPower=R848_RFPowerCalc(uiRFGain,usSSI);//Calc Power
//    _Print("The RF Power is %d dBm\n",uiRFPower); 
	iRFPower=uiRFPower;
#ifdef MAKE_DVB_TX_RF_POWER_SMOOTH
    if(current_mode == AVL_DVBTX){
        if(iRFPower < DVB_TX_RF_POWER_LOW)
            iRFPower = DVB_TX_RF_POWER_LOW;
        else if(iRFPower > DVB_TX_RF_POWER_HIGH)    
            iRFPower = DVB_TX_RF_POWER_HIGH;
        iRFPower -= DVB_TX_RF_POWER_LOW;
        ucSSI = iRFPower*100/(DVB_TX_RF_POWER_HIGH-DVB_TX_RF_POWER_LOW);
        if(ucSSI > 100)
            ucSSI = 100;
    }
    else
#endif    
    {
    AVL_Demod_DVBTxGetNorDigSSI(&ucSSI,iRFPower,uiChipNo);
	}
	
//	_Print("SSI is %d\n",ucSSI);

    return ucSSI;
}

AVL_uchar adjust_value(AVL_uchar uiValue)
{
    if ((90 <= uiValue) || (uiValue <= 10))
    {
        //
    }
    else if ((80 <= uiValue) || (uiValue <= 20))
    {
        uiValue += 5;
    }
    else if ((70 <= uiValue) || (uiValue <= 30))
    {
        uiValue += 10;
    }
    else
    {
        uiValue += 20;
    }

    uiValue = (uiValue >= 100) ? 99 : uiValue;

    return uiValue;
}

AVL_uchar get_SSI_info(AVL_uint32 uiChipNo)
{
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_Tuner *pTuner;
    AVL_uint16 signal_strength = 0;// range 0 - 65535
    AVL_uchar ucSSI = 0;// range 0 - 100    
    AVL_int32  iRFPower = -100;//dBm
    AVL_DemodMode current_mode;

    r = GetMode_Demod(&current_mode,uiChipNo);

    if(r == AVL_EC_OK){
        switch(current_mode)
        {
            case AVL_DVBTX:
                pTuner = &global_tuner_t_c[uiChipNo];
                if(pTuner->fpGetRFStrength != NULL)
                {
                    r = AVL_Demod_I2CBypassOn(uiChipNo);//before tuner control, the tuner I2C control path should be enabled
                    pTuner->fpGetRFStrength(pTuner,&iRFPower);
                    r |= AVL_Demod_I2CBypassOff(uiChipNo);//before the control is done, the tuner I2C control path should be disabled to avoid interference towards tuner
#ifdef MAKE_DVB_TX_RF_POWER_SMOOTH
                    if(iRFPower < DVB_TX_RF_POWER_LOW)
                        iRFPower = DVB_TX_RF_POWER_LOW;
                    else if(iRFPower > DVB_TX_RF_POWER_HIGH)    
                        iRFPower = DVB_TX_RF_POWER_HIGH;
                        
                    iRFPower -= DVB_TX_RF_POWER_LOW;
                        
                    ucSSI = iRFPower*100/(DVB_TX_RF_POWER_HIGH-DVB_TX_RF_POWER_LOW);
                    if(ucSSI > 100)
                        ucSSI = 100;
#else
                    r |= AVL_Demod_DVBTxGetNorDigSSI(&ucSSI,iRFPower,uiChipNo);
#endif
                    if(r != AVL_EC_OK)    
                        ucSSI = 0;
                }
                else
                {
                    //default set to R848
                    ucSSI = AVL_R848_get_RFPower_info(uiChipNo);
//                    _Print("Tuner doesn't support RF signal estimation with unit dBm. Please contact with Availink for precise signal strength indicator.");
                }
                //the range for returned signal_strength is 0 to 100
//                _Print("SSI is %d\n", ucSSI);
                break;
            case AVL_DVBSX:
            case AVL_ISDBT:
            case AVL_DVBC: 
                /* the range for returned signal_strength is 0 to 100 */
                r |= AVL_Demod_GetSSI(&signal_strength, uiChipNo);
                if(r == AVL_EC_OK)
                    ucSSI = signal_strength*100/65535;
//                _Print("SSI is %d\n",ucSSI);
                break;
            default:
                break;
        }
    }

    ucSSI = adjust_value(ucSSI);

    return ucSSI;
}

AVL_uchar get_SQI_info(AVL_uint32 uiChipNo)
{
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_uint16 signal_quality = 0;

    /******** get SQI *******/
    /* the range for returned signal_quality is 0 to 100 */
    r = AVL_Demod_GetSQI(&signal_quality, uiChipNo);
    if(r != AVL_EC_OK)    
        signal_quality = 0;

    signal_quality = adjust_value(signal_quality);

//    _Print("SQI is %d\n",signal_quality);

    return signal_quality;
}

AVL_uint32 get_SNR_info(AVL_uint32 uiChipNo)
{
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_uint32 snr_dbx100 = 0;//value: 100 times as actual SNR
    
    /******** get SNR *******/
    /* the returned value of snr_dbx100 is 100 times as actual SNR */
    r = AVL_Demod_GetSNR(&snr_dbx100, uiChipNo);
    if(r != AVL_EC_OK)    
        snr_dbx100 = 0;
        
//    _Print("SNR is %f\n",(float)snr_dbx100/100);
    
    return snr_dbx100;
}

AVL_uint32 get_PER_info(AVL_uint32 uiChipNo)
{
    AVL_ErrorCode r = AVL_EC_OK;
    AVL_uint32 per_1e9 = 0;
    
    /******** get PER *******/
    /*the returned value of per_1e9 is 1e9 times as actual PER*/
    r = AVL_Demod_GetPER(&per_1e9, uiChipNo);
    if(r != AVL_EC_OK)    
        per_1e9 = 0;
        
//    _Print("PER is %f\n",(float)per_1e9/1e9);
    
    return per_1e9;
}

///just for debug
void AVL_PrintVersion(AVL_uint32 uiChipNo)
{
    AVL_DemodVersion sVerInfo;

    AVL_Demod_GetVersion(&sVerInfo,uiChipNo);
    _Print("SDK Version,Major-Minor-Bulid:%d-%d-%d\n",\
    sVerInfo.stAPI.ucMajor,sVerInfo.stAPI.ucMinor,sVerInfo.stAPI.usBuild);

    _Print("Patch Version,Major-Minor-Bulid:%d-%d-%d\n",\
    sVerInfo.stPatch.ucMajor,sVerInfo.stPatch.ucMinor,sVerInfo.stPatch.usBuild);
}

static _BlindScanCallback s_pBlindScanCallback_Loop = _NULL;
static _BOOL s_bInBlindScan_Loop = _FALSE;
static _U32 s_BlindScan_begin_freqMHz,s_BlindScan_end_freqMHz;
static _BOOL s_bNeedExitBlindScan_Loop;

#ifdef SAT_SCR
_BOOL	g_68xx_use_unicable_device			 = _FALSE;

static _U8	g_bank_select				 = 1;		/*Bank select, 0~7*/
static _U8	g_ub_select					 = 0;		/*User band select, 0~7*/
_U16	g_68xx_ub_freq_MHz				 = 1210;	/*User band center freq MHz*/
#endif

#include "AVL68XX_IBlindscanAPI.h"

static void _mt_blindScanCallback_Loop(_U8 tunerIndex, _BOOL bLocked, _U32 freq, _U32 symbolRate, _U8 process)
{
	BLIND_SCAN_CALLBACK_INFO info;

	if(s_pBlindScanCallback_Loop != _NULL){
        info.u8TunerIndex = tunerIndex;
        info.u8Process = process;
        info.bLocked = bLocked;
        info.u32Frequency = freq / 1000;
        info.u32Symbolrate = symbolRate;
		info.bFrontendAttrInfoValid = _FALSE;

		s_pBlindScanCallback_Loop(&info);
	}	
}

static AVL_ErrorCode _BlindScan(AVL_uint32 uiChipNo)
{
	AVL_ErrorCode r = AVL_EC_OK;
	AVL_uint16	index = 0;
	AVL_ChannelInfo * pChannel;
	AVL_Tuner * pTuner = &global_tuner_s[uiChipNo];
	struct AVL68XX_DVBSx_BlindScanAPI_Setting BSsetting;
	enum AVL68XX_DVBSx_BlindScanAPI_Status BS_Status;
	struct AVL68XX_DVBSx_BlindScanAPI_Setting * pBSsetting = &BSsetting;
	int errorCount = 0;
	
	BS_Status = AVL68XX_DVBSx_BS_Status_Init;

	while(BS_Status != AVL68XX_DVBSx_BS_Status_Exit)
	{
		switch(BS_Status)
		{
    		case AVL68XX_DVBSx_BS_Status_Init:		
    		{
    			//Set demod work mode to DVB-SX
    			AVL_SetWorkMode(uiChipNo, AVL_DVBSX);

    			AVL68XX_DVBSx_IBlindScanAPI_Initialize(pBSsetting);//this function set the parameters blind scan process needed.	
    			AVL68XX_DVBSx_IBlindScanAPI_SetFreqRange(pBSsetting, s_BlindScan_begin_freqMHz, s_BlindScan_end_freqMHz); //Default scan rang is from 950 to 2150. User may call this function to change scan frequency rang.
    			AVL68XX_DVBSx_IBlindScanAPI_SetScanMode(pBSsetting, AVL68XX_DVBSx_BS_Fast_Mode);

#ifdef SAT_SCR
    			if(g_68xx_use_unicable_device)
    			{
    				AVL68XX_DVBSx_IBlindScanAPI_SetSpectrumMode(pBSsetting, AVL_Spectrum_Invert); //Default set is AVL_DVBSx_Spectrum_Normal, it must be set correctly according Board HW configuration
    			}
    			else
#endif					
#if MAIN_SPECTRUM_INVERT		
    			AVL68XX_DVBSx_IBlindScanAPI_SetSpectrumMode(pBSsetting, AVL_Spectrum_Invert); 
#else
    			AVL68XX_DVBSx_IBlindScanAPI_SetSpectrumMode(pBSsetting, AVL_Spectrum_Normal); 
#endif
    			//AVL68XX_DVBSx_IBlindScanAPI_SetMaxLPF(pBSsetting, 340); //Set Tuner max LPF value, this value will difference according tuner type

    			BS_Status = AVL68XX_DVBSx_BS_Status_Start;
    			errorCount = 0;
    			break;
    		}

		    case AVL68XX_DVBSx_BS_Status_Start:		
			{			
				r = AVL68XX_DVBSx_IBlindScanAPI_Start(uiChipNo, pTuner, pBSsetting);
				if(AVL_EC_OK != r)
				{
					if(errorCount++ < 5){
					    _Print("AVL68XX_DVBSx_BS_Status_Start : failed, restart...r = %d\r\n", r);
						BS_Status = AVL68XX_DVBSx_BS_Status_Start;
					}
					else{	
					    _Print("AVL68XX_DVBSx_BS_Status_Start : failed, ...r = %d, quit scan\r\n", r);
						BS_Status = AVL68XX_DVBSx_BS_Status_Exit;
					}
				}
				else{
				    _mt_blindScanCallback_Loop(uiChipNo, _FALSE, pTuner->uiRFFrequencyHz, 0, AVL68XX_DVBSx_IBlindscanAPI_GetProgress(pBSsetting));
					BS_Status = AVL68XX_DVBSx_BS_Status_Wait;
				}

				if(s_bNeedExitBlindScan_Loop){
					BS_Status = AVL68XX_DVBSx_BS_Status_Exit;
					break;
				}
				
				break;
			}

    		case AVL68XX_DVBSx_BS_Status_Wait: 		
    		{
    			r = AVL68XX_DVBSx_IBlindScanAPI_GetCurrentScanStatus(uiChipNo, pBSsetting);
    			if(AVL_EC_GENERAL_FAIL == r)
    			{
    				BS_Status = AVL68XX_DVBSx_BS_Status_Exit;
    			    _Print("AVL_DVBSx_IBlindScanAPI_GetCurrentScanStatus : failed, r = %d\r\n", r);
    			}
    			if(AVL_EC_OK == r)
    			{
    				BS_Status =AVL68XX_DVBSx_BS_Status_Adjust;
    			//	_Print("AVL_DVBSx_IBlindScanAPI_GetCurrentScanStatus : ok, r = %d\r\n", r);
    			}

    			if(AVL_EC_RUNNING == r)
    			{
    				AVL_IBSP_Delay(100);
    			//												    _Print("AVL_DVBSx_IBlindScanAPI_GetCurrentScanStatus : running, r = %d\r\n", r);
    			}

    			if(s_bNeedExitBlindScan_Loop){
    			//											        BS_Status = AVL_DVBSx_BS_Status_Adjust;
    			    BS_Status = AVL68XX_DVBSx_BS_Status_Cancel;
    			    _Print("AVL_DVBSx_IBlindScanAPI_GetCurrentScanStatus : user quit...\r\n");
    			}

    			break;
    		}

    		case AVL68XX_DVBSx_BS_Status_Adjust:	
    		{
    			r = AVL68XX_DVBSx_IBlindScanAPI_Adjust(uiChipNo, pBSsetting);
    			if(AVL_EC_OK != r)
    			{
    				BS_Status = AVL68XX_DVBSx_BS_Status_Exit;
    			    _Print("AVL_DVBSx_IBlindScanAPI_Adjust : failed, r = %d\r\n", r);
    			}
    			BS_Status = AVL68XX_DVBSx_BS_Status_User_Process;
    			break;
    		}

		    case AVL68XX_DVBSx_BS_Status_User_Process:	
    		{
    			//------------Custom code start-------------------
    			//customer can add the callback function here such as adding TP information to TP list or lock the TP for parsing PSI
    			//Add custom code here; Following code is an example

    			/*----- example 1: print Blindscan progress ----*/
    			_Print(" %2d%% \n", AVL68XX_DVBSx_IBlindscanAPI_GetProgress(pBSsetting)); //display progress Percent of blindscan process

    			/*----- example 2: print TP information if found valid TP ----*/
    			if(index < pBSsetting->m_uiChannelCount){
    				while(index < pBSsetting->m_uiChannelCount) //display new TP info found in current stage
    				{
    					pChannel = &pBSsetting->channels[index++];
    					_Print("      Ch%2d: RF: %4d SR: %5d ",index, (pChannel->m_uiFrequency_kHz/1000),(pChannel->m_uiSymbolRate_Hz/1000));

    					_mt_blindScanCallback_Loop(uiChipNo, _FALSE, pChannel->m_uiFrequency_kHz*1000, pChannel->m_uiSymbolRate_Hz / 1000, AVL68XX_DVBSx_IBlindscanAPI_GetProgress(pBSsetting));
                        {
    						AVL_uchar uiLockStatus = 0;
    						AVL_ChannelInfo channel;

    						channel.m_uiSymbolRate_Hz = pChannel->m_uiSymbolRate_Hz;

    					#ifdef SAT_SCR
    						if(g_68xx_use_unicable_device){
    							//_U16 real_freq_MHz;
    							//SCR_ODU_ChannelChange(uiChipNo, pChannel->m_uiFrequency_kHz / 1000, &real_freq_MHz);

    							channel.m_uiFrequency_kHz = g_68xx_ub_freq_MHz * 10;
    						}
    						else
    					#endif
    						{
    							channel.m_uiFrequency_kHz = pChannel->m_uiFrequency_kHz;
    						}
    						
    						//AVL6862_Lock(channel,&uiLockStatus);
    						AVL_LockChannel_DVBSx(uiChipNo, channel.m_uiFrequency_kHz,channel.m_uiSymbolRate_Hz/1000);
    						AVL_Check_LockStatus(uiChipNo, &uiLockStatus);

    						//r = LockSignal_WaitLock(uiChipNo, 3000);

    						if(uiLockStatus){
    							_Print("Locked!");
    							_mt_blindScanCallback_Loop(uiChipNo, _TRUE, pChannel->m_uiFrequency_kHz*1000, pChannel->m_uiSymbolRate_Hz / 1000, AVL68XX_DVBSx_IBlindscanAPI_GetProgress(pBSsetting));
    						}															
    						else	{	
    							_Print("Unlock!");
    						}

    						_Print("\n");
                        }
    				}				
    			}
    			else{
    				_mt_blindScanCallback_Loop(uiChipNo, _FALSE, 0, 0, AVL68XX_DVBSx_IBlindscanAPI_GetProgress(pBSsetting));
    			}
    		
    			index = pBSsetting->m_uiChannelCount;

    			/*----- example 3: Break blindscan process when check key pressed ----*/
    			#if 0
    			if ( _kbhit() ) // demonstrate blindscan exit while process status machine is running
    			{
    				if( _getch() == 'e' )
    				{
    					_Print("Exit by user.\n");
    					BS_Status = Blindscan_Status_Cancel;
    					break;
    				}
    			}
    			#endif
    			if(s_bNeedExitBlindScan_Loop){
    				BS_Status = AVL68XX_DVBSx_BS_Status_Cancel;
    				break;
    			}
    				
    			//------------Custom code end -------------------

    			if ( (AVL68XX_DVBSx_IBlindscanAPI_GetProgress(pBSsetting) < 100)){
    			    errorCount = 0;
    				BS_Status = AVL68XX_DVBSx_BS_Status_Start;
    			    _Print("AVL_DVBSx_IBlindScan : progress[%d] next\r\n",AVL68XX_DVBSx_IBlindscanAPI_GetProgress(pBSsetting));
    		    }
    			else{											
    				BS_Status = AVL68XX_DVBSx_BS_Status_Cancel;
    			    _Print("AVL_DVBSx_IBlindScan : progress[%d] end, quit\r\n",AVL68XX_DVBSx_IBlindscanAPI_GetProgress(pBSsetting));
    		    }
    			break;
    		}

    	    case AVL68XX_DVBSx_BS_Status_Cancel:
    		{ 
    			_Print("AVL_DVBSx_IBlindScan : AVL_DVBSx_BS_Status_Cancel\r\n");
    			r = AVL68XX_DVBSx_IBlindScanAPI_Exit(uiChipNo,pBSsetting);
    			BS_Status = AVL68XX_DVBSx_BS_Status_Exit;
    			break;
    		}

    	    default:
    		{
    		    _Print("AVL_DVBSx_IBlindScan : unknown BS_Status = %d... quit scan\r\n",BS_Status);
    			BS_Status = AVL68XX_DVBSx_BS_Status_Cancel;
    			break;
    		}
		}
	}

	//print all of the TP info found in blindscan process. this isn't necessary for the customer
	//AVL_Display_TP_Info(pBSsetting->channels,pBSsetting->m_uiChannelCount);

	//-------------------------Blindscan Band Process End------------------------
    r |= AVL_Demod_DVBSx_SetFunctionalMode(AVL_FuncMode_Demod,uiChipNo);

	return (r);
}

void AVL68XX_BlindScanStartLoop(AVL_uint32 uiChipNo, _BlindScanCallback pCallback,PBLIND_SCAN_PARAM_INFO pScanParams)
{
	s_pBlindScanCallback_Loop = pCallback;

    AVL68XX_SetExitLockTuner(uiChipNo, _FALSE);	
    
	s_bInBlindScan_Loop = _TRUE;
	s_bNeedExitBlindScan_Loop = _FALSE;

	s_BlindScan_begin_freqMHz = pScanParams->stBlindScanParam.minfrequency / 1000;
	s_BlindScan_end_freqMHz  = pScanParams->stBlindScanParam.maxfrequency / 1000;

	_Print("\r\n\r\n avl_BlindScan_Start_Loop ::: Start.. =%d ,%d \r\n\r\n",s_BlindScan_begin_freqMHz,s_BlindScan_end_freqMHz);

#ifdef SAT_SCR
	if(pScanParams->bScrOn){
		g_68xx_use_unicable_device = _TRUE;
		g_ub_select = pScanParams->u32ScrNumber;
		g_68xx_ub_freq_MHz = pScanParams->u32ScrBandPassFrequency;
		g_bank_select = pScanParams->u32ScrLnbNumber;
	}
	else
	{
		g_68xx_use_unicable_device = _FALSE;
	}
#endif

	_BlindScan(uiChipNo);

#ifdef SAT_SCR
	g_68xx_use_unicable_device = _FALSE;
#endif
	
	_Print("\r\n\r\n avl_BlindScan_Start_Loop ::: End..\r\n\r\n");
	
	if(s_pBlindScanCallback_Loop != _NULL){
		s_pBlindScanCallback_Loop(_NULL);
	}

	s_bInBlindScan_Loop = _FALSE;
}

void AVL68XX_BlindScanStopLoop(AVL_uint32 uiChipNo)
{
	//AVL_ErrorCode r = AVL_EC_OK;
	_Print("\r\n\r\n avl_BlindScan_Stop_Loop ::: Start..\r\n\r\n");
	
	if(s_bInBlindScan_Loop){
	    AVL68XX_SetExitLockTuner(uiChipNo, _TRUE);
		s_bNeedExitBlindScan_Loop = _TRUE;

		while(s_bInBlindScan_Loop){
			External_Delay(100);
		}

		s_bNeedExitBlindScan_Loop = _FALSE;
	}
	
	_Print("\r\n\r\n avl_BlindScan_Stop_Loop ::: OK..\r\n\r\n");
/*
	r = AVL68XX_DVBSx_IBlindScan_Cancel(uiChipNo);
	if(AVL_EC_OK != r)
	{
		_Print("\r\n\r\n avl_BlindScan_Stop_Loop ::: Success..\r\n\r\n");
	}
	else
	{
		_Print("\r\n\r\n avl_BlindScan_Stop_Loop ::: Fail..\r\n\r\n");
	}
*/
}
