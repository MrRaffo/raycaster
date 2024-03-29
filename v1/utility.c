/*
    utility.c
    useful functions
*/

#include <stdio.h>
#include <stdlib.h>

#include "utility.h"

// prints error message then closes program
void UTI_Fatal_Error( char *msg )
{
    printf( "FATAL ERROR: %s\n", msg );
    exit( 1 );
}


// error checked malloc call
void *UTI_EC_Malloc( size_t size )
{
    void *ptr = malloc( size );
    if( ptr == NULL )
    {
        UTI_Fatal_Error( "<UTI_EC_Malloc>: Unable to allocate memory" );
    }

    return ptr;
}
