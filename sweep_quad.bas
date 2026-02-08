#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

dim as double q(...)={0.9,0.0, -2,2.2, 0.9,4.5, DBL_MAX,DBL_MAX, _
							1.2,4.5, 3.0,2.0, 2.6,1.4, DBL_MAX,DBL_MAX, _
							1.2,0, DBL_MAX, DBL_MAX, 0.9,0}
csg_difference()
	csg_sweep_quad(lon(q),@q(0),8)
csg_end()

csg_print_tree()

' es necesaria maxima precision, debido al espesor delgado de la pared
' si ponemos tolerancia alta, las paredes no se generan bien
'csg_save_stl("sweep.stl", 0.05, 5) ' paredes bien
csg_save_stl("sweep_quad.stl", 0.2, 5) ' paredes mal

sleep