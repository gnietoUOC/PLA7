//#include <WiFi101.h>
//#include <PubSubClient.h>

#include "Homie.h"

// Update these with values suitable for your network.

int status = WL_IDLE_STATUS;     

unsigned long next;

#ifdef SECURED
WiFiSSLClient wifiClient;
#else
WiFiClient wifiClient;
#endif
PubSubClient *client;

// Objeto que gestionar el temporizador. Utilizamos el temporizador 4
// porque tiene una resolución de 32 bits. Sin esa resolución y con esa 
// velocidad de reloj no podríamos medir más de 1sg. 
//Adafruit_ZeroTimer timer = Adafruit_ZeroTimer(4);

Homie *homie;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("*** MKR1000 ***");

  // Inicializamos la tarjeta 
  SD.begin(4);
  
  connectWiFi();
  dumpWiFi();

  client = new PubSubClient(wifiClient);
  client->setServer(MQHOST, MQPORT);

  defineDevice();
  homie->dump();

  client->setCallback(callback);
  client->subscribe("Homie/+/+/+/Set");
  
  next = millis();
}

void loop() {
  if (millis()>next) {
    next += MQPERIOD;
    homie->update();
  }

  homie->reconnect();

  client->loop();
 
}

// Vuelco información básica de la conexión
void dumpWiFi() {

  // Imprimo la SSID de la conexión
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Imprimo la configuración de red
  IPAddress ip = WiFi.localIP();
  Serial.print("IP address:  ");
  Serial.println(ip);

  // Máscara de red
  Serial.print("Subnet mask: ");
  Serial.println((IPAddress)WiFi.subnetMask());

  // Dirección del gateway
  Serial.print("Gateway IP:  ");
  Serial.println((IPAddress)WiFi.gatewayIP());

}

// Conexión al AP mediante WiFi
void connectWiFi() {
  
  // Verificamos que la Ethernet está disponible.
  // En una MKR1000 nunca debería fallar.
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  // Nos conectamos al AP (Raspberry)
  while ( status != WL_CONNECTED) {
    Serial.print(".");
    status = WiFi.begin(PISSID,PIPWD);

    // Espero un poco antess de reintentar la conexión
    delay(5000);
  }
  Serial.println("\nConnected.");
}

// Función que es invocada cuando el broker MQTT nos envía un mensaje
// al que previamente nos habíamos suscrito.
// TODO: Registrar directamente un método de la instancia de Homie.
void callback(char* topic, uint8_t* payload, unsigned int length) {
  DPRINTLN("-> Callback");
  homie->callback(topic, payload, length);
  DPRINTLN("<- Callback");
 }

// Método que define nuestro dispositivo para que podamos gestionar
// su estado utilizando la convención Homie
void defineDevice() {

  DPRINTLN("-> defineDevice");

  homie = new Homie(client);

  Device *device = new Device(client,homie,(char *)"MKR1000");

  Node *node = new Node(client, device,(char *)"MKRCORE");

  Memory *m = new Memory(client,node);
  LED *l = new LED(client,node,6);

  node = new Node(client, device,(char *)"MKRRELAY");
  Relay *r = new Relay(client,node,1);
  r = new Relay(client,node,2);

  // Si no está conectado el MKRENV, no añado el módulo de sensores
  // a la definición del dispositivo.
  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
  } else {
    node = new Node(client, device,(char *)"MKRENV");
    Temperature *t = new Temperature(client, node);
    Humidity *h = new Humidity(client, node);
    Pressure *p = new Pressure(client, node);
    Illuminance *i = new Illuminance(client, node);
    UVA *ua = new UVA(client, node);
    UVB *ub = new UVB(client, node);
    UVIndex *u = new UVIndex(client, node);
  }

  DPRINTLN("<- defineDevice");
}

int copy (char *in, char *out) {
  char bb[128];
  int n;
  int rc=0;

  File fin = SD.open(in, FILE_READ);
  if (fin) {
    File fout = SD.open(out, FILE_WRITE);
    if (fout) {
      while ((n=fin.read(bb,sizeof(bb)))>0) {
//        Serial.println(n);
        fout.write(bb,n);
      }
      fout.close();
    } else {
      Serial.println("Could not open file for WRITE");
      rc = 2;
    }
    fin.close();
  } else {
    Serial.println("Could not open file for READ");
    rc = 1;
  }
  return rc;
}

int move (char *in, char *out) {
  int rc = 0;
  if (!copy(in,out)) {
    if (!SD.remove(in)) {
      Serial.println("Could not remove the file.");
      rc = 2;
    }
  } else {
    Serial.println("Could not copy the file.");
    rc = 1;
  }
}

void clearLog() {
  Serial.println("** Clearing Log **");
  move ((char *)"DATALOG.TXT",(char *)"LOG00.TXT");
}
