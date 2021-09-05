// BLACKBOX.CPP - Game Engine 
 
// INCLUDES ///////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  // make sure all macros are included


#include <windows.h>         // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>

#include <iostream.h>        // include important C/C++ stuff
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

#include <ddraw.h>           // directX includes
#include "blackbox.h"        // game library includes
                                   
// DEFINES ////////////////////////////////////////////////////

// TYPES //////////////////////////////////////////////////////

// PROTOTYPES /////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////

extern HWND main_window_handle; // save the window handle
extern HINSTANCE main_instance; // save the instance

// GLOBALS ////////////////////////////////////////////////////

LPDIRECTDRAW7         lpdd         = NULL;   // dd object
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

// these defined the general clipping rectangle
int min_clip_x = 0,                          // clipping rectangle 
    max_clip_x = SCREEN_WIDTH-1,
    min_clip_y = 0,
    max_clip_y = SCREEN_HEIGHT-1;

// these are overwritten globally by DD_Init()
int screen_width  = SCREEN_WIDTH,            // width of screen
    screen_height = SCREEN_HEIGHT,           // height of screen
    screen_bpp    = SCREEN_BPP;              // bits per pixel

// FUNCTIONS //////////////////////////////////////////////////

int DD_Init(int width, int height, int bpp)
{
// this function initializes directdraw
int index; // looping variable

// create object and test for error
if (DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL)!=DD_OK)
   return(0);

// set cooperation level to windowed mode normal
if (lpdd->SetCooperativeLevel(main_window_handle,
           DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN | 
           DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT)!=DD_OK)
    return(0);

// set the display mode
if (lpdd->SetDisplayMode(width,height,bpp,0,0)!=DD_OK)
   return(0);

// set globals
screen_height = height;
screen_width  = width;
screen_bpp    = bpp;

// Create the primary surface
memset(&ddsd,0,sizeof(ddsd));
ddsd.dwSize = sizeof(ddsd);
ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;

// we need to let dd know that we want a complex 
// flippable surface structure, set flags for that
ddsd.ddsCaps.dwCaps = 
  DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

// set the backbuffer count to 1
ddsd.dwBackBufferCount = 1;

// create the primary surface
lpdd->CreateSurface(&ddsd,&lpddsprimary,NULL);

// query for the backbuffer i.e the secondary surface
ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
lpddsprimary->GetAttachedSurface(&ddscaps,&lpddsback);

// create and attach palette

// create palette data
// clear all entries defensive programming
memset(palette,0,256*sizeof(PALETTEENTRY));

// create a R,G,B,GR gradient palette
for (index=0; index < 256; index++)
    {
    // set each entry
    if (index < 64) 
        palette[index].peRed = index*4; 
    else           // shades of green
    if (index >= 64 && index < 128) 
        palette[index].peGreen = (index-64)*4;
    else           // shades of blue
    if (index >= 128 && index < 192) 
       palette[index].peBlue = (index-128)*4;
    else           // shades of grey
    if (index >= 192 && index < 256) 
        palette[index].peRed = palette[index].peGreen = 
        palette[index].peBlue = (index-192)*4;
    
    // set flags
    palette[index].peFlags = PC_NOCOLLAPSE;
    } // end for index

// now create the palette object
if (lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE | DDPCAPS_ALLOW256,
                         palette,&lpddpal,NULL)!=DD_OK)
   return(0);

// attach the palette to the primary
if (lpddsprimary->SetPalette(lpddpal)!=DD_OK)
   return(0);

// clear out both primary and secondary surfaces
DD_Fill_Surface(lpddsprimary,0);
DD_Fill_Surface(lpddsback,0);

// attach a clipper to the screen
RECT screen_rect = {0,0,screen_width,screen_height};
lpddclipper = DD_Attach_Clipper(lpddsback,1,&screen_rect);

// return success
return(1);
} // end DD_Init

///////////////////////////////////////////////////////////////

int DD_Shutdown(void)
{
// this function release all the resources directdraw
// allocated, mainly to com objects

// release the clipper first
if (lpddclipper)
    lpddclipper->Release();

// release the palette
if (lpddpal)
   lpddpal->Release();

// release the secondary surface
if (lpddsback)
    lpddsback->Release();

// release the primary surface
if (lpddsprimary)
   lpddsprimary->Release();

// finally, the main dd object
if (lpdd)
    lpdd->Release();

// return success
return(1);
} // end DD_Shutdown

