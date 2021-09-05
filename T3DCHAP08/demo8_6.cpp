// DEMO8_6.CPP 8-bit polygon transformation demo with matrices

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


// matrices
typedef struct MATRIX1X2_TYP
        {
        float M[2]; // data storage
        } MATRIX1X2, *MATRIX1X2_PTR;
// note that 1x2 has the same memory layout as a VERTEX2DF, hence we
// can use the matrix function written for a MATRIX1X2 to multiply a 
// VERTEX2DF by casting

typedef struct MATRIX3X2_TYP
        {
        float M[3][2]; // data storage
        } MATRIX3X2, *MATRIX3X2_PTR;


// PROTOTYPES  //////////////////////////////////////////////

int Draw_Text_GDI(char *text, int x,int y,int color, LPDIRECTDRAWSURFACE7 lpdds);

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,int color);

int Draw_Line(int x0, int y0, int x1, int y1, UCHAR color, UCHAR *vb_start, int lpitch);

int Draw_Clip_Line(int x0,int y0, int x1, int y1,UCHAR color, 
                    UCHAR *dest_buffer, int lpitch);

int Clip_Line(int &x1,int &y1,int &x2, int &y2);

int Draw_Polygon2D(POLYGON2D_PTR poly, UCHAR *vbuffer, int lpitch);

int Translate_Polygon2D(POLYGON2D_PTR poly, int dx, int dy);

int Rotate_Polygon2D(POLYGON2D_PTR poly, int theta);

int Scale_Polygon2D(POLYGON2D_PTR poly, float sx, float sy);

int Translate_Polygon2D_Mat(POLYGON2D_PTR poly, int dx, int dy);

int Rotate_Polygon2D_Mat(POLYGON2D_PTR poly, int theta);

int Scale_Polygon2D_Mat(POLYGON2D_PTR poly, float sx, float sy);

int Mat_Mul1X2_3X2(MATRIX1X2_PTR ma, 
                   MATRIX3X2_PTR mb,
                   MATRIX1X2_PTR mprod);

inline int Mat_Init_3X2(MATRIX3X2_PTR ma, 
                        float m00, float m01,
                        float m10, float m11,
                        float m20, float m21);


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


// global clipping region

int min_clip_x = 0,      // clipping rectangle 
    max_clip_x = SCREEN_WIDTH - 1,
    min_clip_y = 0,
    max_clip_y = SCREEN_HEIGHT - 1;

char buffer[80];                             // general printing buffer

// storage for our lookup tables
float cos_look[360];
float sin_look[360];

POLYGON2D ship; // the ship

// FUNCTIONS ////////////////////////////////////////////////


inline int Mat_Init_3X2(MATRIX3X2_PTR ma, 
                        float m00, float m01,
                        float m10, float m11,
                        float m20, float m21)
{
// this function fills a 3x2 matrix with the sent data in row major form
ma->M[0][0] = m00; ma->M[0][1] = m01; 
ma->M[1][0] = m10; ma->M[1][1] = m11; 
ma->M[2][0] = m20; ma->M[2][1] = m21; 

// return success
return(1);

} // end Mat_Init_3X2

/////////////////////////////////////////////////////////////////

int Mat_Mul1X2_3X2(MATRIX1X2_PTR ma, 
                   MATRIX3X2_PTR mb,
                   MATRIX1X2_PTR mprod)
{
// this function multiplies a 1x2 matrix against a 
// 3x2 matrix - ma*mb and stores the result
// using a dummy element for the 3rd element of the 1x2 
// to make the matrix multiply valid i.e. 1x3 X 3x2

    for (int col=0; col<2; col++)
        {
        // compute dot product from row of ma 
        // and column of mb

        float sum = 0; // used to hold result

        for (int index=0; index<2; index++)
             {
             // add in next product pair
             sum+=(ma->M[index]*mb->M[index][col]);
             } // end for index

        // add in last element * 1 
        sum+= mb->M[index][col];

        // insert resulting col element
        mprod->M[col] = sum;

        } // end for col

return(1);

} // end Mat_Mul_1X2_3X2

///////////////////////////////////////////////////////////////////////

