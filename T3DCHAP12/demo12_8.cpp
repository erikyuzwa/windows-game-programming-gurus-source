// DEMO12_8.CPP - path finding racing demo
// to compile make sure to include DDRAW.LIB, DSOUND.LIB,
// DINPUT.LIB, WINMM.LIB, and of course 
// T3DLIB1.CPP,T3DLIB2.CPP,T3DLIB3.CPP,

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
#define WINDOW_CLASS_NAME "WINCLASS"  // class name

#define WINDOW_WIDTH    320   // size of window
#define WINDOW_HEIGHT   240

#define NUM_WAYPOINTS 30 // number of waypoints in path

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// TYPES /////////////////////////////////////////////////

typedef struct WAYPOINT_TYP
        {
        float x,y;

        } WAYPOINT, *WAYPOINT_PTR;


// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle           = NULL; // save the window handle
HINSTANCE main_instance           = NULL; // save the instance
char buffer[256];                          // used to print text

BITMAP_IMAGE background_bmp;   // holds the background

BOB          car; // this little race car

int wind_sound_id = -1;        // the ambient wind
int car_sound_id = -1;         // the engine of the car

int vector_display_on = 1;     // used to toggle the informational vector rendering

// the way point list hand compiled :)
WAYPOINT path[NUM_WAYPOINTS] = { 

{332,122}, 
{229,108}, 
{155,97}, 
{104,100}, 
{67,119}, 
{46,159}, 
{55,229}, 
{74,283}, 
{132,364}, 
{206,407}, 
{268,412}, 
{291,405}, 
{303,379}, 
{312,274}, 
{336,244}, 
{383,233}, 
{417,240}, 
{434,278}, 
{426,328}, 
{407,388}, 
{418,415}, 
{452,429}, 
{501,419}, 
{534,376}, 
{562,263}, 
{562,188}, 
{556,112}, 
{530,100}, 
{484,97},
{404,116},};                     


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
						  "Waypoint Path finding demo",	 // title
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

///////////////////////////////////////////////////////////

int Find_Nearest_Waypoint(float x, float y)
{
// this function finds the nearest waypoint to the sent position

int near_id = 0;
int near_dist = 1000,
    test_dist;

for (int index=0; index<NUM_WAYPOINTS; index++)
    {
    // is this waypoint closer?
    if ((test_dist = Fast_Distance_2D(path[index].x - x,path[index].y - y)) < near_dist)
       {
       // set as nearest waypoint
       near_id = index;
       near_dist = test_dist;   
       } // end if

    } // end for index

// test if user want to see all those lines
if (vector_display_on==1)
   {
   // draw it
   DDraw_Lock_Back_Surface();
   Draw_Line(path[near_id].x, path[near_id].y - 8,path[near_id].x, path[near_id].y + 8,
              250, back_buffer, back_lpitch); 

   Draw_Line(path[near_id].x-8, path[near_id].y ,path[near_id].x+8, path[near_id].y,
              250, back_buffer, back_lpitch); 
   DDraw_Unlock_Back_Surface();
   } // end if

// return it
return(near_id);

} // end Find_Nearest_Waypoint

////////////////////////////////////////////////////////////

void Draw_Waypoints(int mode=1)
{
// this function draws the waypoints and the path network

// lock back surface
DDraw_Lock_Back_Surface();

for (int index=0; index < NUM_WAYPOINTS; index++)
    {
    // draw network line too?
    if (mode > 0)
       Draw_Line(path[index].x, path[index].y,
                 path[(index+1)%NUM_WAYPOINTS].x, path[(index+1)%NUM_WAYPOINTS].y,
                 249,   
                 back_buffer, back_lpitch); // video buffer and memory pitch

   Draw_Pixel(path[index].x, path[index].y, 250, back_buffer, back_lpitch);
   Draw_Pixel(path[index].x+1, path[index].y, 250, back_buffer, back_lpitch);
   Draw_Pixel(path[index].x, path[index].y+1, 250, back_buffer, back_lpitch);
   Draw_Pixel(path[index].x+1, path[index].y+1, 250, back_buffer, back_lpitch);

    } // end for index

// lock back surface
DDraw_Unlock_Back_Surface();

} // end Draw_Waypoints

