#define _GNU_SOURCE

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  <fcntl.h>
#include  <err.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <sys/errno.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <pthread.h>
#include  "util.h"

#define SYSCALLWRAP(SNAME, ...)								\
	do {													\
		int ret;											\
		ret = SNAME(__VA_ARGS__);							\
		if (ret) {											\
			err(1, #SNAME);									\
		}													\
	} while(0)
#define MEMSET(val) memset(&val,0,sizeof(val))

static void do_client_work(const int fd) {
	char buf[64];
	int len;
	len = snprintf (buf, sizeof(buf), "[pid:%d] test mesg", getpid());
	if (len < 0) {
		err(1, "snprintf");
	}
	do_write(fd, buf, len);
}
static void client_main(const int server_port) {
	int client_sock;
	struct sockaddr_in client_addr;

	client_sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (client_sock < 0) {
		err(1, "socket");
		return;
	}

	MEMSET (client_addr);
	client_addr.sin_family = PF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(server_port);
	SYSCALLWRAP(connect, client_sock, (struct sockaddr*)&client_addr, sizeof(client_addr));

	do_client_work(client_sock);
	close(client_sock);
}

int main(int argc, char *argv[]){
	client_main(12345);
	return 0;
}
