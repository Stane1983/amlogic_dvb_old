//////////////////////////////////////////////////////////////////////////
/////
// TH DTV MIDDLE WARE DEFINES, PLEASE DO NOT MAKE ANY CHANGE OF THIS FILE
/////
//////////////////////////////////////////////////////////////////////////

#ifndef __VS_FE_EXTERNAL_H__
#define __VS_FE_EXTERNAL_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <linux/string.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/amlogic/aml_gpio_consumer.h>

typedef unsigned char _U8;
typedef unsigned short _U16;
typedef unsigned int _U32;
typedef unsigned long _U32L;
typedef unsigned long long int _U64;
typedef long _S64;
typedef char _S8;
typedef short _S16;
typedef int _S32;
typedef float _FLOAT;
typedef double _DOUBLE;

#define _SUCCESS 0
#define _FAILURE (-1)

typedef enum
{
    _FALSE = 0,
    _TRUE = 1
} _BOOL;

#ifndef _UNUSED
#define _UNUSED(x) ((x)=(x))
#endif

#ifndef _NULL
#ifndef NULL
#define NULL ((void *)0)
#endif
#define _NULL NULL
#endif

#define INVALID_VALUE  (0xFFFFFFFF)

typedef void* EXTERNAL_SEMAPHORE;
//typedef struct semaphore *EXTERNAL_SEMAPHORE

//以下中间件提供的函数接口，供external模块调用
void External_Delay(_U32 delayByMs);

_BOOL External_CreateSemaphore(EXTERNAL_SEMAPHORE *semaphore);
_BOOL External_DeleteSemaphore(EXTERNAL_SEMAPHORE semaphore);
_BOOL External_AcquireSemaphore(EXTERNAL_SEMAPHORE semaphore);
_BOOL External_ReleaseSemaphore(EXTERNAL_SEMAPHORE semaphore);

//i2c_addr : 7 bits addr
_BOOL External_IIC_Read(struct i2c_adapter *i2c_adap, _U8 i2c_slave_addr, _U8 *data, _S32 length);
_BOOL External_IIC_Write(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 length);
_BOOL External_IIC_WriteNoStop(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 length);
_BOOL External_IIC_WriteRead(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 write_length, _S32 read_length);

void External_PIN_Write(_U32 pin, _U8 value);
void External_PIN_Invalid(_U32 pin);

#ifdef __cplusplus
}
#endif

#endif    // __VS_FE_EXTERNAL_H__

//////////////////////////////////////////////////////////////////////////
/////
// TH DTV MIDDLE WARE DEFINES, PLEASE DO NOT MAKE ANY CHANGE OF THIS FILE
/////
//////////////////////////////////////////////////////////////////////////
