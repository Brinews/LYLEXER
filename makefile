CC=g++ 
all: proj3
CXXFLAGS=-g

proj3: parser.o lexer.o inputbuf.o
	g++ -g -o $@ $^

clean:
	rm *.o *.exe
