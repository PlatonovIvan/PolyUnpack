#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <math.h>
extern "C"{
	#include "libdasm/libdasm.h"
	#include <emu/emu.h>
	#include <emu/emu_cpu.h>
	#include <emu/emu_memory.h>
}

using namespace std;
/*
 * ins/insb/insw/insd     -vvod strok iz porta
 * outsb/outsw/outsd      -vivod stroki v port 
 * 
 * 
 * 
 * 
 * 
 * */
enum Register {
	EIP,EBP,ESP,ESI,EDI,
	IP,BP,SP,SI,DI,
	EAX,EBX,ECX,EDX,
	AX,BX,CX,DX,
	AH,BH,CH,DH,
	AL,BL,CL,DL,
	HASFPU
};

const int mem_before = 10*1024; // 10 KiB, min 1k instuctions
const int mem_after = 80*1024; //80 KiB, min 8k instructions

struct InstructionList{
	char* instruction;//points to disas instruction
	InstructionList* pointer;
};

class Reader{
protected:
	void read();
	virtual void parse();
	bool indirect; ///<do we need to delete data at the end?
	string filename; ///<input file name
	const unsigned char *data; ///<buffer containing binary file
	uint dataSize; ///<size of buffer data
	uint dataStart; ///<start of the actual data in buffer
	uint base;///< Base of addresses in memory
public:
	Reader();
	Reader(const Reader *reader);
	~Reader();
	void load(string name);
	void link(const unsigned char *data, uint dataSize);
	string name();
	const unsigned char *pointer(bool nohead=false) const;
	uint size(bool nohead=false) const;
	uint start();
	//virtual 
	uint entrance();
	//virtual 
	uint map(uint addr);
	//virtual 
	bool is_valid(uint addr);
	//virtual 
	bool is_within_one_block(uint a, uint b);
};

class PE_Reader{
protected:
	char* filename;
	FILE *fp;
	int header_offset;
	int table_offset;
	int text_offset;
	int data_offset;
	int text_size;
	int data_size;
	int number_of_entries;
	int entry_point_rva;
	int image_base;
	int text_address_rva;
	//char* buf;
	//char* header;
	//uint DataSize;
	//uint DataStart;
public:
	PE_Reader(char* name);
	PE_Reader(PE_Reader* pe_reader);
	~PE_Reader();
	void PE_Load(char* &, int&);
	void PE_HeaderOffset();
	void PE_TableOffset();
	bool PE_FileType();
	void PE_NumberOfEntries();
	void PE_EntryPoint();
	void PE_ImageBase();
	void PE_ObjectTable();
	void PE_CodeSection(int);
	void PE_DataSection(int);
	bool PE_Compare(const char*,const char*);
	
	//void Link();
	//uint Start();
	//uint Entrance();
	//uint Map();
};

PE_Reader::PE_Reader(char* name){
	int i=0;
	filename=(char*)malloc(sizeof(char)*strlen(name));
	while(name[i]!='\0'){
		filename[i]=name[i];
		i++;
	}
	filename[i]='\0';
	header_offset=0;
	table_offset=0;
	text_offset=0;
	data_offset=0;
	text_size=0;
	data_size=0;
	number_of_entries=0;
	entry_point_rva=0;
	image_base=0;
	text_address_rva=0;
	if ((fp = fopen(filename, "r+b")) == NULL) {
		cout<<"Error: unable to open file "<<filename<<'\n'<<endl;
        exit(-1);
    }
}

PE_Reader::PE_Reader(PE_Reader* pe_reader){
	int i=0;
	free(filename);
	filename=(char*)malloc(sizeof(char)*strlen(pe_reader->filename));
	while(pe_reader->filename[i]!='\0'){
		filename[i]=pe_reader->filename[i];
		i++;
	}
	filename[i]='\0';
	fp=pe_reader->fp;
	header_offset=pe_reader->header_offset;
	table_offset=pe_reader->table_offset;
	text_offset=pe_reader->text_offset;
	data_offset=pe_reader->data_offset;
	text_size=pe_reader->text_size;
	data_size=pe_reader->data_size;
	number_of_entries=pe_reader->number_of_entries;
	entry_point_rva=pe_reader->entry_point_rva;
	image_base=pe_reader->image_base;
	text_address_rva=pe_reader->text_address_rva;
}


