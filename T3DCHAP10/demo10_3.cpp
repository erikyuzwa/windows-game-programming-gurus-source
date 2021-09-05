// DEMO10_3.CPP - Loading and playing a wav file
// with real-time manipulation

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

// directx 7.0 compatibility

#ifndef DSBCAPS_CTRLDEFAULT
#define DSBCAPS_CTRLDEFAULT (DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME )
#endif

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"  // class name

#define WINDOW_WIDTH  400              // size of window
#define WINDOW_HEIGHT 300
#define SCREEN_WIDTH  640              // size of screen
#define SCREEN_HEIGHT 480
#define SCREEN_BPP    8                // bits per pixel

#define MAX_SOUNDS     256 // max number of sounds in system at once 

// digital sound object state defines
#define SOUND_NULL     0 // " "
#define SOUND_LOADED   1
#define SOUND_PLAYING  2
#define SOUND_STOPPED  3


// MACROS /////////////////////////////////////////////////

// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// TYPES //////////////////////////////////////////////////

typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// this holds a single sound
typedef struct pcm_sound_typ
	{
	LPDIRECTSOUNDBUFFER dsbuffer;   // the ds buffer containing the sound
	int state;                      // state of the sound
	int rate;                       // playback rate
	int size;                       // size of sound
	int id;                         // id number of the sound
	} pcm_sound, *pcm_sound_ptr;

// PROTOTYPES /////////////////////////////////////////////

int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);
int DSound_Load_WAV(char *filename, int control_flags = DSBCAPS_CTRLDEFAULT);

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

pcm_sound			sound_fx[MAX_SOUNDS];    // the array of secondary sound buffers


HWND                freq_hwnd,      // window handles for controls
                    volume_hwnd,
                    pan_hwnd;


int                 sound_id = -1;  // id of sound we load for demo

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
        
         // first the static text
         // set the color
         SetTextColor(hdc,RGB(0,255,0));
         SetBkColor(hdc,RGB(0,0,0));
         SetBkMode(hdc,OPAQUE);

         TextOut(hdc,160-5*7,40-20,"VOLUME        ",         strlen("VOLUME        "));
         TextOut(hdc,160-5*13,100-20,"PLAYBACK RATE        ",strlen("PLAYBACK RATE        "));
         TextOut(hdc,160-5*14,160-20,"STEREO PANNING        ",strlen("STEREO PANNING        "));
         
         // end painting
         EndPaint(hwnd,&ps);
         return(0);
        } break;

///////////////////////////////////////////////////////////

    case WM_HSCROLL:
    case WM_VSCROLL:
        {
        int nscrollcode    = (int)LOWORD(wparam); // scroll bar value 
        int npos           = (int)HIWORD(wparam); // scroll box position 
        HWND hwndscrollbar = (HWND)lparam;        // handle of scroll bar 

        // get teh dc for printing
        hdc = GetDC(hwnd);
        
        // set the color
        SetTextColor(hdc,RGB(0,255,0));
        SetBkColor(hdc,RGB(0,0,0));
        SetBkMode(hdc,OPAQUE);
                
        // make sure that the scroll bar is being tracked
        if (nscrollcode==SB_THUMBPOSITION || 
            nscrollcode==SB_THUMBTRACK) 
        {
        // what scroll bar sent message
        if (hwndscrollbar==volume_hwnd)
           {
           // re-position scroll bar
           SetScrollPos(volume_hwnd, SB_CTL,npos,TRUE);
           sprintf(buffer,"VOLUME=%d       ",-npos);
           
           // output text
           TextOut(hdc,160-5*7,40-20,buffer,strlen(buffer)); 

           // set the volume
           sound_fx[sound_id].dsbuffer->SetVolume(-npos);

           } // end if
        else
        if (hwndscrollbar==freq_hwnd)
           {
           // re-position scroll bar
           SetScrollPos(freq_hwnd, SB_CTL,npos,TRUE);
           sprintf(buffer,"PLAYBACK RATE=%d      ",npos);
     
           // output text
           TextOut(hdc,160-5*13,100-20,buffer,strlen(buffer));

           // set the frequency
           sound_fx[sound_id].dsbuffer->SetFrequency(npos);
           } // end if
        // what scroll bar sent message
        else
        if (hwndscrollbar==pan_hwnd)
           {
           // re-position scroll bar
           SetScrollPos(pan_hwnd, SB_CTL,npos,TRUE);
           sprintf(buffer,"STEREO PANNING=%d      ",-(npos-10000));
           
           // output text
           TextOut(hdc,160-5*14,160-20,buffer,strlen(buffer));
 
           // set the stereo panning
           sound_fx[sound_id].dsbuffer->SetPan(-(npos-10000));
           } // end if
        
        } // end if

        // release the dc
        ReleaseDC(hwnd,hdc);

        // message has been processed
        return(0);

        } break;

