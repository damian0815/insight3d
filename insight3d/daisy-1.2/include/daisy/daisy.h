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

#ifndef DAISY_H
#define DAISY_H

#include <cmath>
#include "assert.h"

#include "kutility/general.h"
#include "kutility/math.h"
#include "kutility/fileio.h"

const float g_sigma_0 = 1;
const float g_sigma_1 = sqrt(2);
const float g_sigma_2 = 8;
const float g_sigma_step = std::pow(2,1.0/2);
const int   g_scale_st = int( (log(g_sigma_1/g_sigma_0)) / log(g_sigma_step) );
static int  g_scale_en = 1;

const float g_sigma_init = 1.6;
static int g_grid_orientation_resolution = 360;

static int g_cube_number = 3;
static int g_grid_radius[10]; // m_rad_q_no < 10 --> must make this a member variable

class daisy
{
public:
   daisy();
   ~daisy();

   /// maximum radius of the descriptor region.
   float m_rad;

   /// the number of quantizations of the radius.
   int m_rad_q_no;

   /// the number of quantizations of the angle.
   int m_th_q_no;

   /// the number of quantizations of the gradient orientations.
   int m_hist_th_q_no;

   /// returns the descriptor vector for the point (y, x) !!! use this for
   /// precomputed operations meaning that you must call compute_descriptors()
   /// before calling this function
   inline void get_descriptor(int y, int x, float* &descriptor)
      {
         assert( m_dense_descriptors != NULL );
         descriptor = &(m_dense_descriptors[(y*m_w+x)*m_descriptor_size]);
      }

   /// computed the descriptor and returns the result in 'descriptor' ( allocate
   /// 'descriptor' memory first ie: float descriptor = new
   /// float[m_descriptor_size];
   void get_descriptor(float y, float x, int orientation, float* descriptor );

   /// computes the descriptor at homography-warped grid. (y,x) is not the
   /// coordinates of this image but the coordinates of the original grid where
   /// the homography will be applied. Meaning that the grid is somewhere else
   /// and we warp this grid with H and compute the descriptor on this warped
   /// grid; returns null/false if centers falls outside the image; allocate
   /// 'descriptor' memory first
   bool get_descriptor(float y, float x, int orientation, float* H, float* descriptor );

   /// computes the descriptor at a custom grid.  allocate 'descriptor' memory
   /// first
   bool get_descriptor(float* custom_grid, int orientation, float* descriptor );

   /// returns the size of the descriptor vector
   inline int descriptor_size() { return m_descriptor_size; }

   /// returns the region number.
   inline int grid_point_number() { return m_grid_point_number; }

   /// suppresses output messages.
   /// 0: no output
   /// 1: semi-verbose ( important messages only )
   /// 2: full verbosity
   inline void verbose( size_t verbosity=2 ) { m_verbosity = verbosity; }

   /// releases all the used memory; call this if you want to process
   /// multiple images within a loop.
   void reset();

   /// releases unused memory after descriptor computation is completed.
   void release_auxilary();

   /// computes the descriptors for every pixel in the image.
   void compute_descriptors();

   /// sets image data and size, image is converted to float and normalized.
   template<class T> inline
   void set_image(T* im, int h, int w)
      {
         m_image = kutility::type_cast<float, T>( im, h*w );
         kutility::normalize(m_image, h*w, (float)255.0, true);
         m_w = w;
         m_h = h;
      }

   /// sets the descriptor parameters
   void set_parameters( float rad, int rad_q_no, int th_q_no, int hist_th_q_no );

   /// initializes for get_descriptor(float, float, int) mode: pre-computes
   /// convolutions of gradient layers in m_smoothed_gradient_layers
   void initialize_single_descriptor_mode();

   /// returns all the descriptors.
   float* get_dense_descriptors();

   /// returns the used grid point coordinates.
   float** get_grid_points();

   /// saves the descriptor (y,x) to file "filename"
   void save_descriptor( std::string filename, int y, int x, bool single_row=false);

   /// saves all the descriptors to file "filename"
   void save_descriptors( std::string filename);

   /// tells the destructor not to deallocate the memory for the
   /// m_dense_descriptors after the daisy object is destroyed.
   void detach_dense_descriptor_array();

   /// EXPERIMENTAL: DO NOT USE IF YOU ARE NOT ENGIN TOLA: tells to compute the
   /// scales for every pixel so that the resulting descriptors are scale
   /// invariant.
   inline void scale_invariant( bool state=true )
      {
         g_scale_en = (int)( (log(g_sigma_2/g_sigma_0)) / log(g_sigma_step) ) - g_scale_st;
         m_scale_invariant = state;
      }

