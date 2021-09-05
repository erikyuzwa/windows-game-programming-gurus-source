// DEMO7_8.CPP 8-bit software bitmap clipper demo

// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  // just say no to MFC

#define INITGUID

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

#include <ddraw.h> // include directdraw

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// default screen size
#define SCREEN_WIDTH    320  // size of screen
#define SCREEN_HEIGHT   240
#define SCREEN_BPP      8    // bits per pixel

// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// the happy face structure
typedef struct HAPPY_FACE_TYP
       {
       int x,y;           // position of happy face
       int xv, yv;        // velocity of happy face
       } HAPPY_FACE, *HAPPY_FACE_PTR;

// MACROS /////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// initializes a direct draw struct
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// PROTOTYPES ////////////////////////////////////////////

void Blit_Clipped(int x, int y, 
                  int width, int height, 
                  UCHAR *bitmap, 
                  UCHAR *video_buffer,int mempitch);

// GLOBALS ////////////////////////////////////////////////

HWND      main_window_handle = NULL; // globally track main window
int       window_closed      = 0;    // tracks if window is closed
HINSTANCE hinstance_app      = NULL; // globally track hinstance

// directdraw stuff
LPDIRECTDRAW7         lpdd         = NULL;   // dd4 object
LPDIRECTDRAWSURFACE7  lpddsprimary = NULL;   // dd primary surface
LPDIRECTDRAWSURFACE7  lpddsback    = NULL;   // dd back surface
LPDIRECTDRAWPALETTE   lpddpal      = NULL;   // a pointer to the created dd palette
LPDIRECTDRAWCLIPPER   lpddclipper  = NULL;   // dd clipper
PALETTEENTRY          palette[256];          // color palette
PALETTEENTRY          save_palette[256];     // used to save palettes
DDSURFACEDESC2        ddsd;                  // a direct draw surface description struct
DDBLTFX               ddbltfx;               // used to fill
DDSCAPS2              ddscaps;               // a direct draw surface capabilities struct
HRESULT               ddrval;                // result back from dd calls
DWORD                 start_clock_count = 0; // used for timing

char buffer[80];                             // general printing buffer

// a low tech bitmap that uses palette entry 1 for the color :)
UCHAR happy_bitmap[64] = {0,0,0,0,0,0,0,0,
                          0,0,1,1,1,1,0,0,
                          0,1,0,1,1,0,1,0,
                          0,1,1,1,1,1,1,0,
                          0,1,0,1,1,0,1,0,
                          0,1,1,0,0,1,1,0,
                          0,0,1,1,1,1,0,0,
                          0,0,0,0,0,0,0,0};

UCHAR sad_bitmap[64] = {0,0,0,0,0,0,0,0,
                        0,0,1,1,1,1,0,0,
                        0,1,0,1,1,0,1,0,
                        0,1,1,1,1,1,1,0,
                        0,1,1,0,0,1,1,0,
                        0,1,0,1,1,0,1,0,
                        0,0,1,1,1,1,0,0,
                        0,0,0,0,0,0,0,0};

HAPPY_FACE happy_faces[100]; // this holds all the happy faces


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

///////////////////////////////////////////////////////////

