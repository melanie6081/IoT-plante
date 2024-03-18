#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>  // Ajoutez cette ligne pour la gestion MQTT

const char* ssid = "Pixel Coline";           // Remplacez par le nom de votre réseau WiFi
const char* password = "Raclette"; // Remplacez par le mot de passe de votre réseau WiFi
const char* mqttServer = "maqiatto.com"; // Remplacez par l'adresse IP ou le nom du broker MQTT
const int mqttPort = 1883;  // Port MQTT par défaut

const char* mqttUsername = "colineauber@yahoo.fr";  // Si nécessaire, sinon commentez cette ligne
const char* mqttPassword = "plante";  // Si nécessaire, sinon commentez cette ligne

const char* mqttTopic = "plante";  // Remplacez par le topic MQTT que vous souhaitez utiliser

WiFiClient espClient;
PubSubClient client(espClient);

int ledPin = 5;

void callback(char* topic, byte* payload, unsigned int length) {
  // Traitement des messages MQTT entrants
  Serial.print("Message reçu sur le topic: ");
  Serial.println(topic);

  Serial.print("Contenu du message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Fonction de reconnexion au broker MQTT
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    
    if (client.connect("ESP8266Client", mqttUsername, mqttPassword)) {
      Serial.println("Connecté au broker MQTT");
      client.subscribe(mqttTopic);  // S'abonner au topic MQTT
    } else {
      Serial.print("Échec, rc=");
      Serial.print(client.state());
      Serial.println(" Réessayez dans 5 secondes");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.println();
  Serial.print("Connexion Wi-Fi à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecter");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  digitalWrite(ledPin, HIGH);
  Serial.println();

  Serial.println("Wi-Fi Connecté Succès !");
  Serial.print("NodeMCU IP Address : ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);  // Définir la fonction de rappel pour le traitement des messages entrants
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Envoyer une donnée au broker MQTT
  client.publish(mqttTopic, "Hello, MQTT!");
  
  delay(1000);
}