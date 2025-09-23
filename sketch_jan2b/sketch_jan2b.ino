#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Wi-Fi credentials
#define WIFI_SSID "No Internet"
#define WIFI_PASSWORD "UiU@2025"

// Firebase configuration
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
FirebaseData firebaseData;

// OLED Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions
#define BUTTON_PREV_PIN  6
#define BUTTON_OK_PIN    7
#define BUTTON_NEXT_PIN  15

const int RELAY_PINS[] = {9, 10, 11, 12, 13, 14};
const int IR_SENSOR_PINS[] = {21, 22, 23, 24, 25, 26};

// RFID configuration
#define SS_PIN 1
#define RST_PIN 40
#define SCK_PIN 2
#define MOSI_PIN 42
#define MISO_PIN 41
MFRC522 mfrc522(SS_PIN, RST_PIN);

struct Product {
  const char* name;
  float price;
  int stock;
};

Product products[] = {
  {"Apple", 10.0, 3},
  {"CocaCola", 20.0, 3},
  {"Sprite", 30.0, 3},
  {"Prime", 40.0, 3},
  {"7up", 50.0, 3},
  {"Drinko", 60.0, 3}
};

int currentIndex = 0;
bool confirmDispense = false;
bool isCardAuthenticated = false;

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  // Firebase configuration
  firebaseConfig.host = "esp32-vanding-maching-default-rtdb.firebaseio.com";
  firebaseConfig.auth_token = "CxP7HFUo77dz6yNGANVBYjuS9ZgMYX3hhYV5PBVw";
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

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
    digitalWrite(RELAY_PINS[i], LOW);
    pinMode(IR_SENSOR_PINS[i], INPUT);
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
    okButton();
    delay(300);
  }

  if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
    nextProduct();
    delay(300);
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    isCardAuthenticated = true;
    Serial.println("Card Detected");
    displayMessage("Payment Received");
    logTransaction("Payment Received");
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
    displayMessage("Scan your card to pay");
    return;
  }

  if (!confirmDispense) {
    Product currentProduct = products[currentIndex];
    if (currentProduct.stock > 0) {
      displayMessage("Press OK to confirm");
      confirmDispense = true;
    } else {
      displayMessage("Out of Stock!");
    }
  } else {
    dispenseProduct();
    confirmDispense = false;
    isCardAuthenticated = false;
  }
}

void dispenseProduct() {
  Product& currentProduct = products[currentIndex];
  currentProduct.stock--;

  // Log transaction to Firebase
  String path = "/transactions/" + String(currentIndex);
  Firebase.RTDB.setString(&firebaseData, path + "/name", currentProduct.name);
  Firebase.RTDB.setFloat(&firebaseData, path + "/price", currentProduct.price);
  Firebase.RTDB.setInt(&firebaseData, path + "/stock", currentProduct.stock);

  displayMessage("Dispensing " + String(currentProduct.name));
  delay(2000);
  updateDisplay();
}

void displayMessage(const String& message) {
  display.clearDisplay();
  display.setCursor(0, 20);
  display.print(message);
  display.display();
}

void logTransaction(const String& message) {
  String path = "/logs/";
  Firebase.RTDB.pushString(&firebaseData, path, message);
}
