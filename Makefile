INCFLAGS  = -I include/GL
INCFLAGS += -I include/vecmath
#INCFLAGS += -I ./
#INCFLAGS += -I include/FL

LINKFLAGS  = -lglut -lGL -lGLU
#LINKFLAGS += -Llib -Llib/FL
LINKFLAGS += -lfltk
LINKFLAGS += -lfltk_gl
#LINKFLAGS += -lfltk_z
LINKFLAGS += -lfltk_forms

#LINKFLAGS += -lfltk_images
#LINKFLAGS += -l:libfltk.a -l:libfltk_gl.a
#LINKFLAGS += -lfltk.lib -l fltkgl.lib
#LINKFLAGS += -lfltk -lfltk_gl
#LINKFLAGS += -L lib/vecmath -l:libvecmath.a
#LINKFLAGS += -L ./ -l:libfltk.so.1.1
#LINKFLAGS += -lfltk.lib -lfltkgl.lib -lfltkimages.lib -lfltkjpeg.lib -lfltkforms.lib -lfltkpng.lib -lfltkzlib.lib -libfltk.so.1.1

#CFLAGS  = -std=gnu++11 -Wall -I /usr/local/include /usr/local/bin/fltk-config --cxxflags
CFLAGS  = -g
CFLAGS += -DSOLN
#CCFLAGS = './fltk-config --cxxflags'
#CFLAGS += fltk-config --cxxflags
#CFLAGS    = -O2 -Wall -ansi -DSOLN
CC        = g++
#SRCS     = main.cpp parse.cpp curve.cpp surf.cpp camera.cpp
SRCS      = $(wildcard *.cpp)
SRCS 	 += $(wildcard vecmath/*.cpp)
OBJS      = $(SRCS:.cpp=.o)
PROG      = assignment_2

all: $(SRCS) $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LINKFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@ $(INCFLAGS)

depend:
	makedepend $(INCFLAGS) -Y $(SRCS)

clean:
	rm $(OBJS) $(PROG)

bitmap.o: bitmap.h
camera.o: camera.h
Mesh.o: Mesh.h
MatrixStack.o: MatrixStack.h
modelerapp.o: modelerapp.h ModelerView.h modelerui.h bitmap.h camera.h
modelerui.o: modelerui.h ModelerView.h bitmap.h camera.h modelerapp.h
ModelerView.o: ModelerView.h camera.h
SkeletalModel.o: MatrixStack.h ModelerView.h Joint.h modelerapp.h
