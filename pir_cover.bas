#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()


'fit in abs schedule 40 1-1/2"
'sensor fitted to top cap

#define pipe_diam 57.0
#define box_size 30.0
#define wall 3.0

	csg_difference()
		csg_difference()
			csg_box()		
			csg_scale(box_size,box_size,box_size)		
			
			
			csg_box()		
			csg_scale(box_size*1.1,box_size-(wall*2.0),box_size*1.1)		
			csg_rotate(0,PI/4.0,0)	
			csg_translate(10.0,0,0)
		csg_end()
		csg_translate(pipe_diam/2.0,0,0)
	
		csg_cone(pipe_diam/2.0,pipe_diam/2.0,box_size-wall)		
		csg_translate(0,-box_size/2.0,0)
		
		
	csg_end()
	
	csg_save_stl("pir_cover.stl",2,4)
	


