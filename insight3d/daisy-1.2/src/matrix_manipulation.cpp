/////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it              //
// and/or modify it under the terms of the GNU General Public License  //
// version 2 (or higher) as published by the Free Software Foundation. //
//                                                                     //
// This program is distributed in the hope that it will be useful, but //
// WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// General Public License for more details.                            //
//                                                                     //
// Written and (C) by                                                  //
// Engin Tola                                                          //
//                                                                     //
// web   : http://cvlab.epfl.ch/~tola                                  //
// email : engin.tola@epfl.ch                                          //
//                                                                     //
// If you use this code for research purposes, please refer to the     //
// webpage above                                                       //
/////////////////////////////////////////////////////////////////////////

#include "kutility/matrix_manipulation.h"

namespace kutility
{
   void invert( float* matrix, int r, float* inv_matrix )
   {
      CvMat M;
      cvInitMatHeader(&M, r, r, CV_32FC1, matrix);

      CvMat* invM = cvCreateMat(r,r, CV_32FC1);

      cvInvert( &M, invM );

      for( int i=0; i<r; i++ )
      {
         for( int j=0; j<r; j++ )
            inv_matrix[i*r+j] = float( invM->data.fl[ i*invM->width + j ] );
      }

      cvReleaseMat(&invM);
   }
   void invert( double* matrix, int r, double* inv_matrix )
   {
      CvMat M;
      cvInitMatHeader(&M, r, r, CV_64FC1, matrix);

      CvMat* invM = cvCreateMat(r,r, CV_64FC1);

      cvInvert( &M, invM );

      for( int i=0; i<r; i++ )
      {
         for( int j=0; j<r; j++ )
            inv_matrix[i*r+j] = double( invM->data.db[ i*invM->width + j ] );
      }

      cvReleaseMat(&invM);
   }

   void null_vector( float* matrix, int r, int c, float* n_vector )
   {
      CvMat M;
      cvInitMatHeader(&M, r, c, CV_32FC1, matrix);

      CvMat* U = cvCreateMat(r,r,CV_32FC1);
      CvMat* W = cvCreateMat(r,c,CV_32FC1);
      CvMat* V = cvCreateMat(c,c,CV_32FC1);

      cvSVD( &M, W, U, V, 0 );

      int mins = r < c ? r : c;

      for( int i=0; i<c; i++ )
      {
         n_vector[i] = float(V->data.fl[i*V->width + mins]);
      }

      cvReleaseMat(&U);
      cvReleaseMat(&W);
      cvReleaseMat(&V);
   }
   void null_vector( double* matrix, int r, int c, double* n_vector)
   {
      CvMat M;
      cvInitMatHeader(&M, r, c, CV_64FC1, matrix);

      CvMat* U = cvCreateMat(r,r,CV_64FC1);
      CvMat* W = cvCreateMat(r,c,CV_64FC1);
      CvMat* V = cvCreateMat(c,c,CV_64FC1);

      cvSVD( &M, W, U, V, 0 );

      int mins = r < c ? r : c;

      for( int i=0; i<c; i++ )
      {
         n_vector[i] = double(V->data.db[i*V->width + mins]);
      }

      cvReleaseMat(&U);
      cvReleaseMat(&W);
      cvReleaseMat(&V);
   }
   float* null_vector( float* matrix, int r, int c)
   {
      CvMat M;
      cvInitMatHeader(&M, r, c, CV_32FC1, matrix);

      CvMat* U = cvCreateMat(r,r,CV_32FC1);
      CvMat* W = cvCreateMat(r,c,CV_32FC1);
      CvMat* V = cvCreateMat(c,c,CV_32FC1);

      cvSVD( &M, W, U, V, 0 );

      int mins = r < c ? r : c;

      float* returnData = new float[c];

      for( int i=0; i<c; i++ )
      {
         returnData[i] = float(V->data.fl[i*V->width + mins]);
      }

      cvReleaseMat(&U);
      cvReleaseMat(&W);
      cvReleaseMat(&V);
      return returnData;
   }
   double* null_vector( double* matrix, int r, int c)
   {
      CvMat M;
      cvInitMatHeader(&M, r, c, CV_64FC1, matrix);

      CvMat* U = cvCreateMat(r,r,CV_64FC1);
      CvMat* W = cvCreateMat(r,c,CV_64FC1);
      CvMat* V = cvCreateMat(c,c,CV_64FC1);

      cvSVD( &M, W, U, V, 0 );

      int mins = r < c ? r : c;

      double* returnData = new double[c];

      for( int i=0; i<c; i++ )
      {
         returnData[i] = double(V->data.db[i*V->width + mins]);
      }

      cvReleaseMat(&U);
      cvReleaseMat(&W);
      cvReleaseMat(&V);
      return returnData;
   }

   void pseudo_inverse( double *matrix, int r, int c, double* inv_matrix )
   {
      CvMat M;
      cvInitMatHeader(&M, r, c, CV_64FC1, matrix);

      CvMat *out = cvCreateMat(c,r,CV_64FC1);
      cvInvert( &M, out, CV_SVD );

      for( int i=0; i<r*c; i++ )
         inv_matrix[i] = double(out->data.db[i]);
      cvReleaseMat(&out);
   }
   void pseudo_inverse( float *matrix, int r, int c, float* inv_matrix )
   {
      CvMat M;
      cvInitMatHeader(&M, r, c, CV_32FC1, matrix);

      CvMat *out = cvCreateMat(c,r,CV_32FC1);
      cvInvert( &M, out, CV_SVD );

      for( int i=0; i<r*c; i++ )
         inv_matrix[i] = float(out->data.fl[i]);
      cvReleaseMat(&out);
   }
   float* pseudo_inverse( float *matrix, int r, int c )
   {
      CvMat M;
      cvInitMatHeader(&M, r, c, CV_32FC1, matrix);

      CvMat *out = cvCreateMat(c,r,CV_32FC1);

      cvInvert( &M, out, CV_SVD );

      float* returnData = new float[r*c];

      for( int i=0; i<r*c; i++ )
      {
         returnData[i] = float(out->data.fl[i]);
      }

      cvReleaseMat(&out);

      return returnData;
   }
   double* pseudo_inverse( double *matrix, int r, int c )
   {
      CvMat M;
      cvInitMatHeader(&M, r, c, CV_64FC1, matrix);

      CvMat *out = cvCreateMat(c,r,CV_64FC1);

      cvInvert( &M, out, CV_SVD );

      double* returnData = new double[r*c];

      for( int i=0; i<r*c; i++ )
      {
         returnData[i] = double(out->data.db[i]);
      }

      cvReleaseMat(&out);

      return returnData;
   }

}
