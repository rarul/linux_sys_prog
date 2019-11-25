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

static void do_client_work(const int fd) {
	char buf[64];
	int len;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	struct sockaddr_in server_addr;
	socklen_t server_addr_len;

	client_addr_len = sizeof(client_addr);
	MEMSET(client_addr);
	SYSCALLWRAP(getsockname, fd, (struct sockaddr*)&client_addr, &client_addr_len);
	server_addr_len = sizeof(server_addr);
	MEMSET(server_addr);
	SYSCALLWRAP(getpeername, fd, (struct sockaddr*)&server_addr, &server_addr_len);
	
	len = snprintf (buf, sizeof(buf), "(pid=%d sa=%s sp=%d da=%s dp=%d)", getpid(),
					inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
					inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
	if (len < 0) {
		err(1, "snprintf");
	}
	do_write(fd, buf, sizeof(buf)-1);
}
static void client_port_bind(int fd, int port) {
	struct sockaddr_in client_addr;
	if (port <= 0 || port >= 65536) {
		return;
	}
	MEMSET(client_addr);
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	client_addr.sin_addr.s_addr = INADDR_ANY;
	SYSCALLWRAP(bind, fd, (struct sockaddr*)&client_addr, sizeof(client_addr));
}
static void client_main(const char *server_ip, const int server_port, const int client_port) {
	int client_sock;
	struct sockaddr_in server_addr;

	client_sock = create_socket(AF_INET, SOCK_STREAM);
	if (client_sock < 0) {
		errx(1, "socket");
		return;
	}
	client_port_bind (client_sock, client_port);
	
	MEMSET (server_addr);
	server_addr.sin_family = PF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);
	SYSCALLWRAP(connect, client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	printf("connecting %s:%d\n", server_ip, server_port);

	do_client_work(client_sock);
	close(client_sock);
}

int main(int argc, char *argv[]){
	char *ip_addr = "127.0.0.1";
	int server_port = 12345;
	int client_port = -1;
	if (argc >= 2) {
		int tmp;
		char *p;
		p = strchr(argv[1], ':');
		if (p != NULL) {
			*p = '\0';
			ip_addr = argv[1];
			p++;
		} else {
			p = argv[1];
		}
		tmp = atoi(p);
		if (tmp > 0 && tmp < 65536) {
			server_port = tmp;
		}
	}
	if (argc >= 3) {
		int tmp = atoi(argv[2]);
		if (tmp > 0 && tmp < 65536) {
			client_port = tmp;
		}
	}
	client_main(ip_addr, server_port, client_port);
	return 0;
}
