#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Headers/PolyUnpack.h"

extern "C"{
	#include "libdasm/libdasm.h"
}
using namespace std;

void Instruction_Tree:: Add(InstructionList* & po1, InstructionList* & po2, InstructionList* & po3){
	Instruction_Leaf* p;
	if (root==NULL){
		root=new Instruction_Leaf;
		root->pointer=po1;
		root->valid=1;
		if (po2!=NULL){
			//printf("Left Leaf=%s\n", po2->instruction);
			root->left=new Instruction_Leaf;
			p=root->left;
			p->pointer=po2;
			p->valid=0;
			p->left=NULL;
			p->right=NULL;
		}
		else{
			//printf("Left Leaf=NULL");
			root->left=NULL;
		}
		if (po3!=NULL){
			//printf("Right Leaf=%s\n", po3->instruction);
			root->right=new Instruction_Leaf;
			p=root->right;
			p->pointer=po3;
			p->valid=0;
			p->left=NULL;
			p->right=NULL;
		}
		else{
			//printf("Right Leaf=NULL\n");
			root->right=NULL;
		}
	}
}

int Instruction_Tree::Obhod(const char* command, PolyUnpack& unpack){
	int kol=0;
	int  iterator=0;
	int result=0;
	//Obhod2(root);
	kol=0;
	iterator=0;
	result=0;
	InstructionList* original=NULL;
	Obhod(root, command, original, unpack, kol, iterator, result);
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
		if (condition_command(command)==1){
			bool flag=true;
			InstructionList* po1=NULL;
			InstructionList* po2=NULL;
			InstructionList* po3=NULL;
			Check(root, po1, po2, po3, original, flag);
			if (flag==true){
				Destructor(root);
				//root=NULL;
				//InstructionList* po2=NULL;
				//InstructionList* po3=NULL; 
				int iterator=0;
				//printf("po1->instruction=%s\n", po1->instruction);
				//printf("po2->instruction=%s\n", po2->instruction);
				//printf("po3->instruction=%s\n", po3->instruction);
				Add(po1, po2, po3);
				//Obhod2(root);
			}
		}
		return result;
}

void Instruction_Tree::Obhod(Instruction_Leaf* & p1, const char* command,
							InstructionList* & original, PolyUnpack & unpack, 
									int & kol, int & iterator, int & result){
	Instruction_Tree* p2;
	if ((p1->left!=NULL)&&(p1->right!=NULL)){
		if (p1->left->valid!=-1){
			Obhod(p1->left, command, original, unpack, kol, iterator, result);
		}
		if (p1->right->valid!=-1){
			Obhod(p1->right, command, original, unpack, kol, iterator, result);
		}
		if ((p1->left->valid==-1)&&(p1->right->valid==-1))
			p1->valid=-1;
		return;
	}
	if ((p1->left!=NULL)&&(p1->right==NULL)){
		if (p1->left->valid!=-1){
			Obhod(p1->left, command, original, unpack, kol, iterator, result);
		}
		if (p1->left->valid==-1)
			p1->valid=-1;
		return;
	}
	if ((p1->left==NULL)&&(p1->right!=NULL)){
		if (p1->right->valid!=-1)
			Obhod(p1->right, command, original, unpack, kol, iterator, result);
		if (p1->right->valid==-1)
			p1->valid=-1;
		return;
	}
	if ((p1->left==NULL)&&(p1->right==NULL)){
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
				//printf("%s\n", po2->instruction);
				p1->valid=-1;
				return;
			}
			if ((flag==1)||(flag==2)||(flag==3)){
				original=p1->pointer;
				p1->valid=1;
				p1->left=new Instruction_Leaf;
				Instruction_Leaf* p2=p1->left;
				p2->pointer=po2;
				p2->valid=0;
				p2->left=NULL;
				p2->right=NULL;
				kol++;
				return;
			}
			if((flag==4)||(flag==5)){
				Destructor(root);
				return;
			}
			
			if (flag==0){
				
				original=p1->pointer;
				p1->valid=1;
				p1->left=new Instruction_Leaf;
				Instruction_Leaf* p2=p1->left;
				p2->pointer=po2;
				p2->valid=0;
				p2->left=NULL;
				p2->right=NULL;
				if (po3!=NULL){
					//printf("Right Leaf %s\n", po3->instruction);
					p1->right=new Instruction_Leaf;
					p2=p1->right;
					p2->pointer=po3;
					p2->valid=0;
					p2->left=NULL;
					p2->right=NULL;
				}
				else {
					//printf("Right Leaf=NULL\n");
				}
				kol++;
			}
		}
	}
}


