#include  <unistd.h>
#include  <sys/errno.h>
#include  <sys/syscall.h>

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