   /// EXPERIMENTAL: DO NOT USE IF YOU ARE NOT ENGIN TOLA: tells to compute the
   /// orientations for every pixel so that the resulting descriptors are
   /// rotation invariant. orientation steps are 360/ori_resolution
   inline void rotation_invariant(int ori_resolution=36, bool state=true)
      {
         m_rotation_invariant = state;
         m_orientation_resolution = ori_resolution;
      }

   /// sets the gaussian variances manually. must be called before
   /// initialize() to be considered. must be exact sigma values -> f
   /// converts to incremental format.
   void set_level_gaussians( float* sigma_array, int sz );

   inline int* get_orientation_map()
      {
         return m_orientation_map;
      }

   void set_descriptor_memory( float* descriptor, long int d_size );
   void set_workspace_memory( float* workspace, long int w_size );

   /// Holds the coordinates (y,x) of the grid points of the region.
   float** m_grid_points;

private:

   /// initializes the class: computes gradient and structure-points
   void initialize();

   inline int quantize_radius( float rad )
      {
         if( rad <= m_level_sigmas[0              ] ) return 0;
         if( rad >= m_level_sigmas[g_cube_number-1] ) return g_cube_number-1;

         for( int c=0; c<g_cube_number-1; c++ )
            if( rad >= m_level_sigmas[c] && rad < m_level_sigmas[c+1] )
               return c;
         return -1;
      }

   /// compute the smoothed gradient layers.
   inline void compute_smoothed_gradient_layers();

   /// records the histogram that is computed by trilinear interpolation
   /// regarding the shift in layers and spatial coordinates. hcube is the
   /// histogram cube for a constant smoothness level.
   inline void trilinear_interpolated_histogram( float* descriptor, float y, float x, float shift, float* hcube )
      {
         int ishift = int( shift );
         float hlayer_alpha  = shift - ishift;
         if( hlayer_alpha < 0.001 ) hlayer_alpha = 0.0f;
         if( hlayer_alpha > 0.999 ) hlayer_alpha = 1.0f;

         int mnx = int( x );
         int mny = int( y );

         if( mnx >= m_w-2  || mny >= m_h-2 )
         {
            memset(descriptor, 0, sizeof(float)*m_hist_th_q_no);
            return;
         }

         float alpha = mnx + 1 - x;
         if( alpha < 0.001 ) alpha = 0.0f;
         if( alpha > 0.999 ) alpha = 1.0f;

         float beta  = mny +1 - y;
         if( beta < 0.001 ) beta = 0.0f;
         if( beta > 0.999 ) beta = 1.0f;

         int spatial_shift = mny * m_w + mnx;
         int data_size =  m_w * m_h;

         // shift and intepolate between layers
         for( int h=0; h<m_hist_th_q_no; h++ )
         {
            int layer_below = h+ishift;
            if( layer_below >= m_hist_th_q_no ) layer_below -= m_hist_th_q_no;

            int layer_above = layer_below + 1;
            if( layer_above >= m_hist_th_q_no ) layer_above -= m_hist_th_q_no;

            // A C --> pixel positions
            // B D

            float* A = hcube + layer_above*data_size + spatial_shift;
            float* B = A+m_w;
            float* C = A+1;
            float* D = B+1;

            float above_interpolation =
               beta     * (alpha * ( *A - *C ) + *C ) +
               (1-beta) * (alpha * ( *B - *D ) + *D );

            A = hcube + layer_below*data_size + spatial_shift;
            B = A+m_w;
            C = A+1;
            D = B+1;

            float below_interpolation =
               beta     * (alpha * ( *A - *C ) + *C ) +
               (1-beta) * (alpha * ( *B - *D ) + *D );

            descriptor[h] = hlayer_alpha * ( above_interpolation - below_interpolation ) + below_interpolation;
         }
      }

