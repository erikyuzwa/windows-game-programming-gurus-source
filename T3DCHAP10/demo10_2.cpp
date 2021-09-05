// DEMO10_2.CPP - Making a simple sound with DirectSound
// makes a 100hz pure sine tone, try changing it for fun!

// INCLUDES ///////////////////////////////////////////////

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

// directx 7.0+ compatibility

#ifndef DSBCAPS_CTRLDEFAULT
#define DSBCAPS_CTRLDEFAULT (DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME )
#endif

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"  // class name

#define WINDOW_WIDTH  400              // size of window
#define WINDOW_HEIGHT 400
#define SCREEN_WIDTH  640              // size of screen
#define SCREEN_HEIGHT 480
#define SCREEN_BPP    8                // bits per pixel

// MACROS /////////////////////////////////////////////////

// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// TYPES //////////////////////////////////////////////////

typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// PROTOTYPES /////////////////////////////////////////////

int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE main_instance = NULL; // save the instance
char buffer[80];                // used to print text

LPDIRECTSOUND		lpds;           // directsound interface pointer
DSBUFFERDESC		dsbd;           // directsound description
DSCAPS				dscaps;         // directsound caps
HRESULT				dsresult;       // general directsound result
DSBCAPS             dsbcaps;        // directsound buffer caps

LPDIRECTSOUNDBUFFER	lpdsbprimary,    // you won't need this normally
                    lpdsbsecondary;  // the sound buffers

WAVEFORMATEX        pcmwf;          // generic waveformat structure
              
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
						  "DirectSound 100hz Tone Demo",	 // title
						  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
// start up the directsound sound
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

// this example does everything: it sets up directsound
// creates a secondary buffer, loads it with a synthesizer
// sine wave and plays it

void 	*audio_ptr_1 = NULL,   // used to lock memory
		*audio_ptr_2 = NULL;

DWORD	dsbstatus;              // status of sound buffer

DWORD	audio_length_1    = 0,  // length of locked memory
		audio_length_2    = 0,
		snd_buffer_length = 64000; // working buffer

// allocate memory for buffer
UCHAR *snd_buffer_ptr = (UCHAR *)malloc(snd_buffer_length);

// we need some data for the buffer, you could load a .VOC or .WAV
// but as an example, lets synthesize the data

// fill buffer with a synthesized 100hz sine wave
for (int index=0; index < (int)snd_buffer_length; index++)
	snd_buffer_ptr[index] = 127*sin(6.28*((float)(index%110))/(float)110);

// note the math, 127 is the scale or amplitude
// 6.28 is to convert to radians
// (index % 110) read below
// we are playing at 11025 hz or 11025 cycles/sec therefore, in 1 sec
// we want 100 cycles of our synthesized sound, thus 11025/100 is approx.
// 110, thus we want the waveform to repeat each 110 clicks of index, so
// normalize to 110


// create a directsound object
if (DirectSoundCreate(NULL, &lpds, NULL)!=DS_OK )
	return(0);

// set cooperation level
if (lpds->SetCooperativeLevel(main_window_handle,DSSCL_NORMAL)!=DS_OK)
	return(0);

// set up the format data structure
memset(&pcmwf, 0, sizeof(WAVEFORMATEX));

pcmwf.wFormatTag	  = WAVE_FORMAT_PCM;
pcmwf.nChannels		  = 1;
pcmwf.nSamplesPerSec  = 11025;
pcmwf.nBlockAlign	  = 1;
pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
pcmwf.wBitsPerSample  = 8;
pcmwf.cbSize		  = 0;

// create the secondary buffer (no need for a primary)
memset(&dsbd,0,sizeof(DSBUFFERDESC));
dsbd.dwSize			= sizeof(DSBUFFERDESC);
dsbd.dwFlags		= DSBCAPS_CTRLDEFAULT | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
dsbd.dwBufferBytes	= snd_buffer_length+1;
dsbd.lpwfxFormat	= &pcmwf;

if (lpds->CreateSoundBuffer(&dsbd,&lpdsbsecondary,NULL)!=DS_OK)
	return(0);

// copy data into sound buffer
if (lpdsbsecondary->Lock(0,					 
					  snd_buffer_length,			
    				  &audio_ptr_1, 
					  &audio_length_1,
					  &audio_ptr_2, 
					  &audio_length_2,
					  DSBLOCK_FROMWRITECURSOR)!=DS_OK)
  return(0);

// copy first section of circular buffer
CopyMemory(audio_ptr_1, snd_buffer_ptr, audio_length_1);

// copy last section of circular buffer
CopyMemory(audio_ptr_2, (snd_buffer_ptr+audio_length_1),audio_length_2);

// unlock the buffer
if (lpdsbsecondary->Unlock(audio_ptr_1, 
						   audio_length_1, 
						   audio_ptr_2, 
						   audio_length_2)!=DS_OK)
	return(0);

// play the sound in looping mode
if (lpdsbsecondary->Play(0,0,DSBPLAY_LOOPING )!=DS_OK)
	return(0);

// release the memory since DirectSound has made a copy of it
free(snd_buffer_ptr);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// release the directsoundobject
if (lpds!=NULL)
   lpds->Release();


// return success
return(1);
} // end Game_Shutdown

///////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_SPACE))
    PostMessage(main_window_handle, WM_DESTROY,0,0);

// return success
return(1);
} // end Game_Main

///////////////////////////////////////////////////////////

