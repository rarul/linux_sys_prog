#ifndef __UTIL_H__
#define __UTIL_H__

pid_t gettid(void);
ssize_t do_read (const int fd, void *buf, const size_t count);
ssize_t do_write (const int fd, void *buf, const size_t count);


#endif /* __UTIL_H__ */
