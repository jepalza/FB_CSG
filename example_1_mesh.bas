#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()


	csg_sphere(1)
	csg_translate(5,0,0)
	csg_dup()    
	csg_translate(-10,0,0) 
	csg_save_stl("example_1_mesh.stl", .25, 5)	
