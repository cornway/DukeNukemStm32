// vid_sdl.h -- sdl video driver 

#include "gfx.h"
#include "sdl_video.h"
#include "lcd_main.h"
#include "input_main.h"
#include "sdl_keysym.h"
#include <vid.h>

#define VIDEO_IN_IRAM 1

viddef_t    vid;                // global video state
unsigned short  d_8to16table[256];

// The original defaults
//#define    BASEWIDTH    320
//#define    BASEHEIGHT   200
// Much better for high resolution displays
#define    BASEWIDTH    (320)
#define    BASEHEIGHT   (200)

#define D_SCREEN_PIX_CNT (BASEWIDTH * BASEHEIGHT)
#define D_SCREEN_BYTE_CNT (D_SCREEN_PIX_CNT * sizeof(pix_t))

#if VIDEO_IN_IRAM
pix_t screenbuf[BASEWIDTH * BASEHEIGHT * sizeof(pix_t) + sizeof(SDL_Surface)] = {0};
#endif

int    VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes = 0;
uint8_t    *VGA_pagebase;

static SDL_Surface *screen = NULL;

static boolean mouse_avail;
static float   mouse_x, mouse_y;
static int mouse_oldbuttonstate = 0;

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

void    VID_Init (unsigned char *palette)
{
    int chunk;
    uint8_t *cache;
    int cachesize;
    Uint32 flags;
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
    screen = (SDL_Surface *)&screenbuf[0];
#else
    screen = (SDL_Surface *)Sys_Malloc(BASEWIDTH * BASEHEIGHT * sizeof(pix_t) + sizeof(SDL_Surface), "screen");
    if (screen == NULL)
        Sys_Error ("Not enough memory for video mode\n");
#endif

    // Set video width, height and flags
    flags = (SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);

    memset(screen, 0, sizeof(SDL_Surface));

    screen->pixels = (void *)(screen + 1);
    screen->flags = flags;
    screen->w = BASEWIDTH;
    screen->h = BASEHEIGHT;
    screen->offset = 0;
    screen->pitch = BASEWIDTH;

    VID_SetPalette(palette);

    // now know everything we need to know about the buffer
    VGA_width = vid.conwidth = vid.width;
    VGA_height = vid.conheight = vid.height;
    vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
    vid.numpages = 1;
    vid.colormap = 0;//host_colormap;
    vid.fullbright = 256 - (*((int *)vid.colormap + 2048));
    VGA_pagebase = vid.buffer = screen->pixels;
    VGA_rowbytes = vid.rowbytes = screen->pitch;
    vid.conbuffer = vid.buffer;
    vid.conrowbytes = vid.rowbytes;
    vid.direct = 0;
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


    if (!screen) return;
    if ( x < 0 ) x = screen->w+x-1;
    offset = (uint8_t *)screen->pixels + y*screen->pitch + x;
    while ( height-- )
    {
        memcpy(offset, pbitmap, width);
        offset += screen->pitch;
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
    if (!screen) return;
    if (x < 0) x = screen->w+x-1;
    //SDL_UpdateRect(screen, x, y, width, height);
}


/*
================
Sys_SendKeyEvents
================
*/

const kbdmap_t gamepad_to_kbd_map[JOY_STD_MAX] =
{
#if 0
    [JOY_UPARROW]       = {K_UPARROW, 0},
    [JOY_DOWNARROW]     = {K_DOWNARROW, 0},
    [JOY_LEFTARROW]     = {K_LEFTARROW,0},
    [JOY_RIGHTARROW]    = {K_RIGHTARROW, 0},
    [JOY_K1]            = {'/', PAD_FREQ_LOW},
    [JOY_K4]            = {K_END,  0},
    [JOY_K3]            = {K_CTRL, 0},
    [JOY_K2]            = {K_SPACE,    0},
    [JOY_K5]            = {'a',    0},
    [JOY_K6]            = {'d',    0},
    [JOY_K7]            = {K_DEL,  0},
    [JOY_K8]            = {K_PGDN, 0},
    [JOY_K9]            = {K_ENTER, 0},
    [JOY_K10]           = {K_ESCAPE, PAD_FREQ_LOW},
#endif
    0,
};

void input_post_key (i_event_t event)
{

}

void Sys_SendKeyEvents(void)
{
    input_proc_keys();
}

void IN_Init (void)
{
#if 0
    input_soft_init(gamepad_to_kbd_map);
    input_bind_extra(K_EX_LOOKUP, K_HOME);
    input_bind_extra(K_EX_LOOKUP, K_DEL);
    input_bind_extra(K_EX_LOOKUP, K_INS);

    if ( COM_CheckParm ("-nomouse") )
        return;
    mouse_x = mouse_y = 0.0;
    mouse_avail = 1;
#endif
}

void IN_Shutdown (void)
{
    mouse_avail = 0;
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
