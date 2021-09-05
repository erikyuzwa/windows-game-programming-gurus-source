// DEMO11_10.CPP - Multithreaded DirectX demo
// make sure to compile with multithreaded libraries
// and if the program doesn't work try turning optimizations
// completely off

// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  

// you must #define INITGUID if not done elsewhere
#define INITGUID

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <objbase.h>
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

#include <ddraw.h>     // directX includes
#include <dsound.h>
#include <dmksctrl.h>
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>
#include <dinput.h>
#include "T3DLIB1.H"
#include "T3DLIB2.H"
#include "T3DLIB3.H"

// DEFINES ////////////////////////////////////////////////

// comment this out if you want to see the normal single 
// threaded version
#define USE_MULTITHREADING

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// default screen size
#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      8    // bits per pixel

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256

#define NUM_ALIENS 16    // number of aliens in the sim

// TYPES ///////////////////////////////////////////////////

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL, int num_parms = 0);
int Game_Shutdown(void *parms=NULL, int num_parms = 0);
int Game_Main(void *parms=NULL,  int num_parms = 0);

DWORD WINAPI Alien_Color_Thread(LPVOID data);

// GLOBALS ////////////////////////////////////////////////

// windows vars
HWND      main_window_handle = NULL; // globally track main window
int       window_closed      = 0;    // tracks if window is closed
HINSTANCE main_instance       = NULL; // globally track hinstance

char buffer[80];             // used to print text

// demo globals
BOB aliens[NUM_ALIENS];      // the aliens, each will be a thread

HANDLE thread_handle;  // this holds the handle to the thread
DWORD  thread_id;      // this holds the id of the thread

int terminate_threads = 0;  // global message flag to terminate
int active_threads    = 0;  // number of active threads

// PROTOTYPES //////////////////////////////////////////////

// FUNCTIONS //////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
// this is the main message handler of the system
PAINTSTRUCT		ps;		// used in WM_PAINT
HDC				hdc;	// handle to a device context
char buffer[80];        // used to print strings

// what is the message 
switch(msg)
	{	
	case WM_CREATE: 
        {
		// do initialization stuff here
        // return success
		return(0);
		} break;
   
	case WM_PAINT: 
		{
		// simply validate the window 
   	    hdc = BeginPaint(hwnd,&ps);	 
        
        // end painting
        EndPaint(hwnd,&ps);

        // return success
		return(0);
   		} break;

	case WM_DESTROY: 
		{

		// kill the application, this sends a WM_QUIT message 
		PostQuitMessage(0);

        // return success
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
						    "Multithreaded DirectDraw 8-Bit Demo", // title
						    WS_POPUP | WS_VISIBLE,
					 	    0,0,	  // initial x,y
						    SCREEN_WIDTH,SCREEN_HEIGHT,  // initial width, height
						    NULL,	  // handle to parent 
						    NULL,	  // handle to menu
						    hinstance,// instance of this application
						    NULL)))	// extra creation parms
return(0);

// save main window handle
main_window_handle = hwnd;

// initialize game here
Game_Init();

// enter main event loop
while(TRUE)
	{
    // test if there is a message in queue, if so get it
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

// closedown game here
Game_Shutdown();

// return to Windows like this
return(msg.wParam);

} // end WinMain

// GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms,  int num_parms)
{
// this function is where you do all the initialization 
// for your game

int index;         // looping var
char filename[80]; // used to build up files names

// seed random number generator
srand(Get_Clock());

// initialize directdraw, very important that in the call
// to setcooperativelevel that the flag DDSCL_MULTITHREADED is used
// which increases the response of directX graphics to
// take the global critical section more frequently
DDraw_Init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

// load in the alien bob image
Load_Bitmap_File(&bitmap8bit, "ALIENSGLOW.BMP");

// set the palette to the palette of the aliens
Set_Palette(bitmap8bit.palette);

// create the master alien bob
if (!Create_BOB(&aliens[0],0,0,48,47,1, 
                BOB_ATTR_VISIBLE | BOB_ATTR_SINGLE_FRAME | BOB_ATTR_BOUNCE,DDSCAPS_SYSTEMMEMORY ))
   return(0);

// load the bitmap for alien -- only 1
Load_Frame_BOB(&aliens[0],&bitmap8bit,0,0,0,BITMAP_EXTRACT_MODE_CELL); 

// unload the map bitmap
Unload_Bitmap_File(&bitmap8bit);

// now create all the alien clones :)
for (index = 1; index < NUM_ALIENS; index++)
    Clone_BOB(&aliens[0],&aliens[index]);

// at this point everything has been cloned, now set up aliens at
// random positions
for (index = 0; index < NUM_ALIENS; index++)
    {
    // set position
    Set_Pos_BOB(&aliens[index], rand()%screen_width, rand()%screen_height);

    // set motion velocities
    Set_Vel_BOB(&aliens[index],-4+rand()%8, -4+rand()%8);

    } // end for index

// set clipping rectangle to screen extents so mouse cursor
// doens't mess up at edges
RECT screen_rect = {0,0,screen_width,screen_height}; 
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

// hide the mouse
ShowCursor(FALSE);

#ifdef USE_MULTITHREADING
// create the animation thread for color rotation
thread_handle = CreateThread(NULL,               // default security
			                    0,				    // default stack 
								Alien_Color_Thread,// use this thread function
								(LPVOID)index,      // user data sent to thread
								0,				    // creation flags, 0=start now.
								&thread_id);	// send id back in this var

    // increment number of active threads
    active_threads++;

#endif
 
// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms,  int num_parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

#ifdef USE_MULTITHREADING
// kill all threads first

// set global termination flag
terminate_threads = 1;

// wait for all threads to terminate, when all are terminated active_threads==0
while(active_threads); 

// at this point the threads should all be dead, so close handles
CloseHandle(thread_handle);
#endif

// kill objects

// only need to kill master BOB
Destroy_BOB(&aliens[0]);

// shutdown directdraw
DDraw_Shutdown();

// return success
return(1);
} // end Game_Shutdown

///////////////////////////////////////////////////////////

int Game_Main(void *parms, int num_parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index, index_x, index_y;  // looping vars

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_SPACE))
    PostMessage(main_window_handle, WM_DESTROY,0,0);

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// move the aliens
for (index = 0; index<NUM_ALIENS; index++)
    Move_BOB(&aliens[index]);

// draw the aliens
for (index = 0; index<NUM_ALIENS; index++)
    Draw_BOB(&aliens[index], lpddsback);       

#ifndef USE_MULTITHREADING
// animate the aliens -- color animation
Rotate_Colors(249, 253);  
#endif

// draw some info
Draw_Text_GDI("<ESC> to Exit.",8,8,RGB(0,255,0),lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30 fps
Wait_Clock(30);

// return success
return(1);

} // end Game_Main

// THREADING FUNCTION////////////////////////////////////

DWORD WINAPI Alien_Color_Thread(LPVOID data)
{
// this thread animates the colors of the aliens
// note there are a number of things to consider, such as 
// are the calls to the sub-functions re-entrant, are there
// any deadlocks? is the a potential of confusing DirectX
// etc. also, this function is going to free run, so
// it needs it's own internal timing

for(;;)
	{
	// test for temination message
	if (terminate_threads)
		break;

    // test for next frame message from main event loop
    DWORD start_time = Get_Clock();

    // animate the aliens -- color animation
    // this function happens to be re-entrant, however only one thread is ever
    // calling it, so it doesnt matter, but if multiple threads were
    // then it would HAVE to be re-entrant
    Rotate_Colors(249, 253);  

    // wait for 30 fps
    while((Get_Clock() - start_time) < 30);
	
    } // end for ;;

// decrement number of active threads
if (active_threads > 0) 
   active_threads--;

// just return the data sent to the thread function
return((DWORD)data);

} // end Alien_Color_Thread
