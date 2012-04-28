#ifndef SimpleFunctions_H
#define SimpleFunctions_H

void get_register_name(char* & string, const char*  instruction);
int parse(const char* );
bool compare(const char*, const char*);
int condition_command(const char* command, const char* instruction=NULL);
int GetNumber2 (const char* string);
unsigned int GetNumber (const char* string);




#endif
