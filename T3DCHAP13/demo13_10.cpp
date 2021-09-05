// DEMO13_10.CPP - particle demo
// to compile make sure to include DDRAW.LIB, DSOUND.LIB,
// DINPUT.LIB, WINMM.LIB, and of course the T3DLIB files

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

#define WINDOW_WIDTH    320  // size of window
#define WINDOW_HEIGHT   240

// defines for polygon cannon
#define CANNON_X0       39  // position of tip of cannon
#define CANNON_Y0       372
#define NUM_PROJECTILES 16  // number of projectiles

// defines for particle system
#define PARTICLE_STATE_DEAD               0
#define PARTICLE_STATE_ALIVE              1

// types of particles
#define PARTICLE_TYPE_FLICKER             0
#define PARTICLE_TYPE_FADE                1 

// color of particle
#define PARTICLE_COLOR_RED                0
#define PARTICLE_COLOR_GREEN              1
#define PARTICLE_COLOR_BLUE               2
#define PARTICLE_COLOR_WHITE              3

#define MAX_PARTICLES                     256

// color ranges
#define COLOR_RED_START                   32
#define COLOR_RED_END                     47

#define COLOR_GREEN_START                 96
#define COLOR_GREEN_END                   111

#define COLOR_BLUE_START                  144
#define COLOR_BLUE_END                    159

#define COLOR_WHITE_START                 16
#define COLOR_WHITE_END                   31


// MACROS ///////////////////////////////////////////////

#define RAND_RANGE(x,y) ( (x) + (rand()%((y)-(x)+1)))

// TYPES ///////////////////////////////////////////////

typedef struct PROJ_TYP
{
int state;      // state 0 off, 1 on
float x,y;      // position
float xv, yv;   // velocity
int detonate;   // tracks when the projectile detonates

} PROJECTILE, *PROJECTILE_PTR;

// a single particle
typedef struct PARTICLE_TYP
        {
        int state;           // state of the particle
        int type;            // type of particle effect
        float x,y;           // world position of particle
        float xv,yv;         // velocity of particle
        int curr_color;      // the current rendering color of particle
        int start_color;     // the start color or range effect
        int end_color;       // the ending color of range effect
        int counter;         // general state transition timer
        int max_count;       // max value for counter

        } PARTICLE, *PARTICLE_PTR;


// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// missile interface
void Init_Projectiles(void);
void Move_Projectiles(void);
void Draw_Projectiles(void);
void Fire_Projectile(int angle, float vel);

void Init_Reset_Particles(void);
void Draw_Particles(void);
void Move_Particles(void);
void Start_Particle(int type, int color, int count, int x, int y, int xv, int yv);
void Start_Particle_Explosion(int type, int color, int count, 
                              int x, int y, int xv, int yv, int num_particles);


void Start_Particle_Ring(int type, int color, int count, 
                              int x, int y, int xv, int yv, int num_particles);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle   = NULL; // save the window handle
HINSTANCE main_instance   = NULL; // save the instance
char buffer[256];                 // used to print text

BITMAP_IMAGE background_bmp;      // holds the background

int cannon_ids[8]; // sound ids for cannon
int explosion_ids[8]; // explosion ids

POLYGON2D cannon; // the ship

PROJECTILE missiles[NUM_PROJECTILES]; // array of missiles

float gravity_force =  0.2;  // gravity
float wind_force    = -0.01; // wind resistance

float particle_wind    = 0;    // assume it operates in the X direction
float particle_gravity = .02; // assume it operates in the Y direction

PARTICLE particles[MAX_PARTICLES]; // the particles for the particle engine

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
						  "Collision Demo",	 // title
						  WS_POPUP | WS_VISIBLE,
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

int index; // looping varsIable

char filename[80]; // used to build up filenames

// seed random number generate
srand(Start_Clock());

// start up DirectDraw (replace the parms as you desire)
DDraw_Init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

