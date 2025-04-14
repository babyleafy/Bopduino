#include <Wire.h>
#include <LiquidCrystal.h>

//LED pins
#define red 6     // Pin for red LED
#define green 7  // Pin for green LED

//LCD setup
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// Sensor setup
#define outputA 8   // Rotary encoder pin A
#define outputB 9  // Rotary encoder pin B
const int playButton = 13;  // Button pin
const int MPU_addr = 0x68; 
int currentState, LastState;
unsigned long startTime;   // For tracking prompt timing
int score = 0; // Tracking score
int timeAlotted = 3000;

// Variables for gyroscope readings
int16_t accel_x, accel_y, accel_z;      // Tilt detection
float ax, ay, az;                        // Tilt calculations
float start_ax, start_ay, start_az;      // Store initial position values
bool hasTilted = false;                  // Track if device has been tilted

// Setup
void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  lcd.begin(16, 2);
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(playButton, INPUT);  
  LastState = digitalRead(outputA);

  // For MPU
  Wire.begin();
  Serial.begin(115200);
  // Wake up the MPU6500 by writing 0 to the power management register (0x6B)
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  // Optionally, you can configure the gyro full-scale range here by writing to register 0x1B.
  // For ±250 deg/s, the default value (0) is fine.
  Serial.println("MPU6500 Initialized.");
  
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
  
  // If prompt is "Tilt It!", store initial position
  if (prompt == "Tilt It!") {
    // Read and store current accelerometer position as reference point
    updateAccelData();
    start_ax = ax; // Store initial x position
    start_ay = ay; // Store initial y position 
    start_az = az; // Store initial z position
    hasTilted = false; // Reset tilt state
  }
  
  // Set the start time for the timer
  startTime = millis();
  
  // Loop waiting for input
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
  
  // If we get here, the time elapsed without correct input
  color(false);  // Failure
  displayText("Failed!");
  delay(1000);   // Pause to see the LED
  endState();
}

// Update accelerometer data from MPU
void updateAccelData() {
  // Request 6 bytes from the MPU starting at register 0x3B (Accel X high byte)
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true);  // Request 6 bytes
  
  // Read high and low bytes and combine them for each axis
  accel_x = Wire.read() << 8 | Wire.read();
  accel_y = Wire.read() << 8 | Wire.read();
  accel_z = Wire.read() << 8 | Wire.read();
  
  // Convert raw values to g forces (using 16384 LSB/g for ±2g range)
  ax = accel_x / 16384.0;
  ay = accel_y / 16384.0;
  az = accel_z / 16384.0;
}

bool checkSuccess(String prompt) {
  if (prompt == "Press It!") {
    if (digitalRead(playButton) == HIGH) {
      return true;
    }
  } else if (prompt == "Twist It!") { // Check for rotary encoder movement if prompt is "Twist It!"
    currentState = digitalRead(outputA);
    return (currentState != LastState);
  } else if (prompt == "Tilt It!") { // Check for tilt if prompt is "Tilt It!"
    updateAccelData();
    
    // Calculate the angle between current and starting vectors
    // Using the dot product formula: cos(θ) = (a·b)/(|a|×|b|)
    float dotProduct = ax*start_ax + ay*start_ay + az*start_az;
    float magA = sqrt(ax*ax + ay*ay + az*az);
    float magB = sqrt(start_ax*start_ax + start_ay*start_ay + start_az*start_az);
    
    // Calculate angle between vectors in degrees
    float angle = acos(dotProduct/(magA*magB)) * 180.0 / PI;
    
    // Success if tilted at least 60 degrees from starting position
    if (angle > 60 && !hasTilted) {
      hasTilted = true;
      return true;
    }
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
  // Random number: 0, 1, or 2
  int choice = random(3); // Modified to include third option
  
  if (choice == 0) {
    return "Press It!";
  } else if (choice == 1) {
    return "Twist It!";
  } else {
    return "Tilt It!";
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