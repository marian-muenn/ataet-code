PINOUT
======

STLINK
------

- stlink board

PinCN6 | Designation | Description            | Nucleo Pin
-------|-------------|------------------------|-----------
1      | VDD_TARGET  | VDD from application   | 3V3
2      | SWCLK       | SWD clock              | CN11 Pin 15
3      | GND         | Ground                 | GND
4      | SWDIO       | SWD data input/output  | CN11 Pin 13
5      | NRST        | RESET of target STM32  | CN11 Pin 14
6      | SWO         | Reserved               |

usart1
------

- conflict with usb

usart2
------

- RX
  - PA3
  - ESP32-TX (17)
- TX
  - PD5
  - ESP32-RX (16)

usart3
------

- onboard-STLINK
- not on header
- RX
  - PD9
- TX
  - PD8

uart4
------

- SHELL
- RX
  - PC11
- TX
  - PA0/WKUP
- flow control conflicts with PB0 green LED

uart5
-----

- RX
  - PD2
  - CONSOLE, SHELL
- TX
  - PC12
  - CONSOLE, SHELL

usart6
------

- RX
  - PC7
- TX
  - PC6

uart7
-----

- RX
  - PF6
- TX
  - PF7

uart8
-----

- RX
  - PE0
- TX
  - PE1

SPI1
----

- SCK
  - PA5
- MISO
  - PA6
- MOSI
  - PB5

SPI2
----

- SCK
  - PB10
- MISO
  - PC2
- MOSI
  - PC3
