// DEMO12_6_16b.CPP - pattern demo with tracking instruction
// 16-bit version, make sure desktop is in 16-bit mode!
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
#define WINDOW_CLASS_NAME "WINXCLASS"  // class name

// setup a 640x480 16-bit windowed mode example
#define WINDOW_TITLE      "16-Bit Pattern Demo with Tracking"
#define WINDOW_WIDTH      640   // size of window
#define WINDOW_HEIGHT     480

#define WINDOW_BPP        16    // bitdepth of window (8,16,24 etc.)
                                // note: if windowed and not
                                // fullscreen then bitdepth must
                                // be same as system bitdepth
                                // also if 8-bit the a pallete
                                // is created and attached

#define WINDOWED_APP      1     // 0 not windowed, 1 windowed

#define NUM_PATTERNS    4     // number of patterns in system

// pattern instruction opcodes for skelaton

// directional instructions
#define OPC_E    0  // move west
#define OPC_NE   1  // move northeast
#define OPC_N    2  // move north
#define OPC_NW   3  // move northwest
#define OPC_W    4  // move west
#define OPC_SW   5  // move southwest
#define OPC_S    6  // move south
#define OPC_SE   7  // move southeast

// special instructions
#define OPC_STOP        8  // stop for a moment
#define OPC_RAND        9  // select a random direction
#define OPC_TEST_DIST   10 // test distance 
#define OPC_END        -1  // end pattern


#define MIN_LONELYNESS  100 // minimum distance before skelaton
                            // gets lonely for its pet bat

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle           = NULL; // save the window handle
HINSTANCE main_instance           = NULL; // save the instance
char buffer[80];                          // used to print text

BITMAP_IMAGE background_bmp;   // holds the background
BOB          skelaton;         // the AI skelaton

BOB          bat;                // this time your the bat
int          bat_sound_id  = -1, // sound of bat flapping wings
             laugh_sound_id = -1; // sound of skelaton laughing

// animation sequences for bob
int skelaton_anims[8][4] = { {0,1,0,2},
                             {0+4,1+4,0+4,2+4},
                             {0+8,1+8,0+8,2+8},
                             {0+12,1+12,0+12,2+12},
                             {0+16,1+16,0+16,2+16},
                             {0+20,1+20,0+20,2+20},
                             {0+24,1+24,0+24,2+24},
                             {0+28,1+28,0+28,2+28}, };


// patterns in opcode operand format
int pattern_1[] = {OPC_W, 10, OPC_NW, 10, OPC_N, 10, OPC_NE, 10, 
                   OPC_TEST_DIST, 50, // a distance test
                   OPC_E, 10, OPC_SE, 10, OPC_S, 10, OPC_SW, 10, 
                   OPC_W, 10, OPC_RAND, 10,
                   OPC_TEST_DIST, 50, // a distance test                   
                   OPC_W, 20, OPC_NW, 10, OPC_N, 20, OPC_NE, 10, 
                   OPC_E, 20, OPC_SE, 10, OPC_S, 20, OPC_SW, 10, 
                   OPC_TEST_DIST, 50, // a distance test
                   OPC_W, 10, OPC_END,0};
                   

int pattern_2[] = {OPC_E, 20, OPC_W, 20, OPC_STOP, 20, OPC_NE, 10, 
                   OPC_TEST_DIST, 50, // a distance test                   
                   OPC_W, 10, OPC_NW, 10, OPC_SW, 20, OPC_NW, 20, 
                   OPC_TEST_DIST, 50, // a distance test
                   OPC_SW, 20, OPC_NW, 30, OPC_SW, 10, OPC_S, 50,  
                   OPC_TEST_DIST, 50, // a distance test                   
                   OPC_W, 2, OPC_NW, 2, OPC_N, 2, OPC_NE, 50,  OPC_TEST_DIST, 50, // a distance test
                   OPC_E,2, OPC_SE,2, OPC_S,2, OPC_RAND, 10, OPC_END,0};



