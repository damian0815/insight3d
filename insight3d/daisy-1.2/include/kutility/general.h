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

#ifndef KUTILITY_GENERAL_H
#define KUTILITY_GENERAL_H

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "kutility/kutility.def"
#include "kutility/interaction.h"

namespace kutility
{
   /// increases the size of the array from size to nsize. does not make any sanity check.
   template<class T> inline
   void expand_array( T* &array, int size, int nsize )
   {
      T* out = new T[nsize];
      memcpy( out, array, size*sizeof(T) );
      delete []array;
      array = out;
   }

   /// allocates a memory of size sz and returns a pointer to the array
   template<class T> inline
   T* allocate(const int sz)
   {
      T* array = new T[sz];
      return array;
   }

   /// allocates a memory of size ysz x xsz and returns a double pointer to it
   template<class T> inline
   T** allocate(const int ysz, const int xsz)
   {
      T** mat = new T*[ysz];
      int i;

      for(i=0; i<ysz; i++ )
         mat[i] = new T[xsz];
         // allocate<T>(xsz);

      return mat;
   }

   /// deallocates the memory and sets the pointer to null.
   template<class T> inline
   void deallocate(T* &array)
   {
      delete[] array;
      array = NULL;
   }

   /// deallocates the memory and sets the pointer to null.
   template<class T> inline
   void deallocate(T** &mat, int ysz)
   {
      if( mat == NULL ) return;

      for(int i=0; i<ysz; i++)
         deallocate(mat[i]);

      delete[] mat;
      mat = NULL;
   }

   /// makes a clone of the source array.
   template<class T> inline
   T* clone( T* src, int sz)
   {
      T* dst = kutility::allocate<T>(sz);
      memcpy( dst, src, sizeof(T)*sz);
      return dst;
   }

   /// makes a clone of the source matrix.
   template<class T> inline
   T** clone( T** src, int r, int c)
   {
      T** dst = kutility::allocate<T>(r,c);

      for( int i=0; i<r; i++ )
         memcpy( dst[i], src[i], sizeof(T)*c);
      return dst;
   }

   /// makes a copy of the source array.
   template<class T> inline
   void copy( T* dst, T* src, int sz)
   {
      memcpy( dst, src, sizeof(T)*sz);
   }

   /// makes a copy of the source matrix.
   template<class T> inline
   void copy( T** dst, T** src, int ysz, int xsz)
   {
      int y;

      for( y=0; y<ysz; y++ )
         memcpy( dst[y], src[y], sizeof(T)*xsz);
   }

   /// casts a type T2 array into a type T1 array.
   template<class T1, class T2> inline
   T1* type_cast(T2* data, int sz)
   {
      T1* out = new T1[sz];

      for( int i=0; i<sz; i++ )
         out[i] = (T1)data[i];

      return out;
   }

   /// casts a type T2 matrix into a type T1 matrix
   template<class T1, class T2> inline
   T1** type_cast(T2** data, int ysz, int xsz)
   {
      T1* out = kutility::allocate<T1>(ysz,xsz);

      for( int y=0; y<ysz; y++ )
      for( int x=0; x<xsz; x++ )
         out[y][x] = (T1)data[y][x];

      return out;
   }

   char* strrev(char* szT);

   /// converts a number to an array.
   char* itoa(int value, char* str, int radix);

   /// converts an integer into a string.
   inline std::string num2str( int n )
   {
      char buf[10];
      itoa(n,buf,10);
      std::string retval = buf;
      return retval;
   }

   /// initializes the array arr with value=val
   template<class T> inline
   void initialize(T* &arr, int sz, unsigned char val=0)
   {
      if( arr == NULL ) kutility::error("you should allocate memory first");
      for( int i=0; i<sz; i++ )
         arr[i] = val;
   }

   /// initializes the matrix mat with value=val
   template<class T> inline
   void initialize(T** &mat, int ysz, int xsz, unsigned char val=0)
   {
      if( mat == NULL ) error("you should allocate memory first");

      for( int i=0; i<ysz; i++ )
         initialize(mat[i], xsz, val);
   }

   template<class T> inline
   T precision(T num, int prec)
   {
      double mult = pow(10.0,prec);

      T tmp = (T)floor(mult*num);
      tmp /= mult;
      return tmp;
   }

   template<class T>
   T* precision(T* arr, int sz, int prec, bool in_place=true)
   {
      T* out;

      if( in_place ) out = arr;
      else           out = new T[sz];

      double q = pow(10.0, prec);

      for(int i=0; i<sz; i++)
         out[i] = kutility::precision( arr[i], prec );

      return out;
   }

   template<class T>
   T** precision(T** arr, int r, int c, int prec, bool in_place=true)
   {
      T** out;

      if( in_place ) out = arr;
      else           out = kutility::allocate<T>(r,c);

      double q = pow(10.0, prec);

      int rr,cc;

      for( rr=0; rr<r; rr++ )
         for( cc=0; cc<c; cc++ )
            out[rr][cc] = kutility::precision(arr[rr][cc], prec);

      return out;
   }

