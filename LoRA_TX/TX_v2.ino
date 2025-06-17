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

// config button BOOT for mode switching
#define BOOT_BUTTON 0

// enum for transmission modes
enum TransmissionMode {
  AUTO_MODE,    // Atuo message sending 
  MANUAL_MODE   // manual message sending with Serial input
};

TransmissionMode currentMode = AUTO_MODE;
bool lastBootState = HIGH;
unsigned long lastModeCheck = 0;
unsigned long lastAutoSend = 0;
const unsigned long AUTO_SEND_INTERVAL = 1000; // Gửi tự động mỗi 1 giây

// Sample auto messages
String autoMessages[] = {
  "Hello LoRa!",
  "LoRa 10km!!",
  "Nghiemvida",
};
int autoMessageIndex = 0;
int autoMessageCount = 0;

// Manual message variables
String inputMessage = "";
int manualMessageCount = 0;

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);
  
  // BOOT button setup
  pinMode(BOOT_BUTTON, INPUT_PULLUP);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  delay(2000);
  initializeLoRa();
  
  // Check initial mode based on Serial input or BOOT button
  detectInitialMode();
  displayModeInfo();
  
  Serial.println("=== LoRa Dual Mode Transmitter ===");
  Serial.println("BOOT Button: Switch between AUTO/MANUAL mode");
  Serial.println("Serial Monitor: Auto-switch to MANUAL mode");
  Serial.println("=====================================");
}

void loop() {
  // Check for mode switching via BOOT button or Serial input
  checkModeSwitch();
  
  if (currentMode == AUTO_MODE) {
    handleAutoMode();
  } else {
    handleManualMode();
  }
  
  delay(100);
}

void initializeLoRa() {
  LoRa.setPins(ss, rst, dio0);
  
  while (!LoRa.begin(433E6)) {
    Serial.println("Initializing LoRa...");
    delay(500);
  }
  
  // Configuration for long range transmission
  LoRa.setSpreadingFactor(12);      
  LoRa.setSignalBandwidth(62.5E3);  
  LoRa.setCodingRate4(5);           
  LoRa.setPreambleLength(12);       
  LoRa.setSyncWord(0xA5);           
  LoRa.enableCrc();                 
  LoRa.setTxPower(20);              
  Serial.println("LoRa Initialized - Long Range Configuration!");
}

void detectInitialMode() {
  // Check if Serial Monitor is available
  Serial.println("Detecting connection mode...");
  delay(1000);
  
  if (Serial.available() > 0) {
    currentMode = MANUAL_MODE;
    Serial.println("Serial Monitor detected - Starting in MANUAL mode");
  } else {
    currentMode = AUTO_MODE;
    Serial.println("No Serial input - Starting in AUTO mode");
  }
}

void checkModeSwitch() {
  unsigned long currentTime = millis();
  
  // Check BOOT button state every 100ms
  if (currentTime - lastModeCheck > 100) {
    lastModeCheck = currentTime;
    
    bool currentBootState = digitalRead(BOOT_BUTTON);
    
    // Detect button press (LOW state)
    if (lastBootState == HIGH && currentBootState == LOW) {
      switchMode();
      delay(300); // Debounce
    }
    lastBootState = currentBootState;
  }
  
  // Check Serial Monitor (swich to MANUAL mode if input is detected)
  if (currentMode == AUTO_MODE && Serial.available() > 0) {
    currentMode = MANUAL_MODE;
    Serial.println("\n=== AUTO SWITCHED TO MANUAL MODE ===");
    Serial.println("Serial input detected!");
    displayModeInfo();
  }
}

void switchMode() {
  if (currentMode == AUTO_MODE) {
    currentMode = MANUAL_MODE;
    Serial.println("\n=== SWITCHED TO MANUAL MODE ===");
    Serial.println("Type messages to send via LoRa");
  } else {
    currentMode = AUTO_MODE;
    Serial.println("\n=== SWITCHED TO AUTO MODE ===");
    Serial.println("Sending predefined messages automatically");
    lastAutoSend = millis(); // Reset timer
  }
  displayModeInfo();
}

void handleAutoMode() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastAutoSend >= AUTO_SEND_INTERVAL) {
    sendAutoMessage();
    lastAutoSend = currentTime;
  }
}

void handleManualMode() {
  if (Serial.available()) {
    inputMessage = Serial.readStringUntil('\n');
    inputMessage.trim();
    
    if (inputMessage.length() > 0) {
      if (inputMessage == "auto") {
        switchMode();
      } else if (inputMessage == "status") {
        showStatus();
      } else {
        sendManualMessage(inputMessage);
      }
    }
  }
}

void sendAutoMessage() {
  String message = autoMessages[autoMessageIndex];
  autoMessageCount++;
  
  // Format message with count
  // Example: "1-Hello LoRa!"
  String sendMessage = String(autoMessageCount) + "-" + message;
  
  Serial.println("=== AUTO SENDING ===");
  Serial.print("Sent packet: ");
  Serial.println(sendMessage);
  
  LoRa.beginPacket();
  LoRa.print(sendMessage);
  LoRa.endPacket();
  
  // Update auto message index for next send
  // Wrap around if it exceeds the number of messages
  autoMessageIndex = (autoMessageIndex + 1) % (sizeof(autoMessages) / sizeof(autoMessages[0]));
  
  updateDisplay("AUTO", sendMessage, autoMessageCount);
  Serial.println("Sent successfully!");
  Serial.println("===================");
}

void sendManualMessage(String message) {
  manualMessageCount++;
  
  Serial.println("=== MANUAL SENDING ===");
  Serial.print("Message: ");
  Serial.println(message);
  
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  
  updateDisplay("MANUAL", message, manualMessageCount);
  Serial.println("Sent successfully!");
  Serial.println("==================");
}

void updateDisplay(String mode, String message, int count) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("LoRa TX-");
  display.println(mode);
  display.setCursor(0, 17);
  display.println(message.substring(0, 30));
  display.setCursor(0, 27);
  display.print("Count:");
  display.println(count);
  display.setCursor(0, 37);
  display.println("BOOT: Switch Mode");
  display.display();
}

void displayModeInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("LoRa Dual Mode TX");
  display.setCursor(0, 16);
  if (currentMode == AUTO_MODE) {
    display.println("Mode: AUTO");
    display.setCursor(0, 28);
    display.println("Sending every 1s");
  } else {
    display.println("Mode: MANUAL");
    display.setCursor(0, 28);
    display.println("Type to send");
  }
  display.setCursor(0, 40);
  display.println("BOOT: Switch Mode");
  display.setCursor(0, 52);
  display.println("Serial: Auto Manual");
  display.display();
}

void showStatus() {
  Serial.println("=== SYSTEM STATUS ===");
  Serial.print("Current Mode: ");
  Serial.println(currentMode == AUTO_MODE ? "AUTO" : "MANUAL");
  Serial.print("Auto Messages Sent: ");
  Serial.println(autoMessageCount);
  Serial.print("Manual Messages Sent: ");
  Serial.println(manualMessageCount);
  Serial.println("Commands: 'auto' - switch to auto mode");
  Serial.println("         'status' - show this info");
  Serial.println("====================");
}
