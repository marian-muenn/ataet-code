/*
 * Copyright (c) 2020, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mm_modem.h"
#include "zephyr/device.h"
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/modem/gsm_ppp.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/conn_mgr_monitor.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(mm_gsm_ppp, LOG_LEVEL_DBG);

#define GSM_MODEM_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_gsm_ppp)
#define UART_NODE DT_BUS(GSM_MODEM_NODE)

static const struct device *const gsm_dev = DEVICE_DT_GET(GSM_MODEM_NODE);
static struct net_mgmt_event_callback mgmt_cb;
static bool starting = IS_ENABLED(CONFIG_GSM_PPP_AUTOSTART);

static int cmd_sample_modem_suspend(const struct shell *sh, size_t argc,
                                    char *argv[]) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  if (!starting) {
    shell_fprintf(sh, SHELL_NORMAL, "Modem is already stopped.\n");
    return -ENOEXEC;
  }

  gsm_ppp_stop(gsm_dev);
  starting = false;

  return 0;
}

static int cmd_sample_modem_resume(const struct shell *sh, size_t argc,
                                   char *argv[]) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  if (starting) {
    shell_fprintf(sh, SHELL_NORMAL, "Modem is already started.\n");
    return -ENOEXEC;
  }

  gsm_ppp_start(gsm_dev);
  starting = true;

  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sample_commands,
                               SHELL_CMD(resume, NULL, "Resume the modem\n",
                                         cmd_sample_modem_resume),
                               SHELL_CMD(suspend, NULL, "Suspend the modem\n",
                                         cmd_sample_modem_suspend),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sample, &sample_commands, "Sample application commands",
                   NULL);

static void event_handler(struct net_mgmt_event_callback *cb,
                          uint32_t mgmt_event, struct net_if *iface) {
  ARG_UNUSED(cb);
  ARG_UNUSED(iface);

  if ((mgmt_event & (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)) !=
      mgmt_event) {
    return;
  }

  if (mgmt_event == NET_EVENT_L4_CONNECTED) {
    LOG_INF("Network connected");
    return;
  }

  if (mgmt_event == NET_EVENT_L4_DISCONNECTED) {
    LOG_INF("Network disconnected");
    return;
  }
}

static void modem_on_cb(const struct device *dev, void *user_data) {
  ARG_UNUSED(dev);
  ARG_UNUSED(user_data);

  LOG_INF("GSM modem on callback fired");
}

static void modem_off_cb(const struct device *dev, void *user_data) {
  ARG_UNUSED(dev);
  ARG_UNUSED(user_data);

  LOG_INF("GSM modem off callback fired");
}
static void modem_poweron() {
  const struct gpio_dt_spec modem_pwr =
      GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), modem_gpios);
  // configure the LED pin as output
  // /* Configure the pin */
  gpio_pin_configure_dt(&modem_pwr, GPIO_OUTPUT_HIGH);

  LOG_INF("REBOOTING MODEM");
  gpio_pin_set_dt(&modem_pwr, 1);
  k_sleep(K_MSEC(2100));
  gpio_pin_set_dt(&modem_pwr, 0);
  k_sleep(K_MSEC(2000));
  gpio_pin_set_dt(&modem_pwr, 1);
  k_sleep(K_MSEC(1800));
  gpio_pin_set_dt(&modem_pwr, 0);
  k_sleep(K_MSEC(500));
}
void modem_poweroff() {
  const struct gpio_dt_spec modem_pwr =
      GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), modem_gpios);
  // configure the LED pin as output
  // /* Configure the pin */
  gpio_pin_configure_dt(&modem_pwr, GPIO_OUTPUT_HIGH);

  gpio_pin_set_dt(&modem_pwr, 1);
  k_sleep(K_MSEC(1800));
  gpio_pin_set_dt(&modem_pwr, 0);
  k_sleep(K_MSEC(2000));
  gpio_pin_set_dt(&modem_pwr, 1);
  k_sleep(K_MSEC(2100));
  gpio_pin_set_dt(&modem_pwr, 0);
  k_sleep(K_MSEC(2000));
}

int modem_init(void) {
  const struct device *const uart_dev = DEVICE_DT_GET(UART_NODE);
  /* Optional register modem power callbacks */
  gsm_ppp_register_modem_power_callback(gsm_dev, modem_on_cb, modem_off_cb,
                                        NULL);

  LOG_INF("Board '%s' APN '%s' UART '%s' device %p (%s)", CONFIG_BOARD,
          CONFIG_MODEM_GSM_APN, uart_dev->name, uart_dev, gsm_dev->name);
  modem_poweron();

  net_mgmt_init_event_callback(&mgmt_cb, event_handler,
                               NET_EVENT_L4_CONNECTED |
                                   NET_EVENT_L4_DISCONNECTED);
  net_mgmt_add_event_callback(&mgmt_cb);

  k_sleep(K_MSEC(10000)); // TODO ADD SEMAPHORE
  return 0;
}
