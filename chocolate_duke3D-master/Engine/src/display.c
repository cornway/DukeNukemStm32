/*
 * An SDL replacement for BUILD's VESA code.
 *
 *  Written by Ryan C. Gordon. (icculus@clutteredmind.org)
 *
 * Please do NOT harrass Ken Silverman about any code modifications
 *  (including this file) to BUILD.
 */

/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file IS NOT A PART OF Ken Silverman's original release
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "platform.h"
#include "SDL_Video.h"
#include "SDL_Keysym.h"
#include "unix_compat.h"
#include <gfx.h>
#include <input_main.h>
#include "keyboard.h"
#include "sndcards.h"

#if (!defined PLATFORM_SUPPORTS_SDL)
#error This platform apparently does not use SDL. Do not compile this.
#endif

extern int DSL_Init( void );

#define BUILD_NOMOUSEGRAB    "BUILD_NOMOUSEGRAB"
#define BUILD_WINDOWED       "BUILD_WINDOWED"
#define BUILD_SDLDEBUG       "BUILD_SDLDEBUG"
#define BUILD_RENDERER       "BUILD_RENDERER"
#define BUILD_GLLIBRARY      "BUILD_GLLIBRARY"
#define BUILD_USERSCREENRES  "BUILD_USERSCREENRES"
#define BUILD_MAXSCREENRES   "BUILD_MAXSCREENRES"
#define BUILD_HALLOFMIRRORS  "BUILD_HALLOFMIRRORS"
#define BUILD_GLDUMP         "BUILD_GLDUMP"
#define BUILD_SDLJOYSTICK    "BUILD_SDLJOYSTICK"

#include "build.h"
#include "display.h"
#include "fixedPoint_math.h"
#include "engine.h"
#include "network.h"

#include "mmulti_unstable.h"
#include "mmulti_stable.h"
#include "network.h"
#include "icon.h"

// NATIVE TIMER FUNCTION DECLARATION
/*
 FCS: The timer section sadly uses Native high precision calls to implement timer functions.
 QueryPerformanceFrequency and QueryPerformanceCounter
 it seems SDL precision was not good enough (or rather using unaccurate OS functions) to replicate
 a DOS timer.
 */

int TIMER_GetPlatformTicksInOneSecond(int64_t* t);
void TIMER_GetPlatformTicks(int64_t* t);

//END // NATIVE TIMER FUNCTION DECLARATION






#if ((defined PLATFORM_WIN32))
#include <windows.h>
#endif

#include "draw.h"
#include "cache.h"
#include "SDL_Keysym.h"


/*
 * !!! remove the surface_end checks, for speed's sake. They are a
 * !!!  needed safety right now. --ryan.
 */


#define DEFAULT_MAXRESWIDTH  320
#define DEFAULT_MAXRESHEIGHT 200

#define UNLOCK_SURFACE_AND_RETURN  /*if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface); */return;

    /* !!! move these elsewhere? */
int32_t xres, yres, bytesperline, imageSize, maxpages;
uint8_t* frameplace;

//The frambuffer address
uint8_t* frameoffset;
uint8_t  *screen_raw, vesachecked;
int32_t buffermode, origbuffermode, linearmode;
uint8_t  permanentupdate = 0, vgacompatible;

SDL_Surface *surface = NULL; /* This isn't static so that we can use it elsewhere AH */

static uint32_t sdl_flags = SDL_HWPALETTE;
static int32_t mouse_relative_x = 0;
static int32_t mouse_relative_y = 0;
static short mouse_buttons = 0;
static unsigned int lastkey = 0;
/* so we can make use of setcolor16()... - DDOI */
static uint8_t  drawpixel_color=0;

static uint32_t scancodes[SDLK_LAST];

static int32_t last_render_ticks = 0;
int32_t total_render_time = 1;
int32_t total_rendered_frames = 0;

static char *titleNameLong = NULL;
static char *titleNameShort = NULL;

void restore256_palette (void);
void set16color_palette (void);

int _argc;
char  **_argv;

static void __append_sdl_surface_flag(SDL_Surface *_surface, char  *str,
                                      size_t strsize, Uint32 flag,
                                      const char  *flagstr)
{
    if (_surface->flags & flag)
    {
        if ( (strlen(str) + strlen(flagstr)) >= (strsize - 1) )
            strcpy(str + (strsize - 5), " ...");
        else
            strcat(str, flagstr);
    } /* if */
}


#define append_sdl_surface_flag(a, b, c, fl) __append_sdl_surface_flag(a, b, c, fl, " " #fl)
#define print_tf_state(str, val) dprintf("%s: {%s}\n", str, (val) ? "true" : "false" )

static void output_surface_info(SDL_Surface *_surface)
{
    const SDL_VideoInfo *info;
    char  f[256];


    if (_surface == NULL)
    {
        dprintf("-WARNING- You've got a NULL screen surface!");
    }
    else
    {
        f[0] = '\0';
        dprintf("screen surface is (%dx%dx%dbpp).\n",_surface->w, _surface->h, _surface->format->BitsPerPixel);

        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_SWSURFACE);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_HWSURFACE);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_ASYNCBLIT);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_ANYFORMAT);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_HWPALETTE);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_DOUBLEBUF);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_FULLSCREEN);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_OPENGL);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_OPENGLBLIT);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_RESIZABLE);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_NOFRAME);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_HWACCEL);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_SRCCOLORKEY);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_RLEACCELOK);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_RLEACCEL);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_SRCALPHA);
        append_sdl_surface_flag(_surface, f, sizeof (f), SDL_PREALLOC);

        if (f[0] == '\0')
            strcpy(f, " (none)");

        dprintf("New vidmode flags:%s.\n", f);

        //info = SDL_GetVideoInfo();
        //assert(info != NULL);
/*
        print_tf_state("hardware surface available", info->hw_available);
        print_tf_state("window manager available", info->wm_available);
        print_tf_state("accelerated hardware->hardware blits", info->blit_hw);
        print_tf_state("accelerated hardware->hardware colorkey blits", info->blit_hw_CC);
        print_tf_state("accelerated hardware->hardware alpha blits", info->blit_hw_A);
        print_tf_state("accelerated software->hardware blits", info->blit_sw);
        print_tf_state("accelerated software->hardware colorkey blits", info->blit_sw_CC);
        print_tf_state("accelerated software->hardware alpha blits", info->blit_sw_A);
        print_tf_state("accelerated color fills", info->blit_fill);

        printf("video memory: (%d),\n", info->video_mem);
 */
    }
}


static void output_driver_info(void)
{
#ifdef ORIGCODE
    char  buffer[256];

    if (SDL_VideoDriverName(buffer, sizeof (buffer)) == NULL){
        dprintf("-WARNING- SDL_VideoDriverName() returned NULL!");
    } /* if */
    else
    {
        dprintf("Using SDL video driver \"%s\".", buffer);
    } /* else */
#endif
} /* output_driver_info */


void* get_framebuffer(void){
    return((Uint8 *) surface->pixels);
}



/*
 * !!! This is almost an entire copy of the original setgamemode().
 * !!!  Figure out what is needed for just 2D mode, and separate that
 * !!!  out. Then, place the original setgamemode() back into engine.c,
 * !!!  and remove our simple implementation (and this function.)
 * !!!  Just be sure to keep the non-DOS things, like the window's
 * !!!  titlebar caption.   --ryan.
 */
