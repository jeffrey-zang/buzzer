// =======================
//   Trivia Buzzer Sketch
//  – with 74HC595 for console LEDs
// =======================

const uint8_t playerLEDs[8] = { A0, A1, A2, A3, A4, A5 }; //a6, a7
const uint8_t buttonPins[8] = {  2,  3,  4,  5,  6,  7,  8 };

// shift-register pins (change as needed)
const uint8_t dataPin  = 10;  // SER (DS) on 74HC595
const uint8_t clockPin = 11;  // SRCLK
const uint8_t latchPin = 12;  // RCLK

const uint8_t buzzerPin =  13;
const uint8_t resetPin  =  0;

bool    locked       = false;
uint8_t winner       = 0xFF;
uint8_t consoleState = 0x00;  // bitmask of console LEDs

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Trivia Buzzer (with shift-reg) ==="));

  // Buttons w/ pull-ups (LOW = pressed)
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(resetPin, INPUT_PULLUP);

  // Player LEDs off
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(playerLEDs[i], OUTPUT);
    digitalWrite(playerLEDs[i], LOW);
  }
  pinMode(9, OUTPUT);

  // Shift-register control pins
  pinMode(dataPin,  OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  // Buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  resetAll();
}

void loop() {
  // — Reset button (active-LOW) —
  if (digitalRead(resetPin) == LOW) {
    delay(50);
    if (digitalRead(resetPin) == LOW) {
      Serial.println(F(">> Reset pressed"));
      resetAll();
      while (digitalRead(resetPin) == LOW);
      delay(50);
      Serial.println(F(">> Reset released"));
    }
  }

  // — Buzzers —
  if (!locked) {
    for (uint8_t i = 0; i < 7; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(20);
        if (digitalRead(buttonPins[i]) == LOW) {
          locked = true;
          if(digitalRead(7) == LOW && digitalRead(8) == LOW){ //player 8 button
            winner = 7;
          } else {
            winner = i;
          }
          Serial.print(F("++ Buzz from player "));
          Serial.println(i);

          // light station LED
          if(winner >= 4){
            switch(winner){
              case 4:
                digitalWrite(A4, HIGH);
                digitalWrite(A5, LOW);
                digitalWrite(9, LOW);
                break;
              case 5:
                digitalWrite(A4, LOW);
                digitalWrite(A5, HIGH);
                digitalWrite(9, HIGH);
                break;
              case 6:
                digitalWrite(A4, HIGH);
                digitalWrite(A5, HIGH);
                digitalWrite(9, LOW);
                break;
              case 7:
                digitalWrite(A4, LOW);
                digitalWrite(A5, LOW);
                digitalWrite(9, HIGH);
                break;
            }
          } else {
            digitalWrite(playerLEDs[winner], HIGH);
          }

          // light console LED via shift register
          consoleState = (1 << winner);
          updateShiftRegister(consoleState);

          playBeep(500, 1, true); // default config for beep
          break;
        }
      }
    }
  }
}

void resetAll() {
  if (winner != 0xFF) {
    playBeep(100, 2, false);
  }
  locked       = false;
  winner       = 0xFF;
  consoleState = 0x00;

  // clear player LEDs
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(playerLEDs[i], LOW);
  }
  digitalWrite(A6, LOW);
  digitalWrite(A7, LOW);
  digitalWrite(9, LOW);

  // clear console LEDs
  updateShiftRegister(consoleState);

  Serial.println(F("++ System reset"));
}

// Sends the current consoleState byte to the 74HC595
void updateShiftRegister(uint8_t bits) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, bits);
  digitalWrite(latchPin, HIGH);
}

// ~500 Hz square wave for 200 ms
void playBeep(const unsigned long duration, const unsigned int halfPeriod, bool detection) {
  Serial.println(F("++ Playing beep"));
  unsigned long t0 = millis();
  while (millis() - t0 < duration) {
    if (detection) {
      if (digitalRead(0) == LOW) {
        break;
      }
    }
    digitalWrite(buzzerPin, HIGH);
    delay(halfPeriod);
    digitalWrite(buzzerPin, LOW);
    delay(halfPeriod);
  }
  Serial.println(F("++ Beep done"));
}
