CXX = g++

OBJECTS = SecondPartDisas10.o SimpleFunctions.o Stack.o PolyUnpack.o PE_Reader.o

all: SecondPartDisas10

clean:
	rm -f *.o *~

SecondPartDisas10: SecondPartDisas10.o SimpleFunctions.o Stack.o PolyUnpack.o PE_Reader.o Emulator_LibEmu.o
	$(CXX) -o $@ SecondPartDisas10.o  SimpleFunctions.o Stack.o PolyUnpack.o PE_Reader.o Emulator_LibEmu.o libdasm/libdasm.o -lemu -L$(CURDIR)/libemu -Wl,-rpath -Wl,$(CURDIR)/libemu
 
SecondPartDisas10.o: SecondPartDisas10.cpp Headers/Emulator_LibEmu.h Headers/PolyUnpack.h 

	$(CXX) -o $@ -c SecondPartDisas10.cpp -I$(CURDIR)/libemu/include

#Instruction_Tree.o: Instruction_Tree.cpp 	Headers/Instruction_Tree.h 														
#	$(CXX) -o $@ -c Instruction_Tree.cpp


SimpleFunctions.o: SimpleFunctions.cpp 
	$(CXX) -o $@ -c SimpleFunctions.cpp

Stack.o: Stack.cpp Headers/Stack.h
	$(CXX) -o $@ -c Stack.cpp

PolyUnpack.o: PolyUnpack.cpp 	Headers/PolyUnpack.h Headers/InstructionList.h Headers/Instruction_Leaf.h Headers/Emulator_LibEmu.h Headers/PE_Reader.h Headers/Stack.h Headers/SimpleFunctions.h libdasm/libdasm.h

	$(CXX) -o $@ -c PolyUnpack.cpp

PE_Reader.o: PE_Reader.cpp Headers/PE_Reader.h 
	$(CXX) -o $@ -c PE_Reader.cpp

Emulator_LibEmu.o: Emulator_LibEmu.cpp Headers/Emulator_LibEmu.h  libemu/include/emu/emu.h libemu/include/emu/emu_cpu.h libemu/include/emu/emu_memory.h
	$(CXX) -o $@ -c Emulator_LibEmu.cpp
