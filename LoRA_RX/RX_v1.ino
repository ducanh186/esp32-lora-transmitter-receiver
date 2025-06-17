/* This v1 receiver code works with v1 and v2 sender code.
 * It is designed to receive packets from a LoRa transmitter configured for long-range communication.
 * The receiver displays the received data, RSSI, SNR, and packet count on an OLED display.
 * It uses the Adafruit SSD1306 library for OLED display management and the LoRa library for LoRa communication.
 * The receiver is set up to work with the ESP32 platform and uses SPI and I2C protocols.
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

// SX1278 LoRa module pin definitions
#define ss 5
#define rst 14
#define dio0 22

String receivedData = "";
int packetCount = 0;

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);// Initialize I2C for OLED
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);// Initialize SPI for LoRa
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("LoRa RX - Long Range");
  display.display();
  
  Serial.println("LoRa RX - Long Range Mode");
  LoRa.setPins(ss, rst, dio0);
  
  while (!LoRa.begin(433E6)) {
    Serial.println("Initializing LoRa...");
    delay(500);
  }
  
  //Config same as sender for synchronization
  LoRa.setSpreadingFactor(12);      // SF12 for maximum range
  LoRa.setSignalBandwidth(62.5E3);  // LOW Bandwidth 62.5 kHz
  LoRa.setCodingRate4(8);           // Coding rate 4/8
  LoRa.setPreambleLength(8);        // longer preamble for better sync
  LoRa.setSyncWord(0xA5);           // Sync word with TX
  LoRa.enableCrc();                 // CRC on
  
  Serial.println("LoRa Initialized - Long Range Configuration!");
  Serial.println("Spreading Factor: 12");
  Serial.println("Bandwidth: 62.5 kHz");
  Serial.println("Coding Rate: 4/8");
  Serial.println("Waiting for packets...");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    
    // Record RSSI, SNR, and frequency error
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    long frequency_error = LoRa.packetFrequencyError();
    if(LoRa.available()){
      packetCount++;
    }
    
    // Serial display 
    Serial.println("=== Packet Received ===");
    Serial.print("Data: ");
    Serial.println(receivedData);
    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm");
    Serial.print("SNR: ");
    Serial.print(snr);
    Serial.println(" dB");
    Serial.print("Frequency Error: ");
    Serial.print(frequency_error);
    Serial.println(" Hz");
    Serial.print("Packet Count: ");
    Serial.println(packetCount);
    Serial.println("=====================");
    
    // OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("LoRa RX");
    display.setCursor(0, 17);
    display.print("Data: ");
    display.println(receivedData.substring(0, 30)); // Display first 30 characters
    display.setCursor(0, 27);
    display.print("RSSI:");
    display.print(rssi);
    display.println("dBm");
    display.setCursor(0, 37);
    display.print("SNR:");
    display.print(snr);
    display.println("dB");
    display.setCursor(0, 47);
    display.print("Fre Error: ");
    display.print(frequency_error);
    display.println(" Hz");
    display.display();
  }
  delay(100); 
}
