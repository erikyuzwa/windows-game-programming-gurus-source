// DEMO13_2_16b.CPP - acceleration demo
// 16-bit version, make sure desktop is in 16-bit mode!
// to compile make sure to include DDRAW.LIB, DSOUND.LIB,
// DINPUT.LIB, WINMM.LIB, and of course the T3DLIB files

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
#define WINDOW_TITLE      "16-Bit Acceleration Demo"
#define WINDOW_WIDTH      640   // size of window
#define WINDOW_HEIGHT     480

#define WINDOW_BPP        16    // bitdepth of window (8,16,24 etc.)
                                // note: if windowed and not
                                // fullscreen then bitdepth must
                                // be same as system bitdepth
                                // also if 8-bit the a pallete
                                // is created and attached

#define WINDOWED_APP      1     // 0 not windowed, 1 windowed

// defines for rocket
#define ROCKET_STATE_ON_PAD    0
#define ROCKET_STATE_IN_FLIGHT 1

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle   = NULL; // save the window handle
HINSTANCE main_instance   = NULL; // save the instance
char buffer[80];                          // used to print text

BITMAP_IMAGE background_bmp;   // holds the background
BOB          rocket;           // the rocket

int sound_id = -1;             // general sound

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

// WINX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms)
{
// this function is where you do all the initialization 
// for your game

int index; // looping varsIable

char filename[80]; // used to build up filenames

// seed random number generate
srand(Start_Clock());

// initialize directdraw, very important that in the call
// to setcooperativelevel that the flag DDSCL_MULTITHREADED is used
// which increases the response of directX graphics to
// take the global critical section more frequently
DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);

// load background image
Load_Bitmap_File(&bitmap16bit, "GANTRY24.BMP");
Create_Bitmap(&background_bmp,0,0,640,480,16);
Load_Image_Bitmap16(&background_bmp, &bitmap16bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Unload_Bitmap_File(&bitmap16bit);

// load the bitmaps
Load_Bitmap_File(&bitmap16bit, "ROCKET24.BMP");

// create bob
Create_BOB(&rocket,312,420,24,80,3,BOB_ATTR_MULTI_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY,0,16);

// set animation speed
Set_Anim_Speed_BOB(&rocket, 2);

// load the bob in 
for (index=0; index < 3; index++)
    Load_Frame_BOB16(&rocket, &bitmap16bit, index, index,0,BITMAP_EXTRACT_MODE_CELL);

// set animation state to on pad
rocket.anim_state = ROCKET_STATE_ON_PAD;

// use varsI[0] to hold acceleration mode, set to on
rocket.varsI[0] = 1; // -1 is off

// use varsF[0,1] to hold the velocity and acceleration
rocket.varsF[0] = 0; // initial velocity
rocket.varsF[1] = 0.2; // initial acceleration 0.2 pixel/frame^2

// unload bitmap image
Unload_Bitmap_File(&bitmap16bit);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// load background sounds
sound_id = DSound_Load_WAV("ROCKET.WAV");

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
Destroy_BOB(&rocket);

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

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

static int debounce_a = 0; // used to debounce acceleration toggle key

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


// test if user is toggling acceleration control
if (keyboard_state[DIK_A] && !debounce_a)
   {
   // toggle acceleration
   rocket.varsI[0] = -rocket.varsI[0];
   
   // debounce key, so it doesn't toggle 60 times!
   debounce_a = 1;

   } // end if

if (!keyboard_state[DIK_A])
   debounce_a = 0;

// test if user is changing acceleration
if (keyboard_state[DIK_DOWN])
   {
   // decrease acceleration factor   
   if ((rocket.varsF[1]-=0.1) < 0)
        rocket.varsF[1] = 0.1;
   } // end if
else
if (keyboard_state[DIK_UP])
   {
   // increase acceleration factor   
   rocket.varsF[1]+=0.1;
   } // end if

// test if player is firing rocket
if (keyboard_state[DIK_SPACE] && rocket.anim_state == ROCKET_STATE_ON_PAD)
    {
    // fire the rocket
    rocket.anim_state = ROCKET_STATE_IN_FLIGHT;

    // start sound
    DSound_Play(sound_id, 0);

    } // end if

// test state of rocket
if (rocket.anim_state == ROCKET_STATE_ON_PAD)
   {
   // make sure there is no animation and frame is 0
   rocket.curr_frame = 0;

   } // end if
else // rocket is in flight
    {
    // animate the rocket
    Animate_BOB(&rocket);

    // move the rocket
   
    // update the position with the current velocity
    rocket.y-=(int)(rocket.varsF[0]+0.5);

    // update velocity with acceleration (if acceleration is on)
    if (rocket.varsI[0]==1)
       rocket.varsF[0]+=rocket.varsF[1];

    // test if the rocket has moved off the screen
    if (rocket.y < -4*rocket.height)
       {
       // reset everything
       rocket.anim_state = ROCKET_STATE_ON_PAD;
       rocket.curr_frame = 0;
       rocket.varsF[0]   = 0;

       // reset position
       Set_Pos_BOB(&rocket, 312, 420);

       // turn sound off
       DSound_Stop_All_Sounds();

       } // end if

    } // end else

// draw the rocket
Draw_BOB16(&rocket, lpddsback);

// draw the title
Draw_Text_GDI("(16-Bit Version) ACCELERATION DEMO - Space to Launch, Arrows to Change Acceleration.",10, 10,RGB(0,255,255), lpddsback);

// draw information

// first acceleration mode
if (rocket.varsI[0]==1)
    sprintf(buffer, "Acceleration is ON");
else
    sprintf(buffer, "Acceleration is OFF");

Draw_Text_GDI(buffer,10,25,RGB(0,255,0), lpddsback);

// now current velocity
sprintf(buffer, "Velocity = %f pixels/frame",rocket.varsF[0]);
Draw_Text_GDI(buffer,10,40,RGB(0,255,0), lpddsback);

// and acceleration
sprintf(buffer, "Acceleration = %f pixels/frame^2",rocket.varsF[1]);
Draw_Text_GDI(buffer,10,55,RGB(0,255,0), lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30 fps = 1/30sec = 33 ms
Wait_Clock(33);

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