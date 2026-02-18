#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HardwareSerial.h>

// OLED Display Configurations
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Buttons
#define BUTTON_PREV_PIN  6  // Previous button
#define BUTTON_OK_PIN    7  // OK button
#define BUTTON_NEXT_PIN  15 // Next button

// Relay pins
const int RELAY_PINS[] = {9, 10, 11, 12, 13, 14};

// RFID pins
#define SS_PIN 1    
#define RST_PIN 40  
#define SCK_PIN 2
#define MOSI_PIN 42
#define MISO_PIN 41
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// HardwareSerial for SIM900A
HardwareSerial mySerial(1);

// Products
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
};

int currentIndex = 0;
bool confirmDispense = false;
bool isCardAuthenticated = false;
unsigned long relayOnTime = 2000;  

// SMS Payment Variables
String incomingSMS = "";
String senderPhone = "";
float receivedAmount = 0.0;
const char* paymentNumber = "01516548363"; // Display payment number for manual payment

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
  if (currentIndex < 4) {
    currentIndex++;
    updateDisplay();
  }
}

void dispenseProduct() {
  Product& currentProduct = products[currentIndex];
  int relayIndex = currentIndex;

  // Activate the relay
  digitalWrite(RELAY_PINS[relayIndex], LOW);
  Serial.println("Relay ON");
  delay(relayOnTime); // Keep relay ON for the specified time

  // Deactivate the relay
  digitalWrite(RELAY_PINS[relayIndex], HIGH);
  Serial.println("Relay OFF");

  // Update product stock
  currentProduct.stock--;

  // Display success message
  display.clearDisplay();
  display.setCursor(37, 20);
  display.print("Dispensed\n\n     Successfully!");
  display.display();
  delay(2000);

  // Update the main display with the product list
  updateDisplay();
}

void processPayment(String message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Processing Payment...");
  display.display();
  delay(3000);

  int phoneStart = message.indexOf("from") + 5;
  int phoneEnd = message.indexOf(".", phoneStart);
  senderPhone = message.substring(phoneStart, phoneEnd);
  senderPhone.trim();

  int amountStart = message.indexOf("Tk") + 3;
  int amountEnd = message.indexOf(" ", amountStart);
  String amountStr = message.substring(amountStart, amountEnd);
  receivedAmount = amountStr.toFloat();

  Serial.print("Sender: ");
  Serial.println(senderPhone);
  Serial.print("Amount: BDT ");
  Serial.println(receivedAmount);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Payment Received");
  display.print("Sender: ");
  display.println(senderPhone);
  display.print("Amount: BDT ");
  display.println(receivedAmount);
  display.display();
  delay(3000);

  if (receivedAmount >= products[currentIndex].price) {
    Serial.println("Sufficient amount. Dispensing product...");
    dispenseProduct();
  } else {
    Serial.println("Insufficient amount. No product dispensed.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Insufficient Payment");
    display.display();
  }
}
// RX (GPIO16), TX (GPIO17)
void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  
  Wire.begin(5, 4);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("System Initializing...");
  display.display();

  delay(2000);
  if (checkSIM900A()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SIM900A Connected!");
    display.display();
    Serial.println("SIM900A is ready.");
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SIM900A Not Found!");
    display.display();
    Serial.println("SIM900A not responding.");
    while (1);
  }

  mySerial.println("AT+CMGF=1");
  delay(1000);
  mySerial.println("AT+CNMI=2,2,0,0,0");
  delay(2000);

  pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);
  pinMode(BUTTON_OK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);

  for (int i = 0; i < 6; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH);
  }

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  updateDisplay();
}

bool checkSIM900A() {
  mySerial.println("AT");
  unsigned long startTime = millis();
  while (millis() - startTime < 2000) {
    if (mySerial.available()) {
      String response = mySerial.readString();
      if (response.indexOf("OK") >= 0) {
        return true;
      }
    }
  }
  return false;
}

void loop() {
  if (digitalRead(BUTTON_PREV_PIN) == LOW) {
    previousProduct();
    delay(300);
  }
  if (digitalRead(BUTTON_OK_PIN) == LOW) {
    if (!isCardAuthenticated) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("SCAN CARD TO PAY");
      display.setCursor(0, 20);
      display.print("OR PAY TO:");
      display.setCursor(0, 40);
      display.print(paymentNumber);
      display.display();
    } else {
      dispenseProduct();
      isCardAuthenticated = false;
    }
    delay(300);
  }
  if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
    nextProduct();
    delay(300);
  }
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    isCardAuthenticated = true;
    display.clearDisplay();
    display.setCursor(0, 20);
    display.print("     THANK YOU FOR \n\n      THE PAYMENT.");
    display.display();
    delay(1500);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  while (mySerial.available()) {
    char c = mySerial.read();
    incomingSMS += c;
    if (c == '\n' && incomingSMS.indexOf("You have received") >= 0) {
      processPayment(incomingSMS);
      incomingSMS = "";
    }
    if (incomingSMS.length() > 500) {
      incomingSMS = "";
    }
  }
}