int Game_Main(void *parms = NULL, int num_parms = 0)
{
// this is the main loop of the game, do all your processing
// here

DDBLTFX ddbltfx; // the blitter fx structure
static int feeling_counter = 0;   // tracks how we feel :)
static int happy = 1;             // let's start off being happy

// make sure this isn't executed again
if (window_closed)
   return(0);

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   {
   PostMessage(main_window_handle,WM_CLOSE,0,0);
   window_closed = 1;
   } // end if

// use the blitter to erase the back buffer
// first initialize the DDBLTFX structure
DDRAW_INIT_STRUCT(ddbltfx);

// now set the color word info to the color we desire
ddbltfx.dwFillColor = 0;

// make the blitter call
if (FAILED(lpddsback->Blt(NULL, // pointer to dest RECT, NULL for whole thing
                          NULL, // pointer to source surface
                          NULL, // pointer to source RECT
                          DDBLT_COLORFILL | DDBLT_WAIT, 
                          // do a color fill and wait if you have to
                          &ddbltfx))) // pointer to DDBLTFX holding info
return(0);

// initialize ddsd
DDRAW_INIT_STRUCT(ddsd);

// lock the back buffer surface
if (FAILED(lpddsback->Lock(NULL,&ddsd,
                              DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
                              NULL)))
    return(0);


// increment how we feel
if (++feeling_counter > 200)
   {
   feeling_counter = 0; 
   happy = -happy;
   } // end if

// draw all the happy faces
for (int face=0; face < 100; face++)
    {
    // are we happy or sad?
    if (happy==1) // we are happy :)
       Blit_Clipped(happy_faces[face].x, 
                    happy_faces[face].y,
                    8,8,    
                    happy_bitmap, 
                    (UCHAR *)ddsd.lpSurface,
                    ddsd.lPitch);         
     else // we must be sad :(
     Blit_Clipped(happy_faces[face].x, 
                  happy_faces[face].y,
                  8,8,    
                  sad_bitmap, 
                  (UCHAR *)ddsd.lpSurface,
                  ddsd.lPitch);     

    } // end face

// move all happy faces
for (face=0; face < 100; face++)
    {
    // move
    happy_faces[face].x+=happy_faces[face].xv;
    happy_faces[face].y+=happy_faces[face].yv;

    // check for off screen, if so wrap
    if (happy_faces[face].x > SCREEN_WIDTH)
         happy_faces[face].x = -8;
    else
    if (happy_faces[face].x < -8)
        happy_faces[face].x = SCREEN_WIDTH;

    if (happy_faces[face].y > SCREEN_HEIGHT)
         happy_faces[face].y = -8;
    else
    if (happy_faces[face].y < -8)
        happy_faces[face].y = SCREEN_HEIGHT;

    } // end face

// unlock surface
if (FAILED(lpddsback->Unlock(NULL)))
   return(0);

// flip the pages
while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));

// wait a sec
Sleep(30);

// return success or failure or your own return code here
return(1);

} // end Game_Main

////////////////////////////////////////////////////////////

int Game_Init(void *parms = NULL, int num_parms = 0)
{
// this is called once after the initial window is created and
// before the main event loop is entered, do all your initialization
// here

// create IDirectDraw interface 7.0 object and test for error
if (FAILED(DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL)))
   return(0);

// set cooperation to full screen
if (FAILED(lpdd->SetCooperativeLevel(main_window_handle, 
                                      DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | 
                                      DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT)))
   return(0);

// set display mode 
if (FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,0,0)))
   return(0);

// clear ddsd and set size
DDRAW_INIT_STRUCT(ddsd); 

// enable valid fields
ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;

// set the backbuffer count field to 1, use 2 for triple buffering
ddsd.dwBackBufferCount = 1;

// request a complex, flippable
ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;

// create the primary surface
if (FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL)))
   return(0);

// now query for attached surface from the primary surface

// this line is needed by the call
ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

// get the attached back buffer surface
if (FAILED(lpddsprimary->GetAttachedSurface(&ddsd.ddsCaps, &lpddsback)))
  return(0);

// build up the palette data array
for (int color=1; color < 255; color++)
    {
    // fill with random RGB values
    palette[color].peRed   = rand()%256;
    palette[color].peGreen = rand()%256;
    palette[color].peBlue  = rand()%256;

    // set flags field to PC_NOCOLLAPSE
    palette[color].peFlags = PC_NOCOLLAPSE;
    } // end for color

// now fill in entry 0 and 255 with black and white
palette[0].peRed     = 0;
palette[0].peGreen   = 0;
palette[0].peBlue    = 0;
palette[0].peFlags   = PC_NOCOLLAPSE;

palette[255].peRed   = 255;
palette[255].peGreen = 255;
palette[255].peBlue  = 255;
palette[255].peFlags = PC_NOCOLLAPSE;

// make color 1 yellow
palette[1].peRed     = 255;
palette[1].peGreen   = 255;
palette[1].peBlue    = 0;
palette[1].peFlags   = PC_NOCOLLAPSE;

// create the palette object
if (FAILED(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256 | 
                                DDPCAPS_INITIALIZE, 
                                palette,&lpddpal, NULL)))
