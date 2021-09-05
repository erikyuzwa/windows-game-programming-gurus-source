// DEMO8_8.CPP 8-bit quad drawing demo

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

const double PI = 3.1415926535;

// fixed point mathematics constants
#define FIXP16_SHIFT     16
#define FIXP16_MAG       65536
#define FIXP16_DP_MASK   0x0000ffff
#define FIXP16_WP_MASK   0xffff0000
#define FIXP16_ROUND_UP  0x00008000

// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// a 2D vertex
typedef struct VERTEX2DI_TYP
        {
        int x,y; // the vertex
        } VERTEX2DI, *VERTEX2DI_PTR;

// a 2D vertex
typedef struct VERTEX2DF_TYP
        {
        float x,y; // the vertex
        } VERTEX2DF, *VERTEX2DF_PTR;


// a 2D polygon
typedef struct POLYGON2D_TYP
        {
        int state;        // state of polygon
        int num_verts;    // number of vertices
        int x0,y0;        // position of center of polygon  
        int xv,yv;        // initial velocity
        DWORD color;      // could be index or PALETTENTRY
        VERTEX2DF *vlist; // pointer to vertex list
 
        } POLYGON2D, *POLYGON2D_PTR;


// PROTOTYPES  //////////////////////////////////////////////

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,int color);

// 2d 8-bit triangle rendering
void Draw_Top_Tri(int x1,int y1,int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);

void Draw_Bottom_Tri(int x1,int y1, int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);

void Draw_Top_TriFP(int x1,int y1,int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);

void Draw_Bottom_TriFP(int x1,int y1, int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);

void Draw_Triangle_2D(int x1,int y1,int x2,int y2,int x3,int y3,
                      int color,UCHAR *dest_buffer, int mempitch);

void Draw_TriangleFP_2D(int x1,int y1,int x2,int y2,int x3,int y3,
                        int color,UCHAR *dest_buffer, int mempitch);

inline void Draw_QuadFP_2D(int x0,int y0,int x1,int y1,
                           int x2,int y2,int x3, int y3,
                           int color,UCHAR *dest_buffer, int mempitch);

int Translate_Polygon2D(POLYGON2D_PTR poly, int dx, int dy);

int Rotate_Polygon2D(POLYGON2D_PTR poly, int theta);

int Scale_Polygon2D(POLYGON2D_PTR poly, float sx, float sy);

int Set_Palette_Entry(int color_index, LPPALETTEENTRY color);

int Draw_Text_GDI(char *text, int x,int y,int color, LPDIRECTDRAWSURFACE7 lpdds);

// MACROS /////////////////////////////////////////////////

// tests if a key is up or down
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// initializes a direct draw struct
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// some math macros
#define DEG_TO_RAD(ang) ((ang)*PI/180)
#define RAD_TO_DEG(rads) ((rads)*180/PI)

// GLOBALS ////////////////////////////////////////////////

HWND      main_window_handle = NULL; // globally track main window
int       window_closed      = 0;    // tracks if window is closed
HINSTANCE hinstance_app      = NULL; // globally track hinstance

// directdraw stuff
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


// global clipping region

int min_clip_x = 0,      // clipping rectangle 
    max_clip_x = SCREEN_WIDTH - 1,
    min_clip_y = 0,
    max_clip_y = SCREEN_HEIGHT - 1;

char buffer[80];                             // general printing buffer

// storage for our lookup tables
float cos_look[360];
float sin_look[360];

POLYGON2D object; // the polygon object

// FUNCTIONS ////////////////////////////////////////////////

inline void Draw_QuadFP_2D(int x0,int y0,
                    int x1,int y1,
                    int x2,int y2,
                    int x3, int y3,
                    int color,
                    UCHAR *dest_buffer, int mempitch)
{
// this function draws a 2D quadrilateral

// simply call the triangle function 2x, let it do all the work
Draw_TriangleFP_2D(x0,y0,x1,y1,x3,y3,color,dest_buffer,mempitch);
Draw_TriangleFP_2D(x1,y1,x2,y2,x3,y3,color,dest_buffer,mempitch);

} // end Draw_QuadFP_2D

////////////////////////////////////////////////////////////////////////////////

