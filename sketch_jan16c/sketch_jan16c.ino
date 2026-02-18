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
  {"APPLE", 10.0, 3},
  {"COCACOLA", 20.0, 3},
  {"SPRITE", 30.0, 3},
  {"PRIME", 40.0, 3},
  {"7UP", 50.0, 3},
};

int currentIndex = 0;
bool smsMode = false; // Payment mode: false = RFID, true = SMS
bool waitingForPayment = false;
unsigned long relayOnTime = 2000;
unsigned long paymentStartTime = 0;
const unsigned long paymentTimeout = 120000; // 120 seconds in milliseconds

// SMS Payment Variables
String incomingSMS = "";
String senderPhone = "";
float receivedAmount = 0.0;
const char* paymentNumber = "01516548363"; // Display payment number for manual payment

// Function prototypes
void updateDisplay();
void previousProduct();
void nextProduct();
void selectPaymentMethod();
void startPaymentProcess();
void updatePaymentDisplay();
void processPayment(String message);
void dispenseProduct();
void verifyRFIDPayment(String cardUID);

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

void previousProduct() {
  if (currentIndex > 0) {
    currentIndex--;
    updateDisplay();
    // Reset the payment timer if we're going back
    if (waitingForPayment) {
      waitingForPayment = false;
      display.clearDisplay();
      display.setCursor(0, 20);
      display.println("Payment Canceled");
      display.display();
      delay(2000);
      updateDisplay();  // Return to the product display
    }
  }
}

void nextProduct() {
  if (currentIndex < 4) {
    currentIndex++;
    updateDisplay();
  }
}

void selectPaymentMethod() {
  smsMode = !smsMode; // Toggle payment mode
  display.clearDisplay();
  display.setCursor(0, 0);
  if (smsMode) {
    display.println("Manual SMS Mode");
    display.setCursor(0, 20);
    display.println("Pay to:");
    display.setCursor(0, 40);
    display.print(paymentNumber);
  } else {
    display.println("RFID Payment Mode");
    display.setCursor(0, 20);
    display.println("Scan your card");
  }
  display.display();
}

void startPaymentProcess() {
  if (products[currentIndex].stock <= 0) {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Out of Stock!");
    display.display();
    delay(2000);
    return;
  }

  paymentStartTime = millis();
  waitingForPayment = true;

  display.clearDisplay();
  display.setCursor(0, 0);
  if (smsMode) {
    display.println("Awaiting SMS Payment");
    display.setCursor(0, 20);
    display.println("Pay to:");
    display.setCursor(0, 40);
    display.print(paymentNumber);
  } else {
    display.println("Ready for RFID Payment");
    display.setCursor(0, 20);
    display.println("Scan your card");
  }
  display.display();
}

void updatePaymentDisplay() {
  unsigned long elapsedTime = millis() - paymentStartTime;
  unsigned long remainingTime = (paymentTimeout - elapsedTime) / 1000;

  display.clearDisplay();
  display.setCursor(0, 0);
  if (smsMode) {
    display.println("Awaiting SMS Payment");
    display.setCursor(0, 20);
    display.print("Pay to: ");
    display.print(paymentNumber);
  } else {
    display.println("Awaiting RFID Payment");
  }
  display.setCursor(0, 50);
  display.print("Time Left: ");
  display.print(remainingTime);
  display.print(" sec");
  display.display();
}

void dispenseProduct() {
  Product& currentProduct = products[currentIndex];
  int relayIndex = currentIndex;

  digitalWrite(RELAY_PINS[relayIndex], LOW);
  delay(relayOnTime);
  digitalWrite(RELAY_PINS[relayIndex], HIGH);

  currentProduct.stock--;

  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Product Dispensed");
  display.display();
  delay(2000);

  updateDisplay();
}

void verifyRFIDPayment(String cardUID) {
  // Accept any RFID card
  dispenseProduct();
  waitingForPayment = false;

  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Card Accepted!");
  display.display();
  delay(2000);
}

void processPayment(String message) {
  int amountStart = message.indexOf("Tk") + 3;
  int amountEnd = message.indexOf(" ", amountStart);
  String amountStr = message.substring(amountStart, amountEnd);
  receivedAmount = amountStr.toFloat();

  if (receivedAmount >= products[currentIndex].price) {
    dispenseProduct();
    waitingForPayment = false;
  } else {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Insufficient Payment");
    display.display();
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16, 17);

  Wire.begin(5, 4);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1);
  }

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

void loop() {
  if (digitalRead(BUTTON_PREV_PIN) == LOW) {
    previousProduct();
    delay(300);
  }
  if (digitalRead(BUTTON_OK_PIN) == LOW) {
    startPaymentProcess();
    delay(300);
  }
  if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
    selectPaymentMethod();
    delay(300);
  }

  if (waitingForPayment) {
    unsigned long elapsedTime = millis() - paymentStartTime;
    if (elapsedTime >= paymentTimeout) {
      waitingForPayment = false;
      display.clearDisplay();
      display.setCursor(0, 20);
      display.println("Payment Timeout!");
      display.display();
      delay(2000);
      updateDisplay();
    } else {
      updatePaymentDisplay();
    }
  }

  if (!smsMode && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      cardUID += String(mfrc522.uid.uidByte[i], HEX);
    }
    verifyRFIDPayment(cardUID);
    mfrc522.PICC_HaltA();
  }
///// rfid card working proper ly 
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
