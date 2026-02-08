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
#include <time.h>
#include <unistd.h>
// #include <sys/wait.h>

#include "c-csg.h"
#include "bb.h"
#include "stl.h"
// #include "ttf.h"
#include "noise.h"

struct CSG_NODE *csg_root;
struct CSG_NODE *csg_current_node;
#define csg_current_parent() csg_context_stack[csg_context_stack_len-1]
struct CSG_NODE **csg_context_stack;
int csg_context_stack_len;


//row major matrix
//translation on bottom

double matrix_identity_template[16] =
{
	1.0, 0, 0, 0,
	0, 1.0, 0, 0,
	0, 0, 1.0, 0,
	0, 0, 0.0, 1.0
};

double m_miner(double *m, int r0, int r1, int r2, int c0, int c1, int c2)
{
	return m[4 * r0 + c0] * (m[4 * r1 + c1] * m[4 * r2 + c2] - m[4 * r2 + c1] * m[4 * r1 + c2]) -
	       m[4 * r0 + c1] * (m[4 * r1 + c0] * m[4 * r2 + c2] - m[4 * r2 + c0] * m[4 * r1 + c2]) +
	       m[4 * r0 + c2] * (m[4 * r1 + c0] * m[4 * r2 + c1] - m[4 * r2 + c0] * m[4 * r1 + c1]);
}

void adjoint(double *m, double *adj_out)
{
	adj_out[ 0] =  m_miner(m, 1, 2, 3, 1, 2, 3);
	adj_out[ 1] = -m_miner(m, 0, 2, 3, 1, 2, 3);
	adj_out[ 2] =  m_miner(m, 0, 1, 3, 1, 2, 3);
	adj_out[ 3] = -m_miner(m, 0, 1, 2, 1, 2, 3);
	adj_out[ 4] = -m_miner(m, 1, 2, 3, 0, 2, 3);
	adj_out[ 5] =  m_miner(m, 0, 2, 3, 0, 2, 3);
	adj_out[ 6] = -m_miner(m, 0, 1, 3, 0, 2, 3);
	adj_out[ 7] =  m_miner(m, 0, 1, 2, 0, 2, 3);
	adj_out[ 8] =  m_miner(m, 1, 2, 3, 0, 1, 3);
	adj_out[ 9] = -m_miner(m, 0, 2, 3, 0, 1, 3);
	adj_out[10] =  m_miner(m, 0, 1, 3, 0, 1, 3);
	adj_out[11] = -m_miner(m, 0, 1, 2, 0, 1, 3);
	adj_out[12] = -m_miner(m, 1, 2, 3, 0, 1, 2);
	adj_out[13] =  m_miner(m, 0, 2, 3, 0, 1, 2);
	adj_out[14] = -m_miner(m, 0, 1, 3, 0, 1, 2);
	adj_out[15] =  m_miner(m, 0, 1, 2, 0, 1, 2);
}

double det(double *m)
{
	return m[0] * m_miner(m, 1, 2, 3, 1, 2, 3) -
	       m[1] * m_miner(m, 1, 2, 3, 0, 2, 3) +
	       m[2] * m_miner(m, 1, 2, 3, 0, 1, 3) -
	       m[3] * m_miner(m, 1, 2, 3, 0, 1, 2);
}


int m_invert(double *inv_out, double *m)
{
	adjoint(m, inv_out);
	double inv_det = 1.0f / det(m);

	for(int i = 0; i < 16; ++i)
		inv_out[i] = inv_out[i] * inv_det;

	return 0;
}



