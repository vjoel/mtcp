# Makefile for mtcp and test_mtcp.
#
# make test: makes and runs unit tests
#
# make docs: make docs using doxygen, then browse at docs/html/index.html
#
# make tags: make tags file file using ctags
#

CC = cc

CFLAGS = -c -g -O2 -Wall -I.

# Comment this out for production code:
CFLAGS += -DDEBUG=1

ifeq ($(shell uname),QNX)
  CFLAGS += -DQNX=1
  CFLAGS += -V gcc_ntox86
endif

LINK = cc
LFLAGS = -L.
LIBS = -lm

ifeq ($(shell uname),QNX)
  LIBS += -lsocket
endif

AR = ar
ARFLAGS = rcs

OBJS = mtcp.o

TESTS = test-mtcp.test
BINARIES = $(TESTS)

all: depend tags $(BINARIES)

%.test: %.o $(OBJS)
	$(LINK) $(LFLAGS) $(LIBS) $*.o $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) $*.c -o $*.o

test: $(TESTS)
	run-tests.sh $(TESTS)

docs: Doxyfile *.[ch]
	@ echo "making docs"
	@ doxygen Doxyfile

tags: *.[ch]
	@ echo "making ctags"
	@ ctags -R

depend: *.[ch]
	@ touch depend
	@ makedepend -fdepend *.c 2>makedepend.err

clean:
	\rm -f *.o *.err $(BINARIES) $(OBJS)

distclean: clean
	\rm -f depend tags *.bck *.bak
	\rm -rf doc

.PHONY: clean docs test

include depend
