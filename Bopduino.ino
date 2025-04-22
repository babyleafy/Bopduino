#include <Wire.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <MFRC522.h>

//LED pins
#define red 6     // Pin for red LED
#define green 7  // Pin for green LED

//LCD setup
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Sensor setup
#define outputA 8   // Rotary encoder pin A
#define outputB 9  // Rotary encoder pin B
#define SS_PIN SDA // RFID SS pin
#define RST_PIN 14 // RFID RST pin (A0)
MFRC522 mfrc522(SS_PIN, RST_PIN); // RFID instance.
const int playButton = 1;  // Button pin
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

// Enumeration for different successes
enum ACTION {
  PRESS,
  TWIST,
  TILT,
  SCAN,
  FAIL
};

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
  
  // For RFID
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  // Display welcome message for 2 seconds
  lcd.setCursor(0, 0);
  lcd.print("Play BopDuino!");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Generate random prompt
  ACTION prompt = generatePrompt();
  displayAction(prompt);

  // Store initial position of rotary encoder
  LastState = digitalRead(outputA);
  
  // Store initial position
  updateAccelData();
  start_ax = ax; // Store initial x position
  start_ay = ay; // Store initial y position 
  start_az = az; // Store initial z position

  // Set the start time for the timer
  startTime = millis();
  
  // Loop waiting for input
  while (millis() - startTime < timeAlotted) {
    ACTION result = checkAction();
    
    if (result == prompt) {
      color(true);  // Success!
      displayText("Success!");
      delay(500);
      score += 10;

      if (timeAlotted < 1250) {
        timeAlotted -= 50;
      } else {
        timeAlotted -= 150;
      }
    
      return;
    } else if (result == FAIL) {
      continue;
    } else {
      break;
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

ACTION checkAction() {
  // If button was pressed
  if (digitalRead(playButton) == HIGH) {
    return PRESS;
  }

  // If rotary encoder was twisted
  currentState = digitalRead(outputA);
  if (currentState != LastState) {
    return TWIST;
  }

  // If MPU was tilted
  updateAccelData();

  // Calculate the angle between current and starting vectors
  // Using the dot product formula: cos(θ) = (a·b)/(|a|×|b|)
  float dotProduct = ax*start_ax + ay*start_ay + az*start_az;
  float magA = sqrt(ax*ax + ay*ay + az*az);
  float magB = sqrt(start_ax*start_ax + start_ay*start_ay + start_az*start_az);
  
  // Calculate angle between vectors in degrees
  float angle = acos(dotProduct/(magA*magB)) * 180.0 / PI;

  // Success if tilted at least 60 degrees from starting position
  if (angle > 60) {
    hasTilted = true;
    return TILT;
  }

  // // If there was any RFID scan
  //   // Check for RFID card
  // if (mfrc522.PICC_IsNewCardPresent()) {
  //   // Card is present, attempt to read it
  //   if (mfrc522.PICC_ReadCardSerial()) {
  //     mfrc522.PICC_HaltA(); // Stop reading
  //     return SCAN;
  //   }
  // }

  return FAIL;
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
ACTION generatePrompt() {
  // Random number: 0, 1, 2, or 3
  int choice = random(3);
  
  if (choice == 0) {
    return PRESS;
  } else if (choice == 1) {
    return TWIST;
  } else if (choice == 2) {
    return TILT;
  } else {
    return SCAN;
  }
}

void displayAction(ACTION action) {
  switch (action) {
    case PRESS:
      displayText("Press It!");
      break;
    case TWIST:
      displayText("Twist It!");
      break;
    case TILT:
      displayText("Tilt It!");
      break;
    case SCAN:
      displayText("Scan It!");
      break;
    default:
      displayText("Cooked.");
      break;
  }
}

void displayText(String prompt) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(prompt);
}

void endState() {
  displayText("Your score: " + String(score));
  lcd.setCursor(0, 1);
  lcd.print("Press the button to play again!");
  while(digitalRead(playButton) == LOW) {
    delay(50);
  }
  score = 0;
  timeAlotted = 3000;
  displayText("Play Bopduino!");
  delay(2000);
}