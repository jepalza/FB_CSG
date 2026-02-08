' Libreria CSG para FreeBasic Joseba Epalza <jepalza arroba gmail punto com>
' DLL preparada usando los fuentes originales de http://c-csg.com/
' Enero 2026

#inclib "csg"

#include "noise.bi"

#define PI 3.14159265358979323846
#define PI2 (PI*2.0)

#ifndef NULL
  #define NULL cptr(any ptr,0)
#EndIf

Type _SPHERE 
	As Double r 
End Type 

Type _BOX 
	'1x1x1 centered
	As Double vacio ' nada, vacio, para evitar error al compilar
End Type 

Type _PLANE 
	As Double x, y, z, d 
End Type 

Type _CONE 
	As Double r1, r2, d 
End Type 

Type _SWEEP 
	As Double Ptr v  'this is a copy of original points, the last point guaranteed to equal the first
	As Long v_len 
	As Double xmaxx  'for early out
End Type 

Type _LATHE 
	As Long v_len 

	As Double Ptr v  'this is a copy of original points,  the last point guaranteed to equal the first
	As Double xmaxx  'max of x coords in vs for early out

	As Double d 
	As Double twists 
End Type 

Type _EXTRUDE 
	As Double Ptr v 
	As Long v_len 
	As Double d 
End Type 


Type _EXTRUDE_QUAD 
	As Double Ptr quad  'x,y,cx,cy,x,y,cx,cy,x,y,.., control points set to QUAD_LINE indicates a straight line
	As Long quad_len  'actual double count
	As Double d 
End Type 


Type _SWEEP_QUAD 
	As Double Ptr v  'this is a copy of original points, the last point guaranteed to equal the first
	As Long v_len 
	As Double xmaxx  'for early out
End Type 

Type _LATHE_QUAD 
	As Long v_len 

	As Double Ptr v  'this is a copy of original points,  the last point guaranteed to equal the first
	As Double xmaxx  'max of x coords in vs for early out

	As Double d 
	As Double twists 
End Type 


Type _INTERIOR 
	p As Function(v As Double Ptr) As Long
End Type 

Type _EXTERIOR 
	p as sub(v As Double Ptr)
	As Double padding 
End Type 

Type CSG_NODE 
   As CSG_NODE Ptr parent 
   As CSG_NODE Ptr children 
	As Long children_len 

	As Double transform(15) 

	As Double world_to_object(15) 
	As Double object_to_world(15) 

	As Double bbox(5)  'xmin, ymin, zmin, xmax, ymax, zmax in world coords, no rotation, if isnan(xmin) bbox is not used

	As Single colors(2) 
	As UByte color_set 

	As UByte bbox_valid 

	As UByte object_type 

	Union CSG_OBJECT
		As _SPHERE sphere
		As _BOX box
		As _PLANE plane
		As _CONE cone
		As _SWEEP sweep
		As _SWEEP_QUAD sweep_q
		As _LATHE lathe
		As _LATHE_QUAD lathe_q
		As _EXTRUDE extrude
		As _EXTRUDE_QUAD extrude_q
		As _INTERIOR interior
		As _EXTERIOR exterior
	End Union
	
	As Any Ptr user_tag 
End Type 

Type As CSG_NODE PTR _OBJECT



'user script functions
Declare Sub csg_union Cdecl Alias "csg_union"( ) 'the object that can have children auto push the context,ByVal so objects are then created as children until the context is popped
Declare Sub csg_difference Cdecl Alias "csg_difference"( ) 
Declare Sub csg_intersection Cdecl Alias "csg_intersection"( ) 
Declare Sub csg_end Cdecl Alias "csg_end"( ) 

Declare Sub csg_print_tree Cdecl Alias "csg_print_tree"( ) 'print object tree to console
Declare Sub csg_dimensions Cdecl Alias "csg_dimensions"(ByVal cx As Double Ptr,ByVal cy As Double Ptr,ByVal cz As Double Ptr)  'returns based on bbox dims

Declare Sub csg_identity Cdecl Alias "csg_identity"( ) 'reset matrix for current object

Declare Sub csg_sphere Cdecl Alias "csg_sphere"(ByVal r As Double) 
Declare Sub csg_plane Cdecl Alias "csg_plane"(ByVal x As Double,ByVal y As Double,ByVal z As Double,ByVal r As Double) 
Declare Sub csg_box Cdecl Alias "csg_box"( ) 
Declare Sub csg_cone Cdecl Alias "csg_cone"(ByVal r1 As Double,ByVal r2 As Double,ByVal d As Double) 

