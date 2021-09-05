// DEMO12_7_16b.CPP - memory demo
// 16-bit version
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

// setup a 640x480 16-bit windowed mode example
#define WINDOW_TITLE      "16-Bit Memory Demo"
#define WINDOW_WIDTH      640   // size of window
#define WINDOW_HEIGHT     480

#define WINDOW_BPP        16    // bitdepth of window (8,16,24 etc.)
                                // note: if windowed and not
                                // fullscreen then bitdepth must
                                // be same as system bitdepth
                                // also if 8-bit the a pallete
                                // is created and attached

#define WINDOWED_APP      1     // 0 not windowed, 1 windowed

// defines for ants
#define NUM_ANTS         16 // just change this to whatever, but only 8 ants will be displayed
                            // on HUD
#define ANT_ANIM_UP      0
#define ANT_ANIM_RIGHT   1
#define ANT_ANIM_DOWN    2
#define ANT_ANIM_LEFT    3
#define ANT_ANIM_DEAD    4

// states of ant
#define ANT_WANDERING             0   // moving around randomly
#define ANT_EATING                1   // at a mnm eating it
#define ANT_RESTING               2   // sleeping :)
#define ANT_SEARCH_FOOD           3   // hungry and searching for food
       
// these are the substates that occur during a search for food
        #define ANT_SEARCH_FOOD_S1_SCAN         31  // substate 1
        #define ANT_SEARCH_FOOD_S2_WANDER       32  // substate 2
        #define ANT_SEARCH_FOOD_S3_VECTOR_2CELL 33  // substate 3
        #define ANT_SEARCH_FOOD_S4_VECTOR_2FOOD 34  // substate 4
        #define ANT_SEARCH_FOOD_S5              35  // substate 5
        #define ANT_SEARCH_FOOD_S6              36  // substate 6
        #define ANT_SEARCH_FOOD_S7              37  // substate 7

#define ANT_COMMUNICATING         4   // talking to another ant  
#define ANT_DEAD                  5   // this guy is dead, got too hungry

#define ANT_INDEX_HUNGER_LEVEL       0  // the current hunger level of ant
#define ANT_INDEX_HUNGER_TOLERANCE   1  // the death tolerance of hunger
#define ANT_INDEX_AI_STATE           2  // the artificial intelligence state
#define ANT_INDEX_AI_SUBSTATE        3  // generic substate
#define ANT_INDEX_DIRECTION          4  // direction of motion
#define ANT_INDEX_LAST_TALKED_WITH   5  // last ant talked to
#define ANT_INDEX_MNM_BEING_DEVOURED 6  // the index of mnm being eaten
#define ANT_INDEX_FOOD_TARGET_X      7  // x,y target position of cell or off actual food
#define ANT_INDEX_FOOD_TARGET_Y      8    
#define ANT_INDEX_FOOD_TARGET_ID     9  // the id of the actual piece of food

#define ANT_MEMORY_RESIDUAL_RATE     0.2 // inversely proportional to plasticity of ant memory

#define BITE_SIZE                    5 // bite size of one mouthful in energy units

// defines for food
#define NUM_MNMS         32

// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// ants stuff
void Move_Ants(void);
void Init_Ants(void);
void Draw_Ants(void);

void Draw_Food(void);

// TYPES /////////////////////////////////////////////////

// this is the food
typedef struct MNM_TYP
{
int x,y;     // position of mnm
float energy;  // when this is 0, the mnm is dead

} MNM, *MNM_PTR;


typedef struct ANT_MEMORY_TYP
{

float cell[16][16];     // the actual memory
int   ilayer[16][16];   // used as an input layer to help display engine show
                        // areas during communication and forgetfulness

} ANT_MEMORY, *ANT_MEMORY_PTR;


// MACROS ////////////////////////////////////////////////

#define RAND_RANGE(x,y) ( (x) + (rand()%((y)-(x)+1)))


// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle           = NULL; // save the window handle
HINSTANCE main_instance           = NULL; // save the instance
char buffer[256];                          // used to print text

BITMAP_IMAGE background_bmp;   // holds the background

BOB          ants[NUM_ANTS],   // the ants
             mnm;              // the little food mnm image

MNM food[NUM_MNMS];            // the array of mnms

ANT_MEMORY ants_mem[NUM_ANTS]; // each ant has a 256 cell memory, 16x16 structure

int niceday_sound_id = -1;     // the ambient wind


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

//////////////////////////////////////////////////////////

// T3D GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms)
{
// this function is where you do all the initialization 
// for your game

int index; // looping variable

// seed the random number generator
srand(Get_Clock());

// initialize directdraw, very important that in the call
// to setcooperativelevel that the flag DDSCL_MULTITHREADED is used
// which increases the response of directX graphics to
// take the global critical section more frequently
DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);

