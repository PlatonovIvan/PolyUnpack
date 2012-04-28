#ifndef Emulator_LibEmu_H
#define Emulator_LibEmu_H


enum Register {
	EIP,EBP,ESP,ESI,EDI,
	IP,BP,SP,SI,DI,
	EAX,EBX,ECX,EDX,
	AX,BX,CX,DX,
	AH,BH,CH,DH,
	AL,BL,CL,DL,
	HASFPU
};

class Emulator_LibEmu{
	struct emu *e;
	struct emu_cpu *cpu;
	struct emu_memory *mem;
	unsigned int im_base;
public:
	Emulator_LibEmu();
	~Emulator_LibEmu();
	void begin(char* &, const unsigned int, const unsigned int, const unsigned int);
	void jump(const unsigned int);
	bool step();
	bool get_command(char*, const unsigned int) const;
	bool read_block(char*, const unsigned int) const;
	unsigned int get_register(const Register) const;
	unsigned int get_register(char* &) const;
};

/*
class Emulator_LibEmu{
	struct emu *e;
	struct emu_cpu *cpu;
	struct emu_memory *mem;
	//uint im_base;
public:
	Emulator_LibEmu();
	~Emulator_LibEmu();
	void begin(char* &, const int, const int);
	void jump(const uint);
	bool step();
	bool get_command(char*, const uint) const;
	unsigned int get_register(const Register) const;
	unsigned int get_register(char* &) const;
};
*/

#endif
