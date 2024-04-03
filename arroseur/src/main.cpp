#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>  // Bibliothèque pour la gestion MQTT

#include <DHT.h>

#define DHTPIN D4
// Définition du type de capteur utilisé
#define DHTTYPE DHT11

// Paramètres WiFi
const char* ssid = "Pixel Coline";           // Remplacez par le nom de votre réseau WiFi
const char* password = "Raclette";           // Remplacez par le mot de passe de votre réseau WiFi

// Paramètres MQTT
const char* mqttServer = "maqiatto.com";     // Remplacez par l'adresse IP ou le nom du broker MQTT
const int mqttPort = 1883;                   // Port MQTT par défaut
const char* mqttUsername = "colineauber@yahoo.fr";  // Nom d'utilisateur MQTT
const char* mqttPassword = "plante";                 // Mot de passe MQTT
const char* mqttTopic = "colineauber@yahoo.fr/plante";  // Remplacez par le topic MQTT que vous souhaitez utiliser
const char* mqttH = "colineauber@yahoo.fr/capteur/humidite"; // Topic MQTT pour l'humidité
const char* mqttT = "colineauber@yahoo.fr/capteur/temperature"; // Topic MQTT pour la température
const char* mqttA = "colineauber@yahoo.fr/capteur/alerte"; // Topic MQTT pour les alertes
const char* mqttB = "colineauber@yahoo.fr/bouton"; // Topic MQTT pour les alertes

// Broche pour le relais de la pompe
const int relais_pompe = D2; // // le relais est connecté à la broche 2 de la carte Adruino

WiFiClient espClient;
PubSubClient client(espClient);

int ledPin = 5;

int percentageHumidity = 50; // Valeur par défaut pour l'humidité
int temp = 20; // Valeur par défaut pour la température
int boutonEtat = 0;

int time_1s = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  // Traitement des messages MQTT entrants
  String percentageHumidityString = String("");
  String tempString = String("");
  String boutonString = String("");

  // Analyse des messages en fonction des topics
  if(strcmp(topic, mqttH) == 0){
    // Si le topic est pour l'humidité
    for (int i = 0; i < length; i++) {
      percentageHumidityString += (char)payload[i];
    }
    // Conversion de la valeur de l'humidité en entier
    percentageHumidity = percentageHumidityString.toInt();
  }

  if(strcmp(topic, mqttT) == 0){
    // Si le topic est pour la température
    for (int i = 0; i < length; i++) {
      tempString += (char)payload[i];
    }
    // Conversion de la valeur de la température en entier
    temp = tempString.toInt();
  }
  if(strcmp(topic, mqttB) == 0){
    boutonString += (char)payload[0];
  }
  boutonEtat = boutonString.toInt();

}

void reconnect() {
    /* Code Mélanie
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
  */

  // Fonction de reconnexion au broker MQTT
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    
    if (client.connect("toto", mqttUsername, mqttPassword)) {
      Serial.println("Connecté au broker MQTT");
      client.subscribe(mqttH);
      client.subscribe(mqttA);
      client.subscribe(mqttT);
      client.subscribe(mqttB);
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

  // Attente de connexion au réseau WiFi
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

  // Vérification de la connexion au broker MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - time_1s > 1000){
    time_1s = millis();
  // Affichage des valeurs actuelles d'humidité et de température
  Serial.print("L'humidité est de ");
  Serial.print(percentageHumidity);
  Serial.println(" %");

  Serial.print("La température est de ");
  Serial.print(temp);
  Serial.println(" °C");

  Serial.print("L'etat du bouton est à : ");
  Serial.println(boutonEtat);
  }

  bool alerte;

  // Détermination de l'alerte en fonction de la température
  if(temp > 32 || temp <= 0){
    alerte = true;
  }

  if(temp < 32 || temp > 0){
    alerte = false;
  }

  // Gestion des alertes en fonction de l'humidité et de la température
  if(percentageHumidity <= 15 && alerte == true){
    String message = "Alerte! La temperature est de: " + String(temp) + "°C, l'arrosage automatique a été annulé mais l'humidité est trop basse, elle est actuellement à: " + String(percentageHumidity) + "%";
    client.publish(mqttA, message.c_str());
  }

  if(percentageHumidity > 70){
    String message = "Alerte! Le taux d'humidité est trop élevé, il est actuellement à : " + String(percentageHumidity) + "%";
    client.publish(mqttA, message.c_str());
  }

  if(percentageHumidity <= 30 && alerte == false || boutonEtat == 1){
    digitalWrite(relais_pompe, HIGH);
    delay(1000); //1s
    digitalWrite(relais_pompe, LOW);
    boutonEtat = 0;
  }

  delay(10);
}