// load background image
Load_Bitmap_File(&bitmap8bit, "PARTICLEGRID.BMP");
Create_Bitmap(&background_bmp,0,0,640,480);
Load_Image_Bitmap(&background_bmp, &bitmap8bit,0,0,BITMAP_EXTRACT_MODE_ABS);
Set_Palette(bitmap8bit.palette);
Unload_Bitmap_File(&bitmap8bit);


// hide the mouse
ShowCursor(FALSE);

// initialize directinput
DInput_Init();

// acquire the keyboard only
DInput_Init_Keyboard();

// initilize DirectSound
DSound_Init();

// initialize particles
Init_Reset_Particles();

// load background sounds
cannon_ids[0] = DSound_Load_WAV("CANNON.WAV");

// clone sounds
for (index=1; index < 8; index++)
    cannon_ids[index] = DSound_Replicate_Sound(cannon_ids[0]);

// load background sounds
explosion_ids[0] = DSound_Load_WAV("EXP1.WAV");

// clone sounds
for (index=1; index < 8; index++)
    explosion_ids[index] = DSound_Replicate_Sound(explosion_ids[0]);


// define points of cannon
VERTEX2DF cannon_vertices[4] =  { 0,-2, 30,0, 30,2, 0,2, };

// initialize ship
cannon.state       = 1;   // turn it on
cannon.num_verts   = 4;  
cannon.x0          = CANNON_X0; // position it
cannon.y0          = CANNON_Y0;
cannon.xv          = 0;
cannon.yv          = 0;
cannon.color       = 95; // green
cannon.vlist       = new VERTEX2DF [cannon.num_verts];
 
for (index = 0; index < cannon.num_verts; index++)
    cannon.vlist[index] = cannon_vertices[index];

// build the 360 degree look ups
Build_Sin_Cos_Tables();

// initialize the missiles
Init_Projectiles();

// set clipping rectangle to screen extents so objects dont
// mess up at edges
RECT screen_rect = {0,0,screen_width,screen_height};
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

// set clipping region
min_clip_x = 0;
max_clip_x = screen_width - 1;
min_clip_y = 0;
max_clip_y = screen_height - 1;

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// shut everything down

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

////////////////////////////////////////////////////////////

void Init_Reset_Particles(void)
{
// this function serves as both an init and reset for the particles

// loop thru and reset all the particles to dead
for (int index=0; index<MAX_PARTICLES; index++)
    {
    particles[index].state = PARTICLE_STATE_DEAD;
    particles[index].type  = PARTICLE_TYPE_FADE;
    particles[index].x     = 0;
    particles[index].y     = 0;
    particles[index].xv    = 0;
    particles[index].yv    = 0;
    particles[index].start_color = 0;
    particles[index].end_color   = 0;
    particles[index].curr_color  = 0;
    particles[index].counter     = 0;
    particles[index].max_count   = 0;
    } // end if

} // end Init_Reset_Particles

/////////////////////////////////////////////////////////////////////////

void Start_Particle(int type, int color, int count, int x, int y, int xv, int yv)
{
// this function starts a single particle

int pindex = -1; // index of particle

// first find open particle
for (int index=0; index < MAX_PARTICLES; index++)
    if (particles[index].state == PARTICLE_STATE_DEAD)
       {
       // set index
       pindex = index; 
       break;
       } // end if    

// did we find one
if (pindex==-1)
   return;

// set general state info
particles[pindex].state = PARTICLE_STATE_ALIVE;
particles[pindex].type  = type;
particles[pindex].x     = x;
particles[pindex].y     = y;
particles[pindex].xv    = xv;
particles[pindex].yv    = yv;
particles[pindex].counter     = 0;
particles[pindex].max_count   = count;     

// set color ranges, always the same
   switch(color)
         {
         case PARTICLE_COLOR_RED:
              {
              particles[pindex].start_color = COLOR_RED_START;
              particles[pindex].end_color   = COLOR_RED_END;
              } break;

         case PARTICLE_COLOR_GREEN:
              {
              particles[pindex].start_color = COLOR_GREEN_START;
              particles[pindex].end_color   = COLOR_GREEN_END;
              } break;

         case PARTICLE_COLOR_BLUE:
              {
              particles[pindex].start_color = COLOR_BLUE_START;
              particles[pindex].end_color   = COLOR_BLUE_END;
              } break;

         case PARTICLE_COLOR_WHITE:
              {
              particles[pindex].start_color = COLOR_WHITE_START;
              particles[pindex].end_color   = COLOR_WHITE_END;
              } break;

         break;

         } // end switch

// what type of particle is being requested
if (type == PARTICLE_TYPE_FLICKER)
   {
    // set current color
    particles[index].curr_color  = RAND_RANGE(particles[index].start_color, particles[index].end_color);

   } // end if
else
   {
   // particle is fade type
   // set current color
   particles[index].curr_color  = particles[index].start_color;
   } // end if

} // end Start_Particle

