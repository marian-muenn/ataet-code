#ifndef MM_CONFIG_H
#define MM_CONFIG_H

/**
 * \brief WIFI SSID
 */
#define SSID "SSID"
/**
 * \brief WIFI PSK
 */
#define PSK "WLAN PASSWORT"

/**
 * \brief SNTP Server Name
 */
#define SNTP_SERVER_FQDN "ptbtime1.ptb.de"
/**
 * \brief SNTP Server IP
 */
#define SNTP_SERVER_IP "192.53.103.108"

/**
 * MQTT Port Number
 */
#define MQTT_CONNECT_TIMEOUT_MS 2000
#define MQTT_SLEEP_MSECS 500
#define MQTT_CONNECT_TRIES 4
#define MQTT_BUFFER_SIZE 1024
#define MQTT_CLIENTID "zephyr"
#define MQTT_SERVER_FQDN "zapdos.eit.htwk-leipzig.de"
#define MQTT_SERVER_ADDR "141.57.24.186"
#define MQTT_SERVER_PORT 8883

#define MQTT_USERNAME "MQTT BENUTZER"
#define MQTT_PASSWD "MQTT PASSWORT"

#define CONFIG_NET_SAMPLE_APP_MAX_ITERATIONS 1
#define CONFIG_NET_SAMPLE_APP_MAX_CONNECTIONS 1

#endif // !CONFIG_H