PE_Reader::~PE_Reader(){
	//free(filename);
	fclose(fp);
}

void PE_Reader::PE_HeaderOffset(){
	int offset=60;//offset to PE header (3C)
	int origin=0;
	fseek(fp, offset, origin);
    int header1=getc(fp);
    int header2=getc(fp);
    int header3=getc(fp);
    int header4=getc(fp);
    header_offset=header4*pow(16,6)+header3*pow(16,4)+header2*pow(16,2)+header1;
    printf("PE_HEADER_OFFSET=%d\n", header_offset);
}

void PE_Reader::PE_TableOffset(){
	table_offset=header_offset+248;
}

bool PE_Reader::PE_FileType(){
	int origin=0;
	fseek(fp, header_offset, origin);
    int format1=getc(fp);
    int format2=getc(fp);
    int format3=getc(fp);
    int format4=getc(fp);
    int format=format4*pow(16,6)+format3*pow(16,4)+format2*pow(16,2)+format1;
    printf("FORMAT=%d \n", format);
    if (format!=0x4550){
		return false;
	}
	return true;
}

void PE_Reader:: PE_NumberOfEntries(){
	int origin=0;
	int number_of_entries_offset=header_offset+6;
    fseek(fp, number_of_entries_offset, origin);
    int number_of_entries1=getc(fp);
    int number_of_entries2=getc(fp);
    number_of_entries=number_of_entries2*pow(16,2)+number_of_entries1;
    printf("Number_of_entries=%d\n", number_of_entries);
}

void PE_Reader::PE_EntryPoint(){
	int origin=0;
	int entry_point_offset=header_offset+40;
    fseek(fp, entry_point_offset, origin);
    int entry_point1=getc(fp);
    int entry_point2=getc(fp);
    int entry_point3=getc(fp);
    int entry_point4=getc(fp);
    entry_point_rva=entry_point4*pow(16,6)+entry_point3*pow(16,4)+entry_point2*pow(16,2)+entry_point1;
    printf("Entry_Point=%d\n", entry_point_rva);
}


void PE_Reader::PE_ImageBase(){
	int origin=0;
	int image_base_offset=header_offset+52;
    fseek(fp, image_base_offset, origin);
    int image_base1=getc(fp);
    int image_base2=getc(fp);
    int image_base3=getc(fp);
    int image_base4=getc(fp);
    image_base=image_base4*pow(16,6)+image_base3*pow(16,4)+image_base2*pow(16,2)+image_base1;
    printf("Image_base=%d\n", image_base);
}


void PE_Reader::PE_ObjectTable(){
	int origin=0;
	int section_offset=0;
    int base_of_section=0;
    int table_data_size=0;
    for (int i=0; i<number_of_entries; i++){
		int table_offset=header_offset+248+section_offset;
		fseek(fp, table_offset, origin);
		char section_name[9];
		for (int j=0;j<8; j++){
			section_name[j]=getc(fp);
		}
		section_name[8]='\0';
		//printf("Section_name %s\n", section_name);
		if (PE_Compare(section_name, ".text\0")||(PE_Compare(section_name, "CODE\0"))){
			printf("Section_name=%s\n", section_name);
			PE_CodeSection(table_offset);
		}
		
		if (PE_Compare(section_name, ".data\0")||(PE_Compare(section_name, "DATA\0"))){
			printf("Section_name=%s\n", section_name);
			PE_DataSection(table_offset);
			
		}
		section_offset=section_offset+40;
	}
}