void Draw_Top_TriFP(int x1,int y1,
                    int x2,int y2, 
                    int x3,int y3,
                    int color, 
                    UCHAR *dest_buffer, int mempitch)
{
// this function draws a triangle that has a flat top using fixed point math

int dx_right,    // the dx/dy ratio of the right edge of line
    dx_left,     // the dx/dy ratio of the left edge of line
    xs,xe,       // the starting and ending points of the edges
    height;      // the height of the triangle

int temp_x,        // used during sorting as temps
    temp_y,
    right,         // used by clipping
    left;

UCHAR  *dest_addr;

// test for degenerate
if (y1==y3 || y2==y3)
	return;

// test order of x1 and x2
if (x2 < x1)
   {
   temp_x = x2;
   x2     = x1;
   x1     = temp_x;
   } // end if swap

// compute delta's
height = y3-y1;

dx_left  = ((x3-x1)<<FIXP16_SHIFT)/height;
dx_right = ((x3-x2)<<FIXP16_SHIFT)/height;

// set starting points
xs = (x1<<FIXP16_SHIFT);
xe = (x2<<FIXP16_SHIFT);

// perform y clipping
if (y1<min_clip_y)
   {
   // compute new xs and ys
   xs = xs+dx_left*(-y1+min_clip_y);
   xe = xe+dx_right*(-y1+min_clip_y);

   // reset y1
   y1=min_clip_y;

   } // end if top is off screen

if (y3>max_clip_y)
   y3=max_clip_y;

// compute starting address in video memory
dest_addr = dest_buffer+y1*mempitch;

// test if x clipping is needed
if (x1>=min_clip_x && x1<=max_clip_x &&
    x2>=min_clip_x && x2<=max_clip_x &&
    x3>=min_clip_x && x3<=max_clip_x)
    {
    // draw the triangle
    for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=mempitch)
        {
        memset((UCHAR *)dest_addr+((xs+FIXP16_ROUND_UP)>>FIXP16_SHIFT),
               color, (((xe-xs+FIXP16_ROUND_UP)>>FIXP16_SHIFT)+1));

        // adjust starting point and ending point
        xs+=dx_left;
        xe+=dx_right;

        } // end for

    } // end if no x clipping needed
else
   {
   // clip x axis with slower version

   // draw the triangle
   for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=mempitch)
       {
       // do x clip
       left  = ((xs+FIXP16_ROUND_UP)>>16);
       right = ((xe+FIXP16_ROUND_UP)>>16);

       // adjust starting point and ending point
       xs+=dx_left;
       xe+=dx_right;

       // clip line
       if (left < min_clip_x)
          {
          left = min_clip_x;

          if (right < min_clip_x)
             continue;
          }

       if (right > max_clip_x)
          {
          right = max_clip_x;

          if (left > max_clip_x)
             continue;
          }

       memset((UCHAR  *)dest_addr+(unsigned int)left,
              color,(unsigned int)(right-left+1));

       } // end for

   } // end else x clipping needed

} // end Draw_Top_TriFP

/////////////////////////////////////////////////////////////////////////////

void Draw_Bottom_TriFP(int x1,int y1, 
                       int x2,int y2, 
                       int x3,int y3,
                       int color,
                       UCHAR *dest_buffer, int mempitch)
{

// this function draws a triangle that has a flat bottom using fixed point math

int dx_right,    // the dx/dy ratio of the right edge of line
    dx_left,     // the dx/dy ratio of the left edge of line
    xs,xe,       // the starting and ending points of the edges
    height;      // the height of the triangle

int temp_x,        // used during sorting as temps
    temp_y,
    right,         // used by clipping
    left;

UCHAR  *dest_addr;

if (y1==y2 || y1==y3)
	return;

// test order of x1 and x2
if (x3 < x2)
   {
   temp_x = x2;
   x2     = x3;
   x3     = temp_x;

   } // end if swap

// compute delta's
height = y3-y1;

dx_left  = ((x2-x1)<<FIXP16_SHIFT)/height;
dx_right = ((x3-x1)<<FIXP16_SHIFT)/height;

// set starting points
xs = (x1<<FIXP16_SHIFT);
xe = (x1<<FIXP16_SHIFT); 

// perform y clipping
if (y1<min_clip_y)
   {
   // compute new xs and ys
   xs = xs+dx_left*(-y1+min_clip_y);
   xe = xe+dx_right*(-y1+min_clip_y);

   // reset y1
   y1=min_clip_y;

   } // end if top is off screen

if (y3>max_clip_y)
   y3=max_clip_y;

// compute starting address in video memory
dest_addr = dest_buffer+y1*mempitch;

// test if x clipping is needed
if (x1>=min_clip_x && x1<=max_clip_x &&
    x2>=min_clip_x && x2<=max_clip_x &&
    x3>=min_clip_x && x3<=max_clip_x)
    {
    // draw the triangle
    for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=mempitch)
        {
        memset((UCHAR *)dest_addr+((xs+FIXP16_ROUND_UP)>>FIXP16_SHIFT),
                color, (((xe-xs+FIXP16_ROUND_UP)>>FIXP16_SHIFT)+1));

        // adjust starting point and ending point
        xs+=dx_left;
        xe+=dx_right;

        } // end for

    } // end if no x clipping needed
