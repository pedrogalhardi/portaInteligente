import paho.mqtt.client as mqtt
import os

MQTT_BROKER = os.getenv("MQTT_BROKER", "localhost")
MQTT_PORT = 1883
MQTT_TOPICS = ["porta/status"]

class MQTTClient:
    def __init__(self):
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_publish = self.on_publish

        self.messages = {topic: None for topic in MQTT_TOPICS}

    def on_connect(self, client, userdata, flags, rc):
        print(f"Conectado com código {rc}")
        for topic in MQTT_TOPICS:
            self.client.subscribe(topic)

    def on_message(self, client, userdata, msg):
        # print(f"Mensagem recebida em {msg.topic}: {msg.payload.decode()}")
        self.messages[msg.topic] = msg.payload.decode()

    def on_publish(self, client, userdata, mid):
        print(f"Mensagem publicada com ID: {mid}")

    def start(self):
        self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
        self.client.loop_start()

    def get_last_message(self, topic):
        return self.messages.get(topic)

    def publish_message(self, topic, message):
        self.client.publish(topic, message)