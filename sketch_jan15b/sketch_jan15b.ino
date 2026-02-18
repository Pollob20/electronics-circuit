#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_PREV_PIN 6
#define BUTTON_OK_PIN 7
#define BUTTON_NEXT_PIN 15

const int RELAY_PINS[] = {9, 10, 11, 12, 13, 14};

HardwareSerial mySerial(1); // Use UART1 for SIM900A communication

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
bool paymentReceived = false;
unsigned long paymentStartTime = 0;
unsigned long paymentWindow = 60000; // 60 seconds

String incomingSMS = "";
float receivedAmount = 0.0;

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

void dispenseProduct() {
    Product& currentProduct = products[currentIndex];
    int relayIndex = currentIndex;

    digitalWrite(RELAY_PINS[relayIndex], LOW);
    Serial.println("Relay ON - Dispensing product");
    delay(2000);
    digitalWrite(RELAY_PINS[relayIndex], HIGH);
    Serial.println("Relay OFF - Dispensing completed");

    currentProduct.stock--;
    display.clearDisplay();
    display.setCursor(37, 20);
    display.print("Dispensed\n\n     Successfully!");
    display.display();
    delay(2000);
    updateDisplay();
}

void parseSMS(const String& sms) {
    int amountIndex = sms.indexOf("Tk ");
    if (amountIndex != -1) {
        int endIndex = sms.indexOf(" ", amountIndex + 3); // Find end of amount
        String amountStr = sms.substring(amountIndex + 3, endIndex);
        receivedAmount = amountStr.toFloat();
        Serial.println("Parsed Amount: " + String(receivedAmount));
    }
}

void checkIncomingSMS() {
    while (mySerial.available()) {
        char c = mySerial.read();
        incomingSMS += c;

        if (incomingSMS.indexOf("+CMT") != -1) {
            // End of SMS detected, process it
            Serial.println("Incoming SMS: " + incomingSMS);
            parseSMS(incomingSMS);

            if (receivedAmount >= products[currentIndex].price) {
                paymentReceived = true;
                incomingSMS = "";

                display.clearDisplay();
                display.setCursor(0, 20);
                display.print("Payment Received!");
                display.display();
                delay(2000);
                dispenseProduct();
                break;
            } else {
                display.clearDisplay();
                display.setCursor(0, 20);
                display.print("Insufficient Amount");
                display.display();
                delay(2000);
                incomingSMS = "";
            }
        }
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
        digitalWrite(RELAY_PINS[i], HIGH); // Set all relays OFF initially
    }

    mySerial.println("AT");
    delay(1000);
    mySerial.println("AT+CMGF=1");
    delay(1000);
    mySerial.println("AT+CNMI=2,2,0,0,0");
    delay(2000);

    updateDisplay();
}

void loop() {
    if (digitalRead(BUTTON_PREV_PIN) == LOW) {
        currentIndex = (currentIndex - 1 + 5) % 5; // Navigate products
        updateDisplay();
        delay(200);
    } else if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
        currentIndex = (currentIndex + 1) % 5;
        updateDisplay();
        delay(200);
    } else if (digitalRead(BUTTON_OK_PIN) == LOW) {
        paymentStartTime = millis();
        paymentReceived = false;

        while (millis() - paymentStartTime < paymentWindow) {
            unsigned long elapsedTime = millis() - paymentStartTime;
            unsigned long remainingTime = (paymentWindow - elapsedTime) / 1000; // Convert to seconds

            display.clearDisplay();
            display.setCursor(0, 20);
            display.print("Waiting for Payment");
            display.setCursor(0, 40);
            display.print("Time left: ");
            display.print(remainingTime);
            display.print(" seconds");
            display.display();

            checkIncomingSMS();
            if (paymentReceived) {
                break;
            }

            delay(200); // Prevent too frequent updates
        }

        if (!paymentReceived) {
            display.clearDisplay();
            display.setCursor(0, 20);
            display.print("Payment Timeout");
            display.display();
            delay(2000);
        }
    }
}
