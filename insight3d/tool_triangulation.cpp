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

#include "tool_triangulation.h"

const size_t compute_normals_K = 200;

void tool_triangulate_surface_reconstruction()
{
	if (!INDEX_IS_SET(ui_state.current_shot)) 
	{
		printf("Current shot must be selected.\n");
		return;
	}

	ASSERT(INDEX_IS_SET(ui_state.current_shot), "current shot not set");
	const Shot * const shot = shots.data + ui_state.current_shot;

	// * calculate delaunay triangulation *

	// initialize delaunay
	CvMemStorage * mem_storage = cvCreateMemStorage();
	CvSubdiv2D * subdivision = cvCreateSubdiv2D(CV_SEQ_KIND_SUBDIV2D, sizeof(*subdivision), sizeof(CvSubdiv2DPoint), sizeof(CvQuadEdge2D), mem_storage);
	cvInitSubdivDelaunay2D(
		subdivision, 
		cvRect(
			0, 
			0, 
			shot->width,
			shot->height
		)
	);

	for ALL(shot->points, i)
	{
		const Point * const point = shot->points.data + i; 

		if (!(vertices.data[point->vertex].reconstructed)) continue; // note there should be macro for this

		double 
			x = point->x * shot->width,
			y = point->y * shot->height
		;

		if (x < 0 || x > shot->width || y < 0 || y > shot->height) continue;

		cvSubdivDelaunay2DInsert(subdivision, 
			cvPoint2D32f(
				x, 
				y
			)
		); 
	}

	// * insert the triangles (supported by RANSACing around vertices) * 

	// find the edges in the triangulation (sad and stupid) 
	std::map<int, std::map<int, unsigned int> > picture_edges;
	{
		CvSeqReader reader;
		int total = subdivision->edges->total;
		int elem_size = subdivision->edges->elem_size;
		cvStartReadSeq((CvSeq *)(subdivision->edges), &reader, 0);

		for (int i = 0; i < total; i++)
		{
			CvQuadEdge2D * edge = (CvQuadEdge2D *)(reader.ptr);

			if (CV_IS_SET_ELEM(edge))
			{
				CvSubdiv2DPoint 
					* org = cvSubdiv2DEdgeOrg((CvSubdiv2DEdge)edge), 
					* dst = cvSubdiv2DEdgeDst((CvSubdiv2DEdge)edge)
				;

				size_t org_id, dst_id;
				bool org_found = false, dst_found = false;

				// find these two points among the ones in the pictures
				for ALL(shot->points, j) 
				{
					const Point * const point = shot->points.data + j;

					if (!(vertices.data[point->vertex].reconstructed)) continue;

					if (nearly_zero(
							sqr_value(org->pt.x - point->x * shot->width) + 
							sqr_value(org->pt.y - point->y * shot->height)
						)
					)
					{
						org_found = true;
						org_id = j;
					}

					if (nearly_zero(
							sqr_value(dst->pt.x - point->x * shot->width) + 
							sqr_value(dst->pt.y - point->y * shot->height)
						)
					)
					{
						dst_found = true;
						dst_id = j;
					}
				}

				// add the edge 
				if (org_found && dst_found)
				{
					// only if it's supported by some set
					// if (detected_edges[shot->points.data[org_id].vertex][shot->points.data[dst_id].vertex] > 0)
					{
						picture_edges[org_id][dst_id]++;
						picture_edges[dst_id][org_id]++;
					}
				}
			}

			CV_NEXT_SEQ_ELEM(elem_size, reader);
		}
	}

	// now create the triangles 
	std::set<std::pair<int, std::pair<int, int> > > inserted;
	for (std::map<int, std::map<int, unsigned int> >::iterator edge_i1 = picture_edges.begin(); edge_i1 != picture_edges.end(); ++edge_i1)
	{
		for (std::map<int, unsigned int>::iterator edge_i2 = edge_i1->second.begin(); edge_i2 != edge_i1->second.end(); ++edge_i2)
		{
			if (edge_i2->first <= edge_i1->first) continue;

			if (edge_i2->second >= 1) 
			{
				// find common neighbours of these two vertices to form triangles 
				std::map<int, std::map<int, unsigned int> >::iterator
					edge_i2_neighbours = picture_edges.find(edge_i2->first);

				std::map<int, unsigned int>::iterator 
					edge_i1i = edge_i1->second.begin(), 
					edge_i2i = edge_i2_neighbours->second.begin();

				while (edge_i1i != edge_i1->second.end() && edge_i2i != edge_i2_neighbours->second.end())
				{
					if (edge_i1i->first == edge_i2i->first)
					{
						if (edge_i1i->first != edge_i1->first && edge_i1i->first != edge_i2->first)
						{
							if (edge_i1i->second >= 1 && edge_i2i->second >= 1) 
							{
								if (inserted.find(
									std::make_pair<int, std::pair<int, int> >(
											edge_i1->first,
											std::make_pair<int, int>(edge_i2->first, edge_i1i->first)
										)                 
									) == inserted.end())
								{
									inserted.insert(std::make_pair<int, std::pair<int, int> >(
										edge_i1->first,
										std::make_pair<int, int>(edge_i2->first, edge_i1i->first)
									));

									/*if (
										!vertices.data[shot->points.data[edge_i1->first].vertex].reconstructed || 
										!vertices.data[shot->points.data[edge_i2->first].vertex].reconstructed || 
										!vertices.data[shot->points.data[edge_i1i->first].vertex].reconstructed
									) 
									{
										printf("x");
									}*/ // debug

									/*if (
										detected_edges[shot->points.data[edge_i1->first].vertex][shot->points.data[edge_i2->first].vertex] > 0 ||
										detected_edges[shot->points.data[edge_i1->first].vertex][shot->points.data[edge_i1i->first].vertex] > 0 || 
										detected_edges[shot->points.data[edge_i2->first].vertex][shot->points.data[edge_i1i->first].vertex] > 0
									)*/
									{
										// * 2d compactness * 

										// length 
										const double 
											len_2d_a = sqrt(distance_sq_2(
												shot->points.data[edge_i1->first].x * shot->width, shot->points.data[edge_i1->first].y * shot->height,
												shot->points.data[edge_i2->first].x * shot->width, shot->points.data[edge_i2->first].y * shot->height
											)), 
											len_2d_b = sqrt(distance_sq_2(
												shot->points.data[edge_i2->first].x * shot->width, shot->points.data[edge_i2->first].y * shot->height,
												shot->points.data[edge_i1i->first].x * shot->width, shot->points.data[edge_i1i->first].y * shot->height
											)),
											len_2d_c = sqrt(distance_sq_2(
												shot->points.data[edge_i1i->first].x * shot->width, shot->points.data[edge_i1i->first].y * shot->height,
												shot->points.data[edge_i1->first].x * shot->width, shot->points.data[edge_i1->first].y * shot->height
											))
										;

										const double len_2d = len_2d_a + len_2d_b + len_2d_c;

										// area 
										const double s_2d = 0.5 * len_2d;
										const double area_2d = sqrt(s_2d * (s_2d - len_2d_a) * (s_2d - len_2d_b) * (s_2d - len_2d_c));
										const double sug_2d = sqr_value(len_2d / 4.0);
										const double c_2d = abs(1 - sug_2d / area_2d);

										// * 3d compactness *  

										// length
										const double 
											len_3d_a = sqrt(distance_sq_3(
												vertices.data[shot->points.data[edge_i1->first].vertex].x,
												vertices.data[shot->points.data[edge_i1->first].vertex].y,
												vertices.data[shot->points.data[edge_i1->first].vertex].z,
												vertices.data[shot->points.data[edge_i2->first].vertex].x,
												vertices.data[shot->points.data[edge_i2->first].vertex].y,
												vertices.data[shot->points.data[edge_i2->first].vertex].z
											)),
											len_3d_b = sqrt(distance_sq_3(
												vertices.data[shot->points.data[edge_i2->first].vertex].x,
												vertices.data[shot->points.data[edge_i2->first].vertex].y,
												vertices.data[shot->points.data[edge_i2->first].vertex].z,
												vertices.data[shot->points.data[edge_i1i->first].vertex].x,
												vertices.data[shot->points.data[edge_i1i->first].vertex].y,
												vertices.data[shot->points.data[edge_i1i->first].vertex].z
											)),
											len_3d_c = sqrt(distance_sq_3(
												vertices.data[shot->points.data[edge_i2->first].vertex].x,
												vertices.data[shot->points.data[edge_i2->first].vertex].y,
												vertices.data[shot->points.data[edge_i2->first].vertex].z,
												vertices.data[shot->points.data[edge_i1i->first].vertex].x,
												vertices.data[shot->points.data[edge_i1i->first].vertex].y,
												vertices.data[shot->points.data[edge_i1i->first].vertex].z
											))
										;

										const double len_3d = len_3d_a + len_3d_b + len_3d_c;

										// area
										const double s_3d = 0.5 * len_3d;
										const double area_3d = sqrt(s_3d * (s_3d - len_3d_a) * (s_3d - len_3d_b) * (s_3d - len_3d_c));
										const double sug_3d = sqr_value(len_3d / 4.0); 
										const double c_3d = abs(1 - sug_3d / area_3d);

										if (area_2d > 0 && area_3d > 0 && c_3d < 1 && c_2d < 1 && abs(c_2d - c_3d) < 0.1) 
										{
											size_t polygon_id;
											geometry_new_polygon(polygon_id);
											ADD(polygons.data[polygon_id].vertices);
											LAST(polygons.data[polygon_id].vertices).value = shot->points.data[edge_i1->first].vertex;
											ADD(polygons.data[polygon_id].vertices);
											LAST(polygons.data[polygon_id].vertices).value = shot->points.data[edge_i2->first].vertex;
											ADD(polygons.data[polygon_id].vertices);
											LAST(polygons.data[polygon_id].vertices).value = shot->points.data[edge_i1i->first].vertex;
										}
										else
										{
											printf("%f \t %f\n", c_2d, c_3d);
										}
									}
								}
							}
						}

						++edge_i1i;
						++edge_i2i;
					}
					else if (edge_i1i->first < edge_i2i->first)
					{
						++edge_i1i;
					}
					else if (edge_i1i->first > edge_i2i->first)
					{
						++edge_i2i;
					}
				}
			}
		}
	}

	// now add the triangles 
	/*for ALL(shot->points, i) 
	{
		for (size_t j = i + 1; j < shot->points.count; j++) if (shot->points.data[j].set) 
		{
			for (size_t k = j + 1; k < shot->points.count; k++) if (shot->points.data[k].set) 
			{
				// is this a triangle in the plane? 
				if (picture_edges[i][j] && picture_edges[j][k] && picture_edges[k][i]) 
				{
					// ok, let's fucking add it 
					size_t polygon_id;
					geometry_new_polygon(polygon_id);
					ADD(polygons.data[polygon_id].vertices);
					LAST(polygons.data[polygon_id].vertices).value = shot->points.data[i].vertex;
					ADD(polygons.data[polygon_id].vertices);
					LAST(polygons.data[polygon_id].vertices).value = shot->points.data[j].vertex;
					ADD(polygons.data[polygon_id].vertices);
					LAST(polygons.data[polygon_id].vertices).value = shot->points.data[k].vertex;
				}
			}
		}
	}*/
}

