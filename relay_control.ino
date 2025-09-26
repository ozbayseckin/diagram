// Arduino Mega code to control 16 relays sequentially
// Each relay turns on for 1 second, then off, then moves to the next relay

const int relayPins[16] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}; // Digital pins for relays

void setup() {
  // Initialize all relay pins as outputs and set them to LOW (off)
  for (int i = 0; i < 16; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }
}

void loop() {
  // Sequentially turn each relay on for 1 second, then off
  for (int i = 0; i < 16; i++) {
    digitalWrite(relayPins[i], HIGH); // Turn relay on
    delay(1000);                     // Wait 1 second
    digitalWrite(relayPins[i], LOW);  // Turn relay off
    // No delay between relays, proceeds immediately to next
  }
}
