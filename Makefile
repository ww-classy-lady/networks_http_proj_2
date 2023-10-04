# (i) Wendy Wu
# (ii) wxw428
# (iii) Makefile
# (iv) 9/25/2023
# (v) Description:
# make: will create an executable called proj2 to run proj2.c
# followed format from sample.c's makefile
CC=gcc
CXX=g++
LD=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=$(CFLAGS)

TARGETS=proj2

all: $(TARGETS)

proj2: proj2.o
	$(CC) $(CFLAGS) -o $@ $< 

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CXX) $(CFLAGS) -c $<

clean:
	rm -f *.o

distclean: clean
	rm -f $(TARGETS)