// create triangulation module 
void tool_triangulation_create() 
{
	tool_create(UI_MODE_UNSPECIFIED, "Triangulation of vertices", "Allows to triangulate vertices marked on individual shots by points"); 
	tool_register_menu_function("Main menu|Modelling|Triangulate user vertices|", tool_triangulate_vertices_user);
	tool_register_menu_function("Main menu|Modelling|Triangulate all vertices|", tool_triangulate_vertices);
	tool_register_menu_function("Main menu|Modelling|Triangulate, only trusted|", tool_triangulate_vertices_trusted);
	tool_register_menu_function("Main menu|Modelling|Triangulate, only selected shots|", tool_triangulate_vertices_using_selected_shots);
	tool_register_menu_function("Main menu|Modelling|Clear positions of all vertices|", tool_triangulate_clear);
	tool_register_menu_function("Main menu|Modelling|Compute vertex normals|", tool_triangulate_compute_normals); 
	tool_register_menu_function("Main menu|Modelling|Surface reconstruction|", tool_triangulate_surface_reconstruction);
}

// refresh ui after triangulation 
void triangulate_refresh_ui()
{
	visualization_process_data(vertices, shots);
}

// triangulate all vertices created by the user
void tool_triangulate_vertices_user()
{
	action_triangulate_vertices(NULL, MVG_MIN_INLIERS_TO_TRIANGULATE, MVG_MIN_INLIERS_TO_TRIANGULATE_WEAKER, true, 50); // note that we force larger measurement threshold, this should be replaced by constant
	triangulate_refresh_ui();
}

