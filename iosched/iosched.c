#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  <err.h>
#include  "iosched_util.h"

#define IOPRIO_NORM (4)
#define ARRAYLENGTH(val)  (sizeof(val)/sizeof(val[0]))
#define SYSCALLWRAP(SNAME, ...)								\
	do {													\
		int ret;											\
		ret = SNAME(__VA_ARGS__);							\
		if (ret) {											\
			perror(#SNAME);									\
		}													\
	} while(0)

static int iosched_str_to_class(const char *str) {
	struct iosched_str_class_t {
		int class;
		const char *str;
	} iosched_str_class_array[] = {
		{IOPRIO_CLASS_NONE, "none"},
		{IOPRIO_CLASS_RT, "realtime"},
		{IOPRIO_CLASS_BE, "best-effort"},
		{IOPRIO_CLASS_IDLE, "idle"}
	};
	for(unsigned int i=0; i<ARRAYLENGTH(iosched_str_class_array); i++) {
		if (strcmp(str, iosched_str_class_array[i].str) == 0) {
			return iosched_str_class_array[i].class;
		}
	}
	fprintf (stderr, "unknown class name: %s\n", str);
	return -1;
}
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

static void do_ioprio_set(int ioprio_class, int ioprio_data) {
	SYSCALLWRAP(ioprio_set, IOPRIO_WHO_PROCESS, 0,
				IOPRIO_PRIO_VALUE(ioprio_class, ioprio_data));
}

static void do_ioprio_get() {
	int ioprio = ioprio_get(IOPRIO_WHO_PROCESS, 0); // 0: myself
	if (ioprio < 0) {
		perror("ioprio_get");
		return;
	}
	printf("%s %lu\n", iosched_policy_to_str(ioprio), IOPRIO_PRIO_DATA(ioprio));
}

int main(int argc, char *argv[]) {
	int c;
	int ioprio_class = IOPRIO_CLASS_BE;
	int ioprio_data = IOPRIO_NORM;
	while ((c=getopt(argc, argv, "c:")) != -1) {
		switch (c) {
			case 'c':
				ioprio_class = iosched_str_to_class(optarg);
				break;
			default:
				errx(1, "Invald arg: %d", c);
				break;
		}
	}
	if ( (optind+1) != argc) {
		fprintf(stderr, "iosched [-c class] LEVEL\n");
		return 1;
	}
	ioprio_data = atoi(argv[optind]);
	if (ioprio_class == IOPRIO_CLASS_NONE || ioprio_class == IOPRIO_CLASS_IDLE) {
		ioprio_data = 0;
	}
	
	do_ioprio_set(ioprio_class, ioprio_data);
	do_ioprio_get();
	
	return 0;
}
