#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Relay pin assignments (relays are active HIGH)
const int compressorPower = 2;
const int C1 = 3;
const int A1_valve = 4;
const int RELAY_B1 = 5;
const int B2 = 6;
const int RELAY_A2 = 7;
const int KOMP_KESICI1 = 8;
const int KOMP_CIKIS1 = 9;
const int KOMP_AC = 10;
const int KOMP_EMICI1 = 11;
const int C2 = 12;
const int KOMP_GIRIS1 = 13;
const int KOMP_BASINC1 = 14;
const int ODA_CIKIS = 15;
const int TAHLIYE = 16;

// Buzzer pin
const int buzzerPin = 17;

// Button pins
const int buttonA1 = 18;
const int buttonK1 = 19;

// State definitions
enum State {
  IDLE,
  A1_SEQUENCE1,
  A1_SEQUENCE2,
  WAITING_10MIN,
  POST_10MIN_SEQUENCE,
  K1_SEQUENCE
};

State currentState = IDLE;
unsigned long stateStartTime = 0;
int sequenceStep = 0;
int a1PressCount = 0;

// Button state tracking
bool a1Pressed = false;
bool k1Pressed = false;
bool a1WasPressed = false;
bool k1WasPressed = false;

void setup() {
  // Set all relay pins as outputs
  pinMode(compressorPower, OUTPUT);
  pinMode(C1, OUTPUT);
  pinMode(A1_valve, OUTPUT);
  pinMode(RELAY_B1, OUTPUT);
  pinMode(B2, OUTPUT);
  pinMode(RELAY_A2, OUTPUT);
  pinMode(KOMP_KESICI1, OUTPUT);
  pinMode(KOMP_CIKIS1, OUTPUT);
  pinMode(KOMP_AC, OUTPUT);
  pinMode(KOMP_EMICI1, OUTPUT);
  pinMode(C2, OUTPUT);
  pinMode(KOMP_GIRIS1, OUTPUT);
  pinMode(KOMP_BASINC1, OUTPUT);
  pinMode(ODA_CIKIS, OUTPUT);
  pinMode(TAHLIYE, OUTPUT);

  // Set buzzer pin as output
  pinMode(buzzerPin, OUTPUT);

  // Set button pins as inputs with pullup
  pinMode(buttonA1, INPUT_PULLUP);
  pinMode(buttonK1, INPUT_PULLUP);

  // On power on, activate compressor power
  digitalWrite(compressorPower, HIGH);
  Serial.println("Compressor power ON");
  beep();

  // Ensure all relays start LOW (off)
  digitalWrite(C1, LOW);
  digitalWrite(A1_valve, LOW);
  digitalWrite(RELAY_B1, LOW);
  digitalWrite(B2, LOW);
  digitalWrite(RELAY_A2, LOW);
  digitalWrite(KOMP_KESICI1, LOW);
  digitalWrite(KOMP_CIKIS1, LOW);
  digitalWrite(KOMP_AC, LOW);
  digitalWrite(KOMP_EMICI1, LOW);
  digitalWrite(C2, LOW);
  digitalWrite(KOMP_GIRIS1, LOW);
  digitalWrite(KOMP_BASINC1, LOW);
  digitalWrite(ODA_CIKIS, LOW);
  digitalWrite(TAHLIYE, LOW);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  Serial.begin(9600);
}

void beep() {
  tone(buzzerPin, 0400, 90); // 1000 Hz, 100ms duration
  //Serial.println("Buzzer: Beep");
}