void m_mult (double *dst, double *a, double *b)
{
	dst[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
	dst[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
	dst[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
	dst[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];
	dst[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
	dst[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
	dst[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
	dst[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];
	dst[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
	dst[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
	dst[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
	dst[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];
	dst[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
	dst[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
	dst[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
	dst[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
}

void m_mult_overwrite (double *dst, double *a)
{
	double c[16];
	memcpy(c, dst, sizeof(double) * 16); //copy dst out of the way
	m_mult(dst, c, a);
}

void m_identity(double *dst)
{
	memcpy(dst, matrix_identity_template, 16 * sizeof(double));
}

/*
make a rotation matrix from an angle and axis
*/
void m_rotate(double *dst, double a, double *axis)
{
	double c = cos(a);
	double s = sin(a);
	double t = 1.0f - c;
	m_identity(dst);
	double u[3]; //don't alter inputs
	memcpy(u, axis, sizeof(double) * 3);
	v_normalize(u);
	dst[0] = c + u[0] * u[0] * t;
	dst[1] = u[0] * u[1] * t - u[2] * s;
	dst[2] = u[0] * u[2] * t + u[1] * s;
	dst[4] = u[0] * u[1] * t + u[2] * s;
	dst[5] = c + u[1] * u[1] * t;
	dst[6] = u[1] * u[2] * t - u[0] * s;
	dst[8] = u[2] * u[0] * t - u[1] * s;
	dst[9] = u[2] * u[1] * t + u[0] * s;
	dst[10] = c + u[2] * u[2] * t;
}





void m_scale(double *dst, double *v)
{
	m_identity(dst);
	dst[0] *= v[0];
	dst[5] *= v[1];
	dst[10] *= v[2];
}


void m_translate(double *dst, double *v)
{
	m_identity(dst);
	dst[12] = v[0];
	dst[13] = v[1];
	dst[14] = v[2];
}

inline void m_transform(double *dst, double *v, double *m)
{
	dst[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + m[12];
	dst[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + m[13];
	dst[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + m[14];
}

void m_zero(double *dst)
{
	memset(dst, 0, sizeof(double) * 16);
}

void m_compose(double *dst, char *pattern, ...)
{
	va_list ap;
	double m[16];
	m_identity(dst);
	va_start(ap, pattern);

	for(int i = 0; pattern[i]; i++)
		{
			switch(pattern[i])
				{
				case 't': //translate
					m_identity(m);
					m_translate(m, va_arg(ap, double *));
					m_mult_overwrite(dst, m);
					break;

				case 'r': //rotate
					m_identity(m);
					double a = va_arg(ap, double);
					double *b = va_arg(ap, double *);
					m_rotate(m, a, b);
					m_mult_overwrite(dst, m);
					break;

				case 's': //scale
					m_identity(m);
					m_scale(m, va_arg(ap, double *));
					m_mult_overwrite(dst, m);
					break;
				}
		}

	va_end(ap);
}

/* PRINTING FUNCTIONS */

const char *csg_node_type(struct CSG_NODE *n)
{
	if(!n)
		return "null";

	switch(n->object_type)
		{
		case NTYPE_ROOT:
			return "root";

		case NTYPE_SPHERE:
			return "sphere";

		case NTYPE_BOX:
			return "box";

		case NTYPE_PLANE:
			return "plane";

		case NTYPE_CONE:
			return "cone";

		case NTYPE_SWEEP:
			return "sweep";

		case NTYPE_SWEEP_QUAD:
			return "sweep_quad";

		case NTYPE_LATHE:
			return "lathe";

		case NTYPE_LATHE_QUAD:
			return "lathe_quad";

		case NTYPE_EXTRUDE:
			return "extrude";

		case NTYPE_INTERSECTION:
			return "intersection";

		case NTYPE_DIFFERENCE:
			return "difference";

		case NTYPE_UNION:
			return "union";

		case NTYPE_EXTRUDE_QUAD:
			return "extrude_quad";

		case NTYPE_INTERIOR:
			return "interior";

		default:
			return "unknown";
		}
}


void print_matrix(double *m, int indent)
{
	for(int i = 0; i < indent; i++)
		printf("    ");

	printf("[%4.2f,%4.2f,%4.2f,%4.2f]\n", m[0], m[1], m[2], m[3]);

	for(int i = 0; i < indent; i++)
		printf("    ");

	printf("[%4.2f,%4.2f,%4.2f,%4.2f]\n", m[4], m[5], m[6], m[7]);

	for(int i = 0; i < indent; i++)
		printf("    ");

	printf("[%4.2f,%4.2f,%4.2f,%4.2f]\n", m[8], m[9], m[10], m[11]);

	for(int i = 0; i < indent; i++)
		printf("    ");

	printf("[%4.2f,%4.2f,%4.2f,%4.2f]\n", m[12], m[13], m[14], m[15]);
}

void print_node(struct CSG_NODE *n, int depth)
{
	uint64_t p = (uint64_t)n;
	printf("\n");

	for(int i = 0; i < depth; i++)
		printf("    ");

	printf("instance: %" PRIx64 "\n", p);

	for(int i = 0; i < depth; i++)
		printf("    ");

	printf("type: %s\n", csg_node_type(n));

	for(int i = 0; i < depth; i++)
		printf("    ");

	printf("transform: \n");
	print_matrix(n->transform, depth);

	for(int i = 0; i < depth; i++)
		printf("    ");

	printf("bbox %s [ %0.2f, %0.2f, %0.2f : %0.2f, %0.2f, %0.2f ]\n", (n->bbox_valid ? "valid" : "invalid"), n->bbox[0], n->bbox[1], n->bbox[2], n->bbox[3], n->bbox[4], n->bbox[5]);
	printf("\n");
}



void print_tree(struct CSG_NODE *n, int depth)
{
	print_node(n, depth);

	for(int i = 0; i < n->children_len; i++)
		print_tree(n->children[i], depth + 1);
}



/* CSG_ FUNCTIONS */

void csg_identity(void)
{
	m_identity(csg_current_node->transform);
}

void csg_translate(double x, double y, double z)
{
	double p[3];
	double m[16];
	p[0] = x;
	p[1] = y;
	p[2] = z;
	m_translate(m, p);
	m_mult_overwrite(csg_current_node->transform, m);
}


void csg_scale(double x, double y, double z)
{
	double p[3];
	double m[16];
	p[0] = x;
	p[1] = y;
	p[2] = z;
	m_scale(m, p);
	m_mult_overwrite(csg_current_node->transform, m);
}



void csg_color(float r, float g, float b)
{
	csg_current_node->color[0] = r;
	csg_current_node->color[1] = g;
	csg_current_node->color[2] = b;
	csg_current_node->color_set = 1;
}

void csg_rotate_arbitrary(double a, double x, double y, double z)
{
	double p[3];
	p[0] = x;
	p[1] = y;
	p[2] = z;
	double m[16];
	m_rotate(m, a, p);
	m_mult_overwrite(csg_current_node->transform, m);
}

void csg_rotate(double x, double y, double z)
{
	double p[3];
	double m[16];

	if(x != 0.0)
		{
			p[0] = 1.0;
			p[1] = 0;
			p[2] = 0;
			m_rotate(m, x, p);
			m_mult_overwrite(csg_current_node->transform, m);
		}

	if(y != 0.0)
		{
			p[0] = 0;
			p[1] = 1.0;
			p[2] = 0;
			m_rotate(m, y, p);
			m_mult_overwrite(csg_current_node->transform, m);
		}

	if(z != 0.0)
		{
			p[0] = 0;
			p[1] = 0;
			p[2] = 1.0;
			m_rotate(m, z, p);
			m_mult_overwrite(csg_current_node->transform, m);
		}
}

void csg_print_tree(void)
{
	csg_refresh_transforms();
	print_tree(csg_root, 0);
}

struct CSG_NODE *csg_node(struct CSG_NODE *parent, int otype)
{
	struct CSG_NODE *n = malloc_or_die(sizeof(struct CSG_NODE));
	memset(n, 0, sizeof(struct CSG_NODE));
	n->object_type = otype;

	//append to parent's list of children
	if(parent)
		{
			parent->children = realloc(parent->children, (parent->children_len + 1) * sizeof(struct CSG_NODE *));
			parent->children[parent->children_len] = n;
			parent->children_len++;
			n->parent = parent;
		}

	m_identity(n->transform);
	return n;
}


void csg_push_context(struct CSG_NODE *n)
{
	csg_context_stack = realloc(csg_context_stack, (csg_context_stack_len + 1) * sizeof(csg_context_stack[0]));
	csg_context_stack[csg_context_stack_len++] = n;
}

struct CSG_NODE *csg_pop_context(void)
{
	return csg_context_stack[--csg_context_stack_len];
}


void csg_dup(void) //duplicate current node
{
	if(csg_current_node == csg_root)
		return;

	if(csg_current_node == csg_context_stack[csg_context_stack_len - 1])
		{
			printf("Attempt to cut from within context!  End the object first.\n");
			return;
		}

	csg_paste(clone_tree(csg_current_node));
}


struct CSG_NODE *csg_cut(void) //remove current node and children from main tree
{
	if(csg_current_node == csg_root)
		return NULL;

	if(csg_current_node == csg_context_stack[csg_context_stack_len - 1])
		{
			printf("Attempt to cut from within context!  End the object first.\n");
			return NULL;
		}

	struct CSG_NODE *parent = csg_current_node->parent;

	//remove from parent's children list
	struct CSG_NODE **nl = malloc_or_die( sizeof(struct CSG_NODE *) * parent->children_len - 1 );

	int j = 0;

	for(int i = 0; i < parent->children_len; i++)
		{
			if(parent->children[i] != csg_current_node)
				nl[j++] = parent->children[i];
		}

	parent->children_len = j;
	free(parent->children);
	parent->children = nl;
	//remove parent link
	struct CSG_NODE *dnode = csg_current_node;
	dnode->parent = NULL;
	//update current node ptr to be the old parent
	csg_current_node = parent;
	return dnode;
}

struct CSG_NODE *clone_tree(struct CSG_NODE *n)
{
	struct CSG_NODE *r;
	r = malloc_or_die(sizeof(struct CSG_NODE));
	memcpy(r, n, sizeof(struct CSG_NODE));

	//change out children list
	if(r->children)
		{
			r->children = malloc_or_die( sizeof(struct CSG_NODE *) * r->children_len);

			for(int i = 0; i < r->children_len; i++)
				{
					r->children[i] = clone_tree(n->children[i]);
					r->children[i]->parent = r; //set the parent of the children
				}
		}

	return r;
}


void csg_paste(struct CSG_NODE *n) //put a copy of node and children into the main tree
{
	if(n == NULL)
		return;

	//clone entire tree before inserting into current spot... but no need to clone precompute object data or user supplied vertex data
	struct CSG_NODE *c = clone_tree(n);
	struct CSG_NODE *parent = csg_current_parent();
	//add to parent's child list
	parent->children = realloc(parent->children, (parent->children_len + 1) * sizeof(struct CSG_NODE *));
	parent->children[parent->children_len] = c;
	parent->children_len++;
	//and don't forget to change object's parent link
	c->parent = parent;
	csg_current_node = c;
}


void csg_union(void)
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_UNION);
	csg_push_context(csg_current_node);
}


void csg_difference(void)
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_DIFFERENCE);
	csg_push_context(csg_current_node);
}


void csg_intersection(void)
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_INTERSECTION);
	csg_push_context(csg_current_node);
}


void csg_end(void)
{
	csg_current_node = csg_pop_context();
}


void csg_sphere(double r)
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_SPHERE);
	csg_current_node->object.sphere.r = r;
}

void csg_interior( int (*n)(double *v) )
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_INTERIOR);
	csg_current_node->object.interior.p = n;
	csg_push_context(csg_current_node);
}

void csg_exterior( void (*n)(double *v), double padding )
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_EXTERIOR);
	csg_current_node->object.exterior.p = n;
	csg_current_node->object.exterior.padding = padding;
	csg_push_context(csg_current_node);
}


void csg_box(void)
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_BOX);
}

void csg_plane(double x, double y, double z, double d)
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_PLANE);
	csg_current_node->object.plane.x = x;
	csg_current_node->object.plane.y = y;
	csg_current_node->object.plane.z = z;
	csg_current_node->object.plane.d = d;
}

