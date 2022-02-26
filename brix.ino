/**
 *  AUTHOR: JEFFREY E. SEGOVIA, ECE, ECT
 *  DESCRIPTION: FLOOD STUFF
 *  DATE: February 26, 2021
 *
 **/

#include <Arduino.h>
#include <NewPing.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

// GSM PINS
const byte GSM_TX = 10;                  // 5VT
const byte GSM_RX = 11;                  // 5VR
String priorityNumber = "+639385278444"; // subject to change ** PRIORITY

// float switches
const byte floatOne = 5;
const byte floatTwo = 6;
const byte floatThree = 7;

// LED indicators
const byte ledOne = A0;
const byte ledTwo = A1;
const byte ledThree = A2;

// Ultrasonic Sensor
const byte TRIG = 3;
const byte ECHO = 2;
const int MAX_DISTANCE = 450; // 450 cm

// state variables
bool hasLevelUpdated = false;
int previousLevel = 0;
int currentLevel = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

const int distanceOne = 50;
const int distanceTwo = 40;
const int distanceThree = 30;
const int iteration = 5;

const byte scrollDelay = 200;
const String THESIS_TITLE = "Water Level Monitoring and Flood Warning System";
// instances
NewPing sonar(TRIG, ECHO, MAX_DISTANCE);
SoftwareSerial gsm(GSM_TX, GSM_RX);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// custom function for handling delays
bool jeffDelay(unsigned long interval)
{
  unsigned long now = millis();
  while (!((millis() - now) > interval))
    ;
  return true;
}

void showWelcomeMsg()
{
  lcd.clear();

  int i = 0;
  int l = THESIS_TITLE.length() - 16;
  if (!jeffDelay(scrollDelay * 2))
    return;
  while (i <= l)
  {
    lcd.setCursor(0, 0);
    String toShow = THESIS_TITLE.substring(i, i + 16);
    lcd.print(toShow);
    i = i + 1;
    if (!jeffDelay(scrollDelay))
      return;
  }
  if (!jeffDelay(scrollDelay * 2))
    return;
  String str = "Welcome!";
  lcd.clear();
  lcd.setCursor((16 - str.length()) / 2, 0);
  lcd.print(str);
  str = "Initializing...";
  lcd.clear();
  lcd.setCursor((16 - str.length()) / 2, 1);
  lcd.print(str);
  Serial.println(str);
  if (!jeffDelay(1000))
    return;
  lcd.clear();
}

void jeffSendTxt(String sms)
{
  gsm.println("AT+CMGF=1");
  if (!jeffDelay(1000))
    return;
  gsm.println("AT+CMGS=\"" + priorityNumber + "\"\r");
  if (!jeffDelay(1000))
    return;
  gsm.println(sms);
  if (!jeffDelay(1000))
    return;
  gsm.println((char)26); // ASCII code of CTRL+Z
  if (!jeffDelay(1000))
    return;
  Serial.println("Message has been sent!");
  if (!jeffDelay(500))
    return;
}

void jeffSendToOne(String number, String msg)
{
  gsm.println("AT+CMGF=1");
  if (!jeffDelay(1000))
    return;
  gsm.println("AT+CMGS=\"" + number + "\"\r");
  if (!jeffDelay(1000))
    return;
  gsm.println(msg);
  if (!jeffDelay(1000))
    return;
  gsm.println((char)26); // ASCII code of CTRL+Z
  if (!jeffDelay(1000))
    return;
  Serial.println("Message has been sent!");
  if (!jeffDelay(500))
    return;
}

void initiateCall()
{

  String callCommand = "ATD" + priorityNumber + ";";
  gsm.println(callCommand);
  if (!jeffDelay(100))
    return;
  gsm.println();
  if (!jeffDelay(20000))
    return;           // wait for 25 seconds …
  gsm.println("ATH"); // Drop the phone
  if (!jeffDelay(300))
    return;
}

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

void showLCDMessage(int level)
{
  String msg = level == 0 ? "Status: No Flood" : "Status: Flooding";
  lcd.setCursor(0, 0);
  lcd.print(msg);

  if (level != 0)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level " + String(level));
  }
}

void sendAlert(int level, int waterDistance)
{

  if (level == 1 && waterDistance == 1)
  {
    Serial.println(F("Water level: 1"));
    manageLEDIndicators(1);
    // show LCD Message
    showLCDMessage(1);
    jeffSendTxt("FLOOD ALERT: LEVEL 1");

    return;
  }

  if (level == 2 && waterDistance == 2)
  {
    Serial.println(F("Water level: 2"));
    manageLEDIndicators(2);
    // show LCD Message
    showLCDMessage(2);
    jeffSendTxt("FLOOD ALERT: LEVEL 2");

    return;
  }

  if (level == 3 && waterDistance == 3)
  {
    Serial.println(F("Water level: 3"));
    manageLEDIndicators(3);
    // show LCD Message
    showLCDMessage(3);
    jeffSendTxt("FLOOD ALERT: LEVEL 3");

    return;
  }

  Serial.println(F("No flood."));
  // show LCD Message
  showLCDMessage(0);
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

  // Comms
  Serial.begin(9600);
  gsm.begin(9600);

  // LCD
  // lcd
  lcd.init();
  lcd.backlight();

  Serial.println(THESIS_TITLE);

  showWelcomeMsg();

  jeffSendTxt("SYSTEM RESTARTED");
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
