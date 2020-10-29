//////////////////////////////////////////////////////////////////////////
/////
// TH DTV MIDDLE WARE DEFINES, PLEASE DO NOT MAKE ANY CHANGE OF THIS FILE
/////
//////////////////////////////////////////////////////////////////////////

#ifndef __VS_FE_DEBUG_H__
#define __VS_FE_DEBUG_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <linux/string.h>

#ifndef DEBUG_TAG
#define DEBUG_TAG "[VS_FE]"
#endif

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_WARNING
#endif

#define DEBUG_LEVEL_NONE    5
#define DEBUG_LEVEL_VERBOSE 4
#define DEBUG_LEVEL_ERROR   3
#define DEBUG_LEVEL_WARNING 2
#define DEBUG_LEVEL_INFO    1
#define DEBUG_LEVEL_ALL     0

#define DBG_PRINTK(level, fmt, args...) \
do { \
    if (level >= DEBUG_LEVEL) {printk(DEBUG_TAG"[LV%d][%s][%d]: "fmt"\n", level, __func__, __LINE__, ##args);} \
}  while (0)

#define DBG_VERBOSE(fmt, args...) DBG_PRINTK(DEBUG_LEVEL_VERBOSE, fmt, ##args)
#define DBG_ERROR(fmt, args...) DBG_PRINTK(DEBUG_LEVEL_ERROR, fmt, ##args)
#define DBG_WARNING(fmt, args...) DBG_PRINTK(DEBUG_LEVEL_WARNING, fmt, ##args)
#define DBG_INFO(fmt, args...) DBG_PRINTK(DEBUG_LEVEL_INFO, fmt, ##args)
#define DBG(fmt, args...) DBG_PRINTK(DEBUG_LEVEL_ALL, fmt, ##args)

#define _Print(fmt, args...) DBG_PRINTK(DEBUG_LEVEL_VERBOSE, fmt, ##args)

#ifdef __cplusplus
}
#endif

#endif    // __VS_FE_DEBUG_H__

//////////////////////////////////////////////////////////////////////////
/////
// TH DTV MIDDLE WARE DEFINES, PLEASE DO NOT MAKE ANY CHANGE OF THIS FILE
/////
//////////////////////////////////////////////////////////////////////////
