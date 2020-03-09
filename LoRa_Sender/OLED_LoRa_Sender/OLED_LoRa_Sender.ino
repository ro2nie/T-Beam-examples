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
#define BATTERY_PIN 35 // battery level measurement pin, here is the voltage divider connected
unsigned int waterLevel = 0;

void setup() {
  pinMode(16, OUTPUT);
  pinMode(2, OUTPUT);
  // set battery measurement pin
  adcAttachPin(BATTERY_PIN);
  adcStart(BATTERY_PIN);
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

float getBatteryVoltage() {
  // we've set 10-bit ADC resolution 2^10=1024 and voltage divider makes it half of maximum readable value (which is 3.3V)
  return analogRead(BATTERY_PIN) * (3.3 / 1024.0);
}

void loop() {
  float batteryVoltage = getBatteryVoltage();
  Serial.print(String(waterLevel));
  Serial.print(",");
  Serial.println(String(batteryVoltage));
  //Comma <separated water level>,<battery voltage>
  LoRa.beginPacket();
  LoRa.print(waterLevel);
  LoRa.print(",");
  LoRa.print(batteryVoltage);
  LoRa.endPacket();
  if (waterLevel == 400) {
    waterLevel = 0;
  } else {
    waterLevel += 20;
  }
  delay(2000);
}