void csg_cone(double r1, double r2, double d)
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_CONE);
	csg_current_node->object.cone.r1 = r1;
	csg_current_node->object.cone.r2 = r2;
	csg_current_node->object.cone.d = d;
}

void csg_extrude(int vlen, double *v, double d)
{
	if(!vlen)
		return;

	if(vlen%2)
	{
		printf("polygon array is not divisible by 2");
		return;
	}

	csg_current_node = csg_node(csg_current_parent(), NTYPE_EXTRUDE);
	struct EXTRUDE *o=&csg_current_node->object.extrude;

	o->d = d;

	if(! (v[0] != v[vlen-2] && v[1] != v[vlen-1]) ) //if last point doesn't equal first point, we add first point to end to close poly
	{
		o->v_len = vlen+2;
		o->v = malloc_or_die(sizeof(double)*(vlen+2));

		o->v[vlen]=v[0];
		o->v[vlen+1]=v[1];
	}
	else //just copy
	{
		o->v_len = vlen;
		o->v = malloc_or_die(sizeof(double)*vlen);
	}

	memcpy(o->v,v,sizeof(double)*vlen);

}


void csg_sweep(int vlen, double *v, double r)
{
	if(!vlen)
		return;

	if(vlen%2)
	{
		printf("polygon array is not divisible by 2");
		return;
	}

	csg_current_node = csg_node(csg_current_parent(), NTYPE_SWEEP);
	struct SWEEP *o=&csg_current_node->object.sweep;


	o->xmaxx = 0.0;

	if(! (v[0] != v[vlen-2] && v[1] != v[vlen-1]) ) //if last point doesn't equal first point, we add first point to end to close poly
	{
		o->v_len = vlen+2;
		o->v = malloc_or_die(sizeof(double)*(vlen+2));

		o->v[vlen]=v[0]+r;
		o->v[vlen+1]=v[1];
	}
	else //just copy
	{
		o->v_len = vlen;
		o->v = malloc_or_die(sizeof(double)*vlen);
	}

	for(int i = 0; i < vlen; i+=2) //precompute squared x distance to avoid sqrt
		{
			double x = v[i]+r;
			double y = v[i + 1];

			o->v[i] = x;
			o->v[i + 1] = y;

			if( x*x > o->xmaxx)
				o->xmaxx = x*x;
		}
}


void csg_sweep_quad(int vlen, double *v, double r)
{
	if(!vlen)
		return;

	csg_current_node = csg_node(csg_current_parent(), NTYPE_SWEEP_QUAD);
	struct SWEEP_QUAD *o=&csg_current_node->object.sweep_q;


	o->xmaxx = 0.0;

	o->v_len = vlen;
	o->v = malloc_or_die(sizeof(double)*vlen);

	for(int i = 0; i < vlen; i+=2) //precompute squared x distance to avoid sqrt
		{
			double x = v[i]+r;
			double y = v[i + 1];

			o->v[i] = x;
			o->v[i + 1] = y;

			if(x==QUAD_LINE)
				continue;

			if( x*x > o->xmaxx)
				o->xmaxx = x*x;
		}


}

void csg_lathe(int vlen, double *v, double d, double twists)
{
	if(!vlen)
		return;

	if(vlen%2)
	{
		printf("polygon array is not divisible by 2");
		return;
	}

	csg_current_node = csg_node(csg_current_parent(), NTYPE_LATHE);

	struct LATHE *o=&csg_current_node->object.lathe;

	o->xmaxx = 0.0;


	if(! (v[0] != v[vlen-2] && v[1] != v[vlen-1]) ) //if last point doesn't equal first point, we add first point to end to close poly
	{
		o->v_len = vlen+2;
		o->v = malloc_or_die(sizeof(double)*(vlen+2));

		o->v[vlen]=v[0];
		o->v[vlen+1]=v[1];
	}
	else //just copy
	{
		o->v_len = vlen;
		o->v = malloc_or_die(sizeof(double)*vlen);
	}

	for(int i = 0; i < vlen; i+=2)
		{
			double x = v[i];
			double y = v[i + 1];

			o->v[i] = x;
			o->v[i + 1] = y;

			x*=x;

			if( x > o->xmaxx)
				o->xmaxx = x;
		}

	o->d = d;
	o->twists = twists;

}


void csg_lathe_quad(int vlen, double *v, double d, double twists)
{
	if(!vlen)
		return;


	csg_current_node = csg_node(csg_current_parent(), NTYPE_LATHE_QUAD);

	struct LATHE_QUAD *o=&csg_current_node->object.lathe_q;

	o->xmaxx = 0.0;


	o->v_len = vlen;
	o->v = malloc_or_die(sizeof(double)*vlen);
	memcpy(o->v,v,sizeof(double)*vlen);

	for(int i = 0; i < vlen; i+=2)
		{
			double x = v[i];

			if(x==QUAD_LINE)
				continue;

			x*=x;

			if( x > o->xmaxx)
				o->xmaxx = x;
		}

	o->d = d;
	o->twists = twists;
}

