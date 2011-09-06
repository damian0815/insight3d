/*

  insight3d - image based 3d modelling software
  Copyright (C) 2007-2008  Lukas Mach
                           email: lukas.mach@gmail.com 
                           web: http://mach.matfyz.cz/

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
  
*/

#include "actions.h"

// triangulates vertices given current projection matrices and vertex projections
bool action_triangulate_vertices(
	bool * shots_to_use /*= NULL*/, 
	const int min_inliers /*= MVG_MIN_INLIERS_TO_TRIANGULATE*/, 
	const int min_inliers_weaker /*= MVG_MIN_INLIERS_TO_TRIANGULATE_WEAKER*/,
	const bool only_manual /*= false*/,
	const double measurement_threshold /*= MVG_MEASUREMENT_THRESHOLD*/
)
{
	opencv_begin();

	printf(".");

	// reconstruct all vertices
	for ALL(vertices, i)
	{
		Vertex * vertex = vertices.data + i;
		CvMat * reconstructed_vertex = NULL; 

		if (only_manual && vertex->vertex_type != GEOMETRY_VERTEX_USER) continue;

		if (i % 100 == 0) 
		{
			printf(".");
		}
		 
		// if incidence is defined (i.e., if the vertex has some 2d points)
		if (IS_SET(vertices_incidence, i)) 
		{
			// feed the data inside appropriate matrices 
			const CvMat * * projection_matrices;
			CvMat * points; 
			bool publish_status = publish_triangulation_data(vertices_incidence.data[i], i, projection_matrices, points, shots_to_use);

			// call reconstruction algorithm
			if (publish_status) 
			{
				reconstructed_vertex = mvg_triangulation_RANSAC(
					projection_matrices, points, true, false, 
					min_inliers, 
					min_inliers_weaker, 
					MVG_RANSAC_TRIANGULATION_TRIALS, 
					MVG_MEASUREMENT_THRESHOLD
				);
				FREE(projection_matrices); 
				cvReleaseMat(&points);
			}
		}

		// if the reconstruction succeeded
		if (reconstructed_vertex) 
		{
			if (reconstructed_vertex->rows == 4) opencv_normalize_homogeneous(reconstructed_vertex);

			// save calculated coordinates
			vertex->reconstructed = true; 
			vertex->x = OPENCV_ELEM(reconstructed_vertex, 0, 0); 
			vertex->y = OPENCV_ELEM(reconstructed_vertex, 1, 0); 
			vertex->z = OPENCV_ELEM(reconstructed_vertex, 2, 0);
		} 
		else
		{
			// mark as unreconstructed
			vertex->reconstructed = false;
			vertex->x = 0;
			vertex->y = 0;
			vertex->z = 0;
		}

		// release resources
		cvReleaseMat(&reconstructed_vertex); 
	}

	printf("\n");
	opencv_end();
	return true;
}

// compute projection matrix of current camera from 3d to 2d correspondences
bool action_camera_resection(size_t shot_id, const bool enforce_square_pixels, const bool enforce_zero_skew)
{
	ASSERT_IS_SET(shots, shot_id);
	Shot * const shot = shots.data + shot_id;

	// first count how many points on this shot have their vertices reconstructed 
	size_t n = 0; 
	for ALL(shots.data[shot_id].points, i) 
	{
		const Point * const point = shots.data[shot_id].points.data + i; 
		if (vertices.data[point->vertex].reconstructed) 
		{
			n++;
		}
	}

	if (n < 6) return false;

	// go through all points on this shot and fill them and their vertices 
	// into respective matrices 
	opencv_begin();
	CvMat * mat_vertices = cvCreateMat(3, n, CV_64F), * mat_projected = cvCreateMat(2, n, CV_64F);
	n = 0;
	for ALL(shots.data[shot_id].points, i) 
	{
		const Point * const point = shots.data[shot_id].points.data + i; 
		const Vertex * const vertex = vertices.data + point->vertex;
		if (!vertex->reconstructed) continue;

		// copy the values into matrices
		OPENCV_ELEM(mat_vertices, 0, n) = vertex->x; 
		OPENCV_ELEM(mat_vertices, 1, n) = vertex->y; 
		OPENCV_ELEM(mat_vertices, 2, n) = vertex->z; 
		OPENCV_ELEM(mat_projected, 0, n) = point->x * shots.data[shot_id].width; 
		OPENCV_ELEM(mat_projected, 1, n) = point->y * shots.data[shot_id].height; 

		n++;
	}

	// perform the calculation
	CvMat
		* R = opencv_create_matrix(3, 3),
		* K = opencv_create_matrix(3, 3),
		* T = opencv_create_matrix(3, 1),
		* P = opencv_create_matrix(3, 4);

	bool ok = mvg_resection_RANSAC(mat_vertices, mat_projected, P, K, R, T, false);

	// check result and optionally enforce some constraints
	if (ok)
	{
		// enforce restrictions on calibration matrix 
		if (!mvg_restrict_calibration_matrix(K, enforce_zero_skew, enforce_square_pixels))
		{
			cvReleaseMat(&R);
			cvReleaseMat(&K);
			cvReleaseMat(&T);
			cvReleaseMat(&P);
			cvReleaseMat(&mat_vertices);
			cvReleaseMat(&mat_projected);
			opencv_end();
			return false;
		}

		// if it has been calibrated before, release previous calibration
		if (shot->calibrated) 
		{
			cvReleaseMat(&shot->projection); 
			cvReleaseMat(&shot->rotation);
			cvReleaseMat(&shot->translation); 
			cvReleaseMat(&shot->internal_calibration); 
		}

		// save estimated matrices into shot structure
		shot->projection = P;
		shot->rotation = R; 
		shot->translation = T; 
		shot->internal_calibration = K;

		// fill in the rest of the calibration 
		opencv_end();
		geometry_calibration_from_decomposed_matrices(shot_id);
		opencv_begin();

		// mark as calibrated
		shot->calibrated = true;
	}

	// release resources 
	cvReleaseMat(&mat_vertices); 
	cvReleaseMat(&mat_projected);
	opencv_end();

	return ok;
}
