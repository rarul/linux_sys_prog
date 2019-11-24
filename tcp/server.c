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

#define CLIENT_THREAD_POOL_NUM  (4)

#define SYSCALLWRAP(SNAME, ...)								\
	do {													\
		int ret;											\
		ret = SNAME(__VA_ARGS__);							\
		if (ret) {											\
			err(1, #SNAME);									\
		}													\
	} while(0)
#define MEMSET(val) memset(&val,0,sizeof(val))

static int client_fd_pipe[2] = {-1, -1};
#define PIPE_READ_FD  (client_fd_pipe[0])
#define PIPE_WRITE_FD (client_fd_pipe[1])
static void do_client_work(const int fd) {
	char buf[64];
	ssize_t read_size;
	while (1) {
		MEMSET(buf);
		read_size = do_read (fd, buf, sizeof(buf)-1);
		if (read_size < 0) {
			err(1, "do_read");
		} else if (read_size == 0) {
			// EOF
			close(fd);
			break;
		}
		puts(buf);
	}
}

static void *server_client_thread(void *arg) {
	int client_fd = -1;
	ssize_t read_size;
	while (1) {
		client_fd = -1;
		read_size = do_read (PIPE_READ_FD, &client_fd, sizeof(client_fd));
		if (read_size < 0) {
			err(1, "do_read");
		} else if (read_size == 0) {
			// EOF
			break;
		} else if (read_size != sizeof(client_fd)) {
			// unexpected size
			fprintf (stderr, "do_read() expect %ld, but ret %ld\n",
					 sizeof(client_fd), read_size);
			break;
		}

		do_client_work(client_fd);
				
	}
	return NULL;
}

static void client_thread_init(){
	int i;
	pthread_t thd;
	SYSCALLWRAP( pipe2, client_fd_pipe, O_CLOEXEC);
	for( i=0; i<CLIENT_THREAD_POOL_NUM; i++) {
		SYSCALLWRAP( pthread_create, &thd, NULL, server_client_thread, NULL);
		SYSCALLWRAP( pthread_detach, thd);
	}
}


static void server_main(const int server_port){
	int client_sock = -1;
	int server_sock = -1;
	const int on_value = 1;
	ssize_t write_size = -1;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addrlen = -1;

	server_sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (server_sock < 0) {
		err(1, "socket");
		return;
	}

	SYSCALLWRAP(setsockopt, server_sock, SOL_SOCKET, SO_REUSEADDR, &on_value, sizeof(on_value));
	
	MEMSET(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	//server_addr.sa_addr.s_addr = inet_addr("127.0.0.1");

	SYSCALLWRAP(bind, server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	SYSCALLWRAP(listen, server_sock, SOMAXCONN);

	while (1) {
		MEMSET(client_addr);
		client_addrlen = sizeof(client_addr);
		client_sock = accept4(server_sock, (struct sockaddr *)&client_addr,
							  &client_addrlen, SOCK_CLOEXEC);
		if (client_sock < 0) {
			err(1, "accept4");
			break;
		}
		write_size = do_write(PIPE_WRITE_FD, &client_sock, sizeof(client_sock));
		if (write_size < 0) {
			err(1, "do_write");
		} else if (write_size != sizeof(client_sock)) {
			fprintf (stderr, "do_write() expect %ld, but ret %ld\n",
					 sizeof(client_sock), write_size);
			break;
		}
	}

	close(server_sock);
}

int main(int argc, char *argv[]){
	client_thread_init();
	server_main(12345);
	printf("waiting client_fd to be closed\n");
	sleep(1);
	return 0;
}