void csg_extrude_quad(int qlen, double *quad, int d)
{
	csg_current_node = csg_node(csg_current_parent(), NTYPE_EXTRUDE_QUAD);
	csg_current_node->object.extrude_q.d = d;
	csg_current_node->object.extrude_q.quad = quad;
	csg_current_node->object.extrude_q.quad_len = qlen;
}




void csg_refresh_transforms(void)
{
	refresh_object_to_world(csg_root);
	refresh_extents(csg_root);
}

//returns based on bbox dims
void csg_dimensions(double *cx, double *cy, double *cz)
{
	refresh_object_to_world(csg_current_node);
	refresh_extents_relative(csg_current_node);
	*cx = csg_current_node->bbox[3] - csg_current_node->bbox[0] ;
	*cy = csg_current_node->bbox[4] - csg_current_node->bbox[1] ;
	*cz = csg_current_node->bbox[5] - csg_current_node->bbox[2] ;
}


void refresh_object_to_world(struct CSG_NODE *n)
{
	//from each level, apply transforms going to root to fill in object to world transform.
	memcpy(n->object_to_world, n->transform, sizeof(double) * 16);
	struct CSG_NODE *p = n->parent;

	while(p)
		{
			m_mult_overwrite(n->object_to_world, p->transform);
			p = p->parent;
		}

	m_invert(n->world_to_object, n->object_to_world);

	for(int i = 0; i < n->children_len; i++)
		refresh_object_to_world(n->children[i]);
}




/* VECTOR FUNCTIONS */

void v_cross(double *d, double *a, double *b)
{
	d[0] = a[1] * b[2] - a[2] * b[2];
	d[1] = a[2] * b[0] - a[0] * b[2];
	d[2] = a[0] * b[1] - a[1] * b[0];
}

void v_normalize(double *v)
{
	double d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
}

double v_distance(double *a, double *b)
{
	double sx = a[0] + b[0];
	double sy = a[1] + b[1];
	double sz = a[2] + b[2];
	return sqrt(sx * sx + sy * sy + sz * sz);
}





/*returns 1 if point inside polygon, last point of polygon must be equal to first point*/
int point_inside_poly(int len, double *p, double tx, double ty)
{
	int i, c = 0;

	for (i = 2; i < len; i += 2)
		{
			double vxi = p[i];
			double vyi = p[i + 1];
			double vxj = p[i - 2];
			double vyj = p[i - 1];

			if ( ((vyi < ty && vyj >= ty) ||   (vyj < ty && vyi >= ty)) &&  (vxi <= tx || vxj <= tx) )
				c ^= (vxi + (ty - vyi) / (vyj - vyi) * (vxj - vxi) < tx);
		}

	return c;
}



/* SPHERE */

int bb_sphere(double *bbox, struct SPHERE *n)
{
	//make bbox around sphere, apply local transform
	bbox[0] = -n->r * MNUDGE;
	bbox[1] = -n->r * MNUDGE;
	bbox[2] = -n->r * MNUDGE;
	bbox[3] = n->r * MNUDGE;
	bbox[4] = n->r * MNUDGE;
	bbox[5] = n->r * MNUDGE;
	return 1;
}


struct CSG_NODE *point_in_interior(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);

	if(n->object.interior.p(local))
		return n;

	return NULL;
}


struct CSG_NODE *point_in_sphere(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);

	//is distance between this point and the sphere less than the radius?


	if(local[0]*local[0] + local[1]*local[1] + local[2]*local[2] <= n->object.sphere.r * n->object.sphere.r)
		return n;

	return NULL;
}

/* BOX */

int bb_box(double *bbox, struct BOX *n)
{
	bbox[0] = -0.5 * MNUDGE;
	bbox[1] = -0.5 * MNUDGE;
	bbox[2] = -0.5 * MNUDGE;
	bbox[3] = 0.5 * MNUDGE;
	bbox[4] = 0.5 * MNUDGE;
	bbox[5] = 0.5 * MNUDGE;
	return 1;
}

struct CSG_NODE *point_in_box(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);

	if( local[0] >= -0.5 && local[0] <= 0.5
	        && local[1] >= -0.5 && local[1] <= 0.5
	        && local[2] >= -0.5 && local[2] <= 0.5
	  )
		return n;

	return NULL;
}


/* PLANE */

//plane is solid in direction of normal
struct CSG_NODE *point_in_plane(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);
	//if dot product is negative point is outside...
	double dp = n->object.plane.x * local[0]
	            + n->object.plane.y * local[1]
	            + n->object.plane.z * local[2]
	            - n->object.plane.d;

	if(dp >= 0.0)
		return n;

	return NULL;
}


/* CONE */

int bb_cone(double *bbox, struct CONE *n)
{
	double d = fabs(n->r1);
	double e = fabs(n->r2);

	if(e > d)
		d = e;

	bbox[0] = -d * MNUDGE;
	bbox[1] = -d * MNUDGE; //put lower bound slightly below surface
	bbox[2] = -d * MNUDGE;
	bbox[3] = d * MNUDGE;
	bbox[4] = n->d * MNUDGE;
	bbox[5] = d * MNUDGE;
	return 1;
}

struct CSG_NODE *point_in_cone(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);

	//cone is centered on zero for xz, but y goes up from 0,
	//see if the x,z point is within the circle radius at the given y

	//early out

	if(local[1] > n->object.cone.d || local[1] < 0)
		return NULL;

	double w2 = fabs(n->object.cone.r2);
	double w1 = fabs(n->object.cone.r1);

	if(w1 > w2)
		w2 = w1;

	if(local[0] > w2 || local[0] < -w2)
		return NULL;

	if(local[2] > w2 || local[2] < -w2)
		return NULL;

	//end early out
	double r_scale = ( n->object.cone.r2 - n->object.cone.r1 ) / n->object.cone.d;
	double r = n->object.cone.r1 + r_scale * local[1];

	if(local[0]*local[0] + local[2]*local[2] > r * r)
		return NULL;

	return n;
}


/* EXTRUDE */

int bb_extrude(double *bbox, struct EXTRUDE *n)
{
	double xmin, xmax, zmin, zmax;

	if(!n->v_len)
		return 0;

	xmin = n->v[0];
	xmax = n->v[0];
	zmin = n->v[1];
	zmax = n->v[1];

	for(int p = 0; p < n->v_len; p+=2)
		{
			double a, b;
			a = n->v[p];
			b = n->v[p+1];

			if(a < xmin)
				xmin = a;

			if(a > xmax)
				xmax = a;

			if(b < zmin)
				zmin = b;

			if(b > zmax)
				zmax = b;
		}

	bbox[0] = xmin - (xmax - xmin) * NUDGE;
	bbox[1] = - NUDGE;
	bbox[2] = zmin - (zmax - zmin) * NUDGE;
	bbox[3] = xmax + (xmax - xmin) * NUDGE;
	bbox[4] = n->d * MNUDGE;
	bbox[5] = zmax + (zmax - zmin) * NUDGE;
	return 1;
}


