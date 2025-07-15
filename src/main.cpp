#include <Arduino.h>
#include <MD_MAX72XX.h>

#define DATA_PIN 2
#define CLK_PIN  4
#define CS_PIN   16
#define AOUT   32

#define MAX_DEVICES 1
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW


MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
const uint8_t CAT[8] = {
  0b11111100,
  0b10000010,
  0b01010001,
  0b00000001,
  0b00000001,
  0b01010001,
  0b10000010,
  0b11111100
};

const uint8_t SMILE[8] = {
  0b00100000,
  0b01000100,
  0b00100010,
  0b00000010,
  0b00000010,
  0b00100010,
  0b01000100,
  0b00100000
};

const uint8_t SMILE2[8] = {
  0b01000000,
  0b10000000,
  0b01001100,
  0b00001010,
  0b00001010,
  0b01001100,
  0b10000000,
  0b01000000
};

const uint8_t HEART[8] = {
  0b00111000,
  0b01111100,
  0b01111110,
  0b00111111,
  0b00111111,
  0b01111110,
  0b01111100,
  0b00111000
};

const uint8_t SAD_FACE[8] = {
  0b00000000,
  0b00100010,
  0b01100100,
  0b00000100,
  0b00000100,
  0b01100100,
  0b00100010,
  0b00000000
};

const uint8_t ANGRY_FACE[8] = {
  0b00000000,
  0b01000010,
  0b00100100,
  0b00000100,
  0b00000100,
  0b00100100,
  0b01000010,
  0b00000000
};

enum EmoticonState {
  DRY,
  OPTIMAL,
  WET
};

EmoticonState currentState = OPTIMAL;
EmoticonState previousState = OPTIMAL;
const unsigned long MEASUREMENT_INTERVAL = 3000;
const int SAMPLE_COUNT = 10; // Number of samples to average
int readingHistory[SAMPLE_COUNT];
bool toggleFace = false;

void displayPattern(const uint8_t* pattern);
int calculateAverageMoisture();
EmoticonState determineMoistureState(int moistureValue);


void displayPattern(const uint8_t* pattern) {
  mx.clear();
  for (int row = 0; row < 8; row++) {
    mx.setRow(0, row, pattern[row]);
  }
}

int calculateAverageMoisture() {
  long sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sum += readingHistory[i];
  }
  return sum / SAMPLE_COUNT;
}

EmoticonState determineMoistureState(int moistureValue) {
  if (moistureValue > 2500) {
    return DRY;
  } else if (moistureValue < 900) {
    return WET;
  } else {
    return OPTIMAL;
  }
}

void setup() {
  Serial.begin(9600);
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 3);
  mx.clear();
}

void loop() {
   // Shift old readings to make space for the new one
  for (int i = SAMPLE_COUNT - 1; i > 0; i--) {
    readingHistory[i] = readingHistory[i - 1];
  }
  
  // Read new moisture value
  int newReading = analogRead(AOUT);
  readingHistory[0] = newReading;

  Serial.println(newReading);
  
  // Calculate average
  int averageMoisture = calculateAverageMoisture();
  
  // Determine moisture state
  currentState = determineMoistureState(averageMoisture);

   // If state hasn't changed, toggle pattern for variety
  if (currentState == previousState) {
    toggleFace = !toggleFace;
  } else {
    toggleFace = false;  // Reset toggle when state changes
  }

  switch (currentState) {
    case DRY:
      displayPattern(ANGRY_FACE);
      break;
    case WET:
      displayPattern(toggleFace ? CAT : SAD_FACE);
      break;
    case OPTIMAL:
      displayPattern(toggleFace ? SMILE2 : SMILE);
      break;
  }
  previousState = currentState;
  delay(MEASUREMENT_INTERVAL); // Delay for MEASUREMENT_INTERVAL
}