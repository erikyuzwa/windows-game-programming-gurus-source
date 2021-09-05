// DEMO7_13.CPP 8-bit blitting demo

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

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256

// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// container structure for bitmaps .BMP file
typedef struct BITMAP_FILE_TAG
        {
        BITMAPFILEHEADER bitmapfileheader;  // this contains the bitmapfile header
        BITMAPINFOHEADER bitmapinfoheader;  // this is all the info including the palette
        PALETTEENTRY     palette[256];      // we will store the palette here
        UCHAR            *buffer;           // this is a pointer to the data

        } BITMAP_FILE, *BITMAP_FILE_PTR;

// this will hold our little alien
typedef struct ALIEN_OBJ_TYP
        {
        LPDIRECTDRAWSURFACE7 frames[3]; // 3 frames of animation for complete walk cycle
        int x,y;                        // position of alien
        int velocity;                   // x-velocity
        int current_frame;              // current frame of animation
        int counter;                    // used to time animation
 
        } ALIEN_OBJ, *ALIEN_OBJ_PTR;

// PROTOTYPES  //////////////////////////////////////////////

int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height);

int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename);

int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap);

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,int color);

int Scan_Image_Bitmap(BITMAP_FILE_PTR bitmap, LPDIRECTDRAWSURFACE7 lpdds, int cx,int cy);

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, int mem_flags, int color_key);

int DDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source, int x, int y, 
                      int width, int height, LPDIRECTDRAWSURFACE7 dest, 
                      int transparent);    

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list);

int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds);

// MACROS /////////////////////////////////////////////////

// tests if a key is up or down
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// initializes a direct draw struct
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

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

BITMAP_FILE           bitmap;                // holds the bitmap

ALIEN_OBJ             aliens[3];             // 3 aliens, one on each level

LPDIRECTDRAWSURFACE7  lpddsbackground = NULL;// this will hold the background image

char buffer[80];                             // general printing buffer

int gwidth  = -1;
int gheight = -1;

// FUNCTIONS ////////////////////////////////////////////////

int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename)
{
// this function opens a bitmap file and loads the data into bitmap

int file_handle,  // the file handle
    index;        // looping index

UCHAR   *temp_buffer = NULL; // used to convert 24 bit images to 16 bit
OFSTRUCT file_data;          // the file data information

// open the file if it exists
if ((file_handle = OpenFile(filename,&file_data,OF_READ))==-1)
   return(0);

// now load the bitmap file header
_lread(file_handle, &bitmap->bitmapfileheader,sizeof(BITMAPFILEHEADER));

// test if this is a bitmap file
if (bitmap->bitmapfileheader.bfType!=BITMAP_ID)
   {
   // close the file
   _lclose(file_handle);

   // return error
   return(0);
   } // end if

// now we know this is a bitmap, so read in all the sections

// first the bitmap infoheader

// now load the bitmap file header
_lread(file_handle, &bitmap->bitmapinfoheader,sizeof(BITMAPINFOHEADER));

// now load the color palette if there is one
if (bitmap->bitmapinfoheader.biBitCount == 8)
   {
   _lread(file_handle, &bitmap->palette,MAX_COLORS_PALETTE*sizeof(PALETTEENTRY));

   // now set all the flags in the palette correctly and fix the reversed 
   // BGR RGBQUAD data format
   for (index=0; index < MAX_COLORS_PALETTE; index++)
       {
       // reverse the red and green fields
       int temp_color                = bitmap->palette[index].peRed;
       bitmap->palette[index].peRed  = bitmap->palette[index].peBlue;
       bitmap->palette[index].peBlue = temp_color;
       
       // always set the flags word to this
       bitmap->palette[index].peFlags = PC_NOCOLLAPSE;
       } // end for index

    } // end if

// finally the image data itself
_lseek(file_handle,-(int)(bitmap->bitmapinfoheader.biSizeImage),SEEK_END);

// now read in the image, if the image is 8 or 16 bit then simply read it
// but if its 24 bit then read it into a temporary area and then convert
// it to a 16 bit image

if (bitmap->bitmapinfoheader.biBitCount==8 || bitmap->bitmapinfoheader.biBitCount==16 || 
    bitmap->bitmapinfoheader.biBitCount==24)
   {
   // delete the last image if there was one
   if (bitmap->buffer)
       free(bitmap->buffer);

   // allocate the memory for the image
   if (!(bitmap->buffer = (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage)))
      {
      // close the file
      _lclose(file_handle);

      // return error
      return(0);
      } // end if

   // now read it in
   _lread(file_handle,bitmap->buffer,bitmap->bitmapinfoheader.biSizeImage);

   } // end if
else
   {
   // serious problem
   return(0);

   } // end else

#if 0
// write the file info out 
printf("\nfilename:%s \nsize=%d \nwidth=%d \nheight=%d \nbitsperpixel=%d \ncolors=%d \nimpcolors=%d",
        filename,
        bitmap->bitmapinfoheader.biSizeImage,
        bitmap->bitmapinfoheader.biWidth,
        bitmap->bitmapinfoheader.biHeight,
		bitmap->bitmapinfoheader.biBitCount,
        bitmap->bitmapinfoheader.biClrUsed,
        bitmap->bitmapinfoheader.biClrImportant);
#endif

// close the file
_lclose(file_handle);

// flip the bitmap
Flip_Bitmap(bitmap->buffer, 
            bitmap->bitmapinfoheader.biWidth*(bitmap->bitmapinfoheader.biBitCount/8), 
            bitmap->bitmapinfoheader.biHeight);

// return success
return(1);

} // end Load_Bitmap_File

