#include <stdio.h>
#include <string.h>
#include "Headers/Emulator_LibEmu.h"
extern "C"{
	
	#include "libemu/include/emu/emu.h"
	#include "libemu/include/emu/emu_cpu.h"
	#include "libemu/include/emu/emu_memory.h"
}

Emulator_LibEmu::Emulator_LibEmu(){
	e = emu_new();
	cpu = emu_cpu_get(e);
	mem = emu_memory_get(e);
	im_base=0;
}

Emulator_LibEmu::~Emulator_LibEmu(){
	emu_free(e);
}


void Emulator_LibEmu::begin(char* & buf, const uint len, const uint entry_point, const uint base){
	im_base=base;
	for (int i=0; i<8; i++){
		emu_cpu_reg32_set(cpu, (emu_reg32) i, 0);
	}
	emu_cpu_reg32_set(cpu, esp, int(im_base/2));


	emu_memory_clear(mem);
	emu_memory_write_block(mem, im_base, (void*)buf, len);
	/*
	int i=0;
	for (i=0; i<len; i++){
		emu_memory_write_byte(mem, 0x20000000L+i, buf[i]);
	}
	*/
	emu_memory_write_byte(mem, im_base+len, '\xcc');
	jump(entry_point);
}




void Emulator_LibEmu::jump(const uint pos){
	emu_cpu_eip_set(cpu, im_base+pos);
}

bool Emulator_LibEmu::step(){
	if (emu_cpu_parse(cpu) != 0) {
		//printf("Bad parse\n");
		return false;
	}
	if (emu_cpu_step(cpu) != 0) {
		//printf("Bad Step\n");
		return false;
	}
	return true;
}

bool Emulator_LibEmu::get_command(char *buff, const uint size) const{
	emu_memory_read_block(mem, emu_cpu_eip_get(cpu), buff, size);
	return true;
}

bool Emulator_LibEmu::read_block(char *buff, const uint size) const{
	emu_memory_read_block(mem, im_base, buff, size);
	return true;
}

unsigned int Emulator_LibEmu::get_register(const Register reg) const{
	switch (reg) {
		case EAX:
			return emu_cpu_reg32_get(cpu, eax);
		case EBX:
			return emu_cpu_reg32_get(cpu, ebx);
		case ECX:
			return emu_cpu_reg32_get(cpu, ecx);
		case EDX:
			return emu_cpu_reg32_get(cpu, edx);
		case ESI:
			return emu_cpu_reg32_get(cpu, esi);
		case EDI:
			return emu_cpu_reg32_get(cpu, edi);
		case ESP:
			return emu_cpu_reg32_get(cpu, esp);
		case EBP:
			return emu_cpu_reg32_get(cpu, ebp);
		case EIP:
			return emu_cpu_eip_get(cpu);
		default:;
	}
	return 0;
}

unsigned int Emulator_LibEmu::get_register(char* &string) const{
	int i=0;
	int delta='A'-'a';
	while (string[i]!='\0'){
		if ((string[i]>='A')&&(string[i]<='Z'))
			string[i]=string[i]-delta;
		i++;
	}
	if (strcmp(string,"eip")==0)
		return emu_cpu_eip_get(cpu);
	if (strcmp(string,"ebp")==0)
		return emu_cpu_reg32_get(cpu, ebp);
	if (strcmp(string,"esp")==0)
		return emu_cpu_reg32_get(cpu, esp);
	if (strcmp(string,"esi")==0)
		return emu_cpu_reg32_get(cpu, esi);
	if (strcmp(string,"edi")==0)
		return emu_cpu_reg32_get(cpu, edi);
	if (strcmp(string,"eax")==0)
		return emu_cpu_reg32_get(cpu, eax);
	if (strcmp(string,"ebx")==0)
		return emu_cpu_reg32_get(cpu, ebx);
	if (strcmp(string,"ecx")==0)
		return emu_cpu_reg32_get(cpu, ecx);
	if (strcmp(string,"edx")==0)
		return emu_cpu_reg32_get(cpu, edx);
	return 0;
}
