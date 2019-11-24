#ifndef __UTIL_H__
#define __UTIL_H__

#define MEMSET(val) memset(&val,0,sizeof(val))
#define SYSCALLWRAP(SNAME, ...)								\
	do {													\
		int ret;											\
		ret = SNAME(__VA_ARGS__);							\
		if (ret) {											\
			err(1, #SNAME);									\
		}													\
	} while(0)


pid_t gettid(void);
ssize_t do_read (const int fd, void *buf, const size_t count);
ssize_t do_write (const int fd, void *buf, const size_t count);
int create_socket(int domain, int type);

#endif /* __UTIL_H__ */
