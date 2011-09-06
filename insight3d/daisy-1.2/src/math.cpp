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

#include "kutility/math.h"

namespace kutility
{
   int  digit_number(int num)
   {
      if( num == 0 ) return 1;

      int counter = 0;
      while( num != 0 )
      {
         num /= 10;
         counter++;
      }

      return counter;
   }

   /// miny-maxy : the minimum and maximum interval for the y axis.
   /// rate      : the rate at which the sigmoid reaches maxy from miny.
   /// sym_axis  : symmetry axis in the x axis. sig(sx-d)+sig(sx+d) = maxy:
   ///             sum of the y values from the symmetry point makes maxy.
   float sigmoid(float x, float miny, float maxy, float rate, float sym_axis)
   {
      float xp = exp(rate*(x-sym_axis));
      return (maxy - miny) * xp / ( xp + 1 ) + miny;
   }

   float* gaussian_1d(int fsz, float sigma, float mean )
   {
      float* fltr = new float[fsz];

      int sz = (fsz-1)/2;

      int counter=-1;
      float sum = 0.0;
      float v = 2*sigma*sigma;
      for( int x=-sz; x<=sz; x++ )
      {
         counter++;
         fltr[counter] = exp((-(x-mean)*(x-mean))/v);
         sum += fltr[counter];
      }

      if( sum != 0 )
      {
         for( int x=0; x<fsz; x++ )
            fltr[x] /= sum;
      }

      return fltr;
   }

   float* gaussian_2d(int fsz, float sigma, float mn)
   {
      int fltr_size = fsz * fsz;
      float* fltr = new float[fltr_size];

      int sz = (fsz-1)/2;

      int y,x;
      int counter=-1;
      float sum=0;

      float v = 2*sigma*sigma;

      for( y=-sz; y<=sz; y++ )
      {
         for( x=-sz; x<=sz; x++ )
         {
            counter++;
            fltr[counter] = exp((-(x)*(x-mn)-(y-mn)*(y-mn))/v);
            sum += fltr[counter];
         }
      }

      if( sum != 0 )
      {
         for( x=0; x<fltr_size; x++ )
            fltr[x] /= sum;
      }

      return fltr;
   }

   bool* and_array( bool* a, bool* b, int sz)
   {
      bool* c = kutility::allocate<bool>(sz);
      for( int i=0; i<sz; i++ )
         c[i] = a[i] & b[i];
      return c;
   }

   bool* or_array( bool* a, bool* b, int sz)
   {
      bool* c = kutility::allocate<bool>(sz);
      for( int i=0; i<sz; i++ )
         c[i] = a[i] | b[i];
      return c;
   }


   bool** mark( int **arr, int lsz, int& my, int& mx, int range )
   {
      int x, y;
      int minV = min(arr, lsz, 2, x, y);
      int maxV = max(arr, lsz, 2, x, y);

      int sz = maxV-minV+1;

      if( range != -1 && range > sz)
      {
         sz = range;
         if( minV < 0 )
            minV = -range/2;
         else
            minV = 0;
      }

      bool** mask = allocate<bool>(sz,sz);
      initialize(mask,sz,sz);

      my = sz;
      mx = sz;

      for( int i=0; i<lsz; i++ )
      {
         x = arr[i][0]-minV;
         y = arr[i][1]-minV;

         mask[y][x] = 1;
      }
      return mask;
   }
}


