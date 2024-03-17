#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#ifndef MM_DS18B20_H
#define MM_DS18B20_H

/**
 * @brief Gets the thermometer device from the device tree.
 */
const struct device *get_ds18b20_device(void);

/**
 * @brief prints the current temperature to the console
 */
int print_temperature(const struct device *dev);
#endif // !MM_DS18B20_H
