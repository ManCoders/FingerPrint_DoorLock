#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);  // RX, TX
Adafruit_Fingerprint finger(&mySerial);

const int relay = 4;
const int ledRed = 11;
const int ledBlue = 12;
const int ledGreen = 10;
const int vibrator = 6;

#define buzzer 5

void setup() {
  Serial.begin(9600);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relay, OUTPUT);
  while (!Serial)
    ;

  finger.begin(57600);
  delay(1000);


  while (!finger.verifyPassword()) {
    Serial.println("Waiting for fingerprint sensor...");
    ledAlert();
    delay(1000);  // Wait a bit before retrying
  }

  Serial.println("Found fingerprint sensor!");
  finger.LEDcontrol(0x00);

  enrollFingerprint();
}

void loop() {
  getFingerprintID();
  delay(100);  // Short delay to prevent rapid polling
}

bool enrollFingerprint() {
 digitalWrite(ledRed, HIGH);
  Serial.println("Finger Registering..");

  // Clear the fingerprint database
  if (finger.emptyDatabase() == FINGERPRINT_OK) {
    Serial.println("[Success] Reset Db..");
  } else {
    Serial.println("[Failed] Reset Db..");
    return;
  }

  finger.LEDcontrol(0x01);
  ledGranted();
  Serial.println("Place your finger...");

  // First scan
  while (finger.getImage() != FINGERPRINT_OK)
    ;
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("[Failed] Convert First Image");
    return false;
  }
  Serial.println("[Success] First image captured.");
  delay(1000);

  Serial.println("Remove finger...");
  finger.LEDcontrol(0x00);
  delay(2000);
  finger.LEDcontrol(0x01);
  Serial.println("Place the same finger again...");


  // Wait for finger removal then second scan
  while (finger.getImage() != FINGERPRINT_NOFINGER)
    ;
  while (finger.getImage() != FINGERPRINT_OK)
    ;
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("[Failed] Convert Second Image");
    return false;
  }
  Serial.println("[Success] Second image captured.");


  // Create and store model
  if (finger.createModel() == FINGERPRINT_OK) {
    if (finger.storeModel(1) == FINGERPRINT_OK) {
      Serial.println("[Success] Fingerprint enrolled to ID #1");
    } else {
      Serial.println("[Failed] Store Model");
    }
  } else {
    Serial.println("[Failed] Create Model, Please Try Again!");
    enrollFingerprint();
    return false;
  }

  finger.LEDcontrol(0x00);
  delay(2000);
  return true;
}

void getFingerprintID() {
  digitalWrite(ledRed, LOW);
  digitalWrite(ledBlue, HIGH);
  finger.LEDcontrol(0x01);

  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return;

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Found ID #");
    Serial.println(finger.fingerID);
    Serial.print("Confidence: ");
    Serial.println(finger.confidence);
    finger.LEDcontrol(0x00);
    digitalWrite(relay, HIGH);
    playAccessTone();
    ledGranted();
    delay(3000);
    digitalWrite(relay, LOW);
  } else {
    
    playAlertTone();
    Serial.println("Finger not recognized.");
  }
  finger.LEDcontrol(0x00);
}

void ledGranted() {
  for (int i = 0; i < 3; i++) {  // Blink LED 3 times
    digitalWrite(ledGreen, HIGH);
    delay(200);  // LED on for 200ms
    digitalWrite(ledGreen, LOW);
    delay(200);  // LED off for 200ms
  }
}

void ledAlert() {
  for(int i = 1; i<3; i++){
    digitalWrite(ledRed, HIGH);
    delay(200);
    digitalWrite(ledRed, LOW);
    delay(200);
  }
}


void playAccessTone() {
  ledAlert();
  tone(buzzer, 1500);  // Play 1.5kHz tone
  delay(200);          // For 200 milliseconds
  noTone(buzzer);      // Stop tone
  delay(100);
  tone(buzzer, 2000);  // Then a higher tone (like a "ding-ding")
  delay(200);
  noTone(buzzer);
}

void playAlertTone() {
  ledAlert();
  for (int i = 0; i < 3; i++) {  // Beep 3 times
    tone(buzzer, 1000);          // 1kHz tone
    delay(200);                  // Play for 200ms
    noTone(buzzer);
    delay(100);  // Wait between beeps
  }
}
