#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 27
#define SCL_PIN 26
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int counter = 0;

// Định nghĩa chân kết nối LoRa
#define ss 5
#define rst 14
#define dio0 22

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("LoRa TX - Long Range");
  display.display();
  
  Serial.println("LoRa TX - Long Range Mode");
  LoRa.setPins(ss, rst, dio0);
  
  while (!LoRa.begin(433E6)) {
    Serial.println("Initializing LoRa...");
    delay(500);
  }
  
  // Cấu hình tối ưu cho khoảng cách xa
  LoRa.setTxPower(20);              // Công suất tối đa 20 dBm (100mW)[1]
  LoRa.setSpreadingFactor(12);      // SF12 cho khoảng cách xa nhất[1]
  LoRa.setSignalBandwidth(62.5E3);  // Băng thông thấp 62.5 kHz[1]
  LoRa.setCodingRate4(8);           // Coding rate 4/8 cho độ tin cậy cao
  LoRa.setPreambleLength(8);        // Preamble dài hơn
  LoRa.setSyncWord(0xA5);           // Sync word để tránh nhiễu
  LoRa.enableCrc();                 // Bật CRC để kiểm tra lỗi
  
  Serial.println("LoRa Initialized - Long Range Configuration!");
  Serial.println("TX Power: 20 dBm");
  Serial.println("Spreading Factor: 12");
  Serial.println("Bandwidth: 62.5 kHz");
  Serial.println("Coding Rate: 4/8");
}

void loop() {
  // Gửi gói tin
  LoRa.beginPacket();
  LoRa.print("MSG:");
  LoRa.print(counter);
  LoRa.print(",RSSI:");
  LoRa.print(LoRa.packetRssi());
  LoRa.endPacket();
  
  // Hiển thị trên OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("LoRa TX - Long Range");
  display.setCursor(0, 15);
  display.print("Packet: ");
  display.println(counter);
  display.setCursor(0, 25);
  display.print("SF: 12, BW: 62.5kHz");
  display.setCursor(0, 35);
  display.print("TX Power: 20 dBm");
  display.setCursor(0, 45);
  display.print("Status: Sent");
  display.display();
  
  Serial.print("Sent packet: ");
  Serial.println(counter);
  counter++;
  delay(5000); // Delay dài hơn để tiết kiệm năng lượng
}
