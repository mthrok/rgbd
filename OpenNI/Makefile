IDIR = ./include
SDIR = ./src
BDIR = ./bin
ODIR = ./obj

CXX = c++
CFLAGS = -I$(IDIR) --std=c++11
LIBS = -lOpenNi2 -framework SDL2

_DEPS = RGBDVisualizer.hpp NIDevice.hpp io.hpp types.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = RGBDVisualizer.o NIDevice.o io.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

PROGRAMS = viewer recorder

MKDIR_P = mkdir -p
DIRS = ${ODIR} ${SDIR} ${BDIR}

all : ${DIRS} ${PROGRAMS}

${DIRS} :
	${MKDIR_P} $@

${ODIR}/%.o : ${SDIR}/%.cxx ${ODIR} ${DEPS}
	${CXX} -c -o $@ $< ${CFLAGS}

viewer : ${ODIR}/viewer.o ${OBJ} ${BDIR}
	${CXX} -o ${BDIR}/$@ $< ${OBJ} ${CFLAGS} ${LIBS}

recorder : ${ODIR}/recorder.o ${OBJ} ${BDIR}
	${CXX} -o ${BDIR}/$@ $< ${OBJ} ${CFLAGS} ${LIBS}
