 #include <MemoryFree.h>
 #include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int MPU = 0x68;
int16_t prevAcX, prevAcY, prevAcZ;
unsigned long prevTime = 0;

// Threshold for crash detection (adjust as needed)
const float crashThreshold = 3000; // Adjust based on your requirements

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address and dimensions

void setup() {
    Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    Serial.begin(9600);

    // Initialize the LCD
    lcd.init();
    lcd.backlight();
}

void loop() {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 12, true);
    int16_t AcX = Wire.read() << 8 | Wire.read();
    int16_t AcY = Wire.read() << 8 | Wire.read();
    int16_t AcZ = Wire.read() << 8 | Wire.read();

    // Get current time
    unsigned long currentTime = millis();

    // Calculate time elapsed since last reading
    unsigned long elapsedTime = currentTime - prevTime;

    // Calculate differences in accelerometer readings
    int16_t diffAcX = abs(AcX - prevAcX);
    int16_t diffAcY = abs(AcY - prevAcY);
    int16_t diffAcZ = abs(AcZ - prevAcZ);

    // Check if a sudden change (crash) is detected
    if (elapsedTime < 350 && (diffAcX > crashThreshold || diffAcY > crashThreshold || diffAcZ > crashThreshold)) {
        Serial.println("Crash detected!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Crash detected!");
        delay(5000); // Display crash message for 5 seconds
        lcd.clear();
    }

    // Print accelerometer readings to serial monitor
    Serial.print("Accelerometer: ");
    Serial.print("X = "); Serial.print(AcX);
    Serial.print(" | Y = "); Serial.print(AcY);
    Serial.print(" | Z = "); Serial.println(AcZ);



    // Update previous accelerometer readings and time
    prevAcX = AcX;
    prevAcY = AcY;
    prevAcZ = AcZ;
    prevTime = currentTime;

    delay(333); // Adjust delay as needed
}
