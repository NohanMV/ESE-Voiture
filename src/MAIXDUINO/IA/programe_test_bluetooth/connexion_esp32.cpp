
#include <WiFi.h>
#include <HardwareSerial.h>

const char *ssid = "ESP32_AP";
const char *password = "mot_de_passe";

WiFiServer server(8080);
HardwareSerial MySerial(1); // define a Serial for UART1

const int MySerialRX = 16;
const int MySerialTX = 17;

void setup() {
  Serial.begin(115200);
  MySerial.begin(9600, SERIAL_8N1, MySerialRX, MySerialTX);
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
        
        if (data.startsWith("panneau130.")){
          while (MySerial.available() > 0) {
              uint8_t byteFromSerial = MySerial.read();
          }
          MySerial.write(0x0F); 
        }

        if (data.startsWith("panneau50.")){
          while (MySerial.available() > 0) {
              uint8_t byteFromSerial = MySerial.read();
          }
          MySerial.write(0x0E); 
        }  
        
        if (data.startsWith("Sinter.")){
          while (MySerial.available() > 0) {
              uint8_t byteFromSerial = MySerial.read();
          }
          MySerial.write(0x10); 
        }  

        if (data.startsWith("frouge.")){
          while (MySerial.available() > 0) {
              uint8_t byteFromSerial = MySerial.read();
          }
          MySerial.write(0x07); 
        }  

        if (data.startsWith("stop.")){
          while (MySerial.available() > 0) {
              uint8_t byteFromSerial = MySerial.read();
          }
          MySerial.write(0x11); 
        }  

      }
    }

    // Fermeture de la connexion cliente
    client.stop();
    Serial.println("Client déconnecté");
  }
}
