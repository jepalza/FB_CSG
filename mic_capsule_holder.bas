#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

#define inches_to_mm 25.4
#define degrees_to_rad(a) ((a/360.0)*PI2)



	csg_union()
		csg_difference()
			csg_box()
			csg_scale(16,2,23)

			csg_union()
				csg_cone(1.5,1.5,30)
				csg_translate(-10/2,-1.5,15.0/2.0)
				csg_dup()
				csg_scale(-1,1,1)
			csg_end()
		
			csg_dup()
			csg_translate(0,0,-15)

		csg_end()
		csg_translate(0,-1,0)
	
		csg_difference()
			csg_union()
				csg_box()
				csg_translate(0,0.5,0)
				csg_scale(7,15,7)
				
				csg_cone(30.0/2.0,30.0/2.0,10)
				csg_translate(0,-5,0)
				csg_rotate(PI/2.0,0,0)
				csg_translate(0,22,0)
				
			csg_end()
			
			csg_cone(26.0/2.0,26.0/2.0,10) 'capsule sized cutter
			csg_translate(0,-5,0)
			csg_rotate(PI/2.0,0,0)
			csg_translate(0,22,-2)
			
			csg_cone(26.0/2.0,26.0/2.0,10) 'capsule sized cutter
			csg_translate(0,-5,0)
			csg_scale(.9,1.1,.9)
			csg_rotate(PI/2.0,0,0)
			csg_translate(0,22,0)
			
		csg_end()
	csg_end()

	csg_save_stl("mic_capsule_holder.stl", .6, 8)