int pattern_3[] = { OPC_N, 10, OPC_S, 10, OPC_N, 10, OPC_S, 10, 
                    OPC_TEST_DIST, 50, // a distance test                    
                    OPC_E, 10, OPC_W, 10, OPC_E, 10, OPC_W, 10,
                    OPC_TEST_DIST, 50, // a distance test
                    OPC_NW, 10, OPC_N, 10, OPC_NE, 10, OPC_N, 10, 
                    OPC_TEST_DIST, 60, // a distance test
                    OPC_STOP, 20, OPC_RAND, 5, OPC_E, 50, OPC_S, 50, OPC_W, 50, 
                    OPC_TEST_DIST, 50, // a distance test                    
                    OPC_E, 10, OPC_E, 10, OPC_E, 10, OPC_NW, 100,   OPC_TEST_DIST, 60, // a distance test
                    OPC_STOP, 10, OPC_END,0};


int pattern_4[] = {OPC_W, 100, 
                   OPC_NW, 2,OPC_N, 2,OPC_NE, 2,
                   OPC_E, 100, 
                   OPC_NE, 2,OPC_N, 2,OPC_NW, 2,
                   OPC_TEST_DIST, 50, // a distance test                    
                   OPC_W, 100, 
                   OPC_NW, 2,OPC_N, 2,OPC_NE, 2,
                   OPC_E, 100, 
                   OPC_NE, 2,OPC_N, 2,OPC_NW, 2,
                   OPC_TEST_DIST, 50, // a distance test                    
                   OPC_W, 100, 
                   OPC_NW, 2,OPC_N, 2,OPC_NE, 2,
                   OPC_E, 100, 
                   OPC_NE, 2,OPC_N, 2,OPC_NW, 2,
                   OPC_TEST_DIST, 50, // a distance test
                   OPC_RAND, 10, OPC_RAND, 5,

                   OPC_SW, 2,OPC_S, 2,OPC_SE, 2,
                   OPC_E, 100, 
                   OPC_TEST_DIST, 50, // a distance test
                   OPC_SE, 2,OPC_S, 2,OPC_SW, 2,
                   OPC_W, 100,
                   OPC_TEST_DIST, 50, // a distance test
                   OPC_SW, 2,OPC_S, 2,OPC_SE, 2,
                   OPC_E, 100, 
                   OPC_SE, 2,OPC_S, 2,OPC_SW, 2,
                   OPC_W, 100,
                   OPC_TEST_DIST, 50, // a distance test
                   OPC_SW, 2,OPC_S, 2,OPC_SE, 2,
                   OPC_E, 100, 
                   OPC_TEST_DIST, 50, // a distance test
                   OPC_SE, 2,OPC_S, 2,OPC_SW, 2,
                   OPC_W, 100, OPC_END,0};

// master pattern array
int *patterns[NUM_PATTERNS] = {pattern_1, pattern_2, pattern_3, pattern_4};

int *curr_pattern=NULL;  // current pattern being processed

int opcode,                   // general opcode
    operand,                  // general operand
    skelaton_ip     =0,       // pattern instruction pointer for skelaton
    skelaton_counter=0,       // counter of pattern control
    skelaton_pattern_index;   // the current pattern being executed

// used as a index to string lookup to help print out
char *opcode_names[] = {"OPC_E",
                        "OPC_NE", 
                        "OPC_N",    
                        "OPC_NW",   
                        "OPC_W",    
                        "OPC_SW",   
                        "OPC_S",    
                        "OPC_SE",   
                        "OPC_STOP", 
                        "OPC_RAND", 
                        "OPC_TEST_DIST"};  
// sound stuff

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

int index; // looping variable
int skelaton_anim[2];

char filename[80]; // used to build up filenames

// initialize directdraw, very important that in the call
// to setcooperativelevel that the flag DDSCL_MULTITHREADED is used
// which increases the response of directX graphics to
// take the global critical section more frequently
DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);

