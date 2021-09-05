// DEMO4_9.CPP - Starfield demo based on T3D console

// INCLUDES ///////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN  // just say no to MFC

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

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"
#define WINDOW_WIDTH      400
#define WINDOW_HEIGHT     300

// starfield defines
#define NUM_STARS            256

// MACROS /////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// TYPES //////////////////////////////////////////////////

typedef struct STAR_TYP
        {
        int x,y;        // position of star
        int vel;        // horizontal velocity of star
        COLORREF col;   // color of star
        } STAR, *STAR_PTR;

// PROTOTYPES /////////////////////////////////////////////

void Erase_Stars(void);
void Draw_Stars(void);
void Move_Stars(void);
void Init_Stars(void);

// GLOBALS ////////////////////////////////////////////////

HWND      main_window_handle = NULL; // globally track main window
HINSTANCE hinstance_app      = NULL; // globally track hinstance

HDC       global_dc          = NULL; // tracks a global dc

char buffer[80];                     // general printing buffer

STAR stars[256];                     // holds the starfield

// FUNCTIONS //////////////////////////////////////////////
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

void Init_Stars(void)
{
// this function initializes all the stars

for (int index=0; index < NUM_STARS; index++)
    {
    // select random position
    stars[index].x = rand()%WINDOW_WIDTH;
    stars[index].y = rand()%WINDOW_HEIGHT;

    // set random velocity   
    stars[index].vel = 1 + rand()%16;

    // set intensity which is inversely prop to velocity for 3D effect
    // note, I am mixing equal amounts of RGB to make black -> bright white    
    int intensity = 15*(17 - stars[index].vel);
    stars[index].col = RGB(intensity, intensity, intensity); 

    } // end for index

} // end Init_Stars

////////////////////////////////////////////////////////////

void Erase_Stars(void)
{
// this function erases all the stars
for (int index=0; index < NUM_STARS; index++)
    SetPixel(global_dc, stars[index].x, stars[index].y, RGB(0,0,0));

} // end Erase_Stars

////////////////////////////////////////////////////////////

void Draw_Stars()
{
// this function draws all the stars
for (int index=0; index < NUM_STARS; index++)
    SetPixel(global_dc, stars[index].x, stars[index].y, stars[index].col);


} // end Draw_Stars

////////////////////////////////////////////////////////////

void Move_Stars(void)
{
// this function moves all the stars and wraps them around the 
// screen boundaries
for (int index=0; index < NUM_STARS; index++)
    {
    // move the star and test for edge
    stars[index].x+=stars[index].vel;

    if (stars[index].x >= WINDOW_WIDTH)
        stars[index].x -= WINDOW_WIDTH;
    
    } // end for index

} // end Move_Stars

////////////////////////////////////////////////////////////

int Game_Main(void *parms = NULL, int num_parms = 0)
{
// this is the main loop of the game, do all your processing
// here

// get the time
DWORD start_time = GetTickCount();

// erase the stars
Erase_Stars();

// move the stars
Move_Stars();

// draw the stars
Draw_Stars();

// lock to 30 fps
while((start_time - GetTickCount() < 33));

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   SendMessage(main_window_handle,WM_CLOSE,0,0);

// return success or failure or your own return code here
return(1);

} // end Game_Main

////////////////////////////////////////////////////////////

int Game_Init(void *parms = NULL, int num_parms = 0)
{
// this is called once after the initial window is created and
// before the main event loop is entered, do all your initialization
// here

// first get the dc to the window
global_dc = GetDC(main_window_handle);

// initialize the star field here
Init_Stars();

// return success or failure or your own return code here
return(1);

} // end Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
// this is called after the game is exited and the main event
// loop while is exited, do all you cleanup and shutdown here

// release the global dc
ReleaseDC(main_window_handle, global_dc);

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
						    "T3D Game Console Star Demo", // title
						    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					 	    0,0,	  // initial x,y
						    400,300,  // initial width, height
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

