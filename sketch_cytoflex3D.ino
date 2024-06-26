#include <Servo.h>

Servo myservo;  // Create servo object to control a servo

int potPin = A0;  // Analog pin used to connect the potentiometer
int lastPotValue = 0;  // Variable to store the last potentiometer value for change detection
int angle = 0;  // Variable to store the angle for the servo

const int limitSwitchPin = 2;  // Pin number for the limit switch
const int ledPin = 13;        // Pin number for the LED

const char* tubeLabels[] = {"TubeA", "TubeB", "Tube1", "Tube2", "Tube3", "Tube4"};
int tubePositions[] = {0, 20, 40, 60, 80, 100}; // Relative positions in degrees
const char* accessSequence[] = {"TubeA", "TubeB", "Tube1", "TubeA", "TubeB", "Tube2"}; // Access sequence

int pos_ini = 0;  // Initial position of the servo
int positions[sizeof(accessSequence) / sizeof(accessSequence[0])]; // Positions based on sequence
int numPositions = sizeof(accessSequence) / sizeof(accessSequence[0]); // Number of positions

int currentIndex = 0;  // Current index in the sequence
int switchState = 0;  // Current state of the limit switch
bool isClicked = false; // Flag to handle switch logic

void setup() {
  myservo.attach(9);
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(limitSwitchPin, INPUT_PULLUP);

  for (int i = 0; i < numPositions; i++) {
    for (int j = 0; j < sizeof(tubeLabels) / sizeof(tubeLabels[0]); j++) {
      if (strcmp(accessSequence[i], tubeLabels[j]) == 0) {
        positions[i] = tubePositions[j] + pos_ini;
        break;
      }
    }
  }
  myservo.write(pos_ini);
  Serial.println("Starting at initial position: " + String(pos_ini));
}
void loop() {
  int potValue = analogRead(potPin);
  int delta = abs(potValue - lastPotValue);  // Calculate the change since the last read

  // Update only if the change is greater than a certain threshold (e.g., 5 units)
  if (delta > 5) {
    pos_ini = map(potValue, 0, 1023, 0, 180);  // Map the new position based on the potentiometer value
    lastPotValue = potValue;  // Update the last known potentiometer value

    if (pos_ini > 0 && pos_ini < 180) {
      myservo.write(pos_ini);  // Update the servo position if within valid range
      Serial.print("Potentiometer: ");
      Serial.print(potValue);
      Serial.print(" => pos_ini: ");
      Serial.println(pos_ini);
    }
  }

  switchState = digitalRead(limitSwitchPin);
  
  if (switchState == LOW && !isClicked) {
    digitalWrite(ledPin, HIGH);  // Turn LED on to indicate the switch is pressed
    moveToEppendorf(currentIndex);  // Move to the specified position
    currentIndex++;
    if (currentIndex >= numPositions) {
      currentIndex = 0;
    }
    isClicked = true;  // Set the flag as clicked
    delay(10);  // Debounce delay
  } else if (switchState == HIGH && isClicked) {
    digitalWrite(ledPin, LOW);  // Turn LED off to indicate the switch is no longer pressed
    isClicked = false;  // Reset the click status
    delay(10);  // Debounce delay
  }
}

void moveToEppendorf(int index) {
  if (index >= 0 && index < numPositions) {
    int targetPosition = positions[index] + pos_ini;
    int currentPosition = myservo.read(); // Read current position of the servo
    
    Serial.println("Moving to " + String(accessSequence[index]) + " (sequence " + String(index) + ", angle=" + String(currentPosition) + " -> " + String(targetPosition) + ")");
    
    if (currentPosition < targetPosition) {
      for (int pos = currentPosition; pos <= targetPosition; pos++) {
        myservo.write(pos);
        delay(10); // Small delay to slow down the movement
      }
    } else {
      for (int pos = currentPosition; pos >= targetPosition; pos--) {
        myservo.write(pos);
        delay(10); // Small delay to slow down the movement
      }
    }
  } else {
    Serial.println("Invalid position index"); // Optional: Output error message if index is invalid
  }
}


