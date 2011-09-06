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

#include <iostream>
#include "daisy/daisy.h"

using namespace std;
using namespace kutility;

daisy::daisy()
{
   m_verbosity = 2;
   m_image = 0;
   m_w = 0;
   m_h = 0;

   m_rad = 0;
   m_rad_q_no = 0;
   m_th_q_no  = 0;
   m_hist_th_q_no = 0;
   m_grid_point_number = 0;
   m_descriptor_size = 0;

   m_gradient_layers  = NULL;
   m_smoothed_gradient_layers = NULL;
   m_dense_descriptors   = NULL;
   m_grid_points = NULL;
   m_oriented_grid_points = NULL;

   m_scale_invariant = false;
   m_rotation_invariant = false;

   m_scale_map = NULL;
   m_orientation_map = NULL;
   m_orientation_resolution = 36;
   m_scale_map = NULL;

   m_level_sigmas = NULL;

   m_descriptor_memory = false;
   m_workspace_memory = false;
   m_descriptor_normalization_threshold = 0.154;
}

daisy::~daisy()
{
   deallocate( m_image );
   deallocate( m_gradient_layers );

   if( !m_workspace_memory )
      deallocate( m_smoothed_gradient_layers );

   deallocate( m_grid_points, m_grid_point_number );
   deallocate( m_oriented_grid_points, g_grid_orientation_resolution );
   deallocate( m_orientation_map );
   deallocate( m_scale_map );
   deallocate( m_level_sigmas );

   if( !m_descriptor_memory )
      deallocate( m_dense_descriptors );
}

void daisy::set_parameters( float rad, int rad_q_no, int th_q_no, int hist_th_q_no )
{
   m_rad = rad;                   // radius of the descriptor at the initial scale
   m_rad_q_no = rad_q_no;         // how many pieces shall I divide the radial range ?
   m_th_q_no = th_q_no;           // how many pieces shall I divide the angular range  ?
   m_hist_th_q_no = hist_th_q_no; // how many pieces shall I divide the grad_hist
   m_grid_point_number = m_rad_q_no * m_th_q_no + 1; // +1 is for center pixel
   m_descriptor_size = m_grid_point_number * m_hist_th_q_no;
}

float* daisy::get_dense_descriptors()
{
   return m_dense_descriptors;
}

float** daisy::get_grid_points()
{
   return m_grid_points;
}

void daisy::reset()
{
   deallocate( m_image );
   deallocate( m_gradient_layers );
   deallocate( m_grid_points, m_grid_point_number );
   deallocate( m_orientation_map );
   deallocate( m_scale_map );

   if( !m_descriptor_memory )
      deallocate( m_dense_descriptors );
}

void daisy::release_auxilary()
{
   deallocate(m_image);
   deallocate(m_gradient_layers );
   deallocate( m_orientation_map );
   deallocate( m_scale_map );

   if( !m_workspace_memory )
      deallocate(m_smoothed_gradient_layers);

   deallocate( m_grid_points, m_grid_point_number );
   deallocate( m_oriented_grid_points, g_grid_orientation_resolution );
   deallocate( m_level_sigmas );
}

void daisy::compute_grid_points()
{
   float r_step = m_rad / m_rad_q_no;
   float t_step = 2*pi()/ m_th_q_no;

   if( m_grid_points )
      deallocate( m_grid_points, m_grid_point_number );

   m_grid_points = allocate<float>(m_grid_point_number, 2);
   kutility::initialize( m_grid_points, m_grid_point_number, 2, 0 );

   for( float r=0; r<m_rad_q_no; r++ )
   {
      int region = (int)(r*m_th_q_no)+1;
      for( float t=0; t<m_th_q_no; t++ )
      {
         float y, x;
         polar2cartesian( (r+1)*r_step, t*t_step, y, x );
         m_grid_points[(int)(region+t)][0] = y;
         m_grid_points[(int)(region+t)][1] = x;
      }
   }

   if( m_verbosity > 2 )
   {
      cout<<"grid points:";
      display( m_grid_points, m_grid_point_number, 2 );
   }
}

