# File : Makefile
# Author: Pavol Loffay, xloffa00@@stud.fit.vutbr.cz
# Date: 25.9.2012
# Project: Projekt do predmetu ISA, 

CC=c++
CXXFLAGS=-std=c++98 -pedantic -Wall

all:mypmtud

mtu_ipv6.o:mtu_ipv6.cc mtu_ipv6.h
	$(CC) $(CXXFLAGS) -c mtu_ipv6.cc -o mtu_ipv6.o

mtu_ipv4.o:mtu_ipv4.cc mtu_ipv4.h
	$(CC) $(CXXFLAGS) -c mtu_ipv4.cc -o mtu_ipv4.o

checksum.o:checksum.cc checksum.h
	$(CC) $(CXXFLAGS) -c checksum.cc -o checksum.o

ip.o:ip.cc ip.h
	$(CC) $(CXXFLAGS) -c ip.cc -o ip.o

conversion.o:conversion.cc conversion.h
	$(CC) $(CXXFLAGS) -c conversion.cc -o conversion.o

params.o:params.h params.cc
	$(CC) $(CXXFLAGS) -c params.cc -o params.o

###########################################################

mypmtud.o:mypmtud.cc params.h conversion.h ip.h checksum.h mtu_ipv4.h mtu_ipv6.h
	$(CC) $(CXXFLAGS) -c mypmtud.cc -o mypmtud.o

mypmtud:mypmtud.o params.o conversion.o ip.o checksum.o mtu_ipv4.o mtu_ipv6.o
	$(CC) $(CXXFLAGS) mypmtud.o params.o conversion.o checksum.o ip.o mtu_ipv4.o mtu_ipv6.o -o mypmtud

pack:
	tar -cf xloffa00.tar *.cc *.h Makefile Readme manual.pdf

clean:
	rm -f *.o mypmtud

