#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions for buttons
#define BUTTON_PREV_PIN  6  // Previous button
#define BUTTON_OK_PIN    7  // OK button
#define BUTTON_NEXT_PIN  15 // Next button

// Relay and IR sensor pin definitions
const int RELAY_PINS[] = {9, 10, 11, 12, 13, 14};
const int IR_SENSOR_PINS[] = {21, 22, 23, 24, 25, 26};

// RFID module pins (Updated)
#define SS_PIN 1    //SDA
#define RST_PIN 40  //Reset pin

// Custom SPI Pins
#define SCK_PIN 2
#define MOSI_PIN 42
#define MISO_PIN 41

MFRC522 mfrc522(SS_PIN, RST_PIN); // Initialize RFID with new pins

// Product details
struct Product {
  const char* name;
  float price;
  int stock;
};


Product products[] = {
  {"\n        APPLE", 10.0, 3},
  {"\n       COCACOLA", 20.0, 3},
  {"\n        SPRITE", 30.0, 3},
  {"\n        PRIME", 40.0, 3},
  {"\n         7up", 50.0, 3},
  {"\n        DRINKO", 60.0, 3}
};


int currentIndex = 0;
bool confirmDispense = false;
bool isCardAuthenticated = false;


void setup() {
  Serial.begin(115200);


  Wire.begin(5, 4);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }


  pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);
  pinMode(BUTTON_OK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);


  for (int i = 0; i < 6; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOW); // Ensure relays are initially off
    pinMode(IR_SENSOR_PINS[i], INPUT);
  }


  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN); // Use new SPI pin configuration
  mfrc522.PCD_Init(); // Initialize RFID
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

  // RFID card detection
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    isCardAuthenticated = true;
    Serial.println("Card Detected");
    display.clearDisplay();
    display.setCursor(0, 20);
    display.print("     THANK YOU FOR \n\n      THE PAYMENT.");
    display.display();
    delay(1500);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}

void updateDisplay() {
  display.clearDisplay();

  Product currentProduct = products[currentIndex];

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 1);
  display.println(currentProduct.name);
  display.print("\n\n     Price: ");
  display.println(currentProduct.price);
  display.print("\n\n     Stock: ");
  display.println(currentProduct.stock);

  display.display();
}

void previousProduct() {
  if (currentIndex > 0) {
    currentIndex--;
    updateDisplay();
  }
}

void nextProduct() {
  if (currentIndex < 5) {
    currentIndex++;
    updateDisplay();
  }
}

void okButton() {
  if (!isCardAuthenticated) {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.print("     SCAN YOUR \n\n    CARD  TO PAY!");
    display.display();
    return;
  }

  if (!confirmDispense) {
    Product currentProduct = products[currentIndex];

    if (currentProduct.stock > 0) {
      display.clearDisplay();
      display.setCursor(35, 20);
      display.print("Dispense: \n");
      display.println(currentProduct.name);
      display.setCursor(25, 45);
      display.print("Press OK again");
      display.display();
      confirmDispense = true;
    } else {
      display.clearDisplay();
      display.setCursor(10, 20);
      display.print("Out of Stock!");
      display.display();
      delay(2000);
      updateDisplay();
    }
  } else {
    dispenseProduct();
    confirmDispense = false;
    isCardAuthenticated = false; // Reset card authentication after dispensing
  }
}

void dispenseProduct() {
  Product& currentProduct = products[currentIndex];

  display.clearDisplay();
  display.setCursor(30, 30);
  display.print("Dispensing...");
  display.display();

  int relayIndex = currentIndex;

  // Activate the relay
  digitalWrite(RELAY_PINS[relayIndex], HIGH);





  // Wait for the IR sensor to detect the product
  unsigned long startTime = millis();
  while (digitalRead(IR_SENSOR_PINS[relayIndex]) == HIGH) {
    if (millis() - startTime > 5000) { // Timeout after 5 seconds
      break;
    }
  }





  digitalWrite(RELAY_PINS[relayIndex], LOW); // Turn off the relay

  if (digitalRead(IR_SENSOR_PINS[relayIndex]) == LOW) {
    currentProduct.stock--;
    display.clearDisplay();
    display.setCursor(37, 20);
    display.print("Dispensed\n\n     Successfully!");
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Dispensing Failed!");
  }
  display.display();

  delay(2000);
  updateDisplay();
}

