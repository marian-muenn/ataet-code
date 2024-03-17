#ifndef MM_NSLOOKUP_H
#define MM_NSLOOKUP_H
#include <zephyr/net/socket.h>
void nslookup(const char* , struct zsock_addrinfo**);
void print_addrinfo_results(struct zsock_addrinfo**);
#endif // !MM_NSLOOKUP_H