///////////////////////////////////////////////////////////

int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap)
{
// this function releases all memory associated with "bitmap"
if (bitmap->buffer)
   {
   // release memory
   free(bitmap->buffer);

   // reset pointer
   bitmap->buffer = NULL;

   } // end if

// return success
return(1);

} // end Unload_Bitmap_File

///////////////////////////////////////////////////////////

int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height)
{
// this function is used to flip bottom-up .BMP images

UCHAR *buffer; // used to perform the image processing
int index;     // looping index

// allocate the temporary buffer
if (!(buffer = (UCHAR *)malloc(bytes_per_line*height)))
   return(0);

// copy image to work area
memcpy(buffer,image,bytes_per_line*height);

// flip vertically
for (index=0; index < height; index++)
    memcpy(&image[((height-1) - index)*bytes_per_line],
           &buffer[index*bytes_per_line], bytes_per_line);

// release the memory
free(buffer);

// return success
return(1);

} // end Flip_Bitmap

///////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////   

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,int color)
{
DDBLTFX ddbltfx; // this contains the DDBLTFX structure

// clear out the structure and set the size field 
DDRAW_INIT_STRUCT(ddbltfx);

// set the dwfillcolor field to the desired color
ddbltfx.dwFillColor = color; 

// ready to blt to surface
lpdds->Blt(NULL,       // ptr to dest rectangle
           NULL,       // ptr to source surface, NA            
           NULL,       // ptr to source rectangle, NA
           DDBLT_COLORFILL | DDBLT_WAIT,   // fill and wait                   
           &ddbltfx);  // ptr to DDBLTFX structure

// return success
return(1);
} // end DDraw_Fill_Surface

///////////////////////////////////////////////////////////////

int DDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source, // source surface to draw
                      int x, int y,                 // position to draw at
                      int width, int height,        // size of source surface
                      LPDIRECTDRAWSURFACE7 dest,    // surface to draw the surface on
                      int transparent = 1)          // transparency flag
{
// draw a bob at the x,y defined in the BOB
// on the destination surface defined in dest

RECT dest_rect,   // the destination rectangle
     source_rect; // the source rectangle                             

// fill in the destination rect
dest_rect.left   = x;
dest_rect.top    = y;
dest_rect.right  = x+width-1;
dest_rect.bottom = y+height-1;

// fill in the source rect
source_rect.left    = 0;
source_rect.top     = 0;
source_rect.right   = width-1;
source_rect.bottom  = height-1;

// test transparency flag

if (transparent)
   {
   // enable color key blit
   // blt to destination surface
   if (FAILED(dest->Blt(&dest_rect, source,
                     &source_rect,(DDBLT_WAIT | DDBLT_KEYSRC),
                     NULL)))
           return(0);

   } // end if
else
   {
   // perform blit without color key
   // blt to destination surface
   if (FAILED(dest->Blt(&dest_rect, source,
                     &source_rect,(DDBLT_WAIT),
                     NULL)))
           return(0);

   } // end if

// return success
return(1);

} // end DDraw_Draw_Surface

///////////////////////////////////////////////////////////////

