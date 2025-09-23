
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
//#define SSD1306_I2C_ADDRESS 0x3C  // Define the I2C address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions for buttons
#define BUTTON_PREV_PIN  6  // Previous button on GPIO 18
#define BUTTON_OK_PIN    7   // OK button on GPIO 3
#define BUTTON_NEXT_PIN  15   // Next button on GPIO 8

// Product details (name, price)
struct Product {
  const char* name;
  float price;
};

Product products[] = {
  {"Product 1", 10.0},
  {"Product 2", 20.0},
  {"Product 3", 30.0},
  {"Product 4", 40.0},
  {"Product 5", 50.0},
  {"Product 6", 60.0}
};

int currentIndex = 0;
float paymentReceived = 0;

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

  // Show the first product
  updateDisplay();
}

void loop() {
  // Read the button states
  if (digitalRead(BUTTON_PREV_PIN) == LOW) {
    //previousProduct();
    delay(300);  // Debounce delay
  }
  
  if (digitalRead(BUTTON_OK_PIN) == LOW) {
    //okButton();
    delay(300);  // Debounce delay
  }

  if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
    //nextProduct();
    delay(300);  // Debounce delay
  }
}

void updateDisplay() {
  // Clear the display
  display.clearDisplay();

  // Get current product
  Product currentProduct = products[currentIndex];

  // Display product name and price
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Product: ");
  display.println(currentProduct.name);
  display.print("Price: $");
  display.println(currentProduct.price);

  // Update the display
  display.display();
}