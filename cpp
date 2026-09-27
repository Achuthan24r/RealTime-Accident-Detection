/*
  ============================================================
                   ESP32
              REAL-TIME ANOMALY MONITOR
  ============================================================

  Hardware:
    - ESP32 Dev Module
    - 16x2 I2C LCD @ 0x27
    - DHT11
    - HC-SR04
    - SW-520D 2-pin movement/tilt sensor
    - Active buzzer
    - TM1637 4-digit display

  ============================================================
  PIN MAP
  ============================================================

  LCD:
    SDA -> GPIO 21
    SCL -> GPIO 22

  DHT11:
    DATA -> GPIO 4

  HC-SR04:
    TRIG -> GPIO 5
    ECHO -> GPIO 18

  SW-520D:
    Signal -> GPIO 27
    GND    -> GND

  BUZZER:
    + -> GPIO 23
    - -> GND

  TM1637:
    CLK -> GPIO 25
    DIO -> GPIO 26

  ============================================================
*/

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <TM1637Display.h>

// ============================================================
// PIN DEFINITIONS
// ============================================================

// LCD
#define LCD_SDA 21
#define LCD_SCL 22

// DHT11
#define DHT_PIN 4
#define DHT_TYPE DHT11

// HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 18

// SW-520D
#define SW520D_PIN 27

// Buzzer
#define BUZZER_PIN 23

// TM1637
#define TM1637_CLK 25
#define TM1637_DIO 26

// ============================================================
// OBJECTS
// ============================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht(DHT_PIN, DHT_TYPE);

TM1637Display display(
  TM1637_CLK,
  TM1637_DIO
);

// ============================================================
// VARIABLES
// ============================================================

// Sensor values
float temperature = 0.0;
float humidity = 0.0;
float distance = 0.0;

// Previous distance
float previousDistance = -1;

// Movement event counter
int movementEvents = 0;

// Current anomaly score
int anomalyScore = 0;

// System status
String systemStatus = "NORMAL";

// Timing
unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastMovementCheck = 0;
unsigned long lastSerialUpdate = 0;

// Movement window
unsigned long movementWindowStart = 0;

const unsigned long SENSOR_INTERVAL = 2000;
const unsigned long MOVEMENT_WINDOW = 5000;
const unsigned long LCD_INTERVAL = 1500;

// SW-520D debounce
int lastSWState = HIGH;
unsigned long lastDebounceTime = 0;

const unsigned long DEBOUNCE_TIME = 80;

// LCD page
int lcdPage = 0;