static uint8_t  screenalloctype = 255;
static void init_new_res_vars(int32_t davidoption)
{
    int i = 0;
    int j = 0;

    setupmouse();

    //SDL_WM_SetCaption(titleNameLong, titleNameShort);

    xdim = xres = surface->w;
    ydim = yres = surface->h;

	dprintf("init_new_res_vars %d %d\n",xdim,ydim);

    bytesperline = surface->w;
    vesachecked = 1;
    vgacompatible = 1;
    linearmode = 1;
	qsetmode = surface->h;
	activepage = visualpage = 0;

     
    frameoffset = frameplace = (uint8_t*)surface->pixels;

  	if (screen != NULL)
   	{
       	if (screenalloctype == 0) kkfree((void *)screen);
   	    if (screenalloctype == 1) suckcache((int32_t *)screen);
   		screen = NULL;
   	} /* if */


    	switch(vidoption)
    	{
    		case 1:i = xdim*ydim; break;
    		case 2: xdim = 320; ydim = 200; i = xdim*ydim; break;
    		
    		default: assert(0);
    	}
    	j = ydim*4*sizeof(int32_t);  /* Leave room for horizlookup&horizlookup2 */

		if(horizlookup)
			Sys_Free(horizlookup);

		if(horizlookup2)
			Sys_Free(horizlookup2);
		
		horizlookup = (int32_t*)Sys_Malloc(j);
		horizlookup2 = (int32_t*)Sys_Malloc(j);

    j = 0;
    
    //Build lookup table (X screespace -> frambuffer offset. 
  	for(i = 0; i <= ydim; i++)
    {
        ylookup[i] = j;
        j += bytesperline;
    }

   	horizycent = ((ydim*4)>>1);

    /* Force drawrooms to call dosetaspect & recalculate stuff */
	oxyaspect = oxdimen = oviewingrange = -1;

    //Let the Assembly module how many pixels to skip when drawing a column
	setBytesPerLine(bytesperline);

    
    setview(0L,0L,xdim-1,ydim-1);
    
	setbrightness(curbrightness, palette);

	if (searchx < 0) {
        searchx = halfxdimen;
        searchy = (ydimen>>1);
    }
    
}



static void go_to_new_vid_mode(int davidoption, int w, int h)
{
    getvalidvesamodes();
    SDL_ClearError();
    // don't do SDL_SetVideoMode if SDL_WM_SetIcon not called. See sdl doc for SDL_WM_SetIcon
	surface = SDL_SetVideoMode(w, h, 8, sdl_flags);
    if (surface == NULL)
    {
		Error(EXIT_FAILURE,	"BUILDSDL: Failed to set %dx%d video mode!\n"
							"BUILDSDL: SDL_Error() says [%s].\n",
							w, h, SDL_GetError());
	} /* if */

    output_surface_info(surface);
    init_new_res_vars(davidoption); // dont be confused between vidoption (global) and davidoption
}

static __inline int sdl_mouse_button_filter(void const *event)
{
#ifdef ORIGCODE
        /*
         * What bits BUILD expects:
         *  0 left button pressed if 1
         *  1 right button pressed if 1
         *  2 middle button pressed if 1
         *
         *   (That is, this is what Int 33h (AX=0x05) returns...)
         *
         *  additionally bits 3&4 are set for the mouse wheel
         */
    Uint8 button = event->button;
    if (button >= sizeof (mouse_buttons) * 8)
        return(0);

    if (button == SDL_BUTTON_RIGHT)
        button = SDL_BUTTON_MIDDLE;
    else if (button == SDL_BUTTON_MIDDLE)
        button = SDL_BUTTON_RIGHT;

    if (((const SDL_MouseButtonEvent*)event)->state)
        mouse_buttons |= 1<<(button-1);
    else if (button != 4 && button != 5)
        mouse_buttons ^= 1<<(button-1);
#if 0
    Uint8 bmask = SDL_GetMouseState(NULL, NULL);
    mouse_buttons = 0;
    if (bmask & SDL_BUTTON_LMASK) mouse_buttons |= 1;
    if (bmask & SDL_BUTTON_RMASK) mouse_buttons |= 2;
    if (bmask & SDL_BUTTON_MMASK) mouse_buttons |= 4;
#endif
#endif
    return(0);
} /* sdl_mouse_up_filter */


static int sdl_mouse_motion_filter(i_event_t const *event)
{
#ifdef ORIGCODE

    if (surface == NULL)
		return(0);

    if (event->type == SDL_JOYBALLMOTION)
    {
        mouse_relative_x = event->jball.xrel/100;
        mouse_relative_y = event->jball.yrel/100;
    }
    else
    {
			if (SDL_WM_GrabInput(SDL_GRAB_QUERY)==SDL_GRAB_ON) 
			{
				mouse_relative_x += event->motion.xrel;
	       	    mouse_relative_y += event->motion.yrel;
				//printf("sdl_mouse_motion_filter: mrx=%d, mry=%d, mx=%d, my=%d\n",
				//	mouse_relative_x, mouse_relative_y, event->motion.xrel, event->motion.yrel);
				
				// mouse_relative_* is already reset in 
				// readmousexy(). It must not be
				// reset here because calling this function does not mean
				// we always handle the mouse. 
				// FIX_00001: Mouse speed is uneven and slower in windowed mode vs fullscreen mode.
			}
			else
				mouse_relative_x = mouse_relative_y = 0;
	}
#endif
    return(0);
} /* sdl_mouse_motion_filter */


    /*
     * The windib driver can't alert us to the keypad enter key, which
     *  Ken's code depends on heavily. It sends it as the same key as the
     *  regular return key. These users will have to hit SHIFT-ENTER,
     *  which we check for explicitly, and give the engine a keypad enter
     *  enter event.
     */
static __inline int handle_keypad_enter_hack(const i_event_t *event)
{
    static int kp_enter_hack = 0;
    int retval = 0;

    if (event->sym == SDLK_RETURN)
    {
        if (event->state == keydown)
        {
#if 0 /*FIXME :*/
            if (event->key.keysym.mod & KMOD_SHIFT)
            {
                kp_enter_hack = 1;
                lastkey = scancodes[SDLK_KP_ENTER];
                retval = 1;
            } /* if */
#endif
        } /* if */

        else  /* key released */
        {
            if (kp_enter_hack)
            {
                kp_enter_hack = 0;
                lastkey = scancodes[SDLK_KP_ENTER];
                retval = 1;
            } /* if */
        } /* if */
    } /* if */

    return(retval);
} /* handle_keypad_enter_hack */

void fullscreen_toggle_and_change_driver(void)
{
	
//  FIX_00002: New Toggle Windowed/FullScreen system now simpler and will 
//  dynamically change for Windib or Directx driver. Windowed/Fullscreen 
//  toggle also made available from menu.
//  Replace attempt_fullscreen_toggle(SDL_Surface **surface, Uint32 *flags)
  	
	int32_t x,y;
	x = surface->w;
	y = surface->h;

	BFullScreen =!BFullScreen;
#ifdef ORIGCODE
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
	_platform_init(0, NULL, "Duke Nukem 3D", "Duke3D");
	_setgamemode(ScreenMode,x,y);
	//vscrn();

	return;
}

