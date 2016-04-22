#Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
#Date: 14.04.2012
#Project: Makefile pre projekt 3. do predmetu IPK

CPP=c++
CC=gcc

CPPFLAGS= -std=c++98 -pedantic #-Wall -Wextra
SERVER=rdtserver
CLIENT=rdtclient

all: server client

window.o: window.cc window.h
	$(CPP) $(CPPFLAGS) -c window.cc -o window.o

packet.o: packet.cc packet.h
	$(CPP) $(CPPFLAGS) -c packet.cc -o packet.o

params.o: params.cc params.h
	$(CPP) $(CPPFLAGS) -c params.cc -o params.o 

base64.o: base64.cc base64.h
	$(CPP) $(CPPFLAGS) -c base64.cc -o base64.o

server.o: $(SERVER).cc params.h base64.h packet.h window.h
	$(CPP) $(CPPFLAGS) -c $(SERVER).cc -o $(SERVER).o

server: $(SERVER).o params.o base64.o packet.o window.o
	$(CPP) $(CPPFLAGS) $(SERVER).o params.o base64.o packet.o window.o -o $(SERVER)


client.o: $(CLIENT).cc params.h base64.h packet.h window.h
	$(CPP) $(CPPFLAGS) -c $(CLIENT).cc -o $(CLIENT).o


client: $(CLIENT).o params.o  base64.o packet.o window.o
	$(CPP) $(CPPFLAGS) $(CLIENT).o params.o base64.o packet.o window.o -o $(CLIENT)

pack:
	tar -czf xloffa00.tar.gz Makefile readme.xml *.cc *.h

clean:
	rm -f $(CLIENT) $(SERVER) *.o


