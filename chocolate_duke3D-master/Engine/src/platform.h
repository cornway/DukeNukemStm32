#ifndef _INCLUDE_PLATFORM_H_
#define _INCLUDE_PLATFORM_H_

#define __STM32__

#include <stdint.h>

#if (defined PLATFORM_WIN32)
    #include "win32_compat.h"
#elif (defined PLATFORM_UNIX)
    #include "unix_compat.h"
#elif (defined PLATFORM_DOS)
    #include "doscmpat.h"
#elif (defined __APPLE__)
    #include "macos_compat.h"
#elif (defined __STM32__)
    #define PLATFORM_SUPPORTS_SDL
#else
#error Define your platform!
#endif

#if (!defined __EXPORT__)
    #define __EXPORT__
#endif

uint16_t _swap16(uint16_t D);
unsigned int _swap32(unsigned int D);
#if defined(PLATFORM_MACOSX) && defined(__ppc__)
#define PLATFORM_BIGENDIAN 1
#define BUILDSWAP_INTEL16(x) _swap16(x)
#define BUILDSWAP_INTEL32(x) _swap32(x)
#else
#define PLATFORM_LITTLEENDIAN 1
#define BUILDSWAP_INTEL16(x) (x)
#define BUILDSWAP_INTEL32(x) (x)
#endif

static inline void
writeShort (void *_buf, short v)
{
    uint8_t *buf = (uint8_t *)_buf;
    buf[0] = v & 0xff;
    buf[1] = v >> 8;
}

static inline void
writeLong (void *_buf, unsigned long v)
{
    uint8_t *buf = (uint8_t *)_buf;
    buf[0] = v & 0xff;
    buf[1] = v >> 8;
    buf[2] = v >> 16;
    buf[3] = v >> 24;
}


#endif  /* !defined _INCLUDE_PLATFORM_H_ */

/* end of platform.h ... */


