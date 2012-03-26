#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <math.h>
#include <errno.h>
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
	int number;
	int length;
	bool first;
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
	//char* buf;
	//int len;
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
	void PE_LoadForStaticAnalise(char* &, int&, int&);
	void PE_LoadForDynamicAnalise(char* &, int&, int&, int&, int&);
	void PE_Read(char* &, int&, int&);
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
	//void Load(char* &);
	void begin(char* &, int &, int &, int);
	void jump(uint);
	bool step();
	bool get_command(char*, uint);
	unsigned int get_register(Register);
	unsigned int get_register(char* &);
};

///////////////////////////////////////////////////////
PE_Reader::PE_Reader(char* name){
	int i=0;
	filename=(char*)malloc(sizeof(char)*strlen(name));
	while(name[i]!='\0'){
		filename[i]=name[i];
		i++;
	}
	filename[i]='\0';
	//buf=NULL;
	//len=0;
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
	if ((fp = fopen(filename, "r+b")) == NULL){
		perror(filename);
		//cout<<"Error: unable to open file: "<<filename<<'\n'<<endl;
        cout<<"This Error may be because of the symbols in file name!"<<endl;
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
	//len=pe_reader->len;
	//buf=(char*)malloc(sizeof(char)*len);
	//i=0;
	//while(i<len){
	//	buf[i]=pe_reader->buf[i];
	//	i++;
	//}
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
/*
void PE_Reader::PE_Read(char* &buf, int &len, int &offset){
	int origin=0;
	fseek(fp, offset, origin);
	buf=(char*)malloc(sizeof(char)*len);
	fread(buf, 1, text_size, fp);
	//len=text_size;
	cout<<"End reading\n"<<endl;
	cout<<endl;
}
*/
void PE_Reader::PE_LoadForStaticAnalise(char* &buf, int& len, int& start){	
	PE_HeaderOffset();
	PE_TableOffset();
	PE_FileType();
	PE_NumberOfEntries();
	PE_EntryPoint();//not need in static analise
	//PE_ImageBase();//not need in static analise
	PE_ObjectTable();
	int physical_entry_point=entry_point_rva-text_address_rva+text_offset;
	start=entry_point_rva-text_address_rva;
	printf("Entry_point_rva=%d\n", entry_point_rva);
	printf("Text_address_rva=%d\n", text_address_rva);
	printf("Text_offset=%d\n", text_offset);
	printf("Physical_entry_point!!!=%d\n", physical_entry_point);
	
	//int entry_point=physical_entry_point-text_offset;
	printf("Entry_point %d\n", start);
	len=text_size;
	//PE_Read(buf, text_size, text_offset);
	int origin=0;
	//fseek(fp, 0, origin);
	//fseek(fp, physical_entry_point, origin);
	fseek(fp, text_offset, origin);//need to be this
	
	buf=(char*)malloc(sizeof(char)*text_size);
	if (buf==NULL){
		printf("Memory error  in static analise\n"); 
		exit (2);
	}
	fread(buf, 1, text_size, fp);
	cout<<"End reading\n"<<endl;
	cout<<endl;
};

void PE_Reader::PE_LoadForDynamicAnalise(char* &buf, int& len, int & start, int & offset, int & base){	
	//free(buf);// not need here
	PE_HeaderOffset();
	PE_TableOffset();
	PE_FileType();
	PE_NumberOfEntries();
	PE_EntryPoint();//not need in static analise
	PE_ImageBase();//not need in static analise
	base=image_base;
	PE_ObjectTable();
	start=entry_point_rva-text_address_rva+text_offset;
	offset=text_offset;
	//start=text_offset;
	//start=start-2;
	printf("Physical_entry_point!!!=%d\n", start);

	int file_size;
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	len=file_size;
	buf=(char*)malloc(sizeof(char)*(len)); 
	if (buf==NULL){
		printf("Memory error  in dynamic analise\n"); 
		exit (2);
	}
	fseek(fp, 0, 0);
	int result = fread(buf, 1, file_size, fp);
	if (result!=file_size){
		printf("Error  in reading in dynamic analise\n"); 
		exit (2);
	}
	printf("FILE_SIZE=%d\n", file_size);
	printf("End Reading\n");
	//fseek(fin, 0, SEEK_SET);
};


////////////////////////////////////////////////////////////////////////
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
	emu_free(e);
	//delete[] data;
	delete reader;
}

struct List_of_Return_Addresses{
	int address;
	List_of_Return_Addresses* pointer;
};
 
class Stack{
	List_of_Return_Addresses *start;
public:
	Stack(){start=NULL;};
	void Push(int);
	void Pop(int &);
};

void Stack::Push(int a){
	List_of_Return_Addresses* p=start;
	if (start==NULL){
		start=(List_of_Return_Addresses*)malloc(sizeof(List_of_Return_Addresses));
		start->address=a;
		start->pointer=NULL;
		return;
	}
	while(p->pointer!=NULL){
		p=p->pointer;
	}
	p=p->pointer;
	p=(List_of_Return_Addresses*)malloc(sizeof(List_of_Return_Addresses));
	p->address=a;
	p->pointer=NULL;
	return;
}

void Stack::Pop(int & a){
	List_of_Return_Addresses* p=start;
	List_of_Return_Addresses* p2=p;
	int kol=0;
	if (start==NULL){
		printf("Stack is empty! Don't know where to go!\n");
		exit(33);
		return; 
	}
	while(p->pointer!=NULL){
		p2=p;
		p=p->pointer;
		kol++;
	}
	a=p->address;
	free(p);
	if (kol==0)
		start=NULL;
	else
		p2->pointer=NULL;
	return;
}

class PolyUnpack{
private:
	InstructionList* start;//start of list of instructions
	int n;//=4 start size of each buffer instruction
	//Emulator_LibEmu emulator;
	Stack return_addresses;
	int prev_instr_len;
public:
	PolyUnpack();
	~PolyUnpack();
	void AddInstruction(InstructionList* &, char*,  int, int, bool );
	bool FindFirstInstruction(InstructionList* &);
	void FindInstruction(InstructionList* &, int);
	void StaticAnalise(char*, int);
	void DynamicAnalise(char*, int);
	int Compare(Emulator_LibEmu &, InstructionList* &, char* &, int, int); 
												   //1 is true 
												   //-1 is false
												   //0 is unknown in case of jmp
												   //2 when instruction call was found
												   //3 when instruction ret, retn was found
};

PolyUnpack::PolyUnpack(){
	start=NULL;
	n=4;
	prev_instr_len=0;
}

PolyUnpack::~PolyUnpack(){
	InstructionList* p1=start;
	InstructionList* p2=NULL;
	while(p1!=NULL){
		p2=p1->pointer;
		free(p1);
		p1=p2;
	}
}

void PolyUnpack::AddInstruction(InstructionList* &p1, char* string, int num, int length, bool flag){
	int j=0, k=n, l=0;
	if (start==NULL){
		printf("We are here!!!\n");
		start=(InstructionList*)malloc(sizeof(InstructionList));
		p1=start;
	}
	else{
		p1->pointer=(InstructionList*)malloc(sizeof(InstructionList));
		p1=p1->pointer;
	}
	p1->instruction=(char*)malloc(sizeof(char*)*k);
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
	p1->number=num;
	p1->length=length;
	p1->first=flag;
	p1->pointer=NULL;
}

void PolyUnpack::StaticAnalise(char* FileName, int key=0){
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	InstructionList* p1=NULL; 
	char *data=NULL;
	int i=0, k=n, c = 0, bytes=8, size=0, len=0, number=0;
	char string[256];
	PE_Reader reader(FileName);
	int entry_point=0;
	bool flag=false;
	reader.PE_LoadForStaticAnalise(data, size, entry_point);
	printf("Static analize entry=%d\n", entry_point);
	if(size==0){
		printf("There are no code section in this file. Don't know what to do!\n");
		exit(-1);
	}
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
			if (key==1){
				if  (!len)
					printf("Illegal opcode\n");
				else 
					printf("Else brunch\n");
				printf("%.8x  ", c);
			
				printf("db 0x%.2x\n", *(data + c));
			}
			c++;
			continue;
		}
		if ((bytes)&&(key==1)){
			for (i = 0; i < ((bytes) < (len) ? (bytes) : (len)); i++)
				printf("%.2x", data[c + i]);
			printf("  ");
			for (i = (bytes) < ((len) ? (bytes) : (len)); i < bytes*2 - len; i++)
				printf(" ");
		}
		
		/*
		 * Print absolute offset and raw data bytes up to 'bytes'
		 * (not needed, but looks nice).
		 *
		 */
		/*
		 * Print the parsed instruction, format using user-supplied
		 * format. We could of course format the instruction in some
		 * other way by accessing struct INSTRUCTION members directly.
		 */
		
		inst_pointer=&inst;
		get_instruction_string(inst_pointer, Format(FORMAT_INTEL), (DWORD)c, string, sizeof(string));
		
		if (key==1){
			printf("%.8x  ", c);
			printf("%s\n", string);//need to be here!
		}
		if((c+len)==entry_point)
			prev_instr_len=len;
		if (c==entry_point){
			printf("Len=%d\n", len);
			flag=true;
		}
		else
			flag=false;
		AddInstruction(p1, string, c , len, flag);
		c=c+len;
	} 	
	p1->pointer=NULL;
	printf("+++++++++++++++++++++++++++++++++++++++++++++\n");
	/*
	p1=start;
	int kol_instructions=0;
	while(p1!=NULL){
		printf("%0x    ", p1->number);
		printf("%s\n",p1->instruction);
		p1=p1->pointer;
		kol_instructions++;
	}
	*/
	//cout<<"KOL_INSTRUCTIONS="<<kol_instructions<<endl;
}