int Scan_Image_Bitmap(BITMAP_FILE_PTR bitmap,     // bitmap file to scan image data from
                      LPDIRECTDRAWSURFACE7 lpdds, // surface to hold data
                      int cx, int cy)             // cell to scan image from
{
// this function extracts a bitmap out of a bitmap file

UCHAR *source_ptr,   // working pointers
      *dest_ptr;

DDSURFACEDESC2 ddsd;  //  direct draw surface description 

// get the addr to destination surface memory

// set size of the structure
ddsd.dwSize = sizeof(ddsd);

// lock the display surface
lpdds->Lock(NULL,
            &ddsd,
            DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
            NULL);

// compute position to start scanning bits from
cx = cx*(ddsd.dwWidth+1) + 1;
cy = cy*(ddsd.dwHeight+1) + 1;

gwidth  = ddsd.dwWidth;
gheight = ddsd.dwHeight;

// extract bitmap data
source_ptr = bitmap->buffer + cy*bitmap->bitmapinfoheader.biWidth+cx;

// assign a pointer to the memory surface for manipulation
dest_ptr = (UCHAR *)ddsd.lpSurface;

// iterate thru each scanline and copy bitmap
for (int index_y=0; index_y < ddsd.dwHeight; index_y++)
    {
    // copy next line of data to destination
    memcpy(dest_ptr, source_ptr, ddsd.dwWidth);

    // advance pointers
    dest_ptr   += (ddsd.lPitch); // (ddsd.dwWidth);
    source_ptr += bitmap->bitmapinfoheader.biWidth;
    } // end for index_y

// unlock the surface 
lpdds->Unlock(NULL);

// return success
return(1);

} // end Scan_Image_Bitmap

///////////////////////////////////////////////////////////////

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, int mem_flags, int color_key = 0)
{
// this function creates an offscreen plain surface

DDSURFACEDESC2 ddsd;         // working description
LPDIRECTDRAWSURFACE7 lpdds;  // temporary surface
    
// set to access caps, width, and height
memset(&ddsd,0,sizeof(ddsd));
ddsd.dwSize  = sizeof(ddsd);
ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

// set dimensions of the new bitmap surface
ddsd.dwWidth  =  width;
ddsd.dwHeight =  height;

// set surface to offscreen plain
ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | mem_flags;

// create the surface
if (FAILED(lpdd->CreateSurface(&ddsd,&lpdds,NULL)))
   return(NULL);

// test if user wants a color key
if (color_key >= 0)
   {
   // set color key to color 0
   DDCOLORKEY color_key; // used to set color key
   color_key.dwColorSpaceLowValue  = 0;
   color_key.dwColorSpaceHighValue = 0;

   // now set the color key for source blitting
   lpdds->SetColorKey(DDCKEY_SRCBLT, &color_key);
   } // end if

// return surface
return(lpdds);
} // end DDraw_Create_Surface


///////////////////////////////////////////////////////////////

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

int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds)
{
// this function draws the sent text on the sent surface 
// using color index as the color in the palette

HDC xdc; // the working dc

// get the dc from surface
if (FAILED(lpdds->GetDC(&xdc)))
   return(0);

// set the colors for the text up
SetTextColor(xdc,color);

// set background mode to transparent so black isn't copied
SetBkMode(xdc, TRANSPARENT);

// draw the text a
TextOut(xdc,x,y,text,strlen(text));

// release the dc
lpdds->ReleaseDC(xdc);

// return success
return(1);
} // end Draw_Text_GDI

///////////////////////////////////////////////////////////////

int Game_Main(void *parms = NULL, int num_parms = 0)
{
// this is the main loop of the game, do all your processing
// here

// lookup for proper walking sequence
static int animation_seq[4] = {0,1,0,2};

int index; // general looping variable

// make sure this isn't executed again
if (window_closed)
   return(0);

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   {
   PostMessage(main_window_handle,WM_CLOSE,0,0);
   window_closed = 1;
   } // end if

// copy background to back buffer
DDraw_Draw_Surface(lpddsbackground,0,0, SCREEN_WIDTH,SCREEN_HEIGHT, lpddsback,0);    

// move objects around

for (index=0; index < 3; index++)
    {
    // move each object to the right at its given velocity
    aliens[index].x++; // =aliens[index].velocity;

    // test if off screen edge, and wrap around
    if (aliens[index].x > SCREEN_WIDTH)
       aliens[index].x = - 80;

    // animate bot
    if (++aliens[index].counter >= (8 - aliens[index].velocity))
        {
        // reset counter
        aliens[index].counter = 0;

        // advance to next frame
        if (++aliens[index].current_frame > 3)
           aliens[index].current_frame = 0;
  
        } // end if

    } // end for index


// draw all the bots
for (index=0; index < 3; index++)
    {
    // draw objects
    DDraw_Draw_Surface(aliens[index].frames[animation_seq[aliens[index].current_frame]], 
                       aliens[index].x, aliens[index].y,
                       72,80,
                       lpddsback);

    } // end for index


// flip pages
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

// set display mode to 640x480x8
if (FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,0,0)))
   return(0);

