IDIR = ./include
CXX = c++
CFLAGS = -I$(IDIR) --std=c++11
LIBS = -lOpenNi2 -framework SDL2

_DEPS = RGBDViewer.hpp NIDevice.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

ODIR=obj

_OBJ = RGBDViewer.o NIDevice.o viewer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

SDIR = ./src
BDIR = ./bin

PROGRAMS = viewer

MKDIR_P = mkdir -p
DIRS = ${ODIR} ${SDIR} ${BDIR}

all : ${DIRS} ${PROGRAMS}

${DIRS} :
	${MKDIR_P} $@

${ODIR}/%.o : ${SDIR}/%.cxx ${ODIR} ${DEPS}
	${CXX} -c -o $@ $< ${CFLAGS}

${PROGRAMS} : ${OBJ} ${BDIR}
	${CXX} -o ${BDIR}/$@ ${OBJ} ${CFLAGS} ${LIBS}
