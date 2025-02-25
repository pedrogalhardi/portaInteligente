# mqtt_client_handler.py
import threading
from mqtt_client import MQTTClient

# Instancia o cliente MQTT
mqtt_client_instance = MQTTClient()

# Função que inicia o cliente MQTT
def start_mqtt():
    try:
        mqtt_client_instance.start()
    except Exception as e:
        print(f"Erro ao iniciar o cliente MQTT: {e}")

# Inicia o cliente MQTT em um thread separado
mqtt_client_thread = threading.Thread(target=start_mqtt)
mqtt_client_thread.daemon = True
mqtt_client_thread.start()

# Função para acessar o cliente MQTT
def get_mqtt_client():
    return mqtt_client_instance
