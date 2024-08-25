#include <Wire.h>
#include <U8g2lib.h>
#include <TinyGPS++.h>
#include <RTClib.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

RTC_DS3231 rtc;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
TinyGPSPlus gps;

// Define RX and TX pins for Serial1
const int GPS_RX = 1;  // GPIO 1 / D1
const int GPS_TX = 0;  // GPIO 0 / D0

// MPU6050 I2C Address
const int MPU_ADDR = 0x69;

// MPU6050 Bias values
const long ax_bias = 1012;
const long ay_bias = 123;
const long az_bias = 16305;
const long gx_bias = -196;
const long gy_bias = -190;
const long gz_bias = -97;

// Variables for raw MPU6050 data
int16_t ax, ay, az, gx, gy, gz;

unsigned long displayStartTime = 0;
bool displayOn = false;
bool displayAccelerometer = false;

// Button pin definitions
const int buttonPin1 = D7;
const int buttonPin2 = D8;
bool buttonPressed = false;
bool screenCyclePressed = false;

// Display durations (in milliseconds)
const unsigned long regularDisplayDuration = 10000;
const unsigned long accelerometerDisplayDuration = 30000;

// Declare functions
void startup();
void readGPS();
void readRTC();
void readDisplay();
void readDisplayData();
void readXYZ();
void readButtons();
void readXYZPos();

void setup() {
  u8g2.begin();
  readRTC();
  startup();

  Serial.begin(115200);  // Initialize Serial Monitor
  Serial1.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);  // Initialize Serial1 for GPS
  
  Wire.begin();
  
  // Initialize MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // Wake up MPU-6050
  Wire.endTransmission(true);
  
  // Initialize button pins
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
}

void loop() {
  readXYZ();
  readGPS();
  readRTC();
  readButtons();
  readXYZPos();  // Check accelerometer conditions

  if (displayOn) {
    if (displayAccelerometer) {
      readDisplayData();
    } else {
      readDisplay();
    }
    
    unsigned long elapsedTime = millis() - displayStartTime;
    unsigned long displayDuration = displayAccelerometer ? accelerometerDisplayDuration : regularDisplayDuration;
    
    if (elapsedTime >= displayDuration) {
      displayOn = false;
      u8g2.clearBuffer();
      u8g2.sendBuffer();
    }
  }

  delay(100);
}

void readXYZ() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // Starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read(); // ACCEL_XOUT_H and ACCEL_XOUT_L
  ay = Wire.read() << 8 | Wire.read(); // ACCEL_YOUT_H and ACCEL_YOUT_L
  az = Wire.read() << 8 | Wire.read(); // ACCEL_ZOUT_H and ACCEL_ZOUT_L
  Wire.read() << 8 | Wire.read();      // TEMP_OUT_H and TEMP_OUT_L (ignored)
  gx = Wire.read() << 8 | Wire.read(); // GYRO_XOUT_H and GYRO_XOUT_L
  gy = Wire.read() << 8 | Wire.read(); // GYRO_YOUT_H and GYRO_YOUT_L
  gz = Wire.read() << 8 | Wire.read(); // GYRO_ZOUT_H and GYRO_ZOUT_L

  // Apply bias correction
  ax -= ax_bias;
  ay -= ay_bias;
  az -= az_bias;
  gx -= gx_bias;
  gy -= gy_bias;
  gz -= gz_bias;
}

void readButtons() {
  if (digitalRead(buttonPin1) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      displayOn = true;
      displayStartTime = millis();
    }
  } else {
    buttonPressed = false;
  }

  if (digitalRead(buttonPin2) == LOW) {
    if (!screenCyclePressed) {
      screenCyclePressed = true;
      displayAccelerometer = !displayAccelerometer;
      displayStartTime = millis();
    }
  } else {
    screenCyclePressed = false;
  }
}

void readGPS() {
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    gps.encode(c);
  }
}