   /// finds the minimum and returns the value and its index.
   /// index is returned in the xmn parameter.
   template<class T> inline
   T min(T* data, int sz, int &xmn)
   {
      T minVal=data[0];
      xmn = 0;

      for(int i=1; i<sz; i++ )
      {
         if( minVal > data[i] )
         {
            minVal   = data[i];
            xmn = i;
         }
      }
      return minVal;
   }

   /// finds the minimum and returns the value and its index.
   /// index is returned in the xmn & ymn parameters.
   template<class T> inline
   T min(T** data, int ysz, int xsz, int &ymn, int &xmn)
   {
      T minVal = data[0][0];
      xmn = 0;
      ymn = 0;
      int minx;

      T mn;

      for( int y=0; y<ysz; y++ )
      {
         mn = min(data[y], xsz, minx);

         if( mn < minVal )
         {
            minVal = mn;
            ymn = y;
            xmn = minx;
         }
      }
      return minVal;
   }

   /// finds the maximum and returns the value and its index.
   /// index is returned in the xmx parameter.
   template<class T> inline
   T max(T* data, int sz, int &xmx)
   {
      T maxVal=data[0];
      xmx = 0;

      for(int i=1; i<sz; i++ )
      {
         if( maxVal < data[i] )
         {
            maxVal   = data[i];
            xmx = i;
         }
      }
      return maxVal;
   }

   /// finds the maximum and returns the value and its index.
   /// index is returned in the xmx and ymx parameters.
   template<class T> inline
   T max(T** data, int ysz, int xsz, int &ymx, int &xmx)
   {
      T maxVal = data[0][0];
      xmx = 0;
      ymx = 0;

      int maxx;
      T mx;

      for( int y=0; y<ysz; y++ )
      {
         mx = max(data[y], xsz, maxx);

         if( mx > maxVal )
         {
            maxVal = mx;
            ymx = y;
            xmx = maxx;
         }
      }
      return maxVal;
   }

   /// compares two arrays and returns the maximum elements
   /// if in_place = true returns the result in the first array
   template<class T> inline
   T* max( T* arr_0, T* arr_1, size_t sz, bool in_place=false )
   {
      T* result = NULL;
      if( in_place )
         result = arr_0;
      else
         result = allocate<T>(sz);

      T* p0 = arr_0;
      T* p1 = arr_1;
      T* r  = result;

      for( int i=0; i<sz; i++ )
      {
         if( *p0 > *p1 ) *r = *p0;
         else            *r = *p1;

         p0++;
         p1++;
         r++;
      }
      return result;
   }

   /// finds the interval index the number is in between.
   /// "equality" specifies the use of = or not.
   /// equality = 0 -> NN <  <  |
   /// equality = 1 -> NE <  <= |
   /// equality = 2 -> EN <= <  |
   /// equality = 3 -> EE <= <= |
   template<class T> inline
   int find_interval( T number, T** list, int lsz, int equality )
   {

      for( int i=0; i<lsz; i++ )
      {
         switch(equality)
         {
         case 0: // NN
            if( is_inside( number, list[i][0], list[i][1], 0, 0) )
               return i;
            break;
         case 1: // NE
            if( is_inside( number, list[i][0], list[i][1], 0, 1) )
               return i;
            break;
         case 2: // EN
            if( is_inside( number, list[i][0], list[i][1], 1, 0) )
               return i;
            break;
         case 3: // EE
            if( is_inside( number, list[i][0], list[i][1], 1, 1) )
               return i;
            break;
         default:
            return -1;
            break;
         }
      }
      return -1;
   }


   /// finds the interval index the number is in between.
   /// "equality" specifies the use of = or not.
   /// equality = 0 -> NN <  <  |
   /// equality = 1 -> NE <  <= |
   /// equality = 2 -> EN <= <  |
   /// equality = 3 -> EE <= <= |
   template<class T> inline
   int find_interval( T nx, T ny, T** list, int lsz, int equality )
   {
      for( int i=0; i<lsz; i++ )
      {
         switch(equality)
         {
         case 0: // NN
            if( is_inside( nx, list[i][0], list[i][1], ny, list[i][2], list[i][3], 0, 0) )
               return i;
            break;
         case 1: // NE
            if( is_inside( nx, list[i][0], list[i][1], ny, list[i][2], list[i][3], 0, 1) )
               return i;
            break;
         case 2: // EN
            if( is_inside( nx, list[i][0], list[i][1], ny, list[i][2], list[i][3], 1, 0) )
               return i;
            break;
         case 3: // EE
            if( is_inside( nx, list[i][0], list[i][1], ny, list[i][2], list[i][3], 1, 1) )
               return i;
            break;
         default:
            return -1;
            break;
         }
      }
      return -1;
   }

