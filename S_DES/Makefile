all: mycipher

mycipher: mycipher.o functions.o
	gcc -g $< -o $@ functions.o

mycipher.o: mycipher.c functions_header.h
	gcc -g $< -c mycipher.c
	
functions.o: functions.c functions_header.h
	gcc -g $< -c functions.c
	
clean:
	rm -f mycipher *.o *~ core
	
	

