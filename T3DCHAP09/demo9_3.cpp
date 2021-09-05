// DEMO9_3.CPP - DirectInput Joystick Demo

// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  

#define INITGUID // if not done elsewhere

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <objbase.h>
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


#include <ddraw.h>     // directX includes
#include <dinput.h>
#include "T3DLIB1.H"

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// default screen size
#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      8    // bits per pixel

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256

#define BUTTON_SPRAY    0    // defines for each button
#define BUTTON_PENCIL   1
#define BUTTON_ERASE    2
#define BUTTON_EXIT     3


// TYPES ///////////////////////////////////////////////////


// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL, int num_parms = 0);
int Game_Shutdown(void *parms=NULL, int num_parms = 0);
int Game_Main(void *parms=NULL,  int num_parms = 0);

// GLOBALS ////////////////////////////////////////////////

// windows vars
HWND      main_window_handle = NULL; // globally track main window
int       window_closed      = 0;    // tracks if window is closed
HINSTANCE main_instance      = NULL; // globally track hinstance

char buffer[80];                // used to print text

// directinput globals
LPDIRECTINPUT8        lpdi      = NULL;    // dinput object
LPDIRECTINPUTDEVICE8  lpdikey   = NULL;    // dinput keyboard
LPDIRECTINPUTDEVICE8  lpdimouse = NULL;    // dinput mouse
LPDIRECTINPUTDEVICE8  lpdijoy   = NULL;    // dinput joystick 
GUID                 joystickGUID;        // guid for main joystick
char                 joyname[80];         // name of joystick

// these contain the target records for all di input packets
UCHAR        keyboard_state[256]; // contains keyboard state table
DIMOUSESTATE mouse_state;         // contains state of mouse
DIJOYSTATE   joy_state;           // contains state of joystick

// demo globals

BITMAP_IMAGE playfield;       // used to hold playfield
BITMAP_IMAGE mushrooms[4];    // holds mushrooms
BOB          blaster;         // holds bug blaster

int blaster_anim[5] = {0,1,2,1,0};  // blinking animation

// lets use a line segment for the missle
int missile_x,              // position of missle
    missile_y,            
    missile_state;          // state of missle 0 off, 1 on

// PROTOTYPES //////////////////////////////////////////////

int Start_Missile(void);
int Move_Missile(void);
int Draw_Missile(void);

// FUNCTIONS /////////////////////////////////////////////////

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
main_instance = hinstance;

