#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

dim as double m(...)={0.9,0, 0.9,4.5, 1.2,4.5, 4.6,1.4, 1.2,0}
csg_difference()
	csg_extrude(lon(m),@m(0),1.0)
csg_end()

csg_print_tree()

' es necesaria maxima precision, por el borde de la pieza
' si ponemos tolerancia alta, los bordes se ven serrados
'csg_save_stl("extrude.stl", 0.01, 5) ' bordes bien
csg_save_stl("extrude.stl", 0.2, 5) ' bordes mal

sleep