/* -*- Mode: C; tab-width: 4 -*- */ 

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#ifndef __WIN32__

#endif

#include "sdl_keysym.h"
#include "audio_main.h"
#include "input_main.h"
#include "main.h"
#include "dev_io.h"
#include "debug.h"
#include "begin_code.h"
#include <misc_utils.h>

void Sys_Error (char *error, ...);

boolean        isDedicated;

int noconinput = 0;

char *basedir = ".";
char *cachedir = "/tmp";

// =======================================================================
// General routines
// =======================================================================

void Sys_DebugNumber(int y, int val)
{
}

void Sys_Printf (char *fmt, ...)
{
    va_list         argptr;

    va_start (argptr, fmt);
    dvprintf (fmt, argptr);
    va_end (argptr);
}

void Sys_Quit (void)
{
	for (;;) {}
}

void Sys_Init(void)
{
#if id386
	Sys_SetFPCW();
#endif
}

void SDL_Quit(void)
{
    Sys_Error("-----------SDL_Quit-----------/n");
}

#if !id386

/*
================
Sys_LowFPPrecision
================
*/
void Sys_LowFPPrecision (void)
{
// causes weird problems on Nextstep
}


/*
================
Sys_HighFPPrecision
================
*/
void Sys_HighFPPrecision (void)
{
// causes weird problems on Nextstep
}

#endif	// !id386


void Sys_Error (char *error, ...)
{
    va_list         argptr;

    va_start (argptr, error);
    dvprintf (error, argptr);
    va_end (argptr);

    serial_flush();
    Sys_Quit();
} 

void Sys_Warn (char *warning, ...)
{ 
    va_list         argptr;

    va_start (argptr, warning);
    dvprintf (warning, argptr);
    va_end (argptr);
}

void Sys_DebugLog(char *file, char *fmt, ...)
{
    va_list         argptr;
    
    va_start (argptr, fmt);
    dvprintf (fmt, argptr);
    va_end (argptr);
}

DECLSPEC char * SDLCALL SDL_GetError(void)
{
    return "not implemented yet\n";
}

DECLSPEC void SDLCALL SDL_ClearError(void)
{}

DECLSPEC SDLMod SDLCALL SDL_GetModState(void)
{
    return KMOD_NONE;
}

DECLSPEC uint8_t SDLCALL SDL_GetMouseState(int *x, int *y)
{
    return 0;
}


/*
===============================================================================

FILE IO

===============================================================================
*/
int Sys_FileOpenRead (char *path, int *hndl)
{
    return d_open(path, hndl, "r");
}

int Sys_FileOpenWrite (char *path)
{
    int h;
    d_open(path, &h, "+w");
    return h;
}

void Sys_FileClose (int handle)
{
    d_close(handle);
}

void Sys_FileSeek (int handle, int position)
{
    d_seek(handle, position);
}

int Sys_Feof (int handle)
{
    return d_eof(handle);
}

int Sys_FileRead (int handle, void *dst, int count)
{
    return d_read(handle, dst, count);
}

char *Sys_FileGetS (int handle, char *dst, int count)
{
    return d_gets(handle, dst, count);
}

int Sys_FileWrite (int handle, void *src, int count)
{
    return d_write(handle, src, count);
}

int Sys_FPrintf (int handle, char *fmt, ...)
{
    va_list ap;
    char p[256];
    int   r;

    va_start (ap, fmt);
    r = vsnprintf(p, sizeof(p), fmt, ap);
    va_end (ap);
    if (Sys_FileWrite(handle, p, r) < 0) {
        dprintf("%s Bad : %s\n", __func__, p);
    }
    return r;
}


int	Sys_FileTime (char *path)
{
    return 0;
}

void Sys_mkdir (char *path)
{
    d_mkdir(path);
}


extern volatile uint32_t systime;

double Sys_FloatTime (void)
{
#ifdef __WIN32__

	static int starttime = 0;

	if ( ! starttime )
		starttime = clock();

	return (clock()-starttime)*1.0/1024;

#else

    return systime;

#endif
}

// =======================================================================
// Sleeps for microseconds
// =======================================================================

static volatile int oktogo;

void alarm_handler(int x)
{
	oktogo=1;
}

uint8_t *Sys_ZoneBase (int *size)
{

    Sys_Error("Not supported");
    return NULL;
}

void Sys_LineRefresh(void)
{
}

void Sys_Sleep(void)
{
	HAL_Delay(1);
}

void floating_point_exception_handler(int whatever)
{
    Sys_Error("floating point exception\n");
}

void moncontrol(int x)
{
}

int SDL_main (int argc, const char *argv[])
{

    double  time, oldtime, newtime;
    extern int vcrFile;
    extern int recording;
    static int frame;

    moncontrol(0);

    Sys_Init();


    oldtime = Sys_FloatTime () - 0.1;
}


#if	id386
/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
	int r;
	unsigned long addr;
	int psize = getpagesize();

	//fprintf(stderr, "writable code %lx-%lx\n", startaddr, startaddr+length);

	addr = startaddr & ~(psize-1);

	r = mprotect((char*)addr, length + startaddr - addr, 7);

	if (r < 0)
    		Sys_Error("Protection change failed\n");
}

#endif /*id386*/

