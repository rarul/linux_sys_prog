#define _GNU_SOURCE
#include  <unistd.h>
#include  <sys/syscall.h>
#include  "iosched_util.h"

int ioprio_get(int which, int who) {
	return syscall(SYS_ioprio_get, which, who);
}
int ioprio_set(int which, int who, int ioprio) {
	return syscall(SYS_ioprio_set, which, who, ioprio);
}
