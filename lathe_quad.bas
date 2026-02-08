#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

Dim AS double q(...)={3.0,1.0, DBL_MAX,DBL_MAX, 4.0,1.0, 5.0,0.5, _
				4.0,0.0, DBL_MAX,DBL_MAX, 3.0,0.0, DBL_MAX,DBL_MAX, 3.0,1.0}
csg_difference()
	csg_lathe_quad(lon(q),@q(0),8,3)
csg_end()

' ver resultado de calculos
csg_print_tree()

csg_save_stl("lathe_quad.stl", 0.25, 1)

print "Fin..."
sleep