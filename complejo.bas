#include "c-csg.bi"



#define cx 135.0
#define cy 33.0
#define cz 16.0
#define hole_spread 100.0
#define hole_diameter 4.0
#define hole_counter_sink_diameter 10.0
#define hole_counter_sink_depth 4.0

#define slot_diameter 11.0
#define slot_length 20.0
#define slot_spacing 21.0

#define top_slot_cx 80.0
#define top_slot_cy 10.0

Dim As CSG_NODE ptr csg_current_node=csg_init()

csg_difference()
	csg_union()
		csg_box()
		csg_scale(cx,cz,cy)

		csg_cone(cz/2.0,cz/2.0,cx)
		csg_rotate(0,0,PI/2.0)
		csg_translate(-cx/2.0,0,cy/2.0)			

		csg_cone(cz/2.0,cz/2.0,cx)
		csg_rotate(0,0,PI/2.0)
		csg_translate(-cx/2.0,0,cy/2.0)			
		csg_scale(1,1,-1)
	csg_end()

	csg_union()
		csg_cone(hole_diameter/2.0,hole_diameter/2.0,cz*1.1)
		csg_translate(-hole_spread/2.0,-cz*1.1/2.0,0)
		csg_cone(hole_diameter/2.0,hole_diameter/2.0,cz*1.1)
		csg_translate(hole_spread/2.0,-cz*1.1/2.0,0)
	
		csg_cone(hole_diameter/2.0,hole_counter_sink_diameter/2.0,hole_counter_sink_depth)
		csg_translate(-hole_spread/2.0,cz/2.0-hole_counter_sink_depth,0)
		csg_cone(hole_diameter/2.0,hole_counter_sink_diameter/2.0,hole_counter_sink_depth)
		csg_translate(hole_spread/2.0,cz/2.0-hole_counter_sink_depth,0)
	csg_end()
	csg_translate(0,0,1.5) 'slight offset toward top

	csg_union()
		csg_cone(slot_diameter/2.0,slot_diameter/2.0, cz*1.1)
		csg_translate(0,-cz*1.1/2.0,slot_length/2.0-slot_diameter/2.0)
		csg_cone(slot_diameter/2.0,slot_diameter/2.0, cz*1.1)
		csg_translate(0,-cz*1.1/2.0,-(slot_length/2.0-slot_diameter/2.0))
		csg_box()
		csg_scale(slot_diameter,cz*1.1,slot_length-slot_diameter)
	csg_end()
	csg_rotate(0,PI/8.0,0)
	
	Dim AS _OBJECT slot=csg_cut()
	csg_paste(slot)
	csg_translate(-slot_spacing/2.0,0,0)
	csg_paste(slot)
	csg_translate(-(slot_spacing/2.0+slot_spacing),0,0)
	csg_paste(slot)
	csg_translate(slot_spacing/2.0,0,0)
	csg_paste(slot)
	csg_translate((slot_spacing/2.0+slot_spacing),0,0)
	
	csg_box()
	csg_scale(top_slot_cx,cz*1.1,top_slot_cy)
	csg_translate(0,0,top_slot_cy/2.0+1.5)
	
	csg_box()
	csg_scale(85,11.0,25)
	csg_translate(0,-5.0/2.0,1.5)
	
	csg_box()
	csg_scale(75,8,2)
	csg_translate(0,(-cz/2.0)+5.0/2.0,-12)
csg_end()

print "Guardando..."
csg_save_stl("complejo.stl", 2, 8) ' nombre, tolerancia en mm, hilos CPU para procesar (de 1 a 8)

print "Fin..."
sleep