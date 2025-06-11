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

// Định nghĩa chân kết nối LoRa
#define ss 5
#define rst 14
#define dio0 22

String receivedData = "";
int packetCount = 0;

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
  display.println("LoRa RX - Long Range");
  display.display();
  
  Serial.println("LoRa RX - Long Range Mode");
  LoRa.setPins(ss, rst, dio0);
  
  while (!LoRa.begin(433E6)) {
    Serial.println("Initializing LoRa...");
    delay(500);
  }
  
  // Cấu hình giống bên gửi để đồng bộ
  LoRa.setSpreadingFactor(12);      // SF12 cho khoảng cách xa nhất[1]
  LoRa.setSignalBandwidth(62.5E3);  // Băng thông thấp 62.5 kHz[1]
  LoRa.setCodingRate4(8);           // Coding rate 4/8
  LoRa.setPreambleLength(8);        // Preamble dài hơn
  LoRa.setSyncWord(0xA5);           // Sync word giống bên gửi
  LoRa.enableCrc();                 // Bật CRC
  
  Serial.println("LoRa Initialized - Long Range Configuration!");
  Serial.println("Spreading Factor: 12");
  Serial.println("Bandwidth: 62.5 kHz");
  Serial.println("Coding Rate: 4/8");
  Serial.println("Waiting for packets...");
}

void loop() {
  // Kiểm tra gói tin nhận được
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // Đọc dữ liệu nhận được
    receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    
    // Lấy thông số tín hiệu
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    long frequency_error = LoRa.packetFrequencyError();
    
    packetCount++;
    
    // Hiển thị trên Serial
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
    
    // Hiển thị trên OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("LoRa RX - Long Range");
    display.setCursor(0, 12);
    display.print("Data: ");
    display.println(receivedData.substring(0, 10)); // Hiển thị 10 ký tự đầu
    display.setCursor(0, 22);
    display.print("RSSI: ");
    display.print(rssi);
    display.println(" dBm");
    display.setCursor(0, 32);
    display.print("SNR: ");
    display.print(snr);
    display.println(" dB");
    display.setCursor(0, 42);
    display.print("Packets: ");
    display.println(packetCount);
    display.setCursor(0, 52);
    display.print("SF:12 BW:62.5kHz");
    display.display();
  }
  
  delay(100); // Delay ngắn để không bỏ lỡ gói tin
}
