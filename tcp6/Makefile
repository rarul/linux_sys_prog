TARGETS = server client
LDLIBS = -lpthread
include ../mk/simple_compile.mk

server: server.o util.o
client: client.o util.o
server.o util.o client.o: util.h
