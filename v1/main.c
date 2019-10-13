/*
    main.c
    a template file that sets up an SDL display
*/

#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "utility.h"
#include "quickscreen.h"
#include "vecmat.h"

#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768

#define SCREEN_HORIZONTAL_RESOLUTION            320
#define SCREEN_VERTICAL_RESOLUTION              240

#define MAX_FRAME_TIME      16          // approx 60 fps

#define WORLD_WIDTH     16
#define WORLD_HEIGHT    16

//========================
//  BASIC COLOURS
//========================

colorRGBA_type BLACK        = { 0x00, 0x00, 0x00, 0xff };
colorRGBA_type GREY         = { 0x40, 0x40, 0x40, 0xff };
colorRGBA_type RED          = { 0xff, 0x00, 0x00, 0xff };
colorRGBA_type GREEN        = { 0x00, 0xff, 0x00, 0xff };
colorRGBA_type BLUE         = { 0x00, 0x00, 0xff, 0xff };
colorRGBA_type CYAN         = { 0x00, 0xff, 0xff, 0xff };
colorRGBA_type YELLOW       = { 0xff, 0xff, 0x00, 0xff };

colorRGBA_type DARK_RED     = { 0x80, 0x00, 0x00, 0xff };
colorRGBA_type DARK_GREEN   = { 0x00, 0x80, 0x00, 0xff };
colorRGBA_type DARK_BLUE    = { 0x00, 0x00, 0x80, 0xff };
colorRGBA_type DARK_YELLOW  = { 0x80, 0x80, 0x00, 0xff };


int world_map[WORLD_WIDTH][WORLD_HEIGHT] =
{
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
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

//===============================================
//  MAIN
//===============================================

int main( int argc, char *argv[] )
{
    // ARGUMENT AND ERROR CHECKING HERE

    if( QUI_Create_Display( "Raycasting", SCREEN_WIDTH, SCREEN_HEIGHT, 
                        SCREEN_HORIZONTAL_RESOLUTION, SCREEN_VERTICAL_RESOLUTION ) == 0 )
    {
        UTI_Fatal_Error( "Unable to create display" );
    }

    if( QUI_Load_Media() == 0 )
    {
        UTI_Fatal_Error( "Unable to load media" );
    }

    matrix2d_type matrix, *identity;
    identity = VEC_Identity_Matrix();

    // player vectors
    vector2d_type player_position   = {  8.0,  5.0 };
    vector2d_type player_direction  = { -1.0,  0.0 };
    vector2d_type screen_plane      = {  0.0,  1.0 };
    float current_line              = 0.0;
    float angle_delta               = 0.01;

    int time = 0;
    int old_time = 0;
    int frame_time = 0;

    // loop control
    int running = 1;
    
    while( running != 0 )
    {
        running = QUI_Check_Quit();

        QUI_Clear_Screen( GREY );

        int i;
        vector2d_type ray_pos, ray_dir;

        matrix = *identity;

        VEC_Matrix_Rotation( &matrix, angle_delta );
        player_direction = VEC_Matrix_Transform_Vector( &matrix, player_direction );
        screen_plane = VEC_Matrix_Transform_Vector( &matrix, screen_plane );

        for( i = 0; i < SCREEN_HORIZONTAL_RESOLUTION; i++ )
        {
            current_line = 2 * i / (double)( SCREEN_HORIZONTAL_RESOLUTION ) - 1;          // line on the screen
            ray_pos.x = player_position.x;
            ray_pos.y = player_position.y;
            
            ray_dir = VEC_Scale_Vector( screen_plane, current_line );
            ray_dir = VEC_Vector_Addition( player_direction, ray_dir );

            float xDist, yDist, xDeltaDist, yDeltaDist, wallDist;
            
            xDeltaDist = sqrt( 1 + ( ray_dir.y * ray_dir.y ) / ( ray_dir.x * ray_dir.x ) );
            yDeltaDist = sqrt( 1 + ( ray_dir.x * ray_dir.x ) / ( ray_dir.y * ray_dir.y ) );

            int stepx, stepy, wallhit, walltype, mapx, mapy;

            mapx = (int)player_position.x;
            mapy = (int)player_position.y;

            if( ray_dir.x < 0 )
            {
                stepx = -1;
                xDist = ( ray_pos.x - mapx ) * xDeltaDist;
            }
            else
            {
                stepx = 1;
                xDist = ( mapx + 1.0 - ray_pos.x ) * xDeltaDist;
            }

            if( ray_dir.y < 0 )
            {
                stepy = -1;
                yDist = ( ray_pos.y - mapy ) * yDeltaDist;
            }
            else
            {
                stepy = 1;
                yDist = ( mapy + 1.0 - ray_pos.y ) * yDeltaDist;
            }

            wallhit = 0;

            while( wallhit == 0 && mapx < WORLD_WIDTH && mapy < WORLD_HEIGHT )
            {
                if( xDist < yDist )
                {
                    xDist += xDeltaDist;
                    mapx += stepx;
                    walltype = 0;
                }
                else
                {
                    yDist += yDeltaDist;
                    mapy += stepy;
                    walltype = 1;
                }

                if( world_map[mapx][mapy] > 0 ) 
                {
                    wallhit = 1;
                }
            }

            if( walltype == 0 )
            {
                wallDist = fabs( ( mapx - ray_pos.x + ( 1 - stepx ) / 2 ) / ray_dir.x );
            }
            else
            {
                wallDist = fabs( ( mapy - ray_pos.y + ( 1 - stepy ) / 2 ) / ray_dir.y );
            }

            int lineheight  = abs( (int)( SCREEN_VERTICAL_RESOLUTION / wallDist ) );
            int linestart   = -lineheight / 2 + SCREEN_VERTICAL_RESOLUTION / 2;
            if( linestart < 0 )
            {
                linestart = 0;
            }
            
            int lineend     = lineheight / 2 + SCREEN_VERTICAL_RESOLUTION / 2;
            if( lineend > SCREEN_VERTICAL_RESOLUTION )
            {
                lineend = SCREEN_VERTICAL_RESOLUTION - 1;
            }

            colorRGBA_type color = RED;

            color.r = (unsigned char)(0x100 - (wallDist / 16.0 * 0x100));

            QUI_Draw_Vertical_Line( i, linestart, lineend, color );
        }   
        
        
        QUI_Simple_Text( "This is text", 8, 8, BLUE, BLACK, 0 );

        // timing
        old_time = time;
        time = QUI_GetTicks();
        frame_time = time - old_time;
        if( frame_time < MAX_FRAME_TIME )
        {
            QUI_Delay( MAX_FRAME_TIME - frame_time );
        }
 
        QUI_Refresh_Screen();
    }
    

    QUI_Close();

    return 0;
}
