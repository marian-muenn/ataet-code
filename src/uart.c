#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include "uart.h"
#include <string.h>

// VICTRON UART
#define UART_DEVICE_NODE DT_NODELABEL(uart5)

#define MSG_SIZE 32

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 20, 4);

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

/* receive buffer used in UART ISR callback */
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data) {
  uint8_t c;

  if (!uart_irq_update(uart_dev)) {
    return;
  }

  if (!uart_irq_rx_ready(uart_dev)) {
    return;
  }

  /* read until FIFO empty */
  while (uart_fifo_read(uart_dev, &c, 1) == 1) {
    if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
      /* terminate string */
      rx_buf[rx_buf_pos] = '\0';

      /* if queue is full, message is silently dropped */
      k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

      /* reset the buffer (it was copied to the msgq) */
      rx_buf_pos = 0;
    } else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
      rx_buf[rx_buf_pos++] = c;
    }
    /* else: characters beyond buffer size are dropped */
  }
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf) { printk("%s", buf); }

char *uart_main(char *buff, size_t buffer_size) {
  char tx_buf[MSG_SIZE];

  if (!device_is_ready(uart_dev)) {
    printk("UART device not found!");
    return 0;
  }

  /* configure interrupt and callback to receive data */
  int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);

  if (ret < 0) {
    if (ret == -ENOTSUP) {
      printk("Interrupt-driven UART API support not enabled\n");
    } else if (ret == -ENOSYS) {
      printk("UART device does not support interrupt-driven API\n");
    } else {
      printk("Error setting UART callback: %d\n", ret);
    }
    return 0;
  }
  uart_irq_rx_enable(uart_dev);
  printk("uart enabled, reading victron messages\n");

  // Wait for full message queue and disable interrupts
  while (k_msgq_num_free_get(&uart_msgq) > 0) {
    k_sleep(K_MSEC(200));
  };
  uart_irq_rx_disable(uart_dev);
  strcat(buff, "{\n");

  /* indefinitely wait for input from the user */
  while (k_msgq_get(&uart_msgq, &tx_buf, K_MSEC(30)) == 0) {
    char *occurrence = strstr(tx_buf, "PID:");
    if (occurrence == NULL) {
      continue;
    } else {
      printk("%s", tx_buf);
      strcat(buff, tx_buf);
      break;
    };
  }
  while (k_msgq_get(&uart_msgq, &tx_buf, K_MSEC(30)) == 0) {
    char *occurrence = strstr(tx_buf, "Checksum");
    strcat(buff, tx_buf);
    if (occurrence == NULL) {
      continue;
    } else {
      break;
    };
  }
  return buff;
}
