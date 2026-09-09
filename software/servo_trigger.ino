/*
  servo_trigger.ino

  Runs on a SEPARATE Arduino (not your GRBL board).
  Watches a digital input pin for HIGH/LOW from GRBL's coolant
  enable pin (M8 = ON, M9 = OFF), and drives an SG90 servo to
  one of two angles accordingly, using the proper Servo library
  (real 50Hz pulse-width signal, unlike GRBL's raw spindle PWM).

  WIRING:
    GRBL Uno  A3 (coolant flood pin)  -> this Arduino's TRIGGER_PIN
    GRBL Uno  GND                     -> this Arduino's GND (shared ground, required)
    SG90 signal (orange/yellow)       -> this Arduino's SERVO_PIN
    SG90 power (red)                  -> external 5V supply (NOT Arduino 5V pin)
    SG90 ground (brown/black)         -> shared GND with Arduino + external 5V supply

  COMMANDS TO TEST FROM UGS (sent to the GRBL board, not this one):
    M8   -> pen DOWN angle
    M9   -> pen UP angle
*/

#include <Servo.h>

const int TRIGGER_PIN = 2;   // Digital input reading GRBL's coolant pin state
const int SERVO_PIN   = 9;   // PWM pin driving the SG90 signal wire

const int ANGLE_UP   = 0;    // Pen-up angle, adjust to taste (0-180)
const int ANGLE_DOWN = 90;   // Pen-down angle, adjust to taste (0-180)

Servo penServo;
int lastState = -1; // Force an update on the very first loop

void setup() {
  pinMode(TRIGGER_PIN, INPUT); // Use INPUT_PULLUP instead if wiring is noisy/floating
  penServo.attach(SERVO_PIN);
  penServo.write(ANGLE_UP);    // Start in the up position
  lastState = LOW;
}

void loop() {
  int state = digitalRead(TRIGGER_PIN);

  if (state != lastState) {
    if (state == HIGH) {
      penServo.write(ANGLE_DOWN); // M8 was sent on the GRBL board
    } else {
      penServo.write(ANGLE_UP);   // M9 was sent on the GRBL board
    }
    lastState = state;
    delay(300); // Simple debounce so quick signal glitches don't cause jitter
  }
}
