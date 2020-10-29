#include "vs_fe_external.h"

static _BOOL I2CRead(struct i2c_adapter *i2c_adap, _U8 i2c_slave_addr, _U8 *data, _S32 length)
{
    /* I2C read, please port this function*/
    _BOOL bRet = _TRUE;
    _S32 s32Ret = 0;

    struct i2c_msg msg;

    if ((_NULL == i2c_adap) || (_NULL == data)) {
        return _FALSE;
    }

    memset(&msg, 0, sizeof(msg));
    msg.addr = i2c_slave_addr;
    msg.flags |=  I2C_M_RD; //write I2C_M_RD=0x01
    msg.buf = data;
    msg.len = length;

    s32Ret = i2c_transfer(i2c_adap, &msg, 1);
    if(s32Ret != 1)
    {
        return _FALSE;
    }

    return bRet;
}

static _BOOL I2CWrite(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 length)
{
    /* I2C write, please port this function*/
    _BOOL bRet = _TRUE;
    _S32 s32Ret = 0;

    struct i2c_msg msg;

    if ((_NULL == i2c_adap) || (_NULL == data)) {
        return _FALSE;
    }

    memset(&msg, 0, sizeof(msg));
    msg.addr = i2c_addr;
    msg.flags = 0; //I2C_M_NOSTART;
    msg.buf = data;
    msg.len = length;

    s32Ret = i2c_transfer(i2c_adap, &msg, 1);
    if (s32Ret < 0) {
        return _FALSE;
    }

    return bRet;
}

static _BOOL I2CWriteNoStop(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 length)
{
    /* I2C write no stop, please port this function*/
    _BOOL bRet = _TRUE;
    _S32 s32Ret = 0;

    struct i2c_msg msg[2];

    if ((_NULL == i2c_adap) || (_NULL == data)) {
        return _FALSE;
    }

    memset(msg, 0, sizeof(msg));
    msg[0].addr = i2c_addr;
    msg[0].flags = 0; //I2C_M_NOSTART;
    msg[0].buf = data;
    msg[0].len = 1;

    msg[1].addr = i2c_addr;
    msg[1].flags = I2C_M_NOSTART; //write I2C_M_RD=0x01
    msg[1].buf = data;
    msg[1].len = length;

    s32Ret = i2c_transfer(i2c_adap, msg, 2);
    if (s32Ret < 0) {
        return _FALSE;
    }

    return bRet;
}

static _BOOL I2CWriteRead(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 write_length, _S32 read_length)
{
    /* I2C write read, please port this function*/
    _BOOL bRet = _TRUE;
    _S32 s32Ret = 0;

    struct i2c_msg msg[2];

    if ((_NULL == i2c_adap) || (_NULL == data)) {
        return _FALSE;
    }

    memset(msg, 0, sizeof(msg));
    msg[0].addr = i2c_addr;
    msg[0].flags = 0; //I2C_M_NOSTART;
    msg[0].buf = data;
    msg[0].len = write_length;

    msg[1].addr = i2c_addr;
    msg[1].flags = I2C_M_RD; //write I2C_M_RD=0x01
    msg[1].buf = data;
    msg[1].len = read_length;

    s32Ret = i2c_transfer(i2c_adap, msg, 2);
    if (s32Ret < 0) {
        return _FALSE;
    }

    return bRet;
}

void External_Delay(_U32 delayByMs)
{
    msleep(delayByMs);
}

_BOOL External_CreateSemaphore(EXTERNAL_SEMAPHORE *semaphore)
{
    sema_init((struct semaphore *)semaphore, 1);
    return _TRUE;
}

_BOOL External_DeleteSemaphore(EXTERNAL_SEMAPHORE semaphore)
{
    return _TRUE;
}

_BOOL External_AcquireSemaphore(EXTERNAL_SEMAPHORE semaphore)
{
#if 0
    if(down_interruptible((struct semaphore *)&semaphore))
        return _TRUE;
    return _FALSE;
#else
    //down((struct semaphore *)&semaphore);
    return _TRUE;
#endif
}

_BOOL External_ReleaseSemaphore(EXTERNAL_SEMAPHORE semaphore)
{
    //up((struct semaphore *)&semaphore);
    return _TRUE;
}
_BOOL External_IIC_Read(struct i2c_adapter *i2c_adap, _U8 i2c_slave_addr, _U8 *data, _S32 length)
{
    return I2CRead(i2c_adap, i2c_slave_addr, data, length);
}

_BOOL External_IIC_Write(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 length)
{
    return I2CWrite(i2c_adap, i2c_addr, data, length);
}

_BOOL External_IIC_WriteNoStop(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 length)
{
    return I2CWriteNoStop(i2c_adap, i2c_addr, data, length);
}

_BOOL External_IIC_WriteRead(struct i2c_adapter *i2c_adap, _U8 i2c_addr, _U8 *data, _S32 write_length, _S32 read_length)
{
    return I2CWriteRead(i2c_adap, i2c_addr, data, write_length, read_length);
}

void External_PIN_Write(_U32 pin, _U8 value)
{
     gpio_direction_output(pin, value);
}

void External_PIN_Invalid(_U32 pin)
{
     gpio_direction_input(pin);
}