   inline bool is_digit( char c )
   {
      for( int i=0; i<10; i++ )
         if( c == num2str(i)[0] )
            return true;
      return false;
   }

   inline bool is_number( std::string str )
   {
      int len=str.length();

      for( int i=0; i<len; i++)
      {
         if( is_digit(str[i]) || str[i] == '.' || str[i] == '-' )
            continue;
         else
            return false;
      }
      return true;
   }

   inline bool is_positive_number( std::string str )
   {
      if( !is_number( str ) ) return false;

      float number = atof( str.c_str() );

      if( number > 0.0 ) return true;

      return false;
   }

   inline bool is_negative_number( std::string str )
   {
      if( !is_number( str ) ) return false;

      float number = atof( str.c_str() );

      if( number < 0.0 ) return true;

      return false;
   }

   inline bool is_integer( std::string str )
   {
      int len=str.length();

      for( int i=0; i<len; i++)
      {
         if( is_digit(str[i]) || str[i] == '-' )
            continue;
         else
            return false;
      }
      return true;
   }

   inline bool is_positive_integer( std::string str )
   {
      if( !is_integer( str ) ) return false;

      int number = atoi( str.c_str() );

      if( number > 0 ) return true;

      return false;
   }

   inline bool is_negative_integer( std::string str )
   {
      if( !is_integer( str ) ) return false;

      int number = atoi( str.c_str() );

      if( number < 0 ) return true;

      return false;
   }

   inline void set_integer( int &location, std::string str, std::string param_name="")
   {
      if( !is_integer( str ) )
      {
         std::string errout = param_name + " should be an integer. but it is " + str;
         std::cout<<errout<<std::endl;
         exit(1);
      }
      location = atoi( str.c_str() );
   }

   inline void set_positive_integer( int &location, std::string str, std::string param_name="")
   {
      if( !is_positive_integer( str ) )
      {
         std::string errout;
         if( param_name == "" )
            errout = "parameter should be a postive integer. but it is " + str;
         else
            errout = param_name + " should be a postive integer. but it is " + str;

         std::cout<<errout<<std::endl;
         exit(1);
      }
      location = atoi( str.c_str() );
   }

   /// returns true if file exists
   inline bool does_file_exists( std::string str )
   {
      std::ifstream outfile;
      outfile.open(str.c_str());
      if( outfile.is_open() )
      {
         outfile.close();
         return true;
      }
      else
      {
         outfile.close();
         return false;
      }
   }

   inline void convert_to_grayscale( uchar* cim, int h, int w, uchar* gim )
   {
      assert( (gim!=NULL) && (cim!=NULL) );

      for( int y=0; y<h; y++ )
      {
         for( int x=0; x<w; x++ )
         {
            int gindex=y*w+x;
            int cindex=3*gindex;
            gim[ gindex ] = (uchar)(( cim[ cindex ] + cim[ cindex+1 ] + cim[ cindex+2 ] )/3.0);
         }
      }
   }

   inline void convert_grayscale_to_ipl( uchar* image, int h, int w, IplImage* im )
   {
      assert( im != NULL );
      assert( (im->width==w) && (im->height==h) && (im->depth==IPL_DEPTH_8U) );

      int ws = im->widthStep;

      for( int yy=0; yy<h; yy++ )
      {
         memcpy( &(im->imageData[yy*ws]), &(image[yy*w]), sizeof( uchar ) * w );
      }
   }

   inline void convert_color_to_ipl( uchar* cim, int h, int w, IplImage* im )
   {
      assert( im != NULL );
      assert( (im->width==w) && (im->height==h) && (im->depth==IPL_DEPTH_8U) && (im->nChannels==3) );

      int ws = im->widthStep;

      for( int y=0; y<h; y++ )
      {
         for( int x=0; x<w; x++ )
         {
            im->imageData[ y*ws+x*3   ] = cim[ 3*(y*w+x)+2 ];
            im->imageData[ y*ws+x*3+1 ] = cim[ 3*(y*w+x)+1 ];
            im->imageData[ y*ws+x*3+2 ] = cim[ 3*(y*w+x) ];
         }
      }
   }

   inline void convert_color_to_gray_ipl( uchar* cim, int h, int w, IplImage* im )
   {
      assert( (cim!=NULL) && (im!=NULL) );
      assert( (im->width==w) && (im->height==h) && (im->depth==IPL_DEPTH_8U) && (im->nChannels==1) );

      int ws = im->widthStep;

      for( int y=0; y<h; y++ )
      {
         for( int x=0; x<w; x++ )
         {
            int cindex=3*(y*w+x);
            im->imageData[ y*ws+x ] = (uchar)(( cim[ cindex ] + cim[ cindex+1 ] + cim[ cindex+2 ] )/3.0);
         }
      }
   }
}

#endif
