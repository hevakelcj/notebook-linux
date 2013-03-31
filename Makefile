Target = notebook
CC = g++
INCLUDE = . 
CFLAGS = 

$(Target) : main.o Encrypt.o Interface.o NoteBook.o NoteFile.o
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(Target) main.o Encrypt.o Interface.o NoteBook.o NoteFile.o

main.o : main.cpp Encrypt.h NoteBook.h VirtualArray.h Interface.h
	$(CC) $(CFLAGS) -I$(INCLUDE) -c main.cpp 

Encrypt.o : Encrypt.cpp Encrypt.h VirtualArray.h
	$(CC) $(CFLAGS) -I$(INCLUDE) -c Encrypt.cpp

Interface.o : Interface.cpp Interface.h
	$(CC) $(CFLAGS) -I$(INCLUDE) -c Interface.cpp

NoteBook.o : NoteBook.cpp NoteBook.h Interface.h Encrypt.h NoteFile.h
	$(CC) $(CFLAGS) -I$(INCLUDE) -c NoteBook.cpp

NoteFile.o : NoteFile.cpp NoteFile.h Encrypt.h
	$(CC) $(CFLAGS) -I$(INCLUDE) -c NoteFile.cpp   

install:
	cp $(Target) /usr/local/sbin

clean :
	-rm *.o 
