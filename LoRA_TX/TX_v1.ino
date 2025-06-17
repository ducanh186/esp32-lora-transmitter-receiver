/* This ver 1 is auto transmit data every second with long range configuration
   * LoRa Transmitter using SX1278 module with Adafruit SSD1306 OLED display
   * This code is designed to work with the ESP32 platform.
   * It initializes the LoRa module and OLED display, then sends packets with a counter value.
   * The OLED displays the current packet number and transmission status.
   * The LoRa module is configured for long-range transmission.
*/
#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// OLED display pin definitions
#define SDA_PIN 27
#define SCL_PIN 26
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int counter = 0;

// SX1278 LoRa module pin definitions
#define ss 5
#define rst 14
#define dio0 22

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);// Initialize I2C for OLED
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);// Initialize SPI for LoRa
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("LoRa TX");
  display.display();
  
  Serial.println("LoRa TX - Long Range Mode");
  LoRa.setPins(ss, rst, dio0);
  
  while (!LoRa.begin(433E6)) {
    Serial.println("Initializing LoRa...");
    delay(500);
  }
  
  // Configuration for long range transmission
  LoRa.setTxPower(20);              // Maximum TX power 20 dBm
  LoRa.setSpreadingFactor(12);      // SF12 for maximum range
  LoRa.setSignalBandwidth(62.5E3);  // Low Bandwidth 62.5 kHz
  LoRa.setCodingRate4(8);           // Coding rate 4/8 for better error correction
  LoRa.setPreambleLength(8);        // Preamble longer
  LoRa.setSyncWord(0xA5);           // Sync word to match receiver
  LoRa.enableCrc();                 // CRC enabled for error checking
  
  Serial.println("LoRa Initialized - Long Range Configuration!");
  Serial.println("TX Power: 20 dBm");
  Serial.println("Spreading Factor: 12");
  Serial.println("Bandwidth: 62.5 kHz");
  Serial.println("Coding Rate: 4/8");
}

void loop() {
  // Send a packet 
  LoRa.beginPacket();
  LoRa.print("MSG:");
  LoRa.print(counter);
  LoRa.print(",");
  LoRa.print(LoRa.packetRssi());
  LoRa.endPacket();
  
  //OLED display update
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("LoRa TX");
  display.setCursor(0, 20);
  display.print("Packet: ");
  display.println(counter);
  display.setCursor(0, 30);
  if(Lora.endPacket()) {
    display.print("Status: Ready");
  } else {
    display.print("Status: Error");
  }
  display.display();
  
  Serial.print("Sent packet: ");
  Serial.println(counter);
  counter++;
  delay(1000); //
}