// load background image
Load_Bitmap_File(&bitmap16bit, "DUNGEON24.BMP");
Create_Bitmap(&background_bmp,0,0,640,480,16);
Load_Image_Bitmap16(&background_bmp, &bitmap16bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Unload_Bitmap_File(&bitmap16bit);

// load the bat bitmaps
Load_Bitmap_File(&bitmap16bit, "BATS2_24.BMP");

// create bat bob
Create_BOB(&bat,320,200, 16,16, 5, BOB_ATTR_MULTI_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY,0,16);
Set_Anim_Speed_BOB(&bat, 2);

// load the bat in 
for (index=0; index < 5; index++)
    Load_Frame_BOB16(&bat, &bitmap16bit, index, index, 0, BITMAP_EXTRACT_MODE_CELL);

// unload bat
Unload_Bitmap_File(&bitmap16bit);


// create skelaton bob
if (!Create_BOB(&skelaton,0,0,56,72,32,
           BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_ANIM,DDSCAPS_SYSTEMMEMORY,0,16))
   return(0);

// load the frames in 8 directions, 4 frames each
// each set of frames has a walk and a fire, frame sets
// are loaded in counter clockwise order looking down
// from a birds eys view or the x-z plane
for (int direction = 0; direction < 8; direction++)
    { 
    // build up file name
    sprintf(filename,"SKELSP%d_24.BMP",direction);

    // load in new bitmap file
    Load_Bitmap_File(&bitmap16bit,filename);
 
    Load_Frame_BOB16(&skelaton,&bitmap16bit,0+direction*4,0,0,BITMAP_EXTRACT_MODE_CELL);  
    Load_Frame_BOB16(&skelaton,&bitmap16bit,1+direction*4,1,0,BITMAP_EXTRACT_MODE_CELL);  
    Load_Frame_BOB16(&skelaton,&bitmap16bit,2+direction*4,2,0,BITMAP_EXTRACT_MODE_CELL);  
    Load_Frame_BOB16(&skelaton,&bitmap16bit,3+direction*4,0,1,BITMAP_EXTRACT_MODE_CELL);  

    // unload the bitmap file
    Unload_Bitmap_File(&bitmap16bit);

    // set the animation sequences for skelaton
    Load_Animation_BOB(&skelaton,direction,4,skelaton_anims[direction]);

    } // end for direction

// set up stating state of skelaton
Set_Animation_BOB(&skelaton, 0);
Set_Anim_Speed_BOB(&skelaton, 4);
Set_Vel_BOB(&skelaton, 0,0);
Set_Pos_BOB(&skelaton, 0, 128);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// load background sounds
bat_sound_id = DSound_Load_WAV("BAT.WAV");
laugh_sound_id = DSound_Load_WAV("LAUGH.WAV");

// start the sounds
DSound_Play(bat_sound_id, DSBPLAY_LOOPING);

// hide the mouse
if (!WINDOWED_APP)
   ShowCursor(FALSE);

// seed random number generate
srand(Start_Clock());

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
Destroy_BOB(&skelaton);
Destroy_BOB(&bat);

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

void Skelaton_AI(void)
{
// this function controls the ai of the skelaton and the pattern
// processing


// these are local defines to help compute the direction to move the
// skelaton if it needs to hunt down bat, note they are all in power
// of 2 order, hence, they are mutually exclusive

#define WEST_BIT     1
#define EAST_BIT     2
#define NORTH_BIT    4 
#define SOUTH_BIT    8 

// test if it's time to process a new instruction
if (curr_pattern==NULL)
   {
   // select a random pattern in pattern bank
   skelaton_pattern_index = rand()%NUM_PATTERNS;
   curr_pattern = patterns[skelaton_pattern_index];
   
   // now reset instuction pointer
   skelaton_ip = 0;

   // reset counter
   skelaton_counter = 0;
 
   } // end if

// process next instruction if it's time
if (--skelaton_counter <= 0)
    {
    // get next instruction
    opcode  = curr_pattern[skelaton_ip++];
    operand = curr_pattern[skelaton_ip++];

    // test what the opcode is
    switch(opcode)
        {
        case OPC_E:
            {
            // set direction to east
            Set_Vel_BOB(&skelaton,3,0);
  
            // set animation to east
            Set_Animation_BOB(&skelaton,opcode);
            
            // set counter to instuction operand
            skelaton_counter = operand;

            } break;

        case OPC_NE:
            {
            // set direction to northeast
            Set_Vel_BOB(&skelaton,3,-3);
  
            // set animation to northeast
            Set_Animation_BOB(&skelaton,opcode);
            
            // set counter to instuction operand
            skelaton_counter = operand;

            } break;

        case OPC_N: 
            {
            // set direction to north
            Set_Vel_BOB(&skelaton,0,-3);
  
            // set animation to north
            Set_Animation_BOB(&skelaton,opcode);
            
            // set counter to instuction operand
            skelaton_counter = operand;

            } break;            

        case OPC_NW:
            {
            // set direction to northwest
            Set_Vel_BOB(&skelaton,-3,-3);
  
            // set animation to northwest
            Set_Animation_BOB(&skelaton,opcode);
            
            // set counter to instuction operand
            skelaton_counter = operand;

            } break;            

        case OPC_W: 
            {
            // set direction to west
            Set_Vel_BOB(&skelaton,-3,0);
  
            // set animation to west
            Set_Animation_BOB(&skelaton,opcode);
            
            // set counter to instuction operand
            skelaton_counter = operand;
			 
            } break;            

        case OPC_SW:
            {
            // set direction to southwest
            Set_Vel_BOB(&skelaton,-3,3);
  
            // set animation to southwest
            Set_Animation_BOB(&skelaton,opcode);
            
            // set counter to instuction operand
            skelaton_counter = operand;

            } break;            

        case OPC_S: 
            {
            // set direction to south
            Set_Vel_BOB(&skelaton,0,3);
  
            // set animation to south
            Set_Animation_BOB(&skelaton,opcode);
            
            // set counter to instuction operand
            skelaton_counter = operand;

            } break;            

        case OPC_SE:
            {
            // set direction to southeast
            Set_Vel_BOB(&skelaton,3,3);
  
            // set animation to southeast
            Set_Animation_BOB(&skelaton,opcode);
            
            // set counter to instuction operand
            skelaton_counter = operand;

            } break;            

        case OPC_STOP: 
            {
            // stop motion
            Set_Vel_BOB(&skelaton,0,0);
            
            // set counter to instuction operand
            skelaton_counter = operand;

            } break;

        case OPC_RAND:
            {
            // set counter to instuction operand
            skelaton_counter = 0;

            } break;

        case OPC_TEST_DIST:
             {
             // test distance between bat and skelaton
             // if bat is too far, then move toward bat

             int dx = (bat.x - skelaton.x);
             int dy = (bat.y - skelaton.y);

             // test distance against minimum lonelyness
             if (sqrt((dx*dx) + (dy*dy)) > MIN_LONELYNESS)
                {
                // the skelaton needs to be pointed toward the bat (player)
                // this is a bit hard because we need to point the skelaton
                // in 1 of 8 directions, instead of just giving him a velocity
                // to solve the problem well break it up into a dx and a dy and then
                // use a look up table to set everything up right
   
                DSound_Play(laugh_sound_id);

                int direction = 0; // the bit encoded direction

                // first east-west
                if (bat.x > skelaton.x)
                   direction|=EAST_BIT;
                else
                if (bat.x < skelaton.x)
                   direction|=WEST_BIT;
                
                // now north-south
                if (bat.y > skelaton.y)
                   direction|=SOUTH_BIT;
                else
                if (bat.y < skelaton.y)
                   direction|=NORTH_BIT;
          
                // test final direction, note this could be compressed into
                // another look up table, but this is simpler 
                switch(direction)
                        {
                        case WEST_BIT:
                            {
                            // set motion
                            Set_Vel_BOB(&skelaton,-3,0);
  
                            // set animation 
                            Set_Animation_BOB(&skelaton,OPC_W);
            
                            // set counter to instuction operand
                            skelaton_counter = operand;

                            } break;

                        case EAST_BIT:     
                             {
                            // set motion
                            Set_Vel_BOB(&skelaton,3,0);
  
                            // set animation 
                            Set_Animation_BOB(&skelaton,OPC_E);
            
                            // set counter to instuction operand
                            skelaton_counter = operand;

                             } break;

                        case NORTH_BIT:     
                            {
                            // set motion
                            Set_Vel_BOB(&skelaton,0,-3);
  
                            // set animation 
                            Set_Animation_BOB(&skelaton,OPC_N);
            
                            // set counter to instuction operand
                            skelaton_counter = operand;

                            } break;

                        case SOUTH_BIT:     
                            {
                            // set motion
                            Set_Vel_BOB(&skelaton,0,3);
  
                            // set animation 
                            Set_Animation_BOB(&skelaton,OPC_S);
            
                            // set counter to instuction operand
                            skelaton_counter = operand;
                            } break;

                        case (NORTH_BIT | WEST_BIT):
                            {
                            // set motion
                            Set_Vel_BOB(&skelaton,-3,-3);
  
                            // set animation 
                            Set_Animation_BOB(&skelaton,OPC_NW);
            
                            // set counter to instuction operand
                            skelaton_counter = operand;
                            } break;

                        case (NORTH_BIT | EAST_BIT):
                            {
                            // set motion
                            Set_Vel_BOB(&skelaton,3,-3);
  
                            // set animation 
                            Set_Animation_BOB(&skelaton,OPC_NE);
            
                            // set counter to instuction operand
                            skelaton_counter = operand;
                            } break;

                        case (SOUTH_BIT | WEST_BIT):
                            {
                            // set motion
                            Set_Vel_BOB(&skelaton,-3,3);
  
                            // set animation 
                            Set_Animation_BOB(&skelaton,OPC_SW);
            
                            // set counter to instuction operand
                            skelaton_counter = operand;
                            } break;

                        case (SOUTH_BIT | EAST_BIT):
                            {
                            // set motion
                            Set_Vel_BOB(&skelaton,3,3);
  
                            // set animation 
                            Set_Animation_BOB(&skelaton,OPC_SE);
            
                            // set counter to instuction operand
                            skelaton_counter = operand;
                            } break;

                        default: break;

                        } // end switch 
               
                } // end if

             } break;

        case OPC_END: 
            {
            // stop motion
            Set_Vel_BOB(&skelaton,0,0);
            
            // select a random pattern in pattern bank
            skelaton_pattern_index = rand()%NUM_PATTERNS;
            curr_pattern = patterns[skelaton_pattern_index];
   
            // now reset instuction pointer
            skelaton_ip = 0;

            // reset counter
            skelaton_counter = 0;

            } break;
        
        default: break;

        } // end switch

    } // end if


// draw stats
sprintf(buffer,"Pattern #%d",skelaton_pattern_index);
Draw_Text_GDI(buffer,10, 400,RGB(0,255,0), lpddsback);

sprintf(buffer,"Opcode=%s Operand=%d",opcode_names[opcode],operand);
Draw_Text_GDI(buffer,10, 416,RGB(0,255,0), lpddsback);

sprintf(buffer,"Instruction Ptr=%d ", skelaton_ip);
Draw_Text_GDI(buffer,10, 432,RGB(0,255,0), lpddsback);

sprintf(buffer,"Counter=%d ", skelaton_counter);
Draw_Text_GDI(buffer,10, 448,RGB(0,255,0), lpddsback);

} // end skelaton_AI

//////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw background
Draw_Bitmap16(&background_bmp, back_buffer, back_lpitch,0);

// unlock back surface
DDraw_Unlock_Back_Surface();

// read keyboard
DInput_Read_Keyboard();

// do the ai on skelaton
Skelaton_AI();

// animate the bat
Animate_BOB(&bat);

// the animate the skelaton unless its stopped
if (opcode!=OPC_STOP)
   Animate_BOB(&skelaton);

// move skelaton
Move_BOB(&skelaton);

// test if skelaton is off screen, if so wrap around
if (skelaton.x >= SCREEN_WIDTH)
   skelaton.x = -skelaton.width;
else
if (skelaton.x < -skelaton.width)
   skelaton.x = SCREEN_WIDTH;

if (skelaton.y >= SCREEN_HEIGHT)
   skelaton.y = -skelaton.height;
else
if (skelaton.y < -skelaton.height)
   skelaton.y = SCREEN_HEIGHT;

// let player move the bat
// allow player to move
if (keyboard_state[DIK_RIGHT])
   bat.x+=4;
else
if (keyboard_state[DIK_LEFT])
   bat.x-=4;

if (keyboard_state[DIK_UP])
   bat.y-=4;
else
if (keyboard_state[DIK_DOWN])
   bat.y+=4;


// test if bat is off screen, if so wrap around
if (bat.x >= SCREEN_WIDTH)
   bat.x = -bat.width;
else
if (bat.x < -bat.width)
   bat.x = SCREEN_WIDTH;

if (bat.y >= SCREEN_HEIGHT)
   bat.y = -bat.height;
else
if (bat.y < -bat.height)
   bat.y = SCREEN_HEIGHT;

// draw the skelaton
Draw_BOB(&skelaton, lpddsback);

// draw the bat
Draw_BOB16(&bat, lpddsback);

// draw title
Draw_Text_GDI("(16-Bit Version) Skelaton Pattern Demo With Tracking",10, 10,RGB(0,255,255), lpddsback);

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

    } // end if

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////