void Emulator_LibEmu::begin(char* & buf, int & len, int & entry_point, int prev_instr_len){
	
	for (int i=0; i<8; i++) {
		emu_cpu_reg32_set(cpu, (emu_reg32) i, 0);
	}
	
	emu_cpu_reg32_set(cpu, esp, 0x1000000);

	emu_memory_clear(mem);
	//emu_memory_write_block(mem, offset + reader->start(), reader->pointer(true), reader->size(true));
	//emu_memory_write_block(mem, (offset + start), (void*)(reader->pointer() + start), (end - start));//->this was here
	emu_memory_write_block(mem, 0x20000000L, (void*)buf, len);
	/*int i=0;
	for (i=0; i<len; i++){
		emu_memory_write_byte(mem, 0x20000000L+i, buf[i]);
	}
	emu_memory_write_byte(mem, 0x20000000L+i, '\xcc');
	*/
	emu_memory_write_byte(mem, 0x20000000L+len, '\xcc');
	jump(entry_point);
	printf("ENTRY_POINT!!!!!=%d\n", entry_point);
	printf("END begin\n");
}


void Emulator_LibEmu::jump(uint pos){
	emu_cpu_eip_set(cpu, 0x20000000L+pos);
}

bool Emulator_LibEmu::step() {
	if (emu_cpu_parse(cpu) != 0) {
		printf("Bad parse\n");
		return false;
	}
	if (emu_cpu_step(cpu) != 0) {
		printf("Bad Step\n");
		kol++;
		return false;
	}
	//kol++;
	return true;
}

