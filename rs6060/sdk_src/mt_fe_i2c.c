/****************************************************************************
* MONTAGE PROPRIETARY AND CONFIDENTIAL
* Montage Technology (Shanghai) Inc.
* All Rights Reserved
* --------------------------------------------------------------------------
*
* File:				mt_fe_i2c.c
*
* Current version:	1.00.00
*
* Description:		Define all i2c function for FE module.
*
* Log:	Description			Version		Date		Author
*		---------------------------------------------------------------------
*		Create				1.00.00		2010.09.15	YZ.Huang
*		Modify				1.00.00		2010.09.15	YZ.Huang
****************************************************************************/
#include "mt_fe_def.h"

#include "vs_fe_external.h"
#include "vs_fe_tuner.h"

extern PVS_FE_CTRL g_apstCtrl[SYSTEM_TUNER_NUMBER];

void _mt_sleep(U32 ms)
{
	External_Delay(ms);
}

/*****************************************************************
** Function: _mt_fe_dmd_set_reg
**
**
** Description:	write data to demod register
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	reg_index		U8			register index
**	data			U8			value to write
**
**
** Outputs:
**
**
*****************************************************************/
MT_FE_RET _mt_fe_dmd_set_reg(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 data)
{
	_BOOL bRet = _TRUE;

	struct i2c_adapter *i2c_adap;
	unsigned char i2c_addr = 0;
    unsigned char buf[2] = {0};

	if(NULL == handle)
	{
		return MtFeErr_NullPointer;
	}

	/*
		TODO:
			Obtain the i2c mutex
	*/



	/*
		TODO:
			write data to demodulator register
	*/
	buf[0] = reg_index;
	buf[1] = data;

    if(0 == handle->dev_index){
	    i2c_adap = g_apstCtrl[0]->pstI2cAdap;
	}
	else{
	    i2c_adap = g_apstCtrl[1]->pstI2cAdap;
	}
	i2c_addr = handle->demod_dev_addr;

	bRet = External_IIC_Write(i2c_adap, i2c_addr, buf, 2);

	/*
		TODO:
			Release the i2c mutex
	*/



	return (_TRUE == bRet) ? MtFeErr_Ok : MtFeErr_Fail;
}


/*****************************************************************
** Function: _mt_fe_dmd_get_reg
**
**
** Description:	read data from demod register
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	reg_index		U8			register index
**
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	p_buf			U8*			register data
**
**
*****************************************************************/
MT_FE_RET _mt_fe_dmd_get_reg(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf)
{
	_BOOL bRet = _TRUE;

	struct i2c_adapter *i2c_adap;
	unsigned char i2c_addr = 0;

	if((NULL == handle) || (NULL == p_buf))
	{
		return MtFeErr_NullPointer;
	}

	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*
		TODO:
			read demodulator register value
	*/
	*p_buf = reg_index;

	if(0 == handle->dev_index){
	    i2c_adap = g_apstCtrl[0]->pstI2cAdap;
	}
	else{
	    i2c_adap = g_apstCtrl[1]->pstI2cAdap;
	}
	i2c_addr = handle->demod_dev_addr;

	bRet = External_IIC_WriteRead(i2c_adap, i2c_addr, p_buf, 1, 1);

	/*
		TODO:
			Release the i2c mutex
	*/


	return (_TRUE == bRet) ? MtFeErr_Ok : MtFeErr_Fail;
}


/*****************************************************************
** Function: _mt_fe_tn_set_reg
**
**
** Description:	write data to tuner register
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	reg_index		U8			register index
**	data			U8			value to write
**
**
** Outputs:
**
**
*****************************************************************/
MT_FE_RET _mt_fe_tn_set_reg(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 data)
{
	U8 val, ret;

	_BOOL bRet = _TRUE;

	struct i2c_adapter *i2c_adap;
	unsigned char i2c_addr = 0;
    unsigned char buf[2] = {0};

	if(NULL == handle)
	{
		return MtFeErr_NullPointer;
	}

	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*open I2C repeater*/
	/*Do not care to close the I2C repeater, it will close by itself*/
	val = 0x11;
	ret = _mt_fe_dmd_set_reg(handle, 0x03, val);
	if (ret != MtFeErr_Ok)
		return ret;


	/*Do not sleep any time after I2C repeater is opened.*/
	/*please set tuner register at once.*/


	/*
		TODO:
			write value to tuner register
	*/
	buf[0] = reg_index;
	buf[1] = data;

	if(0 == handle->dev_index){
	    i2c_adap = g_apstCtrl[0]->pstI2cAdap;
	}
	else{
	    i2c_adap = g_apstCtrl[1]->pstI2cAdap;
	}
	i2c_addr = handle->tuner_cfg.tuner_dev_addr;

	bRet = External_IIC_Write(i2c_adap, i2c_addr, buf, 2);

	/*
		TODO:
			Release the i2c mutex
	*/



	return (_TRUE == bRet) ? MtFeErr_Ok : MtFeErr_Fail;
}


