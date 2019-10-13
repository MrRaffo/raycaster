/*
    vecmat.c

    contains all vector and matrix maths
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>         // for memset
#include <math.h>

#include "vecmat.h"

//===================================================================
//  CREATE FUNCTIONS
//===================================================================

// returns 2d vector
vector2d_type VEC_Vector( float x, float y )
{
    vector2d_type vec = { x, y };
    return vec;
}

// returns pointer to 3x3 identity matrix
matrix2d_type *VEC_Identity_Matrix()
{
    matrix2d_type *mat = malloc( sizeof( matrix2d_type ) );

    // set all elements to 0
    memset( mat, 0, sizeof( matrix2d_type ) );

    mat->m[0][0] = 1.0;
    mat->m[1][1] = 1.0;
    mat->m[2][2] = 1.0;

    return mat;
}

//===================================================================
// VECTOR MATHS
//===================================================================

// adds vector2 to vector1
vector2d_type VEC_Vector_Addition( vector2d_type vec1, vector2d_type vec2 )
{   
    vector2d_type newvec;
    newvec.x = vec1.x + vec2.x;
    newvec.y = vec1.y + vec2.y;

    return newvec;
}

// subtracts vec2 from vec1
vector2d_type VEC_Vector_Subtraction( vector2d_type vec1, vector2d_type vec2 )
{
    vector2d_type newvec;
    newvec.x = vec1.x - vec2.x;
    newvec.y = vec1.y - vec2.y;

    return newvec;
}

// scales a vector
vector2d_type VEC_Scale_Vector( vector2d_type vec, float scale )
{
    vec.x *= scale;
    vec.y *= scale;

    return vec;
}


// returns vector magnitude
float VEC_Get_Vector_Magnitude( vector2d_type vec )
{
    return sqrt( vec.x * vec.x + vec.y * vec.y );
}

// returns vector dot product
float VEC_Vector_Dot_Product( vector2d_type vec )
{
    return vec.x * vec.x + vec.y * vec.y;
}

// returns the vector normal
vector2d_type VEC_Vector_Normal( vector2d_type vec )
{
    float mag = VEC_Get_Vector_Magnitude( vec );
    vec.x /= mag;
    vec.y /= mag;

    return vec;
}

//===================================================================
// MATRIX TRANSFORMATIONS
//===================================================================

// scale matrix along x and y axes
void VEC_Matrix_Scale( matrix2d_type *mat, float xscale, float yscale )
{
    mat->m[0][0] *= xscale;
    mat->m[1][1] *= yscale;
    mat->m[2][2] *= xscale;
}

// translate matrix along x and y axes
void VEC_Matrix_Translate( matrix2d_type *mat, float x, float y )
{
    mat->m[0][2] += x;
    mat->m[1][2] += y;
}

// multiply a matrix by another matrix
void VEC_Matrix_Dot_Product( matrix2d_type *mat1, matrix2d_type *mat2 )
{
    int i, j;
    matrix2d_type temp = *mat1;
    for( i = 0; i < 3; i++ )
    {
        for( j = 0; j < 3; j++ )
        {
            mat1->m[i][j] = temp.m[i][0] * mat2->m[0][j] +
                            temp.m[i][1] * mat2->m[1][j] +
                            temp.m[i][2] * mat2->m[2][j];
        }
    }

    return;
}

void VEC_Matrix_Rotation( matrix2d_type *mat, float angle )
{
    matrix2d_type temp;
    
    temp.m[0][0] = cos( angle );    temp.m[0][1] = -sin( angle );   temp.m[0][2] = 0.0f;
    temp.m[1][0] = sin( angle );    temp.m[1][1] = cos( angle );    temp.m[1][2] = 0.0f;
    temp.m[2][0] = 0.0f;            temp.m[2][1] = 0.0f;            temp.m[2][2] = 1.0f;

    VEC_Matrix_Dot_Product( mat, &temp );
}


// transform a vector by a matrix
vector2d_type VEC_Matrix_Transform_Vector( matrix2d_type *mat1, vector2d_type vec )
{
    vector2d_type temp;

    temp.x = vec.x * mat1->m[0][0] + vec.y * mat1->m[0][1] + mat1->m[0][2];
    temp.y = vec.x * mat1->m[1][0] + vec.y * mat1->m[1][1] + mat1->m[1][2];

    return temp;
}


//===================================================================
// DEBUGGING FUNCTIONS
//===================================================================

void VEC_Print_Vector( vector2d_type vec )
{
    printf( "Vector:\t( %.2f, %.2f)\n", vec.x, vec.y );
}

void VEC_Print_Matrix( matrix2d_type *mat )
{
    int i, j;

    printf( "Matrix:\n" );

    for( i = 0; i < 3; i++ )
    {
        for( j = 0; j < 3; j++ )
        {
            printf( "\t%.2f", mat->m[i][j] );
        }
        printf( "\n" );
    }

    return;
}