// simply triangulate all vertices
void tool_triangulate_vertices()
{
	action_triangulate_vertices(NULL);
	triangulate_refresh_ui();
}

// triangulate all vertices using more restrictive constraints 
void tool_triangulate_vertices_trusted() 
{
	action_triangulate_vertices(NULL, 3, 3);
	triangulate_refresh_ui();
}

// triangulate vertices, but use only selected shots 
void tool_triangulate_vertices_using_selected_shots()
{
	bool * shots_to_use = ALLOC(bool, shots.count);

	if (!shots_to_use) 
	{
		core_state.error = CORE_ERROR_OUT_OF_MEMORY; 
		abort();
	}

	for ALL(shots, i) 
	{
		const Shot * const shot = shots.data + i;
		const UI_Shot_Meta * const meta = ui_check_shot_meta(i);

		shots_to_use[i] = meta->selected;
	}

	action_triangulate_vertices(shots_to_use);

	FREE(shots_to_use);

	triangulate_refresh_ui();
}

// forgets the position of all vertices 
void tool_triangulate_clear()
{
	for ALL(vertices, i) 
	{
		Vertex * const vertex = vertices.data + i; 

		vertex->reconstructed = false; 
		vertex->x = 0;
		vertex->y = 0; 
		vertex->z = 0;
	}

	triangulate_refresh_ui();
}

