/*
 * Copyright (c) 2022 Thomas Stranger
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mm_ds18b20.h"
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
/*
 * Get a device structure from a devicetree node with compatible
 * "maxim,ds18b20". (If there are multiple, just pick one.)
 */
const struct device *get_ds18b20_device(void) {
  const struct device *const dev = DEVICE_DT_GET_ANY(maxim_ds18b20);

  if (dev == NULL) {
    /* No such node, or the node does not have status "okay". */
    printk("\nError: no device found.\n");
    return NULL;
  }

  if (!device_is_ready(dev)) {
    printk("\nError: Device \"%s\" is not ready; "
           "check the driver initialization logs for errors.\n",
           dev->name);
    return NULL;
  }

  printk("Found device \"%s\", getting sensor data\n", dev->name);
  return dev;
}

int print_temperature(const struct device *dev) {

  if (dev == NULL) {
    return 0;
  }

  struct sensor_value temp;

  sensor_sample_fetch(dev);
  sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);

  printk("Temp: %d.%06d\n", temp.val1, temp.val2);
  k_sleep(K_MSEC(2000));
  return 0;
}