int Draw_Polygon2D(POLYGON2D_PTR poly, UCHAR *vbuffer, int lpitch)
{
// this function draws a POLYGON2D based on 

// test if the polygon is visible
if (poly->state)
   {
   // loop thru and draw a line from vertices 1 to n
   for (int index=0; index < poly->num_verts-1; index++)
        {
        // draw line from ith to ith+1 vertex
        Draw_Clip_Line(poly->vlist[index].x+poly->x0, 
                       poly->vlist[index].y+poly->y0,
                       poly->vlist[index+1].x+poly->x0, 
                       poly->vlist[index+1].y+poly->y0,
                       poly->color,
                       vbuffer, lpitch);

        } // end for

       // now close up polygon
       // draw line from last vertex to 0th
       Draw_Clip_Line(poly->vlist[0].x+poly->x0, 
                      poly->vlist[0].y+poly->y0,
                      poly->vlist[index].x+poly->x0, 
                      poly->vlist[index].y+poly->y0,
                      poly->color,
                      vbuffer, lpitch);

   // return success
   return(1);
   } // end if
else 
   return(0);

} // end Draw_Polygon2D

///////////////////////////////////////////////////////////////

// the following 3 functions are the standard transforms (no matrices)

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

// test for negative rotation angle
if (theta < 0)
   theta+=360;

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

///////////////////////////////////////////////////////////////////////

// these are the matrix versions, note they are more inefficient for
// single transforms, but their power comes into play when you concatenate
// multiple transformations, not to mention that all transforms are accomplished
// with the same code, just the matrix differs

int Translate_Polygon2D_Mat(POLYGON2D_PTR poly, int dx, int dy)
{
// this function translates the center of a polygon by using a matrix multiply
// on the the center point, this is incredibly inefficient, but for educational purposes
// if we had an object that wasn't in local coordinates then it would make more sense to
// use a matrix, but since the origin of the object is at x0,y0 then 2 lines of code can
// translate, but lets do it the hard way just to see :)

// test for valid pointer
if (!poly)
   return(0);

MATRIX3X2 mt; // used to hold translation transform matrix

// initialize the matrix with translation values dx dy
Mat_Init_3X2(&mt,1,0, 0,1, dx, dy); 

// create a 1x2 matrix to do the transform
MATRIX1X2 p0 = {poly->x0, poly->y0};
MATRIX1X2 p1 = {0,0}; // this will hold result

// now translate via a matrix multiply
Mat_Mul1X2_3X2(&p0, &mt, &p1);

// now copy the result back into polygon
poly->x0 = p1.M[0];
poly->y0 = p1.M[1];

// return success
return(1);

} // end Translate_Polygon2D_Mat

///////////////////////////////////////////////////////////////

int Rotate_Polygon2D_Mat(POLYGON2D_PTR poly, int theta)
{
// this function rotates the local coordinates of the polygon

// test for valid pointer
if (!poly)
   return(0);

// test for negative rotation angle
if (theta < 0)
   theta+=360;

MATRIX3X2 mr; // used to hold rotation transform matrix

// initialize the matrix with translation values dx dy
Mat_Init_3X2(&mr,cos_look[theta],sin_look[theta], 
                 -sin_look[theta],cos_look[theta], 
                  0, 0); 

// loop and rotate each point, very crude, no lookup!!!
for (int curr_vert = 0; curr_vert < poly->num_verts; curr_vert++)
    {
    // create a 1x2 matrix to do the transform
    MATRIX1X2 p0 = {poly->vlist[curr_vert].x, poly->vlist[curr_vert].y};
    MATRIX1X2 p1 = {0,0}; // this will hold result

    // now rotate via a matrix multiply
    Mat_Mul1X2_3X2(&p0, &mr, &p1);

    // now copy the result back into vertex
    poly->vlist[curr_vert].x = p1.M[0];
    poly->vlist[curr_vert].y = p1.M[1];

    } // end for curr_vert

// return success
return(1);

} // end Rotate_Polygon2D_Mat

////////////////////////////////////////////////////////

int Scale_Polygon2D_Mat(POLYGON2D_PTR poly, float sx, float sy)
{
// this function scalesthe local coordinates of the polygon

// test for valid pointer
if (!poly)
   return(0);


MATRIX3X2 ms; // used to hold scaling transform matrix

// initialize the matrix with translation values dx dy
Mat_Init_3X2(&ms,sx,0, 
                 0,sy, 
                 0, 0); 


// loop and scale each point
for (int curr_vert = 0; curr_vert < poly->num_verts; curr_vert++)
    {
    // scale and store result back
    
    // create a 1x2 matrix to do the transform
    MATRIX1X2 p0 = {poly->vlist[curr_vert].x, poly->vlist[curr_vert].y};
    MATRIX1X2 p1 = {0,0}; // this will hold result

    // now scale via a matrix multiply
    Mat_Mul1X2_3X2(&p0, &ms, &p1);

    // now copy the result back into vertex
    poly->vlist[curr_vert].x = p1.M[0];
    poly->vlist[curr_vert].y = p1.M[1];

    } // end for curr_vert

// return success
return(1);

} // end Scale_Polygon2D_Mat


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

