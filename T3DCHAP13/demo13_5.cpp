// DEMO13_5.CPP - simple friction demo
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

#define WINDOW_WIDTH    320            // size of window
#define WINDOW_HEIGHT   240

#define FRICTION_FACTOR     (float)(0.1)  // friction of table
#define PUCK_START_X        30            // starting location of puck
#define PUCK_START_Y        220
#define PUCK_STATE_RESTING  0             // puck is sitting
#define PUCK_STATE_MOVING   1             // puck has been hit

// extents of table
#define TABLE_MAX_X     620
#define TABLE_MIN_X     14
#define TABLE_MAX_Y     370
#define TABLE_MIN_Y     80

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle   = NULL; // save the window handle
HINSTANCE main_instance   = NULL; // save the instance
char buffer[80];                          // used to print text

BITMAP_IMAGE background_bmp;   // holds the background
BOB          puck;             // the ship

int sound_id = -1;             // general sound

float friction = FRICTION_FACTOR; // frictional coefficient

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
						  "Simple Friction Demo",	 // title
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

// WINX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

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
Load_Bitmap_File(&bitmap8bit, "HOCKEY8.BMP");
Create_Bitmap(&background_bmp,0,0,640,480);
Load_Image_Bitmap(&background_bmp, &bitmap8bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Set_Palette(bitmap8bit.palette);
Unload_Bitmap_File(&bitmap8bit);

// load the bitmaps for ship
Load_Bitmap_File(&bitmap8bit, "PUCK8.BMP");

// create bob
Create_BOB(&puck,PUCK_START_X,PUCK_START_Y,32,32,1,BOB_ATTR_SINGLE_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY);

// set state of puck
puck.anim_state = PUCK_STATE_RESTING;

// use varsF[0,1] for the x and y velocity
puck.varsF[0] = 0;
puck.varsF[1] = 0;

// use varsF[2,3] for the x and y position, we need more accuracy than ints
puck.varsF[2] = puck.x;
puck.varsF[3] = puck.y;

// load the frame
Load_Frame_BOB(&puck, &bitmap8bit, 0, 0,0,BITMAP_EXTRACT_MODE_CELL);

// unload bitmap image
Unload_Bitmap_File(&bitmap8bit);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// load background sounds
sound_id = DSound_Load_WAV("PUCK.WAV");

// set clipping rectangle to screen extents so objects dont
// mess up at edges
RECT screen_rect = {0,0,screen_width,screen_height};
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

// hide the mouse
ShowCursor(FALSE);

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
Destroy_BOB(&puck);

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

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw background
Draw_Bitmap(&background_bmp, back_buffer, back_lpitch,0);

// unlock back surface
DDraw_Unlock_Back_Surface();

// read keyboard
DInput_Read_Keyboard();

// is player changing friction
if (keyboard_state[DIK_RIGHT])
   friction+=0.005;
else
if (keyboard_state[DIK_LEFT])
   if ((friction-=0.005) < 0)
       friction = 0.0;

// check if player is hiting puck
if (keyboard_state[DIK_SPACE] && puck.anim_state == PUCK_STATE_RESTING)
    {
    // first set state to motion
    puck.anim_state = PUCK_STATE_MOVING;    

    // reset puck position
    puck.varsF[2] = PUCK_START_X;
    puck.varsF[3] = PUCK_START_Y;

    // select random initial trajectory
    puck.varsF[0] = 12+rand()%8;
    puck.varsF[1] = -8 + rand()%17;

    } // end if

// move puck
puck.varsF[2]+=puck.varsF[0];
puck.varsF[3]+=puck.varsF[1];

///////////////////////////////////////////////////////////

// apply friction in direction opposite current trajectory
float fx = -puck.varsF[0];
float fy = -puck.varsF[1];
float length_f = sqrt(fx*fx+fy*fy); // normally we would avoid square root at all costs!

// compute the frictional resitance

if (fabs(length_f) >= 0.50)
    { 
    fx = friction*fx/length_f;
    fy = friction*fy/length_f;
    } // end if
else
   {
   // force 0.0
   fx=fy=0.0;

   // kill velocity
   puck.varsF[0]=0.0;
   puck.varsF[1]=0.0;
   
   // puck is stuck, let player fire again
   puck.anim_state = PUCK_STATE_RESTING;  
   } // end if

// now apply friction to forward velocity
puck.varsF[0]+=fx;
puck.varsF[1]+=fy;

// test if puck is off screen
if (puck.varsF[2]+puck.width >= TABLE_MAX_X || puck.varsF[2] <= TABLE_MIN_X)
    {
    // invert velocity
    puck.varsF[0] = -puck.varsF[0];

    // move puck
    puck.varsF[2]+=puck.varsF[0];
    puck.varsF[3]+=puck.varsF[1];

    DSound_Play(sound_id, 0);
    } // end if

if (puck.varsF[3]+puck.height >= TABLE_MAX_Y || puck.varsF[3] <= TABLE_MIN_Y)
   {
   // invert velocity
   puck.varsF[1] = -puck.varsF[1];

   // move puck
   puck.varsF[2]+=puck.varsF[0];
   puck.varsF[3]+=puck.varsF[1];

   DSound_Play(sound_id, 0);

   } // end if

// copy floating point position to bob x,y
puck.x = puck.varsF[2]+0.5;
puck.y = puck.varsF[3]+0.5;

// draw skid marks to table if puck is in motion
if (fabs(puck.varsF[0]) > 0 || fabs(puck.varsF[1]) > 0)
   {
   Draw_Pixel(puck.x+puck.width/2-2+rand()%4,puck.y+puck.height/2-2+rand()%4, 16+rand()%8,
               background_bmp.buffer, background_bmp.width);    
   } // end if

// draw the puck
Draw_BOB(&puck,lpddsback);

// draw the title
Draw_Text_GDI("SIMPLE FRICTION DEMO - Hit Space to Shoot Puck, Arrows to Change Friction.",10,10,RGB(0,255,255), lpddsback);

sprintf(buffer,"Friction: X=%f, Y=%f",fx, fy);
Draw_Text_GDI(buffer,10,410,RGB(0,255,0), lpddsback);

sprintf(buffer,"Velocity: X=%f, Y=%f",puck.varsF[0], puck.varsF[1]);
Draw_Text_GDI(buffer,10,430,RGB(0,255,0), lpddsback);

sprintf(buffer,"Frictional Coefficient: %f",friction);
Draw_Text_GDI(buffer,10,450,RGB(0,255,0), lpddsback);

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