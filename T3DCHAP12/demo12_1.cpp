// DEMO12_1.CPP - fly brain demo
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

#define WINDOW_WIDTH    320   // size of window
#define WINDOW_HEIGHT   240

// defines for flys
#define MAX_FLYS        128

// bounding box for flys
#define MIN_X_FLY       190 
#define MAX_X_FLY       450
#define MIN_Y_FLY       350
#define MAX_Y_FLY       470

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

BOB flys[MAX_FLYS];            // the flys

int fly_sound_id = -1;         // sound id for fly sound

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

WNDCLASS winclass;	// this will hold the class we create
HWND	 hwnd;		// generic window handle
MSG		 msg;		// generic message
HDC      hdc;       // generic dc
PAINTSTRUCT ps;     // generic paintstruct

// first fill in the window class stucture
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

// register the window class
if (!RegisterClass(&winclass))
	return(0);

// create the window, note the use of WS_POPUP
if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, // class
						  "Fly Brain Demo",	 // title
						  WS_POPUP | WS_VISIBLE,
					 	  0,0,	   // x,y
						  WINDOW_WIDTH,  // width
                          WINDOW_HEIGHT, // height
						  NULL,	   // handle to parent 
						  NULL,	   // handle to menu
						  hinstance,// instance
						  NULL)))	// creation parms
return(0);

// save the window handle and instance in a global
main_window_handle = hwnd;
main_instance      = hinstance;

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

// start up DirectDraw (replace the parms as you desire)
DDraw_Init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

// load background image
Load_Bitmap_File(&bitmap8bit, "FLYBACK.BMP");
Create_Bitmap(&background_bmp,0,0,640,480);
Load_Image_Bitmap(&background_bmp, &bitmap8bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Set_Palette(bitmap8bit.palette);
Unload_Bitmap_File(&bitmap8bit);

// load the fly bitmaps
Load_Bitmap_File(&bitmap8bit, "FLYS8.BMP");

// create master fly bob
Create_BOB(&flys[0],320,200, 8,8, 4, BOB_ATTR_MULTI_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY);
Set_Anim_Speed_BOB(&flys[0], 1);

// load the fly in 
for (index=0; index<4; index++)
    Load_Frame_BOB(&flys[0], &bitmap8bit, index, index, 0, BITMAP_EXTRACT_MODE_CELL);

// unload flys
Unload_Bitmap_File(&bitmap8bit);


// now replicate flys
for (index = 1; index<MAX_FLYS; index++)
    Clone_BOB(&flys[0], &flys[index]);

// now set all of their values
for (index=0; index<MAX_FLYS; index++)
    {
    // set positions
    Set_Pos_BOB(&flys[index], 320-32+rand()%64, 450-rand()%32);

    // set start frame randomly
    flys[index].curr_frame = 0;

    } // end for index

// initilize DirectSound
DSound_Init();

// load fly sound
fly_sound_id = DSound_Load_WAV("FLYS.WAV");

// start the sound
DSound_Play(fly_sound_id, DSBPLAY_LOOPING);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// shut everything down

// kill all the flys
for (int index=0; index<MAX_FLYS; index++)
    Destroy_BOB(&flys[index]);

// shutdown directdraw last
DDraw_Shutdown();

// now directsound
DSound_Stop_All_Sounds();
DSound_Shutdown();

// return success
return(1);
} // end Game_Shutdown

///////////////////////////////////////////////////////////

void Flys_AI(void)
{
// this function performs the AI for the flys

for (int curr_fly=0; curr_fly<MAX_FLYS; curr_fly++)
    {   
    // test if fly directional counter <= 0
    if (--flys[curr_fly].varsI[0] <= 0)
        {
        // select a new random directional velocity
        flys[curr_fly].xv = -4 + rand()%9;
        flys[curr_fly].yv = -4 + rand()%9;

        // set time for motion to occur
        flys[curr_fly].varsI[0] = 2+rand()%8;

        } // end if

    // move the fly
    Move_BOB(&flys[curr_fly]);

    // animate the fly
    Animate_BOB(&flys[curr_fly]);

    // test if fly has left the fresh meat
    if (flys[curr_fly].x > MAX_X_FLY) 
       {
       flys[curr_fly].xv=-flys[curr_fly].xv;
       flys[curr_fly].x = MAX_X_FLY;
       } // end if

    if (flys[curr_fly].x < MIN_X_FLY) 
       {
       flys[curr_fly].xv=-flys[curr_fly].xv;
       flys[curr_fly].x = MIN_X_FLY;
       } // end if

    if (flys[curr_fly].y > MAX_Y_FLY) 
       {
       flys[curr_fly].yv=-flys[curr_fly].yv;
       flys[curr_fly].y = MAX_Y_FLY;
       } // end if

    if (flys[curr_fly].y < MIN_Y_FLY) 
       {
       flys[curr_fly].yv=-flys[curr_fly].yv;
       flys[curr_fly].y = MIN_Y_FLY;
       } // end if

    } // end for curr_fly

} // end Flys_AI

//////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE))
    {
    PostMessage(main_window_handle, WM_DESTROY,0,0);

    // stop all sounds
    DSound_Stop_All_Sounds();

    // do a screen transition
    Screen_Transitions(SCREEN_REDNESS,NULL,0);

    } // end if

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw background
Draw_Bitmap(&background_bmp, back_buffer, back_lpitch,0);

// unlock back surface
DDraw_Unlock_Back_Surface();

// process the fly ai, move them buzz around dead bodies
Flys_AI();

// draw the flys
for (index=0; index < MAX_FLYS; index++)
     Draw_BOB(&flys[index], lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30ish fps
Wait_Clock(30);

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////