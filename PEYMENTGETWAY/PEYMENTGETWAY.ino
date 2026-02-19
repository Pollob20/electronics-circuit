#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>

// HardwareSerial port 1 for SIM900A communication
HardwareSerial mySerial(1);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String incomingSMS = "";
String senderPhone = "";
float receivedAmount = 0.0;

// Function to check if SIM900A is connected and responding
bool checkSIM900A() {
  mySerial.println("AT"); // Send AT command
  Serial.println("Sending AT command to SIM900A...");
  unsigned long startTime = millis();
  while (millis() - startTime < 2000) { // Wait for 2 seconds
    if (mySerial.available()) {
      String response = mySerial.readString();
      Serial.print("SIM900A Response: ");
      Serial.println(response);
      if (response.indexOf("OK") >= 0) {
        return true;
      }
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200); // Start Serial Monitor
  mySerial.begin(9600, SERIAL_8N1, 16, 17); // Start HardwareSerial (RX=16, TX=17)

  // Initialize OLED display
  Wire.begin(5, 4);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!"); // OLED initialization failed
    while (1); // Halt the code if OLED is not working
  }

  // Display initial status
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("System Initializing...");
  display.display();

  delay(2000); // Wait for the SIM900A module to initialize
  
  // Check SIM900A connectivity
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
    while (1); // Halt the system if SIM900A is not found
  }

  // Configure SIM900A
  mySerial.println("AT+CMGF=1"); // Set SMS text mode
  delay(1000);
  mySerial.println("AT+CNMI=2,2,0,0,0"); // Configure to send SMS directly to serial
  delay(2000);

  // Display ready status
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Ready to Start");
  display.display();
  Serial.println("Ready to start");
}

void processPayment(String message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Processing Payment...");
  display.display();
  delay(3000);

  // Extract phone number and amount from the message
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

  String specificPhoneNumber = "+8801521378632";
  if (senderPhone == specificPhoneNumber) {
    Serial.println("Specific phone number received the payment.");
  }

  float productPrice = 50.00;
  if (receivedAmount >= productPrice) {
    Serial.println("Sufficient amount. Dispensing product...");
    // dispenseProduct(); // Uncomment to add dispensing logic
  } else {
    Serial.println("Insufficient amount. No product dispensed.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Insufficient Payment");
    display.display();
  }
}

void loop() {
  while (mySerial.available()) {
    char c = mySerial.read();
    incomingSMS += c;

    if (c == '\n' && incomingSMS.indexOf("You have received") >= 0) {
      Serial.println("Payment SMS detected.");
      Serial.println(incomingSMS);
      processPayment(incomingSMS);
      incomingSMS = "";
    }

    if (incomingSMS.length() > 500) {
      incomingSMS = "";
    }
  }
}