return(0);

// finally attach the palette to the primary surface
if (FAILED(lpddsprimary->SetPalette(lpddpal)))
   return(0);

// initialize all the happy faces
for (int face = 0; face < 100; face++)
    {
    // set random position
    happy_faces[face].x = rand()%SCREEN_WIDTH;
    happy_faces[face].y = rand()%SCREEN_HEIGHT;    

    // set random velocity (-2,+2)
    happy_faces[face].xv = -2 + rand()%5;
    happy_faces[face].yv = -2 + rand()%5;

    } // end for face

// return success or failure or your own return code here
return(1);

} // end Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
// this is called after the game is exited and the main event
// loop while is exited, do all you cleanup and shutdown here


// now the back buffer surface
if (lpddpal)
   {
   lpddpal->Release();
   lpddpal = NULL;
   } // end if

// now the back buffer surface
if (lpddsback)
   {
   lpddsback->Release();
   lpddsback = NULL;
   } // end if

// now the primary surface
if (lpddsprimary)
   {
   lpddsprimary->Release();
   lpddsprimary = NULL;
   } // end if

// now blow away the IDirectDraw4 interface
if (lpdd)
   {
   lpdd->Release();
   lpdd = NULL;
   } // end if

// return success or failure or your own return code here
return(1);

} // end Game_Shutdown

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
hinstance_app = hinstance;

// register the window class
if (!RegisterClassEx(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindowEx(NULL,                  // extended style
                            WINDOW_CLASS_NAME,     // class
						    "DirectDraw Software Clipping Demo", // title
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

///////////////////////////////////////////////////////////

void Blit_Clipped(int x, int y,          // position to draw bitmap
                  int width, int height, // size of bitmap in pixels
                  UCHAR *bitmap,         // pointer to bitmap data
                  UCHAR *video_buffer,   // pointer to video buffer surface
                  int   mempitch)        // video pitch per line
{
// this function blits and clips the image sent in bitmap to the 
// destination surface pointed to by video_buffer
// the function assumes a 640x480x8 mode 

// first do trivial rejections of bitmap, is it totally invisible?
if ((x >= SCREEN_WIDTH) || (y>= SCREEN_HEIGHT) ||
    ((x + width) <= 0) || ((y + height) <= 0))
return;

// clip source rectangle
// pre-compute the bounding rect to make life easy
int x1 = x;
int y1 = y;
int x2 = x1 + width - 1;
int y2 = y1 + height -1;

// upper left hand corner first
if (x1 < 0)
   x1 = 0;

if (y1 < 0)
   y1 = 0;

// now lower left hand corner
if (x2 >= SCREEN_WIDTH)
    x2 = SCREEN_WIDTH-1;

if (y2 >= SCREEN_HEIGHT)
    y2 = SCREEN_HEIGHT-1;

// now we know to draw only the portions of the bitmap from (x1,y1) to (x2,y2)
// compute offsets into bitmap on x,y axes, we need this to compute starting point
// to rasterize from
int x_off = x1 - x;
int y_off = y1 - y;

// compute number of columns and rows to blit
int dx = x2 - x1 + 1;
int dy = y2 - y1 + 1;

// compute starting address in video_buffer 
video_buffer += (x1 + y1*mempitch);

// compute starting address in bitmap to scan data from
bitmap += (x_off + y_off*width);

// at this point bitmap is pointing to the first pixel in the bitmap that needs to
// be blitted, and video_buffer is pointing to the memory location on the destination
// buffer to put it, so now enter rasterizer loop

UCHAR pixel; // used to read/write pixels

for (int index_y = 0; index_y < dy; index_y++)
     {
     // inner loop, where the action takes place
     for (int index_x = 0; index_x < dx; index_x++)
          {
          // read pixel from source bitmap, test for transparency and plot
          if ((pixel = bitmap[index_x]))
              video_buffer[index_x] = pixel;

          } // end for index_x
     
          // advance pointers
          video_buffer+=mempitch;  // bytes per scanline
          bitmap      +=width;     // bytes per bitmap row

     } // end for index_y

} // end Blit_Clipped
