#ifndef MM_MQTT_H
#define MM_MQTT_H
#include <zephyr/logging/log.h>

#include <zephyr/kernel.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/socket.h>
#include <zephyr/random/random.h>
struct mqtt_payload {
  uint8_t *data;
  size_t size;
  const char *topic;
};
int run_mqtt(struct k_msgq *);
#endif
