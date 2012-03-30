CXX = g++

all: SecondPartDisas9

clean:
	rm -f *.o *~

SecondPartDisas9: SecondPartDisas9.o
	$(CXX) -o $@ SecondPartDisas9.o libdasm/libdasm.o -lemu -L$(CURDIR)/libemu -Wl,-rpath -Wl,$(CURDIR)/libemu

SecondPartDisas9.o: SecondPartDisas9.cpp
	$(CXX) -o $@ -c SecondPartDisas9.cpp -I$(CURDIR)/libemu/include

