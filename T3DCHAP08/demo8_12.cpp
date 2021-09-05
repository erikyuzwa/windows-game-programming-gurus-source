// DEMO8_12.CPP - Sparse universe scrolling demo

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

// size of universe, could be anything
#define MAX_UNIVERSE_X 6400
#define MAX_UNIVERSE_Y 4800

#define NUM_OBJECTS_UNIVERSE 256 // number of objects in the universe

// used to flag type of object
#define GENERATOR_OBJ   0
#define BEACON_OBJ      1  
#define ALIEN_OBJ       2  

// TYPES ///////////////////////////////////////////////////

// need something to hold each object
typedef struct SPARSE_OBJ_TYP
        {
        int type;  // type of object: beacon, generator, or alien
                   // could have just used a bob_ptr here, but I like
                   // doing this the hard way so you can exactly what's happening
                   
        int x,y;   // position of object 

        } SPARSE_OBJ, *SPARSE_OBJ_PTR;

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
BOB generator,  // the generator BOB
    alien,      // a little alien ship BOB
    beacon;     // a beacon BOB

int world_x = 0,   // current position of viewing window
    world_y = 0;

SPARSE_OBJ world[NUM_OBJECTS_UNIVERSE]; // our world consists of 256 objects randomly placed in the universe
                       // that is 10x10

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
						    "DirectDraw 8-Bit Sparse crolling Demo", // title
						    WS_POPUP | WS_VISIBLE,
					 	    0,0,	  // initial x,y
						    SCREEN_WIDTH,SCREEN_HEIGHT,  // initial width, height
						    NULL,	  // handle to parent 
						    NULL,	  // handle to menu
						    hinstance,// instance of this application
						    NULL)))	  // extra creation parms
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

// load in the generator bob image
Load_Bitmap_File(&bitmap8bit, "GENS4.BMP");

// set the palette to the palette of the generators
Set_Palette(bitmap8bit.palette);


// create the generator bob
if (!Create_BOB(&generator,0,0,72,84,4, 
                BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_FRAME,DDSCAPS_SYSTEMMEMORY))
   return(0);

// load each bitmap into the generator BOB object
for (index = 0; index < 4; index++)
    Load_Frame_BOB(&generator,&bitmap8bit,index,index%4,index/4,BITMAP_EXTRACT_MODE_CELL); 

// set up stating state of bob
Set_Animation_BOB(&generator, 0);
Set_Anim_Speed_BOB(&generator, 4);

// unload the map bitmap
Unload_Bitmap_File(&bitmap8bit);


// load in the alien bob image
Load_Bitmap_File(&bitmap8bit, "ALIENS0.BMP");

// create the alien bob
if (!Create_BOB(&alien,0,0,56,30,8, 
                BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_FRAME,DDSCAPS_SYSTEMMEMORY))
   return(0);

// load each bitmap into the alien BOB object
for (index = 0; index < 8; index++)
    Load_Frame_BOB(&alien,&bitmap8bit,index,index%5,index/5,BITMAP_EXTRACT_MODE_CELL); 

// set up stating state of bob
Set_Animation_BOB(&alien, 0);
Set_Anim_Speed_BOB(&alien, 4);

// unload the bitmap
Unload_Bitmap_File(&bitmap8bit);



// load in the beacon bob image
Load_Bitmap_File(&bitmap8bit, "PIPE0.BMP");

// create the alien bob
if (!Create_BOB(&beacon,0,0,42,36,8, 
                BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_FRAME,DDSCAPS_SYSTEMMEMORY))
   return(0);

// load each bitmap into the beacon BOB object
for (index = 0; index < 8; index++)
    Load_Frame_BOB(&beacon,&bitmap8bit,index,index%6,index/6,BITMAP_EXTRACT_MODE_CELL); 

// set up stating state of bob
Set_Animation_BOB(&beacon, 0);
Set_Anim_Speed_BOB(&beacon, 4);

