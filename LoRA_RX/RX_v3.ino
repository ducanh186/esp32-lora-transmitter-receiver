#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ---------- Same pin map ---------- */
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

uint32_t lastCnt = 0;
uint32_t lost    = 0;

void setup() {
  Serial.begin(115200);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); display.display();

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  while (!LoRa.begin(433E6)) { delay(500); }

  /* replicate TX parameters */
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(8);
  LoRa.setPreambleLength(12);
  LoRa.setGain(6);
  LoRa.enableCrc();

  Serial.println("LoRa RX ready");
}

void loop() {
  int pktSize = LoRa.parsePacket();
  if (!pktSize) return;

  String recv = LoRa.readString();
  int rssi = LoRa.packetRssi();
  float snr = LoRa.packetSnr();

  /* --- extract counter --- */
  uint32_t id = recv.substring(0, recv.indexOf(':')).toInt();
  if (id > lastCnt + 1) lost += (id - lastCnt - 1);
  lastCnt = id;

  Serial.printf("ID:%lu  RSSI:%d  SNR:%.1f  \"%s\"\n", id, rssi, snr, recv.c_str());
  drawScreen(id, rssi, snr, lost);
}

/* ---------- OLED helper ---------- */
void drawScreen(uint32_t id, int rssi, float snr, uint32_t lost) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("LoRa RX (SF12 BW62k)");
  display.setCursor(0,17);
  display.print("Data: "); display.println(id);
  display.print("RSSI:"); display.print(rssi); display.println("dBm");
  display.print("SNR:"); display.print(snr,1); display.println("dB");
  display.print("Lost "); display.println(lost);
  display.display();
}
