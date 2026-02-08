#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

dim as double m(...)={0.9,0.0, 0.9,4.5, 1.2,4.5, 1.0,1.4, 1.2,0.0}
csg_difference()
	csg_sweep(lon(m),@m(0),8.0)
csg_end()

csg_print_tree()

' es necesaria maxima precision, debido al espesor delgado de la pared
' si ponemos tolerancia alta, las paredes no se generan bien
'csg_save_stl("sweep.stl", 0.05, 5) ' paredes bien
csg_save_stl("sweep.stl", 0.2, 5) ' paredes mal

sleep