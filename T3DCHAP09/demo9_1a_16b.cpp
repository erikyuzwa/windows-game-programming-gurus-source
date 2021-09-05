// DEMO9_1a_16b.CPP - DirectInput Keyboard Demo Library version
// 16-bit version of keyboard demo

// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  

#define INITGUID // if not done elsewhere

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
#include <dinput.h>
#include "T3DLIB1.H"
#include "T3DLIB2.H"


// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// setup a 640x480 16-bit windowed mode example
#define WINDOW_TITLE      "16-Bit Keyboard Demo"
#define WINDOW_WIDTH      640   // size of window
#define WINDOW_HEIGHT     480

#define WINDOW_BPP        16    // bitdepth of window (8,16,24 etc.)
                                // note: if windowed and not
                                // fullscreen then bitdepth must
                                // be same as system bitdepth
                                // also if 8-bit the a pallete
                                // is created and attached

#define WINDOWED_APP      1     // 0 not windowed, 1 windowed

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256

// skelaton directions
#define SKELATON_EAST         0
#define SKELATON_NEAST        1  
#define SKELATON_NORTH        2
#define SKELATON_NWEST        3
#define SKELATON_WEST         4
#define SKELATON_SWEST        5
#define SKELATON_SOUTH        6
#define SKELATON_SEAST        7

// RGB color of the walls
#define WALL_COLOR_R 239
#define WALL_COLOR_G 8 
#define WALL_COLOR_B 0

// TYPES ///////////////////////////////////////////////////


// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL, int num_parms = 0);
int Game_Shutdown(void *parms=NULL, int num_parms = 0);
int Game_Main(void *parms=NULL,  int num_parms = 0);

// GLOBALS ////////////////////////////////////////////////

// windows vars
HWND      main_window_handle = NULL; // globally track main window
int       window_closed      = 0;    // tracks if window is closed
HINSTANCE main_instance      = NULL; // globally track hinstance

char buffer[80];                // used to print text

// demo globals
BOB          skelaton;     // the player skelaton

// animation sequences for bob
int skelaton_anims[8][4] = { {0,1,0,2},
                             {0+4,1+4,0+4,2+4},
                             {0+8,1+8,0+8,2+8},
                             {0+12,1+12,0+12,2+12},
                             {0+16,1+16,0+16,2+16},
                             {0+20,1+20,0+20,2+20},
                             {0+24,1+24,0+24,2+24},
                             {0+28,1+28,0+28,2+28}, };

BOB          plasma;       // players weapon

BITMAP_IMAGE reactor;      // the background   


// PROTOTYPES //////////////////////////////////////////////

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

// start up DirectDraw (replace the parms as you desire)
DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);

// start up DirectInput
DInput_Init();

// initialize the keyboard
DInput_Init_Keyboard();

// load the background
Load_Bitmap_File(&bitmap16bit, "REACTOR2_24.BMP");

// create and load the reactor bitmap image
Create_Bitmap(&reactor, 0,0, 640, 480,16);
Load_Image_Bitmap16(&reactor,&bitmap16bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Unload_Bitmap_File(&bitmap16bit);

// now let's load in all the frames for the skelaton!!!

// create skelaton bob
if (!Create_BOB(&skelaton,0,0,56,72,32,
           BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_ANIM,DDSCAPS_SYSTEMMEMORY,0,16))
   return(0);

// load the frames in 8 directions, 4 frames each
// each set of frames has a walk and a fire, frame sets
// are loaded in counter clockwise order looking down
// from a birds eys view or the x-z plane
for (int direction = 0; direction < 8; direction++)
    { 
    // build up file name
    sprintf(filename,"SKELSP%d_24.BMP",direction);

    // load in new bitmap file
    Load_Bitmap_File(&bitmap16bit,filename);
 
    Load_Frame_BOB16(&skelaton,&bitmap16bit,0+direction*4,0,0,BITMAP_EXTRACT_MODE_CELL);  
    Load_Frame_BOB16(&skelaton,&bitmap16bit,1+direction*4,1,0,BITMAP_EXTRACT_MODE_CELL);  
    Load_Frame_BOB16(&skelaton,&bitmap16bit,2+direction*4,2,0,BITMAP_EXTRACT_MODE_CELL);  
    Load_Frame_BOB16(&skelaton,&bitmap16bit,3+direction*4,0,1,BITMAP_EXTRACT_MODE_CELL);  

    // unload the bitmap file
    Unload_Bitmap_File(&bitmap16bit);

    // set the animation sequences for skelaton
    Load_Animation_BOB(&skelaton,direction,4,skelaton_anims[direction]);

    } // end for direction

// set up stating state of skelaton
Set_Animation_BOB(&skelaton, 0);
Set_Anim_Speed_BOB(&skelaton, 4);
Set_Vel_BOB(&skelaton, 0,0);
Set_Pos_BOB(&skelaton, 0, 128);

// hide the mouse
if (!WINDOWED_APP)
    ShowCursor(FALSE);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms,  int num_parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// kill the reactor
Destroy_Bitmap(&reactor);

// kill skelaton
Destroy_BOB(&skelaton);

// release keyboard
DInput_Release_Keyboard();

// shutdown DirectInput
DInput_Shutdown();

// shutdonw directdraw
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


int          index;             // looping var
int          dx,dy;             // general deltas used in collision detection
 
static int   player_moving = 0; // tracks player motion

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_SPACE))
    PostMessage(main_window_handle, WM_DESTROY,0,0);

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// lock the back buffer
DDraw_Lock_Back_Surface();

// draw the background reactor image
Draw_Bitmap16(&reactor, back_buffer, back_lpitch, 0);