void loop() {
  // Read button states with debouncing
  bool a1Current = (digitalRead(buttonA1) == LOW);
  bool k1Current = (digitalRead(buttonK1) == LOW);

  if (a1Current && !a1WasPressed) {
    a1Pressed = true;
    a1WasPressed = true;
    Serial.println("Button A1 pressed");
    beep();
  } else if (!a1Current) {
    a1WasPressed = false;
  }

  if (k1Current && !k1WasPressed) {
    k1Pressed = true;
    k1WasPressed = true;
    Serial.println("Button K1 pressed");
    beep();
  } else if (!k1Current) {
    k1WasPressed = false;
  }

  // State machine
  switch (currentState) {
    case IDLE:
      if (a1Pressed) {
        a1PressCount++;
        a1Pressed = false;
        if (a1PressCount == 1) {
          currentState = A1_SEQUENCE1;
          stateStartTime = millis();
          sequenceStep = 0;
          Serial.println("Starting A1 Sequence 1");
        } else if (a1PressCount == 2) {
          currentState = A1_SEQUENCE2;
          stateStartTime = millis();
          sequenceStep = 0;
          a1PressCount = 0;
          Serial.println("Starting A1 Sequence 2");
        }
      }
      if (k1Pressed) {
        k1Pressed = false;
        currentState = K1_SEQUENCE;
        stateStartTime = millis();
        sequenceStep = 0;
        Serial.println("Starting K1 Sequence");
      }
      break;

    case A1_SEQUENCE1:
      handleA1Sequence1();
      break;

    case A1_SEQUENCE2:
      handleA1Sequence2();
      break;

    case WAITING_10MIN:
      if (millis() - stateStartTime > 600000UL) { // 10 minutes
        currentState = POST_10MIN_SEQUENCE;
        stateStartTime = millis();
        sequenceStep = 0;
        Serial.println("Starting Post 10-Min Sequence");
      }
      break;

    case POST_10MIN_SEQUENCE:
      handlePost10MinSequence();
      break;

    case K1_SEQUENCE:
      handleK1Sequence();
      break;
  }

  updateDisplay();
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  String status = "";
  long remaining = 0;

  switch (currentState) {
    case IDLE:
      status = "Hazir";
      break;
    case A1_SEQUENCE1:
      switch (sequenceStep) {
        case 0: status = "C1 Baslatiliyor"; break;
        case 1: status = "C1 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 2: status = "A1 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 3: status = "RELAY_B1 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
      }
      break;
    case A1_SEQUENCE2:
      switch (sequenceStep) {
        case 0: status = "B2 Baslatiliyor"; break;
        case 1: status = "B2 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 2: status = "RELAY_A2 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 3: status = "Kompresor Emme"; remaining = 30000 - (millis() - stateStartTime); break;
        case 4: status = "C2 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 5: status = "Bekleniyor"; remaining = 30000 - (millis() - stateStartTime); break;
        case 6: status = "C1 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 7: status = "Kompresor Basinç"; remaining = 30000 - (millis() - stateStartTime); break;
      }
      break;
    case WAITING_10MIN:
      status = "10 Dk Bekleniyor";
      remaining = 600000 - (millis() - stateStartTime);
      break;
    case POST_10MIN_SEQUENCE:
      switch (sequenceStep) {
        case 0: status = "Oda Cikis"; remaining = 3000 - (millis() - stateStartTime); break;
        case 1: status = "A1 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 2: status = "RELAY_B1 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
      }
      break;
    case K1_SEQUENCE:
      switch (sequenceStep) {
        case 0: status = "B2 Baslatiliyor"; break;
        case 1: status = "B2 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 2: status = "RELAY_A2 Aktif"; remaining = 3000 - (millis() - stateStartTime); break;
        case 3: status = "Kapatiliyor"; break;
      }
      break;
  }

  if (remaining < 0) remaining = 0;

  // Display status and remaining time
  display.print("Durum: ");
  display.print(status);
  if (remaining > 0) {
    display.println();
    if (remaining >= 60000) {
      int min = remaining / 60000;
      int sec = (remaining % 60000) / 1000;
      display.print("Kalan: ");
      display.print(min);
      display.print(" dk ");
      display.print(sec);
      display.print(" sn");
    } else {
      int sec = remaining / 1000;
      display.print("Kalan: ");
      display.print(sec);
      display.print(" sn");
    }
  }
  display.display();
}

void handleA1Sequence1() {
  switch (sequenceStep) {
    case 0:
      digitalWrite(C1, HIGH);
      Serial.println("C1 ON: Moving C piston up");
      beep();
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(C1, LOW);
        Serial.println("C1 OFF");
        beep();
        digitalWrite(A1_valve, HIGH);
        Serial.println("A1 ON: Moving A piston up");
        beep();
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(A1_valve, LOW);
        Serial.println("A1 OFF");
        beep();
        digitalWrite(RELAY_B1, HIGH);
        Serial.println("RELAY_B1 ON: Moving B piston forward");
        beep();
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(RELAY_B1, LOW);
        Serial.println("RELAY_B1 OFF");
        beep();
        currentState = IDLE;
        sequenceStep = 0;
        Serial.println("Returning to IDLE state");
      }
      break;
  }
}

