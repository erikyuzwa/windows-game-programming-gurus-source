// DEMO13_4_16b.CPP - 2D trajectory demo
// 16-bit version
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
#define WINDOW_TITLE      "16-Bit Trajectory Demo"
#define WINDOW_WIDTH      640   // size of window
#define WINDOW_HEIGHT     480

#define WINDOW_BPP        16    // bitdepth of window (8,16,24 etc.)
                                // note: if windowed and not
                                // fullscreen then bitdepth must
                                // be same as system bitdepth
                                // also if 8-bit the a pallete
                                // is created and attached

#define WINDOWED_APP      1     // 0 not windowed, 1 windowed

// defines for polygon cannon
#define CANNON_X0  39       // position of tip of cannon
#define CANNON_Y0  372
#define NUM_PROJECTILES 16  // number of projectiles

// MACROS ///////////////////////////////////////////////

#define RAND_RANGE(x,y) ( (x) + (rand()%((y)-(x)+1)))

// TYPES ///////////////////////////////////////////////

typedef struct PROJ_TYP
{
int state;      // state 0 off, 1 on
float x,y;      // position
float xv, yv;   // velocity

} PROJECTILE, *PROJECTILE_PTR;


// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// missile interface
void Init_Projectiles(void);
void Move_Projectiles(void);
void Draw_Projectiles(void);
void Fire_Projectile(int angle, float vel);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle   = NULL; // save the window handle
HINSTANCE main_instance   = NULL; // save the instance
char buffer[256];                 // used to print text

BITMAP_IMAGE background_bmp;      // holds the background

int cannon_ids[8]; // sound ids

POLYGON2D cannon; // the ship

PROJECTILE missiles[NUM_PROJECTILES]; // array of missiles

