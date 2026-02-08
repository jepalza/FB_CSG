#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()
csg_difference()

	csg_sphere(10)

	csg_plane(0, 1, 0, 5)
	csg_plane(0, -1, 0, 5)

	For i as double = 0.0 to 1.0-0.01 step 1.0 / 32.0
		csg_cone(0.5, 0.5, 6)
		csg_translate( sin( i * PI2 ) * 10 ,0 , cos( i * PI2 ) * 10 )
	Next i
	
csg_end()

csg_save_stl("tambor.stl", .25, 5)