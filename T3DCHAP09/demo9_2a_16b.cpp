// DEMO9_2a_16b.CPP - DirectInput Mouse Demo library version
// 16-bit version, note that we render in 16-bit
// but since we are basing our color selection on a
// palette we still load an 8-bit palette into a datastructure
// so we can synthesize the palette


// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  

#define INITGUID // if not done elsewhere

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <objbase.h>
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


#include <ddraw.h>     // directX includes
#include <dinput.h>
#include "T3DLIB1.H"
#include "T3DLIB2.H"

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// screen size
#define SCREEN_WIDTH      640   // size of screen
#define SCREEN_HEIGHT     480

// setup a 640x480 16-bit windowed mode example
#define WINDOW_TITLE      "16-Bit Mouse Demo"
#define WINDOW_WIDTH      640   // size of window
#define WINDOW_HEIGHT     480

#define WINDOW_BPP        16    // bitdepth of window (8,16,24 etc.)
                                // note: if windowed and not
                                // fullscreen then bitdepth must
                                // be same as system bitdepth
                                // also if 8-bit the a pallete
                                // is created and attached

#define WINDOWED_APP      0     // 0 not windowed, 1 windowed

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256

#define BUTTON_SPRAY    0    // defines for each button
#define BUTTON_PENCIL   1
#define BUTTON_ERASE    2
#define BUTTON_EXIT     3


// TYPES ///////////////////////////////////////////////////


// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL, int num_parms = 0);
int Game_Shutdown(void *parms=NULL, int num_parms = 0);
int Game_Main(void *parms=NULL,  int num_parms = 0);

// GLOBALS ////////////////////////////////////////////////

// windows vars
HWND      main_window_handle = NULL; // globally track main window
int       window_closed      = 0;    // tracks if window is closed
HINSTANCE main_instance      = NULL; // globally track hinstance

char buffer[80];                // used to print text

// demo globals
BOB          buttons,           // a bob with all the buttons
             pointer;           // a pointer bob
BITMAP_IMAGE cpanel;            // the control panel
BITMAP_IMAGE canvas;            // off screen drawing canvas

int mouse_x,                    // used to track mouse
    mouse_y;
    
UCHAR mouse_color=100;          // color of mouse brush

int command_state=0;            // state of user command

// position of control buttons
int buttons_x[] = {509, 559, 509, 559};
int buttons_y[] = {344, 344, 383, 383};

// on/off state of buttons
int buttons_state[] = {0,1,0,0};

// PROTOTYPES //////////////////////////////////////////////

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
						    (WINDOWED_APP ? (WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE) : (WS_POPUP | WS_VISIBLE)), 
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
RECT window_rect = {0,0,WINDOW_WIDTH,WINDOW_HEIGHT};

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
           CW_USEDEFAULT, // x position
           CW_USEDEFAULT, // y position
           window_rect.right - window_rect.left, // width
           window_rect.bottom - window_rect.top, // height
           TRUE);

// show the window, so there's no garbage on first render
ShowWindow(main_window_handle, SW_SHOW);
} // end if windowed

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

// GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms,  int num_parms)
{
// this function is where you do all the initialization 
// for your game

int index;         // looping var
char filename[80]; // used to build up files names

// start up DirectDraw (replace the parms as you desire)
DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);

// start up DirectInput
DInput_Init();

// initialize the mouse
DInput_Init_Mouse();

/////////////////////////////////////////////////////////////////

// set the global mouse position
mouse_x = screen_height/2;
mouse_y = screen_height/2;

// load the master bitmap in with all the graphics
Load_Bitmap_File(&bitmap16bit, "PAINT_24.BMP");

// load a palette from a file since 16-bit modes don't have 
// palettes, when rendering from the palette, we will use 
// this data as an RGB lookup to create the colors
Load_Palette_From_File("PALDATA2.PAL", palette);

// make sure all the surfaces are clean before starting
DDraw_Fill_Surface(lpddsback, 0);
DDraw_Fill_Surface(lpddsprimary, 0);

// create the pointer bob
Create_BOB(&pointer,mouse_x,mouse_y,32,34,1,
           BOB_ATTR_VISIBLE | BOB_ATTR_SINGLE_FRAME,DDSCAPS_SYSTEMMEMORY,0,16);    

// load the image for the pointer in
Load_Frame_BOB16(&pointer,&bitmap16bit,0,0,2,BITMAP_EXTRACT_MODE_CELL);  

// create the button bob
Create_BOB(&buttons,0,0,32,34,8,
           BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_FRAME,DDSCAPS_SYSTEMMEMORY,0,16);    

// load buttons in, two banks of 4, all the off's, then all the on's
for (index=0; index<8; index++)
     Load_Frame_BOB16(&buttons,&bitmap16bit,index, index%4,index/4,BITMAP_EXTRACT_MODE_CELL);  