// T3D GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms)
{
// this function is where you do all the initialization 
// for your game

int index; // looping variable

// start up DirectDraw (replace the parms as you desire)
DDraw_Init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

// load background image
Load_Bitmap_File(&bitmap8bit, "RACETRACK.BMP");
Create_Bitmap(&background_bmp,0,0,640,480);
Load_Image_Bitmap(&background_bmp, &bitmap8bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Set_Palette(bitmap8bit.palette);
Unload_Bitmap_File(&bitmap8bit);

// load the car bitmaps
Load_Bitmap_File(&bitmap8bit, "SIMIMG.BMP");

// create car bob
Create_BOB(&car,334,64, 16,16, 16, BOB_ATTR_MULTI_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY);

// set floating position and direction to west
car.varsF[0] = car.x;
car.varsF[1] = car.y;
car.curr_frame = 12;

// load the car in 
for (index=0; index < 16; index++)
    Load_Frame_BOB(&car, &bitmap8bit, index, index, 3, BITMAP_EXTRACT_MODE_CELL);

// unload car
Unload_Bitmap_File(&bitmap8bit);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// load background sounds
wind_sound_id = DSound_Load_WAV("WIND.WAV");
car_sound_id = DSound_Load_WAV("CARIDLE.WAV");

// start the sounds
DSound_Play(wind_sound_id, DSBPLAY_LOOPING);
DSound_Play(car_sound_id, DSBPLAY_LOOPING);

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
Destroy_BOB(&car);

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

/////////////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

static float speed = 0; // speed of car

static int curr_waypoint = -1,
           test_waypoint;

static int display_debounce = 0; // used to debounce keyboard

int ai_on = 1; // used to enable display ai based on user taking controls

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

// draw the waypoints
if (vector_display_on==1)
   Draw_Waypoints();

// read keyboard
DInput_Read_Keyboard();

// is user toggling vector display
if (keyboard_state[DIK_D] && !display_debounce)
   {
   display_debounce = 1;
   vector_display_on=-vector_display_on;   
   }  // end if

if (!keyboard_state[DIK_D])
    display_debounce = 0;

// these controls not really needed since AI drives...

// allow player to move
if (keyboard_state[DIK_RIGHT])
   {
   if (++car.curr_frame > 15)
      car.curr_frame = 0;

   // turn off ai this cycle
   ai_on = 0;
   } // end if
else
if (keyboard_state[DIK_LEFT])
   {
   if (--car.curr_frame < 0)
      car.curr_frame = 15;

   // turn off ai this cycle
   ai_on = 0;
   } // end if

if (keyboard_state[DIK_UP])
   {
   if ((speed+=0.1) > 4)
      speed = 4.0;

   // turn off ai this cycle
   ai_on = 0;
   } // end if
else
if (keyboard_state[DIK_DOWN])
   {
   speed-=0.1;

   // turn off ai this cycle
   ai_on = 0;
   } // end if

// ai speed control -- pretty dumb huh :)
// better to tie it to angle that it's turning
if (ai_on)
   if ((speed+=0.05) > 4)
      speed = 4.0;

// apply friction
if ((speed-=0.01) < 0)
      speed = 0;

// make engine sound
DSound_Set_Freq(car_sound_id,11000+speed*2000);

// let ai turn the car

// find nearest waypoint first
test_waypoint = (Find_Nearest_Waypoint(car.varsF[0],car.varsF[1])+1)%NUM_WAYPOINTS;

// test if this is a new waypoint and not the last
if (test_waypoint != curr_waypoint)
   curr_waypoint = test_waypoint;

// compute vector toward waypoint
float wx = (path[curr_waypoint].x - car.varsF[0]);
float wy = (path[curr_waypoint].y - car.varsF[1]);

// normalize wx, wy
float length = Fast_Distance_2D(wx+0.5,wy+0.5);
wx/=length;
wy/=length;

// compute angle in radians
float car_angle = ((-90+car.curr_frame*22.5)*PI)/180;

// compute velocity vector of car (use look up for cos, sin in real life
float xv = cos(car_angle);
float yv = sin(car_angle);

// draw vectors
if (vector_display_on==1)
   {
   DDraw_Lock_Back_Surface();
   Draw_Line(car.varsF[0],car.varsF[1], path[curr_waypoint].x, path[curr_waypoint].y, 
              250, back_buffer, back_lpitch); 

   Draw_Line(car.varsF[0],car.varsF[1], car.varsF[0]+16*xv,car.varsF[1]+16*yv,
              246, back_buffer, back_lpitch); 
   DDraw_Unlock_Back_Surface();
   } // end if

// now turn car into waypoint direction, get sign of cross product
// between each vector
float sign_cross = (xv*wy - yv*wx);


if (ai_on)
{
// test the sign to determine which way to turn plus a little slosh
if (sign_cross > 0.3)
   {
   if (++car.curr_frame > 15)
      car.curr_frame = 0;   

   sprintf(buffer,"AI Turning RIGHT", sign_cross);
   Draw_Text_GDI(buffer,320, 460,RGB(255,255,255),lpddsback);

   } // end if
else
if (sign_cross < -0.3)
   {
   if (--car.curr_frame < 0 )
      car.curr_frame = 15;   

   sprintf(buffer,"AI Turning LEFT", sign_cross);
   Draw_Text_GDI(buffer,320, 460,RGB(255,255,255),lpddsback);

   } // end if
else
   {
   sprintf(buffer,"AI Turning 0", sign_cross);
   Draw_Text_GDI(buffer,320, 460,RGB(255,255,255),lpddsback);

   } // end else
} // end if
else
   {
   sprintf(buffer,"AI Disabled by user override", sign_cross);
   Draw_Text_GDI(buffer,320, 460,RGB(255,255,255),lpddsback);
   } // end if


// adjust floating position
car.varsF[0] += (xv*speed);
car.varsF[1] += (yv*speed);

// test for off screen -- user could cause this
if (car.varsF[0] > screen_width)
    car.varsF[0] = 0;
else
if (car.varsF[0] < 0)
    car.varsF[0] = screen_width;

if (car.varsF[1] > screen_height)
    car.varsF[1] = 0;
else
if (car.varsF[1] < 0)
    car.varsF[1] = screen_height;

// position sprite on pixel center
car.x = (0.5+car.varsF[0] - 8);
car.y = (0.5+car.varsF[1] - 8);

// draw the car
Draw_BOB(&car, lpddsback);

// show nearest waypoint
sprintf(buffer,"Nearest Waypoint %d", Find_Nearest_Waypoint(car.varsF[0],car.varsF[1]));
Draw_Text_GDI(buffer,64, 460,RGB(255,255,255),lpddsback);

sprintf(buffer,"Path following demo. Use arrow keys to override AI, <D> to toggle info, and <ESC> to Exit.");
Draw_Text_GDI(buffer,8, 8,RGB(255,255,255),lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30ish fps
Wait_Clock(30);

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