static int sdl_key_filter(const i_event_t *event)
{
    int extended;

    if ( (event->sym == SDLK_m) &&
         (event->state == keydown) && 0
    /*FIXME : (event->key.keysym.mod & KMOD_CTRL) */)
    {


		// FIX_00005: Mouse pointer can be toggled on/off (see mouse menu or use CTRL-M)
		// This is usefull to move the duke window when playing in window mode.
#ifdef ORIGCODE

        if (SDL_WM_GrabInput(SDL_GRAB_QUERY)==SDL_GRAB_ON) 
		{
            SDL_WM_GrabInput(SDL_GRAB_OFF);

			SDL_ShowCursor(1);
		}
		else
		{
            SDL_WM_GrabInput(SDL_GRAB_ON);
			SDL_ShowCursor(0);

		}
#endif
        return(0);
    } /* if */

    else if ( ( (event->sym == SDLK_RETURN) ||
                (event->sym == SDLK_KP_ENTER) ) &&
              (event->state == keydown) && 0
              /* FIXME :(event->key.keysym.mod & KMOD_ALT) */)
    {	fullscreen_toggle_and_change_driver();

		// hack to discard the ALT key...
		lastkey=scancodes[SDLK_RALT]>>8; // extended
		keyhandler();
		lastkey=(scancodes[SDLK_RALT]&0xff)+0x80; // Simulating Key up
		keyhandler();
		lastkey=(scancodes[SDLK_LALT]&0xff)+0x80; // Simulating Key up (not extended)
		keyhandler();
#ifdef ORIGCODE
		SDL_SetModState(KMOD_NONE); // SDL doesnt see we are releasing the ALT-ENTER keys
#endif
		return(0);					
    }								

    if (!handle_keypad_enter_hack(event))
        lastkey = scancodes[event->sym];

//	printf("key.keysym.sym=%d\n", event->key.keysym.sym);

    if (lastkey == 0x0000)   /* No DOS equivalent defined. */
        return(0);

    extended = ((lastkey & 0xFF00) >> 8);
    if (extended != 0)
    {
        lastkey = extended;
        keyhandler();
        lastkey = (scancodes[event->sym] & 0xFF);
    } /* if */

    if (event->state == keyup)
        lastkey += 128;  /* +128 signifies that the key is released in DOS. */

    keyhandler();
    return(0);
} /* sdl_key_filter */


int root_sdl_event_filter(const i_event_t *event)
{
#ifdef ORIGCODE
    switch (event->type)
    {
        case SDL_KEYUP:
            // FIX_00003: Pause mode is now fully responsive - (Thx to Jonathon Fowler tips)
			if(event->key.keysym.sym == SDLK_PAUSE)
				break;
        case SDL_KEYDOWN:
            return(sdl_key_filter(event));
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            {
                //Do Nothing

                //printf("Joybutton UP/DOWN\n");
	            //return(sdl_joystick_button_filter((const SDL_MouseButtonEvent*)event));
                return 0;
            }
        case SDL_JOYBALLMOTION:
        case SDL_MOUSEMOTION:
            return(sdl_mouse_motion_filter(event));
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
			return(sdl_mouse_button_filter((const SDL_MouseButtonEvent*)event));
        case SDL_QUIT:
            /* !!! rcg TEMP */
            Error(0, "Exit through SDL\n"); 
		default:
			//printf("This event is not handled: %d\n",event->type);
			break;
    } /* switch */
#else
        return(sdl_key_filter(event));
#endif
    return(1);
} /* root_sdl_event_filter */

extern void Sys_SendKeyEvents(i_event_t *evts);

static void handle_events(void)
{
#ifdef ORIGCODE
	while(SDL_PollEvent(&event))
        root_sdl_event_filter(&kbdevent);
#else
        Sys_SendKeyEvents(NULL);
#endif
    /*TODO : */
} /* handle_events */


/* bleh...public version... */
void _handle_events(void)
{
    handle_events();
} /* _handle_events */


static void *joystick = NULL;
void _joystick_init(void)
{
#ifdef ORIGCODE
    const char  *envr = getenv(BUILD_SDLJOYSTICK);
    int favored = 0;
    int numsticks;
    int i;

    if (joystick != NULL)
    {
        sprintf("Joystick appears to be already initialized.\n");
        sprintf("...deinitializing for stick redetection...\n");
        _joystick_deinit();
    } /* if */

    if ((envr != NULL) && (strcmp(envr, "none") == 0))
    {
        sprintf("Skipping joystick detection/initialization at user request\n");
        return;
    } /* if */

    sprintf("Initializing SDL joystick subsystem...");
    sprintf(" (export environment variable BUILD_SDLJOYSTICK=none to skip)\n");

    if (SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE) != 0)
    {
        sprintf("SDL_Init(SDL_INIT_JOYSTICK) failed: [%s].\n", SDL_GetError());
        return;
    } /* if */

    numsticks = SDL_NumJoysticks();
    sprintf("SDL sees %d joystick%s.\n", numsticks, numsticks == 1 ? "" : "s");
    if (numsticks == 0)
        return;

    for (i = 0; i < numsticks; i++)
    {
        const char  *stickname = SDL_JoystickName(i);
        if ((envr != NULL) && (strcmp(envr, stickname) == 0))
            favored = i;

        dprintf("Stick #%d: [%s]\n", i, stickname);
    } /* for */

    dprintf("Using Stick #%d.", favored);
    if ((envr == NULL) && (numsticks > 1))
        dprintf("Set BUILD_SDLJOYSTICK to one of the above names to change.\n");

    joystick = SDL_JoystickOpen(favored);
    if (joystick == NULL)
    {
        dprintf("Joystick #%d failed to init: %s\n", favored, SDL_GetError());
        return;
    } /* if */
    dprintf("Joystick initialized. %d axes, %d buttons, %d hats, %d balls.\n",
              SDL_JoystickNumAxes(joystick), SDL_JoystickNumButtons(joystick),
              SDL_JoystickNumHats(joystick), SDL_JoystickNumBalls(joystick));
    SDL_JoystickEventState(SDL_QUERY);
#endif
} /* _joystick_init */


void _joystick_deinit(void)
{
#ifdef ORIGCODE
    if (joystick != NULL)
    {
        dprintf("Closing joystick device...\n");
        SDL_JoystickClose(joystick);
        dprintf("Joystick device closed. Deinitializing SDL subsystem...\n");
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        dprintf("SDL joystick subsystem deinitialized.\n");
        joystick = NULL;
    } /* if */
#endif
} /* _joystick_deinit */


int _joystick_update(void)
{
#ifdef ORIGCODE
    if (joystick == NULL)
        return(0);

    SDL_JoystickUpdate();
    return(1);
#endif
} /* _joystick_update */


int _joystick_axis(int axis)
{
#ifdef ORIGCODE
    if (joystick == NULL)
    {   
        return(0);
    }

    return(SDL_JoystickGetAxis(joystick, axis));
#endif
} /* _joystick_axis */

int _joystick_hat(int hat)
{
#ifdef ORIGCODE
    if (joystick == NULL)
    {   
        return(-1);
    }

    return(SDL_JoystickGetHat(joystick, hat));
#endif
} /* _joystick_axis */

int _joystick_button(int button)
{
#ifdef ORIGCODE
    if (joystick == NULL)
        return(0);

    return(SDL_JoystickGetButton(joystick, button) != 0);
#endif
} /* _joystick_button */


uint8_t  _readlastkeyhit(void)
{
    return(lastkey);
} /* _readlastkeyhit */



#if (!defined __DATE__)
#define __DATE__ "a long, int32_t time ago"
#endif

static void output_sdl_versions(void)
{
#ifdef ORIGCODE
    const SDL_version *linked_ver = SDL_Linked_Version();
    SDL_version compiled_ver;

    SDL_VERSION(&compiled_ver);

    dprintf("SDL display driver for the BUILD engine initializing.\n");
    dprintf("  sdl_driver.c by Ryan C. Gordon (icculus@clutteredmind.org).\n");
    dprintf("Compiled %s against SDL version %d.%d.%d ...\n", __DATE__,
                compiled_ver.major, compiled_ver.minor, compiled_ver.patch);
    dprintf("Linked SDL version is %d.%d.%d ...\n",
                linked_ver->major, linked_ver->minor, linked_ver->patch);
#endif
} /* output_sdl_versions */


/* lousy -ansi flag.  :) */
static char  *string_dupe(const char  *str)
{
    char  *retval = (char *)Sys_Malloc(strlen(str) + 1);
    if (retval != NULL)
        strcpy(retval, str);
    return(retval);
} /* string_dupe */