// Buzzer state
bool alarmActive = false;

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("       HOME GUARDIAN");
  Serial.println("     ESP32 SAFETY SYSTEM");
  Serial.println("================================");

  // ----------------------------------------------------------
  // GPIO
  // ----------------------------------------------------------

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(SW520D_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // ----------------------------------------------------------
  // I2C
  // ----------------------------------------------------------

  Wire.begin(LCD_SDA, LCD_SCL);

  // ----------------------------------------------------------
  // LCD
  // ----------------------------------------------------------

  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("HOME GUARDIAN");

  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  // ----------------------------------------------------------
  // DHT
  // ----------------------------------------------------------

  dht.begin();

  // ----------------------------------------------------------
  // TM1637
  // ----------------------------------------------------------

  display.setBrightness(7);
  display.clear();

  // ----------------------------------------------------------
  // Startup display
  // ----------------------------------------------------------

  delay(2000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SYSTEM READY");

  lcd.setCursor(0, 1);
  lcd.print("MONITORING...");

  display.showNumberDec(0, true);

  delay(2000);

  movementWindowStart = millis();

  Serial.println("System initialized.");
  Serial.println("Monitoring started.");
  Serial.println();
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  unsigned long currentMillis = millis();

  // ----------------------------------------------------------
  // READ SENSORS
  // ----------------------------------------------------------

  if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {

    lastSensorRead = currentMillis;

    readDHT11();

    readUltrasonic();

    calculateAnomaly();
  }

  // ----------------------------------------------------------
  // CHECK SW-520D
  // ----------------------------------------------------------

  if (currentMillis - lastMovementCheck >= 10) {

    lastMovementCheck = currentMillis;

    checkMovement();
  }

  // ----------------------------------------------------------
  // RESET MOVEMENT WINDOW
  // ----------------------------------------------------------

  if (currentMillis - movementWindowStart >= MOVEMENT_WINDOW) {

    movementEvents = 0;

    movementWindowStart = currentMillis;
  }

  // ----------------------------------------------------------
  // UPDATE LCD
  // ----------------------------------------------------------

  if (currentMillis - lastLCDUpdate >= LCD_INTERVAL) {

    lastLCDUpdate = currentMillis;

    updateLCD();

    update7Segment();
  }

  // ----------------------------------------------------------
  // SERIAL DASHBOARD
  // ----------------------------------------------------------

  if (currentMillis - lastSerialUpdate >= 3000) {

    lastSerialUpdate = currentMillis;

    printDashboard();
  }

  // ----------------------------------------------------------
  // BUZZER
  // ----------------------------------------------------------

  controlAlarm();
}

// ============================================================
// DHT11
// ============================================================

void readDHT11() {

  float newTemperature = dht.readTemperature();
  float newHumidity = dht.readHumidity();

  if (!isnan(newTemperature)) {
    temperature = newTemperature;
  }

  if (!isnan(newHumidity)) {
    humidity = newHumidity;
  }
}

// ============================================================
// HC-SR04
// ============================================================

void readUltrasonic() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(
    ECHO_PIN,
    HIGH,
    30000
  );

  if (duration > 0) {

    float newDistance =
      duration * 0.0343 / 2.0;

    // Ignore obviously invalid values
    if (newDistance >= 2 &&
        newDistance <= 400) {

      distance = newDistance;
    }
  }
}

// ============================================================
// SW-520D MOVEMENT DETECTION
// ============================================================

