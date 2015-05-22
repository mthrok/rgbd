IDIR = ./
CXX = c++
CFLAGS = -I$(IDIR) --std=c++11

LIBS = -lOpenNi2 -framework SDL2

OBJ = RGBDViewer.o NIDevice.o viewer.o
programs = viewer

all : ${programs}

%.o : %.cxx
	${CXX} -c -o $@ $<  ${CFLAGS}

${programs} : ${OBJ}
	${CXX} -o $@ ${OBJ} ${LIBS} ${CFLAGS}



