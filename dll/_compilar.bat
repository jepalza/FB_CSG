md obj
del /s *.o
del _resultado.txt

set OPTS=--std=gnu11 -D__-D_POSIX_SOURCE -D_GNU_SOURCE -g -ffast-math -pthread -Wall -O3 -DNOFREETYPE  
set DIRINC=-I.\src
set LIBS=-static -lmingw32 -lm -lpthread 

GCC %OPTS% %DIRINC% -c src\bb.c     -o .\obj\bb.o     2>>_resultado.txt
GCC %OPTS% %DIRINC% -c src\c-csg.c  -o .\obj\c-csg.o  2>>_resultado.txt
GCC %OPTS% %DIRINC% -c src\noise.c  -o .\obj\noise.o  2>>_resultado.txt
GCC %OPTS% %DIRINC% -c src\stl.c    -o .\obj\stl.o    2>>_resultado.txt

GCC ^
  .\obj\bb.o ^
  .\obj\c-csg.o ^
  .\obj\noise.o ^
  .\obj\stl.o ^
  %LIBS% ^
  -o csg.dll -shared ^
     2>>_resultado.txt