// create the bitmap to hold the control panel
Create_Bitmap(&cpanel,500,0,104,424,16);
Load_Image_Bitmap16(&cpanel, &bitmap16bit,150,0,BITMAP_EXTRACT_MODE_ABS);

// create the drawing canvas bitmap
Create_Bitmap(&canvas,0,0,500,SCREEN_HEIGHT,16);
memset(canvas.buffer,0,canvas.width*canvas.height*2);
canvas.attr = BITMAP_ATTR_LOADED;

// hide the mouse
ShowCursor(FALSE);

// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms,  int num_parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// release mouse
DInput_Release_Mouse();

// shutdown DirectInput
DInput_Shutdown();

// unload the bitmap file
Unload_Bitmap_File(&bitmap16bit);

// delete all bobs and bitmaps
Destroy_BOB(&buttons);
Destroy_BOB(&pointer);
Destroy_Bitmap(&cpanel);
Destroy_Bitmap(&canvas);

// shutdonw directdraw
DDraw_Shutdown();

// return success
return(1);
} // end Game_Shutdown

///////////////////////////////////////////////////////////

int Game_Main(void *parms, int num_parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

int index; // looping var


// check of user is trying to exit
if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_SPACE))
    PostMessage(main_window_handle, WM_DESTROY,0,0);

// start the timing clock
Start_Clock();

// clear the drawing surface
DDraw_Fill_Surface(lpddsback, 0);

// get the mouse data
DInput_Read_Mouse();

// move the mouse cursor
mouse_x+=(mouse_state.lX);
mouse_y+=(mouse_state.lY);

// test bounds

// first x boundaries
if (mouse_x >= screen_width)
   mouse_x = screen_width-1;
else
if (mouse_x < 0)
   mouse_x = 0;

// now the y boundaries
if (mouse_y >= screen_height)
   mouse_y= screen_height-1;
else
if (mouse_y < 0)
   mouse_y = 0;

// position the pointer bob to the mouse coords
pointer.x = mouse_x - 16;
pointer.y = mouse_y - 16;

// test what the user is doing with the mouse
if ((mouse_x > 3) && (mouse_x < 500-3) && 
    (mouse_y > 3) && (mouse_y < SCREEN_HEIGHT-3))
   {
   // mouse is within canvas region

   // if left button is down then draw
   if (mouse_state.rgbButtons[0])
      {
      // test drawing mode
      if (buttons_state[BUTTON_PENCIL])
         {
         // draw a pixel 
         Draw_Pixel16(mouse_x, mouse_y, RGB16Bit(palette[mouse_color].peRed, palette[mouse_color].peGreen,palette[mouse_color].peBlue), canvas.buffer, canvas.width*2);
         Draw_Pixel16(mouse_x+1, mouse_y, RGB16Bit(palette[mouse_color].peRed, palette[mouse_color].peGreen,palette[mouse_color].peBlue), canvas.buffer, canvas.width*2);
         Draw_Pixel16(mouse_x, mouse_y+1, RGB16Bit(palette[mouse_color].peRed, palette[mouse_color].peGreen,palette[mouse_color].peBlue), canvas.buffer, canvas.width*2);
         Draw_Pixel16(mouse_x+1, mouse_y+1, RGB16Bit(palette[mouse_color].peRed, palette[mouse_color].peGreen,palette[mouse_color].peBlue), canvas.buffer, canvas.width*2);
         }
      else
         {
         // draw spray
         for (index=0; index<10; index++)
             {
             // get next particle
             int sx=mouse_x-8+rand()%16;
             int sy=mouse_y-8+rand()%16;
            
             // make sure particle is in bounds
             if (sx > 0 && sx < 500 && sy > 0 && sy < screen_height)
                Draw_Pixel16(sx, sy, RGB16Bit(palette[mouse_color].peRed, palette[mouse_color].peGreen,palette[mouse_color].peBlue), canvas.buffer, canvas.width*2);
             } // end for index

         } // end else

      } // end if left button
    else // right button is eraser
    if (mouse_state.rgbButtons[1])
       {
       // test drawing mode
       if (buttons_state[BUTTON_PENCIL]) 
          {
          // erase a pixel 
          Draw_Pixel16(mouse_x,   mouse_y,   0, canvas.buffer, canvas.width*2);
          Draw_Pixel16(mouse_x+1, mouse_y,   0, canvas.buffer, canvas.width*2);
          Draw_Pixel16(mouse_x,   mouse_y+1, 0, canvas.buffer, canvas.width*2);
          Draw_Pixel16(mouse_x+1, mouse_y+1, 0, canvas.buffer, canvas.width*2);
          } // end if
       else
          {
          // erase spray
          for (index=0; index<20; index++)
              {
              // get next particle
              int sx=mouse_x-8+rand()%16;
              int sy=mouse_y-8+rand()%16;
            
              // make sure particle is in bounds
              if (sx > 0 && sx < 500 && sy > 0 && sy < screen_height)
                 Draw_Pixel16(sx, sy, 0, canvas.buffer, canvas.width*2);
              } // end for index
          
          } // end else
       
       } // end if left button
  
   } // end if
