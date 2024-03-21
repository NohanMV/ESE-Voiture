#include <WiFi.h>

const char *ssid = "ESP32_AP";
const char *password = "mot_de_passe";

WiFiServer server(8080);

void setup() {
  Serial.begin(115200);
  
  // Initialisation du point d'accès WiFi
  WiFi.softAP(ssid, password);
  delay(100);

  // Démarrage du serveur sur le port 8080
  server.begin();
  
  Serial.println("Point d'accès WiFi créé");
}

void loop() {
  // Attente d'une connexion cliente
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("Client connecté");

    // Attente des données envoyées par le client
    while (client.connected()) {
      if (client.available()) {
        // Lecture des données et affichage sur le moniteur série
        String data = client.readStringUntil('\n');
        Serial.println("Données reçues : " + data);
      }
    }

    // Fermeture de la connexion cliente
    client.stop();
    Serial.println("Client déconnecté");
  }
}