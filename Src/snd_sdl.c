
#include <stdio.h>
#include "SDL_audio.h"
#include "SDL_byteorder.h"
#include "audio_main.h"
#include "sounds.h"
#include "begin_code.h"

sfx_t ambient_sfx[4];
channel_t   channels[16];
int			total_channels;

typedef struct {
    sfx_t sfx;
    audio_channel_t *achannel;
    uint8_t volume_prev;
} ambient_t;

int sound_started=0;


int         desired_speed = 11025; //11025;
int         desired_bits = 16;

extern dma_t  *shm;

int			soundtime;		// sample PAIRS
int   		paintedtime; 	// sample PAIRS


void CDAudio_Pause(void)
{
}

void CDAudio_Play(byte track, boolean looping)
{
}


void CDAudio_Resume(void)
{
}

int CDAudio_Init(void)
{
}

void CDAudio_Shutdown(void)
{

}

void CDAudio_Update(void)
{

}


void S_PaintChannels(int endtime)
{

}


void S_StopAllSounds(boolean clear)
{

}

void S_ClearPrecache (void)
{

}


void S_BeginPrecaching (void)
{

}

void S_EndPrecaching (void)
{

}

#define NUM_AMBIENTS 4
#define MAX_DYNAMIC_CHANNELS 12
/*
=================
SND_PickChannel
=================
*/
channel_t *SND_PickChannel(int entnum, int entchannel, int *chidx)
{
}       


extern sfxcache_t *S_LoadSound (sfx_t *s);

sfx_t *S_FindName (char *name, sfx_t *sfx)
{
    strcpy (sfx->name, name);
    return sfx;
}

/*
==================
S_PrecacheSound

==================
*/
sfx_t *S_PrecacheSound (char *name, sfx_t *sfx)
{
}

/*
==================
S_TouchSound

==================
*/
void S_TouchSound (char *name, sfx_t	*sfx)
{
    if (!sound_started)
        return;

    sfx = S_FindName (name, sfx);
}


void S_LocalSound (char *sound)
{
}

void S_ExtraUpdate (void)
{
}

/*
=================
SND_Spatialize
=================
*/
void SND_Spatialize(channel_t *ch)
{

}


static audio_channel_t *S_PushSound (channel_t *ch, int channel)
{
}

void S_StartSound(int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation)
{
    channel_t *target_chan, *check;
    audio_channel_t *achannel;
    sfxcache_t	*sc;
    int		vol;
    int		ch_idx, chpush;
    int		skip;

    if (!sound_started)
        return;

    if (!sfx)
        return;

    vol = fvol*255;

// pick a channel to play on
    target_chan = SND_PickChannel(entnum, entchannel, &chpush);
    if (!target_chan)
        return;

// spatialize
    memset (target_chan, 0, sizeof(*target_chan));
    target_chan->master_vol = vol;
    target_chan->entnum = entnum;
    target_chan->entchannel = entchannel;
    SND_Spatialize(target_chan);

    if (!target_chan->leftvol && !target_chan->rightvol)
        return;		// not audible at all

// new channel
    sc = S_LoadSound (sfx);
    if (!sc)
    {
        target_chan->sfx = NULL;
        return;// couldn't load the sound's data
    }

    target_chan->sfx = sfx;
    target_chan->pos = 0.0;
    target_chan->end = paintedtime + sc->length;	

// if an identical sound has also been started this frame, offset the pos
// a bit to keep it from just making the first one louder
    check = &channels[NUM_AMBIENTS];
    for (ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++, check++)
    {
        if (check == target_chan)
            continue;
        if (check->sfx == sfx && !check->pos)
        {
            skip = 1 % (int)(0.1*shm->speed);
            if (skip >= target_chan->end)
                skip = target_chan->end - 1;
            target_chan->pos += skip;
            target_chan->end -= skip;
            break;
        }
    }
    achannel = S_PushSound(target_chan, chpush);
    if (achannel) {
        achannel->complete = NULL;
    }
}

void S_StopSound(int entnum, int entchannel)
{
    int i;

    for (i=0 ; i<MAX_DYNAMIC_CHANNELS ; i++)
    {
        if (channels[i].entnum == entnum
            && channels[i].entchannel == entchannel)
        {
            channels[i].end = 0;
            channels[i].sfx = NULL;
            audio_stop_channel(i);
            return;
        }
    }
}

void S_StaticSound (sfx_t *sfx, float vol, float attenuation)
{
}


void S_Init (void)
{
   sound_started = true;

    shm = (void *) Sys_Malloc(sizeof(*shm));
    shm->splitbuffer = 0;
    shm->samplebits = 16;
    shm->speed = AUDIO_SAMPLE_RATE;
    shm->channels = 2;
    shm->samples = AUDIO_OUT_BUFFER_SIZE;
    shm->samplepos = 0;
    shm->soundalive = true;
    shm->gamealive = true;
    shm->submission_chunk = 1;

    S_StopAllSounds (true);
}

void S_Shutdown(void)
{
}

/*
===================
S_UpdateAmbientSounds
===================
*/
void S_UpdateAmbientSounds (void)
{
}


void S_Update(void)
{
    audio_update();
}

void S_ClearBuffer (void)
{

}

DECLSPEC int SDLCALL SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    return 0;
}

DECLSPEC void SDLCALL SDL_CloseAudio(void)
{}

