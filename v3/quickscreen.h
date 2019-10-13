/*
    quickscreen.h
*/

#ifndef __quickscreen_h__
#define __quickscreen_h__

#define QUI_Print_SDL_Error()         printf( "SDL Error: %s\n", SDL_GetError() )

//===============================================================
//  STRUCTS AND TYPES
//===============================================================

struct colorRGBA_s  {   unsigned char r;
                        unsigned char g;
                        unsigned char b;
                        unsigned char a;
                    };

typedef struct colorRGBA_s colorRGBA_type;

typedef enum {  QUI_COLOR_TRANS,
                QUI_COLOR_BLACK,
                QUI_COLOR_WHITE,
                QUI_COLOR_RED,
                QUI_COLOR_GREEN,
                QUI_COLOR_BLUE } colorIndex_type;

typedef unsigned char byte;

//===============================================================
//  FUNCTION PROTOTYPES
//===============================================================

//=============================
//  INIT AND CONTROL FUNCTIONS
//=============================


// creates a window, returns 1 on success, 0 on failure
int QUI_Create_Display( char *title, int width, int height, int res_width, int res_height );


// loads font, returns 1 on success
int QUI_Load_Media();


// free memory and exit SDL
void QUI_Close();


// wrapper for SDL_Delay function
void QUI_Delay( int milli );


// checks if user quits
int QUI_Check_Quit();


// get keystates
void QUI_Get_Key_State();

// pump keys
void QUI_Pump_Events();


// wrapper for SDL_GetTicks
uint32_t QUI_GetTicks();

//=============================
//  GRAPHICS FUNCTIONS
//=============================

// clear screen
void QUI_Clear_Screen();

// draw surface to window
void QUI_Refresh_Screen();

// creates a colour with the given rgb values
uint32_t QUI_Create_Color( uint8_t r, uint8_t g, uint8_t b, uint8_t a );

// create a pixel at given coordinates
void QUI_Set_Pixel( int x, int y, colorRGBA_type color );


// draws a vertical line to the surface
void QUI_Draw_Vertical_Line( int x1, int y1, int y2, colorRGBA_type color );


// draws a horizontal line to the surface
void QUI_Draw_Horizontal_Line( int x1, int x2, int y, colorRGBA_type color );

// draw a buffer of uint32_t to render_surface
void QUI_Draw_Buffer( uint32_t *buffer );

//=============================
//  FONT FUNCTIONS
//=============================

// places a character at (x, y), draws bg color if draw_bg is non zero
void QUI_Place_Char( int letter, int x, int y, colorRGBA_type forecolor, colorRGBA_type bgcolor, int draw_bg );


// writes a string of text without checking any screen boundaries
void QUI_Simple_Text( char *str, int x, int y, colorRGBA_type forecolor, colorRGBA_type bgcolor, int drawbg );


// wraps text inside an area
void QUI_Text_Box( char *str, int x, int y, int w, int h, colorRGBA_type forecolor, colorRGBA_type bgcolor, int drawbg );


//===============================
//  USEFUL
//===============================

// returns 0 for little endian, 1 for big
int QUI_Get_Endianess();

//=============================================================
//  TESTING
//=============================================================

// contains some testing functions
void QUI_Test();


#endif // __quickscreen_h__
