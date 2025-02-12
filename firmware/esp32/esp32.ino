#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "2.4G";
const char* password = "eunaosei";
const char* mqtt_server = "192.168.1.3";

WiFiClient espClient;
PubSubClient client(espClient);

// Pinos da ESP
const int sensorPin = 4;
const int portaPin = 15;

// Variáveis de status
volatile bool portaAberta = false;

void IRAM_ATTR sensorInterrupt(){
  portaAberta = !portaAberta;
  if (portaAberta) {
    client.publish("porta/status", "ABERTA");
    Serial.println("Status: Porta aberta.");
  } else {
    client.publish("porta/status", "FECHADA");
    Serial.println("Status: Porta fechada.");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (message == "OPEN") {
    digitalWrite(portaPin, HIGH);
    delay(1000);
    digitalWrite(portaPin, LOW);
    Serial.println("Porta aberta.");
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado ao MQTT, assinando o tópico.");
      client.subscribe("porta/comando");
    } else {
      Serial.print("Falha na conexão MQTT, tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(portaPin, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Conectando-se à rede Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println("Conexão Wi-Fi bem-sucedida!");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  client.publish("porta/status", "FECHADA");
  attachInterrupt(digitalPinToInterrupt(sensorPin), sensorInterrupt, CHANGE);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // client.publish("porta/status", "FECHADA");
  delay(1000);
}