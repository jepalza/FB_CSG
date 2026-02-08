#include "c-csg.bi"
Dim As CSG_NODE ptr csg_current_node=csg_init()

dim as double m(...)={3.0,1.0, 4.0,1.0, 5.0,0.5, 4.0,0.0, 3.0,0.0}
csg_difference()
	csg_lathe(lon(m),@m(0),10,8)
csg_end()

' ver resultado de calculos
csg_print_tree()

csg_save_stl("lathe.stl", 0.25, 1)

print "Fin..."
sleep