void _platform_init(int argc, char  **argv, const char  *title, const char  *iconName)
{
    int i;
	int64_t timeElapsed;
	char  dummyString[4096];

	// FIX_00061: "ERROR: Two players have the same random ID" too frequent cuz of internet windows times
    TIMER_GetPlatformTicks(&timeElapsed);
	srand(timeElapsed&0xFFFFFFFF);

	Setup_UnstableNetworking();

    // Look through the command line args
    for(i = 0; i < argc; i++)
    {
        if(argv[i][0] == '-' )
        {
            if(strcmpi(argv[i], "-netmode_stable") == 0)
            {
                //fullscreen = 1;
				//TODO:
//TODO ( "[Todo: handle -netmode <int>]" )
				Setup_StableNetworking();
					
            }
        }
    }
    
    

#ifdef __APPLE__
    SDL_putenv("SDL_VIDEODRIVER=Quartz");
#endif
  	
#ifdef ORIGCODE
    if (SDL_Init(SDL_INIT_VIDEO) == -1){
        Error(EXIT_FAILURE, "BUILDSDL: SDL_Init() failed!\nBUILDSDL: SDL_GetError() says \"%s\".\n", SDL_GetError());
    } 
#endif

	// Set up the correct renderer
	// Becarfull setenv can't reach dll in VC++
	// A way to proceed is to integrate the SDL libs
	// in the exe instead.
	
    // FIX_00004: SDL.dll and SDL_Mixer.dll are now integrated within the exe
    // (this also makes the Windib/Directx driver switching easier with SDL)		

    // This requires to recompile the whole sdl and sdl mixer with the lib
    // switch instead of the default dll switch.
#ifdef ORIGCODE
	putenv("SDL_VIDEO_CENTERED=1");
#endif
    if (title == NULL)
        title = "BUILD";

    if (iconName == NULL)
        iconName = "BUILD";

    titleNameLong = string_dupe(title);
    titleNameShort = string_dupe(iconName);

    sdl_flags = BFullScreen ? SDL_FULLSCREEN : 0;

    sdl_flags |= SDL_HWPALETTE;


    memset(scancodes, '\0', sizeof (scancodes));
    scancodes[SDLK_ESCAPE]          = sc_Escape;
    scancodes[SDLK_1]               = sc_1;
    scancodes[SDLK_2]               = sc_2;
    scancodes[SDLK_3]               = sc_3;
    scancodes[SDLK_4]               = sc_4;
    scancodes[SDLK_5]               = sc_5;
    scancodes[SDLK_6]               = sc_6;
    scancodes[SDLK_7]               = sc_7;
    scancodes[SDLK_8]               = sc_8;
    scancodes[SDLK_9]               = sc_9;
    scancodes[SDLK_0]               = sc_0;
    scancodes[SDLK_MINUS]           = sc_Minus; /* was 0x4A */
    scancodes[SDLK_EQUALS]          = sc_Equals; /* was 0x4E */
    scancodes[SDLK_BACKSPACE]       = sc_BackSpace;
    scancodes[SDLK_TAB]             = sc_Tab;
    scancodes[SDLK_q]               = sc_Q;
    scancodes[SDLK_w]               = sc_W;
    scancodes[SDLK_e]               = sc_E;
    scancodes[SDLK_r]               = sc_R;
    scancodes[SDLK_t]               = sc_T;
    scancodes[SDLK_y]               = sc_Y;
    scancodes[SDLK_u]               = sc_U;
    scancodes[SDLK_i]               = sc_I;
    scancodes[SDLK_o]               = sc_O;
    scancodes[SDLK_p]               = sc_P;
    scancodes[SDLK_LEFTBRACKET]     = sc_OpenBracket;
    scancodes[SDLK_RIGHTBRACKET]    = sc_CloseBracket;
    scancodes[SDLK_RETURN]          = sc_Return;
    scancodes[SDLK_LCTRL]           = sc_LeftControl;
    scancodes[SDLK_a]               = sc_A;
    scancodes[SDLK_s]               = sc_S;
    scancodes[SDLK_d]               = sc_D;
    scancodes[SDLK_f]               = sc_F;
    scancodes[SDLK_g]               = sc_G;
    scancodes[SDLK_h]               = sc_H;
    scancodes[SDLK_j]               = sc_J;
    scancodes[SDLK_k]               = sc_K;
    scancodes[SDLK_l]               = sc_L;
    scancodes[SDLK_SEMICOLON]       = sc_SemiColon;
    scancodes[SDLK_QUOTE]           = sc_Quote;
    scancodes[SDLK_BACKQUOTE]       = sc_Tilde;/*FIXME*/
    scancodes[SDLK_LSHIFT]          = sc_LeftShift;
    scancodes[SDLK_BACKSLASH]       = sc_BackSlash;
    scancodes[SDLK_z]               = sc_Z;
    scancodes[SDLK_x]               = sc_X;
    scancodes[SDLK_c]               = sc_C;
    scancodes[SDLK_v]               = sc_V;
    scancodes[SDLK_b]               = sc_B;
    scancodes[SDLK_n]               = sc_N;
    scancodes[SDLK_m]               = sc_M;
    scancodes[SDLK_COMMA]           = sc_Comma;
    scancodes[SDLK_PERIOD]          = sc_Period;
    scancodes[SDLK_SLASH]           = sc_Slash;
    scancodes[SDLK_RSHIFT]          = sc_RightShift;
    scancodes[SDLK_KP_MULTIPLY]     = -1;
    scancodes[SDLK_LALT]            = sc_LeftAlt;
    scancodes[SDLK_SPACE]           = sc_Space;
    scancodes[SDLK_CAPSLOCK]        = sc_CapsLock;
    scancodes[SDLK_F1]              = sc_F1;
    scancodes[SDLK_F2]              = sc_F2;
    scancodes[SDLK_F3]              = sc_F3;
    scancodes[SDLK_F4]              = sc_F4;
    scancodes[SDLK_F5]              = sc_F5;
    scancodes[SDLK_F6]              = sc_F6;
    scancodes[SDLK_F7]              = sc_F7;
    scancodes[SDLK_F8]              = sc_F8;
    scancodes[SDLK_F9]              = sc_F9;
    scancodes[SDLK_F10]             = sc_F10;
    scancodes[SDLK_NUMLOCK]         = sc_NumLock;
    scancodes[SDLK_SCROLLOCK]       = sc_ScrollLock;
    scancodes[SDLK_KP7]             = -1;
    scancodes[SDLK_KP8]             = -1;
    scancodes[SDLK_KP9]             = -1;
    scancodes[SDLK_KP_MINUS]        = sc_Minus;
    scancodes[SDLK_KP4]             = -1;
    scancodes[SDLK_KP5]             = -1;
    scancodes[SDLK_KP6]             = -1;
    scancodes[SDLK_KP_PLUS]         = sc_Plus;
    scancodes[SDLK_KP1]             = -1;
    scancodes[SDLK_KP2]             = -1;
    scancodes[SDLK_KP3]             = -1;
    scancodes[SDLK_KP0]             = -1;
    scancodes[SDLK_KP_PERIOD]       = -1;
    scancodes[SDLK_F11]             = sc_F11;
    scancodes[SDLK_F12]             = sc_F12;
    scancodes[SDLK_PAUSE]           = sc_Pause; /* SBF - technically incorrect */

    scancodes[SDLK_KP_ENTER]        = 0xE0 | sc_Enter;
    scancodes[SDLK_RCTRL]           = 0xE0 | sc_RightControl;
    scancodes[SDLK_KP_DIVIDE]       = 0xE0 | sc_kpad_Slash;
    scancodes[SDLK_PRINT]           = 0xE037; /* SBF - technically incorrect */
    scancodes[SDLK_SYSREQ]          = 0xE037; /* SBF - for windows... */
    scancodes[SDLK_RALT]            = 0xE038;
    scancodes[SDLK_HOME]            = 0xE047;
    scancodes[SDLK_UP]              = 0xE048;
    scancodes[SDLK_PAGEUP]          = 0xE049;
    scancodes[SDLK_LEFT]            = 0xE04B;
    scancodes[SDLK_RIGHT]           = 0xE04D;
    scancodes[SDLK_END]             = 0xE04F;
    scancodes[SDLK_DOWN]            = 0xE050;
    scancodes[SDLK_PAGEDOWN]        = 0xE051;
    scancodes[SDLK_INSERT]          = 0xE052;
    scancodes[SDLK_DELETE]          = 0xE053;
    
    

    output_sdl_versions();
    output_driver_info();

    DSL_Init();
    MV_Init(stm32769idisco, 11025, 16, 2, 16);
    /*int soundcard,
   int MixRate,
   int Voices,
   int numchannels,
   int samplebits*/
	//dprintf("Video Driver: '%s'.\n", SDL_VideoDriverName(dummyString, 20));

}

