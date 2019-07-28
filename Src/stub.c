 
#include <misc_utils.h>
#include <SDL_stdinc.h>
#include <SDL_Video.h>
#include <audio_main.h>
#include <lcd_main.h>
#include "fx_man.h"
#include <duke3d.h>
#include <debug.h>
#include <bsp_sys.h>
#include <misc_utils.h>
#include <dev_io.h>
#include <begin_code.h>

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
    /*done in audio_init()*/
    return 0;
}

int MUSIC_Shutdown (void)
{
    return 0;
}

DECLSPEC int SDLCALL SDL_LockSurface(SDL_Surface *surface)
{
    return 0;
}

DECLSPEC void SDLCALL SDL_UnlockSurface(SDL_Surface *surface)
{

}

static SDL_Rect sdl320x200 = {0, 0, 320, 200};

static SDL_Rect *phys_modes[] =
{
   &sdl320x200,
   NULL,
};

DECLSPEC const SDL_VideoInfo * SDLCALL SDL_GetVideoInfo(void)
{
    static SDL_VideoInfo info;
    info.hw_available = 0,
    info.wm_available = 0,
    info.UnusedBits1 = 0,
    info.UnusedBits2 = 0,
    info.blit_hw = 0,
    info.blit_hw_CC = 0,
    info.blit_hw_A = 0,
    info.blit_sw = 0,
    info.blit_sw_CC = 0,
    info.blit_sw_A = 0,
    info.blit_fill = 1,
    info.UnusedBits3 = 0,
    info.video_mem = vid_mem_avail();
    info.vfmt = NULL;
    info.current_w = DEV_MAXXDIM;
    info.current_h = DEV_MAXYDIM;
    return &info;
}

DECLSPEC SDL_Rect ** SDLCALL SDL_ListModes(SDL_PixelFormat *format, Uint32 flags)
{
    assert(flags & SDL_HWPALETTE);
    return phys_modes;
}

#define SDL_VIDEO_DRV_NAME "stm32 embed video"

DECLSPEC char * SDLCALL SDL_VideoDriverName(char *namebuf, int maxlen)
{
    snprintf(namebuf, maxlen, "%s\n", SDL_VIDEO_DRV_NAME);
    return namebuf;
}

DECLSPEC void SDLCALL SDL_WM_SetCaption(const char *title, const char *icon)
{

}

DECLSPEC int SDLCALL SDL_Init (uint32_t what)
{
    return 0;
}

A_COMPILE_TIME_ASSERT(pallette_chk, sizeof(uint32_t) == sizeof(SDL_Color));

DECLSPEC int SDLCALL SDL_SetColors(SDL_Surface *surface, 
                                        SDL_Color *colors, int firstcolor, int ncolors)
{
    vid_set_clut((colors + firstcolor), ncolors);
    return 0;
}

DECLSPEC int SDLCALL SDL_putenv(const char *variable)
{
    dprintf("%s() : var= \'%s\'\n", __func__, variable);
    return 0;
}

void SDL_Delay (uint32_t ms)
{
    Sys_Sleep(ms);
}

uint32_t SDL_GetTicks (void)
{
    return d_time();
}

