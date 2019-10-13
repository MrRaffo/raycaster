/*
    quickscreen.c
    all drawing and display related functions
    also handles detection of user input
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "utility.h"
#include "quickscreen.h"

//===============================================================
//  DEFINE AND CONSTANTS
//===============================================================

#define RENDERMODE                  TEXTURE

//==================
// char constants
//==================

// TODO load these values from a font file
#define CHAR_SET_SIZE               256
#define CHAR_PIXEL_WIDTH            8
#define CHAR_PIXEL_HEIGHT           8
#define CHAR_PACKED_SIZE            8           // size of a char in bytes in file
#define CHAR_UNPACKED_SIZE          64          // size when loaded to memory

//==================
// colour constants
//==================

const colorRGBA_type RGBA_TRANS             = { 0x00, 0x00, 0x00, 0x00 };
const colorRGBA_type RGBA_BLACK             = { 0x00, 0x00, 0x00, 0xff };
const colorRGBA_type RGBA_WHITE             = { 0xff, 0xff, 0xff, 0xff };
const colorRGBA_type RGBA_RED               = { 0xff, 0x00, 0x00, 0xff };
const colorRGBA_type RGBA_GREEN             = { 0x00, 0xff, 0x00, 0xff };
const colorRGBA_type RGBA_BLUE              = { 0x00, 0x00, 0xff, 0xff };
                
//===============================================================
//  SCREEN VARIABLES
//===============================================================

static SDL_Window       *screen_window      = NULL;
static SDL_Surface      *screen_surface     = NULL;
static SDL_Surface      *render_surface    = NULL;

static int              screen_width        = 0;
static int              screen_height       = 0;

static int              render_width        = 0;
static int              render_height       = 0;


//===============================================================
//  PRIVATE FUNCTIONS
//===============================================================

// loads font data from file
static int Load_Font_Data();

// creates a mask for the font data
static int Create_Font_Buffer();

// converts input character to its equivilent in my set
static int Convert_Character( char c );


//===============================================================
//  SCREEN FUNCTIONS
//===============================================================

// creates a window, returns 1 on success, 0 on failure
int QUI_Create_Display( char *title, int width, int height, int res_width, int res_height )
{
    // initialize SDL Video system
    if( SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
        UTI_Print_Error( "Unable to initialize SDL Video\n" );
        QUI_Print_SDL_Error();
        return 0;
    }

    // create display window
    screen_window = SDL_CreateWindow(   title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        width, height, SDL_WINDOW_SHOWN );

    if( screen_window == NULL )
    {
        UTI_Print_Error( "Unable to create SDL Window\n" );
        QUI_Print_SDL_Error();
        return 0;
    }

    // create display surface
    screen_surface = SDL_GetWindowSurface( screen_window );
    if( screen_surface == NULL )
    {
        UTI_Print_Error( "Unable to get window surface" );
        QUI_Print_SDL_Error();
        return 0;
    }

    // possibly not wise to use separate variables when the surface contains the info
    screen_width = screen_surface->w;
    screen_height = screen_surface->h;

    render_width = res_width;
    render_height = res_height;

    // set masks
    uint32_t rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif      // SDL_BYTEORDER

    // this is where the graphics will be rendered to, then copied to screen_surface
    render_surface = SDL_CreateRGBSurface( SDL_SWSURFACE, render_width, render_height, 32,
                                            rmask, gmask, bmask, amask );
    if( render_surface == NULL )
    {
        UTI_Print_Error( "Unable to create render_surface" );
        QUI_Print_SDL_Error();
    }

    // sdl initialized and window and surface set up
    return 1;
}


// loads any media needed, returns 1 on success
int QUI_Load_Media()
{
    // load the font data
    if( Load_Font_Data() == 0 )
    {
        UTI_Print_Error( "Unable to load font data" );
        return 0;
    }

    if( Create_Font_Buffer() == 0 )
    {
        UTI_Print_Error( "Unable to unpack font data" );
        return 0;
    }

    // everything load successfully
    return 1;
}


// free memory and exit SDL
void QUI_Close()
{
    SDL_DestroyWindow( screen_window );
    screen_window = NULL;

    SDL_FreeSurface( render_surface );
    render_surface = NULL;

    SDL_Quit();
}


// wrapper for SDL_Delay function
void QUI_Delay( int milli )
{
    SDL_Delay( milli );

    return;
}


// checks if user quits
int QUI_Check_Quit()
{
    SDL_Event e;
    while( SDL_PollEvent( &e ) != 0 )
    {
        if( e.type == SDL_QUIT )
        {
            printf( "QUIT\n" );
            return 0;
        }
        else if( e.type == SDL_KEYDOWN )
        {
            switch( e.key.keysym.sym )
            {
                case SDLK_ESCAPE:
                return 0;
            }
        }

    }

    return 1;
}



const uint8_t *keys;
// get keystates
void QUI_Get_Key_State()
{
    keys = SDL_GetKeyboardState( NULL );
}

// pump keys
void QUI_Pump_Events()
{
    SDL_PumpEvents();
    return;
}

// wrapper for SDL_GetTicks();
uint32_t QUI_GetTicks()
{
    return SDL_GetTicks();
}

//===============================================================
//  DRAW FUNCTIONS
//===============================================================

// clear screen
void QUI_Clear_Screen( colorRGBA_type color )
{
    SDL_FillRect( screen_surface, NULL, color.r*0x10000 + color.g*0x100 + color.b );
    SDL_FillRect( render_surface, NULL, color.r*0x10000 + color.g*0x100 + color.b );

    return;
}

// draw surface to window
void QUI_Refresh_Screen()
{
    SDL_Rect screen_rect;
    screen_rect.x = 0;
    screen_rect.y = 0;
    screen_rect.w = screen_width;
    screen_rect.h = screen_height;
    SDL_BlitScaled( render_surface, NULL, screen_surface, &screen_rect );

    SDL_UpdateWindowSurface( screen_window );
}


// draw a pixel at (x, y)
void QUI_Set_Pixel( int x, int y, colorRGBA_type color )
{
    // check if pixel is within screen bounds
    if( x < 0 || x >= render_width || y < 0 || y >= render_height )
    {
        return;
    }

    uint32_t new_pixel  = SDL_MapRGBA( render_surface->format, color.r, color.g, color.b, color.a );
    
    uint32_t *pixel;
    pixel = (uint32_t *)render_surface->pixels + (y * render_width) + x;
    
    *pixel = new_pixel;

    return;
}

// draw a vertical line to the surface from y1 to y2 at position x
void QUI_Draw_Vertical_Line( int x, int y1, int y2, colorRGBA_type color )
{
    // check line is within screen bounds
    if( x < 0 || x >= render_width )
    {
        return;
    }
    
    // make sure y2 is lower on screen (ie the larger number)
    if( y1 > y2 )
    {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    // make sure line is within vertical screen bounds
    if( y2 < 0 || y1 >= render_height )
    {
        return;
    }

    // if line is on screen but extremes are off, set extremes to screen limits
    if( y1 < 0 )                        y1 = 0;
    if( y2 > render_surface->h-1 )      y2 = render_surface->h-1;

    // draw the line
    for( ; y1 <= y2; y1++ )
    {
        QUI_Set_Pixel( x, y1, color );
    }

    return ;
}


// draws a horizontal line to the surface
void QUI_Draw_Horizontal_Line( int x1, int x2, int y, colorRGBA_type color )
{
    // ensure x1 is smaller than x2
    if( x1 > x2 )
    {
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }

    // ensure line is on screen
    if( x2 < 0 || x1 >= render_width )
    {
        return;
    }

    for( ; x1 < x2; x1++ )
    {
        QUI_Set_Pixel( x1, y, color );
    }

    return;
}
    

// draw a buffer of uint32_t to render_surface
void QUI_Draw_Buffer( uint32_t *buffer )
{
    uint32_t *bufp;
    bufp = render_surface->pixels;          // pointer to surface pixel data
    int x, y;
    for( y = 0; y < render_surface->h; y++ )
    {
        for( x = 0; x < render_surface->w; x++ )
        {
            *bufp = buffer[y * render_surface->w + x];
            bufp++;
        }
    }

    return;
}

//===============================================================
//  CHARACTER FUNCTIONS
//===============================================================

#include <ctype.h>
#include <string.h>

static char                 font_data_file[] = "data/font";         // font raw data
static byte                 *font_data_buffer = NULL;             // buffer for file
static byte                 *pixel_data_buffer = NULL;               // buffer for unpacked font

// loads font data from file
static int Load_Font_Data()
{
    FILE    *file;
    int     filesize = 0;

    // open file, read only
    file = fopen( font_data_file, "r" );
    if( file == NULL )
    {
        UTI_Print_Error( "Unable to open font data file" );
        return 0;
    }

    // get file size
    fseek( file, 0, SEEK_END );
    filesize = ftell( file );
    rewind( file );

    // create data buffer for file
    font_data_buffer = UTI_EC_Malloc( filesize );

    // read data into memory
    fread( font_data_buffer, filesize, 1, file );

    // close file
    fclose( file );

    return 1;
}


// put these values in a font file for versitility
#define     CHAR_LOWER_OFFSET       1
#define     CHAR_UPPER_OFFSET       33
#define     CHAR_NUMBER_OFFSET      62


// creates a bytemap of font data (eliminates need for bitwise operations later
static int Create_Font_Buffer()
{
    // allocate memory for buffer
    pixel_data_buffer = UTI_EC_Malloc( CHAR_SET_SIZE * CHAR_PIXEL_WIDTH * CHAR_PIXEL_HEIGHT );

    int i, j, c;
    for( i = 0; i < CHAR_SET_SIZE * CHAR_PACKED_SIZE; i++ )         
    {
        // get byte of packed data
        for( j = 0; j < CHAR_PACKED_SIZE; j++ )
        {
            // get bit of packed data
            c = font_data_buffer[i] & ( 0x80 >> j );

            // check if bit is set and set byte for 'bytemap'
            pixel_data_buffer[i*CHAR_PACKED_SIZE + j] = ( c == 0 ) ? 0 : 1;
        }
    }

    return 1;
}


// converts input character into its equivilent in my character set
static int Convert_Character( char c )
{
    if( islower( c ) )
    {
        return( c - 'a' ) + CHAR_LOWER_OFFSET;
    }
    else if( isupper( c ) )
    {
        return( c - 'A' ) + CHAR_UPPER_OFFSET;
    }
    else if( isdigit( c ) )
    {
        return( c - '0' ) + CHAR_NUMBER_OFFSET;
    }
    else
    {
        return c;
    }
}


// TODO - consider making this function static, no need to be public
// outputs a character at the given (x,y) coords in the given colour, with bg colour if needed
void QUI_Place_Char( int c, int x, int y, colorRGBA_type forecolor, colorRGBA_type bgcolor, int transparent_flag )
{
    // offset is the number of the character if the font buffer
    int offset = Convert_Character( c ) * CHAR_UNPACKED_SIZE;
    int i, j, pixel;

    for( i = 0; i < CHAR_PIXEL_HEIGHT; i++ )
    {
        for( j = 0; j < CHAR_PIXEL_WIDTH; j++ )
        {
            // check whether this pixel is part of the letter
            pixel = pixel_data_buffer[(i*CHAR_PIXEL_WIDTH) + j + offset];
            if( transparent_flag == 0 && pixel == 0 )
            {
                continue;       // do not draw to the screen if pixel should be transparent
            }
            else
            {
                QUI_Set_Pixel( x+j, y+i, (pixel) ? forecolor : bgcolor );
            }
        }
    }

    return;
}


// prints a line of text to the screen, will not wrap around text if it passes screen edges
void QUI_Simple_Text( char *str, int x, int y, colorRGBA_type forecolor, colorRGBA_type bgcolor, int transparent_flag )
{

    int i, len = strlen( str ) + 1;

    for( i = 0; i < len; i++ )
    {
        QUI_Place_Char( str[i], x, y, forecolor, bgcolor, transparent_flag );
        x = x + CHAR_PIXEL_WIDTH;
    }
}





//=============================================================
//  TESTING
//=============================================================