// Capture BMP of the current frame
int screencapture(char  *filename, uint8_t  inverseit)
{
//  FIX_00006: better naming system for screenshots + message when pic is taken. 
//  Use ./screenshots folder. Screenshot code rerwritten. Faster and
//  makes smaller files. Doesn't freeze or lag the game anymore.
  
	//SDL_SaveBMP(surface, filename);  
	return 0;
} /* screencapture */


void setvmode(int mode)
{
#ifdef ORIGCODE
    if (mode == 0x3)  /* text mode. */
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return;
    } else
        dprintf("setvmode(0x%x) is unsupported in SDL driver.\n", mode);
#endif
} 

int _setgamemode(uint8_t  davidoption, int32_t daxdim, int32_t daydim)
{
	int validated, i;
	SDL_Surface     *image;
	Uint32          colorkey;

    // don't override higher-res app icon on OS X
#ifndef PLATFORM_MACOSX
	// Install icon
	//image = SDL_LoadBMP_RW(SDL_RWFromMem(iconBMP, sizeof(iconBMP)), 1);
	//colorkey = 0; // index in this image to be transparent
    //SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey);
	//SDL_WM_SetIcon(image,NULL);
#endif
    
    if (daxdim > MAXXDIM || daydim > MAXYDIM)
    {
		dprintf("Resolution %dx%d is too high. Changed to %dx%d\n", daxdim, daydim, MAXXDIM,MAXYDIM);
	    daxdim = MAXXDIM;
	    daydim = MAXYDIM;
    } 

	getvalidvesamodes();

	validated = 0;
	for(i=0; i<validmodecnt; i++){
		if(validmodexdim[i] == daxdim && validmodeydim[i] == daydim)
			validated = 1;
	}

	if(!validated){
		dprintf("Resolution %dx%d unsupported. Changed to 640x480\n", daxdim, daydim);
	    daxdim = 640;
	    daydim = 480;
    }

    go_to_new_vid_mode((int) davidoption, daxdim, daydim);

    qsetmode = 200;
    last_render_ticks = getticks();

    return(0);
} /* setgamemode */


static int get_dimensions_from_str(const char  *str, int32_t *_w, int32_t *_h)
{
    char  *xptr = NULL;
    char  *ptr = NULL;
    int32_t w = -1;
    int32_t h = -1;

    if (str == NULL)
        return(0);

    xptr = strchr(str, 'x');
    if (xptr == NULL)
        return(0);

    w = strtol(str, &ptr, 10);
    if (ptr != xptr)
        return(0);

    xptr++;
    h = strtol(xptr, &ptr, 10);
    if ( (*xptr == '\0') || (*ptr != '\0') )
        return(0);

    if ((w <= 1) || (h <= 1))
        return(0);

    if (_w != NULL)
        *_w = w;

    if (_h != NULL)
        *_h = h;

    return(1);
} /* get_dimensions_from_str */


static __inline void get_max_screen_res(int32_t *max_w, int32_t *max_h)
{
    int32_t w = DEFAULT_MAXRESWIDTH;
    int32_t h = DEFAULT_MAXRESHEIGHT;
    const char  *envr = getenv(BUILD_MAXSCREENRES);

    if (envr != NULL)
    {
        if (!get_dimensions_from_str(envr, &w, &h))
        {
            dprintf("User's resolution ceiling [%s] is bogus!\n", envr);
            w = DEFAULT_MAXRESWIDTH;
            h = DEFAULT_MAXRESHEIGHT;
        } /* if */
    } /* if */

    if (max_w != NULL)
        *max_w = w;

    if (max_h != NULL)
        *max_h = h;
}


static void add_vesa_mode(const char  *typestr, int w, int h)
{
    //printf("Adding %s resolution (%dx%d).\n", typestr, w, h);
    validmode[validmodecnt] = validmodecnt;
    validmodexdim[validmodecnt] = w;
    validmodeydim[validmodecnt] = h;
    validmodecnt++;
} /* add_vesa_mode */


/* Let the user specify a specific mode via environment variable. */
static __inline void add_user_defined_resolution(void)
{
    int32_t w;
    int32_t h;
    const char  *envr = getenv(BUILD_USERSCREENRES);

    if (envr == NULL)
        return;

    if (get_dimensions_from_str(envr, &w, &h))
        add_vesa_mode("user defined", w, h);
    else
        dprintf("User defined resolution [%s] is bogus!\n", envr);
} /* add_user_defined_resolution */

static SDL_Rect sdl320x200 = {0, 0, 320, 200};

static SDL_Rect *phys_modes[] =
{
   &sdl320x200,
   NULL,
};

static __inline SDL_Rect **get_physical_resolutions(void)
{
#ifdef ORIGCODE
    const SDL_VideoInfo *vidInfo = SDL_GetVideoInfo();
    SDL_Rect **modes = SDL_ListModes(vidInfo->vfmt, sdl_flags | SDL_FULLSCREEN);
    if (modes == NULL)
    {
        sdl_flags &= ~SDL_FULLSCREEN;
        modes = SDL_ListModes(vidInfo->vfmt, sdl_flags); /* try without fullscreen. */
        if (modes == NULL)
            modes = (SDL_Rect **) -1;  /* fuck it. */
    } /* if */

    if (modes == (SDL_Rect **) -1)
        dprintf("Couldn't get any physical resolutions.\n");
    else
    {
        dprintf("Highest physical resolution is (%dx%d).\n",
                  modes[0]->w, modes[0]->h);
    } /* else */
#endif
    return(phys_modes);
} /* get_physical_resolutions */


static void remove_vesa_mode(int index, const char  *reason)
{
    int i;

    assert(index < validmodecnt);
    //printf("Removing resolution #%d, %dx%d [%s].\n",index, validmodexdim[index], validmodeydim[index], reason);

    for (i = index; i < validmodecnt - 1; i++)
    {
        validmode[i] = validmode[i + 1];
        validmodexdim[i] = validmodexdim[i + 1];
        validmodeydim[i] = validmodeydim[i + 1];
    } /* for */

    validmodecnt--;
} /* remove_vesa_mode */


static __inline void cull_large_vesa_modes(void)
{
    int32_t max_w;
    int32_t max_h;
    int i;
 
    get_max_screen_res(&max_w, &max_h);
    dprintf("Setting resolution ceiling to (%dx%d).\n", max_w, max_h);

    for (i = 0; i < validmodecnt; i++)
    {
        if ((validmodexdim[i] > max_w) || (validmodeydim[i] > max_h))
        {
            remove_vesa_mode(i, "above resolution ceiling");
            i--;  /* list shrinks. */
        } /* if */
    } /* for */
} /* cull_large_vesa_modes */


