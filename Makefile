TARGETS = lib/libdagame.a client/client server/server

all: $(TARGETS)

lib/libdagame.a:
	cd lib/; make

client/client:
	cd client/; make

server/server:
	cd server/; make

clean:
	cd lib/; make clean; cd ..; cd client/; make clean; cd ..; cd server/; make clean;