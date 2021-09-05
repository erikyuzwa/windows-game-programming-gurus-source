
// DEMO13_9.CPP - 2D Forward Kinematic demo
// to compile make sure to include DDRAW.LIB, DSOUND.LIB,
// DINPUT.LIB, WINMM.LIB, and of course the T3DLIB files

// INCLUDES ///////////////////////////////////////////////

#define INITGUID

#define WIN32_LEAN_AND_MEAN  

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

#include <ddraw.h>  // directX includes
#include <dsound.h>
#include <dmksctrl.h>
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>
#include <dinput.h>
#include "T3DLIB1.h" // game library includes
#include "T3DLIB2.h"
#include "T3DLIB3.h"

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINXCLASS"  // class name

#define WINDOW_WIDTH    320  // size of window
#define WINDOW_HEIGHT   240

#define ARM_ANG         2   // arm rotation angle increment

// MACROS ///////////////////////////////////////////////

#define RAND_RANGE(x,y) ( (x) + (rand()%((y)-(x)+1)))

// TYPES ///////////////////////////////////////////////


// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

void Copy_Polygon2D(POLYGON2D_PTR dest, POLYGON2D_PTR source);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle   = NULL; // save the window handle
HINSTANCE main_instance   = NULL; // save the instance
char buffer[256];                 // used to print text

BITMAP_IMAGE background_bmp;      // holds the background

POLYGON2D arm1[2], arm2[2];       // the arms of the linkage
                                  // since the transform function modify
                                  // the local coordites, I need two versions, 
                                  // so I can make a copy of the original object and always
                                  // transform from there, this is just because normally
                                  // you have a set of local coords and the transformed temp
                                  // coords, but I saved space and made them the same, so
                                  // this is a fix to make it work
                                  // hence index 0 is original and 1 will be transformed

// FUNCTIONS //////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
// this is the main message handler of the system
PAINTSTRUCT	ps;		   // used in WM_PAINT
HDC			hdc;	   // handle to a device context

// what is the message 
switch(msg)
	{	
	case WM_CREATE: 
        {
		// do initialization stuff here
		return(0);
		} break;

    case WM_PAINT:
         {
         // start painting
         hdc = BeginPaint(hwnd,&ps);

         // end painting
         EndPaint(hwnd,&ps);
         return(0);
        } break;

	case WM_DESTROY: 
		{
		// kill the application			
		PostQuitMessage(0);
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
// this is the winmain function

WNDCLASS winclass;	// this will hold the class we create
HWND	 hwnd;		// generic window handle
MSG		 msg;		// generic message
HDC      hdc;       // generic dc
PAINTSTRUCT ps;     // generic paintstruct

// first fill in the window class stucture
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

// register the window class
if (!RegisterClass(&winclass))
	return(0);

// create the window, note the use of WS_POPUP
if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, // class
						  "Forward kinematic demo",	 // title
						  WS_POPUP | WS_VISIBLE,
					 	  0,0,	   // x,y
						  WINDOW_WIDTH,  // width
                          WINDOW_HEIGHT, // height
						  NULL,	   // handle to parent 
						  NULL,	   // handle to menu
						  hinstance,// instance
						  NULL)))	// creation parms
return(0);

// save the window handle and instance in a global
main_window_handle = hwnd;
main_instance      = hinstance;

// perform all game console specific initialization
Game_Init();

// enter main event loop
while(1)
	{
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

// shutdown game and release all resources
Game_Shutdown();

// return to Windows like this
return(msg.wParam);

} // end WinMain