int Draw_Clip_Line(int x0,int y0, int x1, int y1,UCHAR color, 
                    UCHAR *dest_buffer, int lpitch)
{
// this helper function draws a clipped line

int cxs, cys,
	cxe, cye;

// clip and draw each line
cxs = x0;
cys = y0;
cxe = x1;
cye = y1;

// clip the line
if (Clip_Line(cxs,cys,cxe,cye))
	Draw_Line(cxs, cys, cxe,cye,color,dest_buffer,lpitch);

// return success
return(1);

} // end Draw_Clip_Line

///////////////////////////////////////////////////////////

int Clip_Line(int &x1,int &y1,int &x2, int &y2)
{
// this function clips the sent line using the globally defined clipping
// region

// internal clipping codes
#define CLIP_CODE_C  0x0000
#define CLIP_CODE_N  0x0008
#define CLIP_CODE_S  0x0004
#define CLIP_CODE_E  0x0002
#define CLIP_CODE_W  0x0001

#define CLIP_CODE_NE 0x000a
#define CLIP_CODE_SE 0x0006
#define CLIP_CODE_NW 0x0009 
#define CLIP_CODE_SW 0x0005

int xc1=x1, 
    yc1=y1, 
	xc2=x2, 
	yc2=y2;

int p1_code=0, 
    p2_code=0;

// determine codes for p1 and p2
if (y1 < min_clip_y)
	p1_code|=CLIP_CODE_N;
else
if (y1 > max_clip_y)
	p1_code|=CLIP_CODE_S;

if (x1 < min_clip_x)
	p1_code|=CLIP_CODE_W;
else
if (x1 > max_clip_x)
	p1_code|=CLIP_CODE_E;

if (y2 < min_clip_y)
	p2_code|=CLIP_CODE_N;
else
if (y2 > max_clip_y)
	p2_code|=CLIP_CODE_S;

if (x2 < min_clip_x)
	p2_code|=CLIP_CODE_W;
else
if (x2 > max_clip_x)
	p2_code|=CLIP_CODE_E;

// try and trivially reject
if ((p1_code & p2_code)) 
	return(0);

// test for totally visible, if so leave points untouched
if (p1_code==0 && p2_code==0)
	return(1);

// determine end clip point for p1
switch(p1_code)
	  {
	  case CLIP_CODE_C: break;

	  case CLIP_CODE_N:
		   {
		   yc1 = min_clip_y;
		   xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
		   } break;
	  case CLIP_CODE_S:
		   {
		   yc1 = max_clip_y;
		   xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);
		   } break;

	  case CLIP_CODE_W:
		   {
		   xc1 = min_clip_x;
		   yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);
		   } break;
		
	  case CLIP_CODE_E:
		   {
		   xc1 = max_clip_x;
		   yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
		   } break;

	// these cases are more complex, must compute 2 intersections
	  case CLIP_CODE_NE:
		   {
		   // north hline intersection
		   yc1 = min_clip_y;
		   xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);

		   // test if intersection is valid, of so then done, else compute next
			if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
				// east vline intersection
				xc1 = max_clip_x;
				yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
				} // end if

		   } break;
	  
	  case CLIP_CODE_SE:
      	   {
		   // south hline intersection
		   yc1 = max_clip_y;
		   xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);	

		   // test if intersection is valid, of so then done, else compute next
		   if (xc1 < min_clip_x || xc1 > max_clip_x)
		      {
			  // east vline intersection
			  xc1 = max_clip_x;
			  yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
			  } // end if

		   } break;
	    
	  case CLIP_CODE_NW: 
      	   {
		   // north hline intersection
		   yc1 = min_clip_y;
		   xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
		   
		   // test if intersection is valid, of so then done, else compute next
		   if (xc1 < min_clip_x || xc1 > max_clip_x)
		      {
			  xc1 = min_clip_x;
		      yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);	
			  } // end if

		   } break;
	  	  
	  case CLIP_CODE_SW:
		   {
		   // south hline intersection
		   yc1 = max_clip_y;
		   xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);	
		   
		   // test if intersection is valid, of so then done, else compute next
		   if (xc1 < min_clip_x || xc1 > max_clip_x)
		      {
			  xc1 = min_clip_x;
		      yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);	
			  } // end if

		   } break;

	  default:break;

	  } // end switch

