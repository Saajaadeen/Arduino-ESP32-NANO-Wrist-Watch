#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <RTClib.h>
#include <TinyGPS++.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGPSPlus gps;
RTC_DS3231 rtc;

const int xPin = A0;
const int yPin = A1;
const int zPin = A2;

const int ledPin = 2;
const float referenceVoltage = 3.6;
const float sensitivity = 0.068;

void readGPS();
void readXYZ();
void readRTC();
void readDisplay();

void setup() {
  Wire.begin(D4, D5);
  rtc.begin();
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 10, 9);
  Serial.println("[+] Board startup Complete");
  delay(2000);
  readDisplay();
}

void loop() {
  readGPS();
  readXYZ();
  readRTC();
}

void readGPS() {
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    gps.encode(c);
  }

  if (gps.location.isValid() && gps.satellites.value() > 0) {
    int satellites = gps.satellites.value();
    float longitude = gps.location.lng();
    float latitude = gps.location.lat();
    Serial.print("[+] GPS ");
    Serial.print(" - Sats: " + String(satellites));
    Serial.print(" - Lng: " + String(longitude, 6));
    Serial.print(" - Lat: " + String(latitude, 6));
  } else {
    Serial.print("[x] Bad GPS data stream, wait 3 seconds");
  }
}

void readXYZ() {
  const int xCenter = 1915;
  const int yCenter = 1915;
  const int zCenter = 1980;
  const int tolerance = 20;

  const int xThresholdLow = xCenter - tolerance;
  const int xThresholdHigh = xCenter + tolerance;
  const int yThresholdLow = yCenter - tolerance;
  const int yThresholdHigh = yCenter + tolerance;
  const int zThresholdLow = zCenter - tolerance;
  const int zThresholdHigh = zCenter + tolerance;

  int rawX = analogRead(xPin);
  int rawY = analogRead(yPin);
  int rawZ = analogRead(zPin);

  String xSign = (rawX < xThresholdLow || rawX > xThresholdHigh) ? (rawX < xCenter ? "-" : "+") : "";
  String ySign = (rawY < yThresholdLow || rawY > yThresholdHigh) ? (rawY < yCenter ? "-" : "+") : "";
  String zSign = (rawZ < zThresholdLow || rawZ > zThresholdHigh) ? (rawZ < zCenter ? "-" : "") : "+";

  Serial.print(" [+] Accel (Raw): ");
  Serial.print("[X] "); Serial.print(rawX); Serial.print(" ("); Serial.print(xSign); Serial.print(")");
  Serial.print(" [Y] "); Serial.print(rawY); Serial.print(" ("); Serial.print(ySign); Serial.print(")");
  Serial.print(" [Z] "); Serial.print(rawZ); Serial.print(" ("); Serial.print(zSign); Serial.print(") ");
}

void readRTC() {
  DateTime now = rtc.now();
  String formattedTime = String(now.year()) + '/' +
                         String(now.month()) + '/' +
                         String(now.day()) + ' ' +
                         String(now.hour()) + ':' +
                         String(now.minute()) + ':' +
                         String(now.second());

  Serial.print("| [+] RTC "); Serial.println(formattedTime);
}

void readDisplay() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 24);
  display.println("Hello World!");
  display.display();
}