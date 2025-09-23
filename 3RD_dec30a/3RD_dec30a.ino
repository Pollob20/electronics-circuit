#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions for buttons
#define BUTTON_PREV_PIN  6  // Previous button
#define BUTTON_OK_PIN    7  // OK button
#define BUTTON_NEXT_PIN  15  // Next button

// Servo and IR sensor pin definitions
#define SERVO_PIN 9
#define IR_SENSOR_PIN 10

Servo dispenserServo;

// Product details (name, price)
struct Product {
  const char* name;
  float price;
};

Product products[] = {
  {"APPLE", 10.0},
  {"COCACOLA", 20.0},
  {"SPRITE", 30.0},
  {"PRIME", 40.0},
  {"7up", 50.0},
  {"DRINKO", 60.0}
};

int currentIndex = 0;
bool confirmDispense = false;

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);

  // Initialize I2C display
  Wire.begin(5, 4);  // SDA=21, SCK=20
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // I2C address 0x3C
    Serial.println("OLED initialization failed!");
    while (1); // Stop if the OLED isn't working
  }
  
  // Set up button pins as input with pull-up resistors
  pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);
  pinMode(BUTTON_OK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);

  // Set up servo and IR sensor
  dispenserServo.attach(SERVO_PIN);
  dispenserServo.write(0); // Ensure the servo is in the initial position
  pinMode(IR_SENSOR_PIN, INPUT);

  // Show the first product
  updateDisplay();
}

void loop() {
  // Read the button states
  if (digitalRead(BUTTON_PREV_PIN) == LOW) {
    previousProduct();
    delay(300);  // Debounce delay
  }
  
  if (digitalRead(BUTTON_OK_PIN) == LOW) {
    okButton();
    delay(300);  // Debounce delay
  }

  if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
    nextProduct();
    delay(300);  // Debounce delay
  }
}

void updateDisplay() {
  display.clearDisplay();

  // Get current product
  Product currentProduct = products[currentIndex];

  // Display product name and price
  display.setTextSize(1.99);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(currentProduct.name);
  display.print("\nPrice:");
  display.println(currentProduct.price);

  // Update the display
  display.display();
}

void previousProduct() {
  if (currentIndex > 0) {
    currentIndex--;
    updateDisplay();
  }
}

void nextProduct() {
  if (currentIndex < sizeof(products) / sizeof(products[0]) - 1) {
    currentIndex++;
    updateDisplay();
  }
}

void okButton() {
  if (!confirmDispense) {
    // Get the current product
    Product currentProduct = products[currentIndex];

    // Display confirmation message
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setCursor(0, 20);
    display.print("Dispense: ");
    display.println(currentProduct.name);
    display.setCursor(0, 40);
    display.print("Press OK again");
    display.display();

    confirmDispense = true;
  } else {
    // Dispense the product
    dispenseProduct();
    confirmDispense = false;
  }
}







void dispenseProduct() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Dispensing...");
  display.display();

  // Rotate servo to dispense (360 degrees twice)
  for (int i = 0; i < 2; i++) {
    dispenserServo.write(180); // Rotate to dispensing position
    delay(1000); // Wait for 1 second
    dispenserServo.write(0); // Rotate back
    delay(1000); // Wait for 1 second
  }

  // Verify product dispensed using IR sensor
  if (digitalRead(IR_SENSOR_PIN) == LOW) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Dispensed Successfully!");
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Dispensing Failed!");
  }
  display.display();

  delay(2000); // Show message for 2 seconds
}
