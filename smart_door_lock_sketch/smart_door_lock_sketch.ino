#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 2

// servo positions
#define UNLOCKED_POS 15
#define LOCKED_POS 90

// authorized fobs
const char* allowedUIDs[] = {"73:AC:65:1C"};
const int numUIDs = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

unsigned long autoLockTime = 60000;
unsigned long dayInterval = 300;
unsigned long nightInterval = 2000;
unsigned long rescanDelay = 2000;

// night mode hours (24hr)
int nightStart = 23;
int nightEnd = 6;

bool unlocked = false;
unsigned long unlockedAt = 0;
unsigned long lastScan = 0;

WebServer server(80);
MFRC522 rfid(SS_PIN, RST_PIN);

bool isNight() {
    struct tm t;
    if (!getLocalTime(&t)) return false;
    int h = t.tm_hour;
    return (h >= nightStart || h < nightEnd);
}

// raw pwm since esp32servo conflicts with spi
void moveServo(int angle) {
  int pw = map(angle, 0, 180, 500, 2500);
    for (int i = 0; i < 50; i++) {
      digitalWrite(SERVO_PIN, HIGH);
      delayMicroseconds(pw);
        digitalWrite(SERVO_PIN, LOW);
        delayMicroseconds(20000 - pw);
    }
}

void unlockDoor(const char* how) {
    if (unlocked) return;
    unlocked = true;
  unlockedAt = millis();
    moveServo(UNLOCKED_POS);
  Serial.print("unlocked via ");
    Serial.println(how);
}

void lockDoor(const char* how) {
  unlocked = false;
    moveServo(LOCKED_POS);
  Serial.print("locked via ");
  Serial.println(how);
}

// handles nfc shortcut request
void handleUnlock() {
    unlockDoor("wifi");
    server.send(200, "text/plain", "unlocked");
}

void handleStatus() {
  server.send(200, "text/plain", unlocked ? "unlocked" : "locked");
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) uid += ":";
  }
  uid.toUpperCase();
    return uid;
}

void checkUID(String uid) {
    for (int i = 0; i < numUIDs; i++) {
    if (uid == String(allowedUIDs[i])) {
        unlockDoor("rfid");
        return;
      }
    }
  Serial.println("denied: " + uid);
}

void setup() {
  Serial.begin(9600);
    delay(2000);

  setCpuFrequencyMhz(160);

  // init servo in locked position
    pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, LOW);
    delay(500);
  moveServo(LOCKED_POS);

  SPI.begin();
    rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_33dB);

  // connect to wifi
    WiFi.begin(ssid, password);
  Serial.print("connecting to wifi");
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
      Serial.print(".");
  }
  Serial.println("");
    Serial.println(WiFi.localIP());

  // sync time for night mode
  configTime(-28800, 3600, "pool.ntp.org");

    server.on("/unlock", HTTP_GET, handleUnlock);
  server.on("/status", HTTP_GET, handleStatus);
    server.begin();

  Serial.println("ready");
}

void loop() {
    server.handleClient();

  // auto lock after 60 seconds
  if (unlocked && millis() - unlockedAt >= autoLockTime) {
      lockDoor("auto");
  }

    unsigned long interval = isNight() ? nightInterval : dayInterval;

  if (millis() - lastScan < rescanDelay) {
      delay(interval);
    return;
  }

  // scan for rfid
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    lastScan = millis();
      String uid = getUID();
    Serial.println("scanned: " + uid);
      checkUID(uid);
    rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
  }

    delay(interval);
}