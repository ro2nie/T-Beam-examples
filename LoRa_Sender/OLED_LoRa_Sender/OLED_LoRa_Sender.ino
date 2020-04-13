#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <axp20x.h>
AXP20X_Class axp;

//https://github.com/tekk/TTGO-T-Beam-Car-Tracker/blob/master/TTGO-T-Beam-Car-Tracker.ino
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND  868E6
#define echoPin 2 // Echo Pin
#define trigPin 13 // Trigger Pin
long duration, distance; // Duration used to calculate distance
char identifier[3] = "wt";

void takeSensorMeasurement()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;
  Serial.println("Level: " + String(distance) + " cm");

  if (distance == 0) {
    distance = -1;
  }
}

void setup() {
  Wire.begin(21, 22);
  if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
    Serial.println("AXP192 Begin PASS");
  } else {
    Serial.println("AXP192 Begin FAIL");
  }
  //https://github.com/LilyGO/TTGO-T-Beam/issues/19
  //https://github.com/lewisxhe/AXP202X_Library
  //https://medium.com/@cglabs/ttgo-t-beam-where-is-power-led-cbd418756f07

  //When playing with these, if IDE fails to upload then try pressing and holding PWR button when flashing. Sometimes press repeatedly.
  //Turn what is needed on
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON); // Works but gives 1.8v on the 3v3 pin. To change it to 3v3 use axp.setDCDC1Voltage(3300)
  axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);// This turns on/off the 5v and 3v3 pin header on the board to save power
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
  axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON); // Required for LoRa
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF); // GPS power (NEO chip)
  axp.setDCDC1Voltage(3300); //Set Pin header on the board to 3.3V line to 3.3V (If this line is not present, will deliver 1.8).

  //LED controls
  //axp.setChgLEDMode(AXP20X_LED_OFF); // LED off
  //axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ); // 1blink/sec, low rate
  //axp.setChgLEDMode(AXP20X_LED_BLINK_4HZ); // 4blink/sec, high rate
  //axp.setChgLEDMode(AXP20X_LED_LOW_LEVEL); // LED full on
  //axp.setChgLEDMode(AXP20X_LED_LOW_LEVEL); // LED full on requires axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

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
  takeSensorMeasurement();
  Serial.print(identifier);
  Serial.print(",");
  Serial.println(distance);
  //<identifier>,<water level>
  LoRa.beginPacket();
  LoRa.print(identifier);
  LoRa.print(",");
  LoRa.print(distance);
  LoRa.endPacket();
  delay(2000);
}