float gravity_force =  0.2;  // gravity
float wind_force    = -0.01; // wind resistance

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
Load_Bitmap_File(&bitmap16bit, "TANKGRID24.BMP");
Create_Bitmap(&background_bmp,0,0,640,480,16);
Load_Image_Bitmap16(&background_bmp, &bitmap16bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Unload_Bitmap_File(&bitmap16bit);

// hide the mouse
if (!WINDOWED_APP)
   ShowCursor(FALSE);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// load background sounds
cannon_ids[0] = DSound_Load_WAV("CANNON.WAV");

// clone sounds
for (index=1; index < 8; index++)
    cannon_ids[index] = DSound_Replicate_Sound(cannon_ids[0]);

// define points of cannon
VERTEX2DF cannon_vertices[4] =  { 0,-2, 30,0, 30,2, 0,2, };

// initialize ship
cannon.state       = 1;   // turn it on
cannon.num_verts   = 4;  
cannon.x0          = CANNON_X0; // position it
cannon.y0          = CANNON_Y0;
cannon.xv          = 0;
cannon.yv          = 0;
cannon.color       = RGB16Bit(0,255,0); // green
cannon.vlist       = new VERTEX2DF [cannon.num_verts];
 
for (index = 0; index < cannon.num_verts; index++)
    cannon.vlist[index] = cannon_vertices[index];

// build the 360 degree look ups
Build_Sin_Cos_Tables();

// initialize the missiles
Init_Projectiles();

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

//////////////////////////////////////////////////////////

void Cannon_Sound(void)
{        
// this functions hunts for an open handle to play a cannon sound

// start a hit sound
for (int sound_index=0; sound_index < 8; sound_index++)
    {
    // test if this sound is playing
    if (DSound_Status_Sound(cannon_ids[sound_index])==0)
       {
       DSound_Play(cannon_ids[sound_index]);
       break;
       } // end if

     } // end for 

} // end Cannon_Sound

///////////////////////////////////////////////////////////

void Init_Projectiles(void)
{
// this function initializes the projectiles
memset(missiles, 0, sizeof(PROJECTILE)*NUM_PROJECTILES);

} // end Init_Projectiles

/////////////////////////////////////////////////////////////

void Move_Projectiles(void)
{
// this function moves all the projectiles and does the physics model
for (int index=0; index<NUM_PROJECTILES; index++)
    {
    if (missiles[index].state==1)
       {
        // translate
        missiles[index].x+=missiles[index].xv;
        missiles[index].y+=missiles[index].yv;

        // apply forces
        missiles[index].xv+=wind_force;
        missiles[index].yv+=gravity_force;

        // test for off screen
        if (missiles[index].x >= screen_width || 
            missiles[index].y >=screen_height || 
            missiles[index].y < 0) 
           {
           // kill the missile
           missiles[index].state = 0;
           } // end if

        } // end if on
 
    } // end for index

} // end Move_Projectiles

/////////////////////////////////////////////////////////////

void Draw_Projectiles(void)
{
// this function draws all the projectiles 
for (int index=0; index < NUM_PROJECTILES; index++)
    {
    // is this one alive?
    if (missiles[index].state==1)
       {
       Draw_Rectangle(missiles[index].x-1,missiles[index].y-1, 
                      missiles[index].x+1,missiles[index].y+1,
                      RGB16Bit(0,255,0), lpddsback);
       } // end if

    } // end for index

} // end Draw_Projectiles

/////////////////////////////////////////////////////////////

void Fire_Projectile(int angle, float vel)
{
// this function starts a projectile with the given angle and velocity 
// at the tip of the cannon
for (int index=0; index < NUM_PROJECTILES; index++)
    {
    // find an open projectile
    if (missiles[index].state==0)
       {
       // set this missile in motion at the head of cannon with the proper angle
       missiles[index].x = cannon.vlist[1].x+cannon.x0;
       missiles[index].y = cannon.vlist[1].y+cannon.y0;

       // compute velocity vector based on angle
       missiles[index].xv = vel*cos_look[angle];
       missiles[index].yv = -vel*sin_look[angle];

       // mark as active
       missiles[index].state = 1;

       // make sound
       Cannon_Sound();

       // bail
       break;

       } // end if

    } // end for index

} // end Fire_Projectile

////////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

static int   curr_angle = 0; // current angle of elevation from horizon
static float curr_vel   = 10; // current velocity of projectile

// start the timing clock
Start_Clock();

// clear the drawing surface
//DDraw_Fill_Surface(lpddsback, 0);

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw background
Draw_Bitmap16(&background_bmp, back_buffer, back_lpitch,0);

// do the graphics
Draw_Polygon2D16(&cannon, back_buffer, back_lpitch);

// unlock back surface
DDraw_Unlock_Back_Surface();

// read keyboard
DInput_Read_Keyboard();

// test for rotate
if ((curr_angle < 90) && keyboard_state[DIK_UP]) // rotate left
   {
   Rotate_Polygon2D_Mat(&cannon, -5);
   curr_angle+=5;
   } // end if
else
if ((curr_angle > 0) &&keyboard_state[DIK_DOWN]) // rotate right
   {
   Rotate_Polygon2D_Mat(&cannon, 5);
   curr_angle-=5;
   } // end if

// test for projectile velocity
if (keyboard_state[DIK_RIGHT])  
   { 
   if (curr_vel < 30) curr_vel+=0.1;
   } // end if
else
if (keyboard_state[DIK_LEFT]) 
   { 
   if (curr_vel > 0) curr_vel-=0.1;
   } // end if

// test for wind force
if (keyboard_state[DIK_W])  
   { 
   if (wind_force < 2) wind_force+=0.01;
   } // end if
else
if (keyboard_state[DIK_E]) 
   { 
   if (wind_force > -2) wind_force-=0.01;
   } // end if

// test for gravity force
if (keyboard_state[DIK_G])  
   { 
   if (gravity_force < 15) gravity_force+=0.1;
   } // end if
else
if (keyboard_state[DIK_B]) 
   { 
   if (gravity_force > -15) gravity_force-=0.1;
   } // end if

// test for fire!
if (keyboard_state[DIK_LCONTROL]) 
   { 
   Fire_Projectile(curr_angle, curr_vel);

   } // end fire

// move all the projectiles
Move_Projectiles();

// draw the projectiles
Draw_Projectiles();

// draw the title
Draw_Text_GDI("(16-Bit Version) Trajectory DEMO, Press <ESC> to Exit.",10, 10,RGB(255,255,255), lpddsback);
Draw_Text_GDI("<RIGHT>, <LEFT> to adjust velocity, <UP>, <DOWN> to adjust angle",10, 25, RGB(255,255,255), lpddsback);
Draw_Text_GDI("<G>, <B> to adjust gravity, <W>, <E> to adjust wind, <CTRL> to fire.",10, 40, RGB(255,255,255), lpddsback);

sprintf(buffer, "Ang=%d, Vel=%f", curr_angle, curr_vel);
Draw_Text_GDI(buffer,10, 60, RGB(255,255,255), lpddsback);

sprintf(buffer, "Wind force=%f, Gravity Force=%f", wind_force, gravity_force);
Draw_Text_GDI(buffer,10, 75, RGB(255,255,255), lpddsback);

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

    } // end if

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////