// unload the bitmap
Unload_Bitmap_File(&bitmap8bit);


// set clipping rectangle to screen extents so mouse cursor
// doens't mess up at edges
RECT screen_rect = {0,0,screen_width,screen_height}; 
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

// hide the mouse
ShowCursor(FALSE);

// now generate the sparse universe
for (index = 0; index < NUM_OBJECTS_UNIVERSE; index++)
    {
    // set random position
    world[index].x = rand()%MAX_UNIVERSE_X;
    world[index].y = rand()%MAX_UNIVERSE_Y;

    // set type
    world[index].type = rand()%3; // 0, 1, 2 generator, alien, beacon

    } // end for index
 
// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms,  int num_parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// kill objects
Destroy_BOB(&alien);
Destroy_BOB(&generator);
Destroy_BOB(&beacon);

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
int screen_x, screen_y;       // screen coords
int width, height, type;      // used to extract bob info
int x0,y0,x1,y1; // used to compute the upper left and lower right corners of each object
int visible = 0; // number of visible objects

BOB_PTR object_ptr; // the object that is going to be rendered

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
    if ((world_x+=8) >= 5760)
       world_x = 5760-1;

    } // end if
else
if (KEY_DOWN(VK_LEFT))
    {
    if ((world_x-=8) < 0)
       world_x = 0;

    } // end if

if (KEY_DOWN(VK_UP))
    {
    if ((world_y-=8) < 0)
       world_y = 0;

    } // end if
else
if (KEY_DOWN(VK_DOWN))
    {
    if ((world_y+=8) >= 4320)
       world_y = 4320-1;

    } // end if

// based on current world position which defines the viewport, i.e,
// world_x, world_y to world_x+640-1, world_y+480-1, scan the object list
// determine any object in that region translate it to screen coords and render it


for (index = 0, visible = 0; index < NUM_OBJECTS_UNIVERSE; index++)
    {
    // get the object position
    x0 = world[index].x;
    y0 = world[index].y;

    // now translate to screen coords, could have done above, but clearer
    x0 = x0 - world_x;
    y0 = y0 - world_y;

    // get type, so we can compute size
    type = world[index].type;
    
    // compute size
    switch(type)
          {
          case GENERATOR_OBJ:
          {
          // get size of this object
          width = generator.width;
          height = generator.height;

          // point to it to render it easier without another switch
          object_ptr = &generator;
          } break;

          case BEACON_OBJ:        
          {
          // get size of this object
          width = beacon.width;
          height = beacon.height;
          
          // point to it to render it easier without another switch
          object_ptr = &beacon;
          } break;

          case ALIEN_OBJ:       
          {
          // get size of this object
          width = alien.width;
          height = alien.height;
          
          // point to it to render it easier without another switch
          object_ptr = &alien;

          } break;        
          
          default: break;

          } // end switch

    // compute extents based on width,height
    x1 = x0 + width - 1;
    y1 = y0 + height - 1;

    // do clip trivial clip
    if ( (x0 >= SCREEN_WIDTH) || (x1 < 0) || (y0 >= SCREEN_HEIGHT) || (y1 < 0))
       continue; // don't need to render this one
    else
       {
       // render this object
       object_ptr->x = x0;
       object_ptr->y = y0;

       Draw_BOB(object_ptr, lpddsback);       

       visible++;

       } // end else

    } // end for index

// animate all the bobs
Animate_BOB(&generator);
Animate_BOB(&beacon);
Animate_BOB(&alien);

// draw some info
Draw_Text_GDI("USE ARROW KEYS TO MOVE, <ESC> to Exit.",8,8,RGB(0,255,0),lpddsback);

sprintf(buffer,"World Position = [%d, %d]  Objects Visible = %d, Objects Clipped = %d   ", world_x, world_y, visible, 256 - visible);
Draw_Text_GDI(buffer,8,screen_height - 32,RGB(0,255,0),lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30 fps
Wait_Clock(30);

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////