////////////////////////////////////////////////////////////////////////////////

void Start_Particle_Explosion(int type, int color, int count, 
                              int x, int y, int xv, int yv, int num_particles)
{
// this function starts a particle explosion at the given position and velocity

while(--num_particles >=0)
    {
    // compute random trajectory angle
    int ang = rand()%360;

    // compute random trajectory velocity
    float vel = 2+rand()%4;

    Start_Particle(type,color,count,
                   x+RAND_RANGE(-4,4),y+RAND_RANGE(-4,4), 
                   xv+cos_look[ang]*vel, yv+sin_look[ang]*vel);        

    } // end while

} // end Start_Particle_Explosion

////////////////////////////////////////////////////////////////////////////////

void Start_Particle_Ring(int type, int color, int count, 
                              int x, int y, int xv, int yv, int num_particles)
{
// this function starts a particle explosion at the given position and velocity
// note the use of look up tables for sin,cos

// compute random velocity on outside of loop
float vel = 2+rand()%4;

while(--num_particles >=0)
    {
    // compute random trajectory angle
    int ang = rand()%360;

    // start the particle
    Start_Particle(type,color,count,
                   x,y, 
                   xv+cos_look[ang]*vel, 
                   yv+sin_look[ang]*vel);        

    } // end while

} // end Start_Particle_Ring

/////////////////////////////////////////////////////////////////////////////////

void Draw_Particles(void)
{
// this function draws all the particles

// lock back surface
DDraw_Lock_Back_Surface();

for (int index=0; index<MAX_PARTICLES; index++)
    {
    // test if particle is alive
    if (particles[index].state==PARTICLE_STATE_ALIVE)
       {
       // render the particle, perform world to screen transform
       int x = particles[index].x;
       int y = particles[index].y;

       // test for clip
       if (x >= SCREEN_WIDTH || x < 0 || y >= SCREEN_HEIGHT || y < 0)
          continue;

       // draw the pixel
       Draw_Pixel(x,y,particles[index].curr_color, back_buffer, back_lpitch);
    
      } // end if

    } // end for index

// unlock the secondary surface
DDraw_Unlock_Back_Surface(); 

} // end Draw_Particles

////////////////////////////////////////////////////////////////////

void Process_Particles(void)
{
// this function moves and animates all particles

for (int index=0; index<MAX_PARTICLES; index++)
    {
    // test if this particle is alive
    if (particles[index].state == PARTICLE_STATE_ALIVE)
       {
       // translate particle
       particles[index].x+=particles[index].xv;
       particles[index].y+=particles[index].yv;

       // update velocity based on gravity and wind
       particles[index].xv+=particle_wind;
       particles[index].yv+=particle_gravity;

       // now based on type of particle perform proper animation
       if (particles[index].type==PARTICLE_TYPE_FLICKER)
          {
          // simply choose a color in the color range and assign it to the current color
          particles[index].curr_color = RAND_RANGE(particles[index].start_color, particles[index].end_color);

          // now update counter
          if (++particles[index].counter >= particles[index].max_count)
             {
             // kill the particle
             particles[index].state = PARTICLE_STATE_DEAD;             

             } // end if

          } // end if
      else
          {
          // must be a fade, be careful!
          // test if it's time to update color
          if (++particles[index].counter >= particles[index].max_count)
             {
              // reset counter
              particles[index].counter = 0;

             // update color
             if (++particles[index].curr_color>particles[index].end_color)
                {
                // transition is complete, terminate particle
                particles[index].state = PARTICLE_STATE_DEAD;  
 
                } // end if

             } // end if

          } // end else
             
       } // end if 

    } // end for index

} // end Process_Particles

