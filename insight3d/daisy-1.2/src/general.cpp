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

#include "kutility/general.h"

namespace kutility
{
   char* itoa(int value, char*  str, int radix)
   {
      int  rem = 0;
      int  pos = 0;
      char ch  = '!' ;
      do
      {
         rem    = value % radix ;
         value /= radix;
         if ( 16 == radix )
         {
            if( rem >= 10 && rem <= 15 )
            {
               switch( rem )
               {
               case 10:
                  ch = 'a' ;
                  break;
               case 11:
                  ch ='b' ;
                  break;
               case 12:
                  ch = 'c' ;
                  break;
               case 13:
                  ch ='d' ;
                  break;
               case 14:
                  ch = 'e' ;
                  break;
               case 15:
                  ch ='f' ;
                  break;
               }
            }
         }
         if( '!' == ch )
         {
            str[pos++] = (char) ( rem + 0x30 );
         }
         else
         {
            str[pos++] = ch ;
         }
      }while( value != 0 );
      str[pos] = '\0' ;
      return strrev(str);
   }

   //strrev the standard way
   // the following directives to make the code portable
   // between windows and Linux.
   char* strrev(char* szT)
   {
      if ( !szT )                 // handle null passed strings.
         return NULL;
      int i = strlen(szT);
      int t = !(i%2)? 1 : 0;      // check the length of the string .
      for(int j = i-1 , k = 0 ; j > (i/2 -t) ; j-- )
      {
         char ch  = szT[j];
         szT[j]   = szT[k];
         szT[k++] = ch;
      }
      return szT;
   }

}

