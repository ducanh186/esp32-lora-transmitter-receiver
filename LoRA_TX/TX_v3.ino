/* This sketch is for the LoRa long-range transmission 
      Maximizing the range using SF12, BW62.5kHz, and other settings.
      It uses an OLED display to show the transmission status.
 */
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ---------- Pin map ---------- */
#define LORA_SCK   23   
#define LORA_MISO  19   
#define LORA_MOSI  18   
#define LORA_SS     5   
#define LORA_RST   14   
#define LORA_DIO0  22   

#define OLED_SDA   27   
#define OLED_SCL   26   
#define SCREEN_W 128
#define SCREEN_H  64
#define OLED_RST  -1

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RST);

/* ---------- LoRa payload ---------- */
uint32_t counter = 0;

/* ---------- Setup ------------------------------------------------------- */
void setup() {
  Serial.begin(115200);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); display.display();

  /* ---- LoRa init ---- */
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  while (!LoRa.begin(433E6)) {            // 433 MHz band
    Serial.print(".");
    delay(500);
  }

  /* ---- Long-range profile ---- */
  LoRa.setTxPower(20);                    // 20 dBm PA_BOOST
  LoRa.setSpreadingFactor(12);            // SF12 – max sensitivity
  LoRa.setSignalBandwidth(62.5E3);        // Narrow BW for better SNR
  LoRa.setCodingRate4(8);                 // 4/8 forward-error coding
  LoRa.setPreambleLength(12);             // Longer preamble improves sync
  LoRa.setGain(6);                        // Max LNA gain (boost)
  LoRa.setOCP(140);                       // 140 mA over-current protection
  LoRa.enableCrc();                       // CRC for reliability

  Serial.println("\nLoRa TX ready (long-range mode)");
}

/* ---------- Main loop --------------------------------------------------- */
void loop() {
  Serial.println("Sending → " + counter);

  LoRa.beginPacket();
  LoRa.print(counter);
  LoRa.endPacket();

  drawScreen(counter);
  if(LoRa.endPacket()){
      counter++;} // increment packet counter
  delay(1000);                            // send every 1 s
}

/* ---------- OLED helper ------------------------------------------------- */
void drawScreen(const int &text) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("LoRa TX (SF12 BW62k)");
  display.print("Data: "); 
  display.println(counter);
  display.display();
}
