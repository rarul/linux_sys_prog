TARGETS := iosched getiosched
include ../mk/simple_compile.mk

iosched: iosched.o iosched_util.o
getiosched: getiosched.o iosched_util.o
iosched.o iosched_util.o getiosched.o: iosched_util.h
