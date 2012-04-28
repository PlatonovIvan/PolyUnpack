#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Headers/PE_Reader.h"


Raw_Reader::Raw_Reader(const char* name){
	int i=0;
	filename=new char[strlen(name)+1];
	while(name[i]!='\0'){
		filename[i]=name[i];
		i++;
	}
	filename[i]='\0';
	if ((fp = fopen(filename, "r+b")) == NULL){
		perror(filename);
		//printf("This Error may be because of the symbols in file name!\n");
        exit(1);
    }
}

Raw_Reader::~Raw_Reader(){
	delete []filename;
	fclose(fp);
}

void Raw_Reader::LoadForStaticAnalise(char* &buf, uint& len, uint& start){	
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	int origin=0;
	fseek(fp, 0, origin);
	buf=new char[len];
	if (buf==NULL){
		//printf("Memory error in static analise\n"); 
		exit (2);
	}
	fread(buf, 1, len, fp);
};

void Raw_Reader::LoadForDynamicAnalise(char* &buf, uint& len, uint & start, uint &base, uint & offset){	
	
	start=0;
	base=0x20000000L;
	offset=0;
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	buf=new char[len];
	if (buf==NULL){
		//printf("Memory error  in dynamic analise\n"); 
		exit (2);
	}
	fseek(fp, 0, 0);
	uint result = fread(buf, 1, len, fp);
	if (result!=len){
		//printf("Error  in reading in dynamic analise\n"); 
		exit (2);
	}
	//printf("FILE_SIZE=%d\n", file_size);
};

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
    uint address_mass[number_of_entries];
	uint size_mass[number_of_entries];
	int table_offset_mass[number_of_entries];
	for (int i=0; i<number_of_entries; i++){
		int table_offset=header_offset+248+section_offset;
		fseek(fp, table_offset, origin);
		table_offset_mass[i]=table_offset;
			
		for (int j=0;j<8; j++){
			section_name[j]=getc(fp);
		}
		section_name[8]='\0';
		//printf("Number=%d Section=%s\n", i, section_name);
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
			//printf("Text section has number=%d\n", i);
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

void PE_Reader::LoadForStaticAnalise(char* &buf, uint& len, uint& start){	
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
		//printf("Memory error in static analise\n"); 
		exit (2);
	}
	fread(buf, 1, text_size, fp);
};

void PE_Reader::LoadForDynamicAnalise(char* &buf, uint& len, uint & start, uint &base, uint & offset){	
	PE_HeaderOffset();
	PE_FileType();
	PE_TableOffset();
	PE_NumberOfEntries();
	PE_EntryPoint();
	PE_ImageBase();
	base=image_base;
	base=0x20000000;
	PE_ObjectTable();
	//printf("Entry_point_rva=%d\n", entry_point_rva);
	//printf("Text_address_rva=%d\n", text_address_rva);
	//printf("Text_offset=%d\n", text_offset);
	start=entry_point_rva-text_address_rva+text_offset;
	offset=text_offset;
	//printf("Physical_entry_point!!!=%d\n", start);

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	buf=new char[len];
	if (buf==NULL){
		//printf("Memory error  in dynamic analise\n"); 
		exit (2);
	}
	fseek(fp, 0, 0);
	uint result = fread(buf, 1, len, fp);
	if (result!=len){
		//printf("Error  in reading in dynamic analise\n"); 
		exit (2);
	}
	//printf("FILE_SIZE=%d\n", file_size);
};
