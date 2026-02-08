type osn_context as long

Declare Sub open_simplex_noise_free Cdecl Alias "open_simplex_noise_free"(ByVal ctx As osn_context Ptr) 

Declare Function open_simplex_noise Cdecl Alias "open_simplex_noise"(ByVal seed As Long ,ByVal ctx As osn_context Ptr Ptr) As Integer 
Declare Function open_simplex_noise_init_perm Cdecl Alias "open_simplex_noise_init_perm"(ByVal ctx As osn_context Ptr ,ByVal p As Short ,ByVal nelements As Integer) As Integer 
Declare Function open_simplex_noise2 Cdecl Alias "open_simplex_noise2"(ByVal ctx As osn_context Ptr ,ByVal x As Double ,ByVal y As Double) As Double 
Declare Function open_simplex_noise3 Cdecl Alias "open_simplex_noise3"(ByVal ctx As osn_context Ptr ,ByVal x As Double ,ByVal y As Double ,ByVal z As Double) As Double 
Declare Function open_simplex_noise4 Cdecl Alias "open_simplex_noise4"(ByVal ctx As osn_context Ptr ,ByVal x As Double ,ByVal y As Double ,ByVal z As Double ,ByVal w As Double) As Double 