// load background image
Load_Bitmap_File(&bitmap16bit, "SIDEWALK224.BMP");
Create_Bitmap(&background_bmp,0,0,640,480,16);
Load_Image_Bitmap16(&background_bmp, &bitmap16bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Unload_Bitmap_File(&bitmap16bit);

// load the ant bitmaps
Load_Bitmap_File(&bitmap16bit, "ANTIMG24.BMP");

// create master ant
Create_BOB(&ants[0],320,200, 24,24, 9, BOB_ATTR_MULTI_ANIM | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY,0,16);

// load the ants in 
for (index=0; index < 9; index++)
    Load_Frame_BOB16(&ants[0], &bitmap16bit, index, index, 0, BITMAP_EXTRACT_MODE_CELL);


// set the animations
int ant_anim_up[3]    = {0,1,-1};
int ant_anim_right[3] = {2,3,-1};
int ant_anim_down[3]  = {4,5,-1};
int ant_anim_left[3]  = {6,7,-1};
int ant_anim_dead[2]  = {8,-1};

Load_Animation_BOB(&ants[0],0,2, ant_anim_up);
Load_Animation_BOB(&ants[0],1,2, ant_anim_right);
Load_Animation_BOB(&ants[0],2,2, ant_anim_down);
Load_Animation_BOB(&ants[0],3,2, ant_anim_left);
Load_Animation_BOB(&ants[0],4,2, ant_anim_dead);

Set_Anim_Speed_BOB(&ants[0],3);
Set_Animation_BOB(&ants[0], ANT_ANIM_UP);

// clone the ants
for (index=1; index < NUM_ANTS; index++)
    Clone_BOB(&ants[0], &ants[index]);

// initialize the ants
Init_Ants();

// create the mnm
Create_BOB(&mnm,0,0, 8,8, 1, BOB_ATTR_SINGLE_FRAME | BOB_ATTR_VISIBLE, DDSCAPS_SYSTEMMEMORY,0,16);

// load the mnm 
Load_Frame_BOB16(&mnm, &bitmap16bit, 0, 1, 141, BITMAP_EXTRACT_MODE_ABS);


// position all the mnms
int num_piles = 3+rand()%3;
int curr_mnm = 0;

for (int piles=0; piles < num_piles; num_piles++)
    {
    // plop down some mnms at the pile position
    int pile_x = 32 + rand()%400;
    int pile_y = rand()%480;

    // compute number of mnms for pile
    int num_mnms_pile = 5 + rand()%15;

    // now find a position for each
    for (index = 0; index < num_mnms_pile; index++)
        {
        // select random position and energy level for mnm
        food[curr_mnm].x = pile_x + rand()%20;
        food[curr_mnm].y = pile_y + rand()%20;
        food[curr_mnm].energy = 600 + rand()%1000;

        // increment total number of mnms thus far
        if (++curr_mnm >= NUM_MNMS)
           break;

        } // end for index

        if (++curr_mnm >= NUM_MNMS)
           break;

      } // end for pile 

// unload ant imagery
Unload_Bitmap_File(&bitmap16bit);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();
DInput_Init_Mouse();

// initilize DirectSound
DSound_Init();

// load background sounds
niceday_sound_id = DSound_Load_WAV("NICEDAY.WAV");

// start the sounds
DSound_Play(niceday_sound_id, DSBPLAY_LOOPING);

// hide the mouse
if (!WINDOWED_APP)
   ShowCursor(FALSE);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

int index; // looping var

// shut everything down

// kill all the bobs
for (index = 0; index<NUM_ANTS; index++)
    Destroy_BOB(&ants[index]);

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

#if 0

// defines for ants
#define NUM_ANTS        16
#define ANT_ANIM_UP      0
#define ANT_ANIM_RIGHT   1
#define ANT_ANIM_DOWN    2
#define ANT_ANIM_LEFT    3

// states of ant
#define ANT_WANDERING             0   // moving around randomly
#define ANT_EATING                1   // at a mnm eating it
#define ANT_RESTING               2   // sleeping :)
#define ANT_SEARCH_FOOD           3   // hungry and searching for food
        #define ANT_SEARCH_FOOD_S1        31  // substate 1
        #define ANT_SEARCH_FOOD_S2        32  // substate 2
#define ANT_COMMUNICATING         4   // talking to another ant  
#define ANT_DEAD                  5   // this guy is dead, got too hungry

#define ANT_INDEX_HUNGER_LEVEL     0
#define ANT_INDEX_HUNGER_TOLERANCE 1 
#define ANT_INDEX_AI_STATE         2
#define ANT_INDEX_AI_SUBSTATE      3
#define ANT_INDEX_DIRECTION        4

#endif


void Init_Ants(void)
{
// this function initializes all the ant positions, states, etc.

int index;

for (index=0; index < NUM_ANTS; index++)
    {
    // set the position of ant
    ants[index].x = rand()%472;
    ants[index].y = rand()%screen_height;

    // set the hunger level and tolerance of this guy
    ants[index].varsI[ANT_INDEX_HUNGER_LEVEL]     = 0;

    // ant will die if hunger level reaches this
    ants[index].varsI[ANT_INDEX_HUNGER_TOLERANCE] = 2000+rand()%2000;  

    // set the ai state of ant
    ants[index].varsI[ANT_INDEX_AI_STATE] = ANT_WANDERING;

    // set last ant talked to as self
    ants[index].varsI[ANT_INDEX_LAST_TALKED_WITH] = index;

    // set how long to wander
    ants[index].counter_1 = RAND_RANGE(150, 300);

    // set direction
    ants[index].varsI[ANT_INDEX_DIRECTION] = RAND_RANGE(ANT_ANIM_UP, ANT_ANIM_LEFT);
    
    // time in that direction
    ants[index].counter_2 = RAND_RANGE(10, 100);

    // start animation
    Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);

    // init ant memory 
    memset(&ants_mem[index], 0, sizeof(ANT_MEMORY));

    } // end for index

} // end Init_Ants

//////////////////////////////////////////////////////////////////

void Draw_Ants(void)
{
// this function draws all the ants

int index;

for (index=0; index < NUM_ANTS; index++)
    {
    // draw the image
    ants[index].x-=8; ants[index].y-=8; // center ant
    Draw_BOB16(&ants[index], lpddsback);
    ants[index].x+=8; ants[index].y+=8; // fix center

    // draw a little number above ant
    sprintf(buffer,"%d", index);
    Draw_Text_GDI(buffer,ants[index].x,ants[index].y-16,RGB(0,255,0),lpddsback);

    // animate the ant
    if (ants[index].varsI[ANT_INDEX_AI_STATE] == ANT_WANDERING ||
        ants[index].varsI[ANT_INDEX_AI_STATE] == ANT_SEARCH_FOOD ||
        ants[index].varsI[ANT_INDEX_AI_STATE] == ANT_DEAD)
    Animate_BOB(&ants[index]);

 

    } // end for index

} // end Draw_Ants

////////////////////////////////////////////////////////////////////

