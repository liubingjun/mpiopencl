MPICXX = mpic++
CXXFLAGS = 
INCPATH = -I/usr/include/CL -I/usr/include/mpi -I.
LIBS = -L/usr/lib -L. -lOpenCL
LIBFLAGS = 

SOURCES = main.cpp mpiOCL.cpp config.cpp
HEADERS = common.h mpiOCL.h config.h mix.h
OBJECTS = main.o mpiOCL.o config.o mix.cpp
TARGET = main

$(TARGET) : $(OBJECTS)
		$(MPICXX) $(LIBFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

main.o : main.cpp mpiOCL.h
		$(MPICXX) -c $(INCPATH) $(CXXFLAGS) -o main.o main.cpp

mpiOCL.o : mpiOCL.cpp mpiOCL.h common.h
		$(MPICXX) -c $(INCPATH) $(CXXFLAGS) -o mpiOCL.o mpiOCL.cpp

clean :
		rm $(TARGET) $(OBJECTS)

