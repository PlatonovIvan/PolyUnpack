#ifndef Stack_H
#define Stack_H

struct List_of_Return_Addresses{
	int address;
	List_of_Return_Addresses* pointer;
};

class Stack{
	List_of_Return_Addresses *start;
public:
	Stack();
	void Push(const int);
	void Pop(int &);
	void Print();
	~Stack();
};

#endif
