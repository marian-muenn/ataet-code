#include "mm_nslookup.h"

void nslookup(const char * hostname, struct zsock_addrinfo **results)
{
	int err;
	
	struct zsock_addrinfo hints = {
		//.ai_family = AF_INET,     // Allow IPv4 Address
		.ai_family = AF_UNSPEC,		// Allow IPv4 or IPv6	
		.ai_socktype = SOCK_STREAM,
	};

	err = zsock_getaddrinfo(hostname, NULL, &hints, (struct zsock_addrinfo **) results);
	if (err) {
		printf("getaddrinfo() failed, err %d\n", errno);
		return;
	}
}

void print_addrinfo_results(struct zsock_addrinfo **results)
{
	char ipv4[INET_ADDRSTRLEN];
	char ipv6[INET6_ADDRSTRLEN];
	struct sockaddr_in *sa;
	struct sockaddr_in6 *sa6;
	struct zsock_addrinfo *rp;
	
	for (rp = (struct zsock_addrinfo *)results; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_addr->sa_family == AF_INET) {
			// IPv4 Address
			sa = (struct sockaddr_in *) rp->ai_addr;
			zsock_inet_ntop(AF_INET, &sa->sin_addr, ipv4, INET_ADDRSTRLEN);
			printf("IPv4: %s\n", ipv4);
		}
		if (rp->ai_addr->sa_family == AF_INET6) {
			// IPv6 Address
			sa6 = (struct sockaddr_in6 *) rp->ai_addr;
			zsock_inet_ntop(AF_INET6, &sa6->sin6_addr, ipv6, INET6_ADDRSTRLEN);
			printf("IPv6: %s\n", ipv6);
		}
	}
}