static __inline void cull_duplicate_vesa_modes(void)
{
    int i;
    int j;

    for (i = 0; i < validmodecnt; i++){
        for (j = i + 1; j < validmodecnt; j++){
            if ( (validmodexdim[i] == validmodexdim[j]) &&(validmodeydim[i] == validmodeydim[j]) ){
                remove_vesa_mode(j, "duplicate");
                j--;  /* list shrinks. */
            }
        }
    }
} 


#define swap_macro(tmp, x, y) { tmp = x; x = y; y = tmp; }

/* be sure to call cull_duplicate_vesa_modes() before calling this. */
static __inline void sort_vesa_modelist(void)
{
    int i;
    int sorted;
    int32_t tmp;

    do
    {
        sorted = 1;
        for (i = 0; i < validmodecnt - 1; i++)
        {
            if ( (validmodexdim[i] >= validmodexdim[i+1]) &&
                 (validmodeydim[i] >= validmodeydim[i+1]) )
            {
                sorted = 0;
                swap_macro(tmp, validmode[i], validmode[i+1]);
                swap_macro(tmp, validmodexdim[i], validmodexdim[i+1]);
                swap_macro(tmp, validmodeydim[i], validmodeydim[i+1]);
            } /* if */
        } /* for */
    } while (!sorted);
} /* sort_vesa_modelist */


static __inline void cleanup_vesa_modelist(void)
{
    cull_large_vesa_modes();
    cull_duplicate_vesa_modes();
    sort_vesa_modelist();
} /* cleanup_vesa_modelist */


static __inline void output_vesa_modelist(void)
{
    char  buffer[256];
    char  numbuf[20];
    int i;

    buffer[0] = '\0';

    for (i = 0; i < validmodecnt; i++)
    {
        sprintf(numbuf, " (%dx%d)",(int32_t) validmodexdim[i], (int32_t) validmodeydim[i]);

        if ( (strlen(buffer) + strlen(numbuf)) >= (sizeof (buffer) - 1) )
            strcpy(buffer + (sizeof (buffer) - 5), " ...");
        else
            strcat(buffer, numbuf);
    } /* for */

    dprintf("Final sorted modelist:%s", buffer);
} 


void getvalidvesamodes(void)
{
    static int already_checked = 0;
    int i;
    SDL_Rect **modes = NULL;
    int stdres[][2] = {
                        {320, 200}/*, {640, 350}, {640, 480},
                        {800, 600}, {1024, 768}*/
                      };

    if (already_checked)
        return;

    already_checked = 1;
   	validmodecnt = 0;
    vidoption = 1;  /* !!! tmp */

        /* fill in the standard resolutions... */
    for (i = 0; i < sizeof (stdres) / sizeof (stdres[0]); i++)
        add_vesa_mode("standard", stdres[i][0], stdres[i][1]);

         /* Anything the hardware can specifically do is added now... */
    modes = get_physical_resolutions();
    for (i = 0; (modes != (SDL_Rect **) -1) && (modes[i] != NULL); i++)
        add_vesa_mode("physical", modes[i]->w, modes[i]->h);

        /* Now add specific resolutions that the user wants... */
    add_user_defined_resolution();

        /* get rid of dupes and bogus resolutions... */
    cleanup_vesa_modelist();

        /* print it out for debugging purposes... */
    output_vesa_modelist();
} 

uint8_t lastPalette[768];
void WriteTranslucToFile(void){
    
    uint8_t buffer[65535*4];
    uint8_t tga_header[18];
    uint8_t* transPointer = transluc;
    uint8_t* bufferPointer = buffer;
    int i;
    int file;
    
    for (i=0; i < 65535; i++) {
        
        bufferPointer[0] = (lastPalette[(*transPointer)*3+0]) / 63.0 * 255;
        bufferPointer[1] = (lastPalette[(*transPointer)*3+1]) / 63.0 * 255;
        bufferPointer[2] = (lastPalette[(*transPointer)*3+2]) / 63.0 * 255;
        bufferPointer[3] = 255;
        
        dprintf("%d,",*transPointer);
        if (i%255 ==0)
            dprintf("\n");
        
        transPointer +=1;
        bufferPointer+=4;
    }
    
    
    
    d_open("transluc.tga", &file, "+w");
    if (file < 0) {
        return;
    }
    memset(tga_header, 0, 18);
    tga_header[2] = 2;
    tga_header[12] = (256 & 0x00FF);
    tga_header[13] = (256  & 0xFF00) / 256;
    tga_header[14] = (256  & 0x00FF) ;
    tga_header[15] =(256 & 0xFF00) / 256;
    tga_header[16] = 32 ;
    
    d_write(file, &tga_header, 18);
    d_write(file, buffer, 65535 * 4);
    d_close(file);
}

void WritePaletteToFile(uint8_t* palette,const char* filename,int width, int height){
    
    uint8_t tga_header[18];
    uint8_t* buffer;
    uint8_t* palettePointer = palette;
    uint8_t* bufferPointer ;
    int i;
    
    int file;

    d_open(filename, &file, "+w");

    memset(tga_header, 0, 18);
    tga_header[2] = 2;
    tga_header[12] = (width & 0x00FF);
    tga_header[13] = (width  & 0xFF00) / 256;
    tga_header[14] = (height  & 0x00FF) ;
    tga_header[15] =(height & 0xFF00) / 256;
    tga_header[16] = 32 ;
    
    d_write(file, &tga_header, 18);
    
    bufferPointer = buffer = Sys_Malloc(width*height*4);
    
    for (i = 0 ; i < width*height ; i++)
    {
        bufferPointer[0] = palettePointer[0] / 63.0 * 255;
        bufferPointer[1] = palettePointer[1] / 63.0 * 255;
        bufferPointer[2] = palettePointer[2] / 63.0 * 255;
        bufferPointer[3] = 255;
        
        bufferPointer += 4;
        palettePointer+= 3;
    }
    
    d_write(file, buffer, width*height * 4);
    d_close(file);
    
    Sys_Free(buffer);
}


void WriteLastPaletteToFile(){
    //WritePaletteToFile(lastPalette,"lastPalette.tga",16,16);
}

int VBE_setPalette(uint8_t  *palettebuffer)
/*
 * (From Ken's docs:)
 *   Set (num) palette palette entries starting at (start)
 *   palette entries are in a 4-byte format in this order:
 *       0: Blue (0-63)
 *       1: Green (0-63)
 *       2: Red (0-63)
 *       3: Reserved
 *
 * Naturally, the bytes are in the reverse order that SDL wants them...
 *  More importantly, SDL wants the color elements in a range from 0-255,
 *  so we do a conversion.
 */
{
    pal_t fmt_swap[256];
    pix_t r, g, b;
    pal_t *sdlp = fmt_swap;
    uint8_t  *p = palettebuffer;
    int i;
    //static updated=0;
    
    //if (updated >=1 )
    //    return ;
    
    //WritePaletteToFile(palettebuffer,"lastPalette.tga",16,16);
    //updated++;
    
   
    //CODE EXPLORATION
    //Used only to write the last palette to file.
    memcpy(lastPalette, palettebuffer, 768);
    
    for (i = 0; i < 256; i++){
        
        b = (Uint8) ((((float) *p++) / 63.0) * 255.0);
        g = (Uint8) ((((float) *p++) / 63.0) * 255.0);
        r = (Uint8) ((((float) *p++) / 63.0) * 255.0);
        *sdlp = GFX_RGB(r,g,b, 0xff);
        p++;
        sdlp++;
    }
    screen_set_clut(fmt_swap, 256);
    return 0;
}


