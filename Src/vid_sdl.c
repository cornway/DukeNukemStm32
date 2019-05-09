// vid_sdl.h -- sdl video driver 

#include "gfx.h"
#include "sdl_video.h"
#include "lcd_main.h"
#include "input_main.h"
#include "sdl_keysym.h"
#include "keyboard.h"
#include <vid.h>

#define VIDEO_IN_IRAM 1

viddef_t    vid;                // global video state

// The original defaults
//#define    BASEWIDTH    320
//#define    BASEHEIGHT   200
// Much better for high resolution displays
#define    BASEWIDTH    (320)
#define    BASEHEIGHT   (200)

#define D_SCREEN_PIX_CNT (BASEWIDTH * BASEHEIGHT)
#define D_SCREEN_BYTE_CNT (D_SCREEN_PIX_CNT * sizeof(pix_t))

#if VIDEO_IN_IRAM
uint8_t screenbuf[BASEWIDTH * BASEHEIGHT * sizeof(pix_t) + sizeof(SDL_Surface)] = {0};
#endif

int    VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes = 0;
uint8_t    *VGA_pagebase;

extern SDL_Surface *surface;
extern int32_t xdim, ydim;
static SDL_Surface *sdlsurface = NULL;

// No support for option menus
void (*vid_menudrawfn)(void) = NULL;
void (*vid_menukeyfn)(int key) = NULL;

void VID_SetPalette (uint8_t* palette)
{

}

void    VID_ShiftPalette (unsigned char *palette)
{
    VID_SetPalette(palette);
}

static SDL_Surface *VID_Init (int width, int height, int bpp, Uint32 flags)
{
    screen_t lcd_screen;

    // Set up display mode (width and height)
    vid.width = BASEWIDTH;
    vid.height = BASEHEIGHT;
    vid.maxwarpwidth = 320;
    vid.maxwarpheight = 200;

    lcd_screen.buf = NULL;
    lcd_screen.width = BASEWIDTH;
    lcd_screen.height = BASEHEIGHT;

    screen_win_cfg(&lcd_screen);

#if VIDEO_IN_IRAM
    sdlsurface = (SDL_Surface *)&screenbuf[0];
#else
    sdlsurface = (SDL_Surface *)Sys_Malloc(BASEWIDTH * BASEHEIGHT * sizeof(pix_t) + sizeof(SDL_Surface), "screen");
    if (screen == NULL)
        Sys_Error ("Not enough memory for video mode\n");
#endif

    // Set video width, height and flags
    flags = (SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);

    memset(sdlsurface, 0, sizeof(SDL_Surface));

    sdlsurface->pixels = (void *)(sdlsurface + 1);
    sdlsurface->flags = flags;
    sdlsurface->w = BASEWIDTH;
    sdlsurface->h = BASEHEIGHT;
    sdlsurface->offset = 0;
    sdlsurface->pitch = BASEWIDTH;

    // now know everything we need to know about the buffer
    VGA_width = vid.conwidth = vid.width;
    VGA_height = vid.conheight = vid.height;
    vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
    vid.numpages = 1;
    vid.colormap = NULL;//host_colormap;
    vid.fullbright = 255;
    VGA_pagebase = vid.buffer = sdlsurface->pixels;
    VGA_rowbytes = vid.rowbytes = sdlsurface->pitch;
    vid.conbuffer = vid.buffer;
    vid.conrowbytes = vid.rowbytes;
    vid.direct = NULL;

    xdim = sdlsurface->w;
    ydim = sdlsurface->h;

    return sdlsurface;
}

DECLSPEC void SDLCALL SDL_UpdateRect
        (SDL_Surface *sdlscreen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
    screen_t screen;
    screen.buf = sdlscreen->pixels;
    screen.width = sdlscreen->w;
    screen.height = sdlscreen->h;
    screen_update(&screen);
}

extern DECLSPEC SDL_Surface * SDLCALL SDL_SetVideoMode
			(int width, int height, int bpp, Uint32 flags)
{
    return VID_Init(width, height, bpp, flags);
}


void    VID_Shutdown (void)
{
    SDL_Quit();
}

#if GFX_COLOR_MODE != GFX_COLOR_MODE_CLUT
#error "Unsupported mode"
#endif

typedef struct {
    pix_t a[4];
} scanline_t;

typedef union {
#if (GFX_COLOR_MODE == GFX_COLOR_MODE_CLUT)
    uint32_t w;
#elif (GFX_COLOR_MODE == GFX_COLOR_MODE_RGB565)
    uint64_t w;
#endif
    scanline_t sl;
} scanline_u;

#define DST_NEXT_LINE(x) (((uint32_t)(x) + BASEWIDTH * 2 * sizeof(pix_t)))
#define W_STEP (sizeof(scanline_t) / sizeof(pix_t))

void uiUpdate (vrect_t *rect, screen_t *lcd_screen)
{
}

void    VID_Update (void *rects)
{
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, uint8_t *pbitmap, int width, int height)
{
    uint8_t *offset;


    if (!sdlsurface) return;
    if ( x < 0 ) x = sdlsurface->w+x-1;
    offset = (uint8_t *)sdlsurface->pixels + y*sdlsurface->pitch + x;
    while ( height-- )
    {
        memcpy(offset, pbitmap, width);
        offset += sdlsurface->pitch;
        pbitmap += width;
    }
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
    if (!sdlsurface) return;
    if (x < 0) x = sdlsurface->w+x-1;
    //SDL_UpdateRect(screen, x, y, width, height);
}


DECLSPEC int SDLCALL SDL_FillRect (SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
    return 0;
}

/*
================
Sys_SendKeyEvents
================
*/

const kbdmap_t gamepad_to_kbd_map[JOY_STD_MAX] =
{
    [JOY_UPARROW]       = {SDLK_UP, 0},
    [JOY_DOWNARROW]     = {SDLK_DOWN, 0},
    [JOY_LEFTARROW]     = {SDLK_LEFT,0},
    [JOY_RIGHTARROW]    = {SDLK_RIGHT, 0},
    [JOY_K1]            = {' ', PAD_FREQ_LOW},
    [JOY_K4]            = {'z',  0},
    [JOY_K3]            = {SDLK_LCTRL, 0},
    [JOY_K2]            = {'a',    0},
    [JOY_K5]            = {',',    0},
    [JOY_K6]            = {'.',    0},
    [JOY_K7]            = {'[',  0},
    [JOY_K8]            = {'\'', 0},
    [JOY_K9]            = {SDLK_RETURN, 0},
    [JOY_K10]           = {SDLK_ESCAPE, PAD_FREQ_LOW},
};

i_event_t *input_post_key (i_event_t  *evts, i_event_t event)
{
    root_sdl_event_filter(&event);
    return NULL;
}

void Sys_SendKeyEvents(i_event_t *evts)
{
    input_proc_keys(NULL);
}

void IN_Init (void)
{
    input_soft_init(gamepad_to_kbd_map);
    input_bind_extra(K_EX_LOOKUP, SDLK_HOME);
    input_bind_extra(K_EX_LOOKUP, SDLK_DELETE);
    input_bind_extra(K_EX_LOOKUP, SDLK_INSERT);
}

void IN_Shutdown (void)
{

}

void IN_Commands (void)
{
}

void IN_Move (void *cmd)
{
}

/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
    return 0;
}
