# Makefile
# Copyright (c) 2019, Fudan University

LIBDIR = $(LIBGISTHOME)/lib/
INCLUDEDIR = $(LIBGISTHOME)/include/

LIBS = $(LIBGISTHOME)/lib/libgist.a \
    $(LIBGISTHOME)/lib/libbtree.a \
    $(LIBGISTHOME)/lib/librtree.a \
    $(LIBGISTHOME)/lib/libnptree.a

# -lstdc++ needed for the sww installation (Sparc Solaris)
CMDLINELIBS = -lnptree -lbtree -lgist -lrtree -lm -lstdc++
CFLAG = -Wcpp -g

DEST = .

SRCS = SRTree.cc
OBJS = SRTree.o

PROG = $(DEST)/SRTree

$(PROG): $(LIBS)
	gcc $(SRCS) -I $(INCLUDEDIR) -L$(LIBDIR) $(CMDLINELIBS) $(CFLAG) -o $(PROG)

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f $(PROG)
