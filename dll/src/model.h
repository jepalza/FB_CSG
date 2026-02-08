#include "c-csg.h"
#include "noise.h"

#define end csg_end
#define union csg_union
#define intersection csg_intersection
#define difference csg_difference

#define cube(a) csg_box(); csg_scale(a,a,a);
#define cylinder(a,b) csg_cone((a)/2.0,(a)/2.0,b); translate(0,-(b)/2.0,0);
#define pipe(a,b,c) csg_difference(); csg_cone((a)/2.0,(a)/2.0,c); csg_cone((b)/2.0,(b)/2.0,(c)*1.001); csg_translate(0,-(c)*0.0005,0); csg_end(); csg_translate(0,-(c)/2.0,0);
#define box csg_box
#define sphere csg_sphere
#define plane csg_plane
#define cone csg_cone
#define sweep csg_sweep
#define sweep_quad csg_sweep_quad
#define extrude csg_extrude
#define extrude_quad csg_extrude_quad
#define lathe csg_lathe
#define lathe_quad csg_lathe_quad
#define text csg_text
#define interior csg_interior
#define exterior csg_exterior

#define dimensions csg_dimensions
#define flip_world_axis csg_flip_world_axis
#define world_z_up csg_world_z_up

#define translate csg_translate
#define scale csg_scale
#define rotate csg_rotate
#define rotate_arbitrary csg_rotate_arbitrary
#define save_stl csg_save_stl
#define cut() csg_cut()
#define paste(x) csg_paste(x)
#define dup() csg_dup()

#define mirror_x() scale(-1,1,1)
#define mirror_y() scale(1,-1,1)
#define mirror_z() scale(1,1,-1)

#define identity() csg_identity()

#define print_tree csg_print_tree


#define len(x) (signed) (sizeof(x)/sizeof(x[0]))
typedef struct CSG_NODE *OBJECT;

extern struct osn_context *osctx;


extern struct CSG_NODE *csg_current_node;
extern struct CSG_NODE *csg_root;


#define model(a) void model(int argc, char *argv[])