#include <Adafruit_Fingerprint.h>

#if (defined(_AVR_) || defined(ESP8266)) && !defined(_AVR_ATmega2560_)
#include <SoftwareSerial.h>
SoftwareSerial mySerial(6, 7);
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Hardware pins
const int ledGreen = 12;
const int ledRed = 11;
const int ledBlue = 13;
const int buzzer = 3;
const int shakeSensor = 10;
const int relay = 8;

// Timing control
unsigned long doorUnlockTime = 0;
bool doorIsUnlocked = false;

unsigned long shakeAlertStart = 0;
bool shakeAlertActive = false;
int shakeAlertStep = 0;
unsigned long lastShakeBlink = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(shakeSensor, INPUT);

  digitalWrite(relay, LOW);

  Serial.println("Initializing fingerprint sensor...");
  finger.begin(57600);
  delay(5);

  while (!finger.verifyPassword()) {
    Serial.println("Waiting for fingerprint sensor...");
    delay(1000);
  }

  Serial.println("Fingerprint sensor found.");
  finger.getTemplateCount();
}

void loop() {
  unsigned long currentMillis = millis();

  if (digitalRead(shakeSensor) == HIGH && !shakeAlertActive) {
    startShakeAlert();
  }

  if (shakeAlertActive) {
    updateShakeAlert(currentMillis);
  }

  if (!doorIsUnlocked) {
    getFingerprintID();
  } else {
    if (currentMillis - doorUnlockTime >= 3000) {
      lockDoor();
    }
  }
}

// Unlock door for 3 seconds
void accessGranted() {
  Serial.println("Access Granted");
  digitalWrite(ledGreen, HIGH);
  digitalWrite(relay, HIGH);
  tone(buzzer, 1000, 200);

  doorUnlockTime = millis();
  doorIsUnlocked = true;
}

void lockDoor() {
  digitalWrite(ledGreen, LOW);
  digitalWrite(relay, LOW);
  doorIsUnlocked = false;
}

// Access denied alert
void accessDenied() {
  Serial.println("Access Denied");
  digitalWrite(ledRed, HIGH);
  tone(buzzer, 500, 200);
  delay(200);
  digitalWrite(ledRed, LOW);
  delay(200);
  tone(buzzer, 500, 200);
  delay(200);
  digitalWrite(ledRed, LOW);
}

// Shake alert: Start
void startShakeAlert() {
  Serial.println("!!! Shake Detected !!!");
  shakeAlertStart = millis();
  shakeAlertActive = true;
  shakeAlertStep = 0;
  lastShakeBlink = 0;
}

// Shake alert: Progress using millis
void updateShakeAlert(unsigned long currentMillis) {
  if (shakeAlertStep >= 10) {
    digitalWrite(ledRed, LOW);
    noTone(buzzer);
    shakeAlertActive = false;
    return;
  }

  if (currentMillis - lastShakeBlink >= 200) {
    if (shakeAlertStep % 2 == 0) {
      digitalWrite(ledRed, HIGH);
      tone(buzzer, 2000);
    } else {
      digitalWrite(ledRed, LOW);
      noTone(buzzer);
    }
    lastShakeBlink = currentMillis;
    shakeAlertStep++;
  }
}

// Fingerprint scan
uint8_t getFingerprintID() {
  digitalWrite(ledBlue, HIGH);
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    digitalWrite(ledBlue, LOW);
    return p;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    digitalWrite(ledBlue, LOW);
    return p;
  }

  p = finger.fingerSearch();
  digitalWrite(ledBlue, LOW);

  if (p == FINGERPRINT_OK) {
    accessGranted();
  } else {
    accessDenied();
  }

  return p;
}