else
   {
   // clip x axis with slower version

   // draw the triangle
   for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=mempitch)
       {
       // do x clip
       left  = ((xs+FIXP16_ROUND_UP)>>FIXP16_SHIFT);
       right = ((xe+FIXP16_ROUND_UP)>>FIXP16_SHIFT);

       // adjust starting point and ending point
       xs+=dx_left;
       xe+=dx_right;

       // clip line
       if (left < min_clip_x)
          {
          left = min_clip_x;

          if (right < min_clip_x)
             continue;
          }

       if (right > max_clip_x)
          {
          right = max_clip_x;

          if (left > max_clip_x)
             continue;
          }

       memset((UCHAR *)dest_addr+left,
              color, (right-left+1));

       } // end for

   } // end else x clipping needed

} // end Draw_Bottom_TriFP

////////////////////////////////////////////////////////////////

void Draw_Top_Tri(int x1,int y1, 
                  int x2,int y2, 
                  int x3,int y3,
                  int color, 
                  UCHAR *dest_buffer, int mempitch)
{
// this function draws a triangle that has a flat top

float dx_right,    // the dx/dy ratio of the right edge of line
      dx_left,     // the dx/dy ratio of the left edge of line
      xs,xe,       // the starting and ending points of the edges
      height;      // the height of the triangle

int temp_x,        // used during sorting as temps
    temp_y,
    right,         // used by clipping
    left;

// destination address of next scanline
UCHAR  *dest_addr = NULL;

// test order of x1 and x2
if (x2 < x1)
   {
   temp_x = x2;
   x2     = x1;
   x1     = temp_x;
   } // end if swap

// compute delta's
height = y3-y1;

dx_left  = (x3-x1)/height;
dx_right = (x3-x2)/height;

// set starting points
xs = (float)x1;
xe = (float)x2+(float)0.5;

// perform y clipping
if (y1 < min_clip_y)
   {
   // compute new xs and ys
   xs = xs+dx_left*(float)(-y1+min_clip_y);
   xe = xe+dx_right*(float)(-y1+min_clip_y);

   // reset y1
   y1=min_clip_y;

   } // end if top is off screen

if (y3>max_clip_y)
   y3=max_clip_y;

// compute starting address in video memory
dest_addr = dest_buffer+y1*mempitch;

// test if x clipping is needed
if (x1>=min_clip_x && x1<=max_clip_x &&
    x2>=min_clip_x && x2<=max_clip_x &&
    x3>=min_clip_x && x3<=max_clip_x)
    {
    // draw the triangle
    for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=mempitch)
        {
        memset((UCHAR *)dest_addr+(unsigned int)xs,
                color,(unsigned int)(xe-xs+1));

        // adjust starting point and ending point
        xs+=dx_left;
        xe+=dx_right;

        } // end for

    } // end if no x clipping needed
else
   {
   // clip x axis with slower version

   // draw the triangle
   for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=mempitch)
       {
       // do x clip
       left  = (int)xs;
       right = (int)xe;

       // adjust starting point and ending point
       xs+=dx_left;
       xe+=dx_right;

       // clip line
       if (left < min_clip_x)
          {
          left = min_clip_x;

          if (right < min_clip_x)
             continue;
          }

       if (right > max_clip_x)
          {
          right = max_clip_x;

          if (left > max_clip_x)
             continue;
          }

       memset((UCHAR  *)dest_addr+(unsigned int)left,
              color,(unsigned int)(right-left+1));

       } // end for

   } // end else x clipping needed

} // end Draw_Top_Tri

/////////////////////////////////////////////////////////////////////////////

