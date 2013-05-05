CFLAGS := -g -pthread -lm -lSDL
targets=client uitest
src = client.c
objs = $(patsubst %.c,%.o,$(src))
libs = -L../lib -ldagame

all: $(targets)
.PHONY: all

client: $(objs) ../lib/net.c
	gcc $(CFLAGS) -o client $(objs) $(libs)

uitest: uitest.o
	gcc $(CFLAGS) -o uitest uitest.o $(libs)

$(objs) : $(src)

clean:
	rm -f *.o $(targets)