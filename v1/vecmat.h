/*
    vecmat.h
*/

#ifndef __vecmat_h__
#define __vecmat_h__


//========================================================================
//  TYPES AND STRUCTS
//========================================================================

struct vector2d_s   {
                        float x;
                        float y;
                    };

// index of vertices to connect to draw a line
struct line2d_s     {
                        int v1;
                        int v2;
                    };

// a 3x3 matrix for 2d transformations
struct matrix2d_s   {
                        float m[3][3];
                    };

typedef struct vector2d_s   vector2d_type;
typedef struct line2d_s     line2d_type;
typedef struct matrix2d_s   matrix2d_type;

//========================================================================
//  FUNCTION PROTOTYPES
//========================================================================

// returns 2d vector
vector2d_type VEC_Vector( float x, float y );

// returns pointer to 3x3 identity matrix
matrix2d_type *VEC_Identity_Matrix();


//================================================================
// VECTOR MATHS
//================================================================

// adds vector2 to vector1
vector2d_type VEC_Vector_Addition( vector2d_type vec1, vector2d_type vec2 );

// subtracts vec2 from vec1
vector2d_type VEC_Vector_Subtraction( vector2d_type vec1, vector2d_type vec2 );

// scales a vector
vector2d_type VEC_Scale_Vector( vector2d_type vec, float scale );

// gets the vector normal
vector2d_type VEC_Vector_Normal( vector2d_type vector );

// get vector dot product
float VEC_Vector_Dot_Product( vector2d_type vec );

// get vector magnitude
float VEC_Get_Vector_Magnitude( vector2d_type vec );

//================================================================
// MATRIX TRANSFORMATIONS
//================================================================

// scale matrix along x and y axes
void VEC_Matrix_Scale( matrix2d_type *mat, float xscale, float yscale );

// translate matrix along x and y axes
void VEC_Matrix_Translate( matrix2d_type *mat, float x, float y );

// multiply a matrix by another matrix
void VEC_Matrix_Dot_Product( matrix2d_type *mat1, matrix2d_type *mat2 );

// rotate matrix by angle in radians
void VEC_Matrix_Rotation( matrix2d_type *mat, float angle );

// transform a vector by a matrix
vector2d_type VEC_Matrix_Transform_Vector( matrix2d_type *mat1, vector2d_type vec );

// DEBUGGING FUNCTIONS

void VEC_Print_Vector( vector2d_type vec );
void VEC_Print_Matrix( matrix2d_type *mat );

#endif // __vecmat_h__
