#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    868E6

//LoRa
int rssi = 0;
String packSize = "--"; //TODO - remove usage of strings. Make it into char[]
char packet[12]; // TODO - check number of chars sent from sensor
//WiFi
const char* ssid     = "<fill in>";
const char* password = "<fill in>";
const char* mqttServer = "<fill in>";
const char* waterTankLevelTopic = "water-tank/level";

WiFiClient espClient;
PubSubClient client(espClient);

void initWiFi() {
  // Start by connecting to a WiFi network
  Serial.println("Connecting to: " + String(ssid));
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void initMqtt() {
  client.setServer(mqttServer, 1883);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ES32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loraData() {
  //  Serial.print(packSize + " bytes");
  //  Serial.print(" - ");
  //  Serial.print(packet);
  char* waterLevel;
  char* batteryVoltage;
  char* group = strtok(packet, ",");

  if (group) {
    waterLevel = group;
    //printf("%s\n", group);
  }
  group = strtok(NULL, ",");

  if (group) {
    batteryVoltage = group;
    //printf("%s\n", group);
  }
  //{"tank":96,"volts":6, "rssi":-80}   //TODO, add id for identifier. Doorbell will use LoRa
  char waterTankMessage[50];
  sprintf(waterTankMessage, "%s%s%s%s%s%d%s", "{\"tank\":", waterLevel, ",\"volts\":", batteryVoltage, ",\"rssi\":", rssi, "}");
  //  Serial.println("TO PUBLISH TO MQTT");
  //  Serial.println(waterTankMessage);
  client.publish(waterTankLevelTopic, waterTankMessage);
}

void cbk(int packetSize) {
  //  Serial.println(packetSize);
  packet[0] = '\0';
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) {
    packet[i] = (char) LoRa.read();
  }
  rssi = LoRa.packetRssi();
  loraData();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Receiver Callback");
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.onReceive(cbk);
  LoRa.receive();
  Serial.println("init ok");

  //WiFi
  initWiFi();
  initMqtt();
  delay(1500);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    cbk(packetSize);
  }
  delay(10);
}