void Instruction_Tree::Obhod2(Instruction_Leaf* & p1){
	Instruction_Tree* p2;
	if ((p1->left!=NULL)&&(p1->right!=NULL)){
		//printf("(p1->left!=NULL)&&(p1->right!=NULL)\n");
		//printf("instruction=%s\n valid=%d\n", p1->pointer->instruction, p1->valid);
		
		Obhod2(p1->left);
		Obhod2(p1->right);

		return;
	}
	if ((p1->left!=NULL)&&(p1->right==NULL)){
		//printf("(p1->left!=NULL)&&(p1->right==NULL)\n");
		//printf("instruction=%s\n valid=%d\n", p1->pointer->instruction, p1->valid);
		
		Obhod2(p1->left);
		
		return;
	}
	if ((p1->left==NULL)&&(p1->right!=NULL)){
		//printf("(p1->left==NULL)&&(p1->right!=NULL)\n");
		//printf("instruction=%s\n valid=%d\n", p1->pointer->instruction, p1->valid);
		
		Obhod2(p1->right);
		
		return;
	}
	if ((p1->left==NULL)&&(p1->right==NULL)){
		//printf("(p1->left==NULL)&&(p1->right==NULL)\n");
		//printf("instruction=%s\n valid=%d\n", p1->pointer->instruction, p1->valid);
		
		return;
	}
}

