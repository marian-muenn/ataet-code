/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mm_mqtt.h"
#include "../mm_config.h"
#include "mm_certs.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"
#include "zephyr/net/mqtt.h"
#include "zephyr/net/tls_credentials.h"
#include <sys/_stdint.h>
#define TLS_SNI_HOSTNAME "zapdos.eit.htwk-leipzig.de"
#define APP_CA_CERT_TAG 1

LOG_MODULE_REGISTER(mqtt, LOG_LEVEL_DBG);

/* Buffers for MQTT client. */
static uint8_t rx_buffer[MQTT_BUFFER_SIZE];
static uint8_t tx_buffer[MQTT_BUFFER_SIZE];

/* The mqtt client struct */
static struct mqtt_client client_ctx;

/* MQTT Broker details. */
static struct sockaddr_storage broker;
static struct zsock_pollfd fds[1];
static struct mqtt_utf8 username = MQTT_UTF8_LITERAL(MQTT_USERNAME);
static struct mqtt_utf8 passwd = MQTT_UTF8_LITERAL(MQTT_PASSWD);
static int nfds;

static bool connected;

static sec_tag_t m_sec_tags[] = {APP_CA_CERT_TAG};

static int mqtt_tls_init(void) {
  int err = -EINVAL;

  err = tls_credential_add(APP_CA_CERT_TAG, TLS_CREDENTIAL_CA_CERTIFICATE,
                           ca_certificate, sizeof(ca_certificate));
  if (err < 0) {
    LOG_ERR("Failed to register public certificate: %d", err);
    return err;
  }
  return err;
}

static void prepare_fds(struct mqtt_client *client) {
  if (client->transport.type == MQTT_TRANSPORT_NON_SECURE) {
    fds[0].fd = client->transport.tcp.sock;
  } else if (client->transport.type == MQTT_TRANSPORT_SECURE) {
    fds[0].fd = client->transport.tls.sock;
  }

  fds[0].events = ZSOCK_POLLIN;
  nfds = 1;
}

static void clear_fds(void) { nfds = 0; }

static int wait(int timeout) {
  int ret = 0;

  if (nfds > 0) {
    ret = zsock_poll(fds, nfds, timeout);
    if (ret < 0) {
      LOG_ERR("poll error: %d", errno);
    }
  }

  return ret;
}

void mqtt_evt_handler(struct mqtt_client *const client,
                      const struct mqtt_evt *evt) {
  int err;

  switch (evt->type) {
  case MQTT_EVT_CONNACK:
    if (evt->result != 0) {
      LOG_ERR("MQTT connect failed %d", evt->result);
      break;
    }

    connected = true;
    LOG_INF("MQTT client connected!");

    break;

  case MQTT_EVT_DISCONNECT:
    LOG_INF("MQTT client disconnected %d", evt->result);

    connected = false;
    clear_fds();

    break;

  case MQTT_EVT_PUBACK:
    if (evt->result != 0) {
      LOG_ERR("MQTT PUBACK error %d", evt->result);
      break;
    }

    LOG_INF("PUBACK packet id: %u", evt->param.puback.message_id);

    break;

  case MQTT_EVT_PUBREC:
    if (evt->result != 0) {
      LOG_ERR("MQTT PUBREC error %d", evt->result);
      break;
    }

    LOG_INF("PUBREC packet id: %u", evt->param.pubrec.message_id);

    const struct mqtt_pubrel_param rel_param = {
        .message_id = evt->param.pubrec.message_id};

    err = mqtt_publish_qos2_release(client, &rel_param);
    if (err != 0) {
      LOG_ERR("Failed to send MQTT PUBREL: %d", err);
    }

    break;

  case MQTT_EVT_PUBCOMP:
    if (evt->result != 0) {
      LOG_ERR("MQTT PUBCOMP error %d", evt->result);
      break;
    }

    LOG_INF("PUBCOMP packet id: %u", evt->param.pubcomp.message_id);

    break;

  case MQTT_EVT_PINGRESP:
    LOG_INF("PINGRESP packet");
    break;

  default:
    break;
  }
}

static int publish(struct mqtt_client *client, struct mqtt_payload *payload) {
  struct mqtt_publish_param param;
  param.message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE;
  param.message.topic.topic = MQTT_UTF8_LITERAL(payload->topic);
  printk("paramTopic: %s\n", param.message.topic.topic.utf8);
  param.message.topic.topic.size = strlen(param.message.topic.topic.utf8);
  param.message.payload.data = payload->data;
  param.message.payload.len = payload->size;
  param.message_id = sys_rand32_get();
  param.dup_flag = 0U;
  param.retain_flag = 0U;
  return mqtt_publish(client, &param);
}

#define RC_STR(rc) ((rc) == 0 ? "OK" : "ERROR")

#define PRINT_RESULT(func, rc) LOG_INF("%s: %d <%s>", (func), rc, RC_STR(rc))

/**
 * \brief Initialisiert die Broker struct
 */
static void broker_init(void) {
  struct sockaddr_in *broker4 = (struct sockaddr_in *)&broker;

  broker4->sin_family = AF_INET;
  broker4->sin_port = htons(MQTT_SERVER_PORT);
  zsock_inet_pton(AF_INET, MQTT_SERVER_ADDR, &broker4->sin_addr);
}
/**
 * \brief Initialisiert den MQTT Client
 * \params mqtt_client Pointer auf den MQTT client
 */