// we need a complex surface system with a primary and backbuffer

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

// create the palette object
if (FAILED(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256 | 
                                DDPCAPS_INITIALIZE, 
                                palette,&lpddpal, NULL)))
return(0);

// finally attach the palette to the primary surface
if (FAILED(lpddsprimary->SetPalette(lpddpal)))
   return(0);

// set clipper up on back buffer since that's where well clip
RECT screen_rect= {0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1};
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

// load the 8-bit image
if (!Load_Bitmap_File(&bitmap,"alley8.bmp"))
   return(0);

// load it's palette into directdraw
if (FAILED(lpddpal->SetEntries(0,0,MAX_COLORS_PALETTE,bitmap.palette)))
   return(0);

// clean the surfaces
DDraw_Fill_Surface(lpddsprimary,0);
DDraw_Fill_Surface(lpddsback,0);

// create the buffer to hold the background
lpddsbackground = DDraw_Create_Surface(640,480,0,-1);

// copy the background bitmap image to the background surface 

// lock the surface
lpddsbackground->Lock(NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);

// get video pointer to primary surfce
UCHAR *image_buffer = (UCHAR *)ddsd.lpSurface;       

// test if memory is linear
if (ddsd.lPitch == SCREEN_WIDTH)
   {
   // copy memory from double buffer to primary buffer
   memcpy((void *)image_buffer, (void *)bitmap.buffer, SCREEN_WIDTH*SCREEN_HEIGHT);
   } // end if
else
   { // non-linear

   // make copy of source and destination addresses
   UCHAR *dest_ptr = image_buffer;
   UCHAR *src_ptr  = bitmap.buffer;

   // memory is non-linear, copy line by line
   for (int y=0; y < SCREEN_HEIGHT; y++)
       {
       // copy line
       memcpy((void *)dest_ptr, (void *)src_ptr, SCREEN_WIDTH);

       // advance pointers to next line
       dest_ptr+=ddsd.lPitch;
       src_ptr +=SCREEN_WIDTH;
       } // end for

   } // end else

// now unlock the primary surface
if (FAILED(lpddsbackground->Unlock(NULL)))
   return(0);

// unload the bitmap file, we no longer need it
Unload_Bitmap_File(&bitmap);

// seed random number generator
srand(GetTickCount());

// initialize all the aliens

// alien on level 1 of complex

aliens[0].x              = rand()%SCREEN_WIDTH;
aliens[0].y              = 116 - 72;                  
aliens[0].velocity       = 2+rand()%4;
aliens[0].current_frame  = 0;             
aliens[0].counter        = 0;       

// alien on level 2 of complex

aliens[1].x              = rand()%SCREEN_WIDTH;
aliens[1].y              = 246 - 72;                  
aliens[1].velocity       = 2+rand()%4;
aliens[1].current_frame  = 0;             
aliens[1].counter        = 0;  

// alien on level 3 of complex

aliens[2].x              = rand()%SCREEN_WIDTH;
aliens[2].y              = 382 - 72;                  
aliens[2].velocity       = 2+rand()%4;
aliens[2].current_frame  = 0;             
aliens[2].counter        = 0;  

// now load the bitmap containing the alien imagery
// then scan the images out into the surfaces of alien[0]
// and copy then into the other two, be careful of reference counts!

// load the 8-bit image
if (!Load_Bitmap_File(&bitmap,"dedsp0.bmp"))
   return(0);

// create each surface and load bits
for (int index = 0; index < 3; index++)
    {
    // create surface to hold image
    aliens[0].frames[index] = DDraw_Create_Surface(72,80,0);

    // now load bits...
    Scan_Image_Bitmap(&bitmap,                 // bitmap file to scan image data from
                      aliens[0].frames[index], // surface to hold data
                      index, 0);               // cell to scan image from    

    } // end for index

// unload the bitmap file, we no longer need it
Unload_Bitmap_File(&bitmap);

// now for the tricky part. There is no need to create more surfaces with the same
// data, so I'm going to copy the surface pointers member for member to each alien
// however, be careful, since the reference counts do NOT go up, you still only need
// to release() each surface once!

for (index = 0; index < 3; index++)
    aliens[1].frames[index] = aliens[2].frames[index] = aliens[0].frames[index];

// return success or failure or your own return code here
return(1);

} // end Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
// this is called after the game is exited and the main event
// loop while is exited, do all you cleanup and shutdown here

// kill all the surfaces


// first the palette
if (lpddpal)
   {
   lpddpal->Release();
   lpddpal = NULL;
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
						    "DirectDraw 8-Bit Blitting Demo", // title
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

