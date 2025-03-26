#include <LiquidCrystal.h>

// LCD setup
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Button and sensor pins
#define MINUTES_BUTTON A0
#define HOURS_BUTTON A1
#define SET_BUTTON A2
#define DISPLAY_SWITCH_BUTTON A4
#define VIBRATION_POT A3

// Variables for time and settings
int minutes = 0, hours = 0;
bool setMode = false, showBattery = false;
int vibrationIntensity = 0, storedVibrationIntensity = 0;

// Last stored values to detect changes
int lastMinutes = -1, lastHours = -1, lastVibrationIntensity = -1;
bool lastSetMode = false, lastShowBattery = false;

// Button state tracking for debouncing
bool lastMinuteButton = HIGH;
bool lastHourButton = HIGH;
bool lastSetButton = HIGH;
bool lastDisplaySwitchButton = HIGH;

void setup() {
    pinMode(MINUTES_BUTTON, INPUT_PULLUP);
    pinMode(HOURS_BUTTON, INPUT_PULLUP);
    pinMode(SET_BUTTON, INPUT_PULLUP);
    pinMode(DISPLAY_SWITCH_BUTTON, INPUT_PULLUP);

    lcd.begin(16, 2);
    updateDisplay();
}

void loop() {
    // Read button states with debounce
    bool minuteButton = digitalRead(MINUTES_BUTTON);
    bool hourButton = digitalRead(HOURS_BUTTON);
    bool setButton = digitalRead(SET_BUTTON);
    bool displaySwitchButton = digitalRead(DISPLAY_SWITCH_BUTTON);

    bool updated = false; // Track if an update is needed

    if (minuteButton == LOW && lastMinuteButton == HIGH) {
        delay(50);
        if (digitalRead(MINUTES_BUTTON) == LOW && !setMode) {
            minutes = (minutes + 10) % 60;
            updated = true;
        }
    }
    lastMinuteButton = minuteButton;

    if (hourButton == LOW && lastHourButton == HIGH) {
        delay(50);
        if (digitalRead(HOURS_BUTTON) == LOW && !setMode) {
            hours = (hours + 1) % 13;
            updated = true;
        }
    }
    lastHourButton = hourButton;

    if (setButton == LOW && lastSetButton == HIGH) {
        delay(50);
        if (digitalRead(SET_BUTTON) == LOW) {
            setMode = !setMode;
            if (setMode) { 
                storedVibrationIntensity = vibrationIntensity; // Store value when locking
            }
            updated = true;
        }
    }
    lastSetButton = setButton;

    if (displaySwitchButton == LOW && lastDisplaySwitchButton == HIGH) {
        delay(50);
        if (digitalRead(DISPLAY_SWITCH_BUTTON) == LOW) {
            showBattery = !showBattery;
            updated = true;
        }
    }
    lastDisplaySwitchButton = displaySwitchButton;

    // **Ensure vibration intensity updates when unlocked**
    if (!setMode) {  
        int potValue = analogRead(VIBRATION_POT);
        vibrationIntensity = map(potValue, 0, 1023, 0, 100); 
        storedVibrationIntensity = vibrationIntensity;
    } else {
        vibrationIntensity = storedVibrationIntensity;
    }

    // Only update the display if a value has changed
    if (updated || minutes != lastMinutes || hours != lastHours || vibrationIntensity != lastVibrationIntensity || showBattery != lastShowBattery || setMode != lastSetMode) {
        updateDisplay();
        lastMinutes = minutes;
        lastHours = hours;
        lastVibrationIntensity = vibrationIntensity;
        lastShowBattery = showBattery;
        lastSetMode = setMode;
    }
}

// Function to update LCD display
void updateDisplay() {
    lcd.clear();
    if (showBattery) {
        lcd.setCursor(0, 0);
        lcd.print("Battery: ");
        lcd.print(getBatteryPercentage());
        lcd.print("%");

        lcd.setCursor(0, 1);
        lcd.print("Vibration: ");
        lcd.print(vibrationIntensity);
        lcd.print("%");
    } else {
        lcd.setCursor(4, 0);
        lcd.print(hours < 10 ? "0" : "");  
        lcd.print(hours);
        lcd.print(":");
        lcd.print(minutes < 10 ? "0" : "");  
        lcd.print(minutes);

        lcd.setCursor(0, 1);
        lcd.print(setMode ? "SET MODE: LOCKED" : "SET MODE: OPEN  ");
    }
}

// Placeholder for battery percentage function
int getBatteryPercentage() {
    return 75; // Replace with actual function when available
}
