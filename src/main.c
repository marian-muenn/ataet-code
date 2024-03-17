// #include <stdio.h>
#include <string.h>
#include <sys/_stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/base64.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

#include "arducam_mega.h"
// #include "mm_config.h"
#include "devices/mm_ds18b20.h"
#include "net/mm_modem.h"
#include "net/mm_mqtt.h"
#include "net/mm_sntp.h"
// #include "net/mm_wifi.h"
#include "uart.h"
#include "zephyr/logging/log.h"
LOG_MODULE_REGISTER(main);

// Define Message queue for 10 Messages of size 1024 Byte
K_MSGQ_DEFINE(mqtt_msgq, sizeof(struct mqtt_payload), 10, 1);
// Define Heap for image Storage
K_HEAP_DEFINE(mqtt_heap, 140000);
void print_logo() {
  printk("\n");
  printk(" _   _ _______        ___  __\n");
  printk("| | | |_   _\\ \\      / / |/ /\n");
  printk("| |_| | | |  \\ \\ /\\ / /| ' / \n");
  printk("|  _  | | |   \\ V  V / | . \\ \n");
  printk("|_| |_| |_|    \\_/\\_/  |_|\\_\\\n");
  printk("\n");
  printk("__     _____ ____ _____ ____   ___  _   _   ____    __  __  ___ "
         "_____ _____ \n");
  printk("\\ \\   / /_ _/ ___|_   _|  _ \\ / _ \\| \\ | | |___ \\  |  \\/  |/ "
         "_ \\_   _|_   _|\n");
  printk(" \\ \\ / / | | |     | | | |_) | | | |  \\| |   __) | | |\\/| | | | "
         "|| |   | |  \n");
  printk("  \\ V /  | | |___  | | |  _ <| |_| | |\\  |  / __/  | |  | | |_| || "
         "|   | |  \n");
  printk("   \\_/  |___\\____| |_| |_| \\_\\\\___/|_| \\_| |_____| |_|  "
         "|_|\\__\\_\\|_|   |_|  \n");
  printk("\n");
}

void init_board() {
  modem_init();
  //  wifi_init();
  //  init_usb();
  arducam_mega_init();
  // arducam_mega_get_id();
  // arducam_mega_print_info();
  arducam_mega_set_brightness(CAM_BRIGHTNESS_LEVEL_DEFAULT);
  arducam_mega_set_contrast(CAM_CONTRAST_LEVEL_DEFAULT);
  // wait for modem to come up
  k_sleep(K_SECONDS(5));
  get_sntp_time_ip(SNTP_SERVER_IP);
}

int main(void) {
  struct mqtt_payload payload = {.data = NULL, .size = 0, .topic = ""};
  init_board();
  // Get thermometer dev
  const struct device *dev = get_ds18b20_device();
  //
  k_sleep(K_SECONDS(5));
  // Victron example
  char buff[512];
  uart_main(buff, 512);
  printk("%s", buff);
  payload.data = buff;
  payload.size = strlen(buff);
  payload.topic = "victron";
  k_sleep(K_SECONDS(1));
  k_msgq_put(&mqtt_msgq, &payload, K_FOREVER);
  run_mqtt(&mqtt_msgq);
  printk("init done \n");
  printk("taking image\n");
  k_sleep(K_MSEC(300));
  int img_size = 0;
  img_size =
      arducam_mega_capture_image(CAM_IMAGE_MODE_HD, CAM_IMAGE_PIX_FMT_JPG);
  printk("took image %d bytes\n", img_size);
  uint8_t *buf = k_heap_alloc(&mqtt_heap, img_size, K_FOREVER);
  if (buf == NULL) {
    LOG_ERR("Error allocating Memory\n");
  }
  for (int i = 0; i < (img_size); i++) {
    buf[i] = camera_read_byte();
  };
  printk("fetched raw image\n");
  // optional: bas64 encode image
  /*size_t dlen = 0;
  size_t olen = 0;
  int b64_ret = base64_encode(NULL, dlen, &olen, buf, img_size);
  printk("Encoded string needs %d bytes\n", olen);
  // set new destination size to what would be needed for the b64 encoded string
  dlen = olen;
  uint8_t *b64_buf = k_heap_alloc(&img_heap, olen, K_FOREVER);
  // encode to b64_buf
  b64_ret = base64_encode(b64_buf, dlen, &olen, buf, img_size);
  printk("Error code: %d", b64_ret);
  struct mqtt_payload payload = {
      .data = b64_buf, .size = olen, .topic = "image__1234"};
 */
  k_msgq_put(&mqtt_msgq, &payload, K_FOREVER);
  run_mqtt(&mqtt_msgq);
  k_heap_free(&mqtt_heap, payload.data);

  // fetch temperature data
  struct sensor_value temp;
  sensor_sample_fetch(dev);
  sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
  char temp_string[32];
  sprintf(temp_string, "{\n\"temperature\": %d.%d\n}", temp.val1, temp.val2);
  payload.data = temp_string;
  payload.size = strlen(temp_string);
  payload.topic = "temperature";

  k_msgq_put(&mqtt_msgq, &payload, K_FOREVER);
  run_mqtt(&mqtt_msgq);

  k_sleep(K_SECONDS(1));

  // Reboot
  printk("rebooting in 15 minutes\n");
  k_sleep(K_MINUTES(15));
  sys_reboot(SYS_REBOOT_COLD);
}
