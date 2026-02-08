#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <float.h>
#include <time.h>


#define PI 3.14159265358979323846
#define PI2 (PI*2.0)


struct SPHERE
{
	double r;
};

struct BOX
{
	//1x1x1 centered
};

struct PLANE
{
	double x, y, z, d;
};

struct CONE
{
	double r1, r2, d;
};

struct SWEEP
{
	double *v; //this is a copy of original points, the last point guaranteed to equal the first
	int v_len;
	double xmaxx; //for early out

};

struct LATHE
{
	int v_len;

	double *v; //this is a copy of original points,  the last point guaranteed to equal the first
	double xmaxx; //max of x coords in vs for early out

	double d;
	double twists;
};

struct EXTRUDE
{
	double *v;
	int v_len;
	double d;
};


struct EXTRUDE_QUAD
{
#define QUAD_LINE DBL_MAX

	double *quad; //x,y,cx,cy,x,y,cx,cy,x,y,.., control points set to QUAD_LINE indicates a straight line
	int quad_len; //actual double count
	double d;
};


struct SWEEP_QUAD
{
	double *v; //this is a copy of original points, the last point guaranteed to equal the first
	int v_len;
	double xmaxx; //for early out
};

struct LATHE_QUAD
{
	int v_len;

	double *v; //this is a copy of original points,  the last point guaranteed to equal the first
	double xmaxx; //max of x coords in vs for early out

	double d;
	double twists;
};




struct INTERIOR
{
	int (*p)(double *v);
};

struct EXTERIOR
{
	void (*p)(double *v);
	double padding;
};


union CSG_OBJECT
{
	struct SPHERE sphere;
	struct BOX box;
	struct PLANE plane;
	struct CONE cone;
	struct SWEEP sweep;
	struct SWEEP_QUAD sweep_q;
	struct LATHE lathe;
	struct LATHE_QUAD lathe_q;
	struct EXTRUDE extrude;
	struct EXTRUDE_QUAD extrude_q;
	struct INTERIOR interior;
	struct EXTERIOR exterior;

};

struct CSG_NODE
{
	struct CSG_NODE *parent;
	struct CSG_NODE **children;
	int children_len;

	double transform[16];

	double world_to_object[16];
	double object_to_world[16];

	double bbox[6]; //xmin, ymin, zmin, xmax, ymax, zmax in world coords, no rotation, if isnan(xmin) bbox is not used


	float color[3];
	uint8_t color_set;

	uint8_t bbox_valid;

	uint8_t object_type;
	union CSG_OBJECT object;

	void *user_tag;
};




#define NTYPE_ROOT 0
#define NTYPE_SPHERE 1
#define NTYPE_BOX 2
#define NTYPE_PLANE 3
#define NTYPE_CONE 4
#define NTYPE_SWEEP 5
#define NTYPE_EXTRUDE 6
#define NTYPE_INTERSECTION 7
#define NTYPE_DIFFERENCE 8
#define NTYPE_UNION 9
#define NTYPE_LATHE 10
#define NTYPE_EXTRUDE_QUAD 11
#define NTYPE_INTERIOR 12
#define NTYPE_EXTERIOR 13
#define NTYPE_SWEEP_QUAD 14
#define NTYPE_LATHE_QUAD 15

#define MNUDGE 1.001 /* multiplier nudge */
#define NUDGE 0.001 /* used as multiplier only, never addition */


//internal functions




struct CSG_NODE *csg_node(struct CSG_NODE *parent, int type);
struct CSG_NODE *csg_get_last_sibling(struct CSG_NODE *n);
struct CSG_NODE *csg_point_inside(double *xyz);  //returns pointer to one node that point is inside of
void csg_push_context(struct CSG_NODE *n);
struct CSG_NODE *csg_pop_context(void);
void refresh_node_matrix(struct CSG_NODE *n);
void refresh_extents(struct CSG_NODE *n);
void bbox_stats(struct CSG_NODE *n, int depth);

struct CSG_NODE *csg_point_hits(struct CSG_NODE *n, double *v);
int csg_point_in_solid(double *v);
void csg_refresh_transforms(void);

void refresh_object_to_world(struct CSG_NODE *n);
void refresh_extents_relative(struct CSG_NODE *n);

struct CSG_NODE *csg_point_to_node(double x, double y, double z); //determines what object is at specified point, returns first found.

int point_inside_poly(int len, double *v, double x, double y);
int point_inside_bezier_poly(int len, double *p, double tx, double ty);

//matrix related functions
void m_mult (double *dst, double *a, double *b);
void m_mult_overwrite (double *dst, double *a);
void m_identity(double *dst);
void m_zero(double *dst);
void m_rotate(double *dst, double a, double *axis);

void m_scale(double *dst, double *v);
void m_translate(double *dst, double *v);

void m_transform(double *dst, double *v, double *m);

void m_compose(double *dst, char *pattern, ...);

void m_rotate_x(double *dst, double a);
void m_rotate_y(double *dst, double a);
void m_rotate_z(double *dst, double a);

//vector functions
void v_cross(double *d, double *a, double *b);
void v_normalize(double *v);
double v_distance(double *a, double *b);

//user script functions

void csg_sphere(double r);
void csg_plane(double x, double y, double z, double r);
void csg_box(void);
void csg_interior(int (*p)(double *v));
void csg_exterior(void (*p)(double *v), double padding);
void csg_cone(double r1, double r2, double d);
void csg_color(float r, float g, float b);

void csg_scale(double x, double y, double z);
void csg_translate(double x, double y, double z);
void csg_identity(void); //reset matrix for current object
void csg_rotate(double x, double y, double z);
void csg_rotate_arbitrary(double a, double x, double y, double z);
void csg_extrude(int nvert, double *v, double d);
void csg_sweep(int poly_len, double *poly, double r);
void csg_lathe(int poly_len, double *poly, double d, double twists);
void csg_sweep_quad(int poly_len, double *poly, double r);
void csg_lathe_quad(int poly_len, double *poly, double d, double twists);
void csg_text(char *text, char *font, double d);
void csg_flip_world_axis(int x, int y, int z);
void csg_world_z_up(void);
void csg_extrude_quad(int qlen, double *quad, int d);

void csg_print_tree(void);//print object tree to console

void csg_union(void); //the object that can have children auto push the context, so objects are then created as children until the context is popped
void csg_difference(void);
void csg_intersection(void);
void csg_end(void);

void csg_dimensions(double *cx, double *cy, double *cz); //returns based on bbox dims

struct CSG_NODE *csg_cut(void); //remove current node and children from main tree
void csg_paste(struct CSG_NODE *n); //put a copy of node and children into the main tree
void csg_dup(void); //duplicate current node
struct CSG_NODE *clone_tree(struct CSG_NODE *n);

void csg_save_slice(char *file, double x1, double z1, double x2, double z2, double y, int image_width, int image_height);
void csg_save_stl(char *file, double resolution, int threads);

void *malloc_or_die(size_t s);


typedef struct
{
	const char *name;	
	void(*entry)(int argc, char*argv[]);	
} MODEL;
void model_add(const char*name,void(*entry)(int argc, char*argv[]));

/*#define model(name) void m_ ## name(int argc, char *argv[]); \
__attribute__((constructor)) void __h_## name(void){model_add(#name,m_ ## name);}\
void m_ ## name(int argc, char *argv[])
*/