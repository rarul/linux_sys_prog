#include  <unistd.h>
#include  <err.h>
#include  <sys/errno.h>
#include  <sys/syscall.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <netinet/tcp.h>

#include  "util.h"

#define GENERIC_DO_READWRITE(syscall)									\
		ssize_t ret_size;												\
		size_t  proceed_size = 0;										\
		while (proceed_size < count) {									\
			ret_size = syscall(fd, buf+proceed_size, (count-proceed_size)); \
			if (ret_size < 0) {											\
				if (errno == EAGAIN || errno == EINTR) {				\
					continue;											\
				}														\
				if (proceed_size <= 0) {								\
					return -1;											\
				}														\
				break;													\
			} else if (ret_size == 0) {									\
				break;													\
			}															\
			proceed_size += ret_size;									\
		}																\
		return proceed_size

ssize_t do_read (const int fd, void *buf, const size_t count) {
	GENERIC_DO_READWRITE(read);
}
ssize_t do_write (const int fd, void *buf, const size_t count) {
	GENERIC_DO_READWRITE(write);
}

pid_t gettid(void) {
	return syscall(SYS_gettid);
}

#define MYSOLSOCKET(thisoption,value)									\
	do {																\
		int on_value = value;											\
		SYSCALLWRAP(setsockopt, sock_fd, SOL_SOCKET, thisoption, &on_value, sizeof(on_value)); \
	} while(0)
#define MYIPPROTOTCP(thisoption,value)									\
	do {																\
		int on_value = value;											\
		SYSCALLWRAP(setsockopt, sock_fd, IPPROTO_TCP, thisoption, &on_value, sizeof(on_value)); \
	} while(0)
int create_socket(int domain, int type) {
	int sock_fd;

	sock_fd = socket(domain, type | SOCK_CLOEXEC, 0);
	if (sock_fd < 0) {
		warn("socket");
		return -1;
	}

	if (domain == AF_INET || domain == AF_INET6) {
		MYSOLSOCKET(SO_REUSEADDR, 1);
		if (type == SOCK_STREAM) {
			MYSOLSOCKET(SO_KEEPALIVE,1);
			MYIPPROTOTCP(TCP_KEEPIDLE,10);
			MYIPPROTOTCP(TCP_KEEPINTVL,1);
			MYIPPROTOTCP(TCP_KEEPCNT,5);
		}
	}

	return sock_fd;
}
