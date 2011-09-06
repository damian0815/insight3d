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
//                                                                            //
// Written and (C) by Engin Tola                                              //
// WWW: http://cvlab.epfl.ch/~tola/                                           //
// Contact <engin.tola@epfl.ch> for comments & bug reports                    //
//                                                                            //
//                                                                            //
// If you use this piece of code for research purposes, please refer          //
// to the webpage above                                                       //
////////////////////////////////////////////////////////////////////////////////

#include "kutility/fileio.h"

namespace kutility
{

   /// loads an image and returns IplImage* its width "w" and its
   /// height "h" it uses OpenCV.
   IplImage* load_ipl_image(const char *file_name)
   {
      IplImage*  im = cvLoadImage( file_name, 0 );

      if( im == NULL )
      {
         kutility::error("could not load the file:", file_name);
      }

      return im;
   }

   /// loads the image using OpenCV's cvLoadImage function and copies
   /// the contents of the image to a uchar* and returns image size in
   /// w & h
   uchar* load_byte_image(const char *file_name, int &w, int &h)
   {
      // load image in grayscale!
      IplImage* im = cvLoadImage( file_name, 0 );

      if( im == NULL )
      {
         kutility::error("could not load the file:", file_name);
      }

      w = im->width;
      h = im->height;

      uchar* new_image = new uchar[w*h];

//       memcpy(new_image, (uchar*)im->imageData, sizeof(uchar)*w*h);

      int ws = im->widthStep;

      for( int y=0; y<h; y++ )
      {
         memcpy( &(new_image[y*w]), &(im->imageData[y*ws] ), w*sizeof(uchar) );
      }

      cvReleaseImage(&im);
      return new_image;
   }

   /// returns the imageData element of the IplImage structure by
   /// casting it to uchar*. RELEASE this memory.
   uchar* get_byte_from_iplimage(IplImage* img)
   {
      if( img->width%4 != 0 )
      {
         kutility::warning("Image size is not a multiple of 4. There may be trouble because of OpenCV data format\n");
      }

      uchar* new_image = allocate<uchar>( img->width * img->height );

      size_t index=0;
      for( int y=0; y<img->height; y++ )
      {
         for( int x=0; x<img->width; x++ )
         {
            new_image[ index ] = img->imageData[ y*img->widthStep+x ];
            index++;
         }
      }

      return new_image;
   }

   /// copies the image data of the IplImage to a uchar* and returns
   /// image size in w and h. The returned image data is at a separate
   /// memory location and you can release it.
   uchar* retrieve_byte_from_iplimage(IplImage* img, int &w, int &h)
   {
      if( img->width%4 != 0 )
      {
         kutility::warning("Image size is not a multiple of 4. There may be trouble because of OpenCV data format\n");
      }

      w = img->width;
      h = img->height;

      int channel = img->nChannels;

      uchar* new_image = new uchar[w*h*channel];
      memcpy(new_image, (uchar*)img->imageData, sizeof(uchar)*w*h*channel);

      return new_image;
   }

   /// loads a ppm image and returns a pointer to the image data. It
   /// also returns image size and channel props in w, h, and bit
   /// params
   uchar* load_ppm_image( string file_name, int &w, int&h, int &bit)
   {
      FILE *fp;

      uchar* data = NULL;

      int P,x,y,z,v;
      int Ascii=1;

      char TBuf[100];

      // default values
      w = h = bit = 0;

      fp = fopen(file_name.c_str(),"r");

      if( fp == NULL )
      {
         kutility::error("load_ppm_image: could not load the file:", file_name);
      }

      // Read header
      if( fgets(TBuf,100,fp)!=TBuf )
      {
         kutility::error("load_ppm_image: could not load the file:", file_name);
      }

      TBuf[2] = '\0';

           if( strcmp(TBuf,"P5")==0 ) { Ascii=0;bit=1; }	// pgm raw
      else if( strcmp(TBuf,"P6")==0 ) { Ascii=0;bit=3; }	// ppm raw
      else if( strcmp(TBuf,"P2")==0 ) { Ascii=1;bit=1; }	// pgm ascii
      else if( strcmp(TBuf,"P3")==0 ) { Ascii=1;bit=3; }	// ppm ascii
      else {
         kutility::error("load_ppm_image: unsupported PNM format for", file_name);
      }

      // Skip comments
      do
      {
         if( fgets(TBuf,100,fp)!=TBuf )
            kutility::error("load_ppm_image: could not load the file:", file_name);
      }
      while( TBuf[0]=='#' );

      // xres/yres
      sscanf(TBuf,"%d %d",&w,&h);

      // Number of gray , usually 255, Just skip it
      if( fgets(TBuf,100,fp)!=TBuf )
         kutility::error("load_ppm_image: could not load the file:", file_name);

      // Allocate data space , as char data... for now
      data = new uchar[w*h*bit];

      if( data==NULL )
      {
         kutility::error("load_ppm_image: out of memory");
      }

      if( Ascii==0 )
      {
         for(y=0; y<h; y++)
         {
#ifdef INVERSE_Y
            P=(h-1-y)*w*bit;
#else
            P=y*w*bit;
#endif
            fread(data+P,1,w*bit,fp);
         }
      }
      else
      {
         for(y=0; y<h;  y++)
            for(x=0; x<w;  x++)
               for(z=0; z<bit;z++)
               {
#ifdef INVERSE_Y
                  P=((h-1-y)*w+x)*bit+z;
#else
                  P=(y*w+x)*bit+z;
#endif
                  fscanf(fp," %d",&v);
                  data[P]=v;
               }
      }
      fclose(fp);

      return data;
   }

   /// loads an array given its size and data type. supported data
   /// types are int and double. "type" should be set to 0 for integer
   /// and to 1 for a double data. By default it is set to double.
   void* load_array( string filename, int size, int type )
   {
      if( type > 2 )
      {
         kutility::error("load_array: unsupported type", filename);
      }

      FILE* fp = fopen(filename.c_str(),"r");

      if( fp == NULL )
      {
         kutility::error("load_array: unable to open ", filename);
      }

      double * d_data=NULL;
      float  * f_data=NULL;
      int    * n_data=NULL;

      double d;
      float  f;
      int    n;

      int scanf_ret = 0;

      if( type == 0 ) n_data = new int   [size];
      if( type == 1 ) d_data = new double[size];
      if( type == 2 ) f_data = new float [size];

      for( int i=0; i<size; i++ )
      {
         if( type == 0 ) scanf_ret = fscanf(fp," %d" ,&n);
         if( type == 1 ) scanf_ret = fscanf(fp," %lg", &d);
         if( type == 2 ) scanf_ret = fscanf(fp," %f" ,&f);

         if( scanf_ret != 1 ) break;

         if( type == 0 ) n_data[i] = n;
         if( type == 1 ) d_data[i] = d;
         if( type == 2 ) f_data[i] = f;
      }
      fclose(fp);

      if( type == 0 ) return (void *)n_data;
      if( type == 1 ) return (void *)d_data;
      if( type == 2 ) return (void *)f_data;
      return NULL;
   }

   void convert_hex(int number, int* hex_array)
   {
      for(int i=0; i<4; i++)
      {
         hex_array[i] = number%256;
         number = number/256;
      }
   }

   /// waits for an input from the console.
   void wait_key()
   {
      char c;
      std::cout<<"\nkey in an input to continue ";
      std::cin>>c;
   }

}
