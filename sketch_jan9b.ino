// এই লাইনগুলোতে GSM মডিউল এবং ESP32-এর মধ্যে সিরিয়াল কমিউনিকেশন শুরু হয়
SoftwareSerial SIM900(16, 17); // RX, TX পিন ডিফাইন
String incomingSMS = ""; // SMS সংরক্ষণের জন্য একটি স্ট্রিং
String senderPhone = ""; // সেন্ডারের নম্বর সংরক্ষণ

void setup() {
  Serial.begin(115200);
  SIM900.begin(9600); // SIM900-এর বড রেট সেট করা

  // GSM মডিউল ইনিশিয়ালাইজ
  SIM900.println("AT");
  delay(1000);
  SIM900.println("AT+CMGF=1"); // SMS মোড টেক্সটে সেট করা
  delay(1000);
  SIM900.println("AT+CNMI=1,2,0,0,0"); // নতুন SMS দেখানো
  delay(1000);

  Serial.println("সিস্টেম প্রস্তুত!");
}



// SMS প্রক্রিয়া করা
void loop() {
  if (SIM900.available()) { 
    char c = SIM900.read(); // নতুন SMS পড়া
    incomingSMS += c;

    // SMS এর শেষে +CMT: খুঁজে পেলে
    if (c == '\n' && incomingSMS.indexOf("+CMT:") >= 0) {
      processSender(); // সেন্ডারের তথ্য বের করা
    }



    // যদি SMS-এ 'bKash' লেখা থাকে
    if (c == '\n' && incomingSMS.indexOf("bKash") >= 0) {
      if (validateSender(senderPhone)) { // সেন্ডার যাচাই করা
        processPayment(incomingSMS); // পেমেন্ট প্রক্রিয়া
      } else {
        Serial.println("অবৈধ সেন্ডার। SMS উপেক্ষা করা হচ্ছে।");
      }
      incomingSMS = ""; // SMS বাফার পরিষ্কার
      senderPhone = ""; // সেন্ডার বাফার পরিষ্কার
    }
  }
}




// সেন্ডারের নম্বর বের করা
void processSender() {
  int startIdx = incomingSMS.indexOf("+CMT:") + 6; 
  int endIdx = incomingSMS.indexOf(",", startIdx);
  senderPhone = incomingSMS.substring(startIdx, endIdx);
  senderPhone.trim(); 
  Serial.print("সেন্ডার: ");
  Serial.println(senderPhone);
}

// সেন্ডার যাচাই করা
bool validateSender(String sender) {
  return sender.equalsIgnoreCase("bKash") || sender == "+8801624747474";
}

// SMS থেকে টাকার পরিমাণ বের করা
void processPayment(String message) {
  Serial.println("পেমেন্ট SMS পাওয়া গেছে:");
  Serial.println(message);

  // পরিমাণ খুঁজে বের করা
  int amountIndex = message.indexOf("BDT");
  if (amountIndex >= 0) {
    String amountStr = message.substring(amountIndex + 4); 
    int endIndex = amountStr.indexOf(' '); 
    amountStr = amountStr.substring(0, endIndex);
    float amount = amountStr.toFloat(); // স্ট্রিং থেকে সংখ্যা রূপান্তর

    Serial.print("প্রাপ্ত অর্থ: BDT ");
    Serial.println(amount);

    // প্রোডাক্টের মূল্য যাচাই করা
    float productPrice = 50.00; 
    if (amount >= productPrice) {
      Serial.println("পর্যাপ্ত অর্থ। প্রোডাক্ট সরবরাহ করা হচ্ছে...");
      dispenseProduct(); // প্রোডাক্ট সরবরাহ ফাংশন কল
    } else {
      Serial.println("পর্যাপ্ত অর্থ নয়। প্রোডাক্ট সরবরাহ করা হবে না।");
    }
  } else {
    Serial.println("পরিমাণ বের করা যায়নি। SMS উপেক্ষা করা হচ্ছে।");
  }
}

// প্রোডাক্ট সরবরাহ করা
void dispenseProduct() {
  digitalWrite(2, HIGH); 
  delay(5000); // ৫ সেকেন্ড ধরে প্রোডাক্ট সরবরাহ
  digitalWrite(2, LOW);
}