///////////////////////////////////////////////////////////////   

LPDIRECTDRAWCLIPPER DD_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds,
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
if ((lpdd->CreateClipper(0,&lpddclipper,NULL))!=DD_OK)
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

if ((lpddclipper->SetClipList(region_data, 0))!=DD_OK)
   {
   // release memory and return error
   free(region_data);
   return(NULL);
   } // end if

// now attach the clipper to the surface
if ((lpdds->SetClipper(lpddclipper))!=DD_OK)
   {
   // release memory and return error
   free(region_data);
   return(NULL);
   } // end if

// all is well, so release memory and send back the pointer to the new clipper
free(region_data);
return(lpddclipper);

} // end DD_Attach_Clipper

///////////////////////////////////////////////////////////////
   
int DD_Flip(void)
{
// this function flip the primary surface with the secondary surface

// flip pages
while(lpddsprimary->Flip(NULL, DDFLIP_WAIT)!=DD_OK);

// flip the surface

// return success
return(1);

} // end DD_Flip

///////////////////////////////////////////////////////////////

DWORD Start_Clock(void)
{
// this function starts the clock, that is, saves the current
// count, use in conjunction with Wait_Clock()

return(start_clock_count = Get_Clock());

} // end Start_Clock

///////////////////////////////////////////////////////////////

DWORD Get_Clock(void)
{
// this function returns the current tick count

// return time
return(GetTickCount());

} // end Get_Clock

///////////////////////////////////////////////////////////////

DWORD Wait_Clock(DWORD count)
{
// this function is used to wait for a specific number of clicks
// since the call to Start_Clock

while((Get_Clock() - start_clock_count) < count);
return(Get_Clock());

} // end Wait_Clock

///////////////////////////////////////////////////////////////

int DD_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,int color)
{
DDBLTFX ddbltfx; // this contains the DDBLTFX structure

// clear out the structure and set the size field 
DD_INIT_STRUCT(ddbltfx);

// set the dwfillcolor field to the desired color
ddbltfx.dwFillColor = color; 

// ready to blt to surface
lpdds->Blt(NULL,       // ptr to dest rectangle
           NULL,       // ptr to source surface, NA            
           NULL,       // ptr to source rectangle, NA
           DDBLT_COLORFILL | DDBLT_WAIT | DDBLT_ASYNC,   // fill and wait                   
           &ddbltfx);  // ptr to DDBLTFX structure

// return success
return(1);
} // end DD_Fill_Surface

///////////////////////////////////////////////////////////////   

int Draw_Rectangle(int x1, int y1, int x2, int y2, int color,
                   LPDIRECTDRAWSURFACE7 lpdds)
{
// this function uses directdraw to draw a filled rectangle

DDBLTFX ddbltfx; // this contains the DDBLTFX structure
RECT fill_area;  // this contains the destination rectangle

// clear out the structure and set the size field 
DD_INIT_STRUCT(ddbltfx);

// set the dwfillcolor field to the desired color
ddbltfx.dwFillColor = color; 

// fill in the destination rectangle data (your data)
fill_area.top    = y1;
fill_area.left   = x1;
fill_area.bottom = y2+1;
fill_area.right  = x2+1;

// ready to blt to surface, in this case blt to primary
lpdds->Blt(&fill_area, // ptr to dest rectangle
           NULL,       // ptr to source surface, NA            
           NULL,       // ptr to source rectangle, NA
           DDBLT_COLORFILL | DDBLT_WAIT | DDBLT_ASYNC,   // fill and wait                   
           &ddbltfx);  // ptr to DDBLTFX structure

// return success
return(1);

} // end Draw_Rectangle

///////////////////////////////////////////////////////////////

int Draw_Text_GDI(char *text, int x,int y,int color, LPDIRECTDRAWSURFACE7 lpdds)
{
// this function draws the sent text on the sent surface 
// using color index as the color in the palette

HDC xdc; // the working dc

// get the dc from surface
if (lpdds->GetDC(&xdc)!=DD_OK)
   return(0);

// set the colors for the text up
SetTextColor(xdc,RGB(palette[color].peRed,palette[color].peGreen,palette[color].peBlue) );

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

