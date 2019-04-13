/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/**********************************************************************
   module: FX_MAN.C

   author: James R. Dose
   date:   March 17, 1994

   Device independant sound effect routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "sndcards.h"
#include "multivoc.h"



#include "dsl.h"

#include "ll_man.h"
#include "user.h"
#include "fx_man.h"

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

static unsigned FX_MixRate;

int FX_SoundDevice = -1;
int FX_ErrorCode = FX_Ok;
int FX_Installed = FALSE;

#define FX_SetErrorCode( status ) \
   FX_ErrorCode = ( status );

/*---------------------------------------------------------------------
   Function: FX_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *FX_ErrorString(int ErrorNumber)

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_SetupCard

   Sets the configuration of a sound device.
---------------------------------------------------------------------*/

int FX_SetupCard
   (
   int SoundCard,
   fx_device *device
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_GetBlasterSettings

   Returns the current BLASTER environment variable settings.
---------------------------------------------------------------------*/

int FX_GetBlasterSettings
   (
   fx_blaster_config *blaster
   )

   {

   return( FX_Ok );
   }


/*---------------------------------------------------------------------
   Function: FX_SetupSoundBlaster

   Handles manual setup of the Sound Blaster information.
---------------------------------------------------------------------*/

int FX_SetupSoundBlaster
   (
   fx_blaster_config blaster,
   int *MaxVoices,
   int *MaxSampleBits,
   int *MaxChannels
   )

   {


   return( FX_Ok );
   }


/*---------------------------------------------------------------------
   Function: FX_Init

   Selects which sound device to use.
---------------------------------------------------------------------*/

int FX_Init
   (
   int SoundCard,
   int numvoices,
   int numchannels,
   int samplebits,
   unsigned mixrate
   )
{
    return( 0 );
}


/*---------------------------------------------------------------------
   Function: FX_Shutdown

   Terminates use of sound device.
---------------------------------------------------------------------*/

int FX_Shutdown
   (
   void
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_SetCallback

   Sets the function to call when a voice is done.
---------------------------------------------------------------------*/

int FX_SetCallBack(void ( *function )( int32_t ))

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_SetVolume

   Sets the volume of the current sound device.
---------------------------------------------------------------------*/

void FX_SetVolume(int volume)
   {
   }


/*---------------------------------------------------------------------
   Function: FX_GetVolume

   Returns the volume of the current sound device.
---------------------------------------------------------------------*/

int FX_GetVolume(void)
   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_SetReverseStereo

   Set the orientation of the left and right channels.
---------------------------------------------------------------------*/

void FX_SetReverseStereo(int setting)

   {
   }


/*---------------------------------------------------------------------
   Function: FX_GetReverseStereo

   Returns the orientation of the left and right channels.
---------------------------------------------------------------------*/

int FX_GetReverseStereo(void)
   {
    return 0;
   }


/*---------------------------------------------------------------------
   Function: FX_SetReverb

   Sets the reverb level.
---------------------------------------------------------------------*/

void FX_SetReverb
   (
   int reverb
   )

   {
   }


/*---------------------------------------------------------------------
   Function: FX_SetFastReverb

   Sets the reverb level.
---------------------------------------------------------------------*/

void FX_SetFastReverb
   (
   int reverb
   )

   {
   }


/*---------------------------------------------------------------------
   Function: FX_GetMaxReverbDelay

   Returns the maximum delay time for reverb.
---------------------------------------------------------------------*/

int FX_GetMaxReverbDelay
   (
   void
   )

   {
   return 0;
   }


/*---------------------------------------------------------------------
   Function: FX_GetReverbDelay

   Returns the current delay time for reverb.
---------------------------------------------------------------------*/

int FX_GetReverbDelay
   (
   void
   )

   {
   return 0;
   }


/*---------------------------------------------------------------------
   Function: FX_SetReverbDelay

   Sets the delay level of reverb to add to mix.
---------------------------------------------------------------------*/

void FX_SetReverbDelay
   (
   int delay
   )

   {
   }


/*---------------------------------------------------------------------
   Function: FX_VoiceAvailable

   Checks if a voice can be play at the specified priority.
---------------------------------------------------------------------*/

int FX_VoiceAvailable
   (
   int priority
   )

   {
   return 0;
   }

/*---------------------------------------------------------------------
   Function: FX_EndLooping

   Stops the voice associated with the specified handle from looping
   without stoping the sound.
---------------------------------------------------------------------*/

int FX_EndLooping
   (
   int handle
   )

   {
   return 0;
   }

/*---------------------------------------------------------------------
   Function: FX_SetPan

   Sets the stereo and mono volume level of the voice associated
   with the specified handle.
---------------------------------------------------------------------*/

int FX_SetPan
   (
   int handle,
   int vol,
   int left,
   int right
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_SetPitch

   Sets the pitch of the voice associated with the specified handle.
---------------------------------------------------------------------*/

int FX_SetPitch
   (
   int handle,
   int pitchoffset
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_SetFrequency

   Sets the frequency of the voice associated with the specified handle.
---------------------------------------------------------------------*/

int FX_SetFrequency
   (
   int handle,
   int frequency
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayVOC

   Begin playback of sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayVOC
   (
   uint8_t *ptr,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayLoopedVOC

   Begin playback of sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayLoopedVOC
   (
   uint8_t *ptr,
   int32_t loopstart,
   int32_t loopend,
   int32_t pitchoffset,
   int32_t vol,
   int32_t left,
   int32_t right,
   int32_t priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayWAV

   Begin playback of sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayWAV
   (
   uint8_t *ptr,
   int32_t pitchoffset,
   int32_t vol,
   int32_t left,
   int32_t right,
   int32_t priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayWAV

   Begin playback of sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayLoopedWAV
   (
   uint8_t *ptr,
   int32_t loopstart,
   int32_t loopend,
   int32_t pitchoffset,
   int32_t vol,
   int32_t left,
   int32_t right,
   int32_t priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayVOC3D

   Begin playback of sound data at specified angle and distance
   from listener.
---------------------------------------------------------------------*/

int FX_PlayVOC3D
   (
   uint8_t *ptr,
   int32_t pitchoffset,
   int32_t angle,
   int32_t distance,
   int32_t priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayWAV3D

   Begin playback of sound data at specified angle and distance
   from listener.
---------------------------------------------------------------------*/

int FX_PlayWAV3D
   (
   uint8_t *ptr,
   int32_t pitchoffset,
   int32_t angle,
   int32_t distance,
   int32_t priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayRaw

   Begin playback of raw sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayRaw
   (
   uint8_t *ptr,
   uint32_t length,
   uint32_t rate,
   int32_t pitchoffset,
   int32_t vol,
   int32_t left,
   int32_t right,
   int32_t priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayLoopedRaw

   Begin playback of raw sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayLoopedRaw
   (
   uint8_t *ptr,
   uint32_t length,
   char *loopstart,
   char *loopend,
   uint32_t rate,
   int32_t pitchoffset,
   int32_t vol,
   int32_t left,
   int32_t right,
   int32_t priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_Pan3D

   Set the angle and distance from the listener of the voice associated
   with the specified handle.
---------------------------------------------------------------------*/

int FX_Pan3D
   (
   int handle,
   int angle,
   int distance
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_SoundActive

   Tests if the specified sound is currently playing.
---------------------------------------------------------------------*/

int FX_SoundActive
   (
   int handle
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_SoundsPlaying

   Reports the number of voices playing.
---------------------------------------------------------------------*/

int FX_SoundsPlaying
   (
   void
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_StopSound

   Halts playback of a specific voice
---------------------------------------------------------------------*/

int FX_StopSound
   (
   int handle
   )

   {
   return( FX_Ok );
   }


/*---------------------------------------------------------------------
   Function: FX_StopAllSounds

   Halts playback of all sounds.
---------------------------------------------------------------------*/

int FX_StopAllSounds
   (
   void
   )

   {
   return( FX_Ok );
   }


/*---------------------------------------------------------------------
   Function: FX_StartDemandFeedPlayback

   Plays a digitized sound from a user controlled buffering system.
---------------------------------------------------------------------*/

int FX_StartDemandFeedPlayback
   (
   void ( *function )( char **ptr, uint32_t *length ),
   int32_t rate,
   int32_t pitchoffset,
   int32_t vol,
   int32_t left,
   int32_t right,
   int32_t priority,
   uint32_t callbackval
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_StartRecording

   Starts the sound recording engine.
---------------------------------------------------------------------*/

int FX_StartRecording
   (
   int MixRate,
   void ( *function )( char *ptr, int length )
   )

   {
   return( 0 );
   }


/*---------------------------------------------------------------------
   Function: FX_StopRecord

   Stops the sound record engine.
---------------------------------------------------------------------*/

void FX_StopRecord
   (
   void
   )

   {
   }
