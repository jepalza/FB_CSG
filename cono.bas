#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

csg_difference()
	csg_cone(5,10,20)
csg_end()

' ver resultado de calculos
csg_print_tree()


print "Guardando..."
csg_save_stl("cono.stl", 0.25, 1) ' nombre, tolerancia en mm, hilos CPU para procesar (de 1 a 8)


print "Fin..."
sleep