// DEMO12_2_16b.CPP - tracking demo
// 16-bit version
// to compile make sure to include DDRAW.LIB, DSOUND.LIB,
// DINPUT.LIB, WINMM.LIB, and of course 
// T3DLIB1.CPP,T3DLIB2.CPP,T3DLIB3.CPP,

// INCLUDES ///////////////////////////////////////////////

#define INITGUID

#define WIN32_LEAN_AND_MEAN  

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <iostream.h> // include important C/C++ stuff
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h> 
#include <math.h>
#include <io.h>
#include <fcntl.h>

#include <ddraw.h>  // directX includes
#include <dsound.h>
#include <dmksctrl.h>
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>
#include <dinput.h>
#include "T3DLIB1.h" // game library includes
#include "T3DLIB2.h"
#include "T3DLIB3.h"

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINXCLASS"  // class name

// setup a 640x480 16-bit windowed mode example
#define WINDOW_TITLE      "16-Bit Tracking Demo"
#define WINDOW_WIDTH      640   // size of window
#define WINDOW_HEIGHT     480

#define WINDOW_BPP        16    // bitdepth of window (8,16,24 etc.)
                                // note: if windowed and not
                                // fullscreen then bitdepth must
                                // be same as system bitdepth
                                // also if 8-bit the a pallete
                                // is created and attached

#define WINDOWED_APP      1     // 0 not windowed, 1 windowed

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle           = NULL; // save the window handle
HINSTANCE main_instance           = NULL; // save the instance
char buffer[80];                          // used to print text

BITMAP_IMAGE background_bmp;   // holds the background

BOB          bat,              // the ai bat bob
             ghost,            // the player ghost bob
             arch;             // to do the 3d arch effect

static int ghost_anim[] = {0,1,2,1}; // animation sequence for ghost

int bat_sound_id  = -1,        // sound of bat flapping wings
    wind_sound_id = -1;        // the ambient wind

// FUNCTIONS //////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
// this is the main message handler of the system
PAINTSTRUCT	ps;		   // used in WM_PAINT
HDC			hdc;	   // handle to a device context

// what is the message 
switch(msg)
	{	
	case WM_CREATE: 
        {
		// do initialization stuff here
		return(0);
		} break;

    case WM_PAINT:
         {
         // start painting
         hdc = BeginPaint(hwnd,&ps);

         // end painting
         EndPaint(hwnd,&ps);
         return(0);
        } break;

	case WM_DESTROY: 
		{
		// kill the application			
		PostQuitMessage(0);
		return(0);
		} break;

	default:break;

    } // end switch