///////////////////////////////////////////////////////////

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
						  "DirectSound WAV File Loading DEMO",	 // title
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

// create some scroll controls to change volume, frequency
// and panning

// create the volume control scroller
volume_hwnd = CreateWindow("SCROLLBAR", // class
						  "",	    // title
						  WS_CHILD | WS_VISIBLE,
					 	  80,40,	    // x,y
						  160,       // width
                          16,      // height
						  hwnd,	    // handle to parent 
						  NULL,	    // handle to menu
						  hinstance,// instance
						  NULL);    // creation parms

// create the frequency control scroller
freq_hwnd = CreateWindow("SCROLLBAR", // class
						  "",	    // title
						  WS_CHILD | WS_VISIBLE,
					 	  80,100,	    // x,y
						  160,       // width
                          16,      // height
						  hwnd,	    // handle to parent 
						  NULL,	    // handle to menu
						  hinstance,// instance
						  NULL);    // creation parms

// create the stereo panning control scroller
pan_hwnd = CreateWindow("SCROLLBAR", // class
						  "",	    // title
						  WS_CHILD | WS_VISIBLE,
					 	  80,160,	// x,y
						  160,      // width
                          16,       // height
						  hwnd,	    // handle to parent 
						  NULL,	    // handle to menu
						  hinstance,// instance
						  NULL);    // creation parms


// set range and value of each scroll bar
SetScrollRange(volume_hwnd, SB_CTL, 0, 4000,TRUE);
SetScrollPos(volume_hwnd, SB_CTL,0,TRUE);

SetScrollRange(freq_hwnd, SB_CTL, 0, 50000,TRUE);
SetScrollPos(freq_hwnd, SB_CTL,11000,TRUE);

SetScrollRange(pan_hwnd, SB_CTL, 0, 20000,TRUE);
SetScrollPos(pan_hwnd, SB_CTL,10000,TRUE);

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

// create a directsound object
if (DirectSoundCreate(NULL, &lpds, NULL)!=DS_OK )
	return(0);

// set cooperation level
if (lpds->SetCooperativeLevel(main_window_handle,DSSCL_NORMAL)!=DS_OK)
	return(0);

// clear array out
memset(sound_fx,0,sizeof(pcm_sound)*MAX_SOUNDS);
	
// initialize the sound fx array
for (int index=0; index<MAX_SOUNDS; index++)
	{
	// test if this sound has been loaded
	if (sound_fx[index].dsbuffer)
		{
		// stop the sound
		sound_fx[index].dsbuffer->Stop();

		// release the buffer
		sound_fx[index].dsbuffer->Release();
	
		} // end if

	// clear the record out
	memset(&sound_fx[index],0,sizeof(pcm_sound));

	// now set up the fields
	sound_fx[index].state = SOUND_NULL;
	sound_fx[index].id    = index;

	} // end for index

// load a wav file in
if ((sound_id = DSound_Load_WAV("FLIGHT.WAV"))!=-1)
   {
   // start the voc playing in looping mode
   sound_fx[sound_id].dsbuffer->Play(0,0,DSBPLAY_LOOPING);
   } // end if

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// release the sound buffer
if (sound_fx[sound_id].dsbuffer)
   sound_fx[sound_id].dsbuffer->Release();

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

