/*
    main.c

    raycaster with textures loaded from file
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "utility.h"
#include "graphics.h"
#include "vecmat.h"

//==================================================================
//  DEFINES AND CONSTANTS
//==================================================================

#define SCREEN_W            1280
#define SCREEN_H            800

#define RES_W               640
#define RES_H               400

#define WORLD_WIDTH         16
#define WORLD_HEIGHT        16

#define PLAYER_START_X      10
#define PLAYER_START_Y      10

#define TEXTURE_FILE        "textures/walls.txr"

// world to render
int WORLD_MAP[WORLD_HEIGHT][WORLD_WIDTH] =
{
    { 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1 },
    { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 4, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
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

uint32_t                        RED         = 0xff0000ff;
uint32_t                        DARK_RED    = 0xff000080;
uint32_t                        WHITE       = 0xffffffff;
uint32_t                        BLACK       = 0xff000000;
uint32_t                        BLUE        = 0xffff0000;
uint32_t                        DARK_BLUE   = 0xff000040;
uint32_t                        GREEN       = 0xff00ff00;

//==================================================================
//  GLOBAL VARIABLES
//==================================================================

vector2d_type                   player_pos = { PLAYER_START_X, PLAYER_START_Y };
vector2d_type                   player_dir;
vector2d_type                   player_screen;          // the screen plane

float                           player_angle = 0.0f;

matrix2d_type                   matrix;

//==================================================================
//  FUNCTION PROTOTYPES
//==================================================================

// renders the scene, all drawing code goes here
void Draw_Scene();

// initialize global variables and structs
void Init_Globals();

//==================================================================
//  MAIN FUNCTION
//==================================================================

int main( int argc, char *argv[] )
{
    // start SDL
    if( GRA_Create_Display( "Raycaster v4 - Textures", SCREEN_W, SCREEN_H, RES_W, RES_H ) == 0 )
    {
        UTI_Fatal_Error( "Unable to start SDL" );
    }

    // generate palette
    if( GRA_Generate_Palette() == 0 )
    {
        UTI_Fatal_Error( "Unable to load palette" );
    }

    // load font
    if( GRA_Load_Font( "data/font" ) == 0 )
    {
        UTI_Fatal_Error( "Unable to load font" );
    }

    // load textures
    if( GRA_Load_Textures( TEXTURE_FILE ) == 0 )
    {
        UTI_Fatal_Error( "Unable to load textures" );
    }

    // load assets

    // loop control
    int running = 1;
    while( running )
    {
        // draw code
        GRA_Clear_Screen();

        GRA_Fill_Screen( DARK_BLUE );

        Draw_Scene();

        GRA_Simple_Text( "Hallo There!", 16, 16, 0xffffffff, 0xff000000, 0 );

        GRA_Refresh_Window();

        player_angle += 0.01f;

        running = GRA_Check_Quit();

        GRA_Delay( 5 );
    }

    GRA_Free_Palette();

    GRA_Free_Textures();

    GRA_Close();


    return 0;
}



//==================================================================
//  FUNCTION BODIES
//==================================================================

//==================
//  RAYCASTING
//==================

// renders the scene, all drawing code goes here
void Draw_Scene()
{
    // data for the current ray
    vector2d_type               ray_pos;
    vector2d_type               ray_dir;

    // transformation matrix
    matrix                    = IDENTITY_MATRIX;

    // get players orientation
    VEC_Matrix_Rotation( &matrix, player_angle );

    // get players new position and heading
    player_dir      = VEC_Matrix_Transform_Vector( &matrix, DIRECTION_UP );
    player_screen   = VEC_Matrix_Transform_Vector( &matrix, DIRECTION_RIGHT );

    // screen variables, screen space runs from -1.0 to 1.0, width depending on the resolution
    int column_index;                   // index of column of pixels being drawn
    float screen_column;                // position of pixel column in normalised screen space

    // this loop runs through all the columns on screen, drawing walls when needed
    for( column_index = 0; column_index < RES_W; column_index++ )
    {
        // get pixel column position in screen space
        screen_column = 2 * column_index / (float)( RES_W ) - 1;

        // ray always starts at the player position
        ray_pos = player_pos;

        // ray_dir is the vector that points from the player to the point on the screen plane
        ray_dir = VEC_Scale_Vector( player_screen, screen_column );
        ray_dir = VEC_Vector_Addition( player_dir, ray_dir );

        // these variables are used to calculate each step of the ray to check for walls
        float       x_dist, y_dist, x_delta, y_delta, ray_length;
        int         step_x, step_y, map_x, map_y;
        
        // flags to determine where to draw a wall
        int wallhit     = 0;
        int walltype    = 0;

        map_x = (int)player_pos.x;      // the map position of the ray, beginning with the
        map_y = (int)player_pos.y;      // players position, used to check for walls

        // calculate how much to increment the ray along each axis
        float x_sqr = ray_dir.x * ray_dir.x;
        float y_sqr = ray_dir.y * ray_dir.y;
        x_delta = sqrt( 1 + (y_sqr) / (x_sqr) );
        y_delta = sqrt( 1 + (x_sqr) / (y_sqr) );

        // check ray x axis direction
        if( ray_dir.x < 0 )
        {
            step_x = -1;                // ray is moving west

            // distance to nearest edge from the current position
            x_dist = (ray_pos.x - map_x) * x_delta;
        }
        else
        {
            step_x = 1;                 // ray is moving east
            x_dist = (map_x + 1 - ray_pos.x) * x_delta;
        }

        // same check for y axis
        if( ray_dir.y < 0 )
        {
            step_y = -1;                // ray is moving north
            y_dist = (ray_pos.y - map_y) * y_delta;
        }
        else
        {
            step_y = 1;
            y_dist = (map_y + 1 - ray_pos.y) * y_delta;
        }

        // raycasting - the ray is extended until is hits a wall (non-zero block on the map)
        while( wallhit == 0 && map_x < WORLD_WIDTH  && map_x > 0
                            && map_y < WORLD_HEIGHT && map_y > 0 )
        {
            // increment shortest first to avoid returning wrong side of a block
            if( x_dist < y_dist )
            {
                x_dist      += x_delta;
                map_x       += step_x;
                walltype    = 0;
            }
            else
            {
                y_dist      += y_delta;
                map_y       += step_y;
                walltype    = 1;
            }

            if( WORLD_MAP[map_y][map_x] > 0 )
            {
                wallhit = 1;
            }
        }

        // if a wall is hit, draw it
        if( wallhit == 1 )
        {
            float texel_normal;             // texel column from 0.0 - 1.0

            if( walltype == 0 )             // if wall is on x axis
            {
                // get the distance to the wall
                ray_length = fabs( (map_x - ray_pos.x + (1 - step_x) / 2) / ray_dir.x );

                // find where on the block the ray hit
                texel_normal = ray_pos.y + ray_length * ray_dir.y;
            }
            else
            {
                ray_length = fabs( (map_y - ray_pos.y + (1 - step_y) / 2) / ray_dir.y );
                texel_normal = ray_pos.x + ray_length * ray_dir.x;
            }

            // the fractional part of texel_normal is the normalised column on the texture
            texel_normal -= floor(  texel_normal );

            // the height of the wall on screen depends on its distance from the player
            // (ray_length)

            int column_height       = abs( (int)( RES_H / ray_length ) );

            // get height above horizon
            int column_start        = -column_height / 2 + RES_H / 2;
            // get height below horizon
            int column_end          =  column_height / 2 + RES_H / 2;

            // the texture index, -1 as map walls start at 1, not 0
            int tex = WORLD_MAP[map_y][map_x] - 1;

            GRA_Draw_Vertical_Texture_Line( texel_normal, column_index, column_start, 
                                            column_end, tex );
        }
    }            

    return;
}



// initialize global variables and structs
void Init_Globals()
{
    player_dir          = DIRECTION_UP;
    player_screen       = DIRECTION_RIGHT;

    return;
}
