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


#include "AVL_Demod.h"
#include "user_defined_function.h"

#include "vs_fe_debug.h"

extern PVS_FE_CTRL g_apstCtrl[SYSTEM_TUNER_NUMBER];

AVL_uint32 AVL_IBSP_Delay(AVL_uint32 uiDelay_ms)
{
    External_Delay(uiDelay_ms);
    
    return(AVL_EC_OK);
}

AVL_uint32 AVL_IBSP_I2C_Read(AVL_uint16 usSlaveAddr,  AVL_puchar pucBuff, AVL_puint16 pusSize)
{
	_BOOL ret;
	struct i2c_adapter *i2c_adap;

	if ( *pusSize == 0 ) //No action
	{
		_Print("AVL_IBSP_I2C_Read error ");    
		return AVL_EC_OK;
	}

	if((usSlaveAddr & AVL68XX_CHIP_1) == AVL68XX_CHIP_1){
	    i2c_adap = g_apstCtrl[1]->pstI2cAdap;
	}
	else{
	    i2c_adap = g_apstCtrl[0]->pstI2cAdap;
	}
	
    usSlaveAddr &= 0x007F;
	
    ret = External_IIC_Read(i2c_adap, usSlaveAddr, pucBuff, *pusSize);
	
	if( !ret )   
      	_Print("error : [%s]  , puiSize - %d end    0x%02X\n\n", __func__, *pusSize, usSlaveAddr);    

    return ret ?  AVL_EC_OK : AVL_EC_I2C_FAIL;
}

AVL_uint32 AVL_IBSP_I2C_Write(AVL_uint16 usSlaveAddr,  AVL_puchar pucBuff,  AVL_puint16  pusSize)
{
	_BOOL ret;
	struct i2c_adapter *i2c_adap;

	if ( *pusSize == 0 ) //No action
	{
		_Print("AVL_IBSP_I2C_Write error ");
		return AVL_EC_OK;
	}

	if((usSlaveAddr & AVL68XX_CHIP_1) == AVL68XX_CHIP_1){
	    i2c_adap = g_apstCtrl[1]->pstI2cAdap;
	}
	else{
	    i2c_adap = g_apstCtrl[0]->pstI2cAdap;
	}
	
    usSlaveAddr &= 0x007F;

    ret = External_IIC_Write(i2c_adap, usSlaveAddr, pucBuff, *pusSize);
	
	if( !ret )   
      	_Print("error : [%s]  , puiSize - %d end    0x%02X\n\n", __func__, *pusSize, usSlaveAddr);    

    return ret ?  AVL_EC_OK : AVL_EC_I2C_FAIL;
}

AVL_uint32 AVL_IBSP_Initialize(AVL_uint32 uiChipNo)
{
    return(AVL_EC_OK);
}

AVL_uint32 AVL_IBSP_InitSemaphore(AVL_psemaphore pSemaphore)
{
	EXTERNAL_SEMAPHORE semphoreId;
	
	if( !External_CreateSemaphore(&semphoreId) )	
		return AVL_EC_GENERAL_FAIL;

	
	*pSemaphore = (AVL_semaphore)semphoreId;
		
    return(AVL_EC_OK);
}

AVL_uint32 AVL_IBSP_ReleaseSemaphore(AVL_psemaphore pSemaphore)
{
	if(External_ReleaseSemaphore((EXTERNAL_SEMAPHORE)((unsigned long)*pSemaphore)))
		return(AVL_EC_OK);
	else
		return(AVL_EC_GENERAL_FAIL);
}

AVL_uint32 AVL_IBSP_WaitSemaphore(AVL_psemaphore pSemaphore)
{
	if(External_AcquireSemaphore((EXTERNAL_SEMAPHORE)((unsigned long)*pSemaphore)))
		return(AVL_EC_OK);
	else
		return(AVL_EC_GENERAL_FAIL);
}

AVL_uint32 AVL_IBSP_Dispose(AVL_uint32 uiChipNo)
{
    return(AVL_EC_OK);
}


