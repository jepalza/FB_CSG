#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

csg_difference()
	csg_color(0,1,1) ' cian para la esfera
	
	csg_sphere(10)

	csg_plane(0, 1, 0, 5)
	csg_plane(0, -1, 0, 5)

	For i as double = 0.0 to 1.0-0.01 step 1.0 / 32.0
		csg_cone(1.5, 1.5, 16)
		csg_translate( sin( i * PI2 ) * 7 ,0 , cos( i * PI2 ) * 7 )
	Next i
	
csg_end()

csg_save_stl("slice.stl", .25, 5)
csg_save_slice("slice.ppm",-15,-15,15,15,1,640,640) ' 640x640=relacion 1:1
