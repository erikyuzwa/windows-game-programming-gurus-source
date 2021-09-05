// DEMO13_7_16b.CPP - 2D collision off irregular shaped object demo
// 16-bit version, make sure your desktop is in 16-bit mode!
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

// setup a 640x480 16-bit windowed mode example
#define WINDOW_TITLE      "16-Bit Irregular Shape Collision Demo"
#define WINDOW_WIDTH      640   // size of window
#define WINDOW_HEIGHT     480

#define WINDOW_BPP        16    // bitdepth of window (8,16,24 etc.)
                                // note: if windowed and not
                                // fullscreen then bitdepth must
                                // be same as system bitdepth
                                // also if 8-bit the a pallete
                                // is created and attached

#define WINDOWED_APP      1     // 0 not windowed, 1 windowed

#define NUM_BALLS       10   // number of pool balls
#define BALL_RADIUS     12   // radius of ball

// center position of shape
#define SHAPE_CENTER_X         320
#define SHAPE_CENTER_Y         240

// variable lookup indices
#define INDEX_X               0 
#define INDEX_Y               1  
#define INDEX_XV              2 
#define INDEX_YV              3  

// MACROS ///////////////////////////////////////////////

#define RAND_RANGE(x,y) ( (x) + (rand()%((y)-(x)+1)))
#define DOT_PRODUCT(ux,uy,vx,vy) ((ux)*(vx) + (uy)*(vy))

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle   = NULL; // save the window handle
HINSTANCE main_instance   = NULL; // save the instance
char buffer[256];                 // used to print text

BITMAP_IMAGE background_bmp;      // holds the background
BOB          balls[NUM_BALLS];    // the balls

int ball_ids[8];                  // sound ids for balls

POLYGON2D shape;                  // the shape to bounce off

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

// initialize directdraw, very important that in the call
// to setcooperativelevel that the flag DDSCL_MULTITHREADED is used
// which increases the response of directX graphics to
// take the global critical section more frequently
DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);

