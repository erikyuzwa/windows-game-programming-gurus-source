// DEMO4_6.CPP  - WM_TIMER demo

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


// timer defines

#define TIMER_ID1_SEC   1 // id of 1 sec timer
#define TIMER_ID3_SEC   2 // id of 3 sec timer
#define TIMER_ID30_SEC  3 // id of 30 sec timer

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
char            buffer[80]; // used to print strings

static int      counter1=0, // counters
                counter2=0,
                counter3=0;

// what is the message 
switch(msg)
	{	
	case WM_CREATE: 
        {
		// do initialization stuff here
        // create a 1 second timer
        SetTimer(hwnd, TIMER_ID1_SEC, 1000,NULL);

        // now the 3 second timer
        SetTimer(hwnd, TIMER_ID3_SEC, 3000,NULL);	

        // and finally the 30 second timer
        SetTimer(hwnd, TIMER_ID30_SEC, 30000,NULL);	

        // return success
		return(0);
		} break;


    case WM_TIMER:
         {
         // there is a timer event, test what timer fired
        switch(wparam)
              {
              case TIMER_ID1_SEC:
                   {
                   // print out a message
                   // get the dc
                   hdc = GetDC(hwnd);

                   // set the color
                   SetTextColor(hdc,RGB(0,255,0));
                   SetBkColor(hdc,RGB(0,0,0));
                   SetBkMode(hdc,OPAQUE);

                   // build up the messages
                   sprintf(buffer,"The 1 second timer has fired %d times",++counter1);

                   // print the message
                   TextOut(hdc,0,0,buffer,strlen(buffer));

                   // release the dc
                   ReleaseDC(hwnd,hdc);
                   } break;

              case TIMER_ID3_SEC:
                   {
                   // make a beep
                   MessageBeep(MB_ICONEXCLAMATION);

                   // get the dc
                   hdc = GetDC(hwnd);

                   // set the color
                   SetTextColor(hdc,RGB(0,255,0));
                   SetBkColor(hdc,RGB(0,0,0));
                   SetBkMode(hdc,OPAQUE);

                   // build up the messages
                   sprintf(buffer,"The 3 second timer has fired %d times",++counter2);

                   // print the message
                   TextOut(hdc,0,20,buffer,strlen(buffer));

                   // release the dc
                   ReleaseDC(hwnd,hdc);

                   } break;

            case TIMER_ID30_SEC:
                   {
                   // make a beep
                   MessageBeep(MB_ICONEXCLAMATION);

                   // get the dc
                   hdc = GetDC(hwnd);

                   // set the color
                   SetTextColor(hdc,RGB(0,255,0));
                   SetBkColor(hdc,RGB(0,0,0));
                   SetBkMode(hdc,OPAQUE);

                   // build up the messages
                   sprintf(buffer,"The 30 second timer has fired %d times",++counter3);

                   // print the message
                   TextOut(hdc,0,40,buffer,strlen(buffer));

                   // release the dc
                   ReleaseDC(hwnd,hdc);

                   } break;

               // .. test for other id's

               default:break;

               } // end switch
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
hinstance_app = hinstance;

// register the window class
if (!RegisterClassEx(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindowEx(NULL,                // extended style
                            WINDOW_CLASS_NAME,   // class
						    "Multiple Timer Demo", // title
						    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					 	    0,0,	  // initial x,y
						    WINDOW_WIDTH, // initial width
                            WINDOW_HEIGHT,// initial height
						    NULL,	  // handle to parent 
						    NULL,	  // handle to menu
						    hinstance,// instance of this application
						    NULL)))	// extra creation parms
return(0);

// save main window handle
main_window_handle = hwnd;

// get the graphics device context 
hdc = GetDC(hwnd);


// enter main event loop, but this time we use PeekMessage()
// instead of GetMessage() to retrieve messages
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
    if (KEYDOWN(VK_ESCAPE))
       SendMessage(hwnd, WM_CLOSE, 0,0);
       
	} // end while


// release the device context
ReleaseDC(hwnd,hdc);

// return to Windows like this
return(msg.wParam);

} // end WinMain

///////////////////////////////////////////////////////////

