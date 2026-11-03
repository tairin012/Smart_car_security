 #include <MemoryFree.h>
 #include <EEPROM.h>
 #include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "smart car"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

#define BLYNK_PRINT Serial


#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <LiquidCrystal_I2C.h>

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

 
#define EspSerial Serial3
#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

BlynkTimer timer;

// Define LCD parameters
#define LCD_ADDRESS 0x27
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

// Define pin for limit switch
const int limitSwitchPin = 9;
const int trigPin = 46;
const int echoPin = 47;

// Define Blynk virtual pin for gas door status
#define GAS_DOOR_VPIN V4

#define IN1_PIN 7
#define IN2_PIN 6
#define IN3_PIN 5
#define IN4_PIN 4

bool forwardState = false;
bool backwardState = false;
bool rightState = false;
bool leftState = false;


const int MPU = 0x68;
int16_t prevAcX, prevAcY, prevAcZ;
unsigned long prevTime = 0;

// Threshold for crash detection (adjust as needed)
const float crashThreshold = 1000; // Adjust based on your requirements

//////////////////////////////
//rpm with hall effect sensor
// Hall effect sensor pin
const int hallSensorPin = 3;

// Variables for RPM calculation
volatile int revolutions = 0;
unsigned long previousMillis = 0;
unsigned long intervalHall = 1000; // 1 second interval
///////////////////////////////////////////


//speed speed speed with ir sensor

const byte PulsesPerRevolution = 2;
const unsigned long ZeroTimeout = 100000;
const byte numReadings = 2;

volatile unsigned long LastTimeWeMeasured;
volatile unsigned long PeriodBetweenPulses = ZeroTimeout + 1000;
volatile unsigned long PeriodAverage = ZeroTimeout + 1000;
unsigned long FrequencyRaw;
unsigned long FrequencyReal;
unsigned long RPM;
unsigned int PulseCounter = 1;
unsigned long PeriodSum;

unsigned long LastTimeCycleMeasure = LastTimeWeMeasured;
unsigned long CurrentMicros = micros();
unsigned int AmountOfReadings = 1;
unsigned int ZeroDebouncingExtra;
unsigned long readings[numReadings];
unsigned long readIndex;  
unsigned long total; 
unsigned long average;


// Define wheel radius in centimeters
const float wheelRadius = 5.0; // in centimeters

// Function to calculate speed from RPM
float calculateSpeedFromRPM(float rpmHall) {
  // Convert RPM to revolutions per hour
  float revPerHour = rpmHall * 60.0;

  // Calculate distance traveled in one revolution (circumference of the wheel)
  float distancePerRevolution = 2 * PI * wheelRadius; // PI = 3.14159

  // Calculate total distance traveled in one hour (in centimeters)
  float distancePerHour = revPerHour * distancePerRevolution;

  // Convert total distance traveled from centimeters to kilometers
  float distanceKmPerHour = distancePerHour / 100000.0; // 1 kilometer = 100000 centimeters

  return distanceKmPerHour;
}



/////////////////////////////speedSpeedSpeed////////////

void setup() {

  Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

   Serial.begin(9600);
  pinMode(limitSwitchPin, INPUT_PULLUP); // Configure the pin as input with internal pull-up resistor

   pinMode(trigPin, OUTPUT);
   pinMode(echoPin, INPUT);

  //speed check interrupt
  attachInterrupt(digitalPinToInterrupt(2), Pulse_Event, RISING);

   // Attach an interrupt to the hall sensor pin
  attachInterrupt(digitalPinToInterrupt(hallSensorPin), countRevolutions, FALLING);

    // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);

  // Initialize Blynk
  Serial.begin(115200);
  EspSerial.begin(ESP8266_BAUD);

  Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);
  
  if (Blynk.connected()) {
    Serial.println("Connected to Blynk");
  } else {
    Serial.println("Not connected to Blynk");
  }

  // Set up Blynk button callbacks
  Blynk.virtualWrite(V0, 0); // Forward
  Blynk.virtualWrite(V1, 0); // Backward
  Blynk.virtualWrite(V2, 0); // Right
  Blynk.virtualWrite(V3, 0); // Left

  timer.setInterval(1, checkForward);
  timer.setInterval(1, checkBack);
  timer.setInterval(1, checkRight);
  timer.setInterval(1, checkLeft);
}


  void checkForward(){
    Blynk.virtualWrite(V0, forwardState);
  }

  void checkBack(){
    Blynk.virtualWrite(V1,backwardState);
  }

  void checkRight(){
    Blynk.virtualWrite(V2, rightState);
  }

  void checkLeft(){
    Blynk.virtualWrite(V3, leftState);
  }


