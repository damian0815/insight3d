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

#ifndef KUTILITY_FILEIO_H
#define KUTILITY_FILEIO_H

#include "kutility/kutility.def"
#include "kutility/general.h"
#include "kutility/interaction.h"

using namespace std;

namespace kutility
{
   IplImage* load_ipl_image(const char *file_name);
   uchar* load_byte_image(const char *file_name, int &w, int &h);

   uchar* get_byte_from_iplimage(IplImage* img);
   uchar* retrieve_byte_from_iplimage(IplImage* img, int &w, int &h);

   uchar* load_ppm_image(const char* file_name, int &w, int&h, int &bit);

   template<class T>
   int load( string filename, T* &out, int size=-1 )
   {
      ifstream fin;
      fin.open( filename.c_str() );
      if( fin.fail() )
      {
         cout<<"no such file: "<<filename<<endl;
         exit(1);
      }

      int tsize;
      bool read_all = false;
      if( size <= 0 )
      {
         tsize = 100;
         read_all = true;
      }
      else
         tsize = size;

      if( out == NULL ) out = new T[tsize];

      int counter = 0;
      fin.peek();
      while( !fin.fail() )
      {
         float val;
         fin >> val;
         if( fin.fail() ) break;

         out[counter] = val;

         counter++;
         if( counter >= tsize )
         {
            if( read_all )
            {
               expand_array( out, tsize, 2*tsize );
               tsize *= 2;
            }
            else
               break;
         }
      }
      if( read_all ) expand_array( out, counter, counter );
      fin.close();

      if( size!= -1 && size != counter )
         cout<<"WARNING: I loaded only "<<counter<<" data points instead of "<<size<<"\n";

      return counter;
   }


   void* load_array( string filename, int size, int type=1 );
   void wait_key();

   /// saves an array given its size and the filename to save it. it
   /// uses a tab to seperate the data points if not specified
   /// otherwise.
   template<class T> inline void save(T* data, int sz, string filename, string sep="\t")
   {
      std::ofstream fout;
      fout.open( filename.c_str(), std::ofstream::out );

      for( int i=0; i<sz; i++ )
         fout<<data[i]<<sep;
      fout<<"\n";

      fout.close();
      return;
   }

   /// saves an array with indexes given its size and the filename to
   /// save it. it uses a tab to seperate the data points and the
   /// indexes if not specified otherwise.
   template<class T> inline void save_indexed(T* data, int sz, string filename, string sep="\t")
   {
      std::ofstream fout;
      fout.open( filename.c_str(), std::ofstream::out );

      for( int i=0; i<sz; i++ )
      {
         fout<<i<<sep<<data[i]<<std::endl;
      }
      fout<<"\n";

      fout.close();
      return;
   }


   /// saves an array in matrix format with rs x cs
   template<class T> inline void save(T* data, int rs, int cs, string filename)
   {
      std::ofstream fout;
      fout.open( filename.c_str(), std::ofstream::out );

      int r, c;
      for( r=0; r<rs; r++ )
      {
         for( c=0; c<cs; c++ )
         {
            fout<<data[r*cs+c]<<"\t";
         }
         fout<<std::endl;
      }

      fout.close();
      return;
   }

   /// saves a vector in matrix format with rs x cs
   template<class T> inline void save(vector<T>* data, int rs, int cs, string filename)
   {
      std::ofstream fout;
      fout.open( filename.c_str(), std::ofstream::out );

      int r, c;
      for( r=0; r<rs; r++ )
      {
         for( c=0; c<cs; c++ )
         {
            fout<<data->at(r*cs+c)<<"\t";
         }
         fout<<std::endl;
      }
      fout.close();
      return;
   }

   /// saves a matrix with rs x cs
   template<class T> inline void save(T** data, int rs, int cs, string filename)
   {
      std::ofstream fout;
      fout.open( filename.c_str(), std::ofstream::out );

      int r, c;
      for( r=0; r<rs; r++ )
      {
         for( c=0; c<cs; c++ )
         {
            fout<<data[r][c]<<"\t";
         }
         fout<<std::endl;
      }
      fout<<std::endl;

      fout.close();
      return;
   }

   ///  converts an integer number to a hex string.
   void convert_hex(int number, int* hex);