float Food_Near_Ant(int cell_x, int cell_y)
{
// this functions scan all the food in the universe and tests if any is
// close to ant in this cell, if so the energy level of the food is scaled 
// and summed to the "memory" strength of that particular geographical location
// in the ants memory
// this algorithm is totally inefficient, in real life this was be called only
// once and all the cell based positions would be pre-computed since the mnms never
// move, but this is just to show you how you would do it if they could move...

int index; // looping var

float food_sum = 0; // used to tally up food in cell sector

// is this mnm in the current cell?
for (index = 0; index < NUM_MNMS; index++)
    {
    // is this mnm still there
    if (food[index].energy > 0)
       {
       // compute cell position, this is dumb, but needed if mnms can move, which
       // they could if you give the ants the ability to pick them up :)
       int mnm_x = food[index].x / 30;
       int mnm_y = food[index].y / 30;

       // is this within the same cell
       if (mnm_x == cell_x && mnm_y == cell_y)
          food_sum+=(food[index].energy/5);

       } // end if

    } // end for index

// now send it back 
return(food_sum);

} // end Food_Near_Ant

////////////////////////////////////////////////////////////////////////////////

int Max_Food_In_Cell(int cell_x, int cell_y, int *food_x, int *food_y)
{
// this function finds the exact location of the mnm with the highest energy in a cell

float max_food = 0;   // used to tally up food in cell sector
int max_food_id = 0; // used to track winner

// is this mnm in the current cell?
for (int index = 0; index < NUM_MNMS; index++)
    {
    // is this mnm still there
    if (food[index].energy > 0)
       {
       // compute cell position, this is dumb, but needed if mnms can move, which
       // they could if you give the ants the ability to pick them up :)
       int mnm_x = food[index].x / 30;
       int mnm_y = food[index].y / 30;

       // is this within the same cell
       if (mnm_x == cell_x && mnm_y == cell_y)
          { 
          // is this higher energy
          if (food[index].energy > max_food)
             {
             // set this as new food
             max_food_id = index;
             max_food = food[index].energy;
             *food_x = food[index].x;
             *food_y = food[index].y;
             } // end if

          } // end if

       } // end if

    } // end for index

// now send it back 
return(max_food_id);

} // end Max_Food_In_Cell

/////////////////////////////////////////////////////////////////////

int Select_State_Rand(int state1, int prob1,            
                      int state2, int prob2,    
                      int state3, int prob3,    
                      int state4, int prob4,    
                      int state5, int prob5,
                      int state6, int prob6)
{
// this function simply selects one of state1...state6 based on the probability
// of each state, if probi is 0 then the state is not considered

int index     = 0,   // looping variable
    curr_elem = 0,   // tracks next entry to place in table
    state_prob[100]; // used to hold generated probability look up

// build probability table
for (index = 0; index < prob1; index++)
    state_prob[curr_elem++] = state1;

for (index = 0; index < prob2; index++)
    state_prob[curr_elem++] = state2;

for (index = 0; index < prob3; index++)
    state_prob[curr_elem++] = state3;

for (index = 0; index < prob4; index++)
    state_prob[curr_elem++] = state4;

for (index = 0; index < prob5; index++)
    state_prob[curr_elem++] = state5;

for (index = 0; index < prob6; index++)
    state_prob[curr_elem++] = state6;

// now select a state
return(state_prob[rand()%100]);

} // end Select_State_Rand

///////////////////////////////////////////////////////////////////////

void Set_New_State(int new_state, int index, int var1=0, int var2=0)
{
// this function sets the state of the ant to new_state

       // reset all state info
       ants[index].varsI[ANT_INDEX_AI_STATE]    = 0;
       ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = 0;
       ants[index].counter_1                    = 0;
       ants[index].counter_2                    = 0;
       
       // now set new state info
       switch(new_state)
           {
           case ANT_WANDERING:      // moving around randomly
               {
               // set the ai state of ant
               ants[index].varsI[ANT_INDEX_AI_STATE] = ANT_WANDERING;

               // set how long to wander
               ants[index].counter_1 = RAND_RANGE(150, 300);

               // set direction
               ants[index].varsI[ANT_INDEX_DIRECTION] = RAND_RANGE(ANT_ANIM_UP, ANT_ANIM_LEFT);
    
               // time in that direction
               ants[index].counter_2 = RAND_RANGE(10, 100);

               // start animation
               Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);

               } break;

          case ANT_EATING:         // at a mnm eating it
               {
               // set state to eating
               ants[index].varsI[ANT_INDEX_AI_STATE] = ANT_EATING;

               // start eating the mnm indexed by var1 
               ants[index].varsI[ANT_INDEX_MNM_BEING_DEVOURED] = var1;

               // counters not used in this state

               } break;          

          case ANT_RESTING:        // sleeping :)
               {
               // set the ai state of ant
               ants[index].varsI[ANT_INDEX_AI_STATE] = ANT_RESTING;

               // set how long to rest
               ants[index].counter_1 = RAND_RANGE(50, 200);
              

               } break;

          case ANT_SEARCH_FOOD:    // hungry and searching for food
               {
               // set state to search for food
               ants[index].varsI[ANT_INDEX_AI_STATE]    = ANT_SEARCH_FOOD;

               // start off by scanning for food
               ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = ANT_SEARCH_FOOD_S1_SCAN;

               // initialize targets tp 0
               ants[index].varsI[ANT_INDEX_FOOD_TARGET_X] = 0;
               ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y] = 0;


               } break;

          case ANT_COMMUNICATING:  // talking to another ant  
               {

               // set the ai state of ant
               ants[index].varsI[ANT_INDEX_AI_STATE] = ANT_COMMUNICATING;

               // set how long to communicate
               ants[index].counter_1 = RAND_RANGE(60, 90);

               // when counter 2 hits 5 then exchange memory
               // thus about 5-10% of the memories will be exchanged
               ants[index].counter_2 = 0;

               } break;

          case ANT_DEAD:           // this guy is dead, got too hungry
               {
               // set the ai state of ant
               ants[index].varsI[ANT_INDEX_AI_STATE] = ANT_DEAD;

               // set direction
               ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_DEAD;
    
               // start animation
               Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
 
               } break;

          default: break;

          } // end switch             

} // end Set_New_State


//////////////////////////////////////////////////////////////////////