struct CSG_NODE *point_in_extrude(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);

	if(local[1] < 0.0 || local[1] > n->object.extrude.d) //can't be inside if y is out
		return NULL;


	if(point_inside_poly(n->object.extrude.v_len, n->object.extrude.v, local[0], local[2]))
		return n;

	return NULL;
}


/* SWEEP */

int bb_sweep(double *bbox, struct SWEEP *n)
{
	if(!n->v_len)
		return 0;

	double xmin = n->v[0];
	double xmax = n->v[0];
	double ymin = n->v[1];
	double ymax = n->v[1];

	for(int p = 0; p < n->v_len; p+=2)
		{
			double a,b;
			a = n->v[p+0];
			b = n->v[p+1];

			if(a < xmin)
				xmin = a;

			if(a > xmax)
				xmax = a;

			if(b < ymin)
				ymin = b;

			if(b > ymax)
				ymax = b;
		}

	bbox[0] = -(xmax);
	bbox[1] = ymin;
	bbox[2] = -(xmax);
	bbox[3] = xmax;
	bbox[4] = ymax;
	bbox[5] = xmax;
	double nudge = ymax * NUDGE;
	bbox[0] -= nudge;
	bbox[1] -= nudge;
	bbox[2] -= nudge;
	bbox[3] += nudge;
	bbox[4] += nudge;
	bbox[5] += nudge;
	return 1;
}



struct CSG_NODE *point_in_sweep(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);
	//drop 3d point into our cross section


	double xx =  local[0] * local[0] + local[2] * local[2]; //xz distance from origin ^ 2

	if(xx > n->object.sweep.xmaxx)
		return NULL;

	double y = local[1]; //y

	if(point_inside_poly(n->object.sweep.v_len, n->object.sweep.v, sqrt(xx), y))
		return n;

	return NULL;
}


/* SWEEP QUAD */

int bb_sweep_q(double *bbox, struct SWEEP_QUAD *n)
{
	if(!n->v_len)
		return 0;

	double xmin = n->v[0];
	double xmax = n->v[0];
	double ymin = n->v[1];
	double ymax = n->v[1];

	for(int p = 0; p < n->v_len; p+=2)
		{
			double a,b;
			a = n->v[p+0];
			b = n->v[p+1];

			if(a==QUAD_LINE)
				continue;

			if(a < xmin)
				xmin = a;

			if(a > xmax)
				xmax = a;

			if(b < ymin)
				ymin = b;

			if(b > ymax)
				ymax = b;
		}

	bbox[0] = -(xmax);
	bbox[1] = ymin;
	bbox[2] = -(xmax);
	bbox[3] = xmax;
	bbox[4] = ymax;
	bbox[5] = xmax;
	double nudge = ymax * NUDGE;
	bbox[0] -= nudge;
	bbox[1] -= nudge;
	bbox[2] -= nudge;
	bbox[3] += nudge;
	bbox[4] += nudge;
	bbox[5] += nudge;
	return 1;
}



struct CSG_NODE *point_in_sweep_q(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);
	//drop 3d point into our cross section


	double xx =  local[0] * local[0] + local[2] * local[2]; //xz distance from origin ^ 2

	if(xx > n->object.sweep_q.xmaxx)
		return NULL;

	double y = local[1]; //y

	if(point_inside_bezier_poly(n->object.sweep_q.v_len, n->object.sweep_q.v, sqrt(xx), y))
		return n;

	return NULL;
}


/* LATHE */

int bb_lathe(double *bbox, struct LATHE *n)
{
	if(!n->v_len)
		return 0;


	double xmin = n->v[0];
	double xmax = n->v[0];
	double ymin = n->v[1];
	double ymax = n->v[1];

	for(int p = 0; p < n->v_len; p+=2)
		{
			double a,b;
			a = n->v[p+0];
			b = n->v[p+1];

			if(a < xmin)
				xmin = a;

			if(a > xmax)
				xmax = a;

			if(b < ymin)
				ymin = b;

			if(b > ymax)
				ymax = b;
		}

	//extent x,z = is +- xmax
	//extent y = ymin , ymax
	bbox[0] = -xmax;
	bbox[1] = 0.0;
	bbox[2] = -xmax;
	bbox[3] = xmax;
	bbox[4] = n->d;
	bbox[5] = xmax;
	double nudge = n->d * NUDGE;
	bbox[0] -= nudge;
	bbox[1] -= nudge;
	bbox[2] -= nudge;
	bbox[3] += nudge;
	bbox[4] += nudge;
	bbox[5] += nudge;
	return 1;
}

struct CSG_NODE *point_in_lathe(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);

	if(local[1] < 0.0 || local[1] > n->object.lathe.d)
		return NULL;

	//poly stands up on y axis
	//drop 3d point into our cross section
	double xx =  local[0] * local[0] + local[2] * local[2]; //xz distance from origin ^ 2

	if(xx > n->object.lathe.xmaxx)
		return NULL;

	double y = local[1];
	//angle of point in respect to y axis
	double angle = atan2(local[0], local[2]);

	//twists can be negative for clockwise rotation
	if(n->object.lathe.twists < 0.0)
		{
			y = -fmod(  y / n->object.lathe.d * n->object.lathe.twists + angle / PI2 - 1.0, 1.0 );
		}
	else
		{
			y = fmod(  y / n->object.lathe.d * n->object.lathe.twists + angle / PI2 + 1.0, 1.0 );
		}

	if(point_inside_poly(n->object.lathe.v_len, n->object.lathe.v, sqrt(xx), y))
		{
			return n;
		}

	return NULL;
}


/* LATHE QUAD */


int bb_lathe_q(double *bbox, struct LATHE_QUAD *n)
{
	if(!n->v_len)
		return 0;


	double xmin = n->v[0];
	double xmax = n->v[0];
	double ymin = n->v[1];
	double ymax = n->v[1];

	for(int p = 0; p < n->v_len; p+=2)
		{
			double a,b;
			a = n->v[p+0];
			b = n->v[p+1];

			if(a==QUAD_LINE)
				continue;

			if(a < xmin)
				xmin = a;

			if(a > xmax)
				xmax = a;

			if(b < ymin)
				ymin = b;

			if(b > ymax)
				ymax = b;
		}

	//extent x,z = is +- xmax
	//extent y = ymin , ymax
	bbox[0] = -xmax;
	bbox[1] = 0.0;
	bbox[2] = -xmax;
	bbox[3] = xmax;
	bbox[4] = n->d;
	bbox[5] = xmax;
	double nudge = n->d * NUDGE;
	bbox[0] -= nudge;
	bbox[1] -= nudge;
	bbox[2] -= nudge;
	bbox[3] += nudge;
	bbox[4] += nudge;
	bbox[5] += nudge;
	return 1;
}

