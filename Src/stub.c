 
#include <misc_utils.h>
#include <SDL_stdinc.h>
#include <SDL_Video.h>
#include <audio_main.h>
#include <lcd_main.h>
#include "fx_man.h"
#include <begin_code.h>
#include <duke3d.h>

extern const char *mus_dir_path;
static uint8_t cd_master_volume = 0x7f;

static const char *_file_ext (const char *path)
{
    const char *p = path;
    while (*p) {
        if (*p++ == '.') {
            return p;
        }
    }
    return NULL;
}

static boolean _is_midi (const char *ext)
{
    if (strncasecmp("MID", ext, 3) == 0) {
        return true;
    }
    return false;
}

static cd_track_t cd;

void PlayMusic(char  *fileName)
{
    char *p;
    char path[MAX_PATH], name[MAX_PATH];

    snprintf(name, sizeof(name), "%s", fileName);

    p = (char *)_file_ext(name);
    if (!_is_midi(p)) {
        assert(0);
    }
    *--p = 0;
    snprintf(path, sizeof(path), "%s/%s.%s", mus_dir_path, name, "WAV");
    cd_play_name(&cd, path);
    cd_volume(&cd, cd_master_volume);
}

void MUSIC_RegisterTimbreBank (unsigned char *timbres)
{

}

int MUSIC_StopSong (void)
{
    cd_stop(&cd);
    return( 0 );
}

void MUSIC_Continue (void)
{
    cd_resume(&cd);
}

void MUSIC_SetVolume (int volume)
{
    volume = max( 0, volume );
    volume = min( volume, 255 );
    cd_master_volume = volume;
    cd_volume(&cd, cd_master_volume);
}


void MUSIC_Pause (void)
{
    cd_pause(&cd);
}

int MUSIC_Init (int SoundCard, int Address)
{
    cd_init();
    return 0;
}

int MUSIC_Shutdown (void)
{
    return 0;
}