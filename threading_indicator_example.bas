#include "c-csg.bi"

#include "crt/math.bi" ' for "floor"

Dim As CSG_NODE ptr csg_current_node=csg_init()

#define inches_to_mm 25.4
#define degrees_to_rad(a) ((a/360.0)*PI2)


#define screw_outside_d 1.572
#define screw_pitch 0.5
#define screw_major_minor_diff (screw_pitch/2.0)
#define screw_angle degrees_to_rad(29.0/2.0)
#define screw_x_dist_on_angle ( screw_major_minor_diff * atn(screw_angle) )
#define screw_minor_r (screw_outside_d/2.0-screw_major_minor_diff)

#define screw_length 26.0
#define screw_twists (screw_length*(1.0/screw_pitch))

#define gear_teeth 16.0
#define gear_proto_r ((((gear_teeth*screw_pitch)/PI)/2.0)+screw_major_minor_diff)
#define gear_proto_len (30.0/inches_to_mm)


	#define rescale (1.0/screw_pitch) 
	
	dim as double m(...)={_
		0,1.0,_
		screw_outside_d/2.0,1.0, _
		screw_minor_r, 1.0-(rescale * screw_x_dist_on_angle),_
		screw_minor_r, 0.5,_
		screw_outside_d/2.0, 0.5 - (rescale * screw_x_dist_on_angle),_
		screw_outside_d/2.0,0,_
		0,0_
		}
		
		
	csg_lathe(lon(m),@m(0),screw_length,screw_twists)
	'csg_translate(0,-screw_length/2.0,0)
	Dim As _OBJECT screw = csg_cut()

	csg_difference()
		csg_cylinder(gear_proto_r*2.0,gear_proto_len)	

		csg_box()
		csg_scale(5.66/inches_to_mm,gear_proto_len,5.66/inches_to_mm)
				
		for a as double=0 to 1.0-0.01 step 1.0/8.0
			csg_cylinder(gear_proto_r/3.8,gear_proto_len*1.01)	
			csg_translate(-gear_proto_r/2.2,0,0)
			csg_rotate(0,a*PI*2.0,0)
		next
	csg_end()
		
	csg_rotate(0,0,PI/2.0)	
	Dim As _OBJECT gear = csg_cut()
		
	csg_difference()
		csg_paste(gear)

		/'
			virtual circle between gear center and the minor diam of the screw
			used to roll the screw along the outside of the gear
			circumference of cutting_r has to be a multiple of screw pitch
		'/		
		
		dim as double cutting_r = gear_proto_r-screw_major_minor_diff 'start with a rough size, work down
		dim as double d = floor((2.0*PI*cutting_r)/screw_pitch)
		cutting_r = ((d * screw_pitch) / PI)/2.0
		
		
		for i as double=0 to 1.2-0.01 step (1.0/1024.0)
			csg_paste(screw)				
			
			csg_translate(0,-2.0*PI*cutting_r*i, screw_minor_r+cutting_r)
			csg_rotate(2.0*PI*i,0,0)
		next
		
	csg_end()
		
	csg_scale(inches_to_mm,inches_to_mm,inches_to_mm)
		
	csg_save_stl("threading_indicator_example.stl",2,8)