// determine clip point for p2
switch(p2_code)
	  {
	  case CLIP_CODE_C: break;

	  case CLIP_CODE_N:
		   {
		   yc2 = min_clip_y;
		   xc2 = x2 + (min_clip_y-y2)*(x1-x2)/(y1-y2);
		   } break;

	  case CLIP_CODE_S:
		   {
		   yc2 = max_clip_y;
		   xc2 = x2 + (max_clip_y-y2)*(x1-x2)/(y1-y2);
		   } break;

	  case CLIP_CODE_W:
		   {
		   xc2 = min_clip_x;
		   yc2 = y2 + (min_clip_x-x2)*(y1-y2)/(x1-x2);
		   } break;
		
	  case CLIP_CODE_E:
		   {
		   xc2 = max_clip_x;
		   yc2 = y2 + (max_clip_x-x2)*(y1-y2)/(x1-x2);
		   } break;

		// these cases are more complex, must compute 2 intersections
	  case CLIP_CODE_NE:
		   {
		   // north hline intersection
		   yc2 = min_clip_y;
		   xc2 = x2 + 0.5+(min_clip_y-y2)*(x1-x2)/(y1-y2);

		   // test if intersection is valid, of so then done, else compute next
			if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
				// east vline intersection
				xc2 = max_clip_x;
				yc2 = y2 + 0.5+(max_clip_x-x2)*(y1-y2)/(x1-x2);
				} // end if

		   } break;
	  
	  case CLIP_CODE_SE:
      	   {
		   // south hline intersection
		   yc2 = max_clip_y;
		   xc2 = x2 + 0.5+(max_clip_y-y2)*(x1-x2)/(y1-y2);	

		   // test if intersection is valid, of so then done, else compute next
		   if (xc2 < min_clip_x || xc2 > max_clip_x)
		      {
			  // east vline intersection
			  xc2 = max_clip_x;
			  yc2 = y2 + 0.5+(max_clip_x-x2)*(y1-y2)/(x1-x2);
			  } // end if

		   } break;
	    
	  case CLIP_CODE_NW: 
      	   {
		   // north hline intersection
		   yc2 = min_clip_y;
		   xc2 = x2 + 0.5+(min_clip_y-y2)*(x1-x2)/(y1-y2);
		   
		   // test if intersection is valid, of so then done, else compute next
		   if (xc2 < min_clip_x || xc2 > max_clip_x)
		      {
			  xc2 = min_clip_x;
		      yc2 = y2 + 0.5+(min_clip_x-x2)*(y1-y2)/(x1-x2);	
			  } // end if

		   } break;
	  	  
	  case CLIP_CODE_SW:
		   {
		   // south hline intersection
		   yc2 = max_clip_y;
		   xc2 = x2 + 0.5+(max_clip_y-y2)*(x1-x2)/(y1-y2);	
		   
		   // test if intersection is valid, of so then done, else compute next
		   if (xc2 < min_clip_x || xc2 > max_clip_x)
		      {
			  xc2 = min_clip_x;
		      yc2 = y2 + 0.5+(min_clip_x-x2)*(y1-y2)/(x1-x2);	
			  } // end if

		   } break;
	
	  default:break;

	  } // end switch

// do bounds check
if ((xc1 < min_clip_x) || (xc1 > max_clip_x) ||
	(yc1 < min_clip_y) || (yc1 > max_clip_y) ||
	(xc2 < min_clip_x) || (xc2 > max_clip_x) ||
	(yc2 < min_clip_y) || (yc2 > max_clip_y) )
	{
	return(0);
	} // end if

// store vars back
x1 = xc1;
y1 = yc1;
x2 = xc2;
y2 = yc2;

return(1);

} // end Clip_Line

/////////////////////////////////////////////////////////////

int Draw_Line(int x0, int y0, // starting position 
              int x1, int y1, // ending position
              UCHAR color,    // color index
              UCHAR *vb_start, int lpitch) // video buffer and memory pitch
{
// this function draws a line from xo,yo to x1,y1 using differential error
// terms (based on Bresenahams work)

int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping

// pre-compute first pixel address in video buffer
vb_start = vb_start + x0 + y0*lpitch;

// compute horizontal and vertical deltas
dx = x1-x0;
dy = y1-y0;

// test which direction the line is going in i.e. slope angle
if (dx>=0)
   {
   x_inc = 1;

   } // end if line is moving right
else
   {
   x_inc = -1;
   dx    = -dx;  // need absolute value

   } // end else moving left

// test y component of slope

if (dy>=0)
   {
   y_inc = lpitch;
   } // end if line is moving down
else
   {
   y_inc = -lpitch;
   dy    = -dy;  // need absolute value

   } // end else moving up

// compute (dx,dy) * 2
dx2 = dx << 1;
dy2 = dy << 1;

// now based on which delta is greater we can draw the line
if (dx > dy)
   {
   // initialize error term
   error = dy2 - dx; 

   // draw the line
   for (index=0; index <= dx; index++)
       {
       // set the pixel
       *vb_start = color;

       // test if error has overflowed
       if (error >= 0) 
          {
          error-=dx2;

          // move to next line
          vb_start+=y_inc;

	   } // end if error overflowed

       // adjust the error term
       error+=dy2;

       // move to the next pixel
       vb_start+=x_inc;

       } // end for

   } // end if |slope| <= 1
else
   {
   // initialize error term
   error = dx2 - dy; 

   // draw the line
   for (index=0; index <= dy; index++)
       {
       // set the pixel
       *vb_start = color;

       // test if error overflowed
       if (error >= 0)
          {
          error-=dy2;

          // move to next line
          vb_start+=x_inc;

          } // end if error overflowed

       // adjust the error term
       error+=dx2;

       // move to the next pixel
       vb_start+=y_inc;

       } // end for

   } // end else |slope| > 1

// return success
return(1);

} // end Draw_Line

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