void Draw_Bottom_Tri(int x1,int y1, 
                     int x2,int y2, 
                     int x3,int y3,
                     int color,
                     UCHAR *dest_buffer, int mempitch)
{
// this function draws a triangle that has a flat bottom

float dx_right,    // the dx/dy ratio of the right edge of line
      dx_left,     // the dx/dy ratio of the left edge of line
      xs,xe,       // the starting and ending points of the edges
      height;      // the height of the triangle

int temp_x,        // used during sorting as temps
    temp_y,
    right,         // used by clipping
    left;

// destination address of next scanline
UCHAR  *dest_addr;

// test order of x1 and x2
if (x3 < x2)
   {
   temp_x = x2;
   x2     = x3;
   x3     = temp_x;
   } // end if swap

// compute delta's
height = y3-y1;

dx_left  = (x2-x1)/height;
dx_right = (x3-x1)/height;

// set starting points
xs = (float)x1;
xe = (float)x1; // +(float)0.5;

// perform y clipping
if (y1<min_clip_y)
   {
   // compute new xs and ys
   xs = xs+dx_left*(float)(-y1+min_clip_y);
   xe = xe+dx_right*(float)(-y1+min_clip_y);

   // reset y1
   y1=min_clip_y;

   } // end if top is off screen

if (y3>max_clip_y)
   y3=max_clip_y;

// compute starting address in video memory
dest_addr = dest_buffer+y1*mempitch;

// test if x clipping is needed
if (x1>=min_clip_x && x1<=max_clip_x &&
    x2>=min_clip_x && x2<=max_clip_x &&
    x3>=min_clip_x && x3<=max_clip_x)
    {
    // draw the triangle
    for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=mempitch)
        {
        memset((UCHAR  *)dest_addr+(unsigned int)xs,
                color,(unsigned int)(xe-xs+1));

        // adjust starting point and ending point
        xs+=dx_left;
        xe+=dx_right;

        } // end for

    } // end if no x clipping needed
else
   {
   // clip x axis with slower version

   // draw the triangle

   for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=mempitch)
       {
       // do x clip
       left  = (int)xs;
       right = (int)xe;

       // adjust starting point and ending point
       xs+=dx_left;
       xe+=dx_right;

       // clip line
       if (left < min_clip_x)
          {
          left = min_clip_x;

          if (right < min_clip_x)
             continue;
          }

       if (right > max_clip_x)
          {
          right = max_clip_x;

          if (left > max_clip_x)
             continue;
          }

       memset((UCHAR  *)dest_addr+(unsigned int)left,
              color,(unsigned int)(right-left+1));

       } // end for

   } // end else x clipping needed

} // end Draw_Bottom_Tri

///////////////////////////////////////////////////////////////////////////////

void Draw_TriangleFP_2D(int x1,int y1,
                        int x2,int y2,
                        int x3,int y3,
                        int color,
    	   			    UCHAR *dest_buffer, int mempitch)
{
// this function draws a triangle on the destination buffer using fixed point
// it decomposes all triangles into a pair of flat top, flat bottom

int temp_x, // used for sorting
    temp_y,
    new_x;

// test for h lines and v lines
if ((x1==x2 && x2==x3)  ||  (y1==y2 && y2==y3))
   return;

// sort p1,p2,p3 in ascending y order
if (y2<y1)
   {
   temp_x = x2;
   temp_y = y2;
   x2     = x1;
   y2     = y1;
   x1     = temp_x;
   y1     = temp_y;
   } // end if

// now we know that p1 and p2 are in order
if (y3<y1)
   {
   temp_x = x3;
   temp_y = y3;
   x3     = x1;
   y3     = y1;
   x1     = temp_x;
   y1     = temp_y;
   } // end if

// finally test y3 against y2
if (y3<y2)
   {
   temp_x = x3;
   temp_y = y3;
   x3     = x2;
   y3     = y2;
   x2     = temp_x;
   y2     = temp_y;

   } // end if

// do trivial rejection tests for clipping
if ( y3<min_clip_y || y1>max_clip_y ||
    (x1<min_clip_x && x2<min_clip_x && x3<min_clip_x) ||
    (x1>max_clip_x && x2>max_clip_x && x3>max_clip_x) )
   return;

// test if top of triangle is flat
if (y1==y2)
   {
   Draw_Top_TriFP(x1,y1,x2,y2,x3,y3,color, dest_buffer, mempitch);
   } // end if
else
if (y2==y3)
   {
   Draw_Bottom_TriFP(x1,y1,x2,y2,x3,y3,color, dest_buffer, mempitch);
   } // end if bottom is flat
else
   {
   // general triangle that's needs to be broken up along long edge
   new_x = x1 + (int)(0.5+(float)(y2-y1)*(float)(x3-x1)/(float)(y3-y1));

   // draw each sub-triangle
   Draw_Bottom_TriFP(x1,y1,new_x,y2,x2,y2,color, dest_buffer, mempitch);
   Draw_Top_TriFP(x2,y2,new_x,y2,x3,y3,color, dest_buffer, mempitch);

   } // end else

} // end Draw_TriangleFP_2D

