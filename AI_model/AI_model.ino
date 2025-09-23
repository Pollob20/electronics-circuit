#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include "TensorFlowLite_ESP32.h" // TensorFlow Lite for Microcontrollers

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

// RFID module pins
#define SS_PIN 1    // New Slave Select (SDA)
#define RST_PIN 40  // New Reset pin

// Custom SPI Pins
#define SCK_PIN 2
#define MOSI_PIN 42
#define MISO_PIN 41

MFRC522 mfrc522(SS_PIN, RST_PIN); // Initialize RFID with new pins

// AI-related variables
#define AUDIO_BUFFER_SIZE 1600
int8_t audioBuffer[AUDIO_BUFFER_SIZE];
bool keywordDetected = false;

// TensorFlow model and interpreter
const unsigned char model[] = {/* Place your quantized TensorFlow Lite model here */};
tflite::MicroInterpreter* interpreter;
tflite::MicroMutableOpResolver<5> resolver;
uint8_t tensorArena[4096];

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
    dispenserServos[i].attach(SERVO_PINS[i]);
    dispenserServos[i].write(0); // Initial position
    pinMode(IR_SENSOR_PINS[i], INPUT);
  }

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN); // Use new SPI pin configuration
  mfrc522.PCD_Init(); // Initialize RFID
  updateDisplay();

  // Initialize TensorFlow model
  resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED, tflite::ops::micro::Register_FULLY_CONNECTED());
  resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX, tflite::ops::micro::Register_SOFTMAX());
  tflite::MicroErrorReporter micro_error_reporter;
  const tflite::Model* model_data = tflite::GetModel(model);
  static tflite::MicroInterpreter static_interpreter(model_data, resolver, tensorArena, sizeof(tensorArena), &micro_error_reporter);
  interpreter = &static_interpreter;
  interpreter->AllocateTensors();
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
    display.print("Card Authenticated");
    display.display();
    delay(1500);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  // Check for keyword detection
  keywordDetected = detectKeyword();
  if (keywordDetected) {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.print("Keyword Detected!");
    display.display();
    delay(1500);
    okButton(); // Trigger dispense
  }
}

bool detectKeyword() {
  // Simulate audio data input
  for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
    audioBuffer[i] = random(-128, 127); // Replace with real audio sampling logic
  }

  // Copy audio data to TensorFlow Lite input tensor
  int8_t* inputBuffer = interpreter->input(0)->data.int8;
  memcpy(inputBuffer, audioBuffer, AUDIO_BUFFER_SIZE);

  // Run inference
  interpreter->Invoke();

  // Get output tensor
  int8_t* output = interpreter->output(0)->data.int8;

  // Check if keyword is detected
  return output[0] > 100; // Adjust threshold based on your model's output
}

// Other functions (updateDisplay, previousProduct, nextProduct, okButton, dispenseProduct)
// remain the same as provided in the original code.
