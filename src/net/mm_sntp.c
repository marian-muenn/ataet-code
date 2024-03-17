#include "mm_sntp.h"
#include <zephyr/posix/time.h>
LOG_MODULE_REGISTER(net_sntp_client_sample, LOG_LEVEL_DBG);
#define SNTP_PORT 123

uint64_t get_sntp_time_ip(char *ip4_addr) {
  struct sntp_ctx ctx;
  struct sockaddr_in addr;
  struct sntp_time sntp_time;
  int rv;

  /* ipv4 */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SNTP_PORT);
  zsock_inet_pton(AF_INET, ip4_addr, &addr.sin_addr);

  rv = sntp_init(&ctx, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (rv < 0) {
    LOG_ERR("Failed to init SNTP IPv4 ctx: %d", rv);
    goto end;
  }

  LOG_INF("Sending SNTP IPv4 request...");
  rv = sntp_query(&ctx, 4 * MSEC_PER_SEC, &sntp_time);
  if (rv < 0) {
    LOG_ERR("SNTP IPv4 request failed: %d", rv);
    goto end;
  }

  LOG_INF("status: %d", rv);
  LOG_INF("time since Epoch: high word: %u, low word: %u",
          (uint32_t)(sntp_time.seconds >> 32), (uint32_t)sntp_time.seconds);

end:
  sntp_close(&ctx);
  const struct timespec sntp_timespec = {sntp_time.seconds, 0};
  clock_settime(CLOCK_REALTIME, &sntp_timespec);
  return sntp_time.seconds;
};