// do the graphics
Draw_Polygon2D(&ship, (UCHAR *)ddsd.lpSurface, ddsd.lPitch);

// test for scale
if (KEYDOWN('A')) // scale up
   Scale_Polygon2D_Mat(&ship, 1.1, 1.1);
else
if (KEYDOWN('S')) // scale down
   Scale_Polygon2D_Mat(&ship, 0.9, 0.9);

// test for rotate
if (KEYDOWN('Z')) // rotate left
   Rotate_Polygon2D_Mat(&ship, -5);
else
if (KEYDOWN('X')) // rotate right
   Rotate_Polygon2D_Mat(&ship, 5);

// test for translate
if (KEYDOWN(VK_RIGHT)) // translate right
   Translate_Polygon2D_Mat(&ship, 5, 0);
else
if (KEYDOWN(VK_LEFT)) // translate left
   Translate_Polygon2D_Mat(&ship, -5, 0);    

// test for translate
if (KEYDOWN(VK_UP)) // translate up
   Translate_Polygon2D_Mat(&ship, 0, -5);
else
if (KEYDOWN(VK_DOWN)) // translate left
   Translate_Polygon2D_Mat(&ship, 0, 5);    

// unlock primary buffer
if (FAILED(lpddsback->Unlock(NULL)))
   return(0);

// draw some information
sprintf(buffer,"(x,y) = [%d, %d]     ",ship.x0, ship.y0);
Draw_Text_GDI(buffer, 8,SCREEN_HEIGHT - 16,255, lpddsback);
Draw_Text_GDI("<A> and <S> - Scale    <Z> and <X> - Rotate     <Arrows> - Translate     <ESC> - Exit", 8,8,255, lpddsback);

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

// make some primary colors
palette[1].peRed     = 255;
palette[1].peGreen   = 0;
palette[1].peBlue    = 0;
palette[1].peFlags   = PC_NOCOLLAPSE;

palette[2].peRed     = 0;
palette[2].peGreen   = 255;
palette[2].peBlue    = 0;
palette[2].peFlags   = PC_NOCOLLAPSE;

palette[3].peRed     = 0;
palette[3].peGreen   = 0;
palette[3].peBlue    = 255;
palette[3].peFlags   = PC_NOCOLLAPSE;

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

// define points of ship
VERTEX2DF ship_vertices[24] = 
{1,11,
 2,8,
 1,7,
 1,-1,
 3,-1,
 3,-2,
 11,-3,
 11,-6,
 3,-7,
 2,-8,
 1,-8,
 1,-7,
 -1,-7,
 -1,-8,
 -2,-8,
 -3,-7,
 -11,-6,
 -11,-3,
 -3,-2,
 -3,-1,
 -1,-1,
 -1,7,
 -2,8,
 -1, 11};

// initialize ship
ship.state       = 1;   // turn it on
ship.num_verts   = 24;  
ship.x0          = SCREEN_WIDTH/2; // position it
ship.y0          = SCREEN_HEIGHT/2;
ship.xv          = 0;
ship.yv          = 0;
ship.color       = 2; // green
ship.vlist       = new VERTEX2DF [ship.num_verts];
 
for (int index = 0; index < ship.num_verts; index++)
    ship.vlist[index] = ship_vertices[index];


// do a quick scale on the vertices, they are a bit too small
Scale_Polygon2D(&ship, 3.0, 3.0);
   
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

// hide the cursor
ShowCursor(0);

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

// show the cursor
ShowCursor(1);

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
						    "DirectDraw 8-Bit Polygon Matrix Transformation Demo", // title
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