void PE_Reader::PE_CodeSection(int section_offset){
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	int origin=0;
	
	//not need in static analise
	int text_address_offset=section_offset+12;
	fseek(fp, text_address_offset, origin);
	int text_address_rva1=getc(fp);
	int text_address_rva2=getc(fp);
	int text_address_rva3=getc(fp);
	int text_address_rva4=getc(fp);
	text_address_rva=text_address_rva4*pow(16,6)+text_address_rva3*pow(16,4)+text_address_rva2*pow(16,2)+text_address_rva1;
	printf("Text_address_rva=%d\n", text_address_rva);
	
	int text_size_offset=section_offset+16;
	fseek(fp, text_size_offset, origin);
	int text_size1=getc(fp);
	int text_size2=getc(fp);
	int text_size3=getc(fp);
	int text_size4=getc(fp);
	text_size=text_size4*pow(16,6)+text_size3*pow(16,4)+text_size2*pow(16,2)+text_size1;
	printf("Code_size%d\n", text_size);
			
	int table_text_offset=section_offset+20;
	fseek(fp, table_text_offset, origin);
	int base_of_text1=getc(fp);
	int base_of_text2=getc(fp);
	text_offset=base_of_text2*pow(16,2)+base_of_text1;
	printf("Code_offset=%d\n", text_offset);
	
	
}

void PE_Reader::PE_DataSection(int section_offset){
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	int origin=0;
	int data_size_offset=section_offset+16;
	fseek(fp, data_size_offset, origin);
	int data_size1=getc(fp);
	int data_size2=getc(fp);
	int data_size3=getc(fp);
	int data_size4=getc(fp);
	data_size=data_size4*pow(16,6)+data_size3*pow(16,4)+data_size2*pow(16,2)+data_size1;
	printf("Data_size%d\n", data_size);
			
	int table_data_offset=section_offset+20;
	fseek(fp, table_data_offset, origin);
	int table_base_of_data1=getc(fp);
	int table_base_of_data2=getc(fp);
	data_offset=table_base_of_data2*pow(16,2)+table_base_of_data1;
	printf("Data_offset=%d\n", data_offset);
}

bool PE_Reader::PE_Compare(const char* string1, const char* string2){
	int i=0;
	while((string1[i]!='\0')&&(string2[i]!='\0')){
		if (string1[i]!=string2[i])
			return false;
		i++;
	}
	return true;
}

void PE_Reader::PE_Load(char* &buf, int& len){
	
	PE_HeaderOffset();
	PE_TableOffset();
	PE_FileType();
	PE_NumberOfEntries();
	PE_EntryPoint();//not need in static analise
	PE_ImageBase();//not need in static analise
	PE_ObjectTable();
	int physical_entry_point=entry_point_rva-text_address_rva+text_offset;
	printf("Physical_entry_point=%d\n", physical_entry_point);
	
	int origin=0;
	fseek(fp, physical_entry_point, origin);
	//fseek(fp, text_offset, origin);
	buf=(char*)malloc(sizeof(char)*text_size);
	fread(buf, 1, 32, fp);
	//fread(buf, 1, text_size, fp);
	len=text_size;
	cout<<"End reading\n"<<endl;
	cout<<endl;
};

////////////////////////////////////////////////////////////////////////
class Emulator_LibEmu{
public:
	struct emu *e;
	struct emu_cpu *cpu;
	struct emu_memory *mem;
	Reader* reader;
	/*unsigned char* data;
	unsigned char* dataPointer;//pointer to load program
	unsigned int dataStart; //vopros->nado podumat'
	uint dataSize;
	uint base; // Max is 0x7fffffffL
	*/
	int offset;
	int kol;
	Emulator_LibEmu();
	~Emulator_LibEmu();
	void Load(char* &);
	void begin(uint);
	void jump(uint);
	bool step();
	bool get_command(char*, uint);
	unsigned int get_register(Register);
};


Reader::Reader(){
	cout<<"We are in constructor"<<endl;
	filename = "\0";
	dataSize = 0;
	dataStart = 0;
	data = NULL;
	indirect = false;
	base = 0x20000000L; // Max is 0x7fffffffL
}