/*****************************************************************
** Function: _mt_fe_tn_get_reg
**
**
** Description:	get tuner register data
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	register		U8			register address
**
**
** Outputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	p_buf			U8*			register data
**
**
*****************************************************************/
MT_FE_RET _mt_fe_tn_get_reg(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf)
{
	U8 val, ret;

	_BOOL bRet = _TRUE;

	struct i2c_adapter *i2c_adap;
	unsigned char i2c_addr = 0;

	if((NULL == handle) || (NULL == p_buf))
	{
		return MtFeErr_NullPointer;
	}

	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*open I2C repeater*/
	/*Do not care to close the I2C repeater, it will close by itself*/
	val = 0x11;
		// IMPORTANT:
		// This value can be 0x11 or 0x12.
		// It depends on the sum of I2C_STOP flags in a whole I2CRead operation flow.
		// 0x11 means there's only ONE I2C_STOP flag.
		// 0x12 means there're two I2C_STOP flags.
		// Please refer to the application notes for detail descriptions
	ret = _mt_fe_dmd_set_reg(handle, 0x03, val);
	if (ret != MtFeErr_Ok)
		return ret;

	/*Do not sleep any time after I2C repeater is opened.*/
	/*please read tuner register at once.*/


	/*
		TODO:
			read tuner register value
	*/
	*p_buf = reg_index;

	if(0 == handle->dev_index){
	    i2c_adap = g_apstCtrl[0]->pstI2cAdap;
	}
	else{
	    i2c_adap = g_apstCtrl[1]->pstI2cAdap;
	}
	i2c_addr = handle->tuner_cfg.tuner_dev_addr;

	bRet = External_IIC_WriteRead(i2c_adap, i2c_addr, p_buf, 1, 1);

	/*
		TODO:
			Release the i2c mutex
	*/


	return (_TRUE == bRet) ? MtFeErr_Ok : MtFeErr_Fail;
}


//Unused
MT_FE_RET _mt_fe_tn_write(MT_FE_RS6060_Device_Handle handle, U8 *p_buf, U16 n_byte)
{
	U8	val, ret;

	/*
		TODO:
			Obtain the i2c mutex
	*/


	/*open I2C repeater*/
	/*Do not care to close the I2C repeater, it will close by itself*/
	val = 0x11;
	ret = _mt_fe_dmd_set_reg(handle, 0x03, val);
	if (ret != MtFeErr_Ok)
		return ret;

	/*Do not sleep any time after I2C repeater is opened.*/
	/*please write N bytes to register at once.*/


	/*
		TODO:
			write N bytes to tuner
	*/



	/*
		TODO:
			Release the i2c mutex
	*/



	return MtFeErr_Ok;
}


/*****************************************************************
** Function: _mt_fe_iic_write
**
**
** Description:	write n bytes via iic to device
**
**
** Inputs:
**
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	p_buf			U8*			pointer of the tuner data
**	n_byte			U16			the data length
**
**
** Outputs:		none
**
**
**
*****************************************************************/
MT_FE_RET _mt_fe_write_fw(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf, U16 n_byte)
{
	_BOOL bRet = _TRUE;

	struct i2c_adapter *i2c_adap;
	unsigned char i2c_addr = 0;
    unsigned char buf[65] = {0};

	if((NULL == handle) || (NULL == p_buf) || (0 == n_byte) || (64 < n_byte))
	{
		return MtFeErr_NullPointer;
	}

	/*
		TODO:
			Obtain the i2c mutex
	*/




	/*
		TODO:
			write n bytes to demodulator
	*/
	buf[0] = reg_index;
    memcpy(&buf[1], p_buf, n_byte);

	if(0 == handle->dev_index){
	    i2c_adap = g_apstCtrl[0]->pstI2cAdap;
	}
	else{
	    i2c_adap = g_apstCtrl[1]->pstI2cAdap;
	}
	i2c_addr = handle->demod_dev_addr;

	bRet = External_IIC_Write(i2c_adap, i2c_addr, buf, n_byte + 1);

	/*
		TODO:
			Release the i2c mutex
	*/

	return (_TRUE == bRet) ? MtFeErr_Ok : MtFeErr_Fail;
}



