#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Headers/InstructionList.h"
#include "Headers/Instruction_Leaf.h"
#include "Headers/Instruction_Tree.h"
#include "Headers/PE_Reader.h"
#include "Headers/Stack.h"
#include "Headers/Emulator_LibEmu.h"
#include "Headers/PolyUnpack.h"
extern "C"{
	#include "libdasm/libdasm.h"
	#include <emu/emu.h>
	#include <emu/emu_cpu.h>
	#include <emu/emu_memory.h>
}

using namespace std;
/*
const int mem_before = 10*1024; // 10 KiB, min 1k instuctions
const int mem_after = 80*1024; //80 KiB, min 8k instructions
*/

void Instruction_Tree:: Add(InstructionList* & po1, InstructionList* & po2, InstructionList* & po3){
	Instruction_Leaf* p;
	if (root==NULL){
		root=new Instruction_Leaf;
		root->pointer=po1;
		root->valid=1;
		if (po2!=NULL){
			root->left=new Instruction_Leaf;
			p=root->left;
			p->pointer=po2;
			p->valid=0;
			p->left=NULL;
			p->right=NULL;
		}
		else{
			root->left=NULL;
		}
		if (po3!=NULL){
			root->right=new Instruction_Leaf;
			p=root->right;
			p->pointer=po3;
			p->valid=0;
			p->left=NULL;
			p->right=NULL;
		}
		else{
			root->right=NULL;
		}
	}
}
/*
void Instruction_Tree::Add_Leaf(Instruction_Leaf* & p1, int depth, 
								InstructionList* & po1, 
								InstructionList* & po2, 
								InstructionList* & po3){
	Instruction_Leaf* p2;
	depth++;
	if ((p1->left!=NULL)&&(p1->right!=NULL)){
		Add_Leaf(p1->left, depth, po1, po2, po3);
		Add_Leaf(p1->right, depth, po1, po2, po3);
		return;
	}
	if ((p1->left!=NULL)&&(p1->right==NULL)){
		Add_Leaf(p1->left, depth, po1, po2, po3);
		return;
	}
	if ((p1->left==NULL)&&(p1->right==NULL)){
		printf("Adding Leaves\n");
		printf("Instruction=%s\n", p1->pointer->instruction);
		if (po1!=NULL)
			printf("New_instruction1=%s\n", po1->instruction);
		if (po2!=NULL)
			printf("New_instruction2=%s\n", po2->instruction);
		if (po3!=NULL)
			printf("New_instruction3=%s\n", po3->instruction);

		//p1->left=(Instruction_Leaf*)malloc(sizeof(Instruction_Leaf*));
		p1->left=new Instruction_Leaf;
		p1=p1->left;
		p1->pointer=po1;
		p1->depth=depth;
		p1->left=NULL;
		p1->right=NULL;
		if ((po2!=NULL)&&(po3!=NULL)){
			depth++;
			p2=p1;
			//p1->left=(Instruction_Leaf*)malloc(sizeof(Instruction_Leaf*));
			p1->left=new Instruction_Leaf;
			p1=p1->left;
			p1->pointer=po2;
			p1->depth=depth;
			p1->valid=0;
			p1->left=NULL;
			p1->right=NULL;
			p1=p2;
			//p1->right=(Instruction_Leaf*)malloc(sizeof(Instruction_Leaf*));
			p1->right=new Instruction_Leaf;
			p1=p1->right;
			p1->pointer=po3;
			p1->depth=depth;
			p1->valid=0;
			p1->left=NULL;
			p1->right=NULL;
		}
		return;
	}
}
*/
/*
void Instruction_Tree::Add_Leaf(Instruction_Leaf* & p1, int depth, 
								InstructionList* & po1, 
								InstructionList* & po2){
	if (p1==NULL)
		throw "p1=NULL";
	Instruction_Leaf* p2=p1;
	p1->left=new Instruction_Leaf;
	p1=p1->left;
	p1->pointer=po1;
	p1->valid=0;
	p1->left=NULL;
	p1->right=NULL;
	if (po2!=NULL){
		p1=p2;
		p1->left=new Instruction_Leaf;
		p1=p1->left;
		p1->pointer=po1;
		p1->valid=0;
		p1->left=NULL;
		p1->right=NULL; 
	}
	return;
}
*/

int Instruction_Tree::Obhod(const char* command, PolyUnpack& unpack){
	int kol=0;
	int  iterator=0;
	int result=0;
	Obhod(root, command, unpack, kol, iterator, result);
	//printf("KOL_of_Valid=%d\n", kol);
	if (kol==0){
		Destructor(root);
		return -1;
	}
	if (kol==1){
		Destructor(root);
		return result;
	}
	if (kol>1)
		return result;
}

