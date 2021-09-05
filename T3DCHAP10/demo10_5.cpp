// DEMO10_5.CPP DirectMusic Example
// make sure to link T3DLIB3.CPP and DEMO10_5.RC


// INCLUDES ///////////////////////////////////////////////

#define INITGUID

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
#include <direct.h>
#include <wchar.h>
#include <fcntl.h>

#include <ddraw.h>
#include <dsound.h> // include dsound, dmusic
#include <dmksctrl.h>
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>

#include "T3DLIB3.H"
#include "DEMO10_5RES.H"

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// default screen size
#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      8    // bits per pixel
#define MAX_COLORS      256  // maximum colors


// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// MACROS /////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// PROTOTYPES /////////////////////////////////////////////

// GLOBALS ////////////////////////////////////////////////

HWND      main_window_handle = NULL; // globally track main window
HINSTANCE hinstance_app      = NULL; // globally track hinstance

int midi_ids[10]; // hold storage for 10 songs 

char buffer[80];                     // general printing buffer

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

     case WM_COMMAND:
         {
          switch(LOWORD(wparam))
                {
                // handle the FILE menu
                case MENU_FILE_ID_EXIT:
                     {
                     // terminate window
                     PostQuitMessage(0);
                     } break;

                // handle the HELP menu
                case MENU_HELP_ABOUT:                 
                     {
                     //  pop up a message box
                     MessageBox(hwnd, "Menu MIDI Demo", 
                               "About MIDI Menu",
                                MB_OK | MB_ICONEXCLAMATION);
                     } break;

                // handle each of midi files
                case MENU_PLAY_ID_0:
                case MENU_PLAY_ID_1:
                case MENU_PLAY_ID_2:
                case MENU_PLAY_ID_3:
                case MENU_PLAY_ID_4:
                case MENU_PLAY_ID_5:
                case MENU_PLAY_ID_6:
                case MENU_PLAY_ID_7:
                case MENU_PLAY_ID_8:
                case MENU_PLAY_ID_9:
                     {
                     // play the midi file
                     int id =  LOWORD(wparam) - MENU_PLAY_ID_0;
                     
                     DMusic_Play(midi_ids[id]);
        
                     // get the device context
                     HDC hdc = GetDC(hwnd);

                     // set the foreground color to random
                     SetTextColor(hdc, RGB(0,255,0));

                     // set the background color to black
                     SetBkColor(hdc, RGB(0,0,0));

                     // finally set the transparency mode to transparent
                     SetBkMode(hdc, OPAQUE);

                     sprintf(buffer,"Playing MIDI segment [%d]   ",id);
 
                     // draw the midi file
                     TextOut(hdc,8,200,buffer, strlen(buffer));

                     ReleaseDC(hwnd, hdc);

                     } break;

                default: break;

             } // end switch wparam

          } break; // end WM_COMMAND
   
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

int index;

// get the device context
HDC hdc = GetDC(main_window_handle);

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   SendMessage(main_window_handle,WM_CLOSE,0,0);

// test for keys 0-9
for (index=0; index<10; index++)
    {
    // start the song
    if (KEYDOWN('0'+index))
        {
        DMusic_Play(midi_ids[index]);

        // set the foreground color to random
        SetTextColor(hdc, RGB(0,255,0));

        // set the background color to black
        SetBkColor(hdc, RGB(0,0,0));

        // finally set the transparency mode to transparent
        SetBkMode(hdc, OPAQUE);

        sprintf(buffer,"Playing MIDI segment [%d]   ",index);
 
        // draw some text at a random location
        TextOut(hdc,8,200,buffer, strlen(buffer));

        } // end if

    } // end for index

// release the device context back to windows
ReleaseDC(main_window_handle, hdc);

// return success or failure or your own return code here
return(1);

} // end Game_Main

///////////////////////////////////////////////////////////

int Game_Init(void *parms = NULL, int num_parms = 0)
{
// this is called once after the initial window is created and
// before the main event loop is entered, do all your initialization
// here
int index;

// initialize directmusic
DMusic_Init();

// load the midi segments
for (index=0; index<10; index++)
    {
    // build up string name
    sprintf(buffer,"MIDIFILE%d.MID",index);
    // load midi file
    midi_ids[index] = DMusic_Load_MIDI(buffer);
    } // end for index


// return success or failure or your own return code here
return(1);

} // end Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
// this is called after the game is exited and the main event
// loop while is exited, do all you cleanup and shutdown here

// delete all the music
DMusic_Delete_All_MIDI();

// shutdown directmusic
DMusic_Shutdown();

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
winclass.hIcon			= LoadIcon(hinstance, MAKEINTRESOURCE(ICON_T3DX));
winclass.hCursor		= LoadCursor(hinstance, MAKEINTRESOURCE(CURSOR_CROSSHAIR)); 
winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
winclass.lpszMenuName	= "SoundMenu";
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
						    "DirectMusic Demo", // title
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

