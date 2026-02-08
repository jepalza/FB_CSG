#include "c-csg.bi"

dim shared as long ptr osctx
Function my_interior_noise cdecl(byval locals as double ptr) as Long
    ' use three octaves 
    dim as double v0,v1,v2

    v0 = open_simplex_noise3(osctx, locals[0] / 2.0, locals[1] / 2.0, locals[2] / 2.0)
    v1 = open_simplex_noise3(osctx, locals[0] / 4.0, locals[1] / 4.0, locals[2] / 4.0)
    v2 = open_simplex_noise3(osctx, locals[0] / 8.0, locals[1] / 8.0, locals[2] / 8.0)

    dim as double v = v0+v1+v2

    if (v>=-1.0) andalso (v<=0.4) then 'arbitrary range is solid
        return 1
	 endif
	
    return 0 
End Function


Dim As CSG_NODE ptr csg_current_node=csg_init(cast(long ptr,@osctx))

csg_interior(@my_interior_noise())
  csg_sphere(5)
  csg_sphere(5)
  csg_translate(5,0,0)
csg_end()

csg_save_stl("interior.stl", .25, 5)