void loop() {
  Blynk.run();
  timer.run();



//   //////////////////rpm calculation from IR sensor///////////////

//   float sumSpeed = 0;
//   int countSpeed = 0;

//  LastTimeCycleMeasure = LastTimeWeMeasured;
//   CurrentMicros = micros();
//   if (CurrentMicros < LastTimeCycleMeasure) {
//     LastTimeCycleMeasure = CurrentMicros;
//   }
//   FrequencyRaw = 10000000000 / PeriodAverage;
//   if (PeriodBetweenPulses > ZeroTimeout - ZeroDebouncingExtra || CurrentMicros - LastTimeCycleMeasure > ZeroTimeout - ZeroDebouncingExtra) {
//     FrequencyRaw = 0;  // Set frequency as 0.
//     ZeroDebouncingExtra = 2000;
//   } else {
//     ZeroDebouncingExtra = 0;
//   }
//   FrequencyReal = FrequencyRaw / 10000;

//   RPM = FrequencyRaw / PulsesPerRevolution * 60;
//   RPM = RPM / 10000;
//   total = total - readings[readIndex];
//   readings[readIndex] = RPM;
//   total = total + readings[readIndex];
//   readIndex = readIndex + 1;

//   if (readIndex >= numReadings) {
//     readIndex = 0;
//   }
//   average = total / numReadings;


// Serial.print("IrRPM:");
// Serial.print(RPM);



//   ////////////////////rpm calculation from IR sensor////////////////

/////////////////////////////




  // Gas door status
  int switchState = digitalRead(limitSwitchPin); // Read the state of the limit switch
  if (switchState == HIGH) {
    lcd.clear();
    Serial.print("\    ,   Gas Opened");
    Serial.println(" ");
    lcd.setCursor(0, 0);
    lcd.print("Gas opened");
    Blynk.virtualWrite(GAS_DOOR_VPIN, 1); // Update Blynk LED status to indicate gas door is closed
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.print("\    ,   Gas Closed");
    Serial.println(" ");
    lcd.print("Gas closed");
    Blynk.virtualWrite(GAS_DOOR_VPIN, 0); // Update Blynk LED status to indicate gas door is opened
  }


    // Hall sensor RPM calculation


  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalHall) {
    float rpmHall = (revolutions / 2.0) * 60.0 / (intervalHall / 1000.0); // Divide by 2 for one magnet pole pair
    float speedKmPerHour  = calculateSpeedFromRPM(rpmHall);
    revolutions = 0;
    previousMillis = currentMillis;
    Serial.print("\HallRpm: ");
    Serial.print(rpmHall);
    Serial.print(" ,  ");
    Serial.print("\HallSpeed: ");
    Serial.print(speedKmPerHour);
    Serial.print("   ,    ");
////////////////print print speed speed hall sensor////////////////////////
    lcd.setCursor(0, 1);
    lcd.print("speed: ");
    lcd.print(speedKmPerHour);
    lcd.print("km/h");
////////////////print print speed speed hall sensor////////////////////////

    }




  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 12, true);
  int16_t AcX = Wire.read() << 8 | Wire.read();
  int16_t AcY = Wire.read() << 8 | Wire.read();
  int16_t AcZ = Wire.read() << 8 | Wire.read();
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - prevTime;
  int16_t diffAcX = abs(AcX - prevAcX);
  int16_t diffAcY = abs(AcY - prevAcY);
  int16_t diffAcZ = abs(AcZ - prevAcZ);
  if (elapsedTime < 350 && (diffAcX > crashThreshold || diffAcY > crashThreshold || diffAcZ > crashThreshold)) {
    // Crash detected!
    Serial.println("\Crash detected!");
    lcd.setCursor(1, 0);
    lcd.print("Crash detected!");
    // Blink the exclamation mark
    unsigned long blinkStartTime = millis();
    while (millis() - blinkStartTime < 7000) { // Blink for 7 seconds
      lcd.setCursor(15, 0);
      lcd.print(" ");
      delay(500);
      lcd.setCursor(15, 0);
      lcd.print("!");
      delay(500); 
    }
  
  }

  prevAcX = AcX;
  prevAcY = AcY;
  prevAcZ = AcZ;
  prevTime = currentTime;
  


  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the duration of the pulse from the echo pin
  long duration = pulseIn(echoPin, HIGH);
  
  // Calculate distance in centimeters based on the speed of sound
  // Distance = (duration / 2) / 29.1 (in microseconds per centimeter)
  float distance_cm = duration / 58.2; // 29.1 (microseconds per centimeter) * 2 = 58.2
  
  // Print the distance to the serial monitor
  Serial.print("\  Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  
  // Display warning on LCD if distance is less than or equal to 10cm
  if (distance_cm <= 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Warning: ");
    lcd.print(distance_cm);
    lcd.print("cm!!");
  }


  // Print accelerometer readings to serial monitor
    Serial.print("Accelerometer: ");
    Serial.print("X = "); Serial.print(AcX);
    Serial.print(" | Y = "); Serial.print(AcY);
    Serial.print(" | Z = "); Serial.println(AcZ);



 
  

  // Motor control based on Blynk button states
  if (forwardState) {
    moveForward();
  } else if (backwardState) {
    moveBackward();
  } else if (rightState) {
    turnRight();
  } else if (leftState) {
    turnLeft();
  } else {
    stopMoving();
  }
}

