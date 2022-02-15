#include <Arduino.h>
#include <NewPing.h>
// float switches
const byte floatOne = 22;
const byte floatTwo = 26;
const byte floatThree = 30;

// LED indicators
const byte ledOne = 36;
const byte ledTwo = 38;
const byte ledThree = 40;

// Ultrasonic Sensor
const byte TRIG = 10;
const byte ECHO = 9;
const int MAX_DISTANCE = 450; // 450 cm

// state variables
bool hasLevelUpdated = false;
int previousLevel = 0;
int currentLevel = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

NewPing sonar(TRIG, ECHO, MAX_DISTANCE);
const int distanceOne = 50;
const int distanceTwo = 40;
const int distanceThree = 30;
const int iteration = 5;

int getLevel()
{
  // without confirmation from ultrasonic
  if (digitalRead(floatThree) == LOW)
    return 3;
  if (digitalRead(floatTwo) == LOW)
    return 2;
  if (digitalRead(floatOne) == LOW)
    return 1;

  return 0;
}

int getWaterLevelBySonar()
{
  unsigned long readTimeMS = sonar.ping_median(iteration) + 3;

  int waterDistance = sonar.convert_cm(readTimeMS);

  if (waterDistance <= distanceThree)
    return 3;
  if (waterDistance <= distanceTwo)
    return 2;
  if (waterDistance <= distanceOne)
    return 1;

  return 0;
}

void resetLEDs()
{
  digitalWrite(ledOne, LOW);
  digitalWrite(ledTwo, LOW);
  digitalWrite(ledThree, LOW);
}

void manageLEDIndicators(int level)
{
  resetLEDs();

  switch (level)
  {
  case 1:
    digitalWrite(ledOne, HIGH);
    break;
  case 2:
    digitalWrite(ledTwo, HIGH);
    break;
  case 3:
    digitalWrite(ledThree, HIGH);
    break;
  default:
    resetLEDs();
    break;
  }
}

void sendAlert(int level, int waterDistance)
{

  if (level == 1 && waterDistance == 1)
  {
    Serial.println(F("Water level: 1"));
    manageLEDIndicators(1);
    // send gsm message here , update LCD
    return;
  }

  if (level == 2 && waterDistance == 2)
  {
    Serial.println(F("Water level: 2"));
    manageLEDIndicators(2);
    // send gsm message here , update LCD
    return;
  }

  if (level == 3 && waterDistance == 3)
  {
    Serial.println(F("Water level: 3"));
    manageLEDIndicators(3);
    // send gsm message here , update LCD
    return;
  }

  Serial.println(F("No flood."));
  resetLEDs();
  return;
}

void setup()
{
  pinMode(floatOne, INPUT_PULLUP);
  pinMode(floatTwo, INPUT_PULLUP);
  pinMode(floatThree, INPUT_PULLUP);

  pinMode(ledOne, OUTPUT);
  pinMode(ledTwo, OUTPUT);
  pinMode(ledThree, OUTPUT);

  Serial.begin(9600);

  Serial.println("Hello! CHANGE THIS TO YOUR THESIS TITLE");
}

void loop()
{

  int level = getLevel();
  int waterDistance = getWaterLevelBySonar();

  Serial.println(waterDistance);

  if (level != previousLevel)
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {

    if (level != currentLevel)
    {
      currentLevel = level;
    }

    sendAlert(currentLevel, waterDistance);
  }
  previousLevel = level;
}
