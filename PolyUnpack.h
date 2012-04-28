#ifndef PolyUnpack2_H
#define PolyUnpack2_H

#include "InstructionList.h"
#include "Instruction_Leaf.h"
#include "Emulator_LibEmu.h"
#include "PE_Reader.h"
#include "Stack.h"
#include "SimpleFunctions.h"


class PolyUnpack; 

class Instruction_Tree{
	Instruction_Leaf* root;
public:
	Instruction_Tree(){root=NULL;};
	void Add(InstructionList* &, InstructionList* &, InstructionList* &);
	void Add_Leaf(Instruction_Leaf* &, int , InstructionList* &, InstructionList* &);
	int Obhod(const char*, PolyUnpack &);
	void Obhod(Instruction_Leaf* &, const char*, InstructionList* &,PolyUnpack &, int &, int &, int &);
	void Obhod2(Instruction_Leaf* &);
	int Check(Instruction_Leaf* &, InstructionList* &, InstructionList* &, InstructionList* &, InstructionList* &, bool & flag);
	bool Is_Empty();
	void Destructor(Instruction_Leaf* &);
	~Instruction_Tree();
	void CopyCommand(Instruction_Leaf* &, const char* str);
};

class PolyUnpack{
	InstructionList* start;//start of list of instructions
	Instruction_Tree root;
	int n;//=4 start size of each buffer instruction
	Emulator_LibEmu* emulator;
	Stack return_addresses;
	unsigned int buf_size;
	unsigned int base;
	unsigned int offset;
	unsigned int code_size;
	int step_kol;
	int invalid_instr_kol;
public:
	PolyUnpack();
	~PolyUnpack();
	void AddInstruction(InstructionList* &, const char*, const int, const int, const bool);
	void FindPadding() const;
	bool FindFirstInstruction(InstructionList* &) const;
	bool FindInstruction(InstructionList* &, const int) const;
	bool is_valid_number(int) const;
	void StaticAnalise(const char*,  const int key); //magic
	void DynamicAnalise(const char*, const int key); //magic
	int Compare(InstructionList* &, const char*, int, const unsigned int); 
												   //1 is true and simple instruction
												   //-1 is false
												   //0 is unknown in case of conditional jump jz/jnz/...
												   //2 when instruction call[], jmp[] was found. Need 
												   //to do emulator.jump()because in other case EIP could be 0
												   //3 when instruction call, ret, retn was found
												   //4 stack is empty
												   //5 invalid address in register case
	int FindNextInstruction(InstructionList* &, InstructionList* &, const char*, int &);
	bool CompareBuffers(const char*);
};
 
/*
class PolyUnpack{
	InstructionList* start;//start of list of instructions
	Instruction_Tree root;
	int n;//=4 start size of each buffer instruction
	Emulator_LibEmu* emulator;
	Stack return_addresses;
	int buf_size;
	//int base;
	int offset;
	int step_kol;
	int invalid_instr_kol;
public:
	PolyUnpack();
	~PolyUnpack();
	void AddInstruction(InstructionList* &, const char*, const int, const int, const bool);
	void FindPadding() const;
	bool FindFirstInstruction(InstructionList* &) const;
	bool FindInstruction(InstructionList* &, const int) const;
	bool is_valid_number(int) const;
	void StaticAnalise(const char*, const int);
	void DynamicAnalise(const char*, const int);
	int Compare(InstructionList* &, const char*, int,  uint, const int); 
												   //1 is true and simple instruction
												   //-1 is false
												   //0 is unknown in case of conditional jump jz/jnz/...
												   //2 when instruction call[], jmp[] was found. Need 
												   //to do emulator.jump()because in other case EIP could be 0
												   //3 when instruction call, ret, retn was found
	int FindNextInstruction(InstructionList* &, InstructionList* &, const char*, int &);
};
*/

#endif