struct CSG_NODE *point_in_lathe_q(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);

	struct LATHE_QUAD *o = &n->object.lathe_q;

	if(local[1] < 0.0 || local[1] > o->d)
		return NULL;

	//poly stands up on y axis
	//drop 3d point into our cross section
	double xx =  local[0] * local[0] + local[2] * local[2]; //xz distance from origin ^ 2

	if(xx > o->xmaxx)
		return NULL;

	double y = local[1];
	//angle of point in respect to y axis
	double angle = atan2(local[0], local[2]);

	//twists can be negative for clockwise rotation
	if(o->twists < 0.0)
		{
			y = -fmod(  y / o->d * o->twists + angle / PI2 - 1.0, 1.0 );
		}
	else
		{
			y = fmod(  y / o->d * o->twists + angle / PI2 + 1.0, 1.0 );
		}

	if(point_inside_bezier_poly(o->v_len, o->v, sqrt(xx), y))
		{
			return n;
		}

	return NULL;
}


/* QUADRATIC BEZIER */

int bb_extrude_q(double *bbox, struct EXTRUDE_QUAD *p)
{
	bbox[1] = 0.0 - NUDGE;
	bbox[4] = p->d * MNUDGE;
	double minx = DBL_MAX;
	double miny = DBL_MAX;
	double maxx = -DBL_MAX;
	double maxy = -DBL_MAX;

	for(int i = 0; i < p->quad_len; i += 2)
		{
			double t = p->quad[i];

			if(t == QUAD_LINE)
				continue; //don't include control points when they are set to coded values

			if( t < minx )
				minx = t;

			if( t > maxx )
				maxx = t;

			t = p->quad[i + 1];

			if( t < miny )
				miny = t;

			if( t > maxy )
				maxy = t;
		}

	minx -= NUDGE;
	miny -= NUDGE;
	maxx *= MNUDGE;
	maxy *= MNUDGE;
	bbox[0] = minx;
	bbox[2] = miny;
	bbox[3] = maxx;
	bbox[5] = maxy;
	return 1;
}

inline int point_inside_triangle(double ax, double ay, double bx, double by, double cx, double cy, double sx, double sy)
{
	double as_x = sx - ax;
	double as_y = sy - ay;
	int s_ab = (bx - ax) * as_y - (by - ay) * as_x > 0.0;

	if( ( (cx - ax)*as_y - (cy - ay)*as_x > 0.0) == s_ab)
		return 0;

	if( ((cx - bx) * (sy - by) - (cy - by) * (sx - bx) > 0.0) != s_ab)
		return 0;

	return 1;
}



double inline dot2d(double x1, double y1, double x2, double y2)
{
	return (x1 * x2) + (y1 * y2);
}

//check if point is inside bezier
int point_inside_quad(double ax, double ay, double bx, double by, double cx, double cy, double px, double py)
{
	// Compute vectors
	double v0x = cx - ax;
	double v0y = cy - ay;
	double v1x = bx - ax;
	double v1y = by - ay;
	double v2x = px - ax;
	double v2y = py - ay;
	// Compute dot products
	double dot00 = dot2d(v0x, v0y, v0x, v0y);
	double dot01 = dot2d(v0x, v0y, v1x, v1y);
	double dot02 = dot2d(v0x, v0y, v2x, v2y);
	double dot11 = dot2d(v1x, v1y, v1x, v1y);
	double dot12 = dot2d(v1x, v1y, v2x, v2y);
	// Compute barycentric coordinates
	double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
	// use the blinn and loop method
	double w = (1.0 - u - v);
	double tcx = 0.5 * v + w;
	double tcy = w;

	if(tcx * tcx - tcy < 0.0)
		return 1;

	return 0;
}

int point_inside_poly_q(double *p, int len, double tx, double ty)
{
	int i, c = 0;

	//hop on control points
	for (i = 2; i < len; i += 4)
		{
			double vxi = p[i + 2];
			double vyi = p[i + 3];
			double vxj = p[i - 2];
			double vyj = p[i - 1];

			if ( ((vyi < ty && vyj >= ty) ||   (vyj < ty && vyi >= ty)) &&  (vxi <= tx || vxj <= tx) )
				c ^= (vxi + (ty - vyi) / (vyj - vyi) * (vxj - vxi) < tx);
		}

	return c;
}



inline int point_inside_bezier_poly(int len, double *p, double tx, double ty)
{
	/*
	first check as regular poly, recording in/out

	check as bezier, if not in shaded area but inside regular check, return 1

	if inside a shaded area,
		if midpoint offset to the control point side is inside of regular polygon
			then the curve is subtracted from the regular poly and return 0
		else, curve is added to poly
		   return 1

	return 0;
	*/
	int c = point_inside_poly_q(p, len, tx, ty);

	//c contains in/out check for poly, now cycle through beziers and see if one has a hit

	for (int i = 2; i < len; i += 4) // hop along control points
		{
			double cx = p[i];

			if(cx == QUAD_LINE)
				continue; //straight lines are of no interest here

			double cy = p[i + 1];
			double x1 = p[i - 2];
			double y1 = p[i - 1];
			double x2 = p[i + 2];
			double y2 = p[i + 3];

			if(!point_inside_triangle(x1, y1, cx, cy, x2, y2, tx, ty))
				continue;

			if(point_inside_quad(x1, y1, cx, cy, x2, y2, tx, ty))
				{
					//check if midpoint + offset is inside or outside poly
					double ox, oy;
					ox = x1 + ((x2 - x1) / 2.0);
					oy = y1 + ((y2 - y1) / 2.0);
					//add a fraction of the control point to offset
					ox += (cx - ox) * 0.000001;
					oy += (cy - oy) * 0.000001;

					if(point_inside_poly_q(p, len, ox, oy))
						return 0;
					else
						return 1;
				}
		}

	if(c)
		return 1;

	return 0;
}

struct CSG_NODE *point_in_extrude_q(struct CSG_NODE *n, double *v)
{
	double local[3];
	m_transform(local, v, n->world_to_object);

	if(local[1] < 0.0 || local[1] > n->object.extrude_q.d)
		return NULL;

	if(point_inside_bezier_poly(n->object.extrude_q.quad_len, n->object.extrude_q.quad, local[0], local[2]))
		return n;

	return NULL;
}