void handleA1Sequence2() {
  switch (sequenceStep) {
    case 0:
      digitalWrite(B2, HIGH);
      Serial.println("B2 ON: Moving B piston backward");
      beep();
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(B2, LOW);
        Serial.println("B2 OFF");
        beep();
        digitalWrite(RELAY_A2, HIGH);
        Serial.println("RELAY_A2 ON: Moving A piston down");
        beep();
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(RELAY_A2, LOW);
        Serial.println("RELAY_A2 OFF");
        beep();
        digitalWrite(KOMP_KESICI1, HIGH);
        Serial.println("KOMP_KESICI1 ON: Closing main air inlet");
        beep();
        digitalWrite(KOMP_CIKIS1, HIGH);
        Serial.println("KOMP_CIKIS1 ON: Opening compressor outlet");
        beep();
        digitalWrite(KOMP_AC, HIGH);
        Serial.println("KOMP_AC ON: Starting compressor suction");
        beep();
        digitalWrite(KOMP_EMICI1, HIGH);
        Serial.println("KOMP_EMICI1 ON: Suctioning room air");
        beep();
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 30000) {
        digitalWrite(KOMP_EMICI1, LOW);
        Serial.println("KOMP_EMICI1 OFF");
        beep();
        digitalWrite(C2, HIGH);
        Serial.println("C2 ON: Moving C piston down");
        beep();
        sequenceStep = 4;
        stateStartTime = millis();
      }
      break;
    case 4:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(C2, LOW);
        Serial.println("C2 OFF");
        beep();
        sequenceStep = 5;
        stateStartTime = millis();
      }
      break;
    case 5:
      if (millis() - stateStartTime > 30000) {
        digitalWrite(C1, HIGH);
        Serial.println("C1 ON: Moving C piston up");
        beep();
        sequenceStep = 6;
        stateStartTime = millis();
      }
      break;
    case 6:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(C1, LOW);
        Serial.println("C1 OFF");
        beep();
        digitalWrite(KOMP_AC, LOW);
        Serial.println("KOMP_AC OFF: Stopping compressor");
        beep();
        digitalWrite(KOMP_CIKIS1, LOW);
        Serial.println("KOMP_CIKIS1 OFF: Closing compressor outlet");
        beep();
        digitalWrite(KOMP_KESICI1, LOW);
        Serial.println("KOMP_KESICI1 OFF: Opening main air inlet");
        beep();
        digitalWrite(KOMP_GIRIS1, HIGH);
        Serial.println("KOMP_GIRIS1 ON: Opening compressor air inlet");
        beep();
        digitalWrite(KOMP_BASINC1, HIGH);
        Serial.println("KOMP_BASINC1 ON: Pressurizing room air");
        beep();
        sequenceStep = 7;
        stateStartTime = millis();
      }
      break;
    case 7:
      if (millis() - stateStartTime > 30000) {
        digitalWrite(KOMP_BASINC1, LOW);
        Serial.println("KOMP_BASINC1 OFF");
        beep();
        digitalWrite(KOMP_GIRIS1, LOW);
        Serial.println("KOMP_GIRIS1 OFF");
        beep();
        currentState = WAITING_10MIN;
        stateStartTime = millis();
        sequenceStep = 0;
        Serial.println("Entering WAITING_10MIN state");
      }
      break;
  }
}

void handlePost10MinSequence() {
  switch (sequenceStep) {
    case 0:
      digitalWrite(ODA_CIKIS, HIGH);
      Serial.println("ODA_CIKIS ON: Venting room air");
      beep();
      sequenceStep = 1;
      stateStartTime = millis();
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(ODA_CIKIS, LOW);
        Serial.println("ODA_CIKIS OFF");
        beep();
        digitalWrite(A1_valve, HIGH);
        Serial.println("A1 ON: Moving A piston up");
        beep();
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(A1_valve, LOW);
        Serial.println("A1 OFF");
        beep();
        digitalWrite(RELAY_B1, HIGH);
        Serial.println("RELAY_B1 ON: Moving B piston forward");
        beep();
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(RELAY_B1, LOW);
        Serial.println("RELAY_B1 OFF");
        beep();
        currentState = IDLE;
        sequenceStep = 0;
        Serial.println("Returning to IDLE state");
      }
      break;
  }
}

void handleK1Sequence() {
  switch (sequenceStep) {
    case 0:
      digitalWrite(B2, HIGH);
      Serial.println("B2 ON: Moving B piston backward");
      beep();
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(B2, LOW);
        Serial.println("B2 OFF");
        beep();
        digitalWrite(RELAY_A2, HIGH);
        Serial.println("RELAY_A2 ON: Moving A piston down");
        beep();
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(RELAY_A2, LOW);
        Serial.println("RELAY_A2 OFF");
        beep();
        digitalWrite(compressorPower, LOW);
        Serial.println("Compressor power OFF");
        beep();
        digitalWrite(TAHLIYE, HIGH);
        Serial.println("TAHLIYE ON: Venting all air");
        beep();
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      // Keep TAHLIYE on, system shut down
      break;
  }
}