void Instruction_Tree::Obhod(Instruction_Leaf* & p1, const char* command, PolyUnpack & unpack, int & kol, int & iterator, int & result){
	Instruction_Tree* p2;
	if ((p1->left!=NULL)&&(p1->right!=NULL)){
		//printf("(p1->left!=NULL)&&(p1->right!=NULL)\n");
		if (p1->left->valid!=-1)
			Obhod(p1->left, command, unpack, kol, iterator, result);
		if (p1->right->valid!=-1)
			Obhod(p1->right, command, unpack, kol, iterator, result);
		if ((p1->left->valid==-1)&&(p1->right->valid==-1))
			p1->valid=-1;
		return;
	}
	if ((p1->left!=NULL)&&(p1->right==NULL)){
		//printf("(p1->left!=NULL)&&(p1->right==NULL)\n");
		if (p1->left->valid!=-1)
			Obhod(p1->left, command, unpack, kol, iterator, result);
		if (p1->left->valid==-1)
			p1->valid=-1;
		return;
	}
	if ((p1->left==NULL)&&(p1->right!=NULL)){
		//printf("(p1->left==NULL)&&(p1->right!=NULL)\n");
		if (p1->right->valid!=-1)
			Obhod(p1->right, command, unpack, kol, iterator, result);
		if (p1->right->valid==-1)
			p1->valid=-1;
		return;
	}
	if ((p1->left==NULL)&&(p1->right==NULL)){
		//printf("(p1->left==NULL)&&(p1->right==NULL)\n");
		if ((p1->valid==0)){
			//printf("Next Leaf=%s\n", p1->pointer->instruction);
			//printf("Command=%s\n", command);
			//kol++;
			InstructionList* po2=p1->pointer;
			InstructionList* po3=NULL;
			int flag;
			flag=unpack.FindNextInstruction(po2, po3, command, iterator);
			if (result<flag){
				result=flag;
			}
			if (flag==-1){
				//printf("flag=-1\n");
				//printf("%s\n", p2->instruction);
				p1->valid=-1;
				return;
			}
			if ((flag==1)||(flag==2)){
				//printf("flag=1\n");
				//printf("%s\n", po2->instruction);
				p1->left=new Instruction_Leaf;
				p1=p1->left;
				p1->pointer=po2;
				p1->valid=0;
				p1->left=NULL;
				p1->right=NULL;
				kol++;
				return;
			}
			if (flag==0){
				//printf("flag=0\n");
				//printf("%s\n", po2->instruction);
				Instruction_Leaf* p2=p1;
				p1->left=new Instruction_Leaf;
				p1=p1->left;
				p1->pointer=po2;
				p1->valid=0;
				p1->left=NULL;
				p1->right=NULL;
				if (po3!=NULL){
					p1=p2;
					p1->right=new Instruction_Leaf;
					p1=p1->right;
					p1->pointer=po3;
					p1->valid=0;
					p1->left=NULL;
					p1->right=NULL;
				}
				kol++;
			}
		}
	}
}
void Instruction_Tree::Destructor(Instruction_Leaf*  & p1){
	//printf("Destructor\n");
	if ((p1->left!=NULL)&&(p1->right!=NULL)){
		Destructor(p1->left);
		Destructor(p1->right);
		delete p1;
		p1=NULL;
		return;
	}
	if ((p1->left!=NULL)&&(p1->right==NULL)){
		Destructor(p1->left);
		delete p1;
		p1=NULL;
		return;
	}
	if ((p1->left==NULL)&&(p1->right!=NULL)){
		Destructor(p1->right);
		delete p1;
		p1=NULL;
		return;
	}
	if ((p1->left==NULL)&&(p1->right==NULL)){
		delete p1;
		p1=NULL;
		return;
	}
}

Instruction_Tree::~Instruction_Tree(){
	if (root!=NULL)
		Destructor(root);
}

bool Instruction_Tree::Is_Empty(){
	if (root==NULL)
		return true;
	return false;
}
 
///////////////////////////////////////////////////////
PE_Reader::PE_Reader(const char* name){
	int i=0;
	filename=new char[strlen(name)+1];
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
	if ((fp = fopen(filename, "r+b")) == NULL){
		perror(filename);
		//printf("This Error may be because of the symbols in file name!\n");
        exit(1);
    }
}

