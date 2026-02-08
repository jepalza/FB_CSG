#include "c-csg.bi"

Dim As CSG_NODE ptr csg_current_node=csg_init()

csg_intersection()
    csg_sphere(10)
    csg_translate(-5,0,0)
    csg_sphere(10)
    csg_translate(5,0,0)
    csg_sphere(10)
    csg_translate(0,0,-5)
    csg_sphere(10)
    csg_translate(0,0,5)
csg_end()

Dim as _OBJECT my_var = csg_cut()

csg_difference()
    csg_sphere(10)
    csg_paste(my_var)
    csg_translate(0,5,0)
    csg_paste(my_var)
    csg_translate(0,-5,0)
csg_end()

csg_save_stl("paste.stl", 1, 3) 

