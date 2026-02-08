#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

#define inches_to_mm 25.4
#define degrees_to_rad(a) ((a/360.0)*PI2)


#define pin_to_pin_length (2.0*inches_to_mm)
#define maximal_width (3.0*inches_to_mm)
#define maximal_height (1.0*inches_to_mm)
#define pin_diameter 10.0
#define wall_thickness 4.0

#define slop 1.0

#define max_bend_angle degrees_to_rad(180.0/8.0)
#define pin_scale 0.97
#define inside_corner_radius 4.0


Dim As _OBJECT cap
Dim As _OBJECT body
	
	csg_union()
		csg_cone(maximal_height/2.0,maximal_height/2.0,maximal_width)
		csg_rotate(PI/2.0,0,0)
		csg_translate(pin_to_pin_length/2.0,0,maximal_width/2.0)
	
		csg_cone(maximal_height/2.0,maximal_height/2.0,maximal_width-wall_thickness*2.0)
		csg_rotate(PI/2.0,0,0)
		csg_translate(-pin_to_pin_length/2.0,0,(maximal_width-wall_thickness*2.0)/2.0)
	
		csg_box()
		csg_translate(0.5,0,0)
		csg_scale(pin_to_pin_length/2.0,maximal_height,maximal_width)
		csg_box()
		csg_translate(-0.5,0,0)
		csg_scale(pin_to_pin_length/2.0,maximal_height,maximal_width-wall_thickness*2.0)
	
		csg_cone(pin_diameter/2.0,pin_diameter/2.0,maximal_width*pin_scale) 'make it slightly smaller for easier fit
		csg_rotate(PI/2.0,0,0)
		csg_translate(-pin_to_pin_length/2.0,0,maximal_width*pin_scale/2.0)
		
		'put edge on top left to act as a stop
		csg_box()
		csg_translate(-0.5,0.5,0)
		csg_scale(maximal_height/2.0,maximal_height/2.0,maximal_width-wall_thickness*2.0)
		csg_translate(-pin_to_pin_length/2.0,0,0)
		'put edge on bottom left to act as a stop
		csg_box()
		csg_translate(-0.5,-0.5,0)
		csg_scale(maximal_height/2.0,maximal_height/2.0,maximal_width-wall_thickness*2.0)
		csg_translate(-pin_to_pin_length/2.0,0,0)
	
	csg_end()
	
	Dim As _OBJECT midform = csg_cut()
	
	csg_difference()
		csg_paste(midform)
		for i as integer =0 to 63			
			csg_paste(midform)
			csg_scale(1,1,1+slop/maximal_width) 'for slop in hinge
				
			'position to rotate around post
			csg_translate(pin_to_pin_length/2.0,0,0)		
			csg_rotate(0,0, -(max_bend_angle*(cast(double,i)/64.0) )) 'only bend on one side
			'move rotated object into hole position
			
			csg_translate(pin_to_pin_length/2.0-slop/2.0,0,0)
		next
		
		'cut out core		
		csg_box()
		'csg_scale((pin_to_pin_length+maximal_height*2.0)*1.1,maximal_height-wall_thickness*2.0, maximal_width-wall_thickness*4.0)
		csg_scale((pin_to_pin_length+maximal_height*2.0)*1.1,maximal_height-(wall_thickness*2.0+inside_corner_radius), maximal_width-wall_thickness*4.0)
		csg_translate(0,inside_corner_radius/2.0,0)
		csg_box()
		csg_translate(0,0.5,0)
		csg_scale((pin_to_pin_length+maximal_height*2.0)*1.1,inside_corner_radius, maximal_width-(wall_thickness*4.0+inside_corner_radius*2.0))
		csg_translate(0,-maximal_height/2.0+wall_thickness,0)
		
		
		
		csg_cone(inside_corner_radius,inside_corner_radius,1.0)
		csg_translate(0,-0.5,0)
		csg_rotate(0,0,PI/2.0)
		csg_scale((pin_to_pin_length+maximal_height*2.0)*1.1,1,1)
		csg_translate(0,-(maximal_height-(wall_thickness*2.0+inside_corner_radius*2.0))/2.0, (maximal_width-(wall_thickness*4.0+inside_corner_radius*2.0))/2.0)
		
		
		csg_cone(inside_corner_radius,inside_corner_radius,1.0)
		csg_translate(0,-0.5,0)
		csg_rotate(0,0,PI/2.0)
		csg_scale((pin_to_pin_length+maximal_height*2.0)*1.1,1,1)
		csg_translate(0,-(maximal_height-(wall_thickness*2.0+inside_corner_radius*2.0))/2.0, -(maximal_width-(wall_thickness*4.0+inside_corner_radius*2.0))/2.0)
		
		'cut out sloppy hole
		csg_cone(pin_diameter/2.0+slop/2.0,pin_diameter/2.0+slop/2.0,maximal_width)
		csg_rotate(PI/2.0,0,0)
		csg_translate(pin_to_pin_length/2.0,0,maximal_width/2.0)
		
	csg_end()
		
		
	midform=csg_cut()
	
	' cut anything that comes through bottom during bend
	csg_difference()
		csg_paste(midform)
		for i as integer =0 to 128	
			csg_box()			
			csg_scale(pin_to_pin_length+maximal_height*2.0,maximal_height,maximal_width)
			csg_translate(0,-maximal_height,0)
			csg_rotate(0,0, (max_bend_angle*(cast(double,i)/128.0) ))
			csg_translate(-pin_to_pin_length/2.0,0,0)				
		next
	csg_end()
		
	midform=csg_cut()	
		
	'cut out top
	csg_difference()
		csg_paste(midform)
		csg_box()
		csg_translate(0,0.5,0)
		csg_scale((pin_to_pin_length+maximal_height*2.0)*1.1,wall_thickness*2.0, maximal_width-wall_thickness*4.0)		
		csg_translate(0,maximal_height/2.0-wall_thickness*1.1,0)
		
		'cut out divets
		for i as double=0 to 1.0-0.01 step 1.0/8.0
			csg_sphere(wall_thickness/2.5)
			csg_translate(i*(pin_to_pin_length+maximal_height)-(pin_to_pin_length+maximal_height)/2.0+wall_thickness,_
				maximal_height/2.0-wall_thickness/2.0,_
				maximal_width/2.0-wall_thickness*2.0)
			
			csg_sphere(wall_thickness/2.5)
			csg_translate(i*(pin_to_pin_length+maximal_height)-(pin_to_pin_length+maximal_height)/2.0+wall_thickness,_
				maximal_height/2.0-wall_thickness/2.0,_
				maximal_width/2.0-wall_thickness*2.0)
			csg_scale(1,1,-1)
		next
	csg_end()
		
	body=csg_cut()
		
		
	'make cap	
	csg_intersection()
		csg_paste(midform)
		
		csg_union()
			csg_box()
			csg_translate(0,0.5,0)
			csg_scale((pin_to_pin_length+maximal_height*2.0)*1.1,wall_thickness*2.0, maximal_width-wall_thickness*4.0)		
			csg_translate(0,maximal_height/2.0-wall_thickness*1.1,0)
			
			'divets
			for i as double=0 to 1.0-0.01 step 1.0/8.0
				csg_sphere(wall_thickness/2.5)
				csg_scale(0.8,0.8,1)
				csg_translate(i*(pin_to_pin_length+maximal_height)-(pin_to_pin_length+maximal_height)/2.0+wall_thickness,_
					maximal_height/2.0-wall_thickness/2.0,_
					maximal_width/2.0-wall_thickness*2.0)
				
				csg_sphere(wall_thickness/2.5)
				csg_scale(0.8,0.8,1)
				csg_translate(i*(pin_to_pin_length+maximal_height)-(pin_to_pin_length+maximal_height)/2.0+wall_thickness,_
					maximal_height/2.0-wall_thickness/2.0,_
					maximal_width/2.0-wall_thickness*2.0)
				csg_scale(1,1,-1)
			next
		csg_end()
	csg_end()
	
	'csg_scale(1,1,1.02) 'slightly stretch it for more holding pressure
	
	cap=csg_cut()
			
	'cut cap on top where it might restrict wire path
	csg_difference()		
		csg_paste(cap)
			
		csg_box()
		csg_scale(pin_to_pin_length+maximal_height*2.0, maximal_height-wall_thickness*2.0, maximal_width)		
		csg_translate(-pin_to_pin_length/2.0,0,0)				
		csg_rotate(0,0, max_bend_angle ) 		
			
		csg_translate(-pin_to_pin_length/2.0,0,0)				
	csg_end()
			
	cap=csg_cut()
	
	
	
	csg_world_z_up()
	
	csg_paste(cap)
	csg_save_stl("example_cable_chain_top.stl", 2, 5)	
	csg_cut()
	
	csg_paste(body)	
	csg_save_stl("example_cable_chain_segment.stl", 2, 5)	