bool Emulator_LibEmu::get_command(char *buff, uint size) {
	emu_memory_read_block(mem, emu_cpu_eip_get(cpu), buff, size);
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

unsigned int Emulator_LibEmu::get_register(char* &string){
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
	if (strcmp(string,"esi")==0){
		printf("Register ESI was found\n");
		return emu_cpu_reg32_get(cpu, esi);
	}
	if (strcmp(string,"edi")==0)
		return emu_cpu_reg32_get(cpu, edi);
	if (strcmp(string,"eax")==0)
		return emu_cpu_reg32_get(cpu, eax);
	if (strcmp(string,"ebx")==0){
		int num=emu_cpu_reg32_get(cpu, ebx);
		printf("EBX=%d\n", num);
		return emu_cpu_reg32_get(cpu, ebx);
	}
	if (strcmp(string,"ecx")==0)
		return emu_cpu_reg32_get(cpu, ecx);
	if (strcmp(string,"edx")==0)
		return emu_cpu_reg32_get(cpu, edx);
	return 0;
}

bool PolyUnpack::FindFirstInstruction(InstructionList* & p){
	InstructionList* point=start;
	while((!point->first)){
		point=point->pointer;
		if (point==NULL)
			break;
	}
	if (point==NULL)
		return false;
	else{
		p=point;
		return true;
	}
} 

void PolyUnpack::DynamicAnalise(char* FileName, int key=0){
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	InstructionList* p=start;
	bool bad_parse=true;
	int i=0;
	int k=n, c = 0, bytes=8, size=0, len=0, flag=1;
	char* string;
	int string_size=256;
	string=(char*)malloc(sizeof(char)*string_size);
	int raw_data_length=256;
	int position=0;
	char* raw_data=(char*)malloc(raw_data_length);
	char* raw_data2=NULL;
	int command_size=10;//default was 10!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	char* command=(char*)malloc(sizeof(char)*command_size);
	for (int i=0; i<command_size; i++){
		command[i]='\0';
	}
	Register reg=EIP;
	Emulator_LibEmu emulator;
	cout<<"Load Start"<<endl;
	getchar();
	char* buf;
	int buf_size=0;
	int entry_point=0;
	getchar();
	int ret_num=0;
	PE_Reader reader(FileName);
	int offset=0;
	int base=0;
	reader.PE_LoadForDynamicAnalise(buf, buf_size, entry_point, offset, base);
	printf("base=%d\n", base);
	getchar();
	
	emulator.begin(buf, buf_size, entry_point, prev_instr_len);
	if(!FindFirstInstruction(p)){
		printf("Can't find First instruction\n");
		exit(33);
	}
	for (i=0; i<raw_data_length; i++){
		raw_data[i]='\0';
	}
	for(int kol=0;kol<500;kol++){
		
		emulator.get_command(command, command_size);			
		
		printf("Begin step\n");
		int old_eip_pointer=emulator.get_register(reg);
		printf("EIP before=%d\n", old_eip_pointer);
		bad_parse=emulator.step(); 
		printf("EIP after=%d\n", emulator.get_register(reg));
		printf("End step\n");
		
		inst_pointer=&inst;
		len=get_instruction(inst_pointer, (BYTE*)(command), MODE_32);
		if (!len || (len + c > command_size)){
			if (key==1){
				if (!len)
					printf("Illegal opcode\n");
				else 
					printf("Else brunch\n");
				printf("%.8x  ", c);
				printf("db 0x%.2x\n", *(command + c));
			}
			c++;
			continue;
		}
		if ((bytes)&&(key==1)){
			for (i = 0; i < ((bytes) < (len) ? (bytes) : (len)); i++){
				printf("%.2x", command[c + i]);
			}
			printf("  ");
			for (i = (bytes) < ((len) ? (bytes) : (len)); i < bytes*2 - len; i++)
				printf(" ");
			}
		inst_pointer=&inst;
		get_instruction_string(inst_pointer, Format(FORMAT_INTEL), (DWORD)c, string, string_size);
		if (key==1){
			printf("%.8x  ", c);
			printf("%s\n", string);
			printf("Step Number=%d\n", kol);
		}
		flag=Compare(emulator, p, string, flag, offset);
		if(flag==-1){
			printf("THIS IS VIRUS\n");
			exit(101);
		}
		if(!bad_parse)
			emulator.jump(old_eip_pointer+len-0x20000000L);
		for (int i=0; i<string_size; i++)
			string[i]='\0';
		for (int i=0; i<command_size; i++)
			command[i]='\0';
	}
	free (string);
	cout<<"***************"<<endl;
	cout<<"This file is valid"<<endl;
	cout<<"***************"<<endl;
}

void PolyUnpack::FindInstruction(InstructionList* & p, int number){
	InstructionList* point=start;
	while((point!=NULL)&&(point->number!=number)){
		point=point->pointer;
	}
	p=point;
	if(point==NULL){
		printf("Number=%d\n", number);
		printf("Instruction not found!\n May be because of unvalid jump.\n");
		exit(1);
	}
}

bool compare(char* string1, const char* string2){
	int i=0;
	while((string1[i]!='\0')&&(string2[i]!='\0')){
		if (string1[i]!=string2[i])
			return false;
		i++;
	}
	return true;
}

int GetNumber(char* string){
	int koef=16;
	int number=0;
	int i=0;
	while(string[i]!='\0'){
		if ((string[i]>='0')&&(string[i]<='9')){
			number=number*koef+(string[i]-'0');
		}
		else{
			number=number*koef+(string[i]-'a'+10);
		}
		i++;
	}
	return number;
}

int PolyUnpack::Compare(Emulator_LibEmu & emulator, InstructionList* &p, char* & command, int flag, int offset){
	int command_number=0; 
	if (flag==0){
		printf("We are here!!! flag=0\n");
		if (strcmp(p->pointer->instruction, command)==0){
			p=p->pointer;
			printf(" INSTRUCTION=%s\n", p->instruction);
			printf(" COMMAND=%s\n", command);
			//p=p->pointer;
			//return 1;
		} 
		else{
			printf("Else brunch\n");
			flag=Compare(emulator, p->pointer, command, 99, offset);
			if (flag==-1){
				int i=0;
				while(p->instruction[i]!=' ')
					i++;
				while(p->instruction[i]==' ')
					i++;
				int number=GetNumber(p->instruction+i+2);
				FindInstruction(p, number);
			}
			if ((flag==0)||(flag==2)||(flag==3)){
				p=p->pointer;
			}
		}
	}
	printf(" INSTRUCTION=%s\n", p->instruction);
	printf(" COMMAND=%s\n", command);
	if ((compare(p->instruction,"jz"))||(compare(p->instruction,"jnz"))||
		(compare(p->instruction,"jns"))||(compare(p->instruction,"jmp"))||
		(compare(p->instruction,"ja"))||(compare(p->instruction,"jna"))||
		(compare(p->instruction,"jnc"))||(compare(p->instruction,"jc"))||
		(compare(p->instruction,"jl"))||(compare(p->instruction,"jnl"))||
		(compare(p->instruction,"js"))||(compare(p->instruction,"jns"))||
		(compare(p->instruction,"jg"))||(compare(p->instruction,"jng"))){
		if (((compare(p->instruction,"jz"))&&(!compare(command,"jz")))||
			((compare(p->instruction,"jnz"))&&(!compare(command,"jnz")))||
			((compare(p->instruction,"jns"))&&(!compare(command,"jns")))||
			((compare(p->instruction,"jmp"))&&(!compare(command,"jmp")))||
			((compare(p->instruction,"ja"))&&(!compare(command,"ja")))||
			((compare(p->instruction,"jna"))&&(!compare(command,"jna")))||
			((compare(p->instruction,"jc"))&&(!compare(command,"jc")))||
			((compare(p->instruction,"jnc"))&&(!compare(command,"jnc")))||
			((compare(p->instruction,"jl"))&&(!compare(command,"jl")))||
			((compare(p->instruction,"js"))&&(!compare(command,"js")))||
			((compare(p->instruction,"jns"))&&(!compare(command,"jns")))||
			((compare(p->instruction,"jg"))&&(!compare(command,"jg")))||
			((compare(p->instruction,"jng"))&&(!compare(command,"jng")))){
			return -1;
		}	
		int i=0;
		while(p->instruction[i]!=' ')
			i++;
		while(p->instruction[i]==' ')
			i++;
		if (p->instruction[i]=='['){
			p=p->pointer;
			return 1;
		}
		else{
			return 0;
		}
	}
	if (compare(p->instruction,"call")){
		printf("We are in call section\n");
		if (!compare (command, "call")){
			return -1;
		}
		int i=0;
		while(p->instruction[i]!=' ')
			i++;
		while(p->instruction[i]==' ')
			i++;
		if (p->instruction[i]=='['){
			p=p->pointer;
			return 1;
		}
		else{
			if((p->pointer!=NULL)&&(flag!=99)){//
				command_number=p->pointer->number;
				return_addresses.Push(command_number);
				printf("Future EIP=%d\n", command_number+offset+0x20000000L);
			}
			else{
				if(flag==99)
					return 2;
				printf("p->pointer==NULL\n");
				//exit(1);
			}
			if ((p->instruction[i])=='0'){
				int number=GetNumber(p->instruction+i+2);	
				FindInstruction(p, number);
			}
			else{
				int n=4;
				int j=0;
				char* string=(char*)malloc(sizeof(char)*4);
				while(p->instruction[i]!='\0'){
					if (j<n){
						string[j]=p->instruction[i];
					}
					else{
						n=n*2;
						char* string2=(char*)malloc(sizeof(char)*n);
						int k=0;
						while(k<j){
							string2[k]=string[k];
							k++;
						}
						free(string);
						string=string2;
					}
					j++;
					i++;	
				}
				string[j]='\0';
				int num=emulator.get_register(string);
				printf("Register was found!!! ESI=%d\n", num);
				FindInstruction(p, num);
				printf("%s\n", p->instruction);
				free(string);
			}
			return 2;
		}
	}
	else{
		printf("WE ARE HERE\n");
		if (strcmp(p->instruction, command)==0){
			if (compare(p->instruction ,"ret")||compare(p->instruction,"retn")){
				int temp;
				if (flag!=99){
					return_addresses.Pop(temp);
					printf("Instruction Ret was found EIP=%d\n", temp+offset+0x20000000L);
					FindInstruction(p, temp);
					emulator.jump(temp+offset);
				}
				return 3;
			}
			p=p->pointer;
			return 1;
		}
		else
			return -1;
	}
	printf("SOMETHING STRANGE COMMAND\n");
	exit(99);
}

int main(int argc, char** argv){
	try{
		PolyUnpack Unpack;
		if ((argc<2)||(argc>4)){
			throw argv[0];
		}
		if (argc==2){
			Unpack.StaticAnalise(argv[1]);
			Unpack.DynamicAnalise(argv[1]);
		}
		if (argc==3){
			if((strcmp(argv[1],"-s")!=0)&&(strcmp(argv[1],"--static")!=0)&&
		      (strcmp(argv[1],"-d")!=0)&&(strcmp(argv[1],"--dynamic")!=0)&& 
		      (strcmp(argv[1],"-sd")!=0)&&(strcmp(argv[1],"-ds")!=0)){
				throw argv[0];
			}
			else{
				if ((strcmp(argv[1],"-s")==0)||(strcmp(argv[1],"--static")==0)){
					Unpack.StaticAnalise(argv[2], 1);
					Unpack.DynamicAnalise(argv[2]);
				}
				if ((strcmp(argv[1],"-sd")==0)||(strcmp(argv[1],"-ds")==0)){
					Unpack.StaticAnalise(argv[2],1);
					Unpack.DynamicAnalise(argv[2],1);
				}
				if((strcmp(argv[1],"-d")==0)||(strcmp(argv[1],"--dynamic")==0)){ 
					Unpack.StaticAnalise(argv[2]);
					Unpack.DynamicAnalise(argv[2],1);
				}
			}
			
		}
		if (argc==4){
			printf("We are here!\n");
			if((strcmp(argv[1],"-s")!=0)&&(strcmp(argv[1],"--static")!=0)&&
		      (strcmp(argv[1],"-d")!=0)&&(strcmp(argv[1],"--dynamic")!=0)&&
		      (strcmp(argv[2],"-s")!=0)&&(strcmp(argv[2],"-d")!=0)&&
		      (strcmp(argv[2],"--static")!=0)&&(strcmp(argv[2],"--dynamic")!=0)){
				throw argv[0];
			}
			Unpack.StaticAnalise(argv[3], 1);
			Unpack.DynamicAnalise(argv[3], 1);	
		}
	}
	catch(char* string1){
		printf("Usage: [-s|--static] [-d|--dynamic] %s <file>\n"
		       "  -s      show static disassemble of file in INTEL format\n"
		       "  -d      show dynamic execution\n"
	 	       "  file    file to be disassembled (required)\n",
			string1);
	}
	catch(...){
		printf("Something strange caught\n");
	}
	return 0;
}