void checkMovement() {

  int reading = digitalRead(SW520D_PIN);

  if (reading != lastSWState) {

    lastDebounceTime = millis();

    lastSWState = reading;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {

    static int stableState = HIGH;

    if (reading != stableState) {

      stableState = reading;

      // LOW = contact closed
      if (stableState == LOW) {

        movementEvents++;

        Serial.print("MOVEMENT EVENT: ");
        Serial.println(movementEvents);
      }
    }
  }
}

// ============================================================
// ANOMALY CALCULATION
// ============================================================

void calculateAnomaly() {

  int vibrationScore = 0;
  int distanceScore = 0;
  int temperatureScore = 0;

  // ----------------------------------------------------------
  // MOVEMENT SCORE
  // ----------------------------------------------------------

  if (movementEvents == 0) {

    vibrationScore = 0;

  }
  else if (movementEvents <= 3) {

    vibrationScore = 30;

  }
  else if (movementEvents <= 8) {

    vibrationScore = 70;

  }
  else {

    vibrationScore = 100;
  }

  // ----------------------------------------------------------
  // DISTANCE CHANGE SCORE
  // ----------------------------------------------------------

  if (previousDistance > 0 &&
      distance > 0) {

    float difference =
      abs(distance - previousDistance);

    if (difference < 5) {

      distanceScore = 0;

    }
    else if (difference < 15) {

      distanceScore = 40;

    }
    else {

      distanceScore = 100;
    }
  }

  // Save current distance
  if (distance > 0) {

    previousDistance = distance;
  }

  // ----------------------------------------------------------
  // TEMPERATURE SCORE
  // ----------------------------------------------------------

  if (temperature < 18 ||
      temperature > 40) {

    temperatureScore = 100;

  }
  else if (temperature < 20 ||
           temperature > 35) {

    temperatureScore = 50;

  }
  else {

    temperatureScore = 0;
  }

  // ----------------------------------------------------------
  // WEIGHTED SCORE
  //
  // Movement  = 60%
  // Distance  = 25%
  // Temperature = 15%
  // ----------------------------------------------------------

  anomalyScore =
    (vibrationScore * 60 / 100) +
    (distanceScore * 25 / 100) +
    (temperatureScore * 15 / 100);

  // ----------------------------------------------------------
  // STATUS
  // ----------------------------------------------------------

  if (anomalyScore >= 70) {

    systemStatus = "CRITICAL";

  }
  else if (anomalyScore >= 40) {

    systemStatus = "WARNING";

  }
  else {

    systemStatus = "NORMAL";
  }

  alarmActive =
    (systemStatus == "CRITICAL");
}

// ============================================================
// LCD
// ============================================================

void updateLCD() {

  lcd.clear();

  // ----------------------------------------------------------
  // PAGE 0 - STATUS
  // ----------------------------------------------------------

  if (lcdPage == 0) {

    lcd.setCursor(0, 0);

    lcd.print("STATUS:");

    lcd.print(systemStatus);

    lcd.setCursor(0, 1);

    lcd.print("SCORE:");

    lcd.print(anomalyScore);

    lcd.print("/100");
  }

  // ----------------------------------------------------------
  // PAGE 1 - TEMPERATURE
  // ----------------------------------------------------------

  else if (lcdPage == 1) {

    lcd.setCursor(0, 0);

    lcd.print("TEMP:");

    lcd.print(temperature, 1);

    lcd.print((char)223);

    lcd.print("C");

    lcd.setCursor(0, 1);

    lcd.print("HUM:");

    lcd.print(humidity, 1);

    lcd.print("%");
  }

  // ----------------------------------------------------------
  // PAGE 2 - DISTANCE
  // ----------------------------------------------------------

  else if (lcdPage == 2) {

    lcd.setCursor(0, 0);

    lcd.print("DISTANCE:");

    lcd.setCursor(0, 1);

    lcd.print(distance, 1);

    lcd.print(" cm");
  }

  // ----------------------------------------------------------
  // PAGE 3 - MOVEMENT
  // ----------------------------------------------------------

  else {

    lcd.setCursor(0, 0);

    lcd.print("MOVEMENT:");

    lcd.print(movementEvents);

    lcd.setCursor(0, 1);

    lcd.print("EVENTS / WINDOW");
  }

  lcdPage++;

  if (lcdPage > 3) {

    lcdPage = 0;
  }
}

// ============================================================
// TM1637
// ============================================================

void update7Segment() {

  // Display anomaly score

  int value = anomalyScore;

  if (value > 9999) {
    value = 9999;
  }

  display.showNumberDec(
    value,
    true
  );
}

// ============================================================
// BUZZER
// ============================================================

void controlAlarm() {

  if (alarmActive) {

    // Beep pattern
    if ((millis() / 250) % 2 == 0) {

      digitalWrite(
        BUZZER_PIN,
        HIGH
      );

    }
    else {

      digitalWrite(
        BUZZER_PIN,
        LOW
      );
    }

  }
  else {

    digitalWrite(
      BUZZER_PIN,
      LOW
    );
  }
}

// ============================================================
// SERIAL DASHBOARD
// ============================================================

void printDashboard() {

  Serial.println();
  Serial.println("================================");
  Serial.println("START");
  Serial.println("================================");

  Serial.print("Temperature : ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  Serial.print("Humidity    : ");
  Serial.print(humidity, 1);
  Serial.println(" %");

  Serial.print("Distance    : ");
  Serial.print(distance, 1);
  Serial.println(" cm");

  Serial.print("Movement    : ");
  Serial.println(movementEvents);

  Serial.print("Anomaly     : ");
  Serial.print(anomalyScore);
  Serial.println(" / 100");

  Serial.print("STATUS      : ");
  Serial.println(systemStatus);

  Serial.println("================================");
}
