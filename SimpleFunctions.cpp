#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Headers/SimpleFunctions.h"

using namespace std;


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

bool compare(const char* string1, const char* string2){
	int i=0;
	while((string1[i]!='\0')&&(string2[i]!='\0')){
		if (string1[i]!=string2[i])
			return false;
		i++;
	}
	return true;
}




int condition_command(const char* command, const char*  instruction){
																		//1 both commands are cond commands, or first and second is NULL
																		//0 first command is not cond command
																		//-1 first is true, second is false	
	int flag;
	if((compare(command,"jz"))||(compare(command,"jnz"))||
		(compare(command,"jns"))||(compare(command,"jmp"))||
		(compare(command,"ja"))||(compare(command,"jna"))||
		(compare(command,"jo"))||(compare(command,"jno"))||
		(compare(command,"jnc"))||(compare(command,"jc"))||
		(compare(command,"jl"))||(compare(command,"jnl"))||
		(compare(command,"js"))||(compare(command,"jns"))||
		(compare(command,"jecxz"))||(compare(command,"jp"))||
		(compare(command,"jpo"))||(compare(command,"loopn"))||
		(compare(command,"loope"))||(compare(command,"loopne"))||
		(compare(command,"loopz"))||(compare(command,"loopnz"))||
		(compare(command,"loop"))||(compare(command,"jg"))||
		(compare(command,"jng"))||(compare(command,"jmpf"))){
		flag=1;
	}
	else{
		flag=0;
	}
	if ((instruction==NULL)||(flag==0)){
		return flag;
	}
	if (((compare(instruction,"jz"))&&(!compare(command,"jz")))||
		((compare(instruction,"jnz"))&&(!compare(command,"jnz")))||
		((compare(instruction,"jns"))&&(!compare(command,"jns")))||
		((compare(instruction,"jmp"))&&(!compare(command,"jmp")))||
		((compare(instruction,"ja"))&&(!compare(command,"ja")))||
		((compare(instruction,"jna"))&&(!compare(command,"jna")))||
		((compare(instruction,"jo"))&&(!compare(command,"jo")))||
		((compare(instruction,"jno"))&&(!compare(command,"jno")))||
		((compare(instruction,"jc"))&&(!compare(command,"jc")))||
		((compare(instruction,"jnc"))&&(!compare(command,"jnc")))||
		((compare(instruction,"jl"))&&(!compare(command,"jl")))||
		((compare(instruction,"js"))&&(!compare(command,"js")))||
		((compare(instruction,"jns"))&&(!compare(command,"jns")))||
		((compare(instruction,"jecxz"))&&(!compare(command,"jecxz")))||
		((compare(instruction,"jp"))&&(!compare(command,"jp")))||
		((compare(instruction,"jpo"))&&(!compare(command,"jpo")))||
		((compare(instruction,"loopn"))&&(!compare(command,"loopn")))||
		((compare(instruction,"loope"))&&(!compare(command,"loope")))||
		((compare(instruction,"loopne"))&&(!compare(command,"loopne")))||
		((compare(instruction,"loopz"))&&(!compare(command,"loopz")))||
		((compare(instruction,"loopnz"))&&(!compare(command,"loopnz")))||
		((compare(instruction,"loop"))&&(!compare(command,"loop")))||
		((compare(instruction,"jg"))&&(!compare(command,"jg")))||
		((compare(instruction,"jng"))&&(!compare(command,"jng")))||
		((compare(instruction,"jmpf"))&&(!compare(command,"jmpf")))){
			return -1;
		}
		else{
			return 1;
		}			
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


unsigned int GetNumber (const char* string){
	unsigned int segment=0;
	unsigned int offset=0;
	int koef=16;
	unsigned int number=0;
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
				offset=0;
				break;
			}
		}
		i++;
	}
	number=segment*pow(koef,4)+offset;
	return number;
}
