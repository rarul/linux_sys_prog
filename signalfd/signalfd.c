#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <err.h>
#include <sys/signalfd.h>
#include <sys/select.h>
#include <sys/errno.h>

#define SYSCALLWRAP(SNAME, ...)		\
	do {							\
		int ret;					\
		ret = SNAME(__VA_ARGS__);	\
		if (ret) {					\
			err(1, #SNAME);			\
		}							\
	} while(0)


static ssize_t do_read(int fd, void *buf, size_t count) {
	size_t read_size = 0;
	while (read_size < count) {
		ssize_t ret_read = read (fd, (buf+read_size), (count-read_size));
		if (ret_read < 0) {
			// err
			if (errno == EINTR || errno == EAGAIN) {
				// retry
				continue;
			}
			if (read_size <= 0) {
				// nothing read;
				return ret_read;
			}
			// partially read
			break;
		} else if (ret_read == 0) {
			// eof
			break;
		}
		read_size += ret_read;
	}
	return read_size;
}

static int prepare_signal_fd() {
	// prepare sigset variable
	sigset_t mask;
	SYSCALLWRAP(sigemptyset, &mask);
	SYSCALLWRAP(sigaddset, &mask, SIGINT);
	SYSCALLWRAP(sigaddset, &mask, SIGTERM);

	// prepare signalfd
	int fd = signalfd(-1, &mask, SFD_CLOEXEC);
	if (fd < 0) {
		err(1, "signalfd");
	}

	// start to block subscribed signals
	SYSCALLWRAP(sigprocmask, SIG_BLOCK, &mask, NULL);
	return fd;
}

static void signal_fd_action(int fd) {
	struct signalfd_siginfo thisinfo;
	ssize_t ret_read = do_read (fd, &thisinfo, sizeof(thisinfo));
	if (ret_read != sizeof(thisinfo)) {
		printf("invalid signalfd read size %zd %zu\n", ret_read, sizeof(thisinfo));
		return;
	}

	switch(thisinfo.ssi_signo) {
		case SIGINT:
			errx(1, "SIGINT received. Goto exit\n");
			break;
		case SIGTERM:
			errx(0, "SIGTERM received. Goto exit\n");
			break;
		default:
			printf("unknown signalfd read %u\n", thisinfo.ssi_signo);
			break;
	}
}

int main() {
	int signal_fd = prepare_signal_fd();

	// prepare fd_set for select
	fd_set def_fds;
	FD_ZERO(&def_fds);
	int max_fd = -1;
#define SETNEWFD(new_fd)			\
	do {							\
		FD_SET(new_fd, &def_fds);	\
		if(new_fd > max_fd) {		\
			max_fd = new_fd;		\
		}							\
	} while(0)
	SETNEWFD(signal_fd);

	while(1) {
		fd_set rfds;
		rfds = def_fds;
		// select
		int retval = select (1+max_fd, &rfds, NULL, NULL, NULL);
		if (retval < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				// retry
				printf("select retry errno=%d\n", errno);
				continue;
			}
			// err
			err(1, "select");
		} else if (retval == 0) {
			// timeout. go retry.
			continue;
		}

		if (FD_ISSET(signal_fd, &rfds)) {
			signal_fd_action(signal_fd);
		}
	}
	return 0;
}
