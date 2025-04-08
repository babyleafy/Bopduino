#include <LiquidCrystal.h>

//LED pins
#define red 6     // Pin for red LED
#define green 7  // Pin for green LED

//LCD setup
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define outputA 8   // Rotary encoder pin A
#define outputB 9  // Rotary encoder pin B
const int playButton = 13;  // Button pin
int currentState, LastState;
unsigned long startTime;   // For tracking prompt timing
bool rotaryMoved = false;  // Flag to track rotary encoder movement

// Setup
void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  lcd.begin(16, 2);
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(playButton, INPUT);  
  Serial.begin(9600);
  LastState = digitalRead(outputA);
  
  // Display welcome message for 2 seconds
  lcd.setCursor(0, 0);
  lcd.print("Play BopDuino!");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Generate random prompt
  String prompt = generatePrompt();
  
  // Display the prompt
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(prompt);
  
  // Set the start time for the 2-second timer
  startTime = millis();
  rotaryMoved = false;
  
  // Loop for 2 seconds waiting for input
  while (millis() - startTime < 2000) {
    
    // Check for button press if prompt is "Press It!"
    if (prompt == "Press It!") {
      if (digitalRead(playButton) == HIGH) {  // Button is pressed (LOW because of INPUT_PULLUP)
        color(true);  // Success!
        delay(1000);  // Pause to see the LED
        return;       // Exit the function to generate a new prompt
      }
    } else if (prompt == "Twist It!") { // Check for rotary encoder movement if prompt is "Twist It!"
      currentState = digitalRead(outputA);
      
      // If state changed (encoder moved)
      if (currentState != LastState) {
        rotaryMoved = true;
        color(true);  // Success!
        delay(1000);  // Pause to see the LED
        return;       // Exit the function to generate a new prompt
      }
      LastState = currentState;  // Save the current state for next comparison
    }
  }
  
  // If we get here, the 2 seconds elapsed without correct input
  color(false);  // Failure
  delay(1000);   // Pause to see the LED
}

// This function chooses which LED to light up
void color(bool success) {
  if (success) {
    digitalWrite(green, HIGH);  // Turn green LED on
    digitalWrite(red, LOW);     // Make sure red LED is off
  } else {
    digitalWrite(red, HIGH);    // Turn red LED on
    digitalWrite(green, LOW);   // Make sure green LED is off
  }
  
  delay(500);  // Keep LED on for half a second
  
  // Turn both LEDs off
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);
}

// Randomly generate a prompt
String generatePrompt() {
  // Random number: 0 or 1
  int choice = random(2);
  
  if (choice == 0) {
    return "Press It!";
  } else {
    return "Twist It!";
  }
}