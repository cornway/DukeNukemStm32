//
//  unix_compat.h
//  Duke3D
//
//  Based on macos_compat.h
//  Copyright Wed, Jul 31, 2013, Juan Manuel Borges Caño (GPLv3+)
//

#ifndef Duke3D_unix_compat_h
#define Duke3D_unix_compat_h

//#define BYTE_ORDER LITTLE_ENDIAN
#define PLATFORM_SUPPORTS_SDL

#include <stdlib.h>

#define kmalloc(x) Sys_Malloc(x)
#define kkmalloc(x) Sys_Malloc(x)
#define kfree(x) Sys_Free(x)
#define kkfree(x) Sys_Free(x)

#ifdef FP_OFF
#undef FP_OFF
#endif

// Horrible horrible macro: Watcom allowed memory pointer to be cast
// to a 32bits integer. The code is unfortunately stuffed with this :( !
#define FP_OFF(x) ((int32_t) (x))

#ifndef max
#define max(x, y)  (((x) > (y)) ? (x) : (y))
#endif

#ifndef min
#define min(x, y)  (((x) < (y)) ? (x) : (y))
#endif

#include <inttypes.h>
#define __int64 int64_t

#ifdef __STM32__
#define FX_SOUND_DEVICE stm32769idisco
#endif

#define O_BINARY 0

#define UDP_NETWORKING 1

/*
#define SOL_IP SOL_SOCKET
#define IP_RECVERR  SO_BROADCAST
*/

#define stricmp strcasecmp
#define strcmpi strcasecmp

#define S_IREAD S_IRUSR
#include <misc_utils.h>
#include <string.h>

#endif