void readRTC() {
  if (!rtc.begin()) {
    Serial.println("[!] Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void readDisplay() {
  u8g2.clearBuffer();

  DateTime now = rtc.now();

  // Set a larger font for the time
  u8g2.setFont(u8g2_font_ncenB14_tr);  // Adjust this to a larger font if available

  // Format the time string with leading zeros
  String timeStr = String(now.hour()) + ":" + 
                   (now.minute() < 10 ? "0" : "") + String(now.minute()) + ":" + 
                   (now.second() < 10 ? "0" : "") + String(now.second());

  // Time string dimensions
  int16_t timeStrWidth = u8g2.getStrWidth(timeStr.c_str());
  int16_t timeStrHeight = u8g2.getMaxCharHeight();
  int xTime = (SCREEN_WIDTH - timeStrWidth) / 2;
  int yTime = (SCREEN_HEIGHT - timeStrHeight) / 2 - 10;

  // Display time in the larger font
  u8g2.drawStr(xTime, yTime + timeStrHeight, timeStr.c_str());

  // Set a smaller font for the date and other information
  u8g2.setFont(u8g2_font_6x10_tr);  // Small, square-like font for date and other data

  // Date string and dimensions
  String dateStr = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
  int16_t dateStrWidth = u8g2.getStrWidth(dateStr.c_str());
  int16_t dateStrHeight = u8g2.getMaxCharHeight();
  int xDate = (SCREEN_WIDTH - dateStrWidth) / 2;
  int yDate = yTime + timeStrHeight + 5;  // Adjust position below the time

  // Display date in the smaller font
  u8g2.drawStr(xDate, yDate + dateStrHeight, dateStr.c_str());

  // Calculate temperature in Fahrenheit
  float tempC = rtc.getTemperature();
  float tempF = tempC * 1.8 + 32;
  String tempStr = String(tempF, 1);

  // Concatenate degree symbol
  String tempDisplayStr = tempStr + "f";

  // Display temperature in the smaller font in the top left corner
  u8g2.drawStr(0, 10, tempDisplayStr.c_str());

  // Longitude (left side) - Limit to 3 decimal places
  String lngStr = String(gps.location.lng(), 3);
  int16_t lngStrWidth = u8g2.getStrWidth(lngStr.c_str());

  // Latitude (right side) - Limit to 5 decimal places
  String latStr = String(gps.location.lat(), 5);
  int16_t latStrWidth = u8g2.getStrWidth(latStr.c_str());

  // Number of Satellites (center) - Display only number
  String satsStr = String(gps.satellites.value());
  int16_t satsStrWidth = u8g2.getStrWidth(satsStr.c_str());

  // Calculate positions
  int xLng = 0;
  int xSats = (SCREEN_WIDTH - (lngStrWidth + satsStrWidth + latStrWidth)) / 2 + lngStrWidth;
  int xLat = SCREEN_WIDTH - latStrWidth;
  
  // Adjust yBottom to position text one line up from the very bottom
  int yBottom = SCREEN_HEIGHT - u8g2.getMaxCharHeight() - 2;

  // Display GPS data in the smaller font
  u8g2.drawStr(xLng, yBottom + u8g2.getMaxCharHeight(), lngStr.c_str());
  u8g2.drawStr(xSats, yBottom + u8g2.getMaxCharHeight(), satsStr.c_str());
  u8g2.drawStr(xLat, yBottom + u8g2.getMaxCharHeight(), latStr.c_str());

  u8g2.sendBuffer();
}

void readDisplayData() {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tr);

  String axStr = "aX: " + String(abs(ax) < 1000 ? 0 : ax);
  String ayStr = "aY: " + String(abs(ay) < 1000 ? 0 : ay);
  String azStr = "aZ: " + String(abs(az) < 1000 ? 0 : az);
  String gxStr = "gX: " + String(abs(gx) < 1000 ? 0 : gx);
  String gyStr = "gY: " + String(abs(gy) < 1000 ? 0 : gy);
  String gzStr = "gZ: " + String(abs(gz) < 1000 ? 0 : gz);

  u8g2.drawStr(0, 10, axStr.c_str());
  u8g2.drawStr(0, 20, ayStr.c_str());
  u8g2.drawStr(0, 30, azStr.c_str());
  u8g2.drawStr(0, 40, gxStr.c_str());
  u8g2.drawStr(0, 50, gyStr.c_str());
  u8g2.drawStr(0, 60, gzStr.c_str());

  u8g2.sendBuffer();
}

void readXYZPos() {
  if (ax > -10000 && ax < -1000 && az > -3000 && az < -1000) {
    displayOn = true;
    displayStartTime = millis();
  }
}

void startup() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  const char* bootMessage = "Booting....";

  int16_t messageWidth = u8g2.getStrWidth(bootMessage);
  int16_t messageHeight = u8g2.getMaxCharHeight();

  int xMessage = (SCREEN_WIDTH - messageWidth) / 2;
  int yMessage = (SCREEN_HEIGHT - messageHeight) / 2 + messageHeight;

  u8g2.drawStr(xMessage, yMessage, bootMessage);
  u8g2.sendBuffer();
  delay(10000);

  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {
    readDisplay();
    delay(100);
  }
  
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}
