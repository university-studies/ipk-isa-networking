# Author: Pavol Loffay
# Date: 25.02.2012
# Projet: Makefile pre projekt 2. do predmetu IPK

CPP=c++
CPPFLAGS= -std=c++98 -pedantic -Wall -Wextra
NAME_PR=protokol

all: server client 
	
server: server.cc
	$(CPP) $(CPPFLAGS) server.cc -o server

client: client.cc
	$(CPP) $(CPPFLAGS) client.cc -o client

#dokumentacia
doc: protokol.tex
	latex $(NAME_PR).tex
	dvips -t a4 $(NAME_PR).dvi
	ps2pdf $(NAME_PR).ps

#is equal to tar -czf xloffa00.tar.gz Makefile .... 
#to extract tar -xzf xloffa00.tar.gz
pack:
	tar -f xloffa00.tar -c server.cc client.cc Makefile $(NAME_PR).pdf
	gzip xloffa00.tar

pack_source:
	zip xloffa00-source.zip Makefile server.cc client.cc $(NAME_PR).tex

clean:
	rm -f client server $(NAME_PR).dvi $(NAME_PR).ps $(NAME_PR).aux $(NAME_PR).log $(NAME_PR).pdf