/// Computes the descriptor by sampling convoluted orientation maps.
void daisy::compute_descriptors()
{
   if( m_scale_invariant    ) compute_scales();
   if( m_rotation_invariant ) compute_orientations();

   if( m_verbosity > 0 ) cout<<"/ computing descriptors"<<endl;

   if( !m_descriptor_memory ) m_dense_descriptors = allocate <float>(m_h*m_w*m_descriptor_size);

   int counter=0;
   time_t st,now;
   time(&st);

#pragma omp parallel for
   for( int y=0; y<m_h; y++ )
   {
      time(&now);
      if( m_verbosity > 0 ) progress( counter, m_h, 50, (time_t)difftime(now,st));
      counter++;
      for( int x=0; x<m_w; x++ )
      {
         int index=y*m_w+x;

         int orientation=0;
         if( m_orientation_map )
            orientation = m_orientation_map[index];

         if( !( orientation >= 0 && orientation < g_grid_orientation_resolution ) )
            orientation = 0;

         get_descriptor( y, x, orientation, &(m_dense_descriptors[index*m_descriptor_size]) );
      }
   }
   if( m_verbosity > 0 ) cout<<"normalizing descriptors\n";
   normalize_descriptors();
   if( m_verbosity > 0 ) cout<<"normalization finished\n";

   time(&now);
   if( m_verbosity >0 )
      cout<<"descriptors computed in "<<difftime(now,st)<<" seconds\n";
}

void daisy::smooth_layers( float*layers, int layer_number, float sigma )
{
   // take the size of the gaussian kernel 5*sigma.
   int fsz = (int)(5*sigma);
   if( fsz%2 == 0 ) fsz++; // kernel size must be odd
   if( fsz < 3 ) fsz = 3;  // kernel size cannot be smaller than 3
   float* filter = gaussian_1d(fsz, sigma, 0);

#pragma omp parallel for
   for( int i=0; i<layer_number; i++ )
   {
      float* layer = layers + i*m_h*m_w;
      convolve_sym( layer, m_h, m_w, filter, fsz, layer );
   }
   deallocate(filter);
}

void daisy::save_descriptor( string filename, int y, int x, bool single_row)
{
   float* feat = &(m_dense_descriptors[(y*m_w+x)*m_descriptor_size]);

   if( single_row )
      save( feat, 1, m_descriptor_size, filename );
   else
      save( feat, m_grid_point_number , m_hist_th_q_no, filename );
}

void daisy::save_descriptors( string filename )
{
   ofstream fout;
   fout.open(filename.c_str());

   for( int y=0; y<m_h; y++ )
   {
      for( int x=0; x<m_w; x++ )
      {
         int index = (y*m_w+x)*m_descriptor_size;
         fout<<y<<" "<<x<<" ";

         for( int d=0; d<m_descriptor_size; d++ )
            fout<<m_dense_descriptors[index+d]<<" ";
         fout<<endl;
      }
   }
   fout.close();
//    save( m_dense_descriptors, m_w*m_h, m_descriptor_size, filename );
}

void daisy::normalize_partial( float* desc )
{
   for( int h=0; h<m_grid_point_number; h++ )
   {
      float norm =  l2norm( &(desc[h*m_hist_th_q_no]), m_hist_th_q_no );
      if( norm != 0.0 )
         divide( desc+h*m_hist_th_q_no, m_hist_th_q_no, norm);
   }
}
void daisy::normalize_full( float* desc )
{
   float norm =  l2norm( desc, m_descriptor_size );
   if( norm != 0.0 )
      divide(desc, m_descriptor_size, norm);
}
void daisy::normalize_sift_way( float* desc )
{
   float norm = l2norm( desc, m_descriptor_size );

   if( norm > 1e-5 )
      divide( desc, m_descriptor_size, norm);

   bool changed = false;
   for( int h=0; h<m_descriptor_size; h++ )
   {
      if( desc[ h ] > m_descriptor_normalization_threshold )
      {
         desc[ h ] = m_descriptor_normalization_threshold;
         changed = true;
      }
   }
   if( changed )
   {
      norm = l2norm( desc, m_descriptor_size );
      if( norm != 0.0 ) divide( desc, m_descriptor_size, norm);
   }
}
void daisy::normalize_descriptors()
{
   int number_of_descriptors = m_h * m_w;

#pragma omp parallel for
   for( int d=0; d<number_of_descriptors; d++ )
   {
      normalize_partial ( m_dense_descriptors+d*m_descriptor_size );
//       normalize_full    ( m_dense_descriptors+d*m_descriptor_size );
//       normalize_sift_way( &(m_dense_descriptors[d*m_descriptor_size]) );
   }
}

