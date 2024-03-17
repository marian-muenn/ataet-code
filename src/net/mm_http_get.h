#include <stdio.h>
#include <stdlib.h>
#include <zephyr/net/socket.h>

void http_get(int sock, char * hostname, char * url);
int connect_socket(struct zsock_addrinfo **results);


