#include "AVL_Tuner.h"

#define R848_TRUE	0
#define R848_FALSE	1
#define BOOL	_BOOL

typedef enum _R848_ErrCode
{
	RT_Success = 1,
	RT_Fail    = 0
}R848_ErrCode;


//----------------------------------------------------------//
//                   Type Define                                    //
//----------------------------------------------------------//
#ifndef UINT8_DEFINED
typedef unsigned char UINT8;
#define UINT8_DEFINED
#endif
#ifndef UINT16_DEFINED
typedef unsigned short UINT16;
#define UINT16_DEFINED
#endif
#ifndef UINT32_DEFINED
typedef unsigned int UINT32;
#define UINT32_DEFINED
#endif
//#define UINT8  unsigned char
//#define UINT16 unsigned short
//#define UINT32 unsigned long


typedef struct _R848_I2C_LEN_TYPE
{
	UINT8 RegAddr;
	UINT8 Data[50];
	UINT8 Len;
}I2C_LEN_TYPE;

typedef struct _R848_I2C_TYPE
{
	UINT8 RegAddr;
	UINT8 Data;
}I2C_TYPE;

R848_ErrCode I2C_Write_Len(I2C_LEN_TYPE *I2C_Info);
R848_ErrCode I2C_Read_Len(I2C_LEN_TYPE *I2C_Info);
R848_ErrCode I2C_Write(I2C_TYPE *I2C_Info);
int R848_Convert(int InvertNum);
