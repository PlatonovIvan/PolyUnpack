#ifndef PE_Reader_H
#define PE_Reader_H

class Reader{
public:
	virtual void LoadForStaticAnalise(char* &, unsigned int&, unsigned int&)=0;
	virtual void LoadForDynamicAnalise(char* &, unsigned int&, unsigned int&, unsigned int&, unsigned int&)=0;
};

class Raw_Reader:public Reader{
	char* filename;
	FILE* fp;
public:
	Raw_Reader(const char*);
	virtual void LoadForStaticAnalise(char* &, unsigned int&, unsigned int&);
	virtual void LoadForDynamicAnalise(char* &, unsigned int&, unsigned int&, unsigned int&, unsigned int&);
	~Raw_Reader();
};

class PE_Reader:public Reader{
	char* filename;
	FILE *fp;
	int header_offset;
	int table_offset;
	int number_of_entries;
	unsigned int text_offset;
	unsigned int data_offset;
	unsigned int text_size;
	unsigned int data_size;
	unsigned int entry_point_rva;
	unsigned int image_base;
	unsigned int text_address_rva;
public:
	PE_Reader(const char* name);
	PE_Reader(const PE_Reader* pe_reader);
	virtual ~PE_Reader();
    virtual void LoadForStaticAnalise(char* &, unsigned int&, unsigned int&);
	virtual void LoadForDynamicAnalise(char* &, unsigned int&, unsigned int&, unsigned int&, unsigned int&);
	//void PE_Read(char* &, int&, int&);
	void PE_HeaderOffset();
	void PE_TableOffset();
	void PE_FileType() const;
	void PE_NumberOfEntries();
	void PE_EntryPoint();
	void PE_ImageBase();
	void PE_ObjectTable();
	void PE_CodeSection(int);
	void PE_DataSection(int);
};

/*
class PE_Reader{
	char* filename;
	FILE *fp;
	int header_offset;
	int table_offset;
	int number_of_entries;
	uint text_offset;
	uint data_offset;
	uint text_size;
	uint data_size;
	uint entry_point_rva;
	uint image_base;
	uint text_address_rva;
public:
	PE_Reader(const char* name);
	PE_Reader(const PE_Reader* pe_reader);
	~PE_Reader();
    void PE_LoadForStaticAnalise(char* &, int&, int&);
	void PE_LoadForDynamicAnalise(char* &, int&, int&, int&);
	//void PE_Read(char* &, int&, int&);
	void PE_HeaderOffset();
	void PE_TableOffset();
	void PE_FileType() const;
	void PE_NumberOfEntries();
	void PE_EntryPoint();
	void PE_ImageBase();
	void PE_ObjectTable();
	void PE_CodeSection(int);
	void PE_DataSection(int);
};
*/

#endif
