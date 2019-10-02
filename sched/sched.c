#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <err.h>
#include  <unistd.h>
#include  <sched.h>
#include  <sys/time.h>
#include  <sys/resource.h>
#include  <sys/errno.h>
#include  <linux/sched.h>

#include  "sched_util.h"

// Note: we call "#" as "Stringification operator"
// Note: we call "#" as "Token Pasting Operator"
#define SYSCALLWRAP(SNAME, ...)								\
	do {													\
		int ret;											\
		ret = SNAME(__VA_ARGS__);							\
		if (ret) {											\
			perror(#SNAME);									\
		}													\
	} while(0)
#define MEMSET(val) memset(&val,0,sizeof(val))

static void do_sched_setattr(const int sched_policy, const int sched_prio) {
	int ret;
	struct sched_attr sattr;
	MEMSET(sattr);
	sattr.size = sizeof(sattr);
	sattr.sched_policy = sched_policy;
	//In usually, SCHED_FLAG_RESET_ON_FORK is not required.
	//sattr.sched_flags = SCHED_FLAG_RESET_ON_FORK;
	switch (sched_policy) {
		case SCHED_OTHER:
		case SCHED_BATCH:
		case SCHED_IDLE:
			sattr.sched_nice = sched_prio;
			break;
		case SCHED_FIFO:
		case SCHED_RR:
			sattr.sched_priority = sched_prio;
			break;
		case SCHED_DEADLINE:
			// we should keep period >= deadline >= sched_runtime
			sattr.sched_runtime = 100 * 1000; // nsec
			sattr.sched_deadline = 200 * 1000; // nsec
			sattr.sched_period = 1000 * 1000; // nsec
			break;
		default:
			errx(1, "Invald SCHED: %d", sched_policy);
			break;
	}
	SYSCALLWRAP(sched_setattr, 0, &sattr, 0);
}
static const char *sched_policy_to_str(const int sched_policy) {
	char *p = NULL;
	switch (sched_policy) {
		case SCHED_OTHER:
			p = "SCHED_OTHER";
			break;
		case SCHED_BATCH:
			p = "SCHED_BATCH";
			break;
		case SCHED_IDLE:
			p = "SCHED_IDLE";
			break;
		case SCHED_FIFO:
			p = "SCHED_FIFO";
			break;
		case SCHED_RR:
			p = "SCHED_RR";
			break;
		case SCHED_DEADLINE:
			p = "SCHED_DEADLINE";
			break;
		default:
			errx(1, "Invald SCHED: %d", sched_policy);
			break;
	}
	return p;
}
static void do_sched_getattr(){
	struct sched_attr sattr;
	MEMSET(sattr);

	SYSCALLWRAP(sched_getattr, 0, &sattr, sizeof(sattr), 0);

	switch (sattr.sched_policy) {
		case SCHED_OTHER:
		case SCHED_BATCH:
		case SCHED_IDLE:
			printf("%s %d\n", sched_policy_to_str(sattr.sched_policy), sattr.sched_nice);
			break;
		case SCHED_FIFO:
		case SCHED_RR:
			printf("%s %d\n", sched_policy_to_str(sattr.sched_policy), sattr.sched_priority);
			break;
		case SCHED_DEADLINE:
			printf("%s: runtime %llu, deadline %llu, period %llu\n", sched_policy_to_str(sattr.sched_policy),
				   sattr.sched_runtime, sattr.sched_deadline, sattr.sched_period);
			break;
		default:
			errx(1, "Invald SCHED: %d", sattr.sched_policy);
			break;
	}

}

static void do_setscheduler(const int sched_policy, const int sched_prio) {
	struct sched_param param;
	MEMSET(param);
	switch (sched_policy) {
		case SCHED_OTHER:
		case SCHED_BATCH:
		case SCHED_IDLE:
			SYSCALLWRAP(sched_setscheduler, 0, sched_policy, &param);
			SYSCALLWRAP(setpriority, PRIO_PROCESS, 0, sched_prio);
			break;
		case SCHED_FIFO:
		case SCHED_RR:
			param.sched_priority = sched_prio;
			SYSCALLWRAP(sched_setscheduler, 0, sched_policy, &param);
			break;
		case SCHED_DEADLINE:
		default:
			errx(1, "Invald SCHED: %d", sched_policy);
			break;
	}
	
}
static void do_getscheduler() {
	int sched_policy = -1;
	int nice_val = -99;
	struct sched_param param;
	MEMSET(param);
	sched_policy = sched_getscheduler(0);
	if (sched_policy < 0) {
		perror("sched_getscheduler");
		return;
	}
	switch (sched_policy) {
		case SCHED_OTHER:
		case SCHED_BATCH:
		case SCHED_IDLE:
			errno = 0;
			nice_val = getpriority(0, 0);
			if (nice_val == -1 && errno != 0) {
				perror("getpriority");
				break;
			}
			printf("%s %d\n", sched_policy_to_str(sched_policy), nice_val);
			break;
		case SCHED_FIFO:
		case SCHED_RR:
			SYSCALLWRAP(sched_getparam, 0, &param);
			printf("%s %d\n", sched_policy_to_str(sched_policy), param.sched_priority);
			break;
		case SCHED_DEADLINE:
		default:
			errx(1, "Invald SCHED: %d", sched_policy);
			break;
	}
}

int main(int argc, char *argv[]){
	//optarg
	//optind
	//opterr
	//optopt

	int c;
	int sched_policy = SCHED_OTHER;
	int sched_prio = 0;

	while ((c=getopt(argc, argv, "f:r:b:o:di")) != -1) {
		switch (c) {
			case 'f':
				sched_policy = SCHED_FIFO;
				sched_prio = atoi(optarg);
				break;
			case 'r':
				sched_policy = SCHED_RR;
				sched_prio = atoi(optarg);
				break;
			case 'b':
				sched_policy = SCHED_BATCH;
				sched_prio = atoi(optarg);
				break;
			case 'o':
				sched_policy = SCHED_OTHER;
				sched_prio = atoi(optarg);
				break;
			case 'i':
				sched_policy = SCHED_IDLE;
				sched_prio = 0;
				break;
			case 'd':
				sched_policy = SCHED_DEADLINE;
				sched_prio = 0;
				break;
			default:
				errx(1, "Invald arg: %d", c);
				break;
		}
	}

#if defined(USE_SCHED_SETATTR)
	do_sched_setattr(sched_policy, sched_prio);
	do_sched_getattr();
#else
	if (SCHED_DEADLINE == sched_policy) {
		do_sched_setattr(sched_policy, sched_prio);
		do_sched_getattr();
	} else {
		do_setscheduler(sched_policy, sched_prio);
		do_getscheduler();
	}
#endif
	
	return 0;
}
