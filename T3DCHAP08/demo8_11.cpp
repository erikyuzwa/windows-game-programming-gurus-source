// DEMO8_11.CPP - Smooth scrolling demo

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
#include "T3DLIB1.H"

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// default screen size
#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      8    // bits per pixel

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256

#define NUM_TEXTURES         10 // normally this would be a multiple of 16 or something!

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
BOB          textures;     // texture memory

int world_x = 0,   // current position of viewing window
    world_y = 0;

// use an array of string pointers, could have used an
// array of chars or int, but harder to initialize
// the characters '0' - '9' represent bitmaps 0-9 in some texture memory
char *world[21] = 
{
"111111111111111111111111111111",
"100000000000000000000000000001",
"100002222220000000000000077701",
"100002222223333333333000077701",
"100002222227777777773000070001",
"100002222227777777773000070001",
"100000000377777777773000070001",
"107777700377777777773000070001",
"177777770377777777773000770001",
"107777700377777777773007700001",
"100777770377777777773777000001",
"100000707377777777773000000001",
"100007777377777777773000000001",
"100000000302222777773000000001",
"100000000332222777773000000001",
"100000000002222333333000000001",
"100000666666666666666666600001",
"100000800000000000000000800001",
"100000800000000000000000800001",
"100000000000000000000000000001",
"111111111111111111111111111111",

};

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
						    "DirectDraw 8-Bit Smooth Scrolling Demo", // title
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

// initialize directdraw
DDraw_Init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

// load in texture maps
Load_Bitmap_File(&bitmap8bit, "SCROLLTEXTURES.BMP");

// set the palette to background image palette
Set_Palette(bitmap8bit.palette);

// create the texture bob
if (!Create_BOB(&textures,0,0,64,64,10, 
                BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_FRAME,DDSCAPS_SYSTEMMEMORY))
   return(0);

// load each texture bitmap into the texture BOB object
for (index = 0; index < NUM_TEXTURES; index++)
    Load_Frame_BOB(&textures,&bitmap8bit,index,index%4,index/4,BITMAP_EXTRACT_MODE_CELL); 

// unload the texture map bitmap
Unload_Bitmap_File(&bitmap8bit);

// set clipping rectangle to screen extents so mouse cursor
// doens't mess up at edges
RECT screen_rect = {0,0,screen_width,screen_height-32}; // 32 pixels at the bottom for controls

// notice at bottom of screen a blank rect, this would be for your
// control panel for example
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

// hide the mouse
ShowCursor(FALSE);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms,  int num_parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// kill texture memory
Destroy_BOB(&textures);

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

int index, index_x, index_y;  // looping vars
int start_map_x,start_map_y,  // map positions
    end_map_x,end_map_y; 

int offset_x, offset_y;       // pixel offsets within cell

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_SPACE))
    PostMessage(main_window_handle, WM_DESTROY,0,0);

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// check for movement (scrolling)
if (KEY_DOWN(VK_RIGHT))
    {
    if ((world_x+=4) >= 1280)
       world_x = 1279;

    } // end if
else
if (KEY_DOWN(VK_LEFT))
    {
    if ((world_x-=4) < 0)
       world_x = 0;

    } // end if

if (KEY_DOWN(VK_UP))
    {
    if ((world_y-=4) < 0)
       world_y = 0;

    } // end if
else
if (KEY_DOWN(VK_DOWN))
    {
    if ((world_y+=4) >= 896)
       world_y = 895;

    } // end if

// compute starting map indices by dividing position by size of cell
start_map_x = world_x/64; // use >> 6 for speed, but this is clearer
start_map_y = world_y/64; 

// compute end of map rectangle for best cast i.e. aligned on 64x64 boundary
end_map_x = start_map_x + 10 - 1;
end_map_y = start_map_y + 7 - 1;

// now compute number of pixels in x,y we are within the tile, i.e
// how much is scrolled off the edge?
offset_x = -(world_x % 64);
offset_y = -(world_y % 64);

// adjust end_map_x,y for offsets
if (offset_x)
   end_map_x++;

if (offset_y)
   end_map_y++;


// set starting position of first upper lh texture
int texture_x = offset_x;
int texture_y = offset_y;

// draw the current window
for (index_y = start_map_y; index_y <= end_map_y; index_y++)
    {
    for (index_x = start_map_x; index_x <= end_map_x; index_x++)
        {
        // set position to blit
        textures.x = texture_x;
        textures.y = texture_y;
        
        // set frame
        textures.curr_frame = world[index_y][index_x] - '0';

        // draw the texture
        Draw_BOB(&textures,lpddsback);

        // update texture position
        texture_x+=64;

        } // end for map_x

    // reset x postion, update y
    texture_x =  offset_x;
    texture_y += 64;

    } // end for map_y


// draw some info
Draw_Text_GDI("USE ARROW KEYS TO MOVE, <ESC> to Exit.",8,8,RGB(255,255,255),lpddsback);

sprintf(buffer,"World Position = [%d, %d]     ", world_x, world_y);
Draw_Text_GDI(buffer,8,screen_height - 32 - 24,RGB(0,255,0),lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30 fps
Wait_Clock(30);

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////