// unlock the back buffer
DDraw_Unlock_Back_Surface();

// get player input

// get the keyboard data
lpdikey->GetDeviceState(256, (LPVOID)keyboard_state);

// reset motion flag
player_moving = 0;

// test direction of motion, this is a good example of testing the keyboard
// although the code could be optimized this is more educational

if (keyboard_state[DIK_RIGHT] && keyboard_state[DIK_UP]) 
   {
   // move skelaton
   skelaton.x+=2;
   skelaton.y-=2;
   dx=2; dy=-2;

   // set motion flag
   player_moving = 1;

   // check animation needs to change
   if (skelaton.curr_animation != SKELATON_NEAST)
      Set_Animation_BOB(&skelaton,SKELATON_NEAST);

   } // end if
else
if (keyboard_state[DIK_LEFT] && keyboard_state[DIK_UP]) 
   {
   // move skelaton
   skelaton.x-=2;
   skelaton.y-=2;
   dx=-2; dy=-2;

   // set motion flag
   player_moving = 1;

   // check animation needs to change
   if (skelaton.curr_animation != SKELATON_NWEST)
      Set_Animation_BOB(&skelaton,SKELATON_NWEST);

   } // end if
else
if (keyboard_state[DIK_LEFT] && keyboard_state[DIK_DOWN]) 
   {
   // move skelaton
   skelaton.x-=2;
   skelaton.y+=2;
   dx=-2; dy=2;

   // set motion flag
   player_moving = 1;

   // check animation needs to change
   if (skelaton.curr_animation != SKELATON_SWEST)
      Set_Animation_BOB(&skelaton,SKELATON_SWEST);

   } // end if
else
if (keyboard_state[DIK_RIGHT] && keyboard_state[DIK_DOWN]) 
   {
   // move skelaton
   skelaton.x+=2;
   skelaton.y+=2;
   dx=2; dy=2;

   // set motion flag
   player_moving = 1;

   // check animation needs to change
   if (skelaton.curr_animation != SKELATON_SEAST)
      Set_Animation_BOB(&skelaton,SKELATON_SEAST);

   } // end if
else
if (keyboard_state[DIK_RIGHT]) 
   {
   // move skelaton
   skelaton.x+=2;
   dx=2; dy=0;

   // set motion flag
   player_moving = 1;

   // check animation needs to change
   if (skelaton.curr_animation != SKELATON_EAST)
      Set_Animation_BOB(&skelaton,SKELATON_EAST);

   } // end if
else
if (keyboard_state[DIK_LEFT])  
   {
   // move skelaton
   skelaton.x-=2;
   dx=-2; dy=0; 
   
   // set motion flag
   player_moving = 1;

   // check animation needs to change
   if (skelaton.curr_animation != SKELATON_WEST)
      Set_Animation_BOB(&skelaton,SKELATON_WEST);

   } // end if
else
if (keyboard_state[DIK_UP])    
   {
   // move skelaton
   skelaton.y-=2;
   dx=0; dy=-2;
   
   // set motion flag
   player_moving = 1;

   // check animation needs to change
   if (skelaton.curr_animation != SKELATON_NORTH)
      Set_Animation_BOB(&skelaton,SKELATON_NORTH);

   } // end if
else
if (keyboard_state[DIK_DOWN])  
   {
   // move skelaton
   skelaton.y+=2;
   dx=0; dy=+2;

   // set motion flag
   player_moving = 1;

   // check animation needs to change
   if (skelaton.curr_animation != SKELATON_SOUTH)
      Set_Animation_BOB(&skelaton,SKELATON_SOUTH);

   } // end if

// only animate if player is moving
if (player_moving)
   {
   // animate skelaton
   Animate_BOB(&skelaton);


   // see if skelaton hit a wall
   
   // lock surface, so we can scan it
   DDraw_Lock_Back_Surface();
   
// call the color scanner with WALL_COLOR the color of the walls
// try to center the scan on the feet of the player
// note since we are uin 16-bit mode, we need to scan the 16 bit value then compare
// it against the 16-bit color code for the green pixel which has values RB(41,231,41)
// but depending if this is a 5.5.5 or 5.6.5 the 16-bit value will be different, however
// during ddraw_init RGB16Bit() was vectored (function pointer) to either 5.5.5 or 5.6.5 
// depending on the actual surface mode, so it should all work out :)
   if (Color_Scan16(skelaton.x+16, skelaton.y+16,
                    skelaton.x+skelaton.width-16, skelaton.y+skelaton.height-16,                                    
                    RGB16Bit(WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B),
                    RGB16Bit(WALL_COLOR_R, WALL_COLOR_G, WALL_COLOR_B), back_buffer,back_lpitch))
      {
      // back the skelaton up along its last trajectory
      skelaton.x-=dx;
      skelaton.y-=dy;
      } // end if
   
   // done, so unlock
   DDraw_Unlock_Back_Surface();

   // check if skelaton is off screen
   if (skelaton.x < 0 || skelaton.x > (screen_width - skelaton.width))
      skelaton.x-=dx;

   if (skelaton.y < 0 || skelaton.y > (screen_height - skelaton.height))
      skelaton.y-=dy;

   } // end if

// draw the skelaton
Draw_BOB16(&skelaton, lpddsback);

// draw some text
Draw_Text_GDI("I STILL HAVE A BONE TO PICK!",0,screen_height - 32,RGB(32,32,32),lpddsback);
Draw_Text_GDI("(16-Bit Version) USE ARROW KEYS TO MOVE, <ESC> TO EXIT.",0,0,RGB(32,32,32),lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30 fps
Wait_Clock(30);

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////