Reader::Reader(const Reader *reader){
	cout<<"We are in copy constructor"<<endl;
	filename = reader->filename;
	dataSize = reader->dataSize;
	dataStart = reader->dataStart;
	data = reader->data;
	indirect = reader->indirect;
}

Reader::~Reader(){
	cout<<"We are in destructor"<<endl;
	if (indirect){
		cout<<"Deleting data"<<endl;
		delete[] data;
	}
}

string Reader::name() {
	return filename;
}

void Reader::load(string name){
	cout<<"We are in load"<<endl;
	if (indirect){
		cout<<"Inderect=true"<<endl;
	}
	cout<<name<<endl;
	filename=name;
	cout<<filename<<endl;
	getchar();
	
	//filename.clear();
	//filename=temporary;
	//cout<<filename<<endl;
	//cout<<"KUKU"<<endl;
	
	if (indirect){
		cout<<"INDIRECT TRUE"<<endl;
	}
	
	read();
	cout<<"++++++++++++++++++++"<<endl;
	//parse();

}

void Reader::link(const unsigned char *data, uint dataSize){
	cout<<"We are in link"<<endl;
	if (indirect) {
		delete[] this->data;
	}
	filename = "direct memory access";
	indirect = false;
	this->data = data;
	this->dataSize = dataSize;
	parse();
}

void Reader::read(){
	cout<<"We are in read"<<endl;
	//indirect=false;
	if (indirect) {
		cout<<"Pochemuto delete"<<endl;
		delete[] data;
	}
	data = NULL;
	dataSize = 0;
	//cout<<"Prisvoenie"<<endl;
	ifstream s(filename.c_str());
	if (!s.good() || s.eof() || !s.is_open()) {
		cerr << "Error opening file" << endl;
		exit(0);
	}
	//cout<<"Open goes good"<<endl;
	s.seekg(0, ios_base::beg);
	ifstream::pos_type begin_pos = s.tellg();
	s.seekg(0, ios_base::end);
	dataSize = static_cast<uint>(s.tellg() - begin_pos);
	s.seekg(0, ios_base::beg);
	indirect = true;
	data = new unsigned char[dataSize];
	s.read((char *) data,dataSize);
	s.close();
}

void Reader::parse()
{
	
}

const unsigned char* Reader::pointer(bool nohead) const{
	return data + (nohead ? dataStart : 0);
}

uint Reader::size(bool nohead) const{
	return dataSize - (nohead ? dataStart : 0);
}

uint Reader::start(){
	return dataStart;
}

uint Reader::entrance(){
	return base;
}

uint Reader::map(uint addr){
	
	cout<<"MAPPING"<<endl;
	cout<<base<<endl;
	return addr + base;
}

bool Reader::is_valid(uint addr){
	addr -= base;
	return (addr >= dataStart) && (addr < dataSize);
}

bool Reader::is_within_one_block(uint a, uint b){
	return is_valid(a) && is_valid(b);
}

Emulator_LibEmu::Emulator_LibEmu(){
	e = emu_new();
	cpu = emu_cpu_get(e);
	mem = emu_memory_get(e);
	reader=new Reader();
	/*data=NULL;
	dataPointer=NULL;
	dataStart=0;
	dataSize = 0;
	base = 0x20000000L;
	*/
}

Emulator_LibEmu::~Emulator_LibEmu(){
	//emu_free(e);
	//delete[] data;
	delete reader;
}


class PolyUnpack{
private:
	InstructionList* start;//start of list of instructions
	int n;//=4 start size of each buffer instruction
	//Emulator_LibEmu emulator;

public:
	PolyUnpack();
	~PolyUnpack();
	int Examine(char* string);
	void StaticAnalise(int,char*,char*,char*);
	//void LibdasmFunctions(INSTRUCTION&, char*, int, char* &);
	void AddInstruction(InstructionList* &, char* );
	void DynamicAnalise(char* &);
};

PolyUnpack::PolyUnpack(){
	start=NULL;
	n=4;
}

PolyUnpack::~PolyUnpack(){
	printf("==============================================\n");
	InstructionList* p1=start;
	InstructionList* p2=NULL;
	while(p1!=NULL){
		p2=p1->pointer;
		free(p1);
		p1=p2;
	}
}

