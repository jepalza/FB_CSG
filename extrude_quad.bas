#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

dim as double q(...)={0.9,0.0, -2,2.2, 0.9,4.5, DBL_MAX,DBL_MAX, _ 
							1.2,4.5, 3.0,2.0, 2.6,1.4, DBL_MAX,DBL_MAX, _
							1.2,0.0, DBL_MAX,DBL_MAX, 0.9,0.0}
csg_difference()
	csg_extrude_quad(lon(q),@q(0),4)
csg_end()

csg_print_tree()

csg_save_stl("extrude_quad.stl", 0.1, 5)

sleep