void daisy::initialize_single_descriptor_mode()
{
   initialize();
   compute_smoothed_gradient_layers();
   compute_oriented_grid_points();
}

void daisy::initialize()
{
   if( m_verbosity > 0 )
      cout<<"initializing...\n";

   if( m_gradient_layers ) deallocate(m_gradient_layers );
   m_gradient_layers = layered_gradient<float>( m_image, m_h, m_w, m_hist_th_q_no );

   //assuming a 0.5 image smoothness, we pull this to 1.6 as in sift
   smooth_layers( m_gradient_layers, m_hist_th_q_no, sqrt( g_sigma_init * g_sigma_init - 0.25 ) );

   if( m_level_sigmas == NULL ) // user didn't set the sigma's; set them from the descriptor parameters
   {
      g_cube_number = m_rad_q_no;
      m_level_sigmas = allocate<float>(g_cube_number);

      g_grid_radius[0] = 0;

      float r_step = m_rad / m_rad_q_no;

      for( int r=0; r< m_rad_q_no; r++ )
      {
         g_grid_radius[r] = r;
         m_level_sigmas[r] = (r+1)*r_step/2;
//          cout<<"sigma"<<r<<": "<<m_level_sigmas[r]<<endl;
      }
  }

   compute_grid_points();
   if( m_verbosity > 0 )
      cout<<"ok!\n";
}

void daisy::set_level_gaussians( float* sigma_array, int sz )
{
   g_cube_number = sz;

   if( m_level_sigmas ) deallocate( m_level_sigmas );
   m_level_sigmas = allocate<float>(g_cube_number);

   for( int r=0; r<g_cube_number; r++ )
   {
      m_level_sigmas[r] = sigma_array[r];
      if( m_verbosity > 1 )
         cout<<"sigma"<<r<<": "<<m_level_sigmas[r]<<endl;
   }

   for( int r=0; r<m_rad_q_no; r++ )
   {
      float seed_sigma = (r+1)*m_rad/m_rad_q_no/2.0;
      g_grid_radius[r] = quantize_radius( seed_sigma );
      if( m_verbosity > 1 )
      {
         cout<<"seed : "<<seed_sigma<<endl;
         cout<<"g_grid_radius["<<r<<"] = "<<g_grid_radius[r]<<" sigma: "<<m_level_sigmas[ g_grid_radius[r] ] <<endl;
      }
   }
}

