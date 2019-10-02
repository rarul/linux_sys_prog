#CFLAGS := -DUSE_SCHED_SETATTR
TARGETS := sched
include ../mk/simple_compile.mk

sched: sched.o sched_util.o
sched.o sched_util.o: sched_util.h