void refresh_extents_relative(struct CSG_NODE *n)
{
	//collect and combine extents of children, bboxes will be in world coords from leaves no need to transform at each level.
	double bbox[6];
	n->bbox_valid = 0; //default to invalid

	switch(n->object_type)
		{
		case NTYPE_DIFFERENCE: //extent is the first child only
			if(!n->children_len)
				{
					n->bbox_valid = 1; //no children but empty bbox is valid
					return;
				}

			refresh_extents_relative(n->children[0]);

			if( n->children[0]->bbox_valid )
				{
					memcpy(n->bbox, n->children[0]->bbox, sizeof(double) * 6);
					n->bbox_valid = 1;
				}

			//compute the others but don't use at this level
			for(int i = 1; i < n->children_len; i++)
				refresh_extents_relative(n->children[i]);

			break;

		case NTYPE_ROOT:
		case NTYPE_UNION:
		case NTYPE_INTERIOR:

			//all children must fit inside extent
			if(!n->children_len)
				{
					n->bbox_valid = 1; //no children but empty bbox is valid
					return;
				}

			for(int i = 0; i < n->children_len; i++)
				{
					refresh_extents_relative(n->children[i]);

					if(!n->children[i]->bbox_valid)
						return;
				}

			memcpy(n->bbox, n->children[0]->bbox, sizeof(double) * 6);

			for(int i = 1; i < n->children_len; i++)
				{
					bb_add(n->bbox, n->children[i]->bbox);
				}

			n->bbox_valid = 1;
			break;

		case NTYPE_EXTERIOR: //expand extents of children as user specified to make room for noise

			//all children must fit inside extent
			if(!n->children_len)
				{
					n->bbox_valid = 1; //no children but empty bbox is valid
					return;
				}

			for(int i = 0; i < n->children_len; i++)
				{
					refresh_extents_relative(n->children[i]);

					if(!n->children[i]->bbox_valid)
						return;
				}

			memcpy(n->bbox, n->children[0]->bbox, sizeof(double) * 6);

			for(int i = 1; i < n->children_len; i++)
				{
					bb_add(n->bbox, n->children[i]->bbox);
				}

			n->bbox[0]-=n->object.exterior.padding;
			n->bbox[3]+=n->object.exterior.padding;
			n->bbox[1]-=n->object.exterior.padding;
			n->bbox[4]+=n->object.exterior.padding;
			n->bbox[2]-=n->object.exterior.padding;
			n->bbox[5]+=n->object.exterior.padding;

			n->bbox_valid = 1;
			break;

		case NTYPE_INTERSECTION: //TODO intersecting bbox becomes bbox
		{
			if(!n->children_len)
				{
					n->bbox_valid = 1; //no children but empty bbox is valid
					return;
				}

			for(int i = 0; i < n->children_len; i++)
				{
					refresh_extents_relative(n->children[i]);

					if(!n->children[i]->bbox_valid)
						return;
				}

			memcpy(n->bbox, n->children[0]->bbox, sizeof(double) * 6);

			for(int i = 1; i < n->children_len; i++)
				{
					bb_add(n->bbox, n->children[i]->bbox);
				}

			n->bbox_valid = 1;
		}
		break;


		case NTYPE_SPHERE:
			if(!bb_sphere(bbox, &n->object.sphere))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		case NTYPE_BOX:
			if(!bb_box(bbox, &n->object.box))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		case NTYPE_PLANE:
			//can't constrain
			break;

		case NTYPE_CONE:
			if(!bb_cone(bbox, &n->object.cone))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		case NTYPE_SWEEP:
			if(!bb_sweep(bbox, &n->object.sweep))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		case NTYPE_SWEEP_QUAD:
			if(!bb_sweep_q(bbox, &n->object.sweep_q))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		case NTYPE_LATHE:
			if(!bb_lathe(bbox, &n->object.lathe))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		case NTYPE_LATHE_QUAD:
			if(!bb_lathe_q(bbox, &n->object.lathe_q))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		case NTYPE_EXTRUDE:
			if(!bb_extrude(bbox, &n->object.extrude))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		case NTYPE_EXTRUDE_QUAD:
			if(!bb_extrude_q(bbox, &n->object.extrude_q))
				return;

			bb_transform_bbox(n->bbox, bbox, n->object_to_world);
			n->bbox_valid = 1;
			break;

		default:
			break;
		}

	return;
}





void refresh_extents(struct CSG_NODE *n)
{
	refresh_extents_relative(n);
	//bb_stats(csg_root,0);
	//print_tree(n,0);
	//bb_stats(csg_root,0);
}



inline int csg_point_in_solid(double *v)
{
	return (csg_point_hits(csg_root, v) ? 1 : 0);
}

struct CSG_NODE *csg_point_hits(struct CSG_NODE *n, double *v) //pass world coordinates down to objects, see what sticks
{
	if(n->bbox_valid && (! bb_point_inside(n->bbox,v)) ) //inf objects don't have valid bbox, but we do hit test them
		return NULL;

	struct CSG_NODE *r=NULL;

	switch(n->object_type)
		{
		case NTYPE_INTERSECTION: //all children must be positive to return this node

			//printf("intersection\n");
			if(!n->children_len)
				break;

			r = n;

			for(int i = 0; i < n->children_len; i++)
				{
					if(!csg_point_hits(n->children[i], v))
						{
							r = NULL;
							break;
						}
				}

			break;

		case NTYPE_DIFFERENCE: //must hit first child, and not any others

			//printf("difference\n");
			if(!n->children_len)
				break;

			struct CSG_NODE *t = csg_point_hits(n->children[0], v);

			if(!t)
				break;

			r = n;

			for(int i = 1; i < n->children_len; i++)
				{
					if(csg_point_hits(n->children[i], v))
						{
							r = NULL;
							break;
						}
				}

			break;

		case NTYPE_UNION: //if any child is positive, then we are positive

			//printf("union\n");
			if(!n->children_len)
				break;

			for(int i = 0; i < n->children_len; i++)
				{
					if(csg_point_hits(n->children[i], v))
						{
							r = n;
							break;
						}
				}

			break;

		case NTYPE_ROOT:

			//printf("root\n");
			for(int i = 0; i < n->children_len; i++)
				{
					r = csg_point_hits(n->children[i], v);

					if(r)
						break;
				}

			break;

		case NTYPE_INTERIOR:
			if(!n->children_len)
				break;

			for(int i = 0; i < n->children_len; i++)
				{
					if(csg_point_hits(n->children[i], v) && point_in_interior(n, v))
						{
							r = n;
							break;
						}
				}
			break;

		case NTYPE_EXTERIOR:
			if(!n->children_len)			
				break;
			

			//go to local space, modify point, then go back to world space before sending it to children
			{
				double pet[3];
				m_transform(pet, v, n->world_to_object);
				n->object.exterior.p(pet);
				double ipet[3];
				m_transform(ipet, pet, n->object_to_world);
			
				for(int i = 0; i < n->children_len; i++)
					{
						if(csg_point_hits(n->children[i], ipet))
							{
								r = n;
								break;
							}
					}
			}
			break;

		case NTYPE_SPHERE:
			r = point_in_sphere(n, v);
			break;

		case NTYPE_BOX:
			r = point_in_box(n, v);
			break;

		case NTYPE_PLANE:
			r = point_in_plane(n, v);
			break;

		case NTYPE_CONE:
			r = point_in_cone(n, v);
			break;

		case NTYPE_SWEEP:
			r = point_in_sweep(n, v);
			break;

		case NTYPE_LATHE:
			r = point_in_lathe(n, v);
			break;

		case NTYPE_SWEEP_QUAD:
			r = point_in_sweep_q(n, v);
			break;

		case NTYPE_LATHE_QUAD:
			r = point_in_lathe_q(n, v);
			break;

		case NTYPE_EXTRUDE:
			r = point_in_extrude(n, v);
			break;

		case NTYPE_EXTRUDE_QUAD:
			r = point_in_extrude_q(n, v);
			break;

		default:
			r = NULL;
			break;
		}

