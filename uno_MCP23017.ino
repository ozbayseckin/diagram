#include <Wire.h>
#include <Adafruit_MCP23X17.h>

// MCP23017 I2C address (default, A0-A2 grounded)
#define MCP23017_ADDRESS 0x20

// Create MCP23017 instance
Adafruit_MCP23X17 mcp;

// Relay indices for MCP23017 GPIO pins (0 to 15)
const int compressorPower = 0;  // GPA0
const int C1 = 1;               // GPA1
const int A1_valve = 2;         // GPA2
const int B1 = 3;               // GPA3
const int B2 = 4;               // GPA4
const int A2 = 5;               // GPA5
const int KOMP_KESICI1 = 6;     // GPA6
const int KOMP_CIKIS1 = 7;      // GPA7
const int KOMP_AC = 8;          // GPB0
const int KOMP_EMICI1 = 9;      // GPB1
const int C2 = 10;              // GPB2
const int KOMP_GIRIS1 = 11;     // GPB3
const int KOMP_BASINC1 = 12;    // GPB4
const int ODA_CIKIS = 13;       // GPB5
const int TAHLİYE = 14;         // GPB6
// Note: GPB7 (pin 15) is unused in this setup

// Button pins (interrupt-capable on Uno)
const int buttonA1 = 2;  // Pin 2
const int buttonK1 = 3;  // Pin 3

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
  // Initialize I2C and MCP23017
  Wire.begin();
  if (!mcp.begin_I2C(MCP23017_ADDRESS)) {
    // If MCP23017 initialization fails, halt (add Serial debug if needed)
    while (1);
  }

  // Set all MCP23017 pins as outputs
  for (int i = 0; i < 16; i++) {
    mcp.pinMode(i, OUTPUT);
    mcp.digitalWrite(i, LOW); // Ensure all relays start OFF
  }

  // Set button pins as inputs with pullup
  pinMode(buttonA1, INPUT_PULLUP);
  pinMode(buttonK1, INPUT_PULLUP);

  // On power on, activate compressor power
  mcp.digitalWrite(compressorPower, HIGH);
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
      mcp.digitalWrite(C1, HIGH);
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(C1, LOW);
        mcp.digitalWrite(A1_valve, HIGH);
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(A1_valve, LOW);
        mcp.digitalWrite(B1, HIGH);
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(B1, LOW);
        currentState = IDLE;
        sequenceStep = 0;
      }
      break;
  }
}

void handleA1Sequence2() {
  switch (sequenceStep) {
    case 0:
      mcp.digitalWrite(B2, HIGH);
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(B2, LOW);
        mcp.digitalWrite(A2, HIGH);
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(A2, LOW);
        mcp.digitalWrite(KOMP_KESICI1, HIGH);
        mcp.digitalWrite(KOMP_CIKIS1, HIGH);
        mcp.digitalWrite(KOMP_AC, HIGH);
        mcp.digitalWrite(KOMP_EMICI1, HIGH);
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 30000) { // 30 seconds
        mcp.digitalWrite(KOMP_EMICI1, LOW);
        mcp.digitalWrite(C2, HIGH);
        sequenceStep = 4;
        stateStartTime = millis();
      }
      break;
    case 4:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(C2, LOW);
        sequenceStep = 5;
        stateStartTime = millis();
      }
      break;
    case 5:
      if (millis() - stateStartTime > 30000) { // 30 seconds wait
        mcp.digitalWrite(C1, HIGH);
        sequenceStep = 6;
        stateStartTime = millis();
      }
      break;
    case 6:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(C1, LOW);
        mcp.digitalWrite(KOMP_AC, LOW);
        mcp.digitalWrite(KOMP_CIKIS1, LOW);
        mcp.digitalWrite(KOMP_KESICI1, LOW);
        mcp.digitalWrite(KOMP_GIRIS1, HIGH);
        mcp.digitalWrite(KOMP_BASINC1, HIGH);
        sequenceStep = 7;
        stateStartTime = millis();
      }
      break;
    case 7:
      if (millis() - stateStartTime > 30000) { // 30 seconds
        mcp.digitalWrite(KOMP_BASINC1, LOW);
        mcp.digitalWrite(KOMP_GIRIS1, LOW);
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
      mcp.digitalWrite(ODA_CIKIS, HIGH);
      sequenceStep = 1;
      break;
    case 1:
      mcp.digitalWrite(ODA_CIKIS, LOW);
      mcp.digitalWrite(A1_valve, HIGH);
      sequenceStep = 2;
      stateStartTime = millis();
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(A1_valve, LOW);
        mcp.digitalWrite(B1, HIGH);
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(B1, LOW);
        currentState = IDLE;
        sequenceStep = 0;
      }
      break;
  }
}

void handleK1Sequence() {
  switch (sequenceStep) {
    case 0:
      mcp.digitalWrite(B2, HIGH);
      sequenceStep = 1;
      break;
    case 1:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(B2, LOW);
        mcp.digitalWrite(A2, HIGH);
        sequenceStep = 2;
        stateStartTime = millis();
      }
      break;
    case 2:
      if (millis() - stateStartTime > 3000) {
        mcp.digitalWrite(A2, LOW);
        mcp.digitalWrite(compressorPower, LOW);
        mcp.digitalWrite(TAHLİYE, HIGH);
        sequenceStep = 3;
        stateStartTime = millis();
      }
      break;
    case 3:
      // Keep TAHLİYE on, system shut down
      break;
  }
}