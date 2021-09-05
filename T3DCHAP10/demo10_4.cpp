// DEMO10_4.CPP DirectMusic Example

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

// directx 7.0 compatibility

#ifndef DSBCAPS_CTRLDEFAULT
#define DSBCAPS_CTRLDEFAULT (DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME )
#endif


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

// Convert from multibyte format to Unicode using the following macro:
#define MULTI_TO_WIDE( x,y )  MultiByteToWideChar( CP_ACP,MB_PRECOMPOSED, y,-1,x,_MAX_PATH);


// initializes a direct draw struct
#define DD_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// GLOBALS ////////////////////////////////////////////////
HWND      main_window_handle = NULL; // globally track main window
HINSTANCE main_instance      = NULL; // globally track hinstance

// direct music globals
IDirectMusicPerformance    *dm_perf = NULL;
IDirectMusicLoader         *dm_loader = NULL;
IDirectMusicSegment        *dm_segment = NULL;
IDirectMusicSegmentState   *dm_segstate = NULL;

// these are overwritten globally by DD_Init()
int screen_width  = SCREEN_WIDTH,            // width of screen
    screen_height = SCREEN_HEIGHT,           // height of screen
    screen_bpp    = SCREEN_BPP;              // bits per pixel

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

///////////////////////////////////////////////////////////

int Game_Main(void *parms = NULL, int num_parms = 0)
{
// this is the main loop of the game, do all your processing
// here

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   SendMessage(main_window_handle,WM_CLOSE,0,0);

// return success or failure or your own return code here
return(1);

} // end Game_Main

////////////////////////////////////////////////////////////

IDirectMusicPerformance* CreatePerformance(void)
{
// this function creates the performance

IDirectMusicPerformance* pPerf;     

if (FAILED(CoCreateInstance(CLSID_DirectMusicPerformance,
                            NULL,
                            CLSCTX_INPROC,
                            IID_IDirectMusicPerformance,
                            (void**)&pPerf)))    
   {
   // return null        
   pPerf = NULL;    
   } // end if

   return pPerf;

} // end CreatePerformance

////////////////////////////////////////////////////////////

IDirectMusicLoader* CreateLoader(void)
{
// this function creates the loader

    IDirectMusicLoader* pLoader;
 
    if (FAILED(CoCreateInstance(
            CLSID_DirectMusicLoader,
            NULL,
            CLSCTX_INPROC, 
            IID_IDirectMusicLoader,
            (void**)&pLoader)))
    {
    pLoader = NULL;
    } // end if

    return pLoader;

} // end CreateLoader

///////////////////////////////////////////////////////////

IDirectMusicSegment* LoadMIDISegment(IDirectMusicLoader* pLoader, 
                                     WCHAR *wszMidiFileName )
{
// this function loads a midi segment off disk

DMUS_OBJECTDESC ObjDesc; 
HRESULT hr;
IDirectMusicSegment* pSegment = NULL;
 
// get current working directory
char szDir[_MAX_PATH];
WCHAR wszDir[_MAX_PATH];
 
if(_getcwd( szDir, _MAX_PATH ) == NULL)
  {
  return NULL;
  } // end if

// convert to wide characters
MULTI_TO_WIDE(wszDir, szDir);

// set the search directory
hr = pLoader->SetSearchDirectory(GUID_DirectMusicAllTypes,wszDir, FALSE);

if (FAILED(hr)) 
   {
   return NULL;
   } // end if
 
// setup object description
ZeroMemory(&ObjDesc, sizeof(DMUS_OBJECTDESC));
ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
ObjDesc.guidClass = CLSID_DirectMusicSegment;
wcscpy(ObjDesc.wszFileName, wszMidiFileName );

ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
 
// load the object and query it for the IDirectMusicSegment interface
// This is done in a single call to IDirectMusicLoader::GetObject
// note that loading the object also initializes the tracks and does 
// everything else necessary to get the MIDI data ready for playback.

hr = pLoader->GetObject(&ObjDesc,IID_IDirectMusicSegment, (void**) &pSegment);

if (FAILED(hr))
   return(0);
 
// ensure that the segment plays as a standard MIDI file
// you now need to set a parameter on the band track
// Use the IDirectMusicSegment::SetParam method and let 
// DirectMusic find the trackby passing -1 (or 0xFFFFFFFF) in the dwGroupBits method parameter.

hr = pSegment->SetParam(GUID_StandardMIDIFile,-1, 0, 0, (void*)dm_perf);

if (FAILED(hr))
   return(0);
  
// This step is necessary because DirectMusic handles program changes and 
// bank selects differently for standard MIDI files than it does for MIDI 
// content authored specifically for DirectMusic. 
// The GUID_StandardMIDIFile parameter must be set before the instruments are downloaded. 

// The next step is to download the instruments. 
// This is necessary even for playing a simple MIDI file 
// because the default software synthesizer needs the DLS data 
// for the General MIDI instrument set
// If you skip this step, the MIDI file will play silently.
// Again, you call SetParam on the segment, this time specifying the GUID_Download parameter:

hr = pSegment->SetParam(GUID_Download, -1, 0, 0, (void*)dm_perf);

if (FAILED(hr))
   return(0);

// return the pointer
return pSegment;
 
} // end LoadSegment

////////////////////////////////////////////////////////////

int Game_Init(void *parms = NULL, int num_parms = 0)
{
// this is called once after the initial window is created and
// before the main event loop is entered, do all your initialization
// here

// set up directmusic
if (FAILED(CoInitialize(NULL)))
   {    
   // Terminate the application.
   return(0);
   }   // end if


// create the performance
dm_perf = CreatePerformance();

if (dm_perf == NULL)
   {
   return(0);// Failure -- performance not created
   } // end if


// initialize the performance
if (FAILED(dm_perf->Init(NULL, NULL, main_window_handle)))
   {
   return(0);// Failure -- performance not initialized
   } // end if 

// add the port to the performance
if (FAILED(dm_perf->AddPort(NULL)))
   {    
   return(0);// Failure -- port not initialized
   } // end if

// create the loader to load object(s) such as midi file
dm_loader = CreateLoader();

if (dm_loader == NULL)
   {
    return(0);// Failure -- loader not created
   } // end if

// release the old segment
if (dm_segment)
   {    
   dm_segment->Release();    
   dm_segment = NULL;
   } // end if

// load the segment
if (dm_loader)
   {
   dm_segment = LoadMIDISegment(dm_loader,L"BATTLE.MID");
   } // end if

// start the song
if (dm_segment)
   {
   dm_perf->PlaySegment(dm_segment, 0, 0, &dm_segstate);
   }  // end if

// return success or failure or your own return code here
return(1);

} // end Game_Init

/////////////////////////////////////////////////////////////

HRESULT FreeDirectMusic(void)
{
// If there is any music playing, stop it. This is 
// not really necessary, because the music will stop when
// the instruments are unloaded or the performance is    // closed down.
dm_perf->Stop( NULL, NULL, 0, 0 ); 

// Unload instruments this will cause silence.
// CloseDown unloads all instruments, so this call is also not 
// strictly necessary.
dm_segment->SetParam(GUID_Unload, -1, 0, 0, (void*)dm_perf); 

// Release the segment.    
dm_segment->Release(); 
// CloseDown and Release the performance object.    
dm_perf->CloseDown();
dm_perf->Release();     

// Release the loader object.
dm_loader->Release();     

// Release COM.    
CoUninitialize(); 
return S_OK;

} // end FreeDirectMusic

///////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
// this is called after the game is exited and the main event
// loop while is exited, do all you cleanup and shutdown here

FreeDirectMusic();

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
main_instance = hinstance;

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

