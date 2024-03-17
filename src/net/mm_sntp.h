#include <zephyr/logging/log.h>
#include <zephyr/net/sntp.h>
#ifdef CONFIG_POSIX_API
#include <arpa/inet.h>
#endif

#include "../mm_config.h"
#include "mm_nslookup.h"

/**
 * \brief Bezieht die Epoch-Zeit vom NTP-Server
 * \param ipv4_addr IP-Adresse des Zeitservers
 */
uint64_t get_sntp_time_ip(char *ipv4_addr);