// process any messages that we didn't take care of 
return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(	HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{
// this is the winmain function

WNDCLASSEX winclass; // this will hold the class we create
HWND	   hwnd;	 // generic window handle
MSG		   msg;		 // generic message
HDC        hdc;      // graphics device context

// first fill in the window class stucture
winclass.cbSize         = sizeof(WNDCLASSEX);
winclass.style			= CS_DBLCLKS | CS_OWNDC | 
                          CS_HREDRAW | CS_VREDRAW;
winclass.lpfnWndProc	= WindowProc;
winclass.cbClsExtra		= 0;
winclass.cbWndExtra		= 0;
winclass.hInstance		= hinstance;
winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
winclass.hCursor		= LoadCursor(NULL, IDC_ARROW); 
winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
winclass.lpszMenuName	= NULL;
winclass.lpszClassName	= WINDOW_CLASS_NAME;
winclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

// save hinstance in global
main_instance = hinstance;

// register the window class
if (!RegisterClassEx(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindowEx(NULL,                  // extended style
                            WINDOW_CLASS_NAME,     // class
						    WINDOW_TITLE, // title
						    (WINDOWED_APP ? (WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION) : (WS_POPUP | WS_VISIBLE)), 
					 	    0,0,	  // initial x,y
						    WINDOW_WIDTH,WINDOW_HEIGHT,  // initial width, height
						    NULL,	  // handle to parent 
						    NULL,	  // handle to menu
						    hinstance,// instance of this application
						    NULL)))	// extra creation parms
return(0);

// save main window handle
main_window_handle = hwnd;

if (WINDOWED_APP)
{
// now resize the window, so the client area is the actual size requested
// since there may be borders and controls if this is going to be a windowed app
// if the app is not windowed then it won't matter
RECT window_rect = {0,0,WINDOW_WIDTH-1,WINDOW_HEIGHT-1};

// make the call to adjust window_rect
AdjustWindowRectEx(&window_rect,
     GetWindowStyle(main_window_handle),
     GetMenu(main_window_handle) != NULL,
     GetWindowExStyle(main_window_handle));

// save the global client offsets, they are needed in DDraw_Flip()
window_client_x0 = -window_rect.left;
window_client_y0 = -window_rect.top;

// now resize the window with a call to MoveWindow()
MoveWindow(main_window_handle,
           0, // x position
           0, // y position
           window_rect.right - window_rect.left, // width
           window_rect.bottom - window_rect.top, // height
           TRUE);

// show the window, so there's no garbage on first render
ShowWindow(main_window_handle, SW_SHOW);
} // end if windowed

// perform all game console specific initialization
Game_Init();

// enter main event loop
while(1)
	{
	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{ 
		// test if this is a quit
        if (msg.message == WM_QUIT)
           break;
	
		// translate any accelerator keys
		TranslateMessage(&msg);

		// send the message to the window proc
		DispatchMessage(&msg);
		} // end if
    
    // main game processing goes here
    Game_Main();

	} // end while

// shutdown game and release all resources
Game_Shutdown();

// return to Windows like this
return(msg.wParam);

} // end WinMain

// T3D GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms)
{
// this function is where you do all the initialization 
// for your game

int index; // looping variable

// initialize directdraw, very important that in the call
// to setcooperativelevel that the flag DDSCL_MULTITHREADED is used
// which increases the response of directX graphics to
// take the global critical section more frequently
DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);

// load background image
Load_Bitmap_File(&bitmap16bit, "GHOSTBACK_24.BMP");
Create_Bitmap(&background_bmp,0,0,640,480,16);
Load_Image_Bitmap16(&background_bmp, &bitmap16bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Unload_Bitmap_File(&bitmap16bit);

// load the bat bitmaps
Load_Bitmap_File(&bitmap16bit, "BATS8_2_24.BMP");

// create bat bob
Create_BOB(&bat,320,200, 16,16, 5, BOB_ATTR_MULTI_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY,0,16);
Set_Anim_Speed_BOB(&bat, 2);

// load the bat in 
for (index=0; index < 5; index++)
    Load_Frame_BOB16(&bat, &bitmap16bit, index, index, 0, BITMAP_EXTRACT_MODE_CELL);

// create the arch bob
Create_BOB(&arch,320,200, 128,152, 1, BOB_ATTR_SINGLE_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY,0,16);

// load the arch
Load_Frame_BOB16(&arch, &bitmap16bit, 0,1,18, BITMAP_EXTRACT_MODE_ABS);

// unload arch and bat bitmap
Unload_Bitmap_File(&bitmap16bit);


// load the ghost bitmaps
Load_Bitmap_File(&bitmap16bit, "GHOSTS8_24.BMP");

// create ghost bob
Create_BOB(&ghost,100,200, 64,100, 3, BOB_ATTR_MULTI_ANIM | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY,0,16);
Set_Anim_Speed_BOB(&ghost, 10);

// load the ghost in 
for (index=0; index < 3; index++)
    Load_Frame_BOB16(&ghost, &bitmap16bit, index, index, 0, BITMAP_EXTRACT_MODE_CELL);

// unload ghost
Unload_Bitmap_File(&bitmap16bit);

// set animation for ghost
Load_Animation_BOB(&ghost, 0, 4, ghost_anim);

// set the animation
Set_Animation_BOB(&ghost,0);

// initialize directinput 
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// load background sounds
bat_sound_id = DSound_Load_WAV("BAT.WAV");
wind_sound_id = DSound_Load_WAV("WIND.WAV");

// start the sounds
DSound_Play(bat_sound_id, DSBPLAY_LOOPING);
DSound_Play(wind_sound_id, DSBPLAY_LOOPING);

// hide the mouse
if (!WINDOWED_APP)
    ShowCursor(FALSE);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// shut everything down

// kill all the bobs
Destroy_BOB(&bat);
Destroy_BOB(&ghost);

// shutdown directdraw last
DDraw_Shutdown();

// now directsound
DSound_Stop_All_Sounds();
DSound_Shutdown();

// shut down directinput
DInput_Shutdown();

// return success
return(1);
} // end Game_Shutdown


//////////////////////////////////////////////////////////

void Bat_AI(void)
{
// this function performs the bat ai

// do tracking algorithm

// first x-axis    
if (ghost.x > bat.x)
   bat.x+=2;
else
if (ghost.x < bat.x)
   bat.x-=2;

// now y-axis
if (ghost.y > bat.y)
   bat.y+=2;
else
if (ghost.y < bat.y)
   bat.y-=2;

// check boundaries
if (bat.x >= SCREEN_WIDTH)
   bat.x = -bat.width;
else
if (bat.x < -bat.width)
   bat.x = SCREEN_WIDTH;

if (bat.y >= SCREEN_HEIGHT)
   bat.y = -bat.height;
else
if (bat.y < -bat.height)
   bat.y = SCREEN_HEIGHT;

} // end Bat_AI

//////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw background
Draw_Bitmap16(&background_bmp, back_buffer, back_lpitch,0);

// unlock back surface
DDraw_Unlock_Back_Surface();

// read keyboard
DInput_Read_Keyboard();

// call the bat AI
Bat_AI();

// the animate the bat
Animate_BOB(&bat);

// draw the bat
Draw_BOB16(&bat, lpddsback);

// allow player to move
if (keyboard_state[DIK_RIGHT])
   ghost.x+=4;
else
if (keyboard_state[DIK_LEFT])
   ghost.x-=4;

if (keyboard_state[DIK_UP])
   ghost.y-=4;
else
if (keyboard_state[DIK_DOWN])
   ghost.y+=4;

// test if player is off screen, if so wrap around
if (ghost.x >= SCREEN_WIDTH)
   ghost.x = -ghost.width;
else
if (ghost.x < -ghost.width)
   ghost.x = SCREEN_WIDTH;

if (ghost.y >= SCREEN_HEIGHT)
   ghost.y = -ghost.height;
else
if (ghost.y < -ghost.height)
   ghost.y = SCREEN_HEIGHT;

// the animate the ghost
Animate_BOB(&ghost);

// draw the ghost
Draw_BOB16(&ghost, lpddsback);

// draw the arches to create a 3D effect
Set_Pos_BOB(&arch, 18, 270);
Draw_BOB16(&arch, lpddsback);

Set_Pos_BOB(&arch, 496, 270);
Draw_BOB16(&arch, lpddsback);

// draw title
Draw_Text_GDI("(16-Bit Version) GOING BATS! - Tracking Demo",10, 10,RGB(0,255,0), lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30ish fps
Wait_Clock(30);

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || keyboard_state[DIK_ESCAPE])
    {
    PostMessage(main_window_handle, WM_DESTROY,0,0);

    // stop all sounds
    DSound_Stop_All_Sounds();

    } // end if

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////