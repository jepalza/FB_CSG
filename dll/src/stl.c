/* 
Copyright (C) 2018 Cyrus Welgan 
cyrus@c-csg.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#define _DEFAULT_SOURCE
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
//#include <endian.h>
#include "c-csg.h"
#include "bb.h"
#include "stl.h"


       
int slave_count = 4;
pthread_t *slaves;
struct MARCH_THREAD_DATA slave_msg;

static pthread_mutex_t slave_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fp_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t to_main = PTHREAD_COND_INITIALIZER;
static pthread_cond_t to_slave = PTHREAD_COND_INITIALIZER;

extern struct CSG_NODE *csg_root;

FILE *stlfp;
uint32_t triangle_count;


uint32_t htole32(uint32_t aa)
{
	return aa;
}

//vertex_offset lists the positions, relative to vertex0, of each of the 8 vertices of a cube
static const double vertex_offset[8][3] =
{
	{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}, {1.0, 0.0, 1.0}, {1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}
};

//tetra_edge_connection lists the index of the endpoint vertices for each of the 6 edges of the tetrahedron
static const int tetra_edge_connection[6][2] =
{
	{0, 1},  {1, 2},  {2, 0},  {0, 3},  {1, 3},  {2, 3}
};

//tetra_edge_connection lists the index of verticies from a cube
// that made up each of the six tetrahedrons within the cube
static const int tetras_in_cube[6][4] =
{
	{0, 5, 1, 6},
	{0, 1, 2, 6},
	{0, 2, 3, 6},
	{0, 3, 7, 6},
	{0, 7, 4, 6},
	{0, 4, 5, 6},
};



// For any edge, if one vertex is inside of the surface and the other is outside of the surface
//  then the edge intersects the surface
// For each of the 4 vertices of the tetrahedron can be two possible states : either inside or outside of the surface
// For any tetrahedron the are 2^4=16 possible sets of vertex states
// This table lists the edges intersected by the surface for all 16 possible vertex states
// There are 6 edges.  For each entry in the table, if edge #n is intersected, then bit #n is set to 1

static const int tetra_edge_flags[16] =
{
	0x00, 0x0d, 0x13, 0x1e, 0x26, 0x2b, 0x35, 0x38, 0x38, 0x35, 0x2b, 0x26, 0x1e, 0x13, 0x0d, 0x00
};


// For each of the possible vertex states listed in tetra_edge_flags there is a specific triangulation
// of the edge intersection points.  tetra_triangles lists all of them in the form of
// 0-2 edge triples with the list terminated by the invalid value -1.

int tetra_triangles[16][7] =
{
	{ -1, -1, -1, -1, -1, -1, -1},
	{ 0,  3,  2, -1, -1, -1, -1},
	{ 0,  1,  4, -1, -1, -1, -1},
	{ 1,  4,  2,  2,  4,  3, -1},

	{ 1,  2,  5, -1, -1, -1, -1},
	{ 0,  3,  5,  0,  5,  1, -1},
	{ 0,  2,  5,  0,  5,  4, -1},
	{ 5,  4,  3, -1, -1, -1, -1},

	{ 3,  4,  5, -1, -1, -1, -1},
	{ 4,  5,  0,  5,  2,  0, -1},
	{ 1,  5,  0,  5,  3,  0, -1},
	{ 5,  2,  1, -1, -1, -1, -1},

	{ 3,  4,  2,  2,  4,  1, -1},
	{ 4,  1,  0, -1, -1, -1, -1},
	{ 2,  3,  0, -1, -1, -1, -1},
	{ -1, -1, -1, -1, -1, -1, -1},
};





inline double find_edge_intersect(double *v1, double *v2)
{
	double p[3];
	double d[3];
	//search edge for intersection point
	int left = csg_point_in_solid(v1);
	int right = csg_point_in_solid(v2);

	if(left == right) //if both start and end are on the same side...
		return 0.5;

	//end points have different values here
	//find midpoint value, if the value is the same as the left, split right hand segment,
	//or if the value is different split left hand segment.
	//and so on, until subdivision depth is reached...
	//it is possible for more than one intersection to be in the space, but oh well.
	d[0] = v2[0] - v1[0];
	d[1] = v2[1] - v1[1];
	d[2] = v2[2] - v1[2];
	double move = 0.25; //divide this by two every loop
	int divs = 16;
	double spot = 0.5; //place on line

	for(int i = 0; i < divs; i++)
		{
			p[0] = v1[0] + d[0] * spot;
			p[1] = v1[1] + d[1] * spot;
			p[2] = v1[2] + d[2] * spot;
			int t = csg_point_in_solid(p);

			if(t != left) //subdivide left
				{
					spot -= move;
				}
			else //subdivide right
				{
					spot += move;
				}

			move /= 2.0;
		}

	return spot;
}




void march_tetrahedron(struct STL stl[], int *stlCount, double ptetrahedron_position[4][3], double ptetrahedron_value[4])
{
	int edge, vert0, vert1, edge_flags, triangle, v1, v2, v3, flag_index = 0;
	double f_offset, f_inv_offset;
	double edge_vertex[6][3];

	//Find which vertices are inside of the surface and which are outside
	for(v1 = 0; v1 < 4; v1++)
		{
			if(ptetrahedron_value[v1] < 1.0)
				flag_index |= 1 << v1;
		}

	//Find which edges are intersected by the surface
	edge_flags = tetra_edge_flags[flag_index];

	//If the tetrahedron is entirely inside or outside of the surface, then there will be no intersections
	if(edge_flags == 0)
		{
			return;
		}

	//Find the point of intersection of the surface with each edge
	// Then find the normal to the surface at those points
	for(edge = 0; edge < 6; edge++)
		{
			//if there is an intersection on this edge
			if(edge_flags & (1 << edge))
				{
					vert0 = tetra_edge_connection[edge][0];
					vert1 = tetra_edge_connection[edge][1];
					//approximate place of intersection on the edge
					//vMarchIntersect(&edge_vertex[edge],&ptetrahedron_position[vert0],&ptetrahedron_position[vert1]);
					f_offset = find_edge_intersect(ptetrahedron_position[vert0], ptetrahedron_position[vert1]);
					f_inv_offset = 1.0 - f_offset;
					edge_vertex[edge][0] = f_inv_offset * ptetrahedron_position[vert0][0]  +  f_offset * ptetrahedron_position[vert1][0];
					edge_vertex[edge][1] = f_inv_offset * ptetrahedron_position[vert0][1]  +  f_offset * ptetrahedron_position[vert1][1];
					edge_vertex[edge][2] = f_inv_offset * ptetrahedron_position[vert0][2]  +  f_offset * ptetrahedron_position[vert1][2];
					//vGetNormal(&asEdgeNorm[edge], edge_vertex[edge][0], edge_vertex[edge][1], edge_vertex[edge][2]);
				}
		}

	//Draw the triangles that were found.  There can be up to 2 per tetrahedron
	for(triangle = 0; triangle < 2; triangle++)
		{
			if(tetra_triangles[flag_index][3 * triangle] < 0)
				break;

			v1 = tetra_triangles[flag_index][3 * triangle];
			v2 = tetra_triangles[flag_index][3 * triangle + 1];
			v3 = tetra_triangles[flag_index][3 * triangle + 2];
			
					
			stl[*stlCount].v1[0] = (float)edge_vertex[v1][0];
			stl[*stlCount].v1[1] = (float)edge_vertex[v1][1];
			stl[*stlCount].v1[2] = (float)edge_vertex[v1][2];
			stl[*stlCount].v2[0] = (float)edge_vertex[v2][0];
			stl[*stlCount].v2[1] = (float)edge_vertex[v2][1];
			stl[*stlCount].v2[2] = (float)edge_vertex[v2][2];
			stl[*stlCount].v3[0] = (float)edge_vertex[v3][0];
			stl[*stlCount].v3[1] = (float)edge_vertex[v3][1];
			stl[*stlCount].v3[2] = (float)edge_vertex[v3][2];
			stlCount[0]++;
		}
}



void march_step(double fX, double fY, double fZ, double fScale)
{
	int vertex, tetrahedron, vertex_in_cube;
	double cube_position[8][3];
	int  cube_value[8];
	double tetrahedron_position[4][3];
	double  tetrahedron_value[4];
	struct STL stl[6 * 2]; //6 calls made here, x2 per tetra, leave room for worst case stl triangles
	int stl_count = 0;

	for(int i = 0; i < stl_count; i++) //clear normal and attribs before writing
		{
			stl[i].n[0] = 0.0;
			stl[i].n[1] = 0.0;
			stl[i].n[2] = 0.0;
			stl[i].attr = 0.0;
		}

	//Make a local copy of the cube's corner positions
	for(vertex = 0; vertex < 8; vertex++)
		{
			cube_position[vertex][0] = fX + vertex_offset[vertex][0] * fScale;
			cube_position[vertex][1] = fY + vertex_offset[vertex][1] * fScale;
			cube_position[vertex][2] = fZ + vertex_offset[vertex][2] * fScale;
		}

	//Make a local copy of the cube's corner values
	for(vertex = 0; vertex < 8; vertex++)
		{
			cube_value[vertex] = csg_point_in_solid(&cube_position[vertex][0]);
		}

	for(tetrahedron = 0; tetrahedron < 6; tetrahedron++)
		{
			for(vertex = 0; vertex < 4; vertex++)
				{
					vertex_in_cube = tetras_in_cube[tetrahedron][vertex];
					tetrahedron_position[vertex][0] = cube_position[vertex_in_cube][0];
					tetrahedron_position[vertex][1] = cube_position[vertex_in_cube][1];
					tetrahedron_position[vertex][2] = cube_position[vertex_in_cube][2];
					tetrahedron_value[vertex] = cube_value[vertex_in_cube];
				}

			march_tetrahedron(stl, &stl_count, tetrahedron_position, tetrahedron_value);
				
			
				
		}

	if(stl_count)
		{
			pthread_mutex_lock(&fp_mutex);
			
			#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
			for(int x=0;x<stl_count;x++)
			{
				uint32_t *l =(uint32_t *)&stl[x];
				for(int k = 0; k<12 ; k++)
					l[k]=htole32(l[k]);
				stl[x].attr=htole16(stl[x].attr);
			}			
			#endif
			
			fwrite(&stl[0], sizeof(struct STL)*stl_count, 1, stlfp);
			triangle_count += stl_count;
			pthread_mutex_unlock(&fp_mutex);
		}
}





void *slave_entry(void *xptr)
{
	struct MARCH_THREAD_DATA d;
	memset(&d, 0, sizeof(struct MARCH_THREAD_DATA));

	while(1)
		{
			pthread_mutex_lock(&slave_mutex);

			while(slave_msg.action == 0)
				{
					pthread_cond_signal(&to_main);	//signal to give work
					pthread_cond_wait(&to_slave, &slave_mutex);
				}

			memcpy(&d, &slave_msg, sizeof(struct MARCH_THREAD_DATA));
			slave_msg.action = 0; //mark consumed
			pthread_mutex_unlock(&slave_mutex);

			if(d.action == -1) //kill slave
				{
					pthread_cond_signal(&to_main);
					break;
				}

			for(double z = d.zstart; z < d.zend; z += d.resolution)
				{
					march_step(d.x, d.y, z, d.resolution);
				}

			//printf("done %u\n",pthread_self());
		}

	return NULL;
}


void create_slaves(int threads)
{
	slave_count = threads;

	if(slaves)
		free(slaves);

	slaves = malloc_or_die(sizeof(pthread_t) * slave_count);

	for(int i = 0; i < slave_count; i++)
		{
			if(pthread_create(&slaves[i], NULL, slave_entry, NULL))
				printf("error creating thread\n");
		}
}

void feed_slaves(double x, double y, double zstart, double zend, double resolution, int action)
{
	pthread_mutex_lock(&slave_mutex);
	slave_msg.x = x;
	slave_msg.y = y;
	slave_msg.zstart = zstart;
	slave_msg.zend = zend;
	slave_msg.resolution = resolution;
	slave_msg.action = action;

	while(slave_msg.action != 0)
		{
			pthread_cond_broadcast(&to_slave); //wake up workers
			pthread_cond_wait(&to_main, &slave_mutex); //wait for worker to signal us
		}

	pthread_mutex_unlock(&slave_mutex);
}

void wait_for_slaves(void)
{
	for(int i = 0; i < slave_count; i++)
		{
			feed_slaves(0, 0, 0, 0, 0, -1); //they only consume one of these before death
		}

	for(int i = 0; i < slave_count; i++)
		{
			pthread_join(slaves[i], NULL);
		}

	free(slaves);
	slaves = NULL;
}


//collect bboxes suitable for reducing marching effort, do not expand differences or intersections, but do explore root and unions
void stl_bb_collect(struct BBLIST **bl, struct CSG_NODE *n)
{
	if(!n->bbox_valid)
		return;

	//only record if we are difference, intersection, or a primitive

	switch(n->object_type)
		{
		case NTYPE_ROOT:		//do not use these bboxes
		case NTYPE_UNION:
			break;

		default:
			bb_list_add(bl, n->bbox);
		}

	//do not expand difference and intersection
	switch(n->object_type)
		{
		case NTYPE_DIFFERENCE:
		case NTYPE_INTERSECTION:
			return;

		default:
			;
		}

	for(int i = 0; i < n->children_len; i++)
		stl_bb_collect(bl, n->children[i]);
}






void stl_short_march(double resolution)
{
	struct BBLIST *bl = NULL;
	stl_bb_collect(&bl, csg_root);
	
	bb_match_resolution_expand(&bl, resolution);
	
	if(!bl)
		return;
	
	double extents[6];
	bb_list_extents(&bl,&extents[0]);

	
	int last_progress=-123;
	
	
	//now scan upwards
	double ystart,yend;
	double y = -DBL_MAX;		
	while( bb_list_next_lowest(&bl, 1, y, &ystart, &yend) )
	{
		
		
		for(y=ystart;y<yend;y+=resolution)
		{
			
			float progress = ( (y-extents[1]) / (extents[4]-extents[1]));				
			
			int iprogress = ((int)(progress*10.0f));
			if (  iprogress != last_progress )
			{
				last_progress=iprogress;
				if(10-iprogress>=0)
					printf("%u ",10-iprogress);
				fflush(stdout);
			}
			
			double xstart,xend;
			double x = -DBL_MAX;		
			while( bb_list_next_lowest(&bl, 0, x, &xstart, &xend) )
			{
				
				
				//printf("x %f -> %f\n",xstart,xend);
				
				for(x=xstart;x<xend;x+=resolution)
				{
					//send z work to slaves
					double zstart,zend;
					double z = -DBL_MAX;		
					while( bb_list_next_lowest(&bl, 2, z, &zstart, &zend) )
					{
						//printf("z %f -> %f\n",zstart,zend);
						
						feed_slaves(x, y, zstart, zend, resolution, 1);
						z=zend;
					}
				}
				
				
			}
			
			
		}
	}
	
	
			
			
			
		

	bb_list_free(&bl);
}


void csg_save_stl(char *file, double resolution, int threads)
{
	/*
	marching tetrahedrons ahoy
	*/
	csg_refresh_transforms();

	if(!csg_root->bbox_valid) //don't want to brute force the walk over the volume, so tell user not to use infinites except in differences.
		{
			printf("error: root bounding box invalid, probably from an infinite object being used outside of a difference object\n");
			return;
		}

	uint8_t stl_header[80];
	triangle_count = 0;
	stlfp = fopen(file, "wb");

	if(!stlfp)
		{
			printf("error opening %s\n", file);
			return;
		}

	memset(stl_header, 0, 80);
	fwrite(stl_header, 80, 1, stlfp);	
	fwrite(&triangle_count, sizeof(uint32_t), 1, stlfp); //write zero for now.. fill in later
		
	create_slaves(threads);	
	stl_short_march(resolution);	
	wait_for_slaves();
	
	fseek(stlfp, 80, SEEK_SET);
	uint32_t count_converted = htole32(triangle_count);
	fwrite(&count_converted, sizeof(uint32_t), 1, stlfp);
		
	printf("saved %u triangles to %s\n", triangle_count,file);
	fclose(stlfp);
}