int VBE_getPalette(int32_t start, int32_t num, uint8_t  *palettebuffer)
{
    SDL_Color *sdlp = surface->format->palette->colors + start;
    uint8_t  *p = palettebuffer + (start * 4);
    int i;

    for (i = 0; i < num; i++)
    {
        *p++ = (Uint8) ((((float) sdlp->b) / 255.0) * 63.0);
        *p++ = (Uint8) ((((float) sdlp->g) / 255.0) * 63.0);
        *p++ = (Uint8) ((((float) sdlp->r) / 255.0) * 63.0);
        *p++ = sdlp->unused;   /* This byte is unused in both SDL and BUILD. */
        sdlp++;
    } 

    return(1);
} 


void _uninitengine(void)
{
#ifdef ORIGCODE
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
} /* _uninitengine */




int setupmouse(void)
{
#ifdef ORIGCODE
	SDL_Event event;

    if (surface == NULL)
        return(0);

    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(0);

    mouse_relative_x = mouse_relative_y = 0;

        /*
         * this global usually gets set by BUILD, but it's a one-shot
         *  deal, and we may not have an SDL surface at that point. --ryan.
         */
    moustat = 1;

	// FIX_00063: Duke's angle changing or incorrect when using toggle fullscreen/window mode
	while(SDL_PollEvent(&event)); // Empying the various pending events (especially the mouse one)

	//SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
#endif
    return(1);
} /* setupmouse */


void readmousexy(short *x, short *y)
{
    if (x)
        *x = mouse_relative_x << 2;
    if (y)
        *y = mouse_relative_y << 2;

    mouse_relative_x = mouse_relative_y = 0;
} /* readmousexy */


void readmousebstatus(short *bstatus)
{
    if (bstatus)
        *bstatus = mouse_buttons;

    // special wheel treatment: make it like a button click
    if(mouse_buttons&8)
        mouse_buttons ^= 8;
    if(mouse_buttons&16)
        mouse_buttons ^= 16;

} /* readmousebstatus */


void _updateScreenRect(int32_t x, int32_t y, int32_t w, int32_t h)
{
    SDL_UpdateRect(surface, x, y, w, h);
}

//int counter= 0 ;
//char bmpName[256];
void _nextpage(void)

{
    Uint32 ticks;

    _handle_events();

    
    SDL_UpdateRect(surface, 0, 0, surface->w, surface->h);
    
    //sprintf(bmpName,"%d.bmp",counter++);
    //SDL_SaveBMP(surface,bmpName);
    
    
    //if (CLEAR_FRAMEBUFFER)
    //    SDL_FillRect(surface,NULL,0);

    ticks = getticks();
    total_render_time = (ticks - last_render_ticks);
    if (total_render_time > 1000){
        total_rendered_frames = 0;
        total_render_time = 1;
        last_render_ticks = ticks;
    } 
    total_rendered_frames++;
} 


uint8_t  readpixel(uint8_t  * offset)
{
    return *offset;
} 

void drawpixel(uint8_t  * location, uint8_t  pixel)
{
    *location = pixel;
}

/* Fix this up The Right Way (TM) - DDOI */
void setcolor16(uint8_t col)
{
	drawpixel_color = col;
}

void drawpixel16(int32_t offset)
{
    drawpixel((uint8_t*)surface->pixels + offset, drawpixel_color);
} /* drawpixel16 */


void fillscreen16(int32_t offset, int32_t color, int32_t blocksize)
{
    Uint8 *surface_end;
    Uint8 *wanted_end;
    Uint8 *pixels;

	//if (SDL_MUSTLOCK(surface))
        //SDL_LockSurface(surface);

    pixels = get_framebuffer();

    /* Make this function pageoffset aware - DDOI */
    if (!pageoffset) { 
	    offset = offset << 3;
	    offset += 640*336;
    }

    surface_end = (pixels + (surface->w * surface->h)) - 1;
    wanted_end = (pixels + offset) + blocksize;

    if (offset < 0)
        offset = 0;

    if (wanted_end > surface_end)
        blocksize = ((uint32_t) surface_end) - ((uint32_t) pixels + offset);

    memset(pixels + offset, (int) color, blocksize);

    //if (SDL_MUSTLOCK(surface))
        //SDL_UnlockSurface(surface);

    _nextpage();
} /* fillscreen16 */


/* Most of this line code is taken from Abrash's "Graphics Programming Blackbook".
Remember, sharing code is A Good Thing. AH */
static __inline void DrawHorizontalRun (uint8_t  **ScreenPtr, int XAdvance, int RunLength, uint8_t  Color)
{
    int i;
    uint8_t  *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
        *WorkingScreenPtr = Color;
        WorkingScreenPtr += XAdvance;
    }
    WorkingScreenPtr += surface->w;
    *ScreenPtr = WorkingScreenPtr;
}

static __inline void DrawVerticalRun (uint8_t  **ScreenPtr, int XAdvance, int RunLength, uint8_t  Color)
{
    int i;
    uint8_t  *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
      	*WorkingScreenPtr = Color;
    	WorkingScreenPtr += surface->w;
    }
    WorkingScreenPtr += XAdvance;
    *ScreenPtr = WorkingScreenPtr;
}

