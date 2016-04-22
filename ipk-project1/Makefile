# Author: Pavol Loffay
# Date: 25.02.2012
# Projet: Makefile pre projekt 1. do predmetu IPK

CPP=c++
CPPFLAGS= -std=c++98 -pedantic -Wall -Wextra

all: webinfo 
	
webinfo: webinfo.cc
	$(CPP) $(CPPFLAGS) webinfo.cc -o webinfo

pack:
	tar -f xloffa00.tar -c webinfo.cc Makefile
	gzip xloffa00.tar
#is equal to tar -czf xloffa00.tar.gz Makefile .... 
#to extract tar -xzf xloffa00.tar.gz

clean:
	rm -f webinfo