int PolyUnpack::Examine(char* string){
	char word [4];
	int i=0;
	while(string[i]==' '){
		i++;
	}
	while((string[i]!=' ')&&(i<=2)){
		word[i]=string[i];
		i++;
	}
	word[i]='\0';
	if((word[0]=='d')&&((word[1]=='b')||(word[1]=='w')||(word[1]=='d'))&&
					(word[2]=='\0')){
		return 1;
	}
	else return 0;
}



void PolyUnpack::AddInstruction(InstructionList* &p1, char* string){
	InstructionList* p2;
	int j=0, k=n, l=0;
	while(string[j]!='\0'){
		if(j==k){
			k=k*2;
			free(p1->instruction);
			p1->instruction=(char*)malloc(sizeof(char)*k);
			for (l=0; l<j; l++)
				p1->instruction[l]=string[l];
		}
		p1->instruction[j]=string[j];
		j++;
	}
	p1->instruction[j]='\0';
	k=n;
	p2=(InstructionList*)malloc(sizeof(InstructionList));
	p2->instruction=(char*)malloc(sizeof(char*)*k);
	p2->pointer=NULL;
	p1->pointer=p2;
	p1=p2;
}


/*
 void PolyUnpack::LibdasmFunctions(INSTRUCTION& inst, char* data, int size, char* & string){
	//INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	int i=0, k=n, c = 0, bytes=8, format = FORMAT_INTEL, len=0;
	while (c < size) {
		inst_pointer=&inst;
		len=get_instruction(inst_pointer, (BYTE*)(data + c), MODE_32);

		// Illegal opcode or opcode longer than remaining buffer
		if (!len || (len + c > size)){
			printf("%.8x  ", c);
			if (bytes) {
				printf("%.2x  ", data[c]);
				for (i = 1; i < bytes*2 - 1; i++)
					printf(" ");
			}
			if (format == FORMAT_INTEL)
				printf("db 0x%.2x\n", *(data + c));
			else
				printf(".byte 0x%.2x\n", *(data + c));
			c++;
			continue;
		}

		printf("%.8x  ", c);
		if (bytes) {
			for (i = 0; i < ((bytes) < (len) ? (bytes) : (len)); i++)
				printf("%.2x", data[c + i]);
			printf("  ");
			for (i = (bytes) < ((len) ? (bytes) : (len)); i < bytes*2 - len; i++)
				printf(" ");
		}
		inst_pointer=&inst;
		get_instruction_string(inst_pointer, Format(format), (DWORD)c, string, sizeof(string));
		
		printf("%s\n", string);

		c=c+len;
	}
}
*/