// register the window class
if (!RegisterClassEx(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindowEx(NULL,                  // extended style
                            WINDOW_CLASS_NAME,     // class
						    "DirectInput Joystick Demo", // title
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

////////////////////////////////////////////////////////////////////

BOOL CALLBACK DI_Enum_Joysticks(LPCDIDEVICEINSTANCE lpddi,
								LPVOID guid_ptr) 
{
// this function enumerates the joysticks, but
// stops at the first one and returns the
// instance guid of it, so we can create it

*(GUID*)guid_ptr = lpddi->guidInstance; 

// copy name into global
strcpy(joyname, (char *)lpddi->tszProductName);

// stop enumeration after one iteration
return(DIENUM_STOP);

} // end DI_Enum_Joysticks

///////////////////////////////////////////////////////////

int Start_Missile(void)
{
// this function starts the missle, if it is currently off

if (missile_state==0)
    {
    // enable missile
    missile_state = 1;

    // computer position of missile
    missile_x = blaster.x + 16;
    missile_y = blaster.y-4;

    // return success
    return(1);
    } // end if

// couldn't start missile
return(0);

} // end Start_Missile

///////////////////////////////////////////////////////////

int Move_Missile(void)
{
// this function moves the missle 

// test if missile is alive
if (missile_state==1)
   {
   // move the missile upward
   if ((missile_y-=10) < 0)
      {
      missile_state = 0;
      return(1);
      } // end if

   // lock secondary buffer
   DDraw_Lock_Back_Surface();

   // add missile collision here


   // unlock surface
   DDraw_Unlock_Back_Surface();

   // return success
   return(1);

   } // end if

// return failure
return(0);

} // end Move_Missle

///////////////////////////////////////////////////////////

int Draw_Missile(void)
{
// this function draws the missile 

// test if missile is alive
if (missile_state==1)
   {
   // lock secondary buffer
   DDraw_Lock_Back_Surface();

   // draw the missile in green
   Draw_Clip_Line(missile_x, missile_y, 
                  missile_x, missile_y+6,
                  250,back_buffer, back_lpitch);

   // unlock surface
   DDraw_Unlock_Back_Surface();

   // return success
   return(1);

   } // end if

// return failure
return(0);

} // end Draw_Missle

///////////////////////////////////////////////////////////

// GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms,  int num_parms)
{
// this function is where you do all the initialization 
// for your game

int index;         // looping var
char filename[80]; // used to build up files names

// initialize directdraw
DDraw_Init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

// joystick creation section ////////////////////////////////

// first create the direct input object
if (DirectInput8Create(main_instance,DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&lpdi,NULL)!=DI_OK)
   return(0);

// first find the fucking GUID of your particular joystick
lpdi->EnumDevices(DI8DEVCLASS_GAMECTRL, 
                  DI_Enum_Joysticks, 
                  &joystickGUID, 
                  DIEDFL_ATTACHEDONLY); 

if (lpdi->CreateDevice(joystickGUID, &lpdijoy, NULL)!=DI_OK)
   return(0);

// set cooperation level
if (lpdijoy->SetCooperativeLevel(main_window_handle, 
	                 DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)!=DI_OK)
   return(0);

// set data format
if (lpdijoy->SetDataFormat(&c_dfDIJoystick)!=DI_OK)
   return(0);

// set the range of the joystick
DIPROPRANGE joy_axis_range;

// first x axis
joy_axis_range.lMin = -24;
joy_axis_range.lMax = 24;

joy_axis_range.diph.dwSize       = sizeof(DIPROPRANGE); 
joy_axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
joy_axis_range.diph.dwObj        = DIJOFS_X;
joy_axis_range.diph.dwHow        = DIPH_BYOFFSET;

lpdijoy->SetProperty(DIPROP_RANGE,&joy_axis_range.diph);

// now y-axis
joy_axis_range.lMin = -24;
joy_axis_range.lMax = 24;

joy_axis_range.diph.dwSize       = sizeof(DIPROPRANGE); 
joy_axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
joy_axis_range.diph.dwObj        = DIJOFS_Y;
joy_axis_range.diph.dwHow        = DIPH_BYOFFSET;

lpdijoy->SetProperty(DIPROP_RANGE,&joy_axis_range.diph);

// and now the dead band

DIPROPDWORD dead_band; // here's our property word

dead_band.diph.dwSize       = sizeof(dead_band);
dead_band.diph.dwHeaderSize = sizeof(dead_band.diph);
dead_band.diph.dwObj        = DIJOFS_X;
dead_band.diph.dwHow        = DIPH_BYOFFSET;

// 10% will be used on both sides of the range +/-
dead_band.dwData            = 1000;

// finally set the property
lpdijoy->SetProperty(DIPROP_DEADZONE,&dead_band.diph);

dead_band.diph.dwSize       = sizeof(dead_band);
dead_band.diph.dwHeaderSize = sizeof(dead_band.diph);
dead_band.diph.dwObj        = DIJOFS_Y;
dead_band.diph.dwHow        = DIPH_BYOFFSET;

// 10% will be used on both sides of the range +/-
dead_band.dwData            = 1000;

// finally set the property
lpdijoy->SetProperty(DIPROP_DEADZONE,&dead_band.diph);


// acquire the joystick
if (lpdijoy->Acquire()!=DI_OK)
   return(0);

///////////////////////////////////////////////////////////

// load the background
Load_Bitmap_File(&bitmap8bit, "MUSH.BMP");

// set the palette to background image palette
Set_Palette(bitmap8bit.palette);

// load in the four frames of the mushroom
for (index=0; index<4; index++)
    {
    // create mushroom bitmaps
    Create_Bitmap(&mushrooms[index],0,0,32,32);
    Load_Image_Bitmap(&mushrooms[index],&bitmap8bit,index,0,BITMAP_EXTRACT_MODE_CELL);  
    } // end for index

// now create the bug blaster bob
Create_BOB(&blaster,0,0,32,32,3,
           BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_ANIM | BOB_ATTR_ANIM_ONE_SHOT,
           DDSCAPS_SYSTEMMEMORY);

// load in the four frames of the mushroom
for (index=0; index<3; index++)
     Load_Frame_BOB(&blaster,&bitmap8bit,index,index,1,BITMAP_EXTRACT_MODE_CELL);  

// unload the bitmap file
Unload_Bitmap_File(&bitmap8bit);

// set the animation sequences for bug blaster
Load_Animation_BOB(&blaster,0,5,blaster_anim);

// set up stating state of bug blaster
Set_Pos_BOB(&blaster,320, 400);
Set_Anim_Speed_BOB(&blaster,3);

// set clipping rectangle to screen extents so objects dont
// mess up at edges
RECT screen_rect = {0,0,screen_width,screen_height};
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

// create mushroom playfield bitmap
Create_Bitmap(&playfield,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
playfield.attr |= BITMAP_ATTR_LOADED;

// fill in the background
Load_Bitmap_File(&bitmap8bit, "GRASS.BMP");

// load the grass bitmap image
Load_Image_Bitmap(&playfield,&bitmap8bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Unload_Bitmap_File(&bitmap8bit);

// seed random number generator
srand(Start_Clock());

// create the random mushroom patch
for (index=0; index<50; index++)
    {
    // select a mushroom
    int mush = rand()%4;

    // set mushroom to random position
    mushrooms[mush].x = rand()%(SCREEN_WIDTH-32);
    mushrooms[mush].y = rand()%(SCREEN_HEIGHT-128);

    // now draw the mushroom into playfield
    Draw_Bitmap(&mushrooms[mush], playfield.buffer, playfield.width,1);

    } // end for

// hide the mouse
ShowCursor(FALSE);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms,  int num_parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// kill the bug blaster
Destroy_BOB(&blaster);

// kill the mushroom maker
for (int index=0; index<4; index++)
    Destroy_Bitmap(&mushrooms[index]);

// kill the playfield bitmap
Destroy_Bitmap(&playfield);

// release joystick
lpdijoy->Unacquire();
lpdijoy->Release();
lpdi->Release();

// shutdonw directdraw
DDraw_Shutdown();

// return success
return(1);
} // end Game_Shutdown

///////////////////////////////////////////////////////////

int Game_Main(void *parms, int num_parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int          index;             // looping var
int          dx,dy;             // general deltas used in collision detection


// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_SPACE))
    PostMessage(main_window_handle, WM_DESTROY,0,0);

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// get joystick data
lpdijoy->Poll(); // this is needed for joysticks only
lpdijoy->GetDeviceState(sizeof(DIJOYSTATE), (LPVOID)&joy_state);

// lock the back buffer
DDraw_Lock_Back_Surface();

// draw the background reactor image
Draw_Bitmap(&playfield, back_buffer, back_lpitch, 0);

// unlock the back buffer
DDraw_Unlock_Back_Surface();

// is the player moving?
blaster.x+=joy_state.lX;
blaster.y+=joy_state.lY;

// test bounds
if (blaster.x > SCREEN_WIDTH-32)
    blaster.x = SCREEN_WIDTH-32;
else
if (blaster.x < 0)
    blaster.x = 0;

if (blaster.y > SCREEN_HEIGHT-32)
    blaster.y = SCREEN_HEIGHT-32;
else
if (blaster.y < SCREEN_HEIGHT-128)
    blaster.y = SCREEN_HEIGHT-128;

// is player firing?
if (joy_state.rgbButtons[0])
   Start_Missile();

// move and draw missle
Move_Missile();
Draw_Missile();

// is it time to blink eyes
if ((rand()%100)==50)
   Set_Animation_BOB(&blaster,0);

// draw blaster
Animate_BOB(&blaster);
Draw_BOB(&blaster,lpddsback);

// draw some text
Draw_Text_GDI("Make My Centipede!",0,0,RGB(255,255,255),lpddsback);

// display joystick and buttons 0-7
sprintf(buffer,"Joystick Stats: X-Axis=%d, Y-Axis=%d, buttons(%d,%d,%d,%d,%d,%d,%d,%d)",
                                                                      joy_state.lX,joy_state.lY,
                                                                      joy_state.rgbButtons[0],
                                                                      joy_state.rgbButtons[1],
                                                                      joy_state.rgbButtons[2],
                                                                      joy_state.rgbButtons[3],
                                                                      joy_state.rgbButtons[4],
                                                                      joy_state.rgbButtons[5],
                                                                      joy_state.rgbButtons[6],
                                                                      joy_state.rgbButtons[7]);

Draw_Text_GDI(buffer,0,SCREEN_HEIGHT-20,RGB(255,255,50),lpddsback);

// print out name of joystick
sprintf(buffer, "Joystick Name & Vendor: %s",joyname);
Draw_Text_GDI(buffer,0,SCREEN_HEIGHT-40,RGB(255,255,50),lpddsback);


// flip the surfaces
DDraw_Flip();

// sync to 30 fps
Wait_Clock(30);

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////