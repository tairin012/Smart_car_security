#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define IN1_PIN 7
#define IN2_PIN 6
#define IN3_PIN 5
#define IN4_PIN 4

const int limitSwitchPin = 9;
const int hallSensorPin = 3;
const int trigPin = 46;
const int echoPin = 47;



const int MPU = 0x68;
int16_t prevAcX, prevAcY, prevAcZ;
unsigned long prevTime = 0;
const float crashThreshold = 1200;

// RPM calculation variables
volatile int revolutions = 0;
unsigned long previousMillis = 0;
unsigned long intervalHall = 1000;

// IR sensor speed calculation variables
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

const float wheelRadius = 5.0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  pinMode(limitSwitchPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin), countRevolutions, FALLING);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
}

void loop() {


  moveForward();
  
  // Gyro crash detection
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
  if (elapsedTime < 150 && (diffAcX > crashThreshold || diffAcY > crashThreshold || diffAcZ > crashThreshold)) {
    Serial.println("\Crash detected!");
    lcd.setCursor(1, 0);
    lcd.print("Crash detected!");
    unsigned long blinkStartTime = millis();
    while (millis() - blinkStartTime < 7000) { // Blink for 7 seconds
      lcd.setCursor(15, 0);
      lcd.print(" ");
      delay(500);
      lcd.setCursor(15, 0);
      lcd.print("!");
      delay(500);
    }
    lcd.clear();
  }
  prevAcX = AcX;
  prevAcY = AcY;
  prevAcZ = AcZ;
  prevTime = currentTime;

  // IR sensor speed calculation
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalHall) {
    float rpmHall = (revolutions / 2.0) * 60.0 / (intervalHall / 1000.0);
    float speedKmPerHour  = calculateSpeedFromRPM(rpmHall);
    revolutions = 0;
    previousMillis = currentMillis;
    Serial.print("\HallRpm: ");
    Serial.print(rpmHall);
    Serial.print(" ,  ");
    Serial.print("\HallSpeed: ");
    Serial.print(speedKmPerHour);
    Serial.print("   ,    ");
    lcd.setCursor(0, 1);
    lcd.print("speed: ");
    lcd.print(speedKmPerHour);
    lcd.print("km/h");
  }

  // Gas door status
  int switchState = digitalRead(limitSwitchPin);
  if (switchState == HIGH) {
    lcd.clear();
    Serial.print("\    ,   Gas Opened");
    Serial.println(" ");
    lcd.setCursor(0, 0);
    lcd.print("Gas opened");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.print("\    ,   Gas Closed");
    Serial.println(" ");
    lcd.print("Gas closed");
  }

  // Ultrasonic sensor distance measurement
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance_cm = duration / 58.2;
  Serial.print("\  Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  if (distance_cm <= 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Warning: ");
    lcd.print(distance_cm);
    lcd.print("cm!!");
  }
}

void countRevolutions() {
  revolutions++;
}

float calculateSpeedFromRPM(float rpmHall) {
  float revPerHour = rpmHall * 60.0;
  float distancePerRevolution = 2 * PI * wheelRadius;
  float distancePerHour = revPerHour * distancePerRevolution;
  float distanceKmPerHour = distancePerHour / 100000.0;
  return distanceKmPerHour;
}


// Motor control functions
void moveForward() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, HIGH);
  digitalWrite(IN4_PIN, LOW);

  delay(5000);
}
