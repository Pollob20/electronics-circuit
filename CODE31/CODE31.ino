//WRONG CODE 31
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
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
const int RELAY_PINS[] = {9, 10, 11, 12}; // 4 relays for 4 products

// IR Sensor pins
const int IR_SENSOR_PINS[] = {47, 48, 20, 21}; // IR sensors for detecting product dispensing

// HardwareSerial for SIM900A
HardwareSerial mySerial(1);

// Authorized sender phone number (the specific number to accept messages from)
const char* authorizedSender = "O1521378632"; // Replace with actual sender phone number

// Products
struct Product {
  const char* name;
  float price;
  int stock;
};

Product products[] = {
  {"\n        NAPA", 10.0, 3},
  {"\n       AZIMEX", 20.0, 3},
  {"\n        PERACITAMAL", 30.0, 3},
  {"\n        FLYMID 500", 40.0, 3},
};

int currentIndex = 0;
bool confirmDispense = false;
unsigned long relayOnTime = 7000;  

// SMS Payment Variables
String incomingSMS = "";
String senderPhone = "";
float receivedAmount = 0.0;
const char* paymentNumber = "O1521378632"; // Display payment number for manual payment

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
  if (currentIndex < 3) {
    currentIndex++;
    updateDisplay();
  }
}

void dispenseProduct() {
  Product& currentProduct = products[currentIndex];
  int relayIndex = currentIndex;

  if (currentProduct.stock <= 0) {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Out of Stock!");
    display.display();
    delay(2000);
    updateDisplay();
    return;
  }

  // Activate the relay (Turn on motor)
  digitalWrite(RELAY_PINS[relayIndex], LOW); // Relay ON
  Serial.println("Relay ON - Motor running");

  // Continuously check the IR sensor to stop the motor immediately when product is detected
  unsigned long startTime = millis(); // Add a timeout to avoid infinite loop
  while (millis() - startTime < relayOnTime) {
    if (digitalRead(IR_SENSOR_PINS[relayIndex]) == LOW) { // IR sensor detects product
      Serial.println("Product detected by IR sensor.");
      break; // Exit loop as product has been dispensed
    }
    delay(50); // Small delay to prevent overloading the CPU
  }

  // Deactivate the relay (Turn off motor)
  digitalWrite(RELAY_PINS[relayIndex], HIGH); // Relay OFF
  Serial.println("Relay OFF - Motor stopped");

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

  // Extract sender phone number from SMS
  int phoneStart = message.indexOf("from") + 5;
  int phoneEnd = message.indexOf(".", phoneStart);
  senderPhone = message.substring(phoneStart, phoneEnd);
  senderPhone.trim();

  // Check if the sender matches the authorized phone number
  if (senderPhone != authorizedSender) {
    Serial.println("-----------------.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Unauthorized Sender!");
    display.display();
    delay(2000);
    return; // Exit the function if the sender is not authorized
  }

  // Extract payment amount from SMS
  int amountStart = message.indexOf("Tk") + 3;
  int amountEnd = message.indexOf(" ", amountStart);
  String amountStr = message.substring(amountStart, amountEnd);
  receivedAmount = amountStr.toFloat();

  Serial.print("Authorized Sender: ");
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

  // Verify payment amount and dispense product
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

  for (int i = 0; i < 4; i++) {  // 4 relays for 4 products
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH);
    pinMode(IR_SENSOR_PINS[i], INPUT);  // IR sensor for detecting dispensing
  }

  updateDisplay();
}

void loop() {
  if (digitalRead(BUTTON_PREV_PIN) == LOW) {
    previousProduct();
    delay(300);
  }
  if (digitalRead(BUTTON_OK_PIN) == LOW) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("    ");
    display.setCursor(0, 20);
    display.print("OR PAY TO:");
    display.setCursor(0, 40);
    display.print(paymentNumber);
    display.display();
    delay(300);
  }
  if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
    nextProduct();
    delay(300);
  }

  while (mySerial.available()) {
    char c = mySerial.read();
    incomingSMS += c;
    if (c == '\n' && incomingSMS.indexOf("You have received") >= 0) {
      processPayment(incomingSMS);
      incomingSMS = ""; // Clear the buffer after processing
    }
    if (incomingSMS.length() > 500) {
      incomingSMS = ""; // Prevent buffer overflow
    }
  }
}
