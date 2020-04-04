#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>

//https://github.com/tekk/TTGO-T-Beam-Car-Tracker/blob/master/TTGO-T-Beam-Car-Tracker.ino
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND  868E6
char identifier[3] = "wt"; //For water tank. Helps disambiguate from other Lora senders
unsigned int waterLevel = 0;

void setup() {
  pinMode(16, OUTPUT);
  pinMode(2, OUTPUT);
  analogReadResolution(10); // Default of 12 is not very linear. Recommended to use 10 or 11 depending on needed resolution.

  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Sender Test");

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("init ok");
}

void loop() {
  Serial.print(identifier);
  Serial.print(",");
  Serial.println(waterLevel);
  //Comma <identifier>,<water level>
  LoRa.beginPacket();
  LoRa.print(identifier);
  LoRa.print(",");
  LoRa.print(waterLevel);
  LoRa.endPacket();
  if (waterLevel == 400) {
    waterLevel = 0;
  } else {
    waterLevel += 20;
  }
  delay(2000);
}
