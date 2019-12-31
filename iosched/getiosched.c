#include  <stdio.h>
#include  <stdlib.h>
#include  "iosched_util.h"

static const char *iosched_policy_to_str(int ioprio) {
	char *p = NULL;
	switch(IOPRIO_PRIO_CLASS(ioprio)) {
		case IOPRIO_CLASS_NONE:
			p = "IOPRIO_CLASS_NONE";
			break;
		case IOPRIO_CLASS_RT:
			p = "IOPRIO_CLASS_RT";
			break;
		case IOPRIO_CLASS_BE:
			p = "IOPRIO_CLASS_BE";
			break;
		case IOPRIO_CLASS_IDLE:
			p = "IOPRIO_CLASS_IDLE";
			break;
		default:
			p = "(UNKNOWN)";
			break;
	}
	return p;
}
static void do_ioprio_get(pid_t pid) {
	int ioprio = ioprio_get(IOPRIO_WHO_PROCESS, pid);
	if (ioprio < 0) {
		perror("ioprio_get");
		return;
	}
	printf("pid %d: %s %lu\n", pid,
		   iosched_policy_to_str(ioprio), IOPRIO_PRIO_DATA(ioprio));
}

int main(int argc, char *argv[]) {
	pid_t pid = 0;
	if (argc != 2) {
		fprintf(stderr, "requires [PID]\n");
		return 1;
	}
	pid = atoi(argv[1]);
	do_ioprio_get(pid);
	return 0;
}
