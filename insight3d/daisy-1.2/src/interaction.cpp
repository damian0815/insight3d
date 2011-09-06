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

#include "kutility/interaction.h"

using std::string;

namespace kutility
{
   void error( string str1, int code )
   {
      std::cout<<"ERROR: "
               <<str1<<std::endl;
      exit(code);
   }

   void error( string str1, string str2, int code )
   {
      std::cout<<"ERROR: "
               <<str1<<" "<<str2<<std::endl;
      exit(code);
   }

   void error( string str1, string str2, string str3, int code )
   {
      std::cout<<"ERROR: "
               <<str1<<" "
               <<str2<<" "
               <<str3<<std::endl;
      exit(code);
   }

   void warning( string str1, string str2, string str3 )
   {
      std::cout<<"WARNING: "
               <<str1<<" "
               <<str2<<" "
               <<str3<<std::endl;
      return;
   }

   void message( string str1, string str2, string str3 )
   {
      std::cout<<str1<<" "
               <<str2<<" "
               <<str3<<std::endl;
      return;
   }

   void major_message( string str1, string str2, string str3, string sep )
   {
      string str = str1 + " " + str2 + " " + str3;

      int length = str.length();

      if( length > 140 ) length = 140;

      for( int i=0; i<length; i++)
         std::cout<<sep;
      std::cout<<std::endl;

      std::cout<<str<<std::endl;

      for( int i=0; i<length; i++)
         std::cout<<sep;
      std::cout<<std::endl;

      return;
   }
}
