RM	= rm -f
CC	= g++
#CFLAGS	= -g -Wall -std=c++0x
CFLAGS	= -O3 -Wall -std=c++0x

#LDOPTS	= -g
LDOPTS	=

GLUTLIBS= -lglut
GLLIBS	= -lGL -lGLU -lGLEW
XLIBS	= -lX11 -lm
LIBS	= $(GLUTLIBS) $(GLLIBS) $(XLIBS)

OBJS	= main.o digraph.o iw_ungraph.o
TARGETS	= main

all::	$(TARGETS)

main:	$(OBJS)
	$(RM) $@
	$(CC) -o $@ $(LDOPTS) $(OBJS) $(LIBS)

clean::
	$(RM) $(TARGETS) $(OBJS)

%.o : %.cpp
	$(CC) -c $(CFLAGS) $< -o $@