int Instruction_Tree::Check(Instruction_Leaf* & p1, InstructionList* & po1, 
							InstructionList* & po2, InstructionList* & po3, 
								InstructionList* & original, bool & flag){
	if ((p1->left!=NULL)&&(p1->right!=NULL)){
		//printf("(p1->left!=NULL)&&(p1->right!=NULL)\n");
		//printf("instruction=%s\n valid=%d\n", p1->pointer->instruction, p1->valid);
		if (p1->left->valid!=-1){
			int temp=Check(p1->left, po1, po2, po3, original, flag);
			if (temp==1){
				po1=p1->pointer;
				po2=p1->left->pointer;
				//printf("p1->pointer->instruction=%s\n", p1->pointer->instruction);
				//printf("command=%s\n", original->instruction);
				if (strcmp(p1->pointer->instruction, original->instruction)!=0){
					flag=false;
				}		 
			}
		}
		if (p1->right->valid!=-1){
			int temp=Check(p1->right, po1, po2, po3, original, flag);
			if (temp==1){
				po1=p1->pointer;
				po3=p1->right->pointer;
				//printf("p1->pointer->instruction=%s\n", p1->pointer->instruction);
				//printf("command=%s\n", original->instruction);
				if (strcmp(p1->pointer->instruction, original->instruction)!=0){
					flag=false;
				}		 
			}
		}
		return 0;
	}
	
	if ((p1->left!=NULL)&&(p1->right==NULL)){
		//printf("(p1->left!=NULL)&&(p1->right==NULL)\n");
		//printf("instruction=%s\n valid=%d\n", p1->pointer->instruction, p1->valid);
		if (p1->left->valid!=-1){
			int temp=Check(p1->left, po1, po2, po3, original, flag);
			if (temp==1){
				po1=p1->pointer;
				po2=p1->left->pointer;
				//printf("p1->pointer->instruction=%s\n", p1->pointer->instruction);
				//printf("command=%s\n", original->instruction);
				if (strcmp(p1->pointer->instruction, original->instruction)!=0){
					flag=false;
				}		 
			}
		}
		return 0;
	}
	
	if ((p1->left==NULL)&&(p1->right!=NULL)){
		//printf("(p1->left==NULL)&&(p1->right!=NULL)\n");
		//printf("instruction=%s\n valid=%d\n", p1->pointer->instruction, p1->valid);
		if (p1->right->valid!=-1){
			int temp=Check(p1->right, po1, po2, po3, original, flag);
			if (temp==1){
				po1=p1->pointer;
				po3=p1->right->pointer;
				//printf("p1->pointer->instruction=%s\n", p1->pointer->instruction);
				//printf("command=%s\n", original->instruction);
				if (strcmp(p1->pointer->instruction, original->instruction)!=0){
					flag=false;
				}		 
			}
		}
		return 0;
	}
	if ((p1->left==NULL)&&(p1->left==NULL)&&(p1->right==NULL)&&(p1->right==NULL)){
		//printf("(p1->left==NULL)&&(p1->right==NULL)\n"); 
		return 1;
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




PolyUnpack::PolyUnpack(){
	start=NULL;
	n=4;
	buf_size=0;
	base=0;
	code_size=0;
	offset=0;
	step_kol=3500;
	invalid_instr_kol=0;
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

void PolyUnpack::StaticAnalise(const char* FileName, const int key){
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	InstructionList* p1=NULL; 
	char *data=NULL;
	int i=0, k=n, c=0, bytes=8, len=0, number=0;
	uint size=0;
	uint entry_point=0;
	char string[32];
	bool flag=false;
	PE_Reader reader(FileName);
	//Raw_Reader reader(FileName);
	reader.LoadForStaticAnalise(data, size, entry_point);
	code_size=size;
	//printf("Static analize entry=%d\n", entry_point);
	if(size==0){
		//printf("There is no code section in this file\n");
		exit(-1);
	}
	while (c < size) {
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


void PolyUnpack::DynamicAnalise(const char* FileName, const int key){
	INSTRUCTION inst;	// declare struct INSTRUCTION
	INSTRUCTION* inst_pointer=NULL;
	InstructionList* p=start;
	bool good_parse=true;
	bool valid_file=true;
	//int invalid_instr_kol=0;
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
	uint entry_point=0;
	PE_Reader reader(FileName);
	//Raw_Reader reader(FileName);
	reader.LoadForDynamicAnalise(buf, buf_size, entry_point, base, offset);
	emulator->begin(buf, buf_size, entry_point, base);
	
	if(!FindFirstInstruction(p)){
		//printf("Can't find First instruction\n");
		//exit(33);
	}

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
			emulator->jump(old_eip_pointer+1-base);
			if(kol>=step_kol)
				break;
			continue;
		}
 
		inst_pointer=&inst;
		c=0;//may be like this!
		get_instruction_string(inst_pointer, Format(FORMAT_INTEL), (DWORD)c, string, string_size);
		flag=Compare(p, string, flag, old_eip_pointer);
		if(flag==-1){
			exit(101);
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
				emulator->jump(old_eip_pointer+len-base);
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
			if ((flag==4)||(flag==5))
				break;
		}
		//emulator.jump(old_eip_pointer+len-0x20000000L);
		//printf("**************************************\n");
		if((flag==1)||(flag==2)){
			//printf("We are in bad parse brunch\n");
			emulator->jump(old_eip_pointer+len-base);
		}
		 
		for (int i=0; i<string_size; i++)
			string[i]='\0';
		for (int i=0; i<command_size; i++)
			command[i]='\0';
		
		if(((p!=NULL)&&(p->padding))||(kol>=step_kol))
			break;
		kol++;
	}
	CompareBuffers(buf);
	delete []string;
	delete []command;
	delete []buf;
	if (valid_file){
		//printf("This file is valid\n");
		exit(0);
	}
	else{
		//printf("This is VIRUS\n");
		//printf("Kol of invalid instructions=%d\n", invalid_instr_kol);
		exit(1);
	}
	//printf("***************\n");
}


bool PolyUnpack::CompareBuffers(const char* old_buf){
	printf("We are here\n");
	printf("+++++++++++++++++++++++++++++++++++\n");
	//printf("%s\n", old_buf);
	
	char* new_buf=new char[buf_size];
	printf("buf_size=%d\n", buf_size);
	emulator->read_block(new_buf, buf_size);
	printf("+++++++++++++++++++++++++++++++++++\n");
	//printf("%s\n", new_buf);
	int i;
	for (i=offset; i<offset+code_size; i++){
		if (new_buf[i]!=old_buf[i]){
			printf("Herovo");
		}
	}
	printf("i=%d\n", i);
	delete []new_buf;
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



int PolyUnpack::Compare(InstructionList* &p, const char* command, int flag, const uint old_eip){
	if (root.Is_Empty()){
		//printf("root=NULL\n");
		if (p==NULL){
			if(!FindInstruction(p, old_eip-offset-base))
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
		FindInstruction(p, emulator->get_register(reg)-offset-base);
		return flag;
	}	
}


int PolyUnpack::FindNextInstruction(InstructionList* &p, InstructionList* & p2, const char* command, int & iterator){
	//printf(" INSTRUCTION=%s\n", p->instruction);
	//printf(" COMMAND=%s\n", command);
	 
	if ((compare(p->instruction,"call"))||(compare(p->instruction,"callf"))){
		if ((!compare (command, "call"))&&(!compare (command, "callf")))
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
				return 3;
			}
			else{
				char* string;
				int i=parse(p->instruction);
				get_register_name(string, p->instruction+i);
				int num=emulator->get_register(string);
				delete []string;
				//printf("Register=%d\n", num);
				if (!is_valid_number(num-base-offset)){
					//exit(0);
					return 5;
				}
				//printf("REGISTER BEFORE CALL=%d\n", num);		
				if (!FindInstruction(p, num-base-offset))
					return -1; //so that this instruction will be unvalid but not next
				//printf("%s\n", p->instruction);
				return 3;
			}
		}
	}
	//printf(" INSTRUCTION=%s\n", p->instruction);
	//printf(" COMMAND=%s\n", command);
	int cond_com=condition_command(p->instruction, command);
	if (cond_com==1){
		
		if(compare(p->instruction, "jmpf")){
			printf("jmpf was found\n");
			p=NULL;
			return 0;
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
				return 3;
			}
			else{
				char* string;
				get_register_name(string, p->instruction+i);
				uint num=emulator->get_register(string);
				//printf("REGISTER BEFORE JUMP=%d\n", num);
				delete []string;
				if (!is_valid_number(num-base-offset)){
					return 5;
					//exit(0);// in register case this is good
				}
				if (!FindInstruction(p, num-base-offset))
					return -1;
				return 1;
			}
		}
		else{
			int i=parse(p->instruction);
			if (p->instruction[i]=='0'){
				uint number=GetNumber(p->instruction+i+2);	
				//InstructionList* p2;
				if (FindInstruction(p2, number)){
		
					if (root.Is_Empty())
						root.Add(p, p->pointer, p2);
					else{
						//printf("p=p->pointer=%s\n", p->pointer->instruction);
						//printf("p2=%s\n", p2->instruction);
						p=p->pointer;
					}
					
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
				//printf("REGISTER BEFORE JUMP=%d\n", num);
				if (!is_valid_number(num-base-offset)){
					return 5;
					exit(0);//good in register case
				}
				//InstructionList* p2;
				if (FindInstruction(p2, num-base-offset)){
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
	if (cond_com==-1){
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
				if (temp==-1){
					p=NULL;
					return 4;
					//printf("Stack is empty\n");
				}
				//return_addresses.Print();
				//printf("Instruction Ret was found EIP=%d\n", temp+offset+0x20000000L);
				if (!FindInstruction(p, temp)){
					return -1;
				}
				emulator->jump(temp+offset);
				return 3;
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