void PolyUnpack::StaticAnalise(int Argc, char* FileName, char* Argv2=NULL, char* Argv3=NULL){
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	InstructionList* p1=NULL; 
	char *data=NULL;
	int i=0, k=n, c = 0, bytes=8, format = FORMAT_INTEL, size=0, len=0;
	char string[256];
	PE_Reader reader(FileName);
	reader.PE_Load(data, size);
	if (size!=0){
		start=(InstructionList*)malloc(sizeof(InstructionList));
		p1=start;
		p1->instruction=(char*)malloc(sizeof(char*)*k);
		p1->pointer=NULL;
	}
	else{
		printf("There are no code section in this file. Don't know what to do!\n");
		exit(-1);
	}
	if (Argc > 2) {
		if (Argv2[0] == '-') {
			switch(Argv2[1]) {
				case 'a':
					format = FORMAT_ATT;
					break;
				case 'i':
					format = FORMAT_INTEL;
					break;
			}
			if (Argc > 3)
				bytes = atoi(Argv3);
		} 
		else
			bytes = atoi(Argv2);
	} 
	cout<<"************************************"<<endl;
	cout<<data<<endl;
	cout<<"************************************"<<endl;
	while (c < size) {
		/*
		 * get_instruction() has the following parameters:
		 *
		 * - &inst is a pointer to struct INSTRUCTION
		 * - data + c is pointer to data to be disassembled
		 * - disassemble in 32-bit mode: MODE_32 
		 */
		inst_pointer=&inst;
		len=get_instruction(inst_pointer, (BYTE*)(data + c), MODE_32);

		// Illegal opcode or opcode longer than remaining buffer
		if (!len || (len + c > size)){
			if  (!len)
				printf("Illegal opcode\n");
			else 
				printf("Else brunch\n");
			printf("%.8x  ", c);
			if (bytes){
				printf("%.2x  ", data[c]);
				for (i = 1; i < bytes*2 - 1; i++)
					printf(" ");
			}
			if (format == FORMAT_INTEL)
				printf("db 0x%.2x\n", *(data + c));
			else
				printf(".byte 0x%.2x\n", *(data + c));
			c++;
			continue;
		}

		/*
		 * Print absolute offset and raw data bytes up to 'bytes'
		 * (not needed, but looks nice).
		 *
		 */
		printf("%.8x  ", c);
		if (bytes){
			for (i = 0; i < ((bytes) < (len) ? (bytes) : (len)); i++)
				printf("%.2x", data[c + i]);
			printf("  ");
			for (i = (bytes) < ((len) ? (bytes) : (len)); i < bytes*2 - len; i++)
				printf(" ");
		}
		/*
		 * Print the parsed instruction, format using user-supplied
		 * format. We could of course format the instruction in some
		 * other way by accessing struct INSTRUCTION members directly.
		 */
		
		inst_pointer=&inst;
		get_instruction_string(inst_pointer, Format(format), (DWORD)c, string, sizeof(string));
		printf("%s\n", string);
		
		AddInstruction(p1,string);
		c=c+len;
	} 
	
	p1->pointer=NULL;
	printf("+++++++++++++++++++++++++++++++++++++++++++++\n");
	p1=start;
	int kol_instructions=0;
	while(p1!=NULL){
		printf("%s\n",p1->instruction);
		p1=p1->pointer;
		kol_instructions++;
	}
	cout<<"KOL_INSTRUCTIONS="<<kol_instructions<<endl;
}

void Emulator_LibEmu::Load(char* &FileName){
	string filename(FileName);
	cout<<"QWERTY"<<endl;
	cout<<filename<<endl;
	reader->load(filename);
}

void Emulator_LibEmu::begin(uint pos){
	if (pos==0) {
		pos = reader->start();
	}
	cout<<"======================="<<endl;
	cout<<pos<<endl;
	offset = reader->map(pos);
	cout<<"======================="<<endl;
	offset=offset - pos;
	//offset-=1;

	uint start = max((int) reader->start(), (int) pos - mem_before), end = min(reader->size(), pos + mem_after);
	
	for (int i=0; i<8; i++) {
		emu_cpu_reg32_set(cpu, (emu_reg32) i, 0);
	}
	emu_cpu_reg32_set(cpu, esp, 0x1000000);

	emu_memory_clear(mem);
	//emu_memory_write_block(mem, offset + reader->start(), reader->pointer(true), reader->size(true));
	emu_memory_write_block(mem, (offset + start), (void*)(reader->pointer() + start), (end - start));

	jump(pos);
}


void Emulator_LibEmu::jump(uint pos) {
	emu_cpu_eip_set(cpu, offset + pos);
}

bool Emulator_LibEmu::step() {
	if (emu_cpu_parse(cpu) != 0) {
		return false;
	}
	if (emu_cpu_step(cpu) != 0) {
		return false;
	}
	kol++;
	return true;
}

bool Emulator_LibEmu::get_command(char *buff, uint size) {
	emu_memory_read_block(mem, emu_cpu_eip_get(cpu), buff, size);
	cout<<buff<<endl;
	return true;
}