void Move_Ants(void)
{
// this function moves all the ants and processes the ai

// up, right, down, left
static int ant_movements_x[4] = { 0,2,0,-2};
static int ant_movements_y[4] = {-2,0,2, 0};

static int clear_ilayer = 0; // tracks when to clear the input memory layer

int index;
int select_new_state = -1;

for (index=0; index < NUM_ANTS; index++)
    {
    // reset new state selector
    select_new_state = -1;

    // what state is ant in?
    switch(ants[index].varsI[ANT_INDEX_AI_STATE])
          {
           case ANT_WANDERING:      // moving around randomly
               {
               // in this state the ant selects random directions and then
               // walks for some period in that direction, if during the ants
               // walk, it stumbles across some food, then it will remember
               // the rough position of the food in int memory
               // burns 1 unit per cycle 

               // move the ant
               ants[index].x+=ant_movements_x[ants[index].varsI[ANT_INDEX_DIRECTION]];
               ants[index].y+=ant_movements_y[ants[index].varsI[ANT_INDEX_DIRECTION]];

               // test if ant is done with direction and needs a new one
               if (--ants[index].counter_2 < 0)
                  {
                  // set direction
                  ants[index].varsI[ANT_INDEX_DIRECTION] = RAND_RANGE(ANT_ANIM_UP, ANT_ANIM_LEFT);
    
                  // time in this new direction
                  ants[index].counter_2 = RAND_RANGE(10, 100);

                  // start animation
                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);

                  } // end if new direction

               // burn food
               ants[index].varsI[ANT_INDEX_HUNGER_LEVEL]++;

               // update memory with presence of food
               int ant_cell_x = ants[index].x / 30;
               int ant_cell_y = ants[index].y / 30;

               // this updates the i,jth memory cell in ant with info about food
               ants_mem[index].cell[ant_cell_x][ant_cell_y] =
                   ANT_MEMORY_RESIDUAL_RATE*ants_mem[index].cell[ant_cell_x][ant_cell_y] + 
                   (1-ANT_MEMORY_RESIDUAL_RATE)*Food_Near_Ant(ant_cell_x, ant_cell_y);

               // test if we are done with this state and need a new one?
               if (--ants[index].counter_1 < 0)                                        
                  {
                  // select either rest or wander, search will pre-empt with logic following
                  // if hungry, state, probability of state, must sum to 100
                  select_new_state = Select_State_Rand(ANT_WANDERING,     70,            
                                                       ANT_EATING,        0,                
                                                       ANT_RESTING,       30,              
                                                       ANT_SEARCH_FOOD,   0,           
                                                       ANT_COMMUNICATING, 0,         
                                                       ANT_DEAD,          0);                    
                   } // end if

   
               // test for pre-empt into communication mode
               for (int dialog_ant = 0; dialog_ant < NUM_ANTS; dialog_ant++)
                   {

                   // try and talk if this isn't the same ant just talked to and this ant
                   // is either resting or wandering
                   if ( ( (dialog_ant!=index) && (ants[index].varsI[ANT_INDEX_LAST_TALKED_WITH] != dialog_ant) ) &&
                        ((ants[dialog_ant].varsI[ANT_INDEX_AI_STATE] == ANT_RESTING) ||
                         (ants[dialog_ant].varsI[ANT_INDEX_AI_STATE] == ANT_WANDERING)) )
                        {
                        // are they close enough to talk?
                        if ((abs(ants[index].x - ants[dialog_ant].x) < 8)  &&
                            (abs(ants[index].y - ants[dialog_ant].y) < 8) )
                            {
                            // set both ants to communicate mode
                            Set_New_State(ANT_COMMUNICATING,index);              
                            Set_New_State(ANT_COMMUNICATING,dialog_ant);
                      
                            // set communicate partners for chat
                            ants[index].varsI[ANT_INDEX_LAST_TALKED_WITH]      = dialog_ant;
                            ants[dialog_ant].varsI[ANT_INDEX_LAST_TALKED_WITH] = index;

                            break;
                            } // end if
                              
                        } // end if

                   } // end for dialog_ant
 

               // test for pre-empt into search mode if hunger is 75% tolerance
               if (ants[index].varsI[ANT_INDEX_HUNGER_LEVEL] > (0.75*ants[index].varsI[ANT_INDEX_HUNGER_TOLERANCE]) )
                  {
                  select_new_state = ANT_SEARCH_FOOD;
                  } // end if                   

        

               } break;

          case ANT_EATING:         // at a mnm eating it
               {
               // in this state the ant is eating and at rest, the ant will
               // eat from a single mnm until it puts itself to 50% of its hunger tolerance
               // and then stop or if the food runs out it will stop
               // ants eat at a rate of 5 energy units per cycle 
               // burns 1 unit per cycle
         
               // eat the mnm up until its gone or the hunger level drops to 50%
                     
               // decrease food supply
               food[ants[index].varsI[ANT_INDEX_MNM_BEING_DEVOURED]].energy-=BITE_SIZE;

               // transfer to ant and decrease hunger level BITE_SIZE units 
               ants[index].varsI[ANT_INDEX_HUNGER_LEVEL]-=BITE_SIZE;

               // is food supply depleted?
               if (food[ants[index].varsI[ANT_INDEX_MNM_BEING_DEVOURED]].energy < 0)
                  {
                  food[ants[index].varsI[ANT_INDEX_MNM_BEING_DEVOURED]].energy = 0;

                  // transfer to search state or rest a sec
                  select_new_state = Select_State_Rand(ANT_WANDERING,     0,            
                                                       ANT_EATING,        0,                
                                                       ANT_RESTING,       30,              
                                                       ANT_SEARCH_FOOD,   70,           
                                                       ANT_COMMUNICATING, 0,         
                                                       ANT_DEAD,          0);  

                                
                  } // end if

               // test if done eating?, i.e. hunger < 50% tolerance
               else
               if (ants[index].varsI[ANT_INDEX_HUNGER_LEVEL] < 
                   (0.50*ants[index].varsI[ANT_INDEX_HUNGER_TOLERANCE]) )
                  {
                  // switch to another state
                  select_new_state = Select_State_Rand(ANT_WANDERING,     50,            
                                                       ANT_EATING,        0,                
                                                       ANT_RESTING,       30,              
                                                       ANT_SEARCH_FOOD,   20,           
                                                       ANT_COMMUNICATING, 0,         
                                                       ANT_DEAD,          0);    


                  } // end if

               } break;          

          case ANT_RESTING:        // sleeping :)
               {
               // ant is simply resting and burns 1 unit per cycle                 

               // test if we are done with this state and need a new one?
               if (--ants[index].counter_1 < 0)                                        
                  {
                  // select either rest or wander, search will pre-empt with logic following
                  // if hungry, state, probability of state, must sum to 100
                  select_new_state = Select_State_Rand(ANT_WANDERING,     90,            
                                                       ANT_EATING,        0,                
                                                       ANT_RESTING,       10,              
                                                       ANT_SEARCH_FOOD,   0,           
                                                       ANT_COMMUNICATING, 0,         
                                                       ANT_DEAD,          0);                    
                 } // end if

               // burn food
               ants[index].varsI[ANT_INDEX_HUNGER_LEVEL]++;


               // test for pre-empt into search mode if hunger is 50% tolerance
               if (ants[index].varsI[ANT_INDEX_HUNGER_LEVEL] > (ants[index].varsI[ANT_INDEX_HUNGER_TOLERANCE] >> 1) )
                  {
                  select_new_state = ANT_SEARCH_FOOD;
                  } // end if    



               } break;

          case ANT_SEARCH_FOOD:    // hungry and searching for food
               {
               // in this state the ant is looking for food based on its memory
               // if the memory is blank then random walks
               // if the ant gets to a location and cant find any food where its
               // memory found some then the memory for that food location is degraded by 1
               // searching takes 2 units of energy per cycle

#if 0
               ants[index].varsI[ANT_INDEX_AI_STATE]    = ANT_SEARCH_FOOD;

               // start off by scanning for food
               ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = ANT_SEARCH_FOOD_S1_SCAN;

               // initialize targets tp 0
               ants[index].varsI[ANT_INDEX_FOOD_TARGET_X] = 0;
               ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y] = 0;

#endif

               // test substate
               switch(ants[index].varsI[ANT_INDEX_AI_SUBSTATE])
                     {     

                     case ANT_SEARCH_FOOD_S1_SCAN:
                          {
                          // this state is transient and doesn't persist, so
                          // no energy expended by it

                          // scan for a "hot" cell
                          float max_energy = 0;
                          int cell_x = 0, cell_y = 0;
                        
                          for (int index_x = 0; index_x < 16; index_x++)
                              for (int index_y = 0; index_y < 16; index_y++)
                                  // does this cell have more food?
                                  if (ants_mem[index].cell[index_x][index_y] > max_energy)
                                     {
                                     // update new max
                                     max_energy = ants_mem[index].cell[index_x][index_y]; 
                                     cell_x = index_x; cell_y = index_y;

                                     } // end if

                          // test max and see if we have a winner?
                          if (max_energy > 0)
                             {
                             // vector to this cell
                            ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = ANT_SEARCH_FOOD_S3_VECTOR_2CELL;

                            // send to cell center
                            ants[index].varsI[ANT_INDEX_FOOD_TARGET_X] = 30*cell_x+15;
                            ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y] = 30*cell_y+15;                              

                            // set counters to 0
                            ants[index].counter_1 = ants[index].counter_2 = 0;                           

                             } // end if
                           else
                              { 
                              // go into wander mode, no knowledge of food
                              // vector to this cell
                              ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = ANT_SEARCH_FOOD_S2_WANDER;

                              // set counters 
                              ants[index].counter_1 = RAND_RANGE(100,150);
                              ants[index].counter_2 = 0;  

                              } // end else

                          } break;

                     case ANT_SEARCH_FOOD_S2_WANDER:
                          {
                          // wander and look for food, when done wandering go back and scan
                             
                          // burn food
                          ants[index].varsI[ANT_INDEX_HUNGER_LEVEL]+=1; 

                          // move the ant
                          ants[index].x+=ant_movements_x[ants[index].varsI[ANT_INDEX_DIRECTION]];
                          ants[index].y+=ant_movements_y[ants[index].varsI[ANT_INDEX_DIRECTION]];

                          // test if ant is done with direction and needs a new one
                          if (--ants[index].counter_2 < 0)
                             {
                             // set direction
                             ants[index].varsI[ANT_INDEX_DIRECTION] = RAND_RANGE(ANT_ANIM_UP, ANT_ANIM_LEFT);
    
                             // time in this new direction
                             ants[index].counter_2 = RAND_RANGE(10, 100);

                             // start animation
                             Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);

                             } // end if new direction

                            // update memory with presence of food
                            int ant_cell_x = ants[index].x / 30;
                            int ant_cell_y = ants[index].y / 30;

                            // this updates the i,jth memory cell in ant with info about food
                            float food_near_ant = Food_Near_Ant(ant_cell_x, ant_cell_y);
                            
                            ants_mem[index].cell[ant_cell_x][ant_cell_y] =
                                 ANT_MEMORY_RESIDUAL_RATE*ants_mem[index].cell[ant_cell_x][ant_cell_y] + 
                                 (1-ANT_MEMORY_RESIDUAL_RATE)*food_near_ant;

                            // test if we just bumped into some food
                            if (food_near_ant > 0)
                               {
                               // find highest source of food
                               int food_x = -1;
                               int food_y = -1;

                               // find the highest food source in cell
                               int food_id = Max_Food_In_Cell(ant_cell_x, ant_cell_y, &food_x, &food_y);
                                                         
                               // pre-empt into vector 2 food
                               ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = 
                                                                   ANT_SEARCH_FOOD_S4_VECTOR_2FOOD;

                               // send to exact position
                               ants[index].varsI[ANT_INDEX_FOOD_TARGET_X] = food_x;
                               ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y] = food_y;
                                                                  
                               // set target id of food
                               ants[index].varsI[ANT_INDEX_FOOD_TARGET_ID] = food_id;

                               // set counters to 0
                               ants[index].counter_1 = ants[index].counter_2 = 0;

                               } // end if
                            
                            // test if we are done with this state and need to go back to scan
                            else
                            if (--ants[index].counter_1 < 0)                                        
                               {
                               // go back to scan state                            
                               ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = ANT_SEARCH_FOOD_S1_SCAN;                            

                               } // end if
             

                          } break;

                     case ANT_SEARCH_FOOD_S3_VECTOR_2CELL: 
                          {
                          // this substate vectors the ant to the center of the cell, once
                          // there the ant "looks" to see if there actually is any food, if so
                          // vectors to it, else goes back and scans

                          // burn food
                          ants[index].varsI[ANT_INDEX_HUNGER_LEVEL]+=2;

                          // pick direction
                          int dx = ants[index].varsI[ANT_INDEX_FOOD_TARGET_X] - ants[index].x;
                          int dy = ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y] - ants[index].y;
                   
                         // pick maxium delta to move in
                         if (abs(dx) >= abs(dy))
                            {
                            // x dominate
                            if (dx > 0)
                               {
                               // move right 
                               if (ants[index].varsI[ANT_INDEX_DIRECTION] != ANT_ANIM_RIGHT)
                                  {
                                  // start animation right
                                  ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_RIGHT;
                                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
                                  } // end if
                               ants[index].x+=ant_movements_x[ants[index].varsI[ANT_INDEX_DIRECTION]];
                               } // end if                             
                            else
                            if (dx < 0)
                               {
                               // move left
                               if (ants[index].varsI[ANT_INDEX_DIRECTION] != ANT_ANIM_LEFT)
                                  {
                                  // start animation left
                                  ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_LEFT;
                                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
                                  } // end if
                                  ants[index].x+=ant_movements_x[ants[index].varsI[ANT_INDEX_DIRECTION]];

                               } // end else

                            } // end if x
                         else
                            {
                            // y dominate
                            if (dy > 0)
                               {
                               // move down
                               if (ants[index].varsI[ANT_INDEX_DIRECTION] != ANT_ANIM_DOWN)
                                  {
                                  // start animation down
                                  ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_DOWN;
                                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
                                  } // end if

                               ants[index].y+=ant_movements_y[ants[index].varsI[ANT_INDEX_DIRECTION]];
                               } // end if                             
                            else
                            if (dy < 0)
                               {
                               // move up
                               if (ants[index].varsI[ANT_INDEX_DIRECTION] != ANT_ANIM_UP)
                                  {
                                  // start animation down
                                  ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_UP;
                                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
                                  } // end if

                                 ants[index].y+=ant_movements_y[ants[index].varsI[ANT_INDEX_DIRECTION]];
                               } // end else

                            } // end else

                          // update memory image ????

                            // update memory with presence of food
                            int ant_cell_x = ants[index].x / 30;
                            int ant_cell_y = ants[index].y / 30;

                            // this updates the i,jth memory cell in ant with info about food
                            float food_near_ant = Food_Near_Ant(ant_cell_x, ant_cell_y);
                            
                            ants_mem[index].cell[ant_cell_x][ant_cell_y] =
                                 ANT_MEMORY_RESIDUAL_RATE*ants_mem[index].cell[ant_cell_x][ant_cell_y] + 
                                 (1-ANT_MEMORY_RESIDUAL_RATE)*food_near_ant;

                          // now test if target reached
                          if (abs(ants[index].x - ants[index].varsI[ANT_INDEX_FOOD_TARGET_X]) < 4 &&
                              abs(ants[index].y - ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y]) < 4)
                             {
                             // center of cell reached, now find the biggest piece of food and
                             // vector to it, if none exist then go back to scan

                            // compute cell position
                            int ant_cell_x = ants[index].x / 30;
                            int ant_cell_y = ants[index].y / 30;

                            // this updates the i,jth memory cell in ant with info about food
                            float food_near_ant = Food_Near_Ant(ant_cell_x, ant_cell_y);
                            
                            // test if we just bumped into some food
                            if (food_near_ant > 0)
                               {
                               // find highest source of food
                               int food_x = -1;
                               int food_y = -1;

                               // find the highest food source in cell
                               int food_id = Max_Food_In_Cell(ant_cell_x, ant_cell_y, &food_x, &food_y);
                                                         
                               // pre-empt into vector 2 food
                               ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = 
                                                      ANT_SEARCH_FOOD_S4_VECTOR_2FOOD;

                               // send to exact position
                               ants[index].varsI[ANT_INDEX_FOOD_TARGET_X] = food_x;
                               ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y] = food_y;
                                                                  
                               // set target id of food
                               ants[index].varsI[ANT_INDEX_FOOD_TARGET_ID] = food_id;

                               // set counters to 0
                               ants[index].counter_1 = ants[index].counter_2 = 0;

                               } // end if
                            else
                               {
                               // set mode to scan
                               ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = ANT_SEARCH_FOOD_S1_SCAN;                            
                               } // end else
 
                             } // end if

                          } break;


                     case ANT_SEARCH_FOOD_S4_VECTOR_2FOOD:
                          {
                          // this substate vectors the ant to the exact x,y of the food 
                          // once there, tests to see if there is food there, if so eats it,
                          // else go to scan
                             
                          // burn food
                          ants[index].varsI[ANT_INDEX_HUNGER_LEVEL]+=2;

                          // pick direction
                          int dx = ants[index].varsI[ANT_INDEX_FOOD_TARGET_X] - ants[index].x;
                          int dy = ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y] - ants[index].y;
                   
                         // pick maxium delta to move in
                         if (abs(dx) >= abs(dy))
                            {
                            // x dominate
                            if (dx > 0)
                               {
                               // move right 
                               if (ants[index].varsI[ANT_INDEX_DIRECTION] != ANT_ANIM_RIGHT)
                                  {
                                  // start animation right
                                  ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_RIGHT;
                                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
                                  } // end if

                               ants[index].x+=ant_movements_x[ants[index].varsI[ANT_INDEX_DIRECTION]];

                               } // end if                             
                            else
                            if (dx < 0)
                               {
                               // move left
                               if (ants[index].varsI[ANT_INDEX_DIRECTION] != ANT_ANIM_LEFT)
                                  {
                                  // start animation left
                                  ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_LEFT;
                                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
                                  } // end if

                                ants[index].x+=ant_movements_x[ants[index].varsI[ANT_INDEX_DIRECTION]];

                               } // end else

                            } // end if x
                         else
                            {
                            // y dominate
                            if (dy > 0)
                               {
                               // move down
                               if (ants[index].varsI[ANT_INDEX_DIRECTION] != ANT_ANIM_DOWN)
                                  {
                                  // start animation down
                                  ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_DOWN;
                                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
                                  } // end if

                               ants[index].y+=ant_movements_y[ants[index].varsI[ANT_INDEX_DIRECTION]];

                               } // end if                             
                            else
                             if (dy < 0)   
                               {
                               // move up
                               if (ants[index].varsI[ANT_INDEX_DIRECTION] != ANT_ANIM_UP)
                                  {
                                  // start animation down
                                  ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_UP;
                                  Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);
                                  } // end if

                                ants[index].y+=ant_movements_y[ants[index].varsI[ANT_INDEX_DIRECTION]];

                               } // end else

                            } // end else

                          // update memory image ????

                            // update memory with presence of food
                            int ant_cell_x = ants[index].x / 30;
                            int ant_cell_y = ants[index].y / 30;

                            // this updates the i,jth memory cell in ant with info about food
                            float food_near_ant = Food_Near_Ant(ant_cell_x, ant_cell_y);
                            
                            ants_mem[index].cell[ant_cell_x][ant_cell_y] =
                                 ANT_MEMORY_RESIDUAL_RATE*ants_mem[index].cell[ant_cell_x][ant_cell_y] + 
                                 (1-ANT_MEMORY_RESIDUAL_RATE)*food_near_ant;


                          // now test if target reached
                          if (abs(ants[index].x - ants[index].varsI[ANT_INDEX_FOOD_TARGET_X]) < 4 &&
                              abs(ants[index].y - ants[index].varsI[ANT_INDEX_FOOD_TARGET_Y]) < 4)
                             {
                             // food reached, now find the biggest piece of food and
                             // vector to it, if none exist then go back to scan

                             // is there any food left?
                             if (ants[index].varsI[ANT_INDEX_FOOD_TARGET_ID] > 0)
                                {
                                // thank god!
                                Set_New_State(ANT_EATING, index, ants[index].varsI[ANT_INDEX_FOOD_TARGET_ID],0);
                                } // end if
                              else
                                 {
                                 // go back to scan
                                 ants[index].varsI[ANT_INDEX_AI_SUBSTATE] = ANT_SEARCH_FOOD_S1_SCAN;                            

                                 } // end if

                             } // end if
                        
                          } break;

                     case ANT_SEARCH_FOOD_S5: break;            
                     case ANT_SEARCH_FOOD_S6: break;           
                     case ANT_SEARCH_FOOD_S7: break;          

                     default: break;
                     } // end switch

               } break;

          case ANT_COMMUNICATING:  // talking to another ant  
               {
               // the ants sit and talk for a little bit, each ant picks
               // 16 memory cells from the matrix of 16x16 and then averages
               // the info, that is, they exchange information
               // this takes 1 unit of energy

               // time to exchange memory?
               if (++ants[index].counter_2 >= 3)
                  {
                  // reset counter
                  ants[index].counter_2 = 0;
                  
                  // select cell to exchange and exchange memory imprints
                  int memx = rand()%16;
                  int memy = rand()%16;

                  // sum with partners experience
                  ants_mem[index].cell[memx][memy] += 
                    0.5*ants_mem[ants[index].varsI[ANT_INDEX_LAST_TALKED_WITH]].cell[memx][memy];
     
                  // illuminate green to show communication
                  ants_mem[index].ilayer[memx][memy] = 250;
             
                  } // end if

               // test if we are done with this state and need a new one?
               if (--ants[index].counter_1 < 0 || 
                ants[ants[index].varsI[ANT_INDEX_LAST_TALKED_WITH]].varsI[ANT_INDEX_AI_STATE]!=ANT_COMMUNICATING)                                        
                  {
                  // select either rest or wander, search will pre-empt with logic following
                  // if hungry, state, probability of state, must sum to 100
                  select_new_state = Select_State_Rand(ANT_WANDERING,     90,            
                                                       ANT_EATING,        0,                
                                                       ANT_RESTING,       10,              
                                                       ANT_SEARCH_FOOD,   0,           
                                                       ANT_COMMUNICATING, 0,         
                                                       ANT_DEAD,          0);                    
                   } // end if


               // burn food
               ants[index].varsI[ANT_INDEX_HUNGER_LEVEL]++;

               // test for pre-empt into search mode if hunger is 50% tolerance
               if (ants[index].varsI[ANT_INDEX_HUNGER_LEVEL] > (ants[index].varsI[ANT_INDEX_HUNGER_TOLERANCE] >> 1) )
                  {
                  select_new_state = ANT_SEARCH_FOOD;
                  } // end if    

               } break;

          case ANT_DEAD:           // this guy is dead, got too hungry
               {
               // the any just sits there and is worm food, no enery expended          

 
               } break;

          default: break;

          } // end switch

    // was there a forced state transition?
