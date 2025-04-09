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
int score = 0; // Tracking score
int timeAlotted = 3000;

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
  displayText(prompt);

  if (prompt == "Twist It!") {
    LastState = digitalRead(outputA); // reset to avoid false positives
  }
  
  // Set the start time for the 2-second timer
  startTime = millis();
  
  // Loop for 2 seconds waiting for input
  while (millis() - startTime < timeAlotted) {
    
    // Check for button press if prompt is "Press It!"
    if (checkSuccess(prompt)) {
      color(true);  // Success!
      displayText("Success!");
      delay(500);
      score += 10;

      if (timeAlotted < 1000) {
        timeAlotted -= 50;
      } else {
        timeAlotted -= 250;
      }
    
      return;
    }
  }
  
  // If we get here, the 2 seconds elapsed without correct input
  color(false);  // Failure
  displayText("Failed!");
  delay(1000);   // Pause to see the LED
  endState();
}

bool checkSuccess(String prompt) {
  if (prompt == "Press It!") {
    if (digitalRead(playButton) == HIGH) {
      return true;
    }
  } else if (prompt == "Twist It!") { // Check for rotary encoder movement if prompt is "Twist It!"
    currentState = digitalRead(outputA);
    return (currentState != LastState);
  }

  return false;
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

void displayText(String prompt) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(prompt);
}

void endState() {
  displayText("Your score: " + String(score));
  while(digitalRead(playButton) == LOW) {
    delay(50);
  }
  score = 0;
  timeAlotted = 3000;
}