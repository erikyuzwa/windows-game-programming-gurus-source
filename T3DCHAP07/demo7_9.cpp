// DEMO7_9.CPP 8-bit clipping demo

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
#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      8    // bits per pixel

#define MAX_COLORS_PALETTE 256 // maximum colors in 256 color palette

// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// MACROS //////////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// this builds a 16 bit color value in 5.5.5 format (1-bit alpha mode)
#define _RGB16BIT555(r,g,b) ((b & 31) + ((g & 31) << 5) + ((r & 31) << 10))

// this builds a 16 bit color value in 5.6.5 format (green dominate mode)
#define _RGB16BIT565(r,g,b) ((b & 31) + ((g & 63) << 5) + ((r & 31) << 11))

// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

// initializes a direct draw struct
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// PROTOTYPES /////////////////////////////////////////////////

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list);

// GLOBALS ////////////////////////////////////////////////////

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

RECT source_rect, // used to hold the destination RECT
     dest_rect;  // used to hold the destination RECT

// make sure this isn't executed again
if (window_closed)
   return(0);

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   {
   PostMessage(main_window_handle,WM_CLOSE,0,0);
   window_closed = 1;
   } // end if

// get a random rectangle for source
int x1 = rand()%SCREEN_WIDTH;
int y1 = rand()%SCREEN_HEIGHT;
int x2 = rand()%SCREEN_WIDTH;
int y2 = rand()%SCREEN_HEIGHT;

// get a random rectangle for destination
int x3 = rand()%SCREEN_WIDTH;
int y3 = rand()%SCREEN_HEIGHT;
int x4 = rand()%SCREEN_WIDTH;
int y4 = rand()%SCREEN_HEIGHT;

// now set up the RECT structure to fill the region from
// (x1,y1) to (x2,y2) on the source surface
source_rect.left   = x1;
source_rect.top    = y1;
source_rect.right  = x2;
source_rect.bottom = y2;

// now set up the RECT structure to fill the region from
// (x3,y3) to (x4,y4) on the destination surface
dest_rect.left   = x3;
dest_rect.top    = y3;
dest_rect.right  = x4;
dest_rect.bottom = y4;

// make the blitter call
if (FAILED(lpddsprimary->Blt(&dest_rect,  // pointer to dest RECT
                             lpddsback,   // pointer to source surface
                             &source_rect,// pointer to source RECT
                             DDBLT_WAIT,  // control flags
                             NULL)))      // pointer to DDBLTFX holding info
   return(0);

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

// clear all entries defensive programming
memset(palette,0,MAX_COLORS_PALETTE*sizeof(PALETTEENTRY));

// create a R,G,B,GR gradient palette
for (int index=0; index < MAX_COLORS_PALETTE; index++)
    {
    // set each entry
    if (index < 64)  // shades of red
        palette[index].peRed = index*4; 
    else            // shades of green
    if (index >= 64 && index < 128) 
        palette[index].peGreen = (index-64)*4;
    else            // shades of blue
    if (index >= 128 && index < 192) 
       palette[index].peBlue = (index-128)*4;
    else            // shades of grey
    if (index >= 192 && index < 256) 
        palette[index].peRed = palette[index].peGreen = 
        palette[index].peBlue = (index-192)*4;
    
    // set flag to force directdraw to leave alone
    palette[index].peFlags = PC_NOCOLLAPSE;
    } // end for index


// draw a color gradient in back buffer
DDRAW_INIT_STRUCT(ddsd);

// lock the back buffer
if (FAILED(lpddsback->Lock(NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL)))
   return(0);

// get alias to start of surface memory for fast addressing
UCHAR *video_buffer = (UCHAR *)ddsd.lpSurface;

// draw the gradient
for (int index_y=0; index_y < SCREEN_HEIGHT; index_y++)
     {
     // fill next line with color
     memset((void *)video_buffer,index_y % 256,SCREEN_WIDTH);

     // advance pointer
     video_buffer+=ddsd.lPitch;
   
     } // end for index_y

// unlock the back buffer
if (FAILED(lpddsback->Unlock(NULL)))
   return(0);

// now create and attach clipper
RECT rect_list[3] = {{10,10,50,50},
                     {100,100,200,200},
                     {300,300, 500, 450}};

if (FAILED(lpddclipper = DDraw_Attach_Clipper(lpddsprimary,3,rect_list)))
   return(0);

// return success or failure or your own return code here
return(1);

} // end Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
// this is called after the game is exited and the main event
// loop while is exited, do all you cleanup and shutdown here

// first the palette
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

///////////////////////////////////////////////////////////

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list)

{
// this function creates a clipper from the sent clip list and attaches
// it to the sent surface

int index;                         // looping var
LPDIRECTDRAWCLIPPER lpddclipper;   // pointer to the newly created dd clipper
LPRGNDATA region_data;             // pointer to the region data that contains
                                   // the header and clip list

// first create the direct draw clipper
if (FAILED(lpdd->CreateClipper(0,&lpddclipper,NULL)))
   return(NULL);

// now create the clip list from the sent data

// first allocate memory for region data
region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER)+num_rects*sizeof(RECT));

// now copy the rects into region data
memcpy(region_data->Buffer, clip_list, sizeof(RECT)*num_rects);

// set up fields of header
region_data->rdh.dwSize          = sizeof(RGNDATAHEADER);
region_data->rdh.iType           = RDH_RECTANGLES;
region_data->rdh.nCount          = num_rects;
region_data->rdh.nRgnSize        = num_rects*sizeof(RECT);

region_data->rdh.rcBound.left    =  64000;
region_data->rdh.rcBound.top     =  64000;
region_data->rdh.rcBound.right   = -64000;
region_data->rdh.rcBound.bottom  = -64000;

// find bounds of all clipping regions
for (index=0; index<num_rects; index++)
    {
    // test if the next rectangle unioned with the current bound is larger
    if (clip_list[index].left < region_data->rdh.rcBound.left)
       region_data->rdh.rcBound.left = clip_list[index].left;

    if (clip_list[index].right > region_data->rdh.rcBound.right)
       region_data->rdh.rcBound.right = clip_list[index].right;

    if (clip_list[index].top < region_data->rdh.rcBound.top)
       region_data->rdh.rcBound.top = clip_list[index].top;

    if (clip_list[index].bottom > region_data->rdh.rcBound.bottom)
       region_data->rdh.rcBound.bottom = clip_list[index].bottom;

    } // end for index

// now we have computed the bounding rectangle region and set up the data
// now let's set the clipping list

if (FAILED(lpddclipper->SetClipList(region_data, 0)))
   {
   // release memory and return error
   free(region_data);
   return(NULL);
   } // end if

// now attach the clipper to the surface
if (FAILED(lpdds->SetClipper(lpddclipper)))
   {
   // release memory and return error
   free(region_data);
   return(NULL);
   } // end if

// all is well, so release memory and send back the pointer to the new clipper
free(region_data);
return(lpddclipper);

} // end DDraw_Attach_Clipper


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
						    "DirectDraw Clipping Demo", // title
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

void Blit_clipped(int x, int y,          // position to draw bitmap
                  int width, int height, // size of bitmap in pixels
                  UCHAR *bitmap,         // pointer to bitmap data
                  UCHAR *video_buffer)   // pointer to video buffer surface
                
{
// this function blits and clips the image sent in bitmap to the 
// destination surface pointed to by video_buffer
// the function assumes a 640x480x8 mode with linear pitch

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
video_buffer += (x1 + y1*640);

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
          video_buffer+=640;   // bytes per scanline
          bitmap      +=width; // bytes per bitmap row

     } // end for index_y

} // end Blit_Clipped