   template<class T> inline
   void save_BMP( string str, int w , int h, T *mask, string mode, bool inverted=false)
   {
      enum { RGB32, BGR32, GBR32, GRAY, MASK };

      int image_data_size = w * h;

      int color_mode = 0;
      if( !mode.compare("c") || !mode.compare("rgb32") )
      {
         color_mode = RGB32;
         image_data_size *= 4;
      }
      else if( !mode.compare("bgr32") )
      {
         color_mode = BGR32;
         image_data_size *= 4;
      }
      else if( !mode.compare("g") || !mode.compare("gray") )
      {
         color_mode = GRAY;
         image_data_size *= 4;
      }
      else if( !mode.compare("m")  )
      {
         color_mode = MASK;
      }

      int hexWidth[4];
      int hexHeight[4];
      int hexFileSize[4];
      int hexIdent[4];


      convert_hex(w, hexWidth);
      convert_hex(h, hexHeight);
      convert_hex(image_data_size+54,hexFileSize);
      convert_hex(image_data_size,hexIdent);

      FILE * maskFile  = fopen( str.c_str() , "w+b");

      char headerArray[54];
      headerArray[0] =(char)0x42 ;
      headerArray[1] =(char)0x4D ;
      headerArray[2] =(char)hexFileSize[0] ;
      headerArray[3] =(char)hexFileSize[1] ;
      headerArray[4] =(char)hexFileSize[2] ;
      headerArray[5] =(char)hexFileSize[3] ;
      headerArray[6] = (char)0x0;
      headerArray[7] = (char)0x0;
      headerArray[8] = (char)0x0;
      headerArray[9] = (char)0x0;
      headerArray[10] = (char)0x36;
      headerArray[11] = (char)0x0;
      headerArray[12] = (char)0x0;
      headerArray[13] = (char)0x0;
      headerArray[14] = (char)0x28;
      headerArray[15] = (char)0x0;
      headerArray[16] = (char)0x0;
      headerArray[17] = (char)0x0;
      headerArray[18] = (char)hexWidth[0];
      headerArray[19] = (char)hexWidth[1];
      headerArray[20] = (char)hexWidth[2];
      headerArray[21] = (char)hexWidth[3];
      headerArray[22] = (char)hexHeight[0];
      headerArray[23] = (char)hexHeight[1];
      headerArray[24] = (char)hexHeight[2];
      headerArray[25] = (char)hexHeight[3];
      headerArray[26] = (char)0x01;
      headerArray[27] = (char)0x0;
      headerArray[28] = (char)0x20;
      headerArray[29] = (char)0x0;
      headerArray[30] = (char)0x0;
      headerArray[31] = (char)0x0;
      headerArray[32] = (char)0x0;
      headerArray[33] = (char)0x0;
      headerArray[34] = (char)hexIdent[0];
      headerArray[35] = (char)hexIdent[1];
      headerArray[36] = (char)hexIdent[2];
      headerArray[37] = (char)hexIdent[3];
      headerArray[38] = (char)0xC4;
      headerArray[39] = (char)0x0E;
      headerArray[40] = (char)0x0;
      headerArray[41] = (char)0x0;
      headerArray[42] = (char)0xC4;
      headerArray[43] = (char)0x0E;
      headerArray[44] = (char)0x0;
      headerArray[45] = (char)0x0;
      headerArray[46] = (char)0x0;
      headerArray[47] = (char)0x0;
      headerArray[48] = (char)0x0;
      headerArray[49] = (char)0x0;
      headerArray[50] = (char)0x0;
      headerArray[51] = (char)0x0;
      headerArray[52] = (char)0x0;
      headerArray[53] = (char)0x0;

      fwrite(headerArray, sizeof(char), 54, maskFile);
      fclose(maskFile);
      maskFile  = fopen( str.c_str() , "a+b");

      uchar* data = kutility::allocate<uchar>(image_data_size);
      
      int index=0, indexM = 0;
      //create bitmap data//
      for(int m=0; m<h; m++)
         for(int n=0; n<w; n++)
         {
            index  = m*w+n;

            if  (inverted) indexM = (h-m-1)*w+n;
            else           indexM = index;

            if( color_mode == RGB32 )
            {
               data[4*indexM  ] = (uchar)(mask[3*index  ]);
               data[4*indexM+1] = (uchar)(mask[3*index+1]);
               data[4*indexM+2] = (uchar)(mask[3*index+2]);
               data[4*indexM+3] = 0;
            }
            else if( color_mode == BGR32 )
            {
               data[4*indexM  ] = (uchar)(mask[3*index+2]);
               data[4*indexM+1] = (uchar)(mask[3*index+1]);
               data[4*indexM+2] = (uchar)(mask[3*index  ]);
               data[4*indexM+3] = 0;
            }
            else if( color_mode == GBR32 )
            {
               data[4*indexM  ] = (uchar)(mask[3*index+1]);
               data[4*indexM+1] = (uchar)(mask[3*index+2]);
               data[4*indexM+2] = (uchar)(mask[3*index  ]);
               data[4*indexM+3] = 0;
            }
            else if( color_mode == GRAY )
            {
               data[4*indexM  ] = (uchar)(mask[index]);
               data[4*indexM+1] = (uchar)(mask[index]);
               data[4*indexM+2] = (uchar)(mask[index]);
               data[4*indexM+3] = 0;
            }
            else if( color_mode ==  MASK )
            {
               data[4*indexM  ] = (uchar)(255*mask[index]);
               data[4*indexM+1] = (uchar)(255*mask[index]);
               data[4*indexM+2] = (uchar)(255*mask[index]);
               data[4*indexM+3] = 0;
            }
         }

      fwrite(data, sizeof(char), image_data_size, maskFile);
      fclose(maskFile);

      deallocate(data);
   }
}

#endif