/////////////////////////////////////////////////////////////

void Draw_Triangle_2D(int x1,int y1,
                      int x2,int y2,
                      int x3,int y3,
                      int color,
					  UCHAR *dest_buffer, int mempitch)
{
// this function draws a triangle on the destination buffer
// it decomposes all triangles into a pair of flat top, flat bottom

int temp_x, // used for sorting
    temp_y,
    new_x;

// test for h lines and v lines
if ((x1==x2 && x2==x3)  ||  (y1==y2 && y2==y3))
   return;

// sort p1,p2,p3 in ascending y order
if (y2<y1)
   {
   temp_x = x2;
   temp_y = y2;
   x2     = x1;
   y2     = y1;
   x1     = temp_x;
   y1     = temp_y;
   } // end if

// now we know that p1 and p2 are in order
if (y3<y1)
   {
   temp_x = x3;
   temp_y = y3;
   x3     = x1;
   y3     = y1;
   x1     = temp_x;
   y1     = temp_y;
   } // end if

// finally test y3 against y2
if (y3<y2)
   {
   temp_x = x3;
   temp_y = y3;
   x3     = x2;
   y3     = y2;
   x2     = temp_x;
   y2     = temp_y;

   } // end if

// do trivial rejection tests for clipping
if ( y3<min_clip_y || y1>max_clip_y ||
    (x1<min_clip_x && x2<min_clip_x && x3<min_clip_x) ||
    (x1>max_clip_x && x2>max_clip_x && x3>max_clip_x) )
   return;

// test if top of triangle is flat
if (y1==y2)
   {
   Draw_Top_Tri(x1,y1,x2,y2,x3,y3,color, dest_buffer, mempitch);
   } // end if
else
if (y2==y3)
   {
   Draw_Bottom_Tri(x1,y1,x2,y2,x3,y3,color, dest_buffer, mempitch);
   } // end if bottom is flat
else
   {
   // general triangle that's needs to be broken up along long edge
   new_x = x1 + (int)(0.5+(float)(y2-y1)*(float)(x3-x1)/(float)(y3-y1));

   // draw each sub-triangle
   Draw_Bottom_Tri(x1,y1,new_x,y2,x2,y2,color, dest_buffer, mempitch);
   Draw_Top_Tri(x2,y2,new_x,y2,x3,y3,color, dest_buffer, mempitch);

   } // end else

} // end Draw_Triangle_2D

////////////////////////////////////////////////////////////////////////////////