else
if ( (mouse_x > 500+16) && (mouse_x < 500+16+8*9) &&
     (mouse_y > 8)      && (mouse_y < 8+32*9))
   {
   // within palette

   // test if button left button is down
   if (mouse_state.rgbButtons[0])
      {
      // see what color cell user is pointing to
      int cell_x = (mouse_x - (500+16))/9;
      int cell_y = (mouse_y - (8))/9;

      // change color
      mouse_color = cell_x + cell_y*8;

      } // end if
   } // end if
else
if ((mouse_x > 500) && (mouse_x < (500+100)) &&
    (mouse_y > 344) && (mouse_y < (383+34)) )
   {  
   // within button area
   // test for each button
   for (index=0; index<4; index++)
       {
       if ((mouse_x > buttons_x[index]) && (mouse_x < (buttons_x[index]+32)) &&
           (mouse_y > buttons_y[index]) && (mouse_y < (buttons_y[index]+34)) )
           break;

       } // end for

   // at this point we know where the user is, now determine what he
   // is doing with the buttons
   switch(index)
         {
         case BUTTON_SPRAY:
             {
             // if left button is down simply activate spray mode
             if (mouse_state.rgbButtons[0])
                {
                // depress button
                buttons_state[index] = 1;

               // de-activate pencil mode
                buttons_state[BUTTON_PENCIL] = 0;
                } // end if
             else
                {
                // make sure button is up
                // buttons_state[index] = 0;
                } // end else

             } break;
         
         case BUTTON_PENCIL:
             {
             // if left button is down activate spray mode
             if (mouse_state.rgbButtons[0])
                {
                // depress button
                buttons_state[index] = 1;

                // de-activate spray mode
                buttons_state[BUTTON_SPRAY] = 0;

                } // end if
             else
                {
                // make sure button is up
                // buttons_state[index] = 0;
                } // end else

             } break;

         case BUTTON_ERASE:
             {
             // test if left button is down, if so clear screen
             if (mouse_state.rgbButtons[0])
                {
                // clear memory
                memset(canvas.buffer,0,canvas.width*canvas.height*2);

                // depress button
                buttons_state[index] = 1;
                } // end if
             else
                {
                // make sure button is up
                buttons_state[index] = 0;
                } // end else
             } break;
         
         case BUTTON_EXIT:
             {
             // test if left button down, if so bail
             if (mouse_state.rgbButtons[0])
                  PostMessage(main_window_handle, WM_DESTROY,0,0);

             } break;

         } // end switch

   } // end if
else
   {
   // no mans land

   } // end else

// lock back buffer
DDraw_Lock_Back_Surface();

// draw the canvas
Draw_Bitmap16(&canvas, back_buffer, back_lpitch,0);

// draw control panel
Draw_Bitmap16(&cpanel,back_buffer,back_lpitch,0);

// unlock back buffer
DDraw_Unlock_Back_Surface();

// draw the color palette
for (int col=0; col < 256; col++)
    {
    Draw_Rectangle(500+16+(col%8)*9,   8+(col/8)*9,
                   500+16+(col%8)*9+8, 8+(col/8)*9+8,
                   RGB16Bit(palette[col].peRed, palette[col].peGreen, palette[col].peBlue),
                   lpddsback);
    
    } // end for col

// draw the current color selected
Draw_Rectangle(533,306,533+34,306+34,RGB16Bit(palette[mouse_color].peRed, palette[mouse_color].peGreen,palette[mouse_color].peBlue),
                                     lpddsback);

// draw the buttons
for (index=0; index<4; index++)
    {
    // set position of button bob
    buttons.x = buttons_x[index];
    buttons.y = buttons_y[index];

    // now select the on/off frame based on if the
    // button is off
    if (buttons_state[index]==0)
        buttons.curr_frame = index;
    else // button is on
        buttons.curr_frame = index+4;

    // draw the button
    Draw_BOB(&buttons, lpddsback);

    } // end for index

static int green = 0;

// display coords
sprintf(buffer,"Pointer (%d,%d)",mouse_x,mouse_y);
Draw_Text_GDI(buffer, 8,screen_height - 16,RGB(0,255,0),lpddsback);
Draw_Text_GDI("(16-Bit Version) T3D Paint Version 2.0 - Press <ESC> to Exit.",0,0,RGB(0,(green & 255),0),lpddsback);

// a little animation
green+=2;

// draw the cursor last
Draw_BOB16(&pointer,lpddsback);

// flip the surfaces
DDraw_Flip();

// sync to 30 fps
Wait_Clock(30);

// return success
return(1);

} // end Game_Main

//////////////////////////////////////////////////////////