////////////////////////////////////////////////////////////////////////////////////

void Cannon_Sound(void)
{        
// this functions hunts for an open handle to play a cannon sound

// start a hit sound
for (int sound_index=0; sound_index < 8; sound_index++)
    {
    // test if this sound is playing
    if (DSound_Status_Sound(cannon_ids[sound_index])==0)
       {
       DSound_Play(cannon_ids[sound_index]);
       break;
       } // end if

     } // end for 

} // end Cannon_Sound

//////////////////////////////////////////////////////////////////////////////////////////

void Explosion_Sound(void)
{        
// this functions hunts for an open handle to play a explosion sound

// start a hit sound
for (int sound_index=0; sound_index < 8; sound_index++)
    {
    // test if this sound is playing
    if (DSound_Status_Sound(explosion_ids[sound_index])==0)
       {
       DSound_Play(explosion_ids[sound_index]);
       break;
       } // end if

     } // end for 

} // end Explosion_Sound


///////////////////////////////////////////////////////////

void Init_Projectiles(void)
{
// this function initializes the projectiles
memset(missiles, 0, sizeof(PROJECTILE)*NUM_PROJECTILES);

} // end Init_Projectiles

/////////////////////////////////////////////////////////////

void Move_Projectiles(void)
{
// this function moves all the projectiles and does the physics model
for (int index=0; index<NUM_PROJECTILES; index++)
    {
    if (missiles[index].state==1)
       {
        // translate
        missiles[index].x+=missiles[index].xv;
        missiles[index].y+=missiles[index].yv;

        // apply forces
        missiles[index].xv+=wind_force;
        missiles[index].yv+=gravity_force;

        // update detonatation counter
        if (--missiles[index].detonate <= 0)
           {
           // select a normal or ring

           if (RAND_RANGE(0,3) == 0)
           {
           // start a particle explosion
           Start_Particle_Ring(PARTICLE_TYPE_FADE, PARTICLE_COLOR_RED+rand()%4, RAND_RANGE(2,5),
                              missiles[index].x, missiles[index].y, 
                              0, 0,RAND_RANGE(75,100));
           } // end if
            else
            {
             Start_Particle_Explosion(PARTICLE_TYPE_FADE, PARTICLE_COLOR_RED+rand()%4, RAND_RANGE(2,5),
                              missiles[index].x, missiles[index].y, 
                              0, 0,RAND_RANGE(20,50));
             } // end if


           // make some noise
           Explosion_Sound();

           // kill the missile
           missiles[index].state = 0;

           } // end if

        // test for off screen
        else
        if (missiles[index].x >= screen_width || 
            missiles[index].y >=screen_height || 
            missiles[index].y < 0) 
           {
           // kill the missile
           missiles[index].state = 0;
           } // end if

        } // end if on
 
    } // end for index

} // end Move_Projectiles

/////////////////////////////////////////////////////////////

void Draw_Projectiles(void)
{
// this function draws all the projectiles 
for (int index=0; index < NUM_PROJECTILES; index++)
    {
    // is this one alive?
    if (missiles[index].state==1)
       {
       Draw_Rectangle(missiles[index].x-1,missiles[index].y-1, 
                      missiles[index].x+1,missiles[index].y+1,
                      95, lpddsback);
       } // end if

    } // end for index

} // end Draw_Projectiles

/////////////////////////////////////////////////////////////

