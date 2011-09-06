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
// web   : http://cvlab.epfl.ch/~tola                                         //
// email : engin.tola@epfl.ch                                                 //
//                                                                            //
// If you use this code for research purposes, please refer to the            //
// webpage above                                                              //
////////////////////////////////////////////////////////////////////////////////

#include "daisy/daisy.h"

using namespace kutility;

enum { NONE, DISPLAY, SAVE_SINGLE, SAVE_ALL };

void display_help()
{
   cout<<"usage: \n";
   cout<<"       -h/--help           : this text\n";
   cout<<"       -i/--image          : image path\n";
   cout<<"       -p/--param          : descriptor parameters\n";
   cout<<"                           : rad radq thq histq\n";
//    cout<<"       -ri/--rotation-inv  : compute rotation invariant descriptors\n";
//    cout<<"                           : orientation_resolution\n";
   cout<<"       -d/--display y x    : displays the y,x point's descriptor \n";
   cout<<"       -s/--save y x       : saves the y,x point's descriptor \n";
   cout<<"       -sa/--save-all      : save all descriptors\n";
   cout<<"       -v/--verbose        : verbose\n";
   cout<<"       -vv                 : verbose more\n";
   cout<<"       -vvv/               : verbose even more\n";
   exit(0);
}

int main( int argc, char **argv  )
{
   int counter=1;

   if(  argc == 1 || !strcmp("-h", argv[counter] ) || !strcmp("--help", argv[counter] ) )
   {
      display_help();
   }

   int w,h;
   uchar* im = NULL;
   int verbose_level=0;

   int opy = -1;
   int opx = -1;

   int rad   = 15;
   int radq  =  3;
   int thq   =  8;
   int histq =  8;

   int orientation_resolution = 18;
   bool rotation_inv = false;

   char buffer[10];
   char* filename=NULL;

   int operation_mode=NONE;

   // Get command line options
   while( counter < argc )
   {
      if( !strcmp("-i", argv[counter] ) || !strcmp("--image", argv[counter]) )
      {
         filename = argv[++counter];
         im = load_byte_image(filename,w,h);
         counter++;
         continue;
      }
      if( !strcmp("-p", argv[counter] ) || !strcmp("--param", argv[counter]) )
      {
         if( argc <= counter+4 ) error( "you must enter daisy params" );
         set_positive_integer( rad, argv[++counter], "rad");
         set_positive_integer( radq, argv[++counter], "radq");
         set_positive_integer( thq, argv[++counter], "thq");
         set_positive_integer( histq, argv[++counter], "histq");
         counter++;
         continue;
      }
      if( !strcmp("-ri", argv[counter] ) || !strcmp("--rotation-inv", argv[counter]) )
      {
         if( argc <= counter+1 ) error( "you must enter orientation resolution" );
         set_positive_integer( orientation_resolution, argv[++counter], "orientation_resolution");
         rotation_inv = true;
         counter++;
         continue;
      }

      if( !strcmp("-d", argv[counter] ) || !strcmp("--display", argv[counter]) )
      {
         if( argc <= counter+2 ) error( "you must enter coordinates" );
         set_positive_integer( opy, argv[++counter], "y");
         set_positive_integer( opx, argv[++counter], "x");
         counter++;
         operation_mode = DISPLAY;
         continue;
      }

      if( !strcmp("-s", argv[counter] ) || !strcmp("--save", argv[counter]) )
      {
         if( argc <= counter+2 ) error( "you must enter coordinates" );
         set_positive_integer( opy, argv[++counter], "y");
         set_positive_integer( opx, argv[++counter], "x");
         counter++;
         operation_mode = SAVE_SINGLE;
         continue;
      }

      if( !strcmp("-sa", argv[counter] ) || !strcmp("--save-all", argv[counter]) )
      {
         operation_mode = SAVE_ALL;
         counter++;
         continue;
      }

      if( !strcmp("-v", argv[counter] ) || !strcmp("--verbose", argv[counter]) )
      {
         counter++;
         verbose_level = 1;
         continue;
      }
      if( !strcmp("-vv", argv[counter] ) )
      {
         counter++;
         verbose_level = 2;
         continue;
      }
      if( !strcmp("-vvv", argv[counter] ) )
      {
         counter++;
         verbose_level = 3;
         continue;
      }
      warning("unknown option");
      cout<<"option : "<<argv[counter]<<endl;
      counter ++;
      exit(1);
   }

   if( filename == NULL )
   {
      error("you haven't specified the filename mate.");
   }

   daisy* desc = new daisy();

   desc->set_image(im,h,w);
   deallocate(im);
   desc->verbose( verbose_level );
   desc->set_parameters(rad, radq, thq, histq);

// !! this part is optional. You don't need to set the workspace memory
   int ws = desc->grid_point_number()* w * h;
   float* workspace = new float[ ws ];
   desc->set_workspace_memory( workspace, ws);
// !! this part is optional. You don't need to set the workspace memory

   desc->initialize_single_descriptor_mode();

// !! this is work in progress. do not enable!
//    if( rotation_inv ) desc->rotation_invariant(orientation_resolution, rotation_inv);
// !! this is work in progress. do not enable!


// !! this part is optional. You don't need to set the descriptor memory
   int ds = w*h*desc->descriptor_size();
   float* descriptor_mem = new float[ds];
   desc->set_descriptor_memory( descriptor_mem, ds );
// !! this part is optional. You don't need to set the descriptor memory

   string fname;
   float* thor = new float[desc->descriptor_size()];
   switch( operation_mode )
   {
   case DISPLAY:
      desc->get_descriptor(opy,opx,0,thor);
      display(thor, desc->grid_point_number(), histq, 0, 0 );
      break;
   case SAVE_SINGLE:
      fname = filename;
      fname = fname + "_y="  + itoa(opy,buffer,10) + "_x=" + itoa(opx,buffer,10) + ".desc";
      desc->get_descriptor(opy,opx,0,thor);
      save( thor, desc->grid_point_number(), histq, fname );
      break;
   case SAVE_ALL:
      desc->compute_descriptors();
      message("saving features...");
      string outname = filename;
      outname +=".features";
      desc->save_descriptors(outname);
      break;
   }
   delete[] thor;

   delete desc;
   delete []workspace;
   delete []descriptor_mem;

   return 0;
}