// T3D GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms)
{
// this function is where you do all the initialization 
// for your game

int index; // looping varsIable

char filename[80]; // used to build up filenames

// seed random number generate
srand(Start_Clock());

// start up DirectDraw (replace the parms as you desire)
DDraw_Init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

// load background image
Load_Bitmap_File(&bitmap8bit, "ARMGRID.BMP");
Create_Bitmap(&background_bmp,0,0,640,480);
Load_Image_Bitmap(&background_bmp, &bitmap8bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Set_Palette(bitmap8bit.palette);
Unload_Bitmap_File(&bitmap8bit);

// hide the mouse
ShowCursor(FALSE);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// load background sounds
// = DSound_Load_WAV("CANNON.WAV");

// define points of arm1
VERTEX2DF arm1_vertices[4] =  { -10,-10, 100,-10, 100,10, -10,10, };

// initialize arm1
arm1[0].state       = 1;   // turn it on
arm1[0].num_verts   = 4;  
arm1[0].x0          = 0; // position it
arm1[0].y0          = 0;
arm1[0].xv          = 0;
arm1[0].yv          = 0;
arm1[0].color       = 250; // green
arm1[0].vlist       = new VERTEX2DF [arm1[0].num_verts];
 
for (index = 0; index < arm1[0].num_verts; index++)
    arm1[0].vlist[index] = arm1_vertices[index];

// define points of arm2
VERTEX2DF arm2_vertices[4] =  { -10,-10, 100,-10, 100,10, -10,10, };

// initialize arm2
arm2[0].state       = 1;   // turn it on
arm2[0].num_verts   = 4;  
arm2[0].x0          = 0; // position it
arm2[0].y0          = 0;
arm2[0].xv          = 0;
arm2[0].yv          = 0;
arm2[0].color       = 250; // green
arm2[0].vlist       = new VERTEX2DF [arm2[0].num_verts];
 
for (index = 0; index < arm2[0].num_verts; index++)
    arm2[0].vlist[index] = arm2_vertices[index];

// build the 360 degree look ups
Build_Sin_Cos_Tables();

// set clipping rectangle to screen extents so objects dont
// mess up at edges
RECT screen_rect = {0,0,screen_width,screen_height};
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

// set clipping region
min_clip_x = 0;
max_clip_x = screen_width - 1;
min_clip_y = 0;
max_clip_y = screen_height - 1;

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// shut everything down

// shutdown directdraw last
DDraw_Shutdown();

// now directsound
DSound_Stop_All_Sounds();
DSound_Shutdown();

// shut down directinput
DInput_Shutdown();

// return success
return(1);

} // end Game_Shutdown

//////////////////////////////////////////////////////////////

void Copy_Polygon2D(POLYGON2D_PTR dest, POLYGON2D_PTR source)
{
// copies one poly to another
dest->state     = source->state;     
dest->num_verts = source->num_verts  ;
dest->x0    = source->x0;        
dest->y0    = source->y0;       
dest->xv    = source->xv;         
dest->yv    = source->yv;         
dest->color = source->color;

// now copy vertex list
if (dest->vlist)
   free (dest->vlist);

// allocate memory for new list
dest->vlist   = new VERTEX2DF [dest->num_verts];

// now copy the vertex list
memcpy(dest->vlist, source->vlist, sizeof(VERTEX2DF)*dest->num_verts);

} // end Copy_Polygon2D

//////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////

int Translate_Polygon2D_Verts_Mat(POLYGON2D_PTR poly, float dx, float dy)
{
// this function translates the local coordinates of the polygon at the vertex
// level rather than the center point

// test for valid pointer
if (!poly)
   return(0);


MATRIX3X2 mt; // used to hold translation transform matrix

// initialize the matrix with translation values dx dy
Mat_Init_3X2(&mt,1,0, 0,1, dx, dy); 

// loop and translate each point
for (int curr_vert = 0; curr_vert < poly->num_verts; curr_vert++)
    {
    // scale and store result back
    
    // create a 1x2 matrix to do the transform
    MATRIX1X2 p0 = {poly->vlist[curr_vert].x, poly->vlist[curr_vert].y};
    MATRIX1X2 p1 = {0,0}; // this will hold result

    // now translate via a matrix multiply
    Mat_Mul_1X2_3X2(&p0, &mt, &p1);

    // now copy the result back into vertex
    poly->vlist[curr_vert].x = p1.M[0];
    poly->vlist[curr_vert].y = p1.M[1];

    } // end for curr_vert

// return success
return(1);

} // end Translate_Polygon2D_Verts_Mat

//////////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

static int   curr_angle1 = -90, 
             curr_angle2 = 45 ; // current link angles

// start the timing clock
Start_Clock();

// clear the drawing surface
//DDraw_Fill_Surface(lpddsback, 0);

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw background
Draw_Bitmap(&background_bmp, back_buffer, back_lpitch,0);

// unlock back surface
DDraw_Unlock_Back_Surface();

// read keyboard
DInput_Read_Keyboard();

// test for arm1 rotation
if (keyboard_state[DIK_A])  
   { 
   curr_angle1-=ARM_ANG;
   } // end if
else
if (keyboard_state[DIK_S]) 
   { 
   curr_angle1+=ARM_ANG;
   } // end if


// test for arm2 rotation
if (keyboard_state[DIK_D])  
   { 
   curr_angle2-=ARM_ANG;
   } // end if
else
if (keyboard_state[DIK_F]) 
   { 
   curr_angle2+=ARM_ANG;
   } // end if


// test for overflow
if (curr_angle1 >= 360)
   curr_angle1-=360;
else
if (curr_angle1 < 0)
   curr_angle1+=360;

if (curr_angle2 >= 360)
   curr_angle2-=360;
else
if (curr_angle2 < 0)
   curr_angle2+=360;


// first arm1

// first copy arms to transform polygon
Copy_Polygon2D(&arm1[1], &arm1[0]);

// rotate
Rotate_Polygon2D_Mat(&arm1[1], curr_angle1);

// now translate
Translate_Polygon2D_Verts_Mat(&arm1[1], 324, 400);

// now arm2

// first copy arms to transform polygon
Copy_Polygon2D(&arm2[1], &arm2[0]);

// rotate
Rotate_Polygon2D_Mat(&arm2[1], curr_angle2);

// and rotate around link 1
Rotate_Polygon2D_Mat(&arm2[1], curr_angle1);

// now translate to link 1
Translate_Polygon2D_Verts_Mat(&arm2[1], (arm1[1].vlist[1].x+arm1[1].vlist[2].x)/2, 
                                        (arm1[1].vlist[1].y+arm1[1].vlist[2].y)/2 );

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw arms
Draw_Polygon2D(&arm1[1], back_buffer, back_lpitch);
Draw_Polygon2D(&arm2[1], back_buffer, back_lpitch);

// unlock back surface
DDraw_Unlock_Back_Surface();

// draw the title
Draw_Text_GDI("Forward Kinematic DEMO, Press <ESC> to Exit.",10, 10,RGB(255,255,255), lpddsback);
Draw_Text_GDI("<A>, <S> to adjust Arm 1, <D>, <F> to adjust Arm 2",10, 25, RGB(255,255,255), lpddsback);

sprintf(buffer, "Arm 1 Angle=%d, Arm 2 Angle=%d", 360-curr_angle1, 360-curr_angle2);
Draw_Text_GDI(buffer,10, 60, RGB(255,255,255), lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30 fps = 1/30sec = 33 ms
Wait_Clock(33);

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || keyboard_state[DIK_ESCAPE])
    {
    PostMessage(main_window_handle, WM_DESTROY,0,0);

    // stop all sounds
    DSound_Stop_All_Sounds();

    // do a screen transition
    Screen_Transitions(SCREEN_DARKNESS,NULL,0);
    } // end if

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////