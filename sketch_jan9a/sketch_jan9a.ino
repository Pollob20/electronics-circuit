#include <SoftwareSerial.h> // সফটওয়্যার সিরিয়াল লাইব্রেরি ইম্পোর্ট
#include <Wire.h> // I2C কমিউনিকেশনের জন্য লাইব্রেরি
#include <Adafruit_GFX.h> // OLED ডিসপ্লের জন্য গ্রাফিক্স লাইব্রেরি
#include <Adafruit_SSD1306.h> // OLED ডিসপ্লের জন্য লাইব্রেরি
//payment get way code
#define SCREEN_WIDTH 128 // OLED ডিসপ্লের প্রস্থ
#define SCREEN_HEIGHT 64 // OLED ডিসপ্লের উচ্চতা

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // OLED ডিসপ্লে অবজেক্ট

// GSM মডিউল এবং ESP32-এর মধ্যে সিরিয়াল কমিউনিকেশন শুরু
SoftwareSerial SIM900(16, 17); // RX = 16, TX = 17 (ESP32 পিন)

// প্রয়োজনীয় ভেরিয়েবল ডিফাইন
String incomingSMS = ""; // SMS সংরক্ষণের জন্য একটি স্ট্রিং
String senderPhone = ""; // সেন্ডারের নম্বর সংরক্ষণ
float receivedAmount = 0.0; // প্রাপ্ত টাকার পরিমাণ

// প্রোডাক্ট ডেলিভারি পিন
const int dispensePin = 2; // প্রোডাক্ট সরবরাহের জন্য একটি পিন

void setup() {
  Serial.begin(115200); // সিরিয়াল মনিটরের জন্য বড রেট সেট করা
  SIM900.begin(9600);   // GSM মডিউলের বড রেট সেট করা

  Wire.begin(5, 4); // SDA = GPIO 5, SCL = GPIO 4
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }

  // ডিসপ্লে পরিষ্কার এবং প্রস্তুত করা
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("System Initializing...");
  display.display();

  // GSM মডিউল ইনিশিয়ালাইজ
  SIM900.println("AT");        // AT কমান্ড পাঠিয়ে মডিউল চেক করা
  delay(1000);                 // ১ সেকেন্ডের ডিলে
  SIM900.println("AT+CMGF=1"); // SMS মোডকে টেক্সটে সেট করা
  delay(1000);                 // ১ সেকেন্ডের ডিলে
  SIM900.println("AT+CNMI=1,2,0,0,0"); // নতুন SMS দেখানোর জন্য সেট করা
  delay(1000);                          // ১ সেকেন্ডের ডিলে

  // প্রোডাক্ট সরবরাহ পিন সেটআপ
  pinMode(dispensePin, OUTPUT); // প্রোডাক্ট সরবরাহ পিনকে আউটপুট হিসেবে সেট করা
  digitalWrite(dispensePin, LOW); // প্রাথমিকভাবে পিন LOW রাখা

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Ready to Start");
  display.display();
  Serial.println("Ready to start"); // সিস্টেম প্রস্তুতির বার্তা
}

// সেন্ডারের নম্বর এবং পরিমাণ বের করা
void processPayment(String message) {
  Serial.println("Payment SMS gotten:"); // পেমেন্ট সংক্রান্ত বার্তা দেখানো
  Serial.println(message); // পুরো SMS প্রিন্ট করা

  // সেন্ডারের নম্বর খুঁজে বের করা
  int phoneStart = message.indexOf("from") + 5;
  int phoneEnd = message.indexOf(".", phoneStart);
  senderPhone = message.substring(phoneStart, phoneEnd);
  senderPhone.trim();

  // প্রাপ্ত টাকার পরিমাণ খুঁজে বের করা
  int amountStart = message.indexOf("Tk") + 3;
  int amountEnd = message.indexOf(" ", amountStart);
  String amountStr = message.substring(amountStart, amountEnd);
  receivedAmount = amountStr.toFloat(); // স্ট্রিংকে ভাসমান সংখ্যা রূপান্তর

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

  // প্রোডাক্টের মূল্য যাচাই করা
  float productPrice = 5.00; // প্রোডাক্টের দাম নির্ধারণ
  if (receivedAmount >= productPrice) { // যদি প্রাপ্ত অর্থ প্রয়োজনীয় দামের চেয়ে বেশি বা সমান হয়
  display.setCursor(0, 0);
    Serial.println("sufficient balance product is dispnacing"); 
    display.display();// সরবরাহ বার্তা
    dispenseProduct(); // প্রোডাক্ট সরবরাহ ফাংশন কল
  } else { // অর্থ পর্যাপ্ত না হলে
    Serial.println("পর্যাপ্ত অর্থ নয়। প্রোডাক্ট সরবরাহ করা হবে না।"); // অর্থ যথেষ্ট না হলে বার্তা
  }
}


// প্রোডাক্ট সরবরাহ করা
void dispenseProduct() {
  digitalWrite(dispensePin, HIGH); // প্রোডাক্ট সরবরাহ পিন সক্রিয় করা
  delay(5000);                     // ৫ সেকেন্ড ধরে সরবরাহ চালু রাখা
  digitalWrite(dispensePin, LOW);  // প্রোডাক্ট সরবরাহ বন্ধ করা
}


void loop() {
  // GSM মডিউল থেকে ডেটা পড়া
  if (SIM900.available()) {
    char c = SIM900.read(); // নতুন SMS পড়া
    incomingSMS += c;

    // SMS-এর শেষে +CMT: খুঁজে পেলে
    if (c == '\n' && incomingSMS.indexOf("+CMT:") >= 0) {
      incomingSMS = ""; // SMS বাফার পরিষ্কার
    }

    // যদি SMS-এ 'You have received' লেখা থাকে
    if (c == '\n' && incomingSMS.indexOf("You have received") >= 0) {
      processPayment(incomingSMS); // পেমেন্ট প্রক্রিয়া
      incomingSMS = ""; // SMS বাফার পরিষ্কার
    }
  }
}
