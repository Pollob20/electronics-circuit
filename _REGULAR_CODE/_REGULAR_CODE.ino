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
#define BUTTON_NEXT_PIN  15 // Next button

// Servo and IR sensor pin definitions
const int SERVO_PINS[] = {9, 10, 11, 12, 13, 14};
const int IR_SENSOR_PINS[] = {21, 22, 23, 24, 25, 26};

Servo dispenserServos[6];

// Product details
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
  {"7up", 50.0, 3},
  {"DRINKO", 60.0, 3}
};

int currentIndex = 0;
bool confirmDispense = false;

void setup() {
  Serial.begin(115200);

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }

  pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);
  pinMode(BUTTON_OK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);

  for (int i = 0; i < 6; i++) {
    dispenserServos[i].attach(SERVO_PINS[i]);
    dispenserServos[i].write(0); // Initial position
    pinMode(IR_SENSOR_PINS[i], INPUT);
  }

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
}

void updateDisplay() {
  display.clearDisplay();

  Product currentProduct = products[currentIndex];

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(currentProduct.name);
  display.print("Price: ");
  display.println(currentProduct.price);
  display.print("Stock: ");
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
  if (!confirmDispense) {
    Product currentProduct = products[currentIndex];

    if (currentProduct.stock > 0) {
      display.clearDisplay();
      display.setCursor(0, 20);
      display.print("Dispense: ");
      display.println(currentProduct.name);
      display.setCursor(0, 40);
      display.print("Press OK again");
      display.display();
      confirmDispense = true;
    } else {
      display.clearDisplay();
      display.setCursor(0, 20);
      display.print("Out of Stock!");
      display.display();
      delay(2000);
      updateDisplay();
    }
  } else {
    dispenseProduct();
    confirmDispense = false;
  }
}

void dispenseProduct() {
  Product& currentProduct = products[currentIndex];

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Dispensing...");
  display.display();

  int servoIndex = currentIndex;
  dispenserServos[servoIndex].write(180);
  delay(1000);
  dispenserServos[servoIndex].write(0);
  delay(1000);

  if (digitalRead(IR_SENSOR_PINS[servoIndex]) == LOW) {
    currentProduct.stock--;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Dispensed Successfully!");
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Dispensing Failed!");
  }
  display.display();

  delay(2000);
  updateDisplay();
}
