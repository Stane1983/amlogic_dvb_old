/****************************************************************************
* MONTAGE PROPRIETARY AND CONFIDENTIAL
* Montage Technology (Shanghai) Inc.
* All Rights Reserved
* --------------------------------------------------------------------------
*
* File:				mt_fe_i2c.h
*
* Current version:	1.00.00
*
* Description:		i2c function prototype for FE module.
*
* Log:	Description			Version			Date			Author
*		---------------------------------------------------------------------
*		Create				1.00.00			2010.09.15		YZ.Huang
*		Modify				1.00.00			2010.09.15		YZ.Huang
****************************************************************************/
#ifndef __MT_DEMOD_I2C_H__
#define __MT_DEMOD_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mt_fe_def.h"

extern void _mt_sleep(U32 ms);
extern MT_FE_RET _mt_fe_dmd_set_reg(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 data);
extern MT_FE_RET _mt_fe_dmd_get_reg(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf);
extern MT_FE_RET _mt_fe_tn_set_reg(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 data);
extern MT_FE_RET _mt_fe_tn_get_reg(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf);
extern MT_FE_RET _mt_fe_tn_write(MT_FE_RS6060_Device_Handle handle, U8 *p_buf, U16 n_byte);
extern MT_FE_RET _mt_fe_write_fw(MT_FE_RS6060_Device_Handle handle, U8 reg_index, U8 *p_buf, U16 n_byte);

#ifdef __cplusplus
}
#endif

#endif /* __MT_DEMOD_I2C_H__ */


