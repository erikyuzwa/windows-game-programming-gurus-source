// DEMO4_7.CPP  - Time interval locking demo

// INCLUDES ///////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN  // just say no to MFC

#include <windows.h>   // include all the windows headers
#include <windowsx.h>  // include useful macros
#include <mmsystem.h>  // very important and include WINMM.LIB too!
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

#define WINDOW_WIDTH  400
#define WINDOW_HEIGHT 300

// MACROS /////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// GLOBALS ////////////////////////////////////////////////
HWND      main_window_handle = NULL; // globally track main window

HINSTANCE hinstance_app      = NULL; // globally track hinstance
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

WNDCLASSEX winclass;  // this will hold the class we create
HWND	   hwnd;	  // generic window handle
MSG		   msg;		  // generic message
HDC        hdc;       // graphics device context

HPEN       pen=NULL;  // used to draw screen saver
int color_change_count = 100; // used to track when to change color

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
if (!(hwnd = CreateWindowEx(NULL,                // extended style
                            WINDOW_CLASS_NAME,   // class
						    "Time Inverval Locked Screen Saver", // title
						    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					 	    0,0,	      // initial x,y
						    WINDOW_WIDTH, // initial width
                            WINDOW_HEIGHT,// initial height
						    NULL,	  // handle to parent 
						    NULL,	  // handle to menu
						    hinstance,// instance of this application
						    NULL)))	  // extra creation parms
return(0);

// save main window handle
main_window_handle = hwnd;

// get the dc and hold onto it
hdc = GetDC(hwnd);

// seed random number generator
srand(GetTickCount());

// endpoints of line
int x1 = rand()%WINDOW_WIDTH;
int y1 = rand()%WINDOW_HEIGHT;
int x2 = rand()%WINDOW_WIDTH;
int y2 = rand()%WINDOW_HEIGHT;

// intial velocity of each end
int x1v = -4 + rand()%8;
int y1v = -4 + rand()%8;
int x2v = -4 + rand()%8;
int y2v = -4 + rand()%8;

// enter main event loop, but this time we use PeekMessage()
// instead of GetMessage() to retrieve messages
while(TRUE)
	{
    // get time referrence
    DWORD start_time = GetTickCount();
    
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

       // is it time to change color
       if (++color_change_count >= 100)
          {
          // reset counter
          color_change_count = 0;

          // create a random colored pen
          if (pen) 
             DeleteObject(pen);

          // create a new pen
          pen = CreatePen(PS_SOLID,1,RGB(rand()%256,rand()%256,rand()%256));

          // select the pen into context
          SelectObject(hdc,pen);

          } // end if


       // move endpoints of line
       x1+=x1v;
       y1+=y1v;
       
       x2+=x2v;
       y2+=y2v;

       // test if either end hit window edge
       if (x1 < 0 || x1 >= WINDOW_WIDTH)
          {
          // invert velocity
          x1v=-x1v;

          // bum endpoint back
          x1+=x1v;
          } // end if

       if (y1 < 0 || y1 >= WINDOW_HEIGHT)
          {
          // invert velocity
          y1v=-y1v;

          // bum endpoint back
          y1+=y1v;
          } // end if

       // now test second endpoint
       if (x2 < 0 || x2 >= WINDOW_WIDTH)
          {
          // invert velocity
          x2v=-x2v;

          // bum endpoint back
          x2+=x2v;
          } // end if

       if (y2 < 0 || y2 >= WINDOW_HEIGHT)
          {
          // invert velocity
          y2v=-y2v;

          // bum endpoint back
          y2+=y2v;
          } // end if

       // move to end one of line
       MoveToEx(hdc, x1,y1, NULL);

       // draw the line to other end
       LineTo(hdc,x2,y2);

       // lock time to 30 fps which is approx. 33 milliseconds
       while((GetTickCount() - start_time) < 33);
 
       // main game processing goes here
	   if (KEYDOWN(VK_ESCAPE))
          SendMessage(hwnd, WM_CLOSE, 0,0);
       
	} // end while


// release the device context
ReleaseDC(hwnd,hdc);

// return to Windows like this
return(msg.wParam);

} // end WinMain

///////////////////////////////////////////////////////////

