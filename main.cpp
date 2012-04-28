#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Headers/PolyUnpack.h"

using namespace std;


int main(int argc, char** argv){
	try{
		PolyUnpack Unpack;
		if ((argc<2)||(argc>4)){
			throw argv[0];
		}
		if (argc==2){
			Unpack.StaticAnalise(argv[1], 0);
			Unpack.DynamicAnalise(argv[1], 0);
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
					Unpack.DynamicAnalise(argv[2], 0);
				}
				if ((strcmp(argv[1],"-sd")==0)||(strcmp(argv[1],"-ds")==0)){
					Unpack.StaticAnalise(argv[2], 1);
					Unpack.DynamicAnalise(argv[2], 1);
				}
				if((strcmp(argv[1],"-d")==0)||(strcmp(argv[1],"--dynamic")==0)){ 
					Unpack.StaticAnalise(argv[2], 0);
					Unpack.DynamicAnalise(argv[2],1);
				}
			}
			
		}
		if (argc==4){
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

