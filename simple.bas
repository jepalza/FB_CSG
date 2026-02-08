#include "c-csg.bi"

' ejemplo en C, convertido a FB
'model(example_1)
'{
'	difference();
'		sphere(10);
'		plane(0, 1, 0, 5);
'		plane(0, -1, 0, 5);
'	end();

'	save_stl("./tmp/example_1_mesh.stl", 2, 5);
'}

Dim As CSG_NODE ptr csg_current_node=csg_init()

csg_difference()
	'csg_color(0,0,1)
	csg_sphere(10)
	'csg_color(1,0,0)
	csg_plane(0, 1,0,5)
	'csg_color(0,1,0)
	csg_plane(0,-1,0,5)
csg_end()

' ver resultado de calculos
csg_print_tree()


print "Guardando..."
csg_save_stl("simple.stl", 0.5, 5) ' nombre, tolerancia en mm, hilos CPU para procesar (de 1 a 8)


print "Fin..."
sleep