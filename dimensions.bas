#include "c-csg.bi"


Dim As CSG_NODE ptr csg_current_node=csg_init()

dim as double plate_cx = 80.0
dim as double plate_cz = 25.0
dim as double text_border = 5.0
dim as double text_cx,text_cy,text_cz

csg_difference()

    csg_box()
    csg_translate(0,-0.5,0)
    csg_scale(plate_cx,4,plate_cz) 

    'csg_text("C-CSG","/usr/share/fonts/truetype/droid/DroidSerif-Bold.ttf",4.5)
	csg_sphere(5)

    'center text on plate
    csg_dimensions(@text_cx,@text_cy,@text_cz) ' coge dimensiones del ultimo objeto creado, una esfera     
    csg_translate(-text_cx/2.0,-4,-text_cz/2.0) 'center text on zero  
    csg_scale((plate_cx-(text_border*2.0))/text_cx,1,(plate_cz-(text_border*2.0))/text_cz)  'scale to correct size to fill plate minus the border padding

csg_end()

csg_save_stl("dimensions.stl", 2, 3) 

