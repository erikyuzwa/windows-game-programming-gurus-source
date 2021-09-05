// DEMO13_3.CPP - two mass gravity simulation
// note: there is currently no upper limit on your velocity
// since this is more realistic, so be careful not to get 
// out of control

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

#define FRICTION_FACTOR  (float)(0.05)  // friction of the virtual space

// these are the gravity constants, they are selected to simply work
#define VIRTUAL_GRAVITY_CONSTANT     (float)0.01
#define SHIP_MASS                    (float)2
#define BLACK_HOLE_MASS              (float)50000
#define MAX_VEL                      (float)30

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
BOB          ship;             // the ship
BOB          black_hole;       // the gravity well

int sound_id = -1;             // general sound

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
						  "Space Gravity Demo",	 // title
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
Load_Bitmap_File(&bitmap8bit, "GRAVSKY8.BMP");
Create_Bitmap(&background_bmp,0,0,640,480);
Load_Image_Bitmap(&background_bmp, &bitmap8bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Set_Palette(bitmap8bit.palette);
Unload_Bitmap_File(&bitmap8bit);

// load the bitmaps for ship
Load_Bitmap_File(&bitmap8bit, "BLAZE8.BMP");

// create bob
Create_BOB(&ship,320,240,22,18,32,BOB_ATTR_MULTI_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY);

// well use varsI[0] to hold the direction 0..15, 0-360 degrees, clockwise
ship.varsI[0] = 0; // along +x axis to start

// use varsF[0,1] for the x and y velocity
ship.varsF[0] = 0;
ship.varsF[1] = 0;

// use varsF[2,3] for the x and y position, we need more accuracy than ints
ship.varsF[2] = ship.x;
ship.varsF[3] = ship.y;

// load the frames in
for (index=0; index < 32; index++)
    Load_Frame_BOB(&ship, &bitmap8bit, index, index%16,index/16,BITMAP_EXTRACT_MODE_CELL);

// unload bitmap image
Unload_Bitmap_File(&bitmap8bit);

// load the bitmaps for blackhole
Load_Bitmap_File(&bitmap8bit, "PHOTON8.BMP");

// create bob
Create_BOB(&black_hole,32+rand()%(SCREEN_WIDTH-64),32+rand()%(SCREEN_HEIGHT-64),
            44,44,7,BOB_ATTR_MULTI_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY);

// set animation speed
Set_Anim_Speed_BOB(&black_hole,3);

// load the frames in
for (index=0; index < 7; index++)
    Load_Frame_BOB(&black_hole, &bitmap8bit, index, index,0,BITMAP_EXTRACT_MODE_CELL);

// unload bitmap image
Unload_Bitmap_File(&bitmap8bit);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// load background sounds
sound_id = DSound_Load_WAV("BHOLE.WAV");

// start the sounds
DSound_Play(sound_id, DSBPLAY_LOOPING);

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
Destroy_BOB(&ship);
Destroy_BOB(&black_hole);

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


// check the player controls

// is the player turning right or left?
if (keyboard_state[DIK_RIGHT])
   {
    // there are 16 possible positions for the ship to point in
    if (++ship.varsI[0] >= 16)
        ship.varsI[0] = 0;
   } // end if
else
if (keyboard_state[DIK_LEFT])
   {
    // there are 16 possible positions for the ship to point in
    if (--ship.varsI[0] < 0)
        ship.varsI[0] = 15;
   } // end if

// now test for forward thrust
if (keyboard_state[DIK_UP])
    {
    // thrust ship in current direction
    
    float rad_angle = (float)ship.varsI[0]*(float)3.14159/(float)8;
    float xv = cos(rad_angle);
    float yv = sin(rad_angle);

    ship.varsF[0]+=xv;
    ship.varsF[1]+=yv;

    // animate the ship
    ship.curr_frame = ship.varsI[0]+16*(rand()%2);

    } // end if
else // show non thrust version
   ship.curr_frame = ship.varsI[0];  

// move ship
ship.varsF[2]+=ship.varsF[0];
ship.varsF[3]+=ship.varsF[1];

// always apply friction in direction opposite current trajectory
float fx = -ship.varsF[0];
float fy = -ship.varsF[1];
float length_f = sqrt(fx*fx+fy*fy); // normally we would avoid square root at all costs!

// compute the frictional resitance

if (fabs(length_f) > 0.1)
    { 
    fx = FRICTION_FACTOR*fx/length_f;
    fy = FRICTION_FACTOR*fy/length_f;
    } // end if
else
    fx=fy=0;

// now apply friction to forward velocity
ship.varsF[0]+=fx;
ship.varsF[1]+=fy;

////////////////////////////////////////////////////////////////////

// gravity calculation section

// step 1: compute vector from black hole to ship, note that the centers
// of each object are used
float grav_x = (black_hole.x + black_hole.width/2)  - (ship.x + ship.width/2);
float grav_y = (black_hole.y + black_hole.height/2) - (ship.y + ship.height/2);
float radius_squared = grav_x*grav_x + grav_y*grav_y; // equal to radius squared
float length_grav = sqrt(radius_squared);

// step 2: normalize the length of the vector to 1.0
grav_x = grav_x/length_grav;
grav_y = grav_y/length_grav;

// step 3: compute the gravity force
float grav_force = (VIRTUAL_GRAVITY_CONSTANT) * (SHIP_MASS * BLACK_HOLE_MASS) / radius_squared;

// step 4: apply gforce in the direction of grav_x, grav_y with the magnitude of grav_force
ship.varsF[0]+=grav_x*grav_force;
ship.varsF[1]+=grav_y*grav_force;

////////////////////////////////////////////////////////////////////

// test if ship is off screen
if (ship.varsF[2] > SCREEN_WIDTH)
   ship.varsF[2] = -ship.width;
else
if (ship.varsF[2] < -ship.width)
   ship.varsF[2] = SCREEN_WIDTH;

if (ship.varsF[3] > SCREEN_HEIGHT)
   ship.varsF[3] = -ship.height;
else
if (ship.varsF[3] < -ship.height)
   ship.varsF[3] = SCREEN_HEIGHT;

// test if velocity is insane
if ( (ship.varsF[0]*ship.varsF[0] + ship.varsF[1]*ship.varsF[1]) > MAX_VEL)
   {
   // scale velocity down
   ship.varsF[0]*=.95;
   ship.varsF[1]*=.95;

   } // end if

// animate the black hole
Animate_BOB(&black_hole);

// draw the black hole
Draw_BOB(&black_hole, lpddsback);

// copy floating point position to bob x,y
ship.x = ship.varsF[2];
ship.y = ship.varsF[3];

// draw the ship
Draw_BOB(&ship,lpddsback);

// draw the title
Draw_Text_GDI("GRAVITY MASS DEMO - Use Arrows to Control Ship.",10, 10,RGB(0,255,255), lpddsback);

sprintf(buffer,"Friction: X=%f, Y=%f",fx, fy);
Draw_Text_GDI(buffer,10,420,RGB(0,255,0), lpddsback);

sprintf(buffer,"Velocity: X=%f, Y=%f",ship.varsF[0], ship.varsF[1]);
Draw_Text_GDI(buffer,10,440,RGB(0,255,0), lpddsback);

sprintf(buffer,"Gravity: X=%f, Y=%f",ship.varsF[2], ship.varsF[3]);
Draw_Text_GDI(buffer,10,460,RGB(0,255,0), lpddsback);

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