// compute normals using robust estimation for one particular vertex
void compute_vertex_normal_from_pointcloud(const size_t vertex_id, ANNkd_tree * ann_kdtree, size_t * vertices_reindex)
{
	Vertex * vertex = vertices.data + vertex_id;

	// we want to find k nearest points for this vertex
	size_t * nearest_ids = ALLOC(size_t, compute_normals_K);
	double * nearest_d = ALLOC(double, compute_normals_K);
	size_t nearest_count = 0;
	memset(nearest_ids, 0, sizeof(size_t) * compute_normals_K);
	memset(nearest_d, 0, sizeof(double) * compute_normals_K);

	// naive implementation
	// go through all vertices
	/*for ALL(vertices, i) 
	{
		if (i == vertex_id) continue;
		const Vertex * searched = vertices.data + i; 
		if (!searched->reconstructed) continue;

		// calculate the distance
		const double distance = 
			sqr_value(searched->x - vertex->x) + 
			sqr_value(searched->y - vertex->y) + 
			sqr_value(searched->z - vertex->z)
		;

		// keep a list of vertices sorted by distance
		bool found = false;
		size_t temp_id;
		double temp_d;

		for (size_t j = 0; j < compute_normals_K; j++)
		{
			bool empty = j >= nearest_count; 

			// decide if we'll place the value here 
			if (!found && (empty || nearest_d[j] > distance))
			{
				temp_id = i;
				temp_d = distance;
				if (nearest_count < compute_normals_K) 
				{
					nearest_count++;
				}
				found = true; 
			}

			// put it there and shift the rest
			if (empty || found) 
			{
				swap_size_t(temp_id, nearest_ids[j]); 
				swap_double(temp_d, nearest_d[j]);
			}

			if (empty) break; 
		}
	}*/

	// find the K nearest vertices
	ANNidxArray ann_ids = new ANNidx[compute_normals_K];
	ANNdistArray ann_ds = new ANNdist[compute_normals_K];
	ANNpoint ann_point = annAllocPt(3);
	ann_point[0] = vertex->x; 
	ann_point[1] = vertex->y; 
	ann_point[2] = vertex->z; 
	ann_kdtree->annkSearch(ann_point, compute_normals_K, ann_ids, ann_ds, 0.05);

	// save the results into our data structures 
	for (size_t i = 0; i < compute_normals_K; i++) 
	{
		nearest_d[i] = sqrt(ann_ds[i]); 
		nearest_ids[i] = vertices_reindex[ann_ids[i]];
	}

	// release ANN resources 
	delete[] ann_ids; 
	delete[] ann_ds;

	// estimate the plane using RANSAC
	double * plane = tool_plane_extraction_subset(vertices, nearest_ids, nearest_count = compute_normals_K);

	if (plane)
	{
		normalize_vector(plane, 3); 
		vertex->nx = plane[X]; 
		vertex->ny = plane[Y];
		vertex->nz = plane[Z];

		FREE(plane);

		// get first camera observing this point 
		// todo we should consider all shots and take the majority 
		const Vertex_Incidence * incidence = vertices_incidence.data + vertex_id; 
		for ALL(incidence->shot_point_ids, i) 
		{
			const Double_Index * id = incidence->shot_point_ids.data + i; 
			const Shot * shot = shots.data + id->primary;

			if (!shot->calibrated) continue;

			// decide orientation 
			// note that we're relying on that the T is precomputed, which is stupid
			// todo fix that (see the note above)
			bool invert = dot_3(plane, shot->T) > plane[3];

			if (invert)
			{
				vertex->nx *= -1; 
				vertex->ny *= -1; 
				vertex->nz *= -1; 
			}

			break;
		}
	}

	// release resources 
	FREE(nearest_ids); 
	FREE(nearest_d);

	// debug print distances 
	/*printf("%d: ", vertices.count);
	for (size_t i = 0; i < K; i++) 
	{
		printf("%lf[%d] ", nearest_d[i], nearest_ids[i]);
	}
	
	printf("\n");*/
}