if (ants[index].varsI[ANT_INDEX_AI_STATE]!=ANT_DEAD && 
   (ants[index].varsI[ANT_INDEX_HUNGER_LEVEL] >= ants[index].varsI[ANT_INDEX_HUNGER_TOLERANCE]))
   {
   // dead ant
   select_new_state = ANT_DEAD;

   } // end if

///////////////////////////////////////////////////////////////////////////

        // add a little forgetfulness, select a memory at random and weaken it
        int memx = rand()%16;
        int memy = rand()%16;

       ants_mem[index].ilayer[memx][memy] = 249; // illuminate red to show degradation

        if (ants_mem[index].cell[memx][memy] > 5)
           {
           ants_mem[index].cell[memx][memy]-=rand()%5;
           } // end if

//////////////////////////////////////////////////////////////////////////

// set new state if needed 
    if (select_new_state >= 0)
       {
       // set the state
       Set_New_State(select_new_state, index);
     
       } // end if
////////////////////////////////////////////////////

   // clear the memory matrix
   if (clear_ilayer >= 5)
      memset(ants_mem[index].ilayer,0, 1024);


   // this point is where the "physics" system need to operate
   // have any of the state changes or motions caused a physical problem
   // in the universe, we could do it in each state, but this is simpler

   // test for out of bounds
   if (ants[index].x > 472)
       {
       // bounce back a bit
       ants[index].x-=4;

       // turn to new heading
       switch(rand()%3)
             {
             case 0: ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_LEFT; break;
             case 1: ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_UP; break;
             case 2: ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_DOWN; break;
             default: break;
             } // end switch
     
       // add a little time to counters     
       ants[index].counter_2 += RAND_RANGE(30, 60);
       ants[index].counter_1 += RAND_RANGE(15, 30);

       // start animation
       Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);

       } // end if
