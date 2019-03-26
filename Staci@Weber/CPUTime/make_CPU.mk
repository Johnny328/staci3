
STACI_DIR=../bin/
FUNC=CPU

CXX=clang++
CXXFLAGS=-g -std=c++11 -Wall -pedantic -I/usr/include/python2.7 -I/usr/local/include/igraph

OBJS += \
$(STACI_DIR)BasicFileIO.o \
$(STACI_DIR)Calibration.o \
$(STACI_DIR)Edge.o \
$(STACI_DIR)HydraulicSolver.o \
$(STACI_DIR)IOxml.o \
$(STACI_DIR)Node.o \
$(STACI_DIR)Pipe.o \
$(STACI_DIR)Pool.o \
$(STACI_DIR)PressurePoint.o \
$(STACI_DIR)Sensitivity.o \
$(STACI_DIR)SVDCalibration.o \
$(STACI_DIR)Staci.o \
$(STACI_DIR)Statistic.o \
$(STACI_DIR)Valve.o \
$(STACI_DIR)xmlParser.o \
$(FUNC).o

%.o: ../../%.cpp
	@echo '[*] Building file: $<'
	@echo '[*] Invoking: CLANG++ Compiler'
	$(CXX) $(CXXFLAGS) -O2 -c -fmessage-length=0 -MMD -MP -MF "$(@:%.o=%.d)" -MT "$(@:%.o=%.d)" -o "$@"  "$<"
	@echo '[*] Finished building: $<'
	@echo ' '

all: $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $(FUNC) $(OBJS)

clean:
	-rm $(STACI_DIR)*.o $(STACI_DIR)*.d $(FUNC) $(FUNC).o