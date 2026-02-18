#include <Wire.h> // I2C যোগাযোগের জন্য লাইব্রেরি
#include <Adafruit_GFX.h> // গ্রাফিক্স ডিসপ্লে লাইব্রেরি
#include <Adafruit_SSD1306.h> // OLED ডিসপ্লের জন্য লাইব্রেরি
#include <HardwareSerial.h> // হার্ডওয়্যার সিরিয়াল ব্যবহারের জন্য লাইব্রেরি

// OLED ডিসপ্লের সেটিংস
#define SCREEN_WIDTH 128 // ডিসপ্লের প্রস্থ
#define SCREEN_HEIGHT 64 // ডিসপ্লের উচ্চতা
#define OLED_RESET    -1 // রিসেট পিন ব্যবহার হচ্ছে না
#define OLED_I2C_ADDR 0x3C // OLED-এর I2C ঠিকানা

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED ডিসপ্লে অবজেক্ট

// SIM900A মডিউলের সেটিংস
#define SIM900A_BAUD 9600 // SIM900A মডিউলের বড রেট

HardwareSerial SIM900A(1); // হার্ডওয়্যার Serial1 (ডেডিকেটেড RX/TX পিন ব্যবহার)

// **ESP32 ডেডিকেটেড পিন**
#define SIM900A_RX RX // ESP32 RX0 (GPIO3)
#define SIM900A_TX TX // ESP32 TX0 (GPIO1)

void setup() {
  // ডিবাগ আউটপুটের জন্য সিরিয়াল মনিটর চালু
  Serial.begin(115200);

  // I2C পিন সেটআপ (SDA: 5, SCL: 4)
  Wire.begin(5, 4);

  // OLED ডিসপ্লে ইনিশিয়ালাইজ
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("OLED ডিসপ্লে ইন্সট্যান্স ব্যর্থ হয়েছে!");
    while (1); // ডিসপ্লে কাজ না করলে লুপে থামিয়ে রাখা
  }

  // ডিসপ্লে ইনিশিয়ালাইজ মেসেজ
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();
  delay(2000);

  // SIM900A মডিউলের জন্য Serial1 ইনিশিয়ালাইজ (ডেডিকেটেড RX/TX পিন)
  SIM900A.begin(SIM900A_BAUD, SERIAL_8N1, SIM900A_RX, SIM900A_TX);
  Serial.println("SIM900A Initializing...");
  delay(2000);

  // SIM900A মডিউলের জন্য AT কমান্ড পাঠানো
  SIM900A.println("AT");
  delay(2000);

  if (SIM900A.available()) {
    String response = SIM900A.readString();
    Serial.println("SIM900A Response: " + response);
  } else {
    Serial.println("No response from SIM900A.");
  }
}

// সিগনাল স্ট্রেংথ পরীক্ষা ফাংশন
bool checkSignalStrength() {
  SIM900A.println("AT+CSQ"); // সিগনাল কোয়ালিটি চেক করার AT কমান্ড
  delay(1000);

  if (SIM900A.available()) {
    String response = SIM900A.readString();
    Serial.println("Response: " + response);

    int startIndex = response.indexOf("+CSQ: ");
    if (startIndex != -1) {
      String signalData = response.substring(startIndex + 6, response.indexOf(",", startIndex));
      int signalValue = signalData.toInt();

      if (signalValue == 99) { // সিগনাল পাওয়া যাচ্ছে না
        Serial.println("Signal not detectable.");
        return false;
      } else if (signalValue >= 0 && signalValue <= 31) { // সিগনাল ঠিক আছে
        int dBm = -113 + (signalValue * 2); // dBm ক্যালকুলেট করা
        Serial.print("Signal Strength (RSSI): ");
        Serial.println(signalValue);
        Serial.print("Approx dBm: ");
        Serial.println(dBm);
        return true;
      }
    }
  }

  Serial.println("Failed to retrieve signal strength.");
  return false;
}



void loop() {
  // ডিসপ্লেতে সিগনাল পরীক্ষা বার্তা দেখানো
  display.clearDisplay();
  display.setCursor(0, 0);
    delay(2000);

  display.println("Checking Signal...");
  display.display();
  delay(2000);

  if (checkSignalStrength()) {
    // সিগনাল ঠিক থাকলে দেখান
    display.setCursor(0, 16);
    display.println("SIM900A IS CONECTED TO NETWORK!");
    display.display();
  } else {
    // সিগনাল না থাকলে দেখান
    display.setCursor(0, 16);
    display.println("No Signal!");
    display.display();
  }

  delay(1000); // পুনরায় সিগনাল চেকের আগে অপেক্ষা
}