int Draw_Text_GDI(char *text, int x,int y,int color, LPDIRECTDRAWSURFACE7 lpdds)
{
// this function draws the sent text on the sent surface 
// using color index as the color in the palette

HDC xdc; // the working dc

// get the dc from surface
if (FAILED(lpdds->GetDC(&xdc)))
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

///////////////////////////////////////////////////////////////////


int Set_Palette_Entry(int color_index, LPPALETTEENTRY color)
{
// this function sets a palette color in the palette
lpddpal->SetEntries(0,color_index,1,color);

// set data in shadow palette
memcpy(&palette[color_index],color,sizeof(PALETTEENTRY));

// return success
return(1);
} // end Set_Palette_Entry

///////////////////////////////////////////////////////////

int Translate_Polygon2D(POLYGON2D_PTR poly, int dx, int dy)
{
// this function translates the center of a polygon

// test for valid pointer
if (!poly)
   return(0);

// translate
poly->x0+=dx;
poly->y0+=dy;

// return success
return(1);

} // end Translate_Polygon2D

///////////////////////////////////////////////////////////////

int Rotate_Polygon2D(POLYGON2D_PTR poly, int theta)
{
// this function rotates the local coordinates of the polygon

// test for valid pointer
if (!poly)
   return(0);

// loop and rotate each point, very crude, no lookup!!!
for (int curr_vert = 0; curr_vert < poly->num_verts; curr_vert++)
    {

    // perform rotation
    float xr = (float)poly->vlist[curr_vert].x*cos_look[theta] - 
                    (float)poly->vlist[curr_vert].y*sin_look[theta];

    float yr = (float)poly->vlist[curr_vert].x*sin_look[theta] + 
                    (float)poly->vlist[curr_vert].y*cos_look[theta];

    // store result back
    poly->vlist[curr_vert].x = xr;
    poly->vlist[curr_vert].y = yr;

    } // end for curr_vert

// return success
return(1);

} // end Rotate_Polygon2D

////////////////////////////////////////////////////////

int Scale_Polygon2D(POLYGON2D_PTR poly, float sx, float sy)
{
// this function scalesthe local coordinates of the polygon

// test for valid pointer
if (!poly)
   return(0);

// loop and scale each point
for (int curr_vert = 0; curr_vert < poly->num_verts; curr_vert++)
    {
    // scale and store result back
    poly->vlist[curr_vert].x *= sx;
    poly->vlist[curr_vert].y *= sy;

    } // end for curr_vert

// return success
return(1);

} // end Scale_Polygon2D

///////////////////////////////////////////////////////////

inline int Draw_Pixel(int x, int y,int color,
                      UCHAR *video_buffer, int lpitch)
{
// this function plots a single pixel at x,y with color
video_buffer[x + y*lpitch] = color;

// return success
return(1);

} // end Draw_Pixel

/////////////////////////////////////////////////////////////

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

// make sure this isn't executed again
if (window_closed)
   return(0);

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   {
   PostMessage(main_window_handle,WM_CLOSE,0,0);
   window_closed = 1;
   } // end if

// clear out the back buffer
DDraw_Fill_Surface(lpddsback, 0);

// lock primary buffer
DDRAW_INIT_STRUCT(ddsd);

if (FAILED(lpddsback->Lock(NULL,&ddsd,
                           DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
                           NULL)))
return(0);

// generate vertices, note these can be anywhere in space, but they must be in CW order
int x0     = rand()%SCREEN_WIDTH;
int y0     = rand()%SCREEN_HEIGHT;
int width  = rand()%SCREEN_WIDTH;
int height = rand()%SCREEN_HEIGHT;

// draw the triangle
Draw_QuadFP_2D(x0,y0,
               x0+width,y0,
               x0+width, y0+height, 
               x0, y0+height,
               rand()%256,(UCHAR *)ddsd.lpSurface, ddsd.lPitch);

// unlock primary buffer
if (FAILED(lpddsback->Unlock(NULL)))
   return(0);

// draw the text
Draw_Text_GDI("Press <ESC> to exit.", 8,8,255, lpddsback);

// perform the flip
while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));

// wait a sec
Sleep(33);

// return success or failure or your own return code here
return(1);

} // end Game_Main

////////////////////////////////////////////////////////////

int Game_Init(void *parms = NULL, int num_parms = 0)
{
// this is called once after the initial window is created and
// before the main event loop is entered, do all your initialization
// here

// seed random number generator
srand(GetTickCount());

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

// make this color glow green
palette[1].peRed     = 0;
palette[1].peGreen   = 16;
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

// clear the surfaces out
DDraw_Fill_Surface(lpddsprimary, 0 );
DDraw_Fill_Surface(lpddsback, 0 );

// define points of object (must be convex)
VERTEX2DF object_vertices[4] = {-100,-100, 100,-100, 100,100, -100, 100};

// initialize polygon object
object.state       = 1;   // turn it on
object.num_verts   = 4;  
object.x0          = SCREEN_WIDTH/2; // position it
object.y0          = SCREEN_HEIGHT/2;
object.xv          = 0;
object.yv          = 0;
object.color       = 1; // animated green
object.vlist       = new VERTEX2DF [object.num_verts];
 
for (int index = 0; index < object.num_verts; index++)
    object.vlist[index] = object_vertices[index];

   
// create sin/cos lookup table

// generate the tables
for (int ang = 0; ang < 360; ang++)
    {
    // convert ang to radians
    float theta = (float)ang*PI/(float)180;

    // insert next entry into table
    cos_look[ang] = cos(theta);
    sin_look[ang] = sin(theta);

    } // end for ang


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
						    "DirectDraw 8-Bit Quad Drawing Demo", // title
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

