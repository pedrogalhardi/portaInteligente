#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>

WebServer server(80);

String wifiSSID = "";
String wifiPassword = "";
String deviceName = "";
String mqttServer = "192.168.1.3";

struct Porta {
  String nome;
  int pinoSensor;
  int pinoPorta;
  volatile bool status;
};

Porta portas[4];

WiFiClient espClient;
PubSubClient client(espClient);

// Função para salvar as configurações na EEPROM
void saveConfig() {
  EEPROM.begin(512);

  // Salva SSID
  for (int i = 0; i < wifiSSID.length(); i++) {
    EEPROM.write(i, wifiSSID[i]);
  }
  EEPROM.write(wifiSSID.length(), '\0'); // Fim da string SSID

  // Salva senha Wi-Fi
  for (int i = 0; i < wifiPassword.length(); i++) {
    EEPROM.write(32 + i, wifiPassword[i]);
  }
  EEPROM.write(32 + wifiPassword.length(), '\0'); // Fim da string senha

  // Salva nome do dispositivo
  for (int i = 0; i < deviceName.length(); i++) {
    EEPROM.write(64 + i, deviceName[i]);
  }
  EEPROM.write(64 + deviceName.length(), '\0'); // Fim da string deviceName

  // Salva informações das portas
  int addr = 96;
  for (int i = 0; i < 4; i++) {
    EEPROM.write(addr++, portas[i].pinoSensor);
    EEPROM.write(addr++, portas[i].pinoPorta);
    for (int j = 0; j < portas[i].nome.length(); j++) {
      EEPROM.write(addr++, portas[i].nome[j]);
    }
    EEPROM.write(addr++, '\0'); // Fim da string nome da porta
  }

  EEPROM.commit();
}

// Função para carregar as configurações da EEPROM
void loadConfig() {
  EEPROM.begin(512);

  // Carrega SSID
  wifiSSID = "";
  int i = 0;
  char c = EEPROM.read(i++);
  while (c != '\0' && i < 32) {
    wifiSSID += c;
    c = EEPROM.read(i++);
  }

  // Carrega senha Wi-Fi
  wifiPassword = "";
  i = 32;
  c = EEPROM.read(i++);
  while (c != '\0' && i < 64) {
    wifiPassword += c;
    c = EEPROM.read(i++);
  }

  // Carrega nome do dispositivo
  deviceName = "";
  i = 64;
  c = EEPROM.read(i++);
  while (c != '\0' && i < 96) {
    deviceName += c;
    c = EEPROM.read(i++);
  }

  // Carrega informações das portas
  i = 96;
  for (int j = 0; j < 4; j++) {
    portas[j].pinoSensor = EEPROM.read(i++);
    portas[j].pinoPorta = EEPROM.read(i++);
    portas[j].nome = "";
    c = EEPROM.read(i++);
    while (c != '\0' && i < 512) {
      portas[j].nome += c;
      c = EEPROM.read(i++);
    }
  }
}

// Função de interrupção para cada porta
void IRAM_ATTR sensorInterrupt(int index) {
  portas[index].status = !portas[index].status;
  if (portas[index].status) {
    client.publish("porta/" + portas[index].nome + "/status", "ABERTA");
    Serial.println("Status: " + portas[index].nome + " aberta.");
  } else {
    client.publish("porta/" + portas[index].nome + "/status", "FECHADA");
    Serial.println("Status: " + portas[index].nome + " fechada.");
  }
}

// Função callback para MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  for (int i = 0; i < 4; i++) {
    if (String(topic) == "porta/" + portas[i].nome + "/comando") {
      if (message == "OPEN") {
        digitalWrite(portas[i].pinoPorta, HIGH);
        delay(1000);
        digitalWrite(portas[i].pinoPorta, LOW);
        Serial.println(portas[i].nome + " aberta.");
      }
    }
  }
}

// Função para reconectar ao MQTT
void reconnect() {
  while (!client.connected()) {
    if (client.connect(deviceName.c_str())) {
      Serial.println("Conectado ao MQTT, assinando os tópicos.");
      for (int i = 0; i < 4; i++) {
        client.subscribe("porta/" + portas[i].nome + "/comando");
      }
    } else {
      Serial.print("Falha na conexão MQTT, tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

// Função para o servidor Web de configuração
void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Configuração do Dispositivo</h1>";
  html += "<form action='/config' method='POST'>";
  html += "Nome da Rede Wi-Fi: <input type='text' name='ssid'><br>";
  html += "Senha Wi-Fi: <input type='password' name='password'><br>";
  html += "Nome/ID do dispositivo: <input type='text' name='device_name'><br><br>";
  
  for (int i = 0; i < 4; i++) {
    html += "Porta " + String(i + 1) + " Nome: <input type='text' name='porta_nome_" + String(i) + "'><br>";
    html += "Porta " + String(i + 1) + " Pino Sensor: <input type='number' name='porta_sensor_" + String(i) + "'><br>";
    html += "Porta " + String(i + 1) + " Pino Controle: <input type='number' name='porta_porta_" + String(i) + "'><br><br>";
  }
  
  html += "<input type='submit' value='Enviar'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

// Função para processar os dados do formulário
void handleConfig() {
  wifiSSID = server.arg("ssid");
  wifiPassword = server.arg("password");
  deviceName = server.arg("device_name");

  for (int i = 0; i < 4; i++) {
    portas[i].nome = server.arg("porta_nome_" + String(i));
    portas[i].pinoSensor = server.arg("porta_sensor_" + String(i)).toInt();
    portas[i].pinoPorta = server.arg("porta_porta_" + String(i)).toInt();
  }

  saveConfig();  // Salva as configurações na EEPROM

  server.send(200, "text/html", "<html><body><h1>Configuração Concluída!</h1></body></html>");
}

void setup() {
  Serial.begin(115200);

  // Carregar configurações da EEPROM
  loadConfig();

  // Se as configurações não foram carregadas corretamente, entra no modo de configuração
  if (wifiSSID == "" || wifiPassword == "" || deviceName == "") {
    WiFi.softAP("ESP32-Config", "12345678");
    server.on("/", HTTP_GET, handleRoot);
    server.on("/config", HTTP_POST, handleConfig);
    server.begin();
    Serial.println("Servidor Web Iniciado. Acesse o ESP32 e configure.");
    while (true) {
      server.handleClient();  // Aguarda a configuração via web
    }
  } else {
    // Conectar ao Wi-Fi
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Wi-Fi Conectado!");

    // Inicializa o cliente MQTT
    client.setServer(mqttServer.c_str(), 1883);
    client.setCallback(callback);

    for (int i = 0; i < 4; i++) {
      pinMode(portas[i].pinoPorta, OUTPUT);
      pinMode(portas[i].pinoSensor, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(portas[i].pinoSensor), [i]() { sensorInterrupt(i); }, CHANGE);
      client.publish("porta/" + portas[i].nome + "/status", "FECHADA");
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(1000);
}
