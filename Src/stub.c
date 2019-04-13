 
#include <misc_utils.h>
#include <SDL_stdinc.h>
#include <SDL_Video.h>
#include "fx_man.h"
#include <begin_code.h>

DECLSPEC void SDLCALL SDL_UpdateRect
        (SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
}

void PlayMusic(char  *fileName)
{
}

void MUSIC_RegisterTimbreBank
(
unsigned char *timbres
)

{

}

int MUSIC_StopSong
(
void
)

{
    return( 0 );
}

void MUSIC_Continue
(
void
)

{

}

void MUSIC_SetVolume
(
int volume
)

{
volume = max( 0, volume );
volume = min( volume, 255 );
}


void MUSIC_Pause
(
void
)

{
}

int MUSIC_Init
   (
   int SoundCard,
   int Address
   )
{
    return 0;
}

int MUSIC_Shutdown
(
void
)
{
    return 0;
}