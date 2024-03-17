#ifndef WIFI_H
#define WIFI_H

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>

#include "../mm_config.h"

// static void handle_wifi_connect_result(struct net_mgmt_event_callback*);
// static void handle_wifi_disconnect_result(struct net_mgmt_event_callback*);
// static void handle_ipv4_result(struct net_if*);
void wifi_disconnect(void);
void wifi_init(void);
#endif // !WIFI_H
