import paho.mqtt.client as mqtt
import ssl
import base64


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("image")


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    f = open("./test.jpg", "wb")
    # out = base64.b64decode(msg.payload)
    # f.write(out)
    # print("wrote {} bytes".format(len(out)))
    f.write(msg.payload)
    print("wrote {} bytes".format(len(msg.payload)))
    f.close()


mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.tls_set(cert_reqs=ssl.CERT_NONE)
mqttc.tls_insecure_set(True)
mqttc.username_pw_set(username="BENUTZER", password="PASSWORT")
mqttc.on_connect = on_connect
mqttc.on_message = on_message

mqttc.connect("zapdos.eit.htwk-leipzig.de", 8883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqttc.loop_forever()
