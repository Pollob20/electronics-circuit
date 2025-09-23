//by using gsm will call another phone number 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the I2C OLED display address (0x3C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  
  // Initialize the I2C communication for OLED
  Wire.begin(5, 4); // SDA=5, SCK=4 as per your setup
  
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // I2C address 0x3C
    Serial.println("OLED initialization failed!");
    while (1); // Stop if the OLED isn't working
  }

  // Clear the display
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  // Display test message
  display.println("SIM900A Status:");
  display.display();  // Show the text on the screen
}

void loop() {
  // Send the AT command to check network registration status
  Serial.println("AT+CREG?");
  delay(2000);  // Increased delay to allow SIM900A to respond
  
  String response = "";
  while (Serial.available()) {
    response += (char)Serial.read(); // Collect available characters
  }

  // Debugging: print the response to the Serial Monitor
  Serial.print("CREG Response: ");
  Serial.println(response);

  // Check if the network registration status is "0,1" (registered to the network)
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("SIM900A Status:");

  if (response.indexOf("+CREG: 0,1") != -1) {
    // Network is connected
    display.println("Connected to Network");
  } else {
    // Network is not connected
    display.println("Not Connected");
  }

  // Check signal strength with AT+CSQ
  Serial.println("AT+CSQ");
  delay(2000);  // Wait for the response
  String signalResponse = "";
  while (Serial.available()) {
    signalResponse += (char)Serial.read(); // Collect available characters
  }

  // Debugging: print the signal strength response to Serial Monitor
  Serial.print("Signal Response: ");
  Serial.println(signalResponse);

  // Parse signal strength (AT+CSQ returns a value like "+CSQ: 10,99")
  int signalStrength = -1;
  if (signalResponse.indexOf("+CSQ") != -1) {
    signalStrength = signalResponse.substring(signalResponse.indexOf(":") + 1, signalResponse.indexOf(",")).toInt();
  }

  // Display signal strength on OLED
  display.setCursor(0, 20);
  if (signalStrength != -1) {
    display.print("Signal Strength: ");
    display.println(signalStrength);
  } else {
    display.println("Signal Strength: N/A");
  }

  display.display();  // Show the updated status on the screen

  // Make the call to the specific number (01890563272)
  Serial.println("Dialing 01890563272...");
  delay(2000);  // Wait for a moment before calling
  Serial.println("ATD01890563272;");  // AT command to dial the number
  delay(5000);  // Wait for 5 seconds before checking again (or adjust based on your needs)
}
