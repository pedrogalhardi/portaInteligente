#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Teste Wifi";
const char* password = "00998877";
const char* mqtt_server = "192.168.159.76";

WiFiClient espClient;
PubSubClient client(espClient);

// Pinos do ESP32
const int sensorPin = 4;   // Sensor magnético MC-38
const int portaPin = 15;   // Controle do atuador (ex: relé)

// Variáveis de status
bool portaAberta = false;
bool statusMudou = false;

void IRAM_ATTR sensorInterrupt() {
  portaAberta = digitalRead(sensorPin) == HIGH;  // Se HIGH, porta fechada; se LOW, porta aberta
  statusMudou = true;  // Indica que o estado mudou para ser processado no loop principal
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
    if (client.connect("LaTIN")) {
      Serial.println("Conectado ao MQTT, assinando o tópico.");
      client.subscribe("porta/comando");
    } else {
      Serial.print("Falha na conexão MQTT, tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(portaPin, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP);  // Usa resistor pull-up interno

  WiFi.begin(ssid, password);
  Serial.print("Conectando-se à rede Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println("Conexão Wi-Fi bem-sucedida!");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  attachInterrupt(digitalPinToInterrupt(sensorPin), sensorInterrupt, CHANGE);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Processa a mudança de status do sensor
  if (statusMudou) {
    statusMudou = false;
    if (portaAberta) {
      client.publish("porta/status", "ABERTA");
      Serial.println("Status: Porta aberta.");
    } else {
      client.publish("porta/status", "FECHADA");
      Serial.println("Status: Porta fechada.");
    }
  }

  delay(100);  // Pequeno delay para estabilidade
}