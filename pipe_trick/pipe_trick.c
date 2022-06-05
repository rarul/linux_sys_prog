#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
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

static int pipe_for_signal[2] = {-1, -1};

typedef struct {
	int signum;
	union siginfo_ext_t {
		void *sigint_data;
		void *sigterm_data;
		void *sigchld_data;
	} siginfo;
} sig_pipe_data_t;

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

static ssize_t do_write(int fd, void *buf, size_t count) {
	size_t write_size = 0;
	while (write_size < count) {
		ssize_t ret_write = write (fd, (buf+write_size), (count-write_size));
		if (ret_write < 0) {
			// err
			if (errno == EINTR || errno == EAGAIN) {
				// retry
#if 0
				// It's bad to retry in signal handler. But what to do ?
				continue;
#endif
			}
			if (write_size <= 0) {
				// nothing write;
				return ret_write;
			}
			// partially write
			break;
		} else if (ret_write == 0) {
			// eof
			break;
		}
		write_size += ret_write;
	}
	return write_size;
}

static void sig_handler(int signum, siginfo_t *info , void *ctx ) {
	// signal handler function
	sig_pipe_data_t sig_pipe_data;
	int saved_errno = errno;

	memset (&sig_pipe_data, 0, sizeof(sig_pipe_data));
	sig_pipe_data.signum = signum;
	sig_pipe_data.siginfo.sigint_data = NULL;
	// send signal info to main loop via pipe
	do_write (pipe_for_signal[1], &sig_pipe_data, sizeof(sig_pipe_data));

	errno = saved_errno;
}

static void sig_action(int fd) {
	// read signal info in main loop via pipe
	sig_pipe_data_t sig_pipe_data;
	memset (&sig_pipe_data, 0, sizeof(sig_pipe_data));
	ssize_t ret_read = do_read(fd, &sig_pipe_data, sizeof(sig_pipe_data));
	if (ret_read != sizeof(sig_pipe_data)) {
		printf("invalid pipe read size %zd %zu\n", ret_read, sizeof(sig_pipe_data));
		return;
	}

	switch(sig_pipe_data.signum) {
		case SIGINT:
			errx(1, "SIGINT received. Goto exit\n");
			break;
		case SIGTERM:
			errx(0, "SIGTERM received. Goto exit\n");
			break;
		default:
			printf("unknown signal num received %d\n", sig_pipe_data.signum);
			break;
	}
}

static void set_fl(int fd, int flags) {
	// helper function to set flag for fd
	int val = fcntl(pipe_for_signal[1], F_GETFL, 0);
	if (val < 0) { err(1, "F_GETFL"); }
	val |= flags;
	if (fcntl(fd, F_SETFL, val) < 0) { err(1, "F_SETFL"); }
}

static void prepare_pipe() {
	// initialize pipe for signal handler
	SYSCALLWRAP(pipe2, pipe_for_signal, O_CLOEXEC);
	set_fl(pipe_for_signal[1], O_NONBLOCK);
}

static void prepare_signal_handler() {
	// install signal handler
	struct sigaction sa_sighandle;
	memset (&sa_sighandle, 0, sizeof(sa_sighandle));
	sa_sighandle.sa_sigaction = sig_handler;
	sa_sighandle.sa_flags = SA_SIGINFO;
#if 0
	// reduce EINTR retry, but not perfect.
	sa_sighandle.sa_flags |= SA_RESTART;
#endif
	SYSCALLWRAP(sigaction, SIGINT, &sa_sighandle, NULL);
	SYSCALLWRAP(sigaction, SIGTERM, &sa_sighandle, NULL);
}

int main() {
	// initialize
	prepare_pipe();
	prepare_signal_handler();

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
	SETNEWFD(pipe_for_signal[0]);

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

		if (FD_ISSET(pipe_for_signal[0], &rfds)) {
			sig_action(pipe_for_signal[0]);
		}
	}
	return 0;
}
