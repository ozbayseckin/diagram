// Arduino Mega code for pneumatic system control with 16 relays and buttons
// Controls compressor, valves, and sequences based on button presses

// Relay pin assignments (relays are active HIGH)
const int compressorPower = 2;
const int C1 = 3;
const int A1_valve = 4;
const int B1 = 5;
const int B2 = 6;
const int A2 = 7;
const int KOMP_KESICI1 = 8;
const int KOMP_CIKIS1 = 9;
const int KOMP_AC = 10;
const int KOMP_EMICI1 = 11;
const int C2 = 12;
const int KOMP_GIRIS1 = 13;
const int KOMP_BASINC1 = 14;
const int ODA_CIKIS = 15;
const int TAHLİYE = 16;

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
  pinMode(B1, OUTPUT);
  pinMode(B2, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(KOMP_KESICI1, OUTPUT);
  pinMode(KOMP_CIKIS1, OUTPUT);
  pinMode(KOMP_AC, OUTPUT);
  pinMode(KOMP_EMICI1, OUTPUT);
  pinMode(C2, OUTPUT);
  pinMode(KOMP_GIRIS1, OUTPUT);
  pinMode(KOMP_BASINC1, OUTPUT);
  pinMode(ODA_CIKIS, OUTPUT);
  pinMode(TAHLİYE, OUTPUT);

  // Set button pins as inputs with pullup
  pinMode(buttonA1, INPUT_PULLUP);
  pinMode(buttonK1, INPUT_PULLUP);

  // On power on, activate compressor power
  digitalWrite(compressorPower, HIGH);

  // Ensure all relays start LOW (off)
  digitalWrite(C1, LOW);
  digitalWrite(A1_valve, LOW);
  digitalWrite(B1, LOW);
  digitalWrite(B2, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(KOMP_KESICI1, LOW);
  digitalWrite(KOMP_CIKIS1, LOW);
  digitalWrite(KOMP_AC, LOW);
  digitalWrite(KOMP_EMICI1, LOW);
  digitalWrite(C2, LOW);
  digitalWrite(KOMP_GIRIS1, LOW);
  digitalWrite(KOMP_BASINC1, LOW);
  digitalWrite(ODA_CIKIS, LOW);
  digitalWrite(TAHLİYE, LOW);
}

void loop() {
  // Read button states with debouncing
  bool a1Current = (digitalRead(buttonA1) == LOW);
  bool k1Current = (digitalRead(buttonK1) == LOW);

  if (a1Current && !a1WasPressed) {
    a1Pressed = true;
    a1WasPressed = true;
  } else if (!a1Current) {
    a1WasPressed = false;
  }

  if (k1Current && !k1WasPressed) {
    k1Pressed = true;
    k1WasPressed = true;
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
        } else if (a1PressCount == 2) {
          currentState = A1_SEQUENCE2;
          stateStartTime = millis();
          sequenceStep = 0;
          a1PressCount = 0;
        }
      }
      if (k1Pressed) {
        k1Pressed = false;
        currentState = K1_SEQUENCE;
        stateStartTime = millis();
        sequenceStep = 0;
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
      }
      break;

    case POST_10MIN_SEQUENCE:
      handlePost10MinSequence();
      break;

    case K1_SEQUENCE:
      handleK1Sequence();
      break;
  }
}

void handleA1Sequence1() {
  switch (sequenceStep) {
    case 0:
      digitalWrite(C1, HIGH);
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(C1, LOW);
        digitalWrite(A1_valve, HIGH);
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(A1_valve, LOW);
        digitalWrite(B1, HIGH);
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(B1, LOW);
        currentState = IDLE;
        sequenceStep = 0;
      }
      break;
  }
}

void handleA1Sequence2() {
  switch (sequenceStep) {
    case 0:
      digitalWrite(B2, HIGH);
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(B2, LOW);
        digitalWrite(A2, HIGH);
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(A2, LOW);
        digitalWrite(KOMP_KESICI1, HIGH);
        digitalWrite(KOMP_CIKIS1, HIGH);
        digitalWrite(KOMP_AC, HIGH);
        digitalWrite(KOMP_EMICI1, HIGH);
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 30000) { // 30 seconds
        digitalWrite(KOMP_EMICI1, LOW);
        digitalWrite(C2, HIGH);
        sequenceStep = 4;
        stateStartTime = millis();
      }
      break;
    case 4:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(C2, LOW);
        sequenceStep = 5;
        stateStartTime = millis();
      }
      break;
    case 5:
      if (millis() - stateStartTime > 30000) { // 30 seconds wait
        digitalWrite(C1, HIGH);
        sequenceStep = 6;
        stateStartTime = millis();
      }
      break;
    case 6:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(C1, LOW);
        digitalWrite(KOMP_AC, LOW);
        digitalWrite(KOMP_CIKIS1, LOW);
        digitalWrite(KOMP_KESICI1, LOW);
        digitalWrite(KOMP_GIRIS1, HIGH);
        digitalWrite(KOMP_BASINC1, HIGH);
        sequenceStep = 7;
        stateStartTime = millis();
      }
      break;
    case 7:
      if (millis() - stateStartTime > 30000) { // 30 seconds
        digitalWrite(KOMP_BASINC1, LOW);
        digitalWrite(KOMP_GIRIS1, LOW);
        currentState = WAITING_10MIN;
        stateStartTime = millis();
        sequenceStep = 0;
      }
      break;
  }
}

void handlePost10MinSequence() {
  switch (sequenceStep) {
    case 0:
      digitalWrite(ODA_CIKIS, HIGH);
      sequenceStep = 1;
      break;
    case 1:
      // Assuming immediate after activation, but if delay needed, add timer
      digitalWrite(ODA_CIKIS, LOW);
      digitalWrite(A1_valve, HIGH);
      sequenceStep = 2;
      stateStartTime = millis();
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(A1_valve, LOW);
        digitalWrite(B1, HIGH);
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(B1, LOW);
        currentState = IDLE;
        sequenceStep = 0;
      }
      break;
  }
}

void handleK1Sequence() {
  switch (sequenceStep) {
    case 0:
      digitalWrite(B2, HIGH);
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(B2, LOW);
        digitalWrite(A2, HIGH);
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        digitalWrite(A2, LOW);
        digitalWrite(compressorPower, LOW);
        digitalWrite(TAHLİYE, HIGH);
        sequenceStep = 3;
        // Assuming immediate shutdown, no further steps
        stateStartTime = millis();
      }
      break;
    case 3:
      // Keep TAHLİYE on, system shut down
      // Could add timer if needed to turn off TAHLİYE
      break;
  }
}
