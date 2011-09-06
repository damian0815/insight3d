////////////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it                     //
// and/or modify it under the terms of the GNU General Public License         //
// version 2 (or higher) as published by the Free Software Foundation.        //
//                                                                            //
// This program is distributed in the hope that it will be useful, but        //
// WITHOUT ANY WARRANTY; without even the implied warranty of                 //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          //
// General Public License for more details.                                   //
//                                                                            //
// Written and (C) by                                                         //
// Engin Tola                                                                 //
//                                                                            //
// web   : http://cvlab.epfl.ch/~tola/                                        //
// email : engin.tola@epfl.ch                                                 //
//                                                                            //
// If you use this code for research purposes, please refer to the            //
// webpage above                                                              //
////////////////////////////////////////////////////////////////////////////////

#ifndef KUTILITY_MATRIX_MANIPULATION_H
#define KUTILITY_MATRIX_MANIPULATION_H

#include "cv.h"

namespace kutility
{
   template<class T>
   inline void transpose(T* m, int rn, int cn, T* mt)
   {
      for( int r=0; r<rn; r++ )
         for( int c=0; c<cn; c++ )
            mt[ c*rn+r ] = m[ r*cn+c ];
   }

   template<typename T1, typename T2, typename T3> inline
   void multiply_matrices(T1 *mat_1, int r1, int c1, T2 *mat_2, int r2, int c2, T3 *result)
   {
      if     ( r1 == 3 && c1 == 4 && c2 == 1 ) matrix_multiplication(mat_1, 3, 4, mat_2, 4, 1, result);
      else if( r1 == 3 && c1 == 3 && c2 == 4 ) matrix_multiplication(mat_1, 3, 3, mat_2, 3, 4, result);
      else if( r1 == 3 && c1 == 3 && c2 == 1 ) matrix_multiplication(mat_1, 3, 3, mat_2, 3, 1, result);
      else if( r1 == 3 && c1 == 3 && c2 == 3 ) matrix_multiplication(mat_1, 3, 3, mat_2, 3, 3, result);
      else if( r1 == 3 && c1 == 1 && c2 == 3 ) matrix_multiplication(mat_1, 3, 1, mat_2, 1, 3, result);
      else
         matrix_multiplication(mat_1, r1, c1, mat_2, r2, c2, result);
   }

   template<typename T1, typename T2, typename T3> inline
   void matrix_multiplication(T1 *mat_1, int r1, int c1, T2 *mat_2, int r2, int c2, T3 *result)
   {
      T3 res;
      for( int r=0; r<r1; r++ )
      {
         for( int c=0; c<c2; c++ )
         {
            res = 0;
            for( int k=0; k<c1; k++ )
            {
               res += mat_1[r*c1+k] * mat_2[k*c2+c];
            }

            result[r*c2+c] = T3(res);
         }
      }
   }

   /// decomposes a 3x3 A matrix into RQ where R is upper triangular and Q is a
   /// rotation matrix.
   template<class T> inline
   void rq_givens_decomposition( T* A, T* R, T* Q )
   {
      // set a32 = 0 by Qx
      T s =  A[7] / sqrt( A[7]*A[7] + A[8]*A[8] );
      T c = -A[8] / sqrt( A[7]*A[7] + A[8]*A[8] );

      T Qx[] = {1, 0, 0, 0, c, -s, 0, s, c };

      T AQx[9];
      multiply_matrices( A, 3, 3, Qx, 3, 3, AQx );

      // set a31 = 0 by Qy
      s = AQx[6] / sqrt( AQx[6]*AQx[6] + AQx[8]*AQx[8] );
      c = AQx[8] / sqrt( AQx[6]*AQx[6] + AQx[8]*AQx[8] );

      T Qy[] = {c, 0, s, 0, 1, 0, -s, 0, c };
      T AQxQy[9];
      multiply_matrices( AQx, 3, 3, Qy, 3, 3, AQxQy );

      // set a21 = 0 by Qz
      s =  AQxQy[3] / sqrt( AQxQy[3]*AQxQy[3] + AQxQy[4]*AQxQy[4] );
      c = -AQxQy[4] / sqrt( AQxQy[3]*AQxQy[3] + AQxQy[4]*AQxQy[4] );

      T Qz[] = {c, -s, 0, s, c, 0, 0, 0, 1 };

//       float AQxQyQz[9] =  R; is an upper triangular matrix
      multiply_matrices( AQxQy, 3, 3, Qz, 3, 3, R );

      T Qxt[9]; transpose(Qx, 3, 3, Qxt);
      T Qyt[9]; transpose(Qy, 3, 3, Qyt);
      T Qzt[9]; transpose(Qz, 3, 3, Qzt);

      T QztQyt[9];
      multiply_matrices(Qzt, 3, 3, Qyt, 3, 3, QztQyt );
      multiply_matrices( QztQyt, 3, 3, Qxt, 3, 3, Q );

      if( R[0] < 0 )
      {
         R[0] = -R[0];
         Q[0] = -Q[0];
         Q[1] = -Q[1];
         Q[2] = -Q[2];
      }
      if( R[4] < 0 )
      {
         R[4] = -R[4];
         Q[3] = -Q[3];
         Q[4] = -Q[4];
         Q[5] = -Q[5];
      }
      if( R[8] < 0 )
      {
         R[8] = -R[8];
         Q[6] = -Q[6];
         Q[7] = -Q[7];
         Q[8] = -Q[8];
      }
   }

   float*  null_vector( float*  matrix, int r, int c );
   double* null_vector( double* matrix, int r, int c );
   void null_vector( float*  matrix, int r, int c, float  *n_vector );
   void null_vector( double* matrix, int r, int c, double *n_vector );

   float*  pseudo_inverse( float*  matrix, int r, int c );
   double* pseudo_inverse( double* matrix, int r, int c );
   void pseudo_inverse( double *matrix, int r, int c, double *inv_matrix );
   void pseudo_inverse( float  *matrix, int r, int c, float  *inv_matrix );

   void invert( float*  matrix, int r, float*  inv_matrix );
   void invert( double* matrix, int r, double* inv_matrix );
}
#endif
