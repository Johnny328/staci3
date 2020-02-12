
STACI_DIR=../bin/
FUNC=spr2inp

CXX=clang++
CXXFLAGS=-std=c++17 -I/usr/include/python2.7 -I/usr/local/include/igraph -ligraph -O3 -fopenmp -mavx

OBJS += \
$(STACI_DIR)BasicFileIO.o \
$(STACI_DIR)Edge.o \
$(STACI_DIR)HydraulicSolver.o \
$(STACI_DIR)Graph.o \
$(STACI_DIR)IOinp.o \
$(STACI_DIR)IOxml.o \
$(STACI_DIR)Node.o \
$(STACI_DIR)Pipe.o \
$(STACI_DIR)Pool.o \
$(STACI_DIR)PressurePoint.o \
$(STACI_DIR)Pump.o \
$(STACI_DIR)Shutdown.o \
$(STACI_DIR)Staci.o \
$(STACI_DIR)Statistic.o \
$(STACI_DIR)Valve.o \
$(STACI_DIR)ValveFCV.o \
$(STACI_DIR)ValveISO.o \
$(STACI_DIR)ValvePRV.o \
$(STACI_DIR)ValvePSV.o \
$(STACI_DIR)ValveTCV.o \
$(STACI_DIR)xmlParser.o \
$(FUNC).o

%.o: ../../%.cpp
	@echo '[*] Building file: $<'
	@echo '[*] Invoking: CLANG++ Compiler'
	$(CXX) $(CXXFLAGS) -c -MT "$(@:%.o=%.d)" -o "$@"  "$<"
	@echo '[*] Finished building: $<'
	@echo ' '

all: $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $(FUNC) $(OBJS)

clean:
	-rm $(STACI_DIR)*.o $(FUNC) $(FUNC).o