void drawline16(int32_t XStart, int32_t YStart, int32_t XEnd, int32_t YEnd, uint8_t  Color)
{
    int Temp, AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta;
    int WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;
    uint8_t  *ScreenPtr;
    int32_t dx, dy;

    //if (SDL_MUSTLOCK(surface))
        //SDL_LockSurface(surface);

	dx = XEnd-XStart;
    dy = YEnd-YStart;
    
    //Analyse the slope
	if (dx >= 0)
	{
		if ((XStart > 639) || (XEnd < 0)) return;
		if (XStart < 0) { if (dy) YStart += scale(0-XStart,dy,dx); XStart = 0; }
		if (XEnd > 639) { if (dy) YEnd += scale(639-XEnd,dy,dx); XEnd = 639; }
	}
	else
	{
		if ((XEnd > 639) || (XStart < 0)) return;
		if (XEnd < 0) { if (dy) YEnd += scale(0-XEnd,dy,dx); XEnd = 0; }
		if (XStart > 639) { if (dy) YStart += scale(639-XStart,dy,dx); XStart = 639; }
	}
	if (dy >= 0)
	{
		if ((YStart >= ydim16) || (YEnd < 0)) return;
		if (YStart < 0) { if (dx) XStart += scale(0-YStart,dx,dy); YStart = 0; }
		if (YEnd >= ydim16) { if (dx) XEnd += scale(ydim16-1-YEnd,dx,dy); YEnd = ydim16-1; }
	}
	else
	{
		if ((YEnd >= ydim16) || (YStart < 0)) return;
		if (YEnd < 0) { if (dx) XEnd += scale(0-YEnd,dx,dy); YEnd = 0; }
		if (YStart >= ydim16) { if (dx) XStart += scale(ydim16-1-YStart,dx,dy); YStart = ydim16-1; }
	}

	/* Make sure the status bar border draws correctly - DDOI */
	if (!pageoffset) { YStart += 336; YEnd += 336; }

    /* We'll always draw top to bottom */
    if (YStart > YEnd) {
        Temp = YStart;
        YStart = YEnd;
        YEnd = Temp;
        Temp = XStart;
        XStart = XEnd;
        XEnd = Temp;
    }

    /* Point to the bitmap address first pixel to draw */
    ScreenPtr = (uint8_t  *) (get_framebuffer()) + XStart + (surface->w * YStart);

    /* Figure out whether we're going left or right, and how far we're going horizontally */
    if ((XDelta = XEnd - XStart) < 0)
    {
        XAdvance = (-1);
        XDelta = -XDelta;
    } else {
        XAdvance = 1;
    }

    /* Figure out how far we're going vertically */
    YDelta = YEnd - YStart;

    /* Special cases: Horizontal, vertical, and diagonal lines */
    if (XDelta == 0)
    {
        for (i=0; i <= YDelta; i++)
        {
            *ScreenPtr = Color;
            ScreenPtr += surface->w;
        }

        UNLOCK_SURFACE_AND_RETURN;
    }
    if (YDelta == 0)
    {
    	for (i=0; i <= XDelta; i++)
    	{
      	    *ScreenPtr = Color;
    	    ScreenPtr += XAdvance;
    	}
        UNLOCK_SURFACE_AND_RETURN;
    }
    if (XDelta == YDelta)
    {
    	for (i=0; i <= XDelta; i++)
        {
            *ScreenPtr = Color;
            ScreenPtr += XAdvance + surface->w;
        }
        UNLOCK_SURFACE_AND_RETURN;
    }

    /* Determine whether the line is X or Y major, and handle accordingly */
    if (XDelta >= YDelta) /* X major line */
    {
        WholeStep = XDelta / YDelta;
        AdjUp = (XDelta % YDelta) * 2;
        AdjDown = YDelta * 2;
        ErrorTerm = (XDelta % YDelta) - (YDelta * 2);

        InitialPixelCount = (WholeStep / 2) + 1;
        FinalPixelCount = InitialPixelCount;

        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0)) InitialPixelCount--;
        if ((WholeStep & 0x01) != 0) ErrorTerm += YDelta;

        DrawHorizontalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);

        for (i=0; i<(YDelta-1); i++)
        {
            RunLength = WholeStep;
            if ((ErrorTerm += AdjUp) > 0)
            {
        	RunLength ++;
        	ErrorTerm -= AdjDown;
            }

            DrawHorizontalRun(&ScreenPtr, XAdvance, RunLength, Color);
         }

         DrawHorizontalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);

         UNLOCK_SURFACE_AND_RETURN;
    } else {	/* Y major line */
    	WholeStep = YDelta / XDelta;
    	AdjUp = (YDelta % XDelta) * 2;
    	AdjDown = XDelta * 2;
        ErrorTerm = (YDelta % XDelta) - (XDelta * 2);
        InitialPixelCount = (WholeStep / 2) + 1;
        FinalPixelCount = InitialPixelCount;

        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0)) InitialPixelCount --;
        if ((WholeStep & 0x01) != 0) ErrorTerm += XDelta;

        DrawVerticalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);

        for (i=0; i<(XDelta-1); i++)
        {
            RunLength = WholeStep;
            if ((ErrorTerm += AdjUp) > 0)
            {
            	RunLength ++;
            	ErrorTerm -= AdjDown;
            }

            DrawVerticalRun(&ScreenPtr, XAdvance, RunLength, Color);
        }

        DrawVerticalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
        UNLOCK_SURFACE_AND_RETURN;
     }
} /* drawline16 */


void clear2dscreen(void)
{
    SDL_Rect rect;

    rect.x = rect.y = 0;
    rect.w = surface->w;

	if (qsetmode == 350)
        rect.h = 350;
	else if (qsetmode == 480)
	{
		if (ydim16 <= 336)
            rect.h = 336;
        else
            rect.h = 480;
	} /* else if */

    //SDL_FillRect(surface, &rect, 0);
} /* clear2dscreen */


void _idle(void)
{
    if (surface != NULL)
        _handle_events();
    HAL_Delay(1);
} /* _idle */

void *_getVideoBase(void)
{
    return((void *) surface->pixels);
}


//-------------------------------------------------------------------------------------------------
//  TIMER
//=================================================================================================




// FIX_00007: game speed corrected. The game speed is now as the real
// DOS duke3d. Unloading a full 200 bullet pistol must take 45.1 sec.
// SDL timer was not fast/accurate enough and was slowing down the gameplay,
// so bad


static int64_t timerfreq=0;
static int32_t timerlastsample=0;
static int timerticspersec=0;
static void (*usertimercallback)(void) = NULL;

//  This timer stuff is all Ken's idea.

//
// installusertimercallback() -- set up a callback function to be called when the timer is fired
//
void (*installusertimercallback(void (*callback)(void)))(void)
{
	void (*oldtimercallback)(void);

	oldtimercallback = usertimercallback;
	usertimercallback = callback;

	return oldtimercallback;
}


/*
 inittimer() -- initialise timer
 FCS: The tickspersecond parameter is a ratio value that helps replicating
      oldschool DOS tick per seconds.
 
      The way the timer work is:
      float newSystemTickPerSecond = [0,1]
      tickPerSecond on a DOS system = tickspersecond * newSystemTickPerSecond ;
*/

int inittimer(int tickspersecond)
{
	int64_t t;
	
    
	if (timerfreq) return 0;	// already installed

	//printf("Initialising timer, with tickPerSecond=%d\n",tickspersecond);

	// OpenWatcom seems to want us to query the value into a local variable
	// instead of the global 'timerfreq' or else it gets pissed with an
	// access violation
	if (!TIMER_GetPlatformTicksInOneSecond(&t)) {
		dprintf("Failed fetching timer frequency\n");
		return -1;
	}
	timerfreq = t;
	timerticspersec = tickspersecond;
	TIMER_GetPlatformTicks(&t);
	timerlastsample = (int32_t)(t*timerticspersec / timerfreq);

	usertimercallback = NULL;
    
	return 0;
}

//
// uninittimer() -- shut down timer
//
void uninittimer(void)
{
	if (!timerfreq) return;

	timerfreq=0;
	timerticspersec = 0;
}

//
// sampletimer() -- update totalclock
//
void sampletimer(void)
{
	int64_t i;
	int32_t n;
	
	if (!timerfreq) return;

	TIMER_GetPlatformTicks(&i);
    
    
	n = (int32_t)(i*timerticspersec / timerfreq) - timerlastsample;
	if (n>0) {
		totalclock += n;
		timerlastsample += n;
	}

	if (usertimercallback) for (; n>0; n--) usertimercallback();
}


/*
   getticks() -- returns the windows ticks count
   FCS: This seeems to be only used in the multiplayer code
*/
uint32_t getticks(void)
{
	int64_t i;
	TIMER_GetPlatformTicks(&i);
	return (uint32_t)(i*(int32_t)(1000)/timerfreq);
}


//
// gettimerfreq() -- returns the number of ticks per second the timer is configured to generate
//
int gettimerfreq(void)
{
	return timerticspersec;
}



void initkeys(void)
{
    /* does nothing in SDL. Key input handling is set up elsewhere. */
    /* !!! why not here? */
}

void uninitkeys(void)
{
    /* does nothing in SDL. Key input handling is set up elsewhere. */
}


//unsigned int32_t getticks(void)
//{
//    return(SDL_GetTicks());
//} /* getticks */


//Timer on windows 98 used to be really poor but now it is very accurate
// We can just use what SDL uses, no need for QueryPerformanceFrequency or QueryPerformanceCounter
// (which I bet SDL is using anyway).
#if 0//PLATFORM_WIN32 
int TIMER_GetPlatformTicksInOneSecond(int64_t* t)
{
    QueryPerformanceFrequency((LARGE_INTEGER*)t);
    return 1;
}

void TIMER_GetPlatformTicks(int64_t* t)
{
    QueryPerformanceCounter((LARGE_INTEGER*)t);
}
#else
//FCS: Let's try to use SDL again: Maybe SDL library is accurate enough now.
int TIMER_GetPlatformTicksInOneSecond(int64_t* t)
{
    *t = 1000;
    return 1;
}
    
void TIMER_GetPlatformTicks(int64_t* t)
{
    *t = HAL_GetTick();
}
#endif
/* end of sdl_driver.c ... */

