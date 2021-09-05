// BLACKBOX.H - Header file for demo game engine library

// watch for multiple inclusions
#ifndef BLACKBOX
#define BLACKBOX

// DEFINES ////////////////////////////////////////////////////

// default screen size
#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      8    // bits per pixel
#define MAX_COLORS      256  // maximum colors

// MACROS /////////////////////////////////////////////////////

// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// initializes a direct draw struct
#define DD_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// EXTERNALS //////////////////////////////////////////////////

extern LPDIRECTDRAW7         lpdd;                 // dd object
extern LPDIRECTDRAWSURFACE7  lpddsprimary;         // dd primary surface
extern LPDIRECTDRAWSURFACE7  lpddsback;            // dd back surface
extern LPDIRECTDRAWPALETTE   lpddpal;              // a pointer to the created dd palette
extern LPDIRECTDRAWCLIPPER   lpddclipper;          // dd clipper
extern PALETTEENTRY          palette[256];         // color palette
extern PALETTEENTRY          save_palette[256];    // used to save palettes
extern DDSURFACEDESC2        ddsd;                 // a direct draw surface description struct
extern DDBLTFX               ddbltfx;              // used to fill
extern DDSCAPS2              ddscaps;              // a direct draw surface capabilities struct
extern HRESULT               ddrval;               // result back from dd calls
extern DWORD                 start_clock_count;    // used for timing

// these defined the general clipping rectangle
extern int min_clip_x,                             // clipping rectangle 
           max_clip_x,                  
           min_clip_y,     
           max_clip_y;                  

// these are overwritten globally by DD_Init()
extern int screen_width,                            // width of screen
           screen_height,                           // height of screen
           screen_bpp;                              // bits per pixel 

// PROTOTYPES /////////////////////////////////////////////////

// DirectDraw functions
int DD_Init(int width, int height, int bpp);
int DD_Shutdown(void);
LPDIRECTDRAWCLIPPER DD_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list);
int DD_Flip(void);
int DD_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,int color);

// general utility functions
DWORD Start_Clock(void);
DWORD Get_Clock(void);
DWORD Wait_Clock(DWORD count);

// graphics functions
int Draw_Rectangle(int x1, int y1, int x2, int y2, int color,LPDIRECTDRAWSURFACE7 lpdds=lpddsback);

// gdi functions
int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds=lpddsback);
int Draw_Text_GDI(char *text, int x,int y,int color, LPDIRECTDRAWSURFACE7 lpdds=lpddsback);

#endif
