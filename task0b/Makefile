all: looper

looper: looper.o LineParser.o
	gcc -g -m32 -Wall -o looper looper.o LineParser.o

looper.o: looper.c
	gcc -g -m32 -Wall -c -o looper.o looper.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall -c -o LineParser.o LineParser.c

.PHONY: clean

clean:
	rm -rf ./*.o looper