else
if (ants[index].x <= 16)
    {
    // bounce back a bit
    ants[index].x+=4;

    // turn to new heading
    switch(rand()%3)
          {
          case 0: ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_RIGHT; break;
          case 1: ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_UP; break;
          case 2: ants[index].varsI[ANT_INDEX_DIRECTION] = ANT_ANIM_DOWN; break;
          default: break;
          } // end switch
     
    // add a little time to counter     
    ants[index].counter_2 += RAND_RANGE(30, 60);
    ants[index].counter_1 += RAND_RANGE(15, 30);

    // start animation
    Set_Animation_BOB(&ants[index], ants[index].varsI[ANT_INDEX_DIRECTION]);

    } // end if

if (ants[index].y < 0)
    ants[index].y = screen_height;
else
if (ants[index].y > screen_height)
    ants[index].y = 0;

///////////////////////////////////////////////////////////////////////////////////

    } // end for index

// update global memory clear counter
if (++clear_ilayer > 5)
    clear_ilayer= 0;



} // end Move_Ants

///////////////////////////////////////////////////////////////////

void Draw_Food(void)
{
// draw the food

int index;

for (index=0; index<NUM_MNMS; index++)
    {
    if (food[index].energy > 0)
       {
       // set position of mnm
       Set_Pos_BOB(&mnm, food[index].x-4, food[index].y-4);
       Draw_BOB16(&mnm, lpddsback);
       } // end if

    } // end for index

} // end Draw_Food