void daisy::compute_smoothed_gradient_layers()
{
   if( m_verbosity > 0 ) std::cout<<"/ smoothed orientation layers..."<<std::endl;

   int data_size = m_h * m_w;
   int cube_size = m_hist_th_q_no * data_size;

   if( !m_workspace_memory )
      m_smoothed_gradient_layers = new float[g_cube_number * cube_size];

   float* prev_cube = m_gradient_layers; // size m_hist_th_q_no * m_w * m_h
   float* cube = NULL;

   for( int r=0; r<g_cube_number; r++ )
   {
      cube = m_smoothed_gradient_layers + r*cube_size;

      // incremental smoothing
      float sigma;
      if( r == 0 ) sigma = m_level_sigmas[0];
      else         sigma = sqrt( m_level_sigmas[r]*m_level_sigmas[r] - m_level_sigmas[r-1]*m_level_sigmas[r-1] );

      // take the size of the gaussian kernel 5*sigma.
      int fsz = (int)(5*sigma);
      if( fsz%2 == 0 ) fsz++; // kernel size must be odd
      if( fsz < 3 ) fsz = 3;  // kernel size cannot be smaller than 3
      float* filter = gaussian_1d(fsz, sigma, 0);

      for( int th=0; th<m_hist_th_q_no; th++ )
      {
         convolve_sym( &(prev_cube[th*data_size]), m_h, m_w, filter, fsz, &(cube[th*data_size]) );
      }
      deallocate(filter);
      prev_cube = cube;
   }
   if( m_verbosity > 0 ) std::cout<<"/ orientation layers smoothed"<<std::endl;
}
void daisy::compute_oriented_grid_points()
{
   m_oriented_grid_points = allocate<float>(g_grid_orientation_resolution, m_grid_point_number*2 );

   for( int i=0; i<g_grid_orientation_resolution; i++ )
   {
      float angle = -i*2.0*pi()/g_grid_orientation_resolution;

      float kos = cos( angle );
      float zin = sin( angle );

      float* point_list = m_oriented_grid_points[ i ];

      for( int k=0; k<m_grid_point_number; k++ )
      {
         float y = m_grid_points[k][0];
         float x = m_grid_points[k][1];

         point_list[2*k+1] =  x*kos + y*zin; // x
         point_list[2*k  ] = -x*zin + y*kos; // y
      }
   }

   if( m_verbosity > 1 )
   {
      // if want to display the descriptor structure
      int str_w = 2*m_rad+1;
      int str_size = square( str_w );

      int* structure = allocate<int>( str_size );

      for( int ori=0; ori<g_grid_orientation_resolution; ori++ )
      {
         memset( structure, 0, sizeof(int)*str_size );
         for( int reg=0; reg<m_grid_point_number; reg++ )
         {
            float y = (int)(m_oriented_grid_points[ori] [2*reg  ] + m_rad);
            float x = (int)(m_oriented_grid_points[ori] [2*reg+1] + m_rad);

            structure[(int)(y*str_w+x)] = reg+1;
         }
//          display( structure, str_w, str_w, 1, 1, 2, 2, "  " );
//          wait_key();
      }
      deallocate(structure);
   }
}

/// sets a custom grid
void daisy::set_grid_points()
{
   // I should implement this
   cout<<"set_grid_points::this is not implemented yet \n";
   exit(1);
}

void daisy::smooth_histogram(float *hist, int bins)
{
   int i;
   float prev, temp;

   prev = hist[bins - 1];
   for (i = 0; i < bins; i++)
   {
      temp = hist[i];
      hist[i] = (prev + hist[i] + hist[(i + 1 == bins) ? 0 : i + 1]) / 3.0;
      prev = temp;
   }
}

float daisy::interpolate_peak(float left, float center, float right)
{
   if( center < 0.0 )
   {
      left = -left;
      center = -center;
      right = -right;
   }
   assert(center >= left  &&  center >= right);

   float den = (left - 2.0 * center + right);

   if( den == 0 ) return 0;
   else           return 0.5*(left -right)/den;
}

int daisy::filter_size( float sigma )
{
   int fsz = (int)(5*sigma);

   // kernel size must be odd
   if( fsz%2 == 0 ) fsz++;

   // kernel size cannot be smaller than 3
   if( fsz < 3 ) fsz = 3;

   return fsz;
}