// Interrupt service routine to count revolutions
void countRevolutions() {
  revolutions++;
} 

// Callback function to handle Blynk button states
BLYNK_WRITE(V0) {
  forwardState = param.asInt();
}

BLYNK_WRITE(V1) {
  backwardState = param.asInt();
}

BLYNK_WRITE(V2) {
  rightState = param.asInt();
}

BLYNK_WRITE(V3) {
  leftState = param.asInt();
}

// Motor control functions
void moveForward() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, HIGH);
  digitalWrite(IN4_PIN, LOW);
}

void moveBackward() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, HIGH);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, HIGH);
}

void turnRight() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, HIGH);
  digitalWrite(IN3_PIN, HIGH);
  digitalWrite(IN4_PIN, LOW);
}

void turnLeft() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, HIGH);
}

void stopMoving() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
}


void Pulse_Event() {
  PeriodBetweenPulses = micros() - LastTimeWeMeasured;
  LastTimeWeMeasured = micros();
  if (PulseCounter >= AmountOfReadings)  {
    PeriodAverage = PeriodSum / AmountOfReadings;
    PulseCounter = 1;
    PeriodSum = PeriodBetweenPulses;

    int RemapedAmountOfReadings = map(PeriodBetweenPulses, 40000, 5000, 1, 10);
    RemapedAmountOfReadings = constrain(RemapedAmountOfReadings, 1, 10);
    AmountOfReadings = RemapedAmountOfReadings;
  } else {
    PulseCounter++;
    PeriodSum = PeriodSum + PeriodBetweenPulses;
  }
}