	return r;
}





void csg_color_from_node(uint8_t *rgb, struct CSG_NODE *n)
{
	if(!n)
		{
			rgb[0] = 0;
			rgb[1] = 0;
			rgb[2] = 0;
		}
	else
		{
			float r, g, b;
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			//if this node, or any parent nodes have a color set then use it, parent overwrites child
			struct CSG_NODE *np = n;

			while(np)
				{
					if(np->color_set)
						{
							r = np->color[0];
							g = np->color[1];
							b = np->color[2];
						}

					np = np->parent;
				}

			rgb[0] = (r * 255.0f);
			rgb[1] = (g * 255.0f);
			rgb[2] = (b * 255.0f);
		}
}


void csg_save_slice(char *file, double x1, double z1, double x2, double z2, double y, int image_width, int image_height)
{
	double xscale = (x2 - x1) / ((double)image_width);
	double zscale = (z2 - z1) / ((double)image_height);
	FILE *fp;
	fp = fopen(file, "wb");

	if(!fp)
		{
			printf("could not open %s for writing\n", file);
			return;
		}

	if(fprintf(fp, "P6\n#csgfun\n%u %u\n255\n", image_width, image_height) < 0)
		{
			printf("error writing header to file: %s", file);
			fclose(fp);
			return;
		}

	csg_refresh_transforms();

	for(int py = 0; py < image_height; py++)
		for(int px = 0; px < image_width; px++)
			{
				double u_px = x1 + px * xscale;
				double u_pz = z1 + py * zscale;
				double xyz[3];
				xyz[0] = u_px;
				xyz[1] = y;
				xyz[2] = u_pz;
				struct CSG_NODE *n = csg_point_hits(csg_root, xyz);
				uint8_t pixel[3];
				csg_color_from_node(pixel, n);

				if(fwrite(pixel, 1, 3, fp) < 3)
					{
						printf("error writing body to file: %s", file);
						fclose(fp);
						return;
					}
			}

	fclose(fp);
}


void csg_flip_world_axis(int x, int y, int z)
{
	if(x)
		csg_root->transform[0] *= -1.0; //mirror x axis

	if(y)
		csg_root->transform[5] *= -1.0; //mirror y axis

	if(z)
		csg_root->transform[10] *= -1.0; //mirror z axis
}

void csg_world_z_up(void)
{
	double m[16];
	m_zero(m);
	m[0]=1.0;
	m[6]=1.0;
	m[9]=1.0;
	m[15]=1.0;

	m_mult_overwrite(csg_root->transform,m);
}




void *malloc_or_die(size_t s)
{
	void *p = malloc(s);
	if(!p)
	{
		fprintf(stderr,"malloc failed\n");
		exit(EXIT_FAILURE);
	}
	return p;
}

/*
struct{MODEL*list;size_t length,allocated;} models;
__attribute__((constructor(101),used)) static void MODEL_init(void)
{
	models.list=malloc_or_die(sizeof(MODEL)*(models.allocated=20));
	if(!models.list)
	{
		fprintf(stderr,"malloc failed");
		exit(-1);
	}

	models.length=0;
}

__attribute__((destructor(101),used)) static void model_destroy(void)
{
	free(models.list);
}

void model_add(const char*name, void(*entry)(int argc, char *argv[]))
{
	MODEL block={name,entry};
	if(models.length>=models.allocated)
	{
		models.list=realloc(models.list,sizeof(MODEL)*(models.allocated+=20));
		if(!models.list)
		{			
			fprintf(stderr,"realloc failed");
			exit(EXIT_FAILURE);
		}
	}
	models.list[models.length++]=block;
}
*/

struct osn_context *osctx;

/*
void setup_and_run(void(*entry)(int argc, char* argv[]), int argc, char *argv[])
{
	time_t start = time(NULL);
	csg_root = csg_node(NULL, NTYPE_ROOT);
	csg_current_node = csg_root;
	csg_push_context(csg_current_node); //everything is a collection under the root

	open_simplex_noise(47279, &osctx);

	entry(argc, argv);

	printf("elapsed seconds: %u\n", (unsigned int) (time(NULL) - start) );
}
*/

/*
int main(int argc, char *argv[])
{
	if(argc<2)
	{
		fprintf(stderr,"c-csg model_name ...\n");
		for (size_t i = 0; i<models.length; i++)
			fprintf(stderr,"    %s\n",models.list[i].name);
		fprintf(stderr,"    all\n");
		exit(EXIT_FAILURE);
	}
	
	int run_all = strcmp(argv[1],"all")==0;
	
	for (size_t i = 0; i<models.length; i++)
	{
		if ((!run_all) && strcmp(models.list[i].name, argv[1]) != 0)
			continue;

		//execute in another process to keep parent clean
		if(fork()==0)
		{
			setup_and_run(models.list[i].entry,argc,argv);
			exit(EXIT_SUCCESS);
		}
		wait(NULL);		
		
		if(!run_all)
			exit(EXIT_SUCCESS);
	}
	
	if(run_all)
		exit(EXIT_SUCCESS);
	
	fprintf(stderr,"model not found\n");
	exit(EXIT_FAILURE);	
}
*/

void model(int argc, char *argv[]);

// para usarlo como se descarga de la WEB, con EXE independientes
#if 0
int main(int argc, char *argv[]) { 
	time_t start = time(NULL); 
	csg_root = csg_node(NULL, NTYPE_ROOT); 
	csg_current_node = csg_root; 
	csg_push_context(csg_current_node);  
	//open_simplex_noise(47279, &osctx); 
	
	model(argc,argv);
	
	printf("elapsed seconds: %u\n", (unsigned int) (time(NULL) - start) ); 
} 
#endif

// para usarlo como DLL desde FB
#if 1
struct CSG_NODE *csg_init(int *osctx_return)
{ 
	csg_root = csg_node(NULL, NTYPE_ROOT); 
	csg_current_node = csg_root; 
	csg_push_context(csg_current_node);  
	open_simplex_noise(47279, &osctx); 
	if(osctx_return!=NULL) *osctx_return=(int)osctx;
	return csg_current_node;
} 
#endif