// compute normals using robust estimation for all vertices 
void tool_triangulate_compute_normals() 
{
	// export vertices into array for ANN 
	size_t * vertices_refactor = ALLOC(size_t, vertices.count);
	
	ANNpointArray ann_points = annAllocPts(vertices.count, 3);

	size_t count = 0;
	for ALL(vertices, i) 
	{
		if (!vertices.data[i].reconstructed) continue; 

		vertices_refactor[count] = i;
		ann_points[count][0] = vertices.data[i].x;
		ann_points[count][1] = vertices.data[i].y;
		ann_points[count][2] = vertices.data[i].z;

		count++;
	}

	ANNkd_tree * ann_kdtree = new ANNkd_tree(ann_points, count, 3);

	for ALL(vertices, i) 
	{
		if (vertices.data[i].reconstructed) 
		{
			compute_vertex_normal_from_pointcloud(i, ann_kdtree, vertices_refactor);
		}

		if (i % 100 == 0) printf("."); 
	}
	printf("\n");

	free(vertices_refactor);
	delete ann_kdtree;

	// * create detected polygons *
	/*const int detected_edges_threshold = 1;

	// go through all edges 
	std::set<std::pair<int, std::pair<int, int> > > inserted;
	for (std::map<int, std::map<int, unsigned int> >::iterator edge_i1 = detected_edges.begin(); edge_i1 != detected_edges.end(); ++edge_i1)
	{
		for (std::map<int, unsigned int>::iterator edge_i2 = edge_i1->second.begin(); edge_i2 != edge_i1->second.end(); ++edge_i2) 
		{
			if (edge_i2->first <= edge_i1->first) continue;

			if (edge_i2->second >= detected_edges_threshold) 
			{
				// find common neighbours of these two vertices to form triangles 
				std::map<int, std::map<int, unsigned int> >::iterator
					edge_i2_neighbours = detected_edges.find(edge_i2->first);

				std::map<int, unsigned int>::iterator 
					edge_i1i = edge_i1->second.begin(), 
					edge_i2i = edge_i2_neighbours->second.begin();

				while (edge_i1i != edge_i1->second.end() && edge_i2i != edge_i2_neighbours->second.end())
				{
					if (edge_i1i->first == edge_i2i->first)
					{
						if (edge_i1i->first != edge_i1->first && edge_i1i->first != edge_i2->first)
						{
							if (edge_i1i->second >= detected_edges_threshold && edge_i2i->second >= detected_edges_threshold) 
							{
								if (inserted.find(
									std::make_pair<int, std::pair<int, int> >(
											edge_i1->first,
											std::make_pair<int, int>(edge_i2->first, edge_i1i->first)
										)
									) == inserted.end())
								{
									inserted.insert(std::make_pair<int, std::pair<int, int> >(
											edge_i1->first,
											std::make_pair<int, int>(edge_i2->first, edge_i1i->first)
										));

									size_t polygon_id;
									geometry_new_polygon(polygon_id);
									ADD(polygons.data[polygon_id].vertices);
									LAST(polygons.data[polygon_id].vertices).value = edge_i1->first;
									ADD(polygons.data[polygon_id].vertices);
									LAST(polygons.data[polygon_id].vertices).value = edge_i2->first;
									ADD(polygons.data[polygon_id].vertices);
									LAST(polygons.data[polygon_id].vertices).value = edge_i1i->first;
								}
							}
						}

						++edge_i1i;
						++edge_i2i;
					}
					else if (edge_i1i->first < edge_i2i->first)
					{
						++edge_i1i;
					}
					else if (edge_i1i->first > edge_i2i->first)
					{
						++edge_i2i;
					}
				}
			}
		}
	}*/
}