void daisy::compute_scales()
{
   cout<<"###############################################################################\n";
   cout<<"# scale detection is work-in-progress! do not use it if you're not Engin Tola #\n";
   cout<<"###############################################################################\n\n";

   int imsz = m_w * m_h;

   if( m_verbosity > 0 )
   {
      cout<<"detecting scales...\n";
      cout<<"k: "<<g_sigma_step<<" scale_st: "<<g_scale_st<<" scale_en:  "<<g_scale_en<<endl;
   }

   float sigma = pow( g_sigma_step, g_scale_st)*g_sigma_0;

   float* sim = blur_gaussian_2d<float,float>( m_image, m_h, m_w, sigma, filter_size(sigma), false);

   float* next_sim = NULL;

   float* max_dog = allocate<float>(imsz);

   m_scale_map = allocate<float>(imsz);

   memset( max_dog, 0, imsz*sizeof(float) );
   memset( m_scale_map, 0, imsz*sizeof(float) );

   int i;
   float sigma_prev;
   float sigma_new;
   float sigma_inc;

   sigma_prev = g_sigma_0;
   for( i=0; i<g_scale_en; i++ )
   {
      sigma_new  = pow( g_sigma_step, g_scale_st+i  ) * g_sigma_0;
      sigma_inc  = sqrt( sigma_new*sigma_new - sigma_prev*sigma_prev );
      sigma_prev = sigma_new;

      if( m_verbosity > 0 )
         cout <<"[i = "<<i<<"/"<<g_scale_en<<"] smoothing: sigma_inc = "
              <<sigma_inc<<", sigma_new = "<<sigma_new<<", fsz : "<<filter_size(sigma_inc)<<endl;

      next_sim = blur_gaussian_2d<float,float>( sim, m_h, m_w, sigma_inc, filter_size( sigma_inc ) , false);

#pragma omp parallel for
      for( int p=0; p<imsz; p++ )
      {
         float dog = fabs( next_sim[p] - sim[p] );
         if( dog > max_dog[p] )
         {
            max_dog[p] = dog;
            m_scale_map[p] = i;
         }
      }
      deallocate( sim );

      sim = next_sim;
   }

   blur_gaussian_2d<float,float>( m_scale_map, m_h, m_w, 10.0, filter_size(10), true);

#pragma omp parallel for
   for( int q=0; q<imsz; q++ )
   {
      m_scale_map[q] = round( m_scale_map[q] );
   }

//    save( m_scale_map, m_h, m_w, "scales.dat");

   deallocate( sim );
   deallocate( max_dog );
}
void daisy::compute_orientations()
{
   cout<<"#####################################################################################\n";
   cout<<"# orientation detection is work-in-progress! do not use it if you're not Engin Tola #\n";
   cout<<"#####################################################################################\n\n";

   time_t sto, eno;
   time(&sto);

   if( m_verbosity > 0 )
      cout<<"/ starting orientation computation\n";

   assert( m_image != NULL );

   int data_size = m_w*m_h;
   float* rotation_layers = layered_gradient( m_image, m_h, m_w, m_orientation_resolution );

   m_orientation_map = zeros<int>( data_size );

   int ori, max_ind;
   int ind;
   float max_val;

   int next, prev;
   float peak, angle;

   int x, y, kk;

   float* hist=NULL;

   float sigma_inc;
   float sigma_prev = 0;
   float sigma_new;

   for( int scale=0; scale<g_scale_en; scale++ )
   {
      sigma_new  = pow( g_sigma_step, scale  ) * m_rad/3.0;
      sigma_inc  = sqrt( sigma_new*sigma_new - sigma_prev*sigma_prev );
      sigma_prev = sigma_new;

      smooth_layers( rotation_layers, m_orientation_resolution, sigma_inc);

      for( y=0; y<m_h; y ++ )
      {
         hist = allocate<float>(m_orientation_resolution);

         for( x=0; x<m_w; x++ )
         {
            ind = y*m_w+x;

            if( m_scale_invariant && m_scale_map[ ind ] != scale ) continue;

            for( ori=0; ori<m_orientation_resolution; ori++ )
            {
               hist[ ori ] = rotation_layers[ori*data_size+ind];
            }

            for( kk=0; kk<6; kk++ )
               smooth_histogram( hist, m_orientation_resolution );

            max_val = -1;
            max_ind =  0;
            for( ori=0; ori<m_orientation_resolution; ori++ )
            {
               if( hist[ori] > max_val )
               {
                  max_val = hist[ori];
                  max_ind = ori;
               }
            }

            prev = max_ind-1;
            if( prev < 0 )
               prev += m_orientation_resolution;

            next = max_ind+1;
            if( next >= m_orientation_resolution )
               next -= m_orientation_resolution;

            peak = interpolate_peak(hist[prev], hist[max_ind], hist[next]);
            angle = (max_ind + peak)*360.0/m_orientation_resolution;

            int iangle = int(angle);

            if( iangle <    0 ) iangle += 360;
            if( iangle >= 360 ) iangle -= 360;


            if( !(iangle >= 0.0 && iangle < 360.0) )
            {
               angle = 0;
            }

            m_orientation_map[ ind ] = iangle;
         }
         deallocate(hist);
      }
   }
//    save( m_orientation_map, m_h, m_w, "orientations");

   deallocate( rotation_layers );
   time(&eno);

   if( m_verbosity > 0 )
      cout<<"\\ finished orientation computation in "<<difftime(eno,sto)<<" seconds\n";

   compute_oriented_grid_points();
}

