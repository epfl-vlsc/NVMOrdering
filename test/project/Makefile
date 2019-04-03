CXX=clang++
FLAGS=-std=c++11

LINK_TARGET = main

OBJS = nvm.o scl.o dcl.o main.o


all : $(LINK_TARGET)
	echo All done

$(LINK_TARGET) : $(OBJS)
	$(CXX) $(FLAGS) -g -o $@ $^

%.o : %.cpp
	$(CXX) $(FLAGS) -g -o $@ -c $<

scl.o: scl.h
dcl.o: dcl.h
nvm.o: nvm.h

clean:
	rm *.o main