PE_Reader::PE_Reader(const PE_Reader* pe_reader){
	int i=0;
	if (filename!=NULL)
		delete []filename;
	filename=new char[strlen(pe_reader->filename)+1];
	while (pe_reader->filename[i]!='\0'){
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
	delete []filename;
	fclose(fp);
}

void PE_Reader::PE_HeaderOffset(){
	int offset=60;
	int origin=0;
	fseek(fp, offset, origin);
    int header1=getc(fp);
    int header2=getc(fp);
    int header3=getc(fp);
    int header4=getc(fp);
    header_offset=header4*pow(16,6)+header3*pow(16,4)+header2*pow(16,2)+header1;
    //printf("PE_HEADER_OFFSET=%d\n", header_offset);
}

void PE_Reader::PE_TableOffset(){
	table_offset=header_offset+248;
}

void PE_Reader::PE_FileType() const{
	int origin=0;
	fseek(fp, header_offset, origin);
    int format1=getc(fp);
    int format2=getc(fp);
    int format3=getc(fp);
    int format4=getc(fp);
    int format=format4*pow(16,6)+format3*pow(16,4)+format2*pow(16,2)+format1;
    if (format==0x4550){
		//printf("FORMAT = Portable Executable \n");
		return;
	}
	else
		throw "Unknown format\n";
}

void PE_Reader:: PE_NumberOfEntries(){
	int origin=0;
	int number_of_entries_offset=header_offset+6;
    fseek(fp, number_of_entries_offset, origin);
    int number_of_entries1=getc(fp);
    int number_of_entries2=getc(fp);
    number_of_entries=number_of_entries2*pow(16,2)+number_of_entries1;
    //printf("Number_of_entries=%d\n", number_of_entries);
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
    //printf("Entry_Point_Rva=%d\n", entry_point_rva);
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
    //printf("Image_base=%d\n", image_base);
}

void PE_Reader::PE_ObjectTable(){
	int origin=0;
	int section_offset=0;
    char section_name[8];
    int address_mass[number_of_entries];
	int size_mass[number_of_entries];
	int table_offset_mass[number_of_entries];
	for (int i=0; i<number_of_entries; i++){
		int table_offset=header_offset+248+section_offset;
		fseek(fp, table_offset, origin);
		table_offset_mass[i]=table_offset;
			
		for (int j=0;j<8; j++){
			section_name[j]=getc(fp);
		}
		section_name[8]='\0';
		
		int section_address_offset=table_offset+12;
		fseek(fp, section_address_offset, origin);
		int section_address_rva1=getc(fp);
		int section_address_rva2=getc(fp);
		int section_address_rva3=getc(fp);
		int section_address_rva4=getc(fp);
		address_mass[i]=section_address_rva4*pow(16,6)+section_address_rva3*pow(16,4)+section_address_rva2*pow(16,2)+section_address_rva1;
		
		int section_size_offset=table_offset+16;
		fseek(fp, section_size_offset, origin);
		int section_size1=getc(fp);
		int section_size2=getc(fp);
		int section_size3=getc(fp);
		int section_size4=getc(fp);
		size_mass[i]=section_size4*pow(16,6)+section_size3*pow(16,4)+section_size2*pow(16,2)+section_size1;
		
		//printf("Section_name %s\n", section_name);
		//printf("Section_address_rva %d\n", address_mass[i]);
		//printf("Section_size %d\n", size_mass[i]);
		
		section_offset=section_offset+40;
	}
	
	for (int i=0; i<number_of_entries; i++){
		if ((entry_point_rva>=address_mass[i])&&(entry_point_rva<=address_mass[i]+size_mass[i])){
			//printf("Text section has number=%d\n", i+1);
			text_address_rva=address_mass[i];
			text_size=size_mass[i];
			PE_CodeSection(table_offset_mass[i]);
		}
	}
}


void PE_Reader::PE_CodeSection(int section_offset){ 
	int origin=0;
	int text_address_offset=section_offset+12;
	fseek(fp, text_address_offset, origin);
	int text_address_rva1=getc(fp);
	int text_address_rva2=getc(fp);
	int text_address_rva3=getc(fp);
	int text_address_rva4=getc(fp);
	text_address_rva=text_address_rva4*pow(16,6)+text_address_rva3*pow(16,4)+text_address_rva2*pow(16,2)+text_address_rva1;
	//printf("Text_address_rva=%d\n", text_address_rva);
	
	int text_size_offset=section_offset+16;
	fseek(fp, text_size_offset, origin);
	int text_size1=getc(fp);
	int text_size2=getc(fp);
	int text_size3=getc(fp);
	int text_size4=getc(fp);
	text_size=text_size4*pow(16,6)+text_size3*pow(16,4)+text_size2*pow(16,2)+text_size1;
	//printf("Code_size%d\n", text_size);
			
	int table_text_offset=section_offset+20;
	fseek(fp, table_text_offset, origin);
	int base_of_text1=getc(fp);
	int base_of_text2=getc(fp);
	text_offset=base_of_text2*pow(16,2)+base_of_text1;
	//printf("Code_offset=%d\n", text_offset);	
}

void PE_Reader::PE_DataSection(int section_offset){
	int origin=0;
	int data_size_offset=section_offset+16;
	fseek(fp, data_size_offset, origin);
	int data_size1=getc(fp);
	int data_size2=getc(fp);
	int data_size3=getc(fp);
	int data_size4=getc(fp);
	data_size=data_size4*pow(16,6)+data_size3*pow(16,4)+data_size2*pow(16,2)+data_size1;
	//printf("Data_size%d\n", data_size);
			
	int table_data_offset=section_offset+20;
	fseek(fp, table_data_offset, origin);
	int table_base_of_data1=getc(fp);
	int table_base_of_data2=getc(fp);
	data_offset=table_base_of_data2*pow(16,2)+table_base_of_data1;
	//printf("Data_offset=%d\n", data_offset);
}

void PE_Reader::PE_LoadForStaticAnalise(char* &buf, int& len, int& start){	
	PE_HeaderOffset();
	PE_FileType();
	PE_TableOffset();
	PE_NumberOfEntries();
	PE_EntryPoint();
	//PE_ImageBase();//not need in static analise
	PE_ObjectTable();
	//int physical_entry_point=entry_point_rva-text_address_rva+text_offset;
	start=entry_point_rva-text_address_rva;
	//printf("Entry_point_rva=%d\n", entry_point_rva);
	//printf("Text_address_rva=%d\n", text_address_rva);
	//printf("Text_offset=%d\n", text_offset);
	//printf("Physical_entry_point!!!=%d\n", physical_entry_point);
	
	//printf("Entry_point %d\n", start);
	len=text_size;
	int origin=0;
	fseek(fp, text_offset, origin);
	buf=new char[text_size];
	if (buf==NULL){
		//printf("Memory error  in static analise\n"); 
		exit (2);
	}
	fread(buf, 1, text_size, fp);
};

void PE_Reader::PE_LoadForDynamicAnalise(char* &buf, int& len, int & start, int & offset){	
	
	PE_HeaderOffset();
	PE_FileType();
	PE_TableOffset();
	PE_NumberOfEntries();
	PE_EntryPoint();
	PE_ImageBase();
	PE_ObjectTable();
	//printf("Entry_point_rva=%d\n", entry_point_rva);
	//printf("Text_address_rva=%d\n", text_address_rva);
	//printf("Text_offset=%d\n", text_offset);
	start=entry_point_rva-text_address_rva+text_offset;
	offset=text_offset;
	//printf("Physical_entry_point!!!=%d\n", start);

	int file_size;
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	len=file_size;
	buf=new char[len];
	if (buf==NULL){
		//printf("Memory error  in dynamic analise\n"); 
		exit (2);
	}
	fseek(fp, 0, 0);
	int result = fread(buf, 1, file_size, fp);
	if (result!=file_size){
		//printf("Error  in reading in dynamic analise\n"); 
		exit (2);
	}
	//printf("FILE_SIZE=%d\n", file_size);
};

Emulator_LibEmu::Emulator_LibEmu(){
	e = emu_new();
	cpu = emu_cpu_get(e);
	mem = emu_memory_get(e);
}

Emulator_LibEmu::~Emulator_LibEmu(){
	emu_free(e);
}

Stack::~Stack(){
	List_of_Return_Addresses *p1=start;
	List_of_Return_Addresses *p2;
	while (p1!=NULL){
		p2=p1->pointer;
		delete p1;
		p1=p2;
	}
}

void Stack::Push(const int a){
	if (start==NULL){
		start=new List_of_Return_Addresses;
		start->address=a;
		start->pointer=NULL;
		return;
	}
	List_of_Return_Addresses* p=start;
	while(p->pointer!=NULL){
		p=p->pointer;
	}
	p->pointer=new List_of_Return_Addresses;
	p=p->pointer;
	p->address=a;
	p->pointer=NULL;
	return;
}

void Stack::Pop(int & a){
	if (start==NULL){
		//printf("Stack is empty! Don't know where to go!\n");
		exit(0);
		//exit(33);
		return; 
	}
	List_of_Return_Addresses* p=start;
	List_of_Return_Addresses* p2=start;
	int kol=0;
	while(p->pointer!=NULL){
		p2=p;
		p=p->pointer;
		kol++;
	}
	a=p->address;
	delete p;
	if (kol==0)
		start=NULL;
	else
		p2->pointer=NULL;
	return;
}

void Stack::Print(){
	List_of_Return_Addresses* p=start;
	while (p!=NULL){
		//printf("Address=%d\n", p->address);
		p=p->pointer;
	}
}


PolyUnpack::PolyUnpack(){
	start=NULL;
	n=4;
	buf_size=0;
	offset=0;
	step_kol=10000;
	emulator=new Emulator_LibEmu;
}

PolyUnpack::~PolyUnpack(){
	InstructionList* p1=start;
	InstructionList* p2;
	while(p1!=NULL){
		p2=p1->pointer;
		delete []p1->instruction;
		delete p1;
		p1=p2;
	}
	delete emulator;
}

void PolyUnpack::AddInstruction(InstructionList* &p1, const char* string, const int number, const int length, const bool flag){
	if (start==NULL){
		start=new InstructionList;
		p1=start;
	}
	else{
		p1->pointer=new InstructionList;
		p1=p1->pointer;
	}
	p1->instruction=new char[strlen(string)+1];
	int i=0;
	while (string[i]!='\0'){
		p1->instruction[i]=string[i];
		i++;
	}
	p1->instruction[i]='\0';
	
	/*
	int j=0, k=n;
	p1->instruction=new char[k];
	while(string[j]!='\0'){
		if(j==k){
			k=k*2;
			free(p1->instruction);
			p1->instruction=new char[k];
			for (int l=0; l<j; l++)
				p1->instruction[l]=string[l];
		}
		p1->instruction[j]=string[j];
		j++;
	}
	p1->instruction[j]='\0';
	*/ 
	p1->number=number;
	p1->length=length;
	p1->first=flag;
	p1->padding=false;
	p1->pointer=NULL;
	
}

void PolyUnpack::FindPadding() const{
	InstructionList* p1=start;
	InstructionList* p2=NULL;
	while (p1!=NULL){
		if (strcmp(p1->instruction, "add [eax],al")==0){
			if (p2==NULL)
				p2=p1;
		}
		else{
			p2=NULL;
		}
		p1=p1->pointer;
	}
	p1=p2;
	while (p1!=NULL){
		p1->padding=true;
		p1=p1->pointer;
	}
}


void PolyUnpack::StaticAnalise(const char* FileName, const int key=0){
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	InstructionList* p1=NULL; 
	char *data=NULL;
	int i=0, k=n, c=0, bytes=8, size=0, len=0, number=0;
	char string[32];
	PE_Reader reader(FileName);
	int entry_point=0;
	bool flag=false;
	reader.PE_LoadForStaticAnalise(data, size, entry_point);
	//printf("Static analize entry=%d\n", entry_point);
	if(size==0){
		//printf("There are no code section in this file. Don't know what to do!\n");
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
		
		if (key==1)
			printf("%.8x  ", c);
		
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
		
		if (key==1)
			printf("%s\n", string);

		if (c==entry_point){
			//printf("THIS IS ENTRY_POINT\n");
			flag=true;
		}
		else
			flag=false;
		AddInstruction(p1, string, c , len, flag);
		c=c+len;
	} 	
	FindPadding();
	
	delete []data;
	//printf("+++++++++++++++++++++++++++++++++++++++++++++\n");
}

void Emulator_LibEmu::begin(char* & buf, const int len, const int entry_point){
	for (int i=0; i<8; i++) {
		emu_cpu_reg32_set(cpu, (emu_reg32) i, 0);
	}
	emu_cpu_reg32_set(cpu, esp, 0x1000000);

	emu_memory_clear(mem);
	emu_memory_write_block(mem, 0x20000000L, (void*)buf, len);
	/*int i=0;
	for (i=0; i<len; i++){
		emu_memory_write_byte(mem, 0x20000000L+i, buf[i]);
	}
	*/
	emu_memory_write_byte(mem, 0x20000000L+len, '\xcc');
	jump(entry_point);
}


void Emulator_LibEmu::jump(const uint pos){
	emu_cpu_eip_set(cpu, 0x20000000L+pos);
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

bool Emulator_LibEmu::get_command(char *buff, uint size) const{
	emu_memory_read_block(mem, emu_cpu_eip_get(cpu), buff, size);
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

bool compare(const char* string1, const char* string2){
	int i=0;
	while((string1[i]!='\0')&&(string2[i]!='\0')){
		if (string1[i]!=string2[i])
			return false;
		i++;
	}
	return true;
}
 
bool PolyUnpack::FindFirstInstruction(InstructionList* & p) const{
	InstructionList* point=start;
	
	while((point!=NULL)&&(!point->first)){
		point=point->pointer;
	}
	if(point==NULL){
		p=NULL;
		return false;
	}
	else{
		p=point;
		return true;
	}
} 

void PolyUnpack::DynamicAnalise(const char* FileName, const int key=0){
	//Instruction_Tree root;
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	InstructionList* p=start;
	bool good_parse=true;
	bool valid_file=true;
	int invalid_instr_kol=0;
	int  bytes=8, c=0, size=0, len=0, flag=1;
	int string_size=32;
	int command_size=32;
	char* string=new char[string_size];
	char* command=new char[command_size];
	
	for (int i=0; i<command_size; i++){
		command[i]='\0';
	}
	
	Register reg=EIP;
	char* buf;
	buf_size=0;
	int entry_point=0;
	PE_Reader reader(FileName);
	reader.PE_LoadForDynamicAnalise(buf, buf_size, entry_point, offset);
	emulator->begin(buf, buf_size, entry_point);
	
	if(!FindFirstInstruction(p)){
		printf("Can't find First instruction\n");
		//exit(33);
	}
	/*
	if (entry_point==0)
		exit(1);
	*/ 

	int kol=0;
	for(;;){
		
		emulator->get_command(command, command_size);			
		//printf("Begin step\n");
		uint old_eip_pointer=emulator->get_register(reg);
		//printf("EIP before=%d\n", old_eip_pointer);
		good_parse=emulator->step(); 
		//printf("EIP after=%d\n", emulator->get_register(reg));
		//printf("End step\n");
		
		inst_pointer=&inst;
		len=get_instruction(inst_pointer, (BYTE*)(command), MODE_32);
		if (!len || (len>command_size)){
			if (key==1){
				if (!len)
					printf("Illegal opcode\n");
				else {
					printf("Else brunch\n");
					printf("It is impossible!\n");
				}
				printf("%.8x  ", c);
				printf("db 0x%.2x\n", command);
			}
			kol++;
			emulator->jump(old_eip_pointer+1-0x20000000L);
			if(kol>=10000)
				exit(0);
			continue;
		}
 
		inst_pointer=&inst;
		c=0;//may be like this!
		get_instruction_string(inst_pointer, Format(FORMAT_INTEL), (DWORD)c, string, string_size);
		flag=Compare(p, string, flag, old_eip_pointer, len);
		if(flag==-1){
			//exit(101);
			valid_file=false;
			invalid_instr_kol++;
			if(key==1){
				printf("????????????????????????????????????????????????\n");
				for (int i = 0; i < ((bytes) < (len) ? (bytes) : (len)); i++){
					//printf("%.2x", command[c + i]);
					printf("%.2x", command[i]);
				}
				printf("  ");
				for (int i = (bytes) < ((len) ? (bytes) : (len)); i < bytes*2 - len; i++)
					printf(" ");
				printf("%.8x  ", c);
				printf("ILLEGAL = %s\n", string);
				printf("Step Number=%d\n", kol);
				printf("????????????????????????????????????????????????\n");
			}
			//p=start;
			//exit(101);
		}
		else{
			if (flag==2)
				emulator->jump(old_eip_pointer+len-0x20000000L);
			if (key==1){
				for (int i = 0; i < ((bytes) < (len) ? (bytes) : (len)); i++)
					printf("%.2x", command[i]);
				printf("  ");
				for (int i = (bytes) < ((len) ? (bytes) : (len)); i < bytes*2 - len; i++)
					printf(" ");
				printf("%.8x  ", c);
				printf("%s\n", string);
				printf("Step Number=%d\n", kol);
			}
		}
		//emulator.jump(old_eip_pointer+len-0x20000000L);
		//printf("**************************************\n");
		if((!good_parse)||(compare(string, "test ecx,0x3"))){
			printf("We are in bad parse brunch\n");
			//emulator->jump(old_eip_pointer+len-0x20000000L);
		}
		 
		for (int i=0; i<string_size; i++)
			string[i]='\0';
		for (int i=0; i<command_size; i++)
			command[i]='\0';
		/*
		if ((p!=NULL)&&(p->padding))
			printf("p->padding\n");
		*/
		if(((p!=NULL)&&(p->padding))||(kol>=step_kol))
			break;
		kol++;
	}
	delete []string;
	delete []command;
	delete []buf;
	if (valid_file){
		printf("This file is valid\n");
		exit(0);
	}
	else{
		printf("This is VIRUS\n");
		printf("Kol of invalid instructions=%d\n", invalid_instr_kol);
		exit(1);
	}
	//printf("***************\n");
}

bool PolyUnpack::FindInstruction(InstructionList* & p, const int number) const{
	//printf("Number=%d\n", number);
	if (!is_valid_number(number)){
		//printf("Invalid Address!\n");
		exit(1);
	}
	InstructionList* point=start;
	while((point!=NULL)&&(point->number!=number)){
		point=point->pointer;
	}
	p=point;
	if(point==NULL){
		//printf("Number=%d\n", number);
		//printf("Instruction not found!\n May be because of unvalid jump.\n");
		return false;
	}
	return true;
}

bool PolyUnpack:: is_valid_number(const int number) const{
	if ((number>=0)&&(number<=buf_size)){
		return true;
	}
	return false;
}

int parse(const char* string){
	int i=0;
	while(string[i]!=' ')
		i++;
	while(string[i]==' ')
		i++;
	return i;
}

void get_register_name(char* & string, const char* instruction){
	int n=4;
	int i=0;
	string=new char[strlen(instruction)];
	while(instruction[i]!='\0'){
		string[i]=instruction[i];
		i++;	
	}
	string[i]='\0';
}

int GetNumber2 (const char* string){
	int koef=16;
	int number=0;
	int i=0;
	while(string[i]!='\0'){
		if ((string[i]>='0')&&(string[i]<='9')){
			number=number*koef+(string[i]-'0');
		}
		else{
			if ((string[i]>='a')&&(string[i]<='f'))
				number=number*koef+(string[i]-'a'+10);
			else{
				//printf("Invalid number\n");
				number=0;
				break;
			}
		}
		i++;
	}
	return number;
}


uint GetNumber (const char* string){
	uint segment=0;
	uint offset=0;
	int koef=16;
	uint number=0;
	int i=0;
	while(string[i]!='\0'){
		if ((string[i]>='0')&&(string[i]<='9')){
			offset=offset*koef+(string[i]-'0');
		}
		else{
			if ((string[i]>='a')&&(string[i]<='f'))
				offset=offset*koef+(string[i]-'a'+10);
			else{
				if (string[i]==':'){
					i=i+2;
					segment=offset;
				}
				/*
				else
					printf("Invalid number\n");
				*/
				offset=0;
				break;
			}
		}
		i++;
	}
	number=segment*pow(koef,4)+offset;
	return number;
}


int PolyUnpack::Compare(InstructionList* &p, const char* command, int flag, const uint old_eip, const int len){
	if (root.Is_Empty()){
		//printf("root=NULL\n");
		if (p==NULL){
			if(!FindInstruction(p, old_eip-offset-0x20000000L))
				return -1;	
		}
		int iterator=0;
		InstructionList* p2=NULL;
		flag=FindNextInstruction(p, p2, command, iterator);
		if (flag==-1)
			p=NULL;
		return flag;
	}
	else{
		//printf("root!=NULL\n");
		int flag;
		Register reg=EIP;
		flag=root.Obhod(command, *this);
		if (flag==2){
			//printf("FLAG==2\n");
			emulator->jump(old_eip+len-0x20000000L);
		}
		FindInstruction(p, emulator->get_register(reg)-offset-0x20000000L);
		return flag;
	}	
}


int PolyUnpack::FindNextInstruction(InstructionList* &p, InstructionList* & p2, const char* command, int & iterator){
	//printf(" INSTRUCTION=%s\n", p->instruction);
	//printf(" COMMAND=%s\n", command);
	 
	if (compare(p->instruction,"call")){
		if (!compare (command, "call"))
			return -1;
		int i=parse(p->instruction);
		if (p->instruction[i]=='['){
			if (command[i]!='[')
				return -1;
			p=p->pointer;	
			return 2;
		}
		else{
			if(p->pointer!=NULL){
				if (iterator==0){
					return_addresses.Push(p->pointer->number);
					//return_addresses.Print();
					iterator++;
					//printf("Future EIP=%d\n", p->pointer->number+offset+0x20000000L);
				}
			}
			else
				return -1;
			if ((p->instruction[i])=='0'){
				int number=GetNumber(p->instruction+i+2);	
				if(!FindInstruction(p, number))
					return -1;
				return 1;
			}
			else{
				char* string;
				int i=parse(p->instruction);
				get_register_name(string, p->instruction+i);
				int num=emulator->get_register(string);
				delete []string;
				//printf("Register=%d\n", num);
				if (!is_valid_number(num-0x20000000L-offset)){
					exit(0);
				}
				//printf("REGISTER BEFORE CALL=%d\n", num);		
				if (!FindInstruction(p, num-0x20000000L-offset))
					return -1; //so that this instruction will be unvalid but not next
				//printf("%s\n", p->instruction);
				return 1;
			}
		}
	}
	//printf(" INSTRUCTION=%s\n", p->instruction);
	//printf(" COMMAND=%s\n", command);
	if ((compare(p->instruction,"jz"))||(compare(p->instruction,"jnz"))||
		(compare(p->instruction,"jns"))||(compare(p->instruction,"jmp"))||
		(compare(p->instruction,"ja"))||(compare(p->instruction,"jna"))||
		(compare(p->instruction,"jo"))||(compare(p->instruction,"jno"))||
		(compare(p->instruction,"jnc"))||(compare(p->instruction,"jc"))||
		(compare(p->instruction,"jl"))||(compare(p->instruction,"jnl"))||
		(compare(p->instruction,"js"))||(compare(p->instruction,"jns"))||
		(compare(p->instruction,"jecxz"))||(compare(p->instruction,"jp"))||
		(compare(p->instruction,"jpo"))||(compare(p->instruction,"loopn"))||
		(compare(p->instruction,"loop"))||(compare(p->instruction,"jg"))||(compare(p->instruction,"jng"))||
		(compare(p->instruction,"jmpf"))){
		if (((compare(p->instruction,"jz"))&&(!compare(command,"jz")))||
			((compare(p->instruction,"jnz"))&&(!compare(command,"jnz")))||
			((compare(p->instruction,"jns"))&&(!compare(command,"jns")))||
			((compare(p->instruction,"jmp"))&&(!compare(command,"jmp")))||
			((compare(p->instruction,"ja"))&&(!compare(command,"ja")))||
			((compare(p->instruction,"jna"))&&(!compare(command,"jna")))||
			((compare(p->instruction,"jo"))&&(!compare(command,"jo")))||
			((compare(p->instruction,"jno"))&&(!compare(command,"jno")))||
			((compare(p->instruction,"jc"))&&(!compare(command,"jc")))||
			((compare(p->instruction,"jnc"))&&(!compare(command,"jnc")))||
			((compare(p->instruction,"jl"))&&(!compare(command,"jl")))||
			((compare(p->instruction,"js"))&&(!compare(command,"js")))||
			((compare(p->instruction,"jns"))&&(!compare(command,"jns")))||
			((compare(p->instruction,"jecxz"))&&(!compare(command,"jecxz")))||
			((compare(p->instruction,"jp"))&&(!compare(command,"jp")))||
			((compare(p->instruction,"jpo"))&&(!compare(command,"jpo")))||
			((compare(p->instruction,"loopn"))&&(!compare(command,"loopn")))||
			((compare(p->instruction,"loop"))&&(!compare(command,"loop")))||
			((compare(p->instruction,"jg"))&&(!compare(command,"jg")))||
			((compare(p->instruction,"jng"))&&(!compare(command,"jng")))||
			((compare(p->instruction,"jmpf"))&&(!compare(command,"jmpf")))){
			return -1;
		}	
		if(compare(p->instruction, "jmp")){
			int i=parse(p->instruction);
			if (p->instruction[i]=='['){
				if (command[i]!='[')
					return -1;
				p=p->pointer;	
				return 2;
			}
			if (p->instruction[i]=='0'){
				uint number=GetNumber(p->instruction+i+2);	
				if(!FindInstruction(p, number))
					return -1;
				return 1;
			}
			else{
				char* string;
				get_register_name(string, p->instruction+i);
				uint num=emulator->get_register(string);
				//printf("REGISTER BEFORE JUMP=%d\n", num);
				delete []string;
				if (!is_valid_number(num-0x20000000L-offset)){
					exit(0);// in register case this is good
				}
				if (!FindInstruction(p, num-0x20000000L-offset))
					return -1;
				return 1;
			}
		}
		else{
			int i=parse(p->instruction);
			if (p->instruction[i]=='0'){
				uint number=GetNumber(p->instruction+i+2);	
				InstructionList* p2;
				if (FindInstruction(p2, number)){
					if (root.Is_Empty())
						root.Add(p, p->pointer, p2);
					else
						p=p->pointer;
				}
				else{
					if (root.Is_Empty()){
						//p2=NULL;
						root.Add(p, p->pointer, p2);
					}
					else{
						p=p->pointer;
						//p2=NULL;
					}
				}
				return 0;
			}
			else{
				char* string;
				get_register_name(string, p->instruction+i);
				uint num=emulator->get_register(string);
				delete []string;
				printf("REGISTER BEFORE JUMP=%d\n", num);
				if (!is_valid_number(num-0x20000000L-offset)){
					exit(0);//good in register case
				}
				InstructionList* p2;
				if (FindInstruction(p2, num-0x20000000L-offset)){
					if (root.Is_Empty())
						root.Add(p, p->pointer, p2);
					else
						p=p->pointer;
				}
				else{
					if (root.Is_Empty()){
						//p2=NULL;
						root.Add(p, p->pointer, p2);
					}
					else{
						p=p->pointer;
						//p2=NULL;
					}
				}
				return 0;
			}
			return 0;
		}
		return -1;	
	}
	if (strcmp(p->instruction, command)==0){
		if ((compare(p->instruction ,"ret"))||(compare(p->instruction,"retn"))||
			(compare(p->instruction, "retf"))||(compare(p->instruction, "iret"))||
											(compare(p->instruction, "rep ret"))){
			int temp;
			if (iterator==0){
				iterator++;
				return_addresses.Pop(temp);
				//return_addresses.Print();
				//printf("Instruction Ret was found EIP=%d\n", temp+offset+0x20000000L);
				if (!FindInstruction(p, temp)){
					return -1;
				}
				emulator->jump(temp+offset);
				return 1;
			}
		}
		p=p->pointer;
		return 1;
	}
	else{
		p=NULL;
		return -1;
	}
	throw "Bad command\n";
}


/*

int PolyUnpack::Compare(Emulator_LibEmu & emulator, InstructionList* &p, char* & command, int flag, int offset, uint old_eip){
	printf("Compare Function\n");
	int command_number=0; 
	if(flag==-1){
		if(!FindInstruction(p, old_eip-0x20000000L-offset)){
			return -1;
		}
	}
	if (flag==0){

		printf("We are here!!! flag=0\n");
		if ((p->pointer!=NULL)&&((strcmp(p->pointer->instruction, command)==0)||
			((compare(p->pointer->instruction, "jmp"))&&(compare(command, "jmp")))||
			((compare(p->pointer->instruction, "call"))&&(compare(command,"call"))))){
			if (strcmp(p->pointer->instruction, command)==0)
				p=p->pointer;
			else{
				int i=0;
				while (command[i]!=' '){
					printf("Command[0]=%c\n", command[i]);
					i++;
				}
				while (command[i]==' '){
					printf("Command[0]=%c\n", command[i]);
					i++;
				}
				printf("Command[]=%c\n", command[i]);
				if ((p->pointer->instruction[i]!='[')&&(command[i]!='[')){
					printf("TYT\n");
					p=p->pointer;
				}
				else{
					flag=Compare(emulator, p->pointer, command, 99, offset, old_eip);
					printf("FLAG=%d\n", flag);
					printf("Instr=%s\n", p->instruction);
					if ((flag==-1)||(flag==14)){
						int i=parse(p->instruction);
						int number=GetNumber(p->instruction+i+2);
						if (!FindInstruction(p, number)){
							printf("Bad\n");
							return -1;
						}
					}
					if ((flag==0)||(flag==2)||(flag==3)){
						p=p->pointer;
					}
					printf("New_Instruction=%s\n", p->instruction);
					
				}
				printf("This bruch\n");
			} 
		}
		else{
			printf("Else brunch\n");
			flag=Compare(emulator, p->pointer, command, 99, offset, old_eip);
			printf("FLAG=%d\n", flag);
			printf("Instr=%s\n", p->instruction);
			if (flag==-1){
				int i=parse(p->instruction);
				int number=GetNumber(p->instruction+i+2);
				if (!FindInstruction(p, number)){
					return -1;
				}
			}
			if ((flag==0)||(flag==2)||(flag==3)){
				p=p->pointer;
			} 
		}
	}
	if (p==NULL){
		return -1;
	}
	printf(" INSTRUCTION=%s\n", p->instruction);
	printf(" COMMAND=%s\n", command);
	if ((compare(p->instruction,"jz"))||(compare(p->instruction,"jnz"))||
		(compare(p->instruction,"jns"))||(compare(p->instruction,"jmp"))||
		(compare(p->instruction,"ja"))||(compare(p->instruction,"jna"))||
		(compare(p->instruction,"jo"))||(compare(p->instruction,"jno"))||
		(compare(p->instruction,"jnc"))||(compare(p->instruction,"jc"))||
		(compare(p->instruction,"jl"))||(compare(p->instruction,"jnl"))||
		(compare(p->instruction,"js"))||(compare(p->instruction,"jns"))||
		(compare(p->instruction,"jecxz"))||(compare(p->instruction,"jp"))||
		(compare(p->instruction,"jpo"))||(compare(p->instruction,"loop"))||
		(compare(p->instruction,"jg"))||(compare(p->instruction,"jng"))||
		(compare(p->instruction,"jmpf"))){
		if (((compare(p->instruction,"jz"))&&(!compare(command,"jz")))||
			((compare(p->instruction,"jnz"))&&(!compare(command,"jnz")))||
			((compare(p->instruction,"jns"))&&(!compare(command,"jns")))||
			((compare(p->instruction,"jmp"))&&(!compare(command,"jmp")))||
			((compare(p->instruction,"ja"))&&(!compare(command,"ja")))||
			((compare(p->instruction,"jna"))&&(!compare(command,"jna")))||
			((compare(p->instruction,"jo"))&&(!compare(command,"jo")))||
			((compare(p->instruction,"jno"))&&(!compare(command,"jno")))||
			((compare(p->instruction,"jc"))&&(!compare(command,"jc")))||
			((compare(p->instruction,"jnc"))&&(!compare(command,"jnc")))||
			((compare(p->instruction,"jl"))&&(!compare(command,"jl")))||
			((compare(p->instruction,"js"))&&(!compare(command,"js")))||
			((compare(p->instruction,"jns"))&&(!compare(command,"jns")))||
			((compare(p->instruction,"jecxz"))&&(!compare(command,"jecxz")))||
			((compare(p->instruction,"jp"))&&(!compare(command,"jp")))||
			((compare(p->instruction,"jpo"))&&(!compare(command,"jpo")))||
			((compare(p->instruction,"loop"))&&(!compare(command,"loop")))||
			((compare(p->instruction,"jg"))&&(!compare(command,"jg")))||
			((compare(p->instruction,"jng"))&&(!compare(command,"jng")))||
			((compare(p->instruction,"jmpf"))&&(!compare(command,"jmpf")))){
			return -1;
		}	
		int i=parse(p->instruction);
		if (p->instruction[i]=='['){
			p=p->pointer;
			return 14;
		}
		else{
			if(compare(p->instruction, "jmp")){
				int i;
				if (compare(p->instruction, "jmpf")){
					i=5;
				}
				else{
					i=4;
				}
				printf("p->instruction[i]=%c\n", p->instruction[i]);
				if (p->instruction[i]=='0'){
					uint number=GetNumber(p->instruction+i+2);	
					printf("MYNumber=%d\n", number);
					FindInstruction(p, number);
				}
				else{
					char* string;
					
					get_register_name(string, p->instruction+i);
					
					uint num=emulator.get_register(string);
					printf("REGISTER BEFORE JUMP=%d\n", num);
					if (!is_valid_number(num-0x20000000L-offset)){
						exit(0);
					}
					if (!FindInstruction(p, num-0x20000000L-offset)){
						return -1;
					}
					if (p!=NULL)
						printf("%s\n", p->instruction);
					free(string);
				}
				return 1;
			}
			return 0;
		}
	}
	if (compare(p->instruction,"call")){
		//printf("We are in call section\n");
		if (!compare (command, "call")){
			return -1;
		}
		int i=parse(p->instruction);
		if (p->instruction[i]=='['){
			p=p->pointer;
			return 14;
		}
		else{
			if((p->pointer!=NULL)&&(flag!=99)){
				command_number=p->pointer->number;
				return_addresses.Push(command_number);
				printf("Future EIP=%d\n", command_number+offset+0x20000000L);
			}
			else{
				if(flag==99)
					return 2;
			}
			if ((p->instruction[i])=='0'){
				int number=GetNumber(p->instruction+i+2);	
				if(!FindInstruction(p, number)){
					return -1;
				}
			}
			else{
				char* string;
				get_register_name(string, p->instruction+i);
				
				int num=emulator.get_register(string);
				printf("Vot zdes\n");
				printf("Register=%d\n", num);
				if (!is_valid_number(num-0x20000000L-offset)){
					exit(0);
				}
				printf("REGISTER BEFORE JUMP=%d\n", num);
				FindInstruction(p, num-0x20000000L-offset);
				
				
				if (p!=NULL)
					printf("%s\n", p->instruction);
				free(string);
			}
			return 2;
		}
	}
	else{
		//printf("WE ARE HERE\n");
		if (strcmp(p->instruction, command)==0){
			
			if ((compare(p->instruction ,"ret"))||(compare(p->instruction,"retn"))||
				(compare(p->instruction, "retf"))||(compare(p->instruction, "iret"))||
				(compare(p->instruction, "rep ret"))){
				int temp;
				if (flag!=99){
					return_addresses.Pop(temp);
					printf("RETADDRESS=%d\n", temp);
					printf("Instruction Ret was found EIP=%d\n", temp+offset+0x20000000L);
					if (!FindInstruction(p, temp)){
						return -1;
					}
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
*/

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
	catch (const char* str){
		printf("%s\n", str);
	}
	catch(...){
		printf("Something strange caught\n");
	}
	return 0;
}