Declare Sub csg_interior Cdecl Alias "csg_interior"(p As Function cdecl(v As Double ptr) as Long )
Declare Sub csg_exterior Cdecl Alias "csg_exterior"(p as sub cdecl(v As Double ptr) )

Declare Sub csg_scale Cdecl Alias "csg_scale"(ByVal x As Double,ByVal y As Double,ByVal z As Double) 
Declare Sub csg_translate Cdecl Alias "csg_translate"(ByVal x As Double,ByVal y As Double,ByVal z As Double) 

Declare Sub csg_rotate Cdecl Alias "csg_rotate"(ByVal x As Double,ByVal y As Double,ByVal z As Double) 
Declare Sub csg_rotate_arbitrary Cdecl Alias "csg_rotate_arbitrary"(ByVal a As Double,ByVal x As Double,ByVal y As Double,ByVal z As Double) 

Declare Sub csg_extrude Cdecl Alias "csg_extrude"(ByVal nvert As Long,ByVal v As Double Ptr,ByVal d As Double) 

Declare Sub csg_sweep Cdecl Alias "csg_sweep"(ByVal poly_len As Long,ByVal poly As Double Ptr,ByVal r As Double) 
Declare Sub csg_sweep_quad Cdecl Alias "csg_sweep_quad"(ByVal poly_len As Long,ByVal poly As Double Ptr,ByVal r As Double) 

Declare Sub csg_lathe Cdecl Alias "csg_lathe"(ByVal poly_len As Long,ByVal poly As Double Ptr,ByVal d As Double,ByVal twists As Double) 
Declare Sub csg_lathe_quad Cdecl Alias "csg_lathe_quad"(ByVal poly_len As Long,ByVal poly As Double Ptr,ByVal d As Double,ByVal twists As Double) 

Declare Sub csg_flip_world_axis Cdecl Alias "csg_flip_world_axis"(ByVal x As Long,ByVal y As Long,ByVal z As Long) 
Declare Sub csg_world_z_up Cdecl Alias "csg_world_z_up"( ) 
Declare Sub csg_extrude_quad Cdecl Alias "csg_extrude_quad"(ByVal qlen As Long,ByVal quad As Double Ptr,ByVal d As Long) 

Declare Function csg_cut Cdecl Alias "csg_cut"( ) As CSG_NODE Ptr  'remove current node and children from main tree
Declare Sub csg_paste Cdecl Alias "csg_paste"(ByVal n As CSG_NODE Ptr) 'put a copy of node and children into the main tree
Declare Sub csg_dup Cdecl Alias "csg_dup"( ) 'duplicate current node as CSG_NODE 

Declare Sub csg_save_slice Cdecl Alias "csg_save_slice"(ByVal file As Zstring Ptr,ByVal x1 As Double,ByVal z1 As Double,ByVal x2 As Double,ByVal z2 As Double,ByVal y As Double,ByVal image_width As Long,ByVal image_height As Long) 
Declare Sub csg_save_stl Cdecl Alias "csg_save_stl"(ByVal file As Zstring Ptr,ByVal resolution As Double,ByVal threads As Long) 

Declare Sub csg_color Cdecl Alias "csg_color"(ByVal r As Single,ByVal g As Single,ByVal b As Single) 
Declare Sub csg_text Cdecl Alias "csg_text"(ByVal text As Zstring Ptr,ByVal font As Zstring Ptr,ByVal d As Double) 


' varios mediante llamadas a funciones definidas
#define csg_cube(a) csg_box(): csg_scale(a,a,a):
#define csg_cylinder(a,b) csg_cone((a)/2.0,(a)/2.0,b): csg_translate(0,-(b)/2.0,0):
#define csg_pipe(a,b,c) csg_difference(): csg_cone((a)/2.0,(a)/2.0,c): csg_cone((b)/2.0,(b)/2.0,(c)*1.001): csg_translate(0,-(c)*0.0005,0): csg_end(): csg_translate(0,-(c)/2.0,0):

#define csg_mirror_x() csg_scale(-1,1,1)
#define csg_mirror_y() csg_scale(1,-1,1)
#define csg_mirror_z() csg_scale(1,1,-1)

' nota: definido en "crt\limits.bi", pero por comodidad, duplicado aqui
#define DBL_MAX (1.7976931348623157e+308)

#define lon(m) (ubound((m))+1)

' inicializar
Declare Function csg_init Cdecl Alias "csg_init"(byval osctx as long ptr=null) as CSG_NODE Ptr