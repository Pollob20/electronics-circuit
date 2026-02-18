#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>

// OLED Display Configurations
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions for buttons
#define BUTTON_PREV_PIN  6  // Previous button
#define BUTTON_OK_PIN    7  // OK button
#define BUTTON_NEXT_PIN  15 // Next button

// Relay pin definitions
const int RELAY_PINS[] = {31, 20, 21, 47};  // Relay pins for the products

// IR Sensor pin definitions
const int IR_SENSORS[] = {11, 12, 13, 14}; // IR sensor pins for detecting product dispense

// RFID module pins (Updated)
#define SS_PIN 1    
#define RST_PIN 40  
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
 };

int currentIndex = 0;
bool confirmDispense = false;
bool isCardAuthenticated = false;





//UPDATEEEEEEE......// Relay timing variables
unsigned long previousMillis = 0;  // Stores the last time the relay was turned ON
bool relayActive = false;          // Relay state (active or inactive)




// Function to update the display
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




// Function to handle the previous product selection
void previousProduct() {
  if (currentIndex > 0) {
    currentIndex--;
    updateDisplay();
  }
}

// Function to handle the next product selection
void nextProduct() {
  if (currentIndex < 4) {
    currentIndex++;
    updateDisplay();
  }
}



// Function to handle the OK button press
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




// Function to check if the product is dispensed using IR sensor
bool isProductDispensed(int sensorPin) {
  return digitalRead(sensorPin) == LOW; // Assuming LOW indicates product presence
}







// Function to handle the product dispensing and relay control
void dispenseProduct() {
  Product& currentProduct = products[currentIndex];
  int relayIndex = currentIndex;





  //UPDATE NUMBER 222222222 Turn ON the relay
  digitalWrite(RELAY_PINS[relayIndex], LOW);  // Relay ON
  relayActive = true;  // Mark the relay as active





  // Display "Dispensing..." message
  display.clearDisplay();
  display.setCursor(30, 30);
  display.print("Dispensing...");
  display.display();


  // Wait until the product is detected by the IR sensor
  while (relayActive) {
    if (isProductDispensed(IR_SENSORS[relayIndex])) {
      digitalWrite(RELAY_PINS[relayIndex], HIGH);  // Relay OFF
      relayActive = false;  // Stop the relay after detecting the product
    }
  }

  // Reduce stock count
  currentProduct.stock--;

  // Display success message
  display.clearDisplay();
  display.setCursor(37, 20);
  display.print("Dispensed\n\n     Successfully!");
  display.display();

  delay(2000);  // Optional delay before updating the display
  updateDisplay();
}






void setup() {
  Serial.begin(115200);
  // Initialize OLED display
  Wire.begin(5, 4); // SDA = GPIO 5, SCL = GPIO 4
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }
  // Initialize buttons
  pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);
  pinMode(BUTTON_OK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);
  // Initialize relays
  for (int i = 0; i < 6; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH); // Ensure relays are off initially
  }
  // Initialize IR sensors
  for (int i = 0; i < 4; i++) {
    pinMode(IR_SENSORS[i], INPUT);
  }
  // Initialize RFID
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  updateDisplay();
}









void loop() {
  if (digitalRead(BUTTON_PREV_PIN) == LOW) {    // Previous button 6
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
