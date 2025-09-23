//gsm_conected_or_not!!!!!!!
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#include <HardwareSerial.h>
HardwareSerial gsm(1);

void setup() {
  Serial.begin(115200);         // Debug Serial
  gsm.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17 for GSM SIM900A
  
  // Initialize Display
  if (!display.begin(SSD1306_I2C_ADDRESS, 0x3C)) {
    Serial.println("Display initialization failed!");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // Test GSM Module
  gsm.println("AT");
  delay(1000);

  // Check GSM Response
  if (gsm.available()) {
    String response = gsm.readString();
    if (response.indexOf("OK") >= 0) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Connected");
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Disconnected");
    }
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("No GSM Response");
  }
  display.display();
}

void loop() {
  // Additional functionality can be added here
}