void daisy::set_descriptor_memory( float* descriptor, long int d_size )
{
   assert( m_descriptor_memory == false );
   assert( m_h*m_w != 0 );
   assert( d_size >= m_h * m_w * m_descriptor_size );

   m_dense_descriptors = descriptor;
   m_descriptor_memory = true;
}

void daisy::set_workspace_memory( float* workspace, long int w_size )
{
   assert( m_workspace_memory == false );
   assert( m_h*m_w != 0 );
   assert( w_size >= g_cube_number * m_hist_th_q_no * m_h * m_w );

   m_smoothed_gradient_layers = workspace;
   m_workspace_memory = true;
}

void daisy::get_descriptor(float y, float x, int orientation, float* descriptor )
{
   assert( y >= 0 && y < m_h );
   assert( x >= 0 && x < m_w );
   assert( orientation >= 0 && orientation < 360 );
   assert( m_smoothed_gradient_layers );
   assert( m_oriented_grid_points );
   assert( descriptor != NULL );

   int cube_size = m_hist_th_q_no * m_w*m_h;
   float shift = orientation / 360.0 * m_hist_th_q_no;

   // innermost region
   float fshift = shift - (int)shift;

//    trilinear_interpolated_histogram( descriptor, y, x,      shift  , m_smoothed_gradient_layers );
   if( fshift < 0.01 ) bilinear_interpolated_histogram ( descriptor, y, x, (int)shift  , m_smoothed_gradient_layers+g_grid_radius[0]*cube_size );
   if( fshift > 0.99 ) bilinear_interpolated_histogram ( descriptor, y, x, (int)shift+1, m_smoothed_gradient_layers+g_grid_radius[0]*cube_size );
   else                trilinear_interpolated_histogram( descriptor, y, x,      shift  , m_smoothed_gradient_layers+g_grid_radius[0]*cube_size );

   // petals of the flower
   for( int r=0; r<m_rad_q_no; r++ )
   {
      int rdt  = r*m_th_q_no+1;

      for( int region=rdt; region<rdt+m_th_q_no; region++ )
      {
         float yi = y + m_oriented_grid_points[orientation][2*region  ];
         float xi = x + m_oriented_grid_points[orientation][2*region+1];

         if( is_outside(xi, 0, m_w-1, yi, 0, m_h-1) ) continue;

         int place  = region*m_hist_th_q_no;

//          trilinear_interpolated_histogram( descriptor+place, yi, xi, shift, m_smoothed_gradient_layers+r*cube_size );
         if( fshift < 0.01 ) bilinear_interpolated_histogram ( descriptor+place, yi, xi, (int)shift  , m_smoothed_gradient_layers+g_grid_radius[r]*cube_size );
         if( fshift > 0.99 ) bilinear_interpolated_histogram ( descriptor+place, yi, xi, (int)shift+1, m_smoothed_gradient_layers+g_grid_radius[r]*cube_size );
         else                trilinear_interpolated_histogram( descriptor+place, yi, xi,      shift  , m_smoothed_gradient_layers+g_grid_radius[r]*cube_size );
      }
   }
   normalize_descriptor( descriptor );
}

