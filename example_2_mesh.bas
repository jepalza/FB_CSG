#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

	csg_intersection()
		csg_sphere(10)
		csg_translate(-5, 0, 0)
		csg_sphere(10)
		csg_translate(5, 0, 0)
	csg_end()
	csg_save_stl("example_2_mesh.stl", .25, 5)    