   /// records the histogram that is computed by bilinear interpolation
   /// regarding the shift in the spatial coordinates. hcube is the
   /// histogram cube for a constant smoothness level.
   inline void bilinear_interpolated_histogram( float* descriptor, float y, float x, int shift, float* hcube )
      {
         int mnx = int( x );
         int mny = int( y );

         if( mnx >= m_w-2  || mny >= m_h-2 )
         {
            memset(descriptor, 0, sizeof(float)*m_hist_th_q_no);
            return;
         }

         float alpha = mnx + 1 - x;
         if( alpha < 0.001 ) alpha = 0.0f;
         if( alpha > 0.999 ) alpha = 1.0f;

         float beta  = mny +1 - y;
         if( beta < 0.001 ) beta = 0.0f;
         if( beta > 0.999 ) beta = 1.0f;

         int spatial_shift = mny * m_w + mnx;
         int data_size =  m_w * m_h;

         for( int h=0; h<m_hist_th_q_no; h++ )
         {
            int layer = h + shift;
            if( layer >= m_hist_th_q_no ) layer -= m_hist_th_q_no;

            // A C --> pixel positions
            // B D
            float* A = hcube + layer*data_size  + spatial_shift;
            float* B = A+m_w;
            float* C = A+1;
            float* D = B+1;

            descriptor[h] =
               beta     * (alpha * ( *A - *C ) + *C ) +
               (1-beta) * (alpha * ( *B - *D ) + *D ) ;
         }
      }

   void normalize_sift_way( float* desc );
   void normalize_partial ( float* desc );
   void normalize_full    ( float* desc );

   /// applies one of the normalizations (sift,partial,full) to the desciptors.
   void normalize_descriptors();

   inline void normalize_descriptor(float* desc)
      {
         normalize_partial(desc);
      }

   int filter_size( float sigma );

   /// computes scales for every pixel and scales the structure grid so that the
   /// resulting descriptors are scale invariant.  you must set
   /// m_scale_invariant flag to 1 for the program to call this function
   void compute_scales();

   /// Return a number in the range [-0.5, 0.5] that represents the location of
   /// the peak of a parabola passing through the 3 evenly spaced samples.  The
   /// center value is assumed to be greater than or equal to the other values
   /// if positive, or less than if negative.
   float interpolate_peak(float left, float center, float right);

   /// Smooth a histogram by using a [1/3 1/3 1/3] kernel.  Assume the histogram
   /// is connected in a circular buffer.
   void smooth_histogram(float *hist, int bins);

   /// computes pixel orientations and rotates the structure grid so that
   /// resulting descriptors are rotation invariant. If the scales is also
   /// detected, then orientations are computed at the computed scales. you must
   /// set m_rotation_invariant flag to 1 for the program to call this function
   void compute_orientations();

   /// the clipping threshold to use in normalization: values above this value
   /// are clipped to this value for normalize_sift_way() function
   float m_descriptor_normalization_threshold;

   /// Computes the locations of the unscaled unrotated points where the
   /// histograms are going to be computed according to the given parameters.
   void compute_grid_points();

   /// Computes the locations of the unscaled rotated points where the
   /// histograms are going to be computed according to the given parameters.
   void compute_oriented_grid_points();

   /// Sets the locations of the unscaled unrotated points where the histograms
   /// are going to be computed. Call this function before initializion.
   void set_grid_points();

   /// smooths each of the layers by a Gaussian having "sigma" standart
   /// deviation.
   void smooth_layers( float*layers, int layer_number, float sigma );

   /// if set to true, no verbose information is printed. should change for
   /// different levels of verbosity.
   size_t m_verbosity;

   float* m_image;
   int  m_w, m_h;

   /// if set to false, destructor won't delete m_dense_descriptors.
   bool m_release_descriptors;

   /// stores the descriptors : its size is [ m_w * m_h * m_descriptor_size ].
   float* m_dense_descriptors;

   /// stores the gradient magnitudes: each layer is a different orientation.
   float* m_gradient_layers;

   /// stores the layered gradients in successively smoothed form: layer[n] =
   /// m_gradient_layers * gaussian( sigma_n ); n>= 0
   float* m_smoothed_gradient_layers;

   /// if set to true, descriptors are scale invariant
   bool m_scale_invariant;

   /// if set to true, descriptors are rotation invariant
   bool m_rotation_invariant;

   /// number of bins in the histograms while computing orientation
   int m_orientation_resolution;

   /// hold the scales of the pixels
   float* m_scale_map;

   /// holds the orientaitons of the pixels
   int* m_orientation_map;

   /// Holds the oriented coordinates (y,x) of the grid points of the region.
   float** m_oriented_grid_points;

   /// holds the gaussian sigmas for radius quantizations for an incremental
   /// application
   float* m_level_sigmas;

   bool m_descriptor_memory;
   bool m_workspace_memory;

   /// the number of grid locations
   int m_grid_point_number;

   /// the size of the descriptor vector
   int m_descriptor_size;
};

#endif