void Fire_Projectile(int angle, float vel)
{
// this function starts a projectile with the given angle and velocity 
// at the tip of the cannon
for (int index=0; index < NUM_PROJECTILES; index++)
    {
    // find an open projectile
    if (missiles[index].state==0)
       {
       // set this missile in motion at the head of cannon with the proper angle
       missiles[index].x = cannon.vlist[1].x+cannon.x0;
       missiles[index].y = cannon.vlist[1].y+cannon.y0;

       // compute velocity vector based on angle
       missiles[index].xv = vel*cos_look[angle];
       missiles[index].yv = -vel*sin_look[angle];

       // set detonation time
       missiles[index].detonate = RAND_RANGE(30,40);
 
       // mark as active
       missiles[index].state = 1;

       // make sound
       Cannon_Sound();

       // bail
       break;

       } // end if

    } // end for index

} // end Fire_Projectile

////////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var

static int   curr_angle = 0; // current angle of elevation from horizon
static float curr_vel   = 10; // current velocity of projectile

// start the timing clock
Start_Clock();

// clear the drawing surface
//DDraw_Fill_Surface(lpddsback, 0);

// lock back buffer and copy background into it
DDraw_Lock_Back_Surface();

// draw background
Draw_Bitmap(&background_bmp, back_buffer, back_lpitch,0);

// do the graphics
Draw_Polygon2D(&cannon, back_buffer, back_lpitch);

// unlock back surface
DDraw_Unlock_Back_Surface();

// read keyboard
DInput_Read_Keyboard();

// test for rotate
if ((curr_angle < 90) && keyboard_state[DIK_UP]) // rotate left
   {
   Rotate_Polygon2D_Mat(&cannon, -5);
   curr_angle+=5;
   } // end if
else
if ((curr_angle > 0) &&keyboard_state[DIK_DOWN]) // rotate right
   {
   Rotate_Polygon2D_Mat(&cannon, 5);
   curr_angle-=5;
   } // end if

// test for projectile velocity
if (keyboard_state[DIK_RIGHT])  
   { 
   if (curr_vel < 30) curr_vel+=0.1;
   } // end if
else
if (keyboard_state[DIK_LEFT]) 
   { 
   if (curr_vel > 0) curr_vel-=0.1;
   } // end if

// test for wind force
if (keyboard_state[DIK_W])  
   { 
   if (particle_wind < 2) particle_wind+=0.01;
   } // end if
else
if (keyboard_state[DIK_E]) 
   { 
   if (particle_wind > -2) particle_wind-=0.01;
   } // end if

// test for gravity force
if (keyboard_state[DIK_G])  
   { 
   if (particle_gravity < 5) particle_gravity+=0.01;
   } // end if
else
if (keyboard_state[DIK_B]) 
   { 
   if (particle_gravity > -5) particle_gravity-=0.01;
   } // end if

// test for fire!
if (keyboard_state[DIK_LCONTROL]) 
   { 
   Fire_Projectile(curr_angle, curr_vel);

   } // end fire

// move all the projectiles
Move_Projectiles();

// move particles
Process_Particles();

// draw the projectiles
Draw_Projectiles();

// draw the particles
Draw_Particles();

// draw the title
Draw_Text_GDI("Particle System DEMO, Press <ESC> to Exit.",10, 10,RGB(0,255,0), lpddsback);
Draw_Text_GDI("<RIGHT>, <LEFT> to adjust velocity, <UP>, <DOWN> to adjust angle",10, 25, RGB(255,255,255), lpddsback);
Draw_Text_GDI("<G>, <B> adjusts particle gravity, <W>, <E> adjusts particle wind, <CTRL> to fire.",10, 40, RGB(255,255,255), lpddsback);

sprintf(buffer, "Cannon: Ang=%d, Vel=%f", curr_angle, curr_vel);
Draw_Text_GDI(buffer,10, 60, RGB(255,255,255), lpddsback);

sprintf(buffer, "Particle: Wind force=%f, Gravity Force=%f", particle_wind, particle_gravity);
Draw_Text_GDI(buffer,10, 75, RGB(255,255,255), lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30 fps = 1/30sec = 33 ms
Wait_Clock(33);

// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || keyboard_state[DIK_ESCAPE])
    {
    PostMessage(main_window_handle, WM_DESTROY,0,0);

    // stop all sounds
    DSound_Stop_All_Sounds();

    // do a screen transition
    Screen_Transitions(SCREEN_DARKNESS,NULL,0);
    } // end if

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////