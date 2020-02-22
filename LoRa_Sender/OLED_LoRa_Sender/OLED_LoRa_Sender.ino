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

unsigned int counter = 0;

String rssi = "RSSI --";
String packSize = "--";
String packet ;

void setup() {
  pinMode(16, OUTPUT);
  pinMode(2, OUTPUT);
  
  // set battery measurement pin
  adcAttachPin(BATTERY_PIN);
  adcStart(BATTERY_PIN);
  analogReadResolution(10); // Default of 12 is not very linear. Recommended to use 10 or 11 depending on needed resolution.

  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

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
  //LoRa.onReceive(cbk);
  //LoRa.receive();
  Serial.println("init ok");
}

int getBatteryVoltage() {
  // we've set 10-bit ADC resolution 2^10=1024 and voltage divider makes it half of maximum readable value (which is 3.3V)
  return analogRead(BATTERY_PIN) * 2.0 * (3.3 / 1024.0);
}


void loop() {
  Serial.println(String(counter));

  //send packet
  LoRa.beginPacket();
  LoRa.print("{\"tank\":");
  LoRa.print(counter);
  LoRa.print(",");
  LoRa.print("\"volts\":");
  LoRa.print(getBatteryVoltage());
  LoRa.print("}");
  LoRa.endPacket();

  //Dummy percentage
  if (counter < 100) {
    counter++;
  } else {
    counter = 0;
  }
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
