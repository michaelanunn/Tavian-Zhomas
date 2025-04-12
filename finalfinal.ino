#include <LiquidCrystal.h>

// Function prototypes
int getAverageAnalogReading(int pin);
int getBatteryPercentage(int analogValue);
void triggerVibration();

// Pin definitions
const int motorPin = 3;
const int potPin = A3;
const int batteryPin = A5; // Separate analog pin for battery sensor
const int numSamples = 20;

// LCD setup
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Button and sensor pins
#define MINUTES_BUTTON A0
#define HOURS_BUTTON A1
#define SET_BUTTON A2
#define DISPLAY_SWITCH_BUTTON A4

int minutes = 0, hours = 0;
bool setMode = false, showBattery = false;
int vibrationIntensity = 0, storedVibrationIntensity = 0;

// For change tracking
int lastMinutes = -1, lastHours = -1, lastVibrationIntensity = -1;
bool lastSetMode = false, lastShowBattery = false;

// Button state memory
bool lastMinuteButton = HIGH;
bool lastHourButton = HIGH;
bool lastSetButton = HIGH;
bool lastDisplaySwitchButton = HIGH;

// Timing
unsigned long previousMillis = 0;
const unsigned long interval = 1000;

unsigned long lastLCDUpdate = 0;
const unsigned long lcdUpdateInterval = 500; // Now updates every 500ms

void setup() {
    Serial.begin(9600);
    pinMode(motorPin, OUTPUT);
    pinMode(MINUTES_BUTTON, INPUT_PULLUP);
    pinMode(HOURS_BUTTON, INPUT_PULLUP);
    pinMode(SET_BUTTON, INPUT_PULLUP);
    pinMode(DISPLAY_SWITCH_BUTTON, INPUT_PULLUP);
    pinMode(7, OUTPUT);

    lcd.begin(16, 2);
    updateDisplay();  // Initial display
}

void loop() {
    handleButtons();
    handleVibrationInput();
    controlVibrationMotor();

    if (setMode && (hours > 0 || minutes > 0)) {
        runCountdown();
    }

    if ((minutes != lastMinutes || hours != lastHours ||
         storedVibrationIntensity != lastVibrationIntensity ||
         showBattery != lastShowBattery || setMode != lastSetMode) &&
         millis() - lastLCDUpdate >= lcdUpdateInterval) {

        updateDisplay();
        lastLCDUpdate = millis();

        lastMinutes = minutes;
        lastHours = hours;
        lastVibrationIntensity = storedVibrationIntensity;
        lastShowBattery = showBattery;
        lastSetMode = setMode;
    }
}

void handleButtons() {
    bool minuteButton = digitalRead(MINUTES_BUTTON);
    bool hourButton = digitalRead(HOURS_BUTTON);
    bool setButton = digitalRead(SET_BUTTON);
    bool displaySwitchButton = digitalRead(DISPLAY_SWITCH_BUTTON);

    if (minuteButton == LOW && lastMinuteButton == HIGH) {
        delay(50);
        if (digitalRead(MINUTES_BUTTON) == LOW && !setMode) {
            minutes = (minutes + 10) % 60;
        }
    }
    lastMinuteButton = minuteButton;

    if (hourButton == LOW && lastHourButton == HIGH) {
        delay(50);
        if (digitalRead(HOURS_BUTTON) == LOW && !setMode) {
            hours = (hours + 1) % 13;
        }
    }
    lastHourButton = hourButton;

    if (setButton == LOW && lastSetButton == HIGH) {
        delay(50);
        if (digitalRead(SET_BUTTON) == LOW) {
            setMode = !setMode;
        }
    }
    lastSetButton = setButton;

    if (displaySwitchButton == LOW && lastDisplaySwitchButton == HIGH) {
        delay(50);
        if (digitalRead(DISPLAY_SWITCH_BUTTON) == LOW) {
            showBattery = !showBattery;
        }
    }
    lastDisplaySwitchButton = displaySwitchButton;
}

void handleVibrationInput() {
    int potValue = analogRead(potPin);
    int mappedValue = map(potValue, 0, 845, 0, 100); // More responsive: 850 = 100%
    mappedValue = constrain(mappedValue, 0, 100);    // ✅ Keep within bounds

    // Prevent unnecessary updates if the pot is jittery
    if (abs(mappedValue - storedVibrationIntensity) >= 2) {
        storedVibrationIntensity = mappedValue;
    }

    if (!setMode && (hours == 0 && minutes == 0)) {
        vibrationIntensity = storedVibrationIntensity;
    }
}

void controlVibrationMotor() {
    int motorValue = map(vibrationIntensity, 0, 100, 0, 255);
    analogWrite(motorPin, motorValue);
    Serial.println(motorValue);
}

void updateDisplay() {
    lcd.clear();

    if (showBattery) {
        int batteryAnalog = getAverageAnalogReading(batteryPin); // Battery now reads separately

        lcd.setCursor(0, 0);
        lcd.print("Battery: ");
        lcd.print(getBatteryPercentage(batteryAnalog));
        lcd.print("%");

        lcd.setCursor(0, 1);
        lcd.print("Vibration: ");
        lcd.print(storedVibrationIntensity);
        lcd.print("%");
    } else {
        lcd.setCursor(4, 0);
        lcd.print(hours < 10 ? "0" : "");
        lcd.print(hours);
        lcd.print(":");
        lcd.print(minutes < 10 ? "0" : "");
        lcd.print(minutes);

        lcd.setCursor(0, 1);
        lcd.print(setMode ? "SET MODE: LOCKED" : "SET MODE: OPEN ");
    }
}

int getBatteryPercentage(int analogValue) {
    const int minADC = 710;
    const int maxADC = 970;
    float batteryPercentage = ((float)(analogValue - minADC) / (maxADC - minADC)) * 100.0;

    if (batteryPercentage > 100.0) batteryPercentage = 100.0;
    else if (batteryPercentage < 0.0) batteryPercentage = 0.0;

    return (int)batteryPercentage;
}

void runCountdown() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        if (minutes == 0 && hours > 0) {
            hours--;
            minutes = 59;
        } else if (minutes > 0) {
            minutes--;
        }

        if (minutes == 0 && hours == 0) {
            triggerVibration();
        }
    }
}

void triggerVibration() {
    int motorValue = map(storedVibrationIntensity, 0, 100, 0, 255);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! WAKE UP !!!");

    // Haptic pattern: vibrate 3s, pause 0.5s for 30 seconds total
    unsigned long startTime = millis();
    while (millis() - startTime < 30000) {
        analogWrite(motorPin, motorValue);
        delay(3000);
        analogWrite(motorPin, 0);
        delay(500);
    }

    analogWrite(motorPin, 0);
    vibrationIntensity = 0;

    // ✅ Show ALARM OVER message
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("ALARM OVER");
    lcd.setCursor(2, 1);
    lcd.print("Good Morning!");
    delay(4000); // Show message for 4 seconds
}

int getAverageAnalogReading(int pin) {
    long sum = 0;
    for (int i = 0; i < numSamples; i++) {
        sum += analogRead(pin);
        delay(5);
    }
    return sum / numSamples;
}