static void client_init(struct mqtt_client *client) {
  mqtt_client_init(client);

  broker_init();

  /* MQTT client configuration */
  client->broker = &broker;
  client->evt_cb = mqtt_evt_handler;
  client->client_id.utf8 = (uint8_t *)MQTT_CLIENTID;
  client->client_id.size = strlen(MQTT_CLIENTID);

  client->password = &passwd;
  client->user_name = &username;
  client->protocol_version = MQTT_VERSION_3_1_1;

  /* MQTT buffers configuration */
  client->rx_buf = rx_buffer;
  client->rx_buf_size = sizeof(rx_buffer);
  client->tx_buf = tx_buffer;
  client->tx_buf_size = sizeof(tx_buffer);

  /* MQTT transport configuration */
  client->transport.type = MQTT_TRANSPORT_SECURE;
  struct mqtt_sec_config *tls_config = &client->transport.tls.config;
  tls_config->peer_verify = TLS_PEER_VERIFY_OPTIONAL;
  tls_config->cipher_list = NULL;
  tls_config->sec_tag_list = m_sec_tags;
  tls_config->sec_tag_count = ARRAY_SIZE(m_sec_tags);
  tls_config->hostname = ""; // TLS_SNI_HOSTNAME;
}

/* In this routine we block until the connected variable is 1 */
static int try_to_connect(struct mqtt_client *client) {
  int rc, i = 0;

  while (i++ < MQTT_CONNECT_TRIES && !connected) {

    client_init(client);
    LOG_INF("mqtt username: %s", client->user_name->utf8);
    rc = mqtt_connect(client);
    if (rc != 0) {
      PRINT_RESULT("mqtt_connect", rc);
      LOG_INF("mqtt connect did not work");
      k_sleep(K_MSEC(MQTT_SLEEP_MSECS));
      continue;
    }

    prepare_fds(client);

    if (wait(MQTT_CONNECT_TIMEOUT_MS)) {
      LOG_INF("MQTT input connected");
      mqtt_input(client);
    }

    if (!connected) {
      LOG_INF("MQTT not connected");
      mqtt_abort(client);
    }
  }

  if (connected) {
    return 0;
  }

  return -EINVAL;
}

static int process_mqtt_and_sleep(struct mqtt_client *client, int timeout) {
  int64_t remaining = timeout;
  int64_t start_time = k_uptime_get();
  int rc;

  while (remaining > 0 && connected) {
    if (wait(remaining)) {
      rc = mqtt_input(client);
      if (rc != 0) {
        PRINT_RESULT("mqtt_input", rc);
        return rc;
      }
    }

    rc = mqtt_live(client);
    if (rc != 0 && rc != -EAGAIN) {
      PRINT_RESULT("mqtt_live", rc);
      return rc;
    } else if (rc == 0) {
      rc = mqtt_input(client);
      if (rc != 0) {
        PRINT_RESULT("mqtt_input", rc);
        return rc;
      }
    }

    remaining = timeout + start_time - k_uptime_get();
  }

  return 0;
}

#define SUCCESS_OR_EXIT(rc)                                                    \
  {                                                                            \
    if (rc != 0) {                                                             \
      return 1;                                                                \
    }                                                                          \
  }
#define SUCCESS_OR_BREAK(rc)                                                   \
  {                                                                            \
    if (rc != 0) {                                                             \
      break;                                                                   \
    }                                                                          \
  }

static int publisher(struct k_msgq *mqtt_msgq) {
  int i, rc, r = 0;
  struct mqtt_payload payload;

  LOG_INF("attempting to connect: ");
  rc = try_to_connect(&client_ctx);
  PRINT_RESULT("try_to_connect", rc);
  SUCCESS_OR_EXIT(rc);

  i = 0;
  while (i++ < CONFIG_NET_SAMPLE_APP_MAX_ITERATIONS && connected) {
    r = -1;

    rc = mqtt_ping(&client_ctx);
    PRINT_RESULT("mqtt_ping", rc);
    SUCCESS_OR_BREAK(rc);

    rc = process_mqtt_and_sleep(&client_ctx, MQTT_SLEEP_MSECS);
    SUCCESS_OR_BREAK(rc);
    printk("fetching payload from msqq\n");
    k_sleep(K_MSEC(500));

    while (k_msgq_get(mqtt_msgq, &payload, K_TIMEOUT_ABS_MS(1000)) == 0) {
      printk("fetched payload\n");
      LOG_HEXDUMP_INF(&payload, sizeof(struct mqtt_payload),
                      "payload after msgq\n");
      k_sleep(K_MSEC(200));

      LOG_HEXDUMP_INF(&(payload.size), sizeof(size_t), "payload size");
      printk("size: %d\n", payload.size);
      k_sleep(K_SECONDS(1));
      LOG_HEXDUMP_INF(payload.data, sizeof(uint8_t) * 4,
                      " first 4 bytes of payload data");
      LOG_HEXDUMP_INF(payload.topic, 4 * sizeof(char), "payload topic");

      k_sleep(K_SECONDS(1));
      printk("LEN: %d\n", payload.size);
      rc = publish(&client_ctx, &payload);
      PRINT_RESULT("mqtt_publish", rc);
      SUCCESS_OR_BREAK(rc);
      rc = process_mqtt_and_sleep(&client_ctx, MQTT_SLEEP_MSECS);
      SUCCESS_OR_BREAK(rc);
    }
    r = 0;
  }

  rc = mqtt_disconnect(&client_ctx);
  PRINT_RESULT("mqtt_disconnect", rc);
  LOG_INF("Bye!");
  return r;
}

int run_mqtt(struct k_msgq *mqtt_msgq) {
  int r = 0;

  LOG_INF("starting mqtt");
  r = mqtt_tls_init();
  LOG_INF("tls_init: %d", r);

  r = 0;
  r = publisher(mqtt_msgq);
  return r;
}
