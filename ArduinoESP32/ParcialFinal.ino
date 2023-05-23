#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin de datos del sensor DS18B20
#define ONE_WIRE_BUS 22
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int potPin = 34;  // Pin para la lectura del potenciómetro
const int Led = 2;      // Pin para el LED

// Configuración de la red Wi-Fi
const char* ssid = "BLH";             // Nombre de la red Wi-Fi
const char* password = "brianleonH";  // Contraseña de la red Wi-Fi

// Configuración del broker MQTT
const char* mqttBroker = "industrial.api.ubidots.com";  // Dirección IP del broker MQTT
const char* mqttClientId = "smartpoultrycare";      // Identificador del cliente MQTT
const char* inTopic1 = "/v1.6/devices/smartpoultrycare";  // Tópico para recibir mensajes
const char* inTopic2 = "/v1.6/devices/smartpoultrycare";  // Tópico para suscribirse

const char* mqttUser = "BBFF-QpCGTEAUsUsIuGxBAjaY0Zjruhbys4";  // Usuario del broker MQTT
const char* mqttPass = "1234";  // Contraseña del broker MQTT
const int mqttPort = 1883;  // Puerto del broker MQTT

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Función de callback para procesar los mensajes MQTT recibidos
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  int estado = doc["estado"];
  Serial.println(estado);
  digitalWrite(Led, estado == 1 ? HIGH : LOW);
}

void setup() {
  pinMode(Led, OUTPUT);
  Serial.begin(115200);  // Inicialización de la conexión Serial
  setupWiFi();           // Conexión a la red Wi-Fi
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);
  Serial.println("Setup done");
  delay(1500);
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Reconexión al broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqttClientId, mqttUser, mqttPass)) {
      Serial.println("connected");
      client.subscribe(inTopic2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Lectura del valor del potenciómetro y cálculo del valor de pH correspondiente
  int potValue = analogRead(potPin);
  int phValues[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
  int phValue = 0;
  int rangeSize = 4096 / 14;  // Tamaño de cada rango (aproximadamente 292)

  for (int i = 0; i < 14; i++) {
    int rangeMin = i * rangeSize;            // Valor mínimo del rango
    int rangeMax = (i + 1) * rangeSize - 1;  // Valor máximo del rango (exclusivo)

    if (potValue >= rangeMin && potValue <= rangeMax) {
      phValue = phValues[i];
      break;
    }
  }

  // Lectura de la temperatura desde el sensor DS18B20
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  int temperatura = (int)temperature;

  // Creación del objeto JSON y asignación de los valores de pH y temperatura
  StaticJsonDocument<64> doc;
  doc["ph"] = phValue;
  doc["temperatura"] = temperatura;


  // Serialización del objeto JSON en una cadena y publicación en el tópico correspondiente
  String payload;
  serializeJson(doc, payload);
  Serial.println("dato a enviar: " + payload);
  client.publish(inTopic1, payload.c_str());

  delay(5000);
}