///////////////////////////////////////////////////////////////////

void Draw_Info(void)
{
// this function draws the state information for 8 ants

static char *state_to_text[6] = {"Wander", "Eating", "Resting", "Srchfood", "Talking", "Dead"};         

static int intensity_to_index[16] = {0,65,94,151,166,175,203,208,221,227,233,236,239,242,245,245};

#define DELTA_LINE 14

for (int index=0; index < NUM_ANTS; index++)
    {
    int px = 480;
    int py = 8+index * 60;


    // only show infor for first 8 ants
    if (index < 8 )
    {
    // print name of ant and state
    sprintf(buffer,"Ant%d=%s,%d", index, state_to_text[ants[index].varsI[ANT_INDEX_AI_STATE]], ants[index].varsI[ANT_INDEX_AI_SUBSTATE]);    
    Draw_Text_GDI(buffer,px, py, RGB(0,255,0),lpddsback);


    // print counters
    sprintf(buffer, "C1:%d,C2:%d", ants[index].counter_1, ants[index].counter_2);
    Draw_Text_GDI(buffer,px, py+DELTA_LINE, RGB(0,255,0),lpddsback);

    // print hunger level and tolerance
    sprintf(buffer,"Hungr:%d,Tol:%d", ants[index].varsI[ANT_INDEX_HUNGER_LEVEL],
                                      ants[index].varsI[ANT_INDEX_HUNGER_TOLERANCE]);

    Draw_Text_GDI(buffer,px, py+2*DELTA_LINE, RGB(0,255,0),lpddsback);

    // draw memory, the brighter the green dot, the stronger the memory
    Draw_Rectangle(px+124, py, px+124+18, py+18, 0, lpddsback);

    // dont draw memory if ant dead
    if (ants[index].varsI[ANT_INDEX_AI_STATE]!=ANT_DEAD)
    {
    DDraw_Lock_Back_Surface();
    for (int x = 0; x<16; x++)
        for (int y=0; y<16; y++)
            {
            // get memory and convert into pixel value to disply
            // dark means low recall, bright white means strong memory of food
            float mem_strength = ants_mem[index].cell[x][y];

            // each mnm is worth on average 500, so if a cell has a 2500 then we know there is 
            // a lot of food, use this as the highest strength
            if (mem_strength > 2500)
               mem_strength = 2500;

            // normalize and scale to color range
            int color_intensity = (int)((mem_strength/2500)*15);
            Draw_Pixel16(px+124+x+1,py+y+1, RGB16Bit((color_intensity*16),0,0), back_buffer, back_lpitch);
      
            int active_pixel = ants_mem[index].ilayer[x][y];

            if (active_pixel)
               // scan input layer to see where degradation and communication have taken place
               Draw_Pixel16(px+124+x+1,py+y+1, RGB16Bit(0,active_pixel,0), back_buffer, back_lpitch);
            } // end for

        DDraw_Unlock_Back_Surface();
       } // end if
       else
           // draw flat line :)
           {
           DDraw_Lock_Back_Surface();
           HLine16(px+124,px+124+18, py+9, RGB16Bit(255,0,0), back_buffer, back_lpitch);
           DDraw_Unlock_Back_Surface();
           } // end else

       } // end if < 8

    } // end for index

} // end Draw_Info


////////////////////////////////////////////////////////////////////

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

// move ants and perform ai
Move_Ants();

// draw the food
Draw_Food();

// draw ants and animate imagery
Draw_Ants(); 

// draw info
Draw_Info();


sprintf(buffer,"(16-Bit Version) Artificial Intelligence Memory Demo. Press <ESC> to Exit.");
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

    } // end if

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////