// load background image
Load_Bitmap_File(&bitmap8bit, "GREENGRID24.BMP");
Create_Bitmap(&background_bmp,0,0,640,480,16);
Load_Image_Bitmap16(&background_bmp, &bitmap8bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Unload_Bitmap_File(&bitmap8bit);

// load the bitmaps
Load_Bitmap_File(&bitmap8bit, "POOLBALLS24.BMP");

// create master ball
Create_BOB(&balls[0],0,0,24,24,6,BOB_ATTR_MULTI_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY,0,16);

// load the imagery in
for (index=0; index < 6; index++)
    Load_Frame_BOB16(&balls[0], &bitmap8bit, index, index,0,BITMAP_EXTRACT_MODE_CELL);

// create all the clones
for (index=1; index < NUM_BALLS; index++)
    Clone_BOB(&balls[0], &balls[index]);

// now set the initial conditions of all the balls
for (index=0; index < NUM_BALLS; index++)
    {
    // set position in center of object
    balls[index].varsF[INDEX_X] = RAND_RANGE( SHAPE_CENTER_X-50,  SHAPE_CENTER_X+50);
    balls[index].varsF[INDEX_Y] = RAND_RANGE( SHAPE_CENTER_Y-50,  SHAPE_CENTER_Y+50);

   do
    {
    // set initial velocity
    balls[index].varsF[INDEX_XV] = RAND_RANGE(-100, 100)/30;
    balls[index].varsF[INDEX_YV] = RAND_RANGE(-100, 100)/30;
    }
    while (balls[index].varsF[INDEX_XV]==0 && balls[index].varsF[INDEX_XV]==0);

    // set ball color
    balls[index].curr_frame = rand()%6;

    } // end for index

// unload bitmap image
Unload_Bitmap_File(&bitmap8bit);

// define points of shape
VERTEX2DF shape_vertices[10] =  
{ 328-SHAPE_CENTER_X,60-SHAPE_CENTER_Y,
  574-SHAPE_CENTER_X,162-SHAPE_CENTER_Y,
  493-SHAPE_CENTER_X,278-SHAPE_CENTER_Y,
  605-SHAPE_CENTER_X,384-SHAPE_CENTER_Y,
  484-SHAPE_CENTER_X,433-SHAPE_CENTER_Y,
  306-SHAPE_CENTER_X,349-SHAPE_CENTER_Y,
  150-SHAPE_CENTER_X,413-SHAPE_CENTER_Y,
  28-SHAPE_CENTER_X,326-SHAPE_CENTER_Y,
  152-SHAPE_CENTER_X,281-SHAPE_CENTER_Y,
  73-SHAPE_CENTER_X,138-SHAPE_CENTER_Y };
 

// initialize shape
shape.state       = 1;   // turn it on
shape.num_verts   = 10;  
shape.x0          = SHAPE_CENTER_X;
shape.y0          = SHAPE_CENTER_Y;
shape.xv          = 0;
shape.yv          = 0;
shape.color       = RGB16Bit(0,255,0); // green
shape.vlist       = new VERTEX2DF [shape.num_verts];
 
for (index = 0; index < shape.num_verts; index++)
    shape.vlist[index] = shape_vertices[index];

// hide the mouse
if (!WINDOWED_APP)
   ShowCursor(FALSE);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// build the 360 degree look ups
Build_Sin_Cos_Tables();

// initilize DirectSound
DSound_Init();

// load background sounds
ball_ids[0] = DSound_Load_WAV("PBALL.WAV");

// clone sounds
for (index=1; index<8; index++)
    ball_ids[index] = DSound_Replicate_Sound(ball_ids[0]);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// shut everything down

// kill all the bobs
for (int index=0; index<NUM_BALLS; index++)
    Destroy_BOB(&balls[index]);

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

//////////////////////////////////////////////////////////

void Ball_Sound(void)
{        
// this functions hunts for an open handle to play a collision sound

// start a hit sound
for (int sound_index=0; sound_index < 8; sound_index++)
    {
    // test if this sound is playing
    if (DSound_Status_Sound(ball_ids[sound_index])==0)
       {
       DSound_Play(ball_ids[sound_index]);
       break;
       } // end if

     } // end for 

} // end Ball_Sound

///////////////////////////////////////////////////////////

void Compute_Collisions(void)
{
// this function computes if any ball has hit one of the edges of the polygon
// if so the ball is bounced

float length,s,t,s1x,s1y,s2x,s2y,p0x,p0y,p1x,p1y,p2x,p2y,p3x,p3y,xi,yi,npx,npy,Nx,Ny,Fx,Fy;


for (int index = 0; index < NUM_BALLS; index++)
    {
    // first move ball
    balls[index].varsF[INDEX_X]+=balls[index].varsF[INDEX_XV];
    balls[index].varsF[INDEX_Y]+=balls[index].varsF[INDEX_YV];

    // now project velocity vector forward and test for intersection with all lines of polygon shape

    // build up vector in direction of trajectory
    p0x=balls[index].varsF[INDEX_X];
    p0y=balls[index].varsF[INDEX_Y];

#if 1 // this is the velocity vector used as segment 1
    p1x=balls[index].varsF[INDEX_X]+balls[index].varsF[INDEX_XV];
    p1y=balls[index].varsF[INDEX_Y]+balls[index].varsF[INDEX_YV];
    s1x=p1x-p0x;
    s1y=p1y-p0y;

    // normalize and scale to 1.25*radius
    length = sqrt(s1x*s1x+s1y*s1y);
    s1x = 1.25*BALL_RADIUS*s1x/length;
    s1y = 1.25*BALL_RADIUS*s1y/length; 
    p1x = p0x + s1x;
    p1y = p0y + s1y;


#endif

    // for each line try and intersect
    for (int line=0; line < shape.num_verts; line++)
    {
    // now build up vector based on line
    p2x=shape.vlist[line].x+shape.x0;
    p2y=shape.vlist[line].y+shape.y0;

    p3x=shape.vlist[(line+1)%(shape.num_verts)].x+shape.x0;
    p3y=shape.vlist[(line+1)%(shape.num_verts)].y+shape.y0;

    s2x=p3x-p2x;
    s2y=p3y-p2y;

#if 0 // this is the perp vector used as segment 1
    // normalize s2x, s2y to create a perpendicular collision vector from the ball center
    length = sqrt(s2x*s2x+s2y*s2y);
    s1x = BALL_RADIUS*s2y/length;
    s1y = -BALL_RADIUS*s2x/length; 
    p1x = p0x+s1x;
    p1y = p0y+s1y;
#endif
    // compute s and t, the parameters
    s = (-s1y*(p0x-p2x) + s1x*(p0y-p2y))/(-s2x*s1y + s1x*s2y);
    t =  (s2x*(p0y-p2y) - s2y*(p0x-p2x))/(-s2x*s1y + s1x*s2y);

    // test for valid range (0..1)
    if (s >= 0 && s <=1 && t >= 0 && t <=1)
       {
       // find collision point based on s
       xi = p0x+s*s1x;
       yi = p0y+s*s1y;

       // now we know point of intersection, reflect ball at current location

       // N = (-I . N')*N'
       // F = 2*N + I
       npx = -s2y;
       npy = s2x;

       // normalize p
       length = sqrt(npx*npx+npy*npy);
       npx/=length;
       npy/=length;

       // compute N = (-I . N')*N'
       Nx = -(balls[index].varsF[INDEX_XV]*npx + balls[index].varsF[INDEX_YV]*npy)*npx;
       Ny = -(balls[index].varsF[INDEX_XV]*npx + balls[index].varsF[INDEX_YV]*npy)*npy;

       // compute F = 2*N + I
       Fx = 2*Nx + balls[index].varsF[INDEX_XV];
       Fy = 2*Ny + balls[index].varsF[INDEX_YV];

       // update velocity with results
       balls[index].varsF[INDEX_XV] = Fx;
       balls[index].varsF[INDEX_YV] = Fy;

       balls[index].varsF[INDEX_X]+=balls[index].varsF[INDEX_XV];
       balls[index].varsF[INDEX_Y]+=balls[index].varsF[INDEX_YV];

       // make sound
       Ball_Sound();


       // break out of for line
       //break;

       } // end if
 
      } // end for line

    } // end for ball index

} // end Collision_Collisions

//////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var
static rotate = 0;
// start the timing clock
Start_Clock();

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw background
Draw_Bitmap16(&background_bmp, back_buffer, back_lpitch,0);

// draw shape
Draw_Polygon2D16(&shape, back_buffer, back_lpitch);

// have a little fun
if (++rotate > 10)
{
Rotate_Polygon2D(&shape,1);
rotate=0;
}
// unlock back surface
DDraw_Unlock_Back_Surface();

// read keyboard
DInput_Read_Keyboard();

// move the balls and compute collisions
Compute_Collisions();


// draw the balls
for (index=0; index < NUM_BALLS; index++)
    {
    balls[index].x = balls[index].varsF[INDEX_X]+0.5-BALL_RADIUS;
    balls[index].y = balls[index].varsF[INDEX_Y]+0.5-BALL_RADIUS;
    
    Draw_BOB16(&balls[index], lpddsback);
    } // end for

// draw the velocity vectors
DDraw_Lock_Back_Surface();
for (index=0; index < NUM_BALLS; index++)
    {
    Draw_Clip_Line16(balls[index].varsF[INDEX_X]+0.5, 
              balls[index].varsF[INDEX_Y]+0.5,
              balls[index].varsF[INDEX_X]+2*balls[index].varsF[INDEX_XV]+0.5,
              balls[index].varsF[INDEX_Y]+2*balls[index].varsF[INDEX_YV]+0.5,
              RGB16Bit(255,255,255), back_buffer, back_lpitch); 
    } // end for
DDraw_Unlock_Back_Surface();

// draw the title
Draw_Text_GDI("(16-Bit Version) Object to Contour Collision DEMO, Press <ESC> to Exit.",10, 10,RGB(255,255,255), lpddsback);


// flip the surfaces
DDraw_Flip();

// run collision algorithm here
Compute_Collisions();

// sync to 30 fps = 1/30sec = 33 ms
Wait_Clock(33);

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || keyboard_state[DIK_ESCAPE])
    {
    PostMessage(main_window_handle, WM_DESTROY,0,0);

    // stop all sounds
    DSound_Stop_All_Sounds();

    } // end if

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////