int DSound_Load_WAV(char *filename, int control_flags)
{
// this function loads a .wav file, sets up the directsound 
// buffer and loads the data into memory, the function returns 
// the id number of the sound


HMMIO 			hwav;    // handle to wave file
MMCKINFO		parent,  // parent chunk
                child;   // child chunk
WAVEFORMATEX    wfmtx;   // wave format structure

int	sound_id = -1,       // id of sound to be loaded
	index;               // looping variable

UCHAR *snd_buffer,       // temporary sound buffer to hold voc data
      *audio_ptr_1=NULL, // data ptr to first write buffer 
	  *audio_ptr_2=NULL; // data ptr to second write buffer

DWORD audio_length_1=0,  // length of first write buffer
	  audio_length_2=0;  // length of second write buffer
			
// step one: are there any open id's ?
for (index=0; index < MAX_SOUNDS; index++)
	{	
    // make sure this sound is unused
	if (sound_fx[index].state==SOUND_NULL)
	   {
	   sound_id = index;
	   break;
	   } // end if

	} // end for index

// did we get a free id?
if (sound_id==-1)
	return(-1);

// set up chunk info structure
parent.ckid 	    = (FOURCC)0;
parent.cksize 	    = 0;
parent.fccType	    = (FOURCC)0;
parent.dwDataOffset = 0;
parent.dwFlags		= 0;

// copy data
child = parent;

// open the WAV file
if ((hwav = mmioOpen(filename, NULL, MMIO_READ | MMIO_ALLOCBUF))==NULL)
    return(-1);

// descend into the RIFF 
parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

if (mmioDescend(hwav, &parent, NULL, MMIO_FINDRIFF))
    {
    // close the file
    mmioClose(hwav, 0);

    // return error, no wave section
    return(-1); 	
    } // end if

// descend to the WAVEfmt 
child.ckid = mmioFOURCC('f', 'm', 't', ' ');

if (mmioDescend(hwav, &child, &parent, 0))
    {
    // close the file
    mmioClose(hwav, 0);

    // return error, no format section
    return(-1); 	
    } // end if

// now read the wave format information from file
if (mmioRead(hwav, (char *)&wfmtx, sizeof(wfmtx)) != sizeof(wfmtx))
    {
    // close file
    mmioClose(hwav, 0);

    // return error, no wave format data
    return(-1);
    } // end if

// make sure that the data format is PCM
if (wfmtx.wFormatTag != WAVE_FORMAT_PCM)
    {
    // close the file
    mmioClose(hwav, 0);

    // return error, not the right data format
    return(-1); 
    } // end if

// now ascend up one level, so we can access data chunk
if (mmioAscend(hwav, &child, 0))
   {
   // close file
   mmioClose(hwav, 0);

   // return error, couldn't ascend
   return(-1); 	
   } // end if

// descend to the data chunk 
child.ckid = mmioFOURCC('d', 'a', 't', 'a');

if (mmioDescend(hwav, &child, &parent, MMIO_FINDCHUNK))
    {
    // close file
    mmioClose(hwav, 0);

    // return error, no data
    return(-1); 	
    } // end if

// finally!!!! now all we have to do is read the data in and
// set up the directsound buffer

// allocate the memory to load sound data
snd_buffer = (UCHAR *)malloc(child.cksize);

// read the wave data 
mmioRead(hwav, (char *)snd_buffer, child.cksize);

// close the file
mmioClose(hwav, 0);

// set rate and size in data structure
sound_fx[sound_id].rate  = wfmtx.nSamplesPerSec;
sound_fx[sound_id].size  = child.cksize;
sound_fx[sound_id].state = SOUND_LOADED;

// set up the format data structure
memset(&pcmwf, 0, sizeof(WAVEFORMATEX));

pcmwf.wFormatTag	  = WAVE_FORMAT_PCM;  // pulse code modulation
pcmwf.nChannels		  = 1;                // mono 
pcmwf.nSamplesPerSec  = 11025;            // always this rate
pcmwf.nBlockAlign	  = 1;                
pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
pcmwf.wBitsPerSample  = 8;
pcmwf.cbSize		  = 0;

// prepare to create sounds buffer
dsbd.dwSize			= sizeof(DSBUFFERDESC);
dsbd.dwFlags		= control_flags | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
dsbd.dwBufferBytes	= child.cksize;
dsbd.lpwfxFormat	= &pcmwf;

// create the sound buffer
if (FAILED(lpds->CreateSoundBuffer(&dsbd,&sound_fx[sound_id].dsbuffer,NULL)))
   {
   // release memory
   free(snd_buffer);

   // return error
   return(-1);
   } // end if

// copy data into sound buffer
if (FAILED(sound_fx[sound_id].dsbuffer->Lock(0,					 
								      child.cksize,			
								      (void **) &audio_ptr_1, 
								      &audio_length_1,
								      (void **)&audio_ptr_2, 
								      &audio_length_2,
								      DSBLOCK_FROMWRITECURSOR)))
								 return(0);

// copy first section of circular buffer
memcpy(audio_ptr_1, snd_buffer, audio_length_1);

// copy last section of circular buffer
memcpy(audio_ptr_2, (snd_buffer+audio_length_1),audio_length_2);

// unlock the buffer
if (FAILED(sound_fx[sound_id].dsbuffer->Unlock(audio_ptr_1, 
									    audio_length_1, 
									    audio_ptr_2, 
									    audio_length_2)))
 							     return(0);

// release the temp buffer
free(snd_buffer);

// return id
return(sound_id);

} // end DSound_Load_WAV
