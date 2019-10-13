/*
    main.c
    raycasting program

    follows the explanation at permadi.com/tutorial/raycast/
    differs in that it uses vectors and matrices but the pricipals are the same
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "utility.h"
#include "quickscreen.h"
#include "vecmat.h"

//===============================================================
//  DEFINE AND CONSTANTS
//===============================================================

#define WINDOW_WIDTH                1024
#define WINDOW_HEIGHT               640

#define SCREEN_WIDTH_RES            1024
#define SCREEN_HEIGHT_RES           640

#define PALETTE_SIZE                256         // number of colours to choose from

#define MAX_FRAME_TIME              16          // limit to approx 60 fps

#define WORLD_WIDTH                 16
#define WORLD_HEIGHT                16

#define BLOCK_SIZE                  64          // each block in the world is 64*64*64
#define PLAYER_HEIGHT               32

#define PLAYER_GRID_START_POS_X     3.5
#define PLAYER_GRID_START_POS_Y     9.0

#define CIRCLE_RADIANS              6.28        // radians in one circle
#define CIRCLE_DEGREES              360

#define PLAYER_FOV                  90          // 90 degrees
#define PLAYER_SPEED                5           // movement per frame

// world to render
int WORLD_MAP[WORLD_HEIGHT][WORLD_WIDTH] =
{
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

// these vectors are multiplied by the transformation matrix each frame to get the players
// correct orientation, the screen falls between -1.0 ( 0 ) and 1.0 ( SCREEN_WIDTH_RES - 1 )
const vector2d_type             DIRECTION_UP        = {  0.0, -1.0 };  // vector always points up
const vector2d_type             DIRECTION_RIGHT     = {  1.0,  0.0 };  

const matrix2d_type             IDENTITY_MATRIX     = {{   
                                                        { 1.0, 0.0, 0.0 },
                                                        { 0.0, 1.0, 0.0 },
                                                        { 0.0, 0.0, 1.0 } 
                                                      }};

colorRGBA_type      RED         = { 0xff, 0x00, 0x00, 0xff };
colorRGBA_type      DARK_RED    = { 0x80, 0x00, 0x00, 0xff };
colorRGBA_type      GREY        = { 0xa0, 0xa0, 0xa0, 0xff };
colorRGBA_type      WHITE       = { 0xff, 0xff, 0xff, 0xff };

//======================
//  PLAYER VARIABLES
//======================

vector2d_type           player_pos      = { PLAYER_GRID_START_POS_X, PLAYER_GRID_START_POS_Y };
vector2d_type           player_dir;
vector2d_type           player_screen;

float                   player_angle    = 0.0;

matrix2d_type           matrix;

//===========================
//  LOOKUP TABLES AND BUFFERS
//===========================

// oldschool stuff, formed by code
float           *SIN_LOOKUP         = NULL;
float           *COS_LOOKUP         = NULL;
float           *TAN_LOOKUP         = NULL;

uint32_t        *PALETTE_TABLE      = NULL;
uint32_t        *SCREEN_BUFFER      = NULL;     // doubles as a SDL_Surface, dimensions will be
                                                // SCREEN_WIDTH_RES x SCREEN_HEIGHT_RES

//===============================================================
//  FUNCTION PROTOTYPES
//===============================================================

// all functions return 1 on success or 0 on failure unless indicated

//===================
// TRIGONOMETRY
//===================

// creates and populates COS_LOOKUP and SIN_LOOKUP
int Create_Trig_Lookup_Tables();

// retrieves lookup values
float Get_Sin( int angle );
float Get_Cos( int angle );
float Get_Tan( int angle );

//===================
// COLOUR AND SCREEN
//===================

// populates a 256 colour palette for quick lookup
int Create_Palette_Table();

// creates a buffer to write pixel data to
int Create_Screen_Buffer();

// clear screen buffer by setting it to value of color
void Fill_Screen_Buffer( uint32_t color );

// draws data to the screen buffer
void Draw_Vertical_Line( int column_index, int column_start, int column_end, uint32_t color );


// TODO
// int Create_Texture_Data();

//====================
//  RAYCASTING
//====================

// render current scene
void Draw_Scene();

//=============================
//  SYSTEM AND MEMORY
//=============================

// initialize globals
void Init_Globals();

// free all malloc'd data
void Free_All_Memory();


//===============================================================
//  MAIN FUNCTION
//===============================================================

int main( int argc, char *argv[] )
{
    // create a display window
    if( QUI_Create_Display( "Raycaster v2", WINDOW_WIDTH, WINDOW_HEIGHT, 
                            SCREEN_WIDTH_RES, SCREEN_HEIGHT_RES ) == 0 )
    {
        UTI_Fatal_Error( "Unable to create display" );
    }

    // load font
    if( QUI_Load_Media() == 0 )
    {
        UTI_Fatal_Error( "Unable to load font data" );
    }

    // create old-fashioned lookup tables
    if( Create_Trig_Lookup_Tables() == 0 )
    {
        UTI_Fatal_Error( "Unable to create trigonometry look-up tables" );
    }

    // create a 225 colour palette
    if( Create_Palette_Table() == 0 )
    {
        UTI_Fatal_Error( "Unable to create colour palette" );
    }

    // create a buffer to edit instead of using QUI_Set_Pixel
    if( Create_Screen_Buffer() == 0 )
    {
        UTI_Fatal_Error( "Unable to create screen buffer" );
    }

    // loop control
    int running = 1;

    while( running )
    {
        // clear screen
        Fill_Screen_Buffer( 0xff800000 );

        // draw current scene from players perspective to buffer
        Draw_Scene();

        // draw buffer to the render_surface
        QUI_Draw_Buffer( SCREEN_BUFFER );

        // test the text still works
        QUI_Simple_Text( "This is text", 8, 8, WHITE, RED, 0 );

        // convert render_surface to the screen_surface and display
        QUI_Refresh_Screen();

        // don't use too much CPU time
        QUI_Delay( 5 );
    
        // change angle to show effect
        player_angle += 0.01f;
        if( player_angle > CIRCLE_RADIANS ) player_angle = 0.0f;
        
        // check if user wants to quit
        running = QUI_Check_Quit();
    }

    QUI_Close();
    
    Free_All_Memory();

    return 0;
}

//===============================================================
//  FUNCTION BODIES
//===============================================================

//=======================
//  TRIGONOMETRY
//=======================

// creates and populates COS_LOOKUP and SIN_LOOKUP
int Create_Trig_Lookup_Tables()
{
    // create a table of 360 values for each function
    SIN_LOOKUP = UTI_EC_Malloc( sizeof( float ) * CIRCLE_DEGREES );
    COS_LOOKUP = UTI_EC_Malloc( sizeof( float ) * CIRCLE_DEGREES );
    TAN_LOOKUP = UTI_EC_Malloc( sizeof( float ) * CIRCLE_DEGREES );

    int i;
    for( i = 0; i < CIRCLE_DEGREES; i++ )
    {
        SIN_LOOKUP[i] = sin( (float)i );
        COS_LOOKUP[i] = cos( (float)i );
        TAN_LOOKUP[i] = tan( (float)i );
    }

    UTI_Print_Debug( "Trigonometry look-up tables created" );
    
    return 1;
}
    
// get sin of given angle in degrees
float Get_Sin( int angle )
{
    return SIN_LOOKUP[angle];
}

// get cos of given angle in degrees
float Get_Cos( int angle )
{
    return COS_LOOKUP[angle];
}

float Get_Tan( int angle )
{
    return TAN_LOOKUP[angle];
}

//=======================
//  COLOUR AND SCREEN
//=======================


// populates a 256 colour palette for quick lookup
int Create_Palette_Table()
{
    // initialize palette memory
    PALETTE_TABLE = UTI_EC_Malloc( sizeof( uint32_t ) * PALETTE_SIZE );

    // colour components
    uint32_t red, green, blue;
    const uint8_t alpha = 0xff;         // alpha will not change here

    red = green = blue = 0;

    uint32_t final_color;

    // fill in first 225 colours of palette
    int i = 0;
    for( red = 0; red <= 255; red += 51 )
    {
        for( green = 0; green <= 255; green += 51 )
        {
            for( blue = 0; blue <= 255; blue += 51 )
            {
                // generate colour
                final_color = ( red*0x1000000 + green*0x10000 + blue*0x100 + alpha );
                PALETTE_TABLE[i] = final_color;
                i++;
            }
        }
    }


    // fill in rest of palette (last 40 entries) with black
    final_color = 0xffffffff;
    while( i < PALETTE_SIZE )
    {
        PALETTE_TABLE[i] = final_color;
        i++;
    }
    
    UTI_Print_Debug( "Palette table created" );
    
    return 1;
}


// creates a buffer to write pixel data to
int Create_Screen_Buffer()
{    
    // create a buffer of uint32_t
    SCREEN_BUFFER = UTI_EC_Malloc( sizeof( uint32_t ) * SCREEN_WIDTH_RES * SCREEN_HEIGHT_RES );

    Fill_Screen_Buffer( 0 );
    
    UTI_Print_Debug( "Screen buffer created" );

    return 1;
}


// sets all pixels in buffer to color
void Fill_Screen_Buffer( uint32_t color )
{
    int i = 0;
    for( i = 0; i < SCREEN_WIDTH_RES * SCREEN_HEIGHT_RES; i++ )
    {
        SCREEN_BUFFER[i] = color;
    }

    return;
}

void Draw_Vertical_Line( int column_index, int column_start, int column_end, uint32_t color )
{
    // make sure column is on screen
    if( column_index > SCREEN_WIDTH_RES || column_index < 0 )
    {
        return;
    }

    // check column_start is higher on screen (ie a lower value)
    if( column_start > column_end )
    {
        int temp;
        temp = column_start;
        column_start = column_end;
        column_end = temp;
    }

    // make sure at least part of column is on screen
    if( column_start >= SCREEN_HEIGHT_RES || column_end < 0 )
    {
        return;
    }

    // make sure theres no attempt to write to part of the buffer that isn't there
    if( column_start < 0 )                      column_start = 0;
    if( column_end > SCREEN_HEIGHT_RES - 1)     column_end = SCREEN_HEIGHT_RES - 1;

    for( ; column_start <= column_end; column_start++ )
    {
        SCREEN_BUFFER[column_start * SCREEN_WIDTH_RES + column_index] = color;
    }

    return;
}

//====================
//  RAYCASTING
//====================

// render current scene
void Draw_Scene()
{
    vector2d_type           ray_pos;                    // data for current ray       
    vector2d_type           ray_dir;
    matrix                  = IDENTITY_MATRIX;

    // get players current orientation
    VEC_Matrix_Rotation( &matrix, player_angle );
    player_dir      = VEC_Matrix_Transform_Vector( &matrix, DIRECTION_UP );
    player_screen   = VEC_Matrix_Transform_Vector( &matrix, DIRECTION_RIGHT );

    // screen space runs from -1.0 to 1.0, scaled with the screen resolution
    int column_index;               // index of column of pixels
    float screen_column;      // position of column in screen space
    for( column_index = 0; column_index < SCREEN_WIDTH_RES; column_index++ )
    {
        // get pixel column position in screen space (between -1.0 and 1.0)
        screen_column = 2 * column_index / (double)( SCREEN_WIDTH_RES ) - 1; 
        
        // ray always starts at player position
        ray_pos = player_pos;

        // ray_dir will be the vector that points from the player to the
        // point on the screen plane the ray is pointing at
        ray_dir = VEC_Scale_Vector( player_screen, screen_column );
        ray_dir = VEC_Vector_Addition( player_dir, ray_dir );

        // these variables are used to calculate ray steps and check for walls
        float   x_dist, y_dist, x_delta, y_delta, ray_length;
        int     step_x, step_y, map_x, map_y;
        int     wallhit         = 0;                // flag, stop when this is 1
        int     walltype        = 0;                // 0 for vertical wall, 1 for horizontal

        map_x = (int)player_pos.x;          // these values are used to check against WORLD_MAP
        map_y = (int)player_pos.y;          // to see if a block is a wall or not

        // calculate how much to increment the ray in each direction
        float x_sqr = ray_dir.x * ray_dir.x;
        float y_sqr = ray_dir.y * ray_dir.y;
        x_delta = sqrt( 1 + (y_sqr) / (x_sqr) );
        y_delta = sqrt( 1 + (x_sqr) / (y_sqr) );

        // check ray x axis direction
        if( ray_dir.x < 0 )
        {
            step_x = -1;        // ray is moving west

            // distance to nearest edge from current position
            x_dist = (ray_pos.x - map_x) * x_delta;   
        }                                               
        else
        {
            step_x =  1;        // ray is moving east
            x_dist = (map_x + 1 - ray_pos.x) * x_delta;
        }


        // same for y axis
        if( ray_dir.y < 0 )
        {
            step_y = -1;        // ray is moving north
            y_dist = (ray_pos.y - map_y) * y_delta;
        }
        else
        {
            step_y = 1;
            y_dist = (map_y + 1 - ray_pos.y) * y_delta;
        }

        // now for the actual ray-casting - the ray is extended until it hits a wall belonging
        // to a block that needs to be drawn (non-zero in WORLD_MAP)
        while( wallhit == 0 && map_x < WORLD_WIDTH  && map_x > 0 
                            && map_y < WORLD_HEIGHT && map_y > 0 )
        {
            // increment shortest first to avoid returning wrong side of a block
            if( x_dist < y_dist )
            {
                x_dist += x_delta;
                map_x += step_x;
                walltype = 0;
            }
            else
            {
                y_dist += y_delta;
                map_y += step_y;
                walltype = 1;
            }

            // not sure why world map is referenced this way, but it works
            if( WORLD_MAP[map_y][map_x] > 0 )
            {
                wallhit = 1;
            }
        }

        // in the current form of the program a wall should always be hit (the edges of the map
        // are all blocks)
        if( wallhit == 1 )
        {
            if( walltype == 0 )
            {
                // get distance to the wall
                ray_length = fabs( (map_x - ray_pos.x + (1 - step_x) / 2) / ray_dir.x );
            }
            else
            {
                ray_length = fabs( (map_y - ray_pos.y + (1 - step_y) / 2) / ray_dir.y );
            }
        }

        // now we know how far away the wall is, we can calculate how tall it should
        // appear on screen

        int column_height   = abs( (int)( SCREEN_HEIGHT_RES / ray_length ) );

        // get height above horizon
        int column_start    = -column_height / 2 + SCREEN_HEIGHT_RES / 2;
        // get height below horizon
        int column_end      =  column_height / 2 + SCREEN_HEIGHT_RES / 2;

        // set a colour - these ones are chosen at random
        uint32_t color;
        if( walltype == 0 )
            color = 0xff0000ff;       
        else
            color = 0xff000080;
        
        Draw_Vertical_Line( column_index, column_start, column_end, color );
    }

    return;
}

//=============================
//  SYSTEM AND MEMORY
//=============================

// initialize globals
void Init_Globals()
{
    player_dir          = DIRECTION_UP;
    player_screen       = DIRECTION_RIGHT;
}


// free all malloc'd data
void Free_All_Memory()
{
    free( SIN_LOOKUP );
    free( COS_LOOKUP );
    free( TAN_LOOKUP );
    free( PALETTE_TABLE );
    free( SCREEN_BUFFER );
}

//===============================================================
//  TESTING FUNCTIONS
//===============================================================