bool daisy::get_descriptor(float* custom_grid, int orientation, float* descriptor )
{
   assert( orientation >= 0 && orientation < 360 );
   assert( m_smoothed_gradient_layers );
   assert( descriptor != NULL );

   int cube_size = m_hist_th_q_no * m_w*m_h;
   float shift = orientation / 360.0 * m_hist_th_q_no;

   // innermost region
   float fshift = shift - (int)shift;

   float hy = custom_grid[0];
   float hx = custom_grid[1];

   if( is_outside( hx, 0, m_w, hy, 0, m_h ) ) return false;

   if( fshift < 0.01 ) bilinear_interpolated_histogram ( descriptor, hy, hx, (int)shift  , m_smoothed_gradient_layers+g_grid_radius[0]*cube_size );
   if( fshift > 0.99 ) bilinear_interpolated_histogram ( descriptor, hy, hx, (int)shift+1, m_smoothed_gradient_layers+g_grid_radius[0]*cube_size );
   else                trilinear_interpolated_histogram( descriptor, hy, hx,      shift  , m_smoothed_gradient_layers+g_grid_radius[0]*cube_size );

   // petals of the flower
   for( int r=0; r<m_rad_q_no; r++ )
   {
      int rdt  = r*m_th_q_no+1;

      for( int region=rdt; region<rdt+m_th_q_no; region++ )
      {
         hy = custom_grid[2*region  ];
         hx = custom_grid[2*region+1];

         if( is_outside(hx, 0, m_w-1, hy, 0, m_h-1) ) continue;
         int place  = region*m_hist_th_q_no;

//          trilinear_interpolated_histogram( descriptor+place, yi, xi, shift, m_smoothed_gradient_layers+r*cube_size );
         if( fshift < 0.01 ) bilinear_interpolated_histogram ( descriptor+place, hy, hx, (int)shift  , m_smoothed_gradient_layers+g_grid_radius[r]*cube_size );
         if( fshift > 0.99 ) bilinear_interpolated_histogram ( descriptor+place, hy, hx, (int)shift+1, m_smoothed_gradient_layers+g_grid_radius[r]*cube_size );
         else                trilinear_interpolated_histogram( descriptor+place, hy, hx,      shift  , m_smoothed_gradient_layers+g_grid_radius[r]*cube_size );
      }
   }
   normalize_descriptor( descriptor );
   return true;
}

bool daisy::get_descriptor(float y, float x, int orientation, float* H, float* descriptor )
{
   assert( orientation >= 0 && orientation < 360 );
   assert( m_smoothed_gradient_layers );
   assert( descriptor != NULL );

   int cube_size = m_hist_th_q_no * m_w*m_h;
   float shift = orientation / 360.0 * m_hist_th_q_no;
   float fshift = shift - (int)shift;

   int* hradius = zeros<int>(m_rad_q_no);
   float radius;

   float hy, hx, ry, rx;

   point_transform_via_homography(H, x, y, hx, hy );
   if( is_outside( hx, 0, m_w, hy, 0, m_h ) ) return false;

   point_transform_via_homography(H, x+m_level_sigmas[g_grid_radius[0]], y, rx, ry);
   radius =  l2norm( ry, rx, hy, hx );
   hradius[0] = quantize_radius( radius );

   if( fshift < 0.01 ) bilinear_interpolated_histogram ( descriptor, hy, hx, (int)shift  , m_smoothed_gradient_layers+hradius[0]*cube_size );
   if( fshift > 0.99 ) bilinear_interpolated_histogram ( descriptor, hy, hx, (int)shift+1, m_smoothed_gradient_layers+hradius[0]*cube_size );
   else                trilinear_interpolated_histogram( descriptor, hy, hx,      shift  , m_smoothed_gradient_layers+hradius[0]*cube_size );

   for( int r=0; r<m_rad_q_no; r++)
   {
      int rdt = r*m_th_q_no + 1;

      for( int th=0; th<m_th_q_no; th++ )
      {
         int region = rdt + th;

         float gy = y + m_grid_points[region][0];
         float gx = x + m_grid_points[region][1];

         point_transform_via_homography(H, gx, gy, hx, hy);
         if( th == 0 )
         {
            point_transform_via_homography(H, gx+m_level_sigmas[g_grid_radius[r]], gy, rx, ry);
            radius = l2norm( ry, rx, hy, hx );
            hradius[r] = quantize_radius( radius );
         }

         if( is_outside(hx, 0, m_w-1, hy, 0, m_h-1) ) continue;
         int place  = region*m_hist_th_q_no;

//          trilinear_interpolated_histogram( descriptor+place, yi, xi, shift, m_smoothed_gradient_layers+r*cube_size );
         if( fshift < 0.01 ) bilinear_interpolated_histogram ( descriptor+place, hy, hx, (int)shift  , m_smoothed_gradient_layers+hradius[r]*cube_size );
         if( fshift > 0.99 ) bilinear_interpolated_histogram ( descriptor+place, hy, hx, (int)shift+1, m_smoothed_gradient_layers+hradius[r]*cube_size );
         else                trilinear_interpolated_histogram( descriptor+place, hy, hx,      shift  , m_smoothed_gradient_layers+hradius[r]*cube_size );
      }
   }

   deallocate(hradius);
   return true;
}