unsigned int Emulator_LibEmu::get_register(Register reg) {
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

void PolyUnpack::DynamicAnalise(char* & FileName){
	//char* rawdata=(char*)malloc(sizeof(char)*2000);
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	int c = 0, bytes=8, format = FORMAT_INTEL, len=0;
	char strings[256];
	string rawdata;
	int command_size=10;//default was 10!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	char* command=(char*)malloc(sizeof(char)*command_size);
	for (int i=0; i<command_size; i++){
		command[i]='\0';
	}
	Register reg=EIP;
	//emulator.kol=0;
	printf("Hello\n");
	//emulator=Emulator_LibEmu();
	Emulator_LibEmu emulator;
	cout<<"Load Start"<<endl;
	getchar();
	emulator.Load(FileName);
	printf("Start begin\n");
	emulator.begin(0);
	printf("End begin\n");
	//printf("%s\n", emulator.dataPointer);
	int i=0, j=0, k=0, m=0;
	
	for(i=0;i<1000;i++){
		//printf("Start step\n");
		//printf("End step\n");
		if(emulator.step()){
			//getchar();
			//printf("EIP=%d\n", emulator.get_register(reg));
			//printf("Start get_command\n");
			emulator.get_command(command, command_size);
			//printf("End get_command\n");
			inst_pointer=&inst;
			len=get_instruction(inst_pointer, (BYTE*)(command), MODE_32);
			inst_pointer=&inst;
			get_instruction_string(inst_pointer, Format(format), (DWORD)c, strings, sizeof(strings));
			cout<<strings<<endl;
			/*
			for (int i=0; i<command_size; i++){
				command[i]='\0';
			}*/
			rawdata+=command;
			/*
			 m=0;
			for (j=k;k<command_size; k++){
				rawdata(command[m];
				m++;
			}
			k=k+command_size;
			*/
		}
	}
	cout<<"***************"<<endl;
	//cout<<rawdata<<endl;
	cout<<"***************"<<endl;
	
	/*
	char* data=(char*)malloc(sizeof(char)*rawdata.size());
	int size=rawdata.size();
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	int c = 0, bytes=8, format = FORMAT_INTEL, len=0;
	char string[256];
	k=n;
	while (c < size) {

		inst_pointer=&inst;
		len=get_instruction(inst_pointer, (BYTE*)(data + c), MODE_32);

		// Illegal opcode or opcode longer than remaining buffer
		if (!len || (len + c > size)){
			printf("%.8x  ", c);
			if (bytes) {
				printf("%.2x  ", data[c]);
				for (i = 1; i < bytes*2 - 1; i++)
					printf(" ");
			}
			if (format == FORMAT_INTEL)
				printf("db 0x%.2x\n", *(data + c));
			else
				printf(".byte 0x%.2x\n", *(data + c));
			c++;
			continue;
		}
		
		printf("%.8x  ", c);
		if (bytes) {
			for (i = 0; i < ((bytes) < (len) ? (bytes) : (len)); i++)
				printf("%.2x", data[c + i]);
			printf("  ");
			for (i = (bytes) < ((len) ? (bytes) : (len)); i < bytes*2 - len; i++)
				printf(" ");
		}
		
		inst_pointer=&inst;
		get_instruction_string(inst_pointer, Format(format), (DWORD)c, string, sizeof(string));
		printf("%s\n", string);
		c=c+len;
	} 
	*/
	//cout<<rawdata<<endl;
	
	//printf("Kol=%d\n", emulator.kol);
	
}

int main(int argc, char** argv){
	try{
		if (argc<2){
			throw argv[0];
		}
		PolyUnpack Unpack;
		Unpack.StaticAnalise(argc, argv[1], argv[2], argv[3]);
		Unpack.DynamicAnalise(argv[1]);
	}
	catch(char* string1){
		printf("Usage: %s <file> [-a|-i] [bytes]\n"
		       "  file    file to be disassembled (required)\n"
		       "  -a      format: ATT (optional)\n"
		       "  -i      format: INTEL (optional, default)\n"
	 	       "  bytes   show raw instruction data (optional, default 8)\n\n",
			string1);
	}
	catch(...){
		printf("Something strange caught\n");
	}
	return 0;
}
