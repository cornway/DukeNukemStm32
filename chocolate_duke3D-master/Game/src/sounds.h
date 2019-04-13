//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aint32_t with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

//****************************************************************************
//
// sounds.h
//
//****************************************************************************
#include "audiolib/fx_man.h"
#include <misc_utils.h>
#include <stdint.h>

#ifndef _sounds_public_
#define _sounds_public_

#ifndef byte
#define byte uint8_t
#endif

typedef struct
{
	int 	length;
	int 	loopstart;
	int 	speed;
	int 	width;
	int 	stereo;
	byte	data[1];		// variable sized
} sfxcache_t;

typedef struct
{
	boolean		gamealive;
	boolean		soundalive;
	boolean		splitbuffer;
	int				channels;
	int				samples;				// mono samples in buffer
	int				submission_chunk;		// don't mix less than this #
	int				samplepos;				// in mono samples
	int				samplebits;
	int				speed;
	unsigned char	*buffer;
} dma_t;

typedef struct cache_user_s
{
	void	*data;
} cache_user_t;

typedef struct sfx_s
{
	char 	name[32];
	cache_user_t	cache;
} sfx_t;

typedef struct
{
	int left;
	int right;
} portable_samplepair_t;

typedef struct
{
	int		rate;
	int		width;
	int		channels;
	int		loopstart;
	int		samples;
	int		dataofs;		// chunk starts this many bytes from file start
} wavinfo_t;

typedef struct
{
	sfx_t	*sfx;			// sfx number
	int		leftvol;		// 0-255 volume
	int		rightvol;		// 0-255 volume
	int		end;			// end time in global paintsamples
	int 	pos;			// sample position in sfx
	int		looping;		// where to loop, -1 = no looping
	int		entnum;			// to allow overriding a specific sound
	int		entchannel;		//
	int		master_vol;		// 0-255 master volume
} channel_t;


#define HIRESMUSICPATH "tunes"

extern int32_t FXDevice;
extern int32_t MusicDevice;
extern int32_t FXVolume;
extern int32_t MusicVolume;
extern fx_blaster_config BlasterConfig;
extern int32_t NumVoices;
extern int32_t NumChannels;
extern int32_t NumBits;
extern int32_t MixRate;
extern int32_t MidiPort;
extern int32_t ReverseStereo;

void SoundStartup( void );
void SoundShutdown( void );
void MusicStartup( void );
void MusicShutdown( void );

/* sounds.c */
void clearsoundlocks(void);

/* dunno where this came from; I added it. --ryan. */
void testcallback(uint32_t num);

#endif
