//
//  dukeunix.h
//  Duke3D
//
//  Created by fabien sanglard on 12-12-12.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#ifndef Duke3D_dukeunix_h
#define Duke3D_dukeunix_h


#define cdecl
#define __far
#define __interrupt


//#define STUBBED(x)
#ifdef __SUNPRO_C
#define STUBBED(x) fprintf(stderr,"STUB: %s (??? %s:%d)\n",x,__FILE__,__LINE__)
#else
#define STUBBED(x) /*fprintf(stderr,"STUB: %s (%s, %s:%d)\n",x,__FUNCTION__,__FILE__,__LINE__)*/
#endif

#define PATH_SEP_CHAR '/'
#define PATH_SEP_STR  "/"
#define ROOTDIR       "/"
#define CURDIR        "./"

#ifndef O_BINARY
#define O_BINARY 0
#endif

struct find_t
{
    int dir;
    char  pattern[MAX_PATH];
    char  name[MAX_PATH];
};
int _dos_findfirst(char  *filename, int x, struct find_t *f);
int _dos_findnext(struct find_t *f);

struct dosdate_t
{
    uint8_t  day;
    uint8_t  month;
    unsigned int year;
    uint8_t  dayofweek;
};

void _dos_getdate(struct dosdate_t *date);

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif



#ifndef strcmpi
#define strcmpi(x, y) strcasecmp(x, y)
#endif

#ifndef __STM32__
#define __STM32__
#endif

#if defined(__STM32__)

#define stderr (-1)
#define stdout (-1)
#define Z_AvailHeap() Sys_AllocBytesLeft()

#elif defined(DC)
#undef stderr
#undef stdout
#undef getchar
/* kos compat */
#define stderr (-1)
#define stdout (-1)
#define Z_AvailHeap() ((10 * 1024) * 1024)
#else
// 64 megs should be enough for anybody.  :)  --ryan.
#define Z_AvailHeap() ((64 * 1024) * 1024)
#endif

#define printchrasm(x,y,ch) dprintf("%c", (uint8_t ) (ch & 0xFF))

#ifdef __GNUC__
#define GCC_PACK1_EXT __attribute__((packed,aligned(1)))
#endif


// FCS: Game.c features calls to mkdir without the proper flags.
// Giving all access is ugly but it is just game OK !
#define mkdir(X) mkdir(X,0777)

#define getch() 0

#endif
