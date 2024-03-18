#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>  // Ajoutez cette ligne pour la gestion MQTT

#include <DHT.h>

#define DHTPIN D4
// Definit le type de capteur utilise
#define DHTTYPE DHT11

const char* ssid = "Pixel Coline";           // Remplacez par le nom de votre réseau WiFi
const char* password = "Raclette"; // Remplacez par le mot de passe de votre réseau WiFi
const char* mqttServer = "maqiatto.com"; // Remplacez par l'adresse IP ou le nom du broker MQTT
const int mqttPort = 1883;  // Port MQTT par défaut

const char* mqttUsername = "colineauber@yahoo.fr";  // Si nécessaire, sinon commentez cette ligne
const char* mqttPassword = "plante";  // Si nécessaire, sinon commentez cette ligne

const char* mqttTopic = "colineauber@yahoo.fr/plante";  // Remplacez par le topic MQTT que vous souhaitez utiliser
const char* mqttH = "colineauber@yahoo.fr/capteur/humidite";
const char* mqttT = "colineauber@yahoo.fr/capteur/temperature";
const char* mqttA = "colineauber@yahoo.fr/alerte";


const int relais_pompe = D2; // // le relais est connecté à la broche 2 de la carte Adruino

WiFiClient espClient;
PubSubClient client(espClient);

int ledPin = 5;

void callback(char* topic, byte* payload, unsigned int length) {

  int percentageHumididy;
  float temp;
  boolean alerte=false;

  Serial.println(String((char*) payload));

  if(topic==mqttH){
    for (int i = 0; i < length; i++) {
      percentageHumididy=(int)payload[i];
      Serial.print("Message reçu sur le topic: ");
      Serial.println(topic);

      Serial.print("Contenu du message: ");
      Serial.print(percentageHumididy + " %");
    }
  }

  if(topic==mqttT){
    for (int i = 0; i < length; i++) {
      temp=(float)payload[i];
      Serial.print("Message reçu sur le topic: ");
      Serial.println(topic);

      Serial.print("Contenu du message: ");
      Serial.print(temp);
    }
  }

  if(topic==mqttA){
    for (int i = 0; i < length; i++) {
      alerte=(boolean)payload[i];
      Serial.print("Message reçu sur le topic: ");
      Serial.println(topic);

      Serial.print("Contenu du message: ");
      Serial.print(alerte);
    }
  }
  
  // Traitement des messages MQTT entrants
  //Serial.print("Message reçu sur le topic: ");
  //Serial.println(topic);

  /**Serial.print("Contenu du message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();**/

  if(percentageHumididy <= 30 && alerte == false){
    (relais_pompe, HIGH);
    delay(5000); //5s
    digitalWrite(relais_pompe, LOW);
  }

}

void reconnect() {

  char mac[19]={0};
  // Fonction de reconnexion au broker MQTT
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    
    WiFi.macAddress().toCharArray(mac, 19);
    if (client.connect(mac, mqttUsername, mqttPassword)) {
      Serial.println("Connecté au broker MQTT");
      client.subscribe(mqttH);
      client.subscribe(mqttA);
      client.subscribe(mqttT);
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

  pinMode(relais_pompe, OUTPUT);
}

void loop() {
  //digitalWrite(relais_pompe, HIGH);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(3000);

}
