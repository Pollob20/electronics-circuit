#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HardwareSerial.h>

// OLED Display Configurations
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button Definitions
#define BUTTON_PREV_PIN 6
#define BUTTON_OK_PIN 7
#define BUTTON_NEXT_PIN 15

// Relay Pins
const int RELAY_PINS[] = {9, 10, 11, 12, 13, 14};

// RFID Pins
#define SS_PIN 1
#define RST_PIN 40
#define SCK_PIN 2
#define MOSI_PIN 42
#define MISO_PIN 41
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Product Struct
struct Product {
  const char* name;
  float price;
  int stock;
};

Product products[] = {
  {"APPLE", 10.0, 3},
  {"COCACOLA", 20.0, 3},
  {"SPRITE", 30.0, 3},
  {"PRIME", 40.0, 3},
  {"7UP", 50.0, 3},
};

int currentIndex = 0;
bool isCardAuthenticated = false; // Tracks if a card was scanned

unsigned long relayOnTime = 2000; // 2 seconds relay ON time

// Update Display
void updateDisplay() {
  display.clearDisplay();
  Product currentProduct = products[currentIndex];
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 1);
  display.println(currentProduct.name);
  display.print("\nPrice: ");
  display.println(currentProduct.price);
  display.print("\nStock: ");
  display.println(currentProduct.stock);
  display.display();
}

// Navigate to Previous Product
void previousProduct() {
  if (currentIndex > 0) {
    currentIndex--;
    updateDisplay();
  }
}

// Navigate to Next Product
void nextProduct() {
  if (currentIndex < 4) {
    currentIndex++;
    updateDisplay();
  }
}

// Dispense Product
void dispenseProduct() {
  Product& currentProduct = products[currentIndex];
  if (currentProduct.stock > 0) {
    int relayIndex = currentIndex;

    // Activate Relay
    digitalWrite(RELAY_PINS[relayIndex], LOW); // Turn ON Relay
    delay(relayOnTime);                        // Keep it ON for 2 seconds
    digitalWrite(RELAY_PINS[relayIndex], HIGH); // Turn OFF Relay

    currentProduct.stock--; // Reduce Stock

    // Display Dispensed Message
    display.clearDisplay();
    display.setCursor(20, 20);
    display.println("Dispensed!");
    display.display();
    delay(2000);

    updateDisplay(); // Refresh Display
  } else {
    display.clearDisplay();
    display.setCursor(20, 20);
    display.println("Out of Stock!");
    display.display();
    delay(2000);

    updateDisplay();
  }
}

// Handle OK Button
void okButton() {
  if (isCardAuthenticated) {
    dispenseProduct();
    isCardAuthenticated = false; // Reset card authentication
  } else {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Scan Card First!");
    display.display();
    delay(2000);

    updateDisplay();
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  Wire.begin(5, 4);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }

  // Initialize Buttons
  pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);
  pinMode(BUTTON_OK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);

  // Initialize Relays
  for (int i = 0; i < 6; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH); // Ensure relays are OFF
  }

  // Initialize RFID
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();

  updateDisplay();
}

void loop() {
  if (digitalRead(BUTTON_PREV_PIN) == LOW) {
    previousProduct();
    delay(300);
  }
  if (digitalRead(BUTTON_OK_PIN) == LOW) {
    okButton();
    delay(300);
  }
  if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
    nextProduct();
    delay(300);
  }

  // Handle RFID Card Detection
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    isCardAuthenticated = true;
    display.clearDisplay();
    display.setCursor(20, 20);
    display.println("Card Accepted!");
    display.display();
    delay(2000);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    updateDisplay();
  }
}
