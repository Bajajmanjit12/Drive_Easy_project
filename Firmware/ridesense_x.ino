#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPSPlus.h>

// ================= WIFI =================
const char* ssid = "HOTSPOT NAME";
const char* password = "PASSWORD";

// ================= TELEGRAM =================
String botToken = "YOUR BOT TAKOEN8365285694:AAEWv1LkOuT2f3AnszF8OH1K0rQaOhltM1A";
String chatID = "YOUR CHAT ID";

// ================= MPU =================
Adafruit_MPU6050 mpu;
bool crashDetectedFinal = false;
// ================= GPS =================
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

// ================= IR SENSOR =================
#define IR_PIN 27

// ================= BUZZER =================
#define BUZZER_PIN 19

// ================= MQ3 =================
#define MQ3_PIN 34
int alcoholThreshold = 1200;
float alcoholSmooth = 0;
bool alcoholDetected = false;

// ================= CRASH =================
unsigned long noMotionTime = 4000;
bool impactDetected = false;
unsigned long impactTime = 0;
float prevAcc = 1.0;

// ================= DROWSINESS =================
unsigned long eyeClosedStart = 0;
bool eyeClosed = false;
bool drowsyAlertSent = false;
const unsigned long DROWSY_TIME = 2000;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(IR_PIN, INPUT);
  pinMode(MQ3_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("🚀 System Starting...");

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");

  // MPU
  Wire.begin(21, 22);
  if (!mpu.begin()) {
    Serial.println("❌ MPU6050 not found!");
    while (1);
  }
  Serial.println("✅ MPU Ready");

  // GPS
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

 
}

// ================= LOOP =================
void loop() {

  // ===== GPS =====
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // =====================================================
  // 😴 DROWSINESS DETECTION
  // =====================================================
 // =====================================================
// 😴 DROWSINESS DETECTION
// =====================================================

static int stableCount = 0;

int irState = digitalRead(IR_PIN);

// Sensor LED ON = LOW
bool isEyeClosed = (irState == LOW);

if (isEyeClosed) {

  stableCount++;

  // Stable detection
  if (stableCount >= 3) {

    // 🔊 Buzzer Sound
    tone(BUZZER_PIN, 1200);

    if (!eyeClosed) {

      eyeClosed = true;
      eyeClosedStart = millis();
      drowsyAlertSent = false;

      Serial.println("👁️ Eye Closed...");
    }

    if (!drowsyAlertSent &&
        millis() - eyeClosedStart >= DROWSY_TIME) {

      Serial.println("😴 DROWSINESS DETECTED!");

      // Beep Beep Alert
      for(int i=0; i<5; i++) {

        tone(BUZZER_PIN, 1500);
        delay(150);

        noTone(BUZZER_PIN);
        delay(100);
      }

      if (WiFi.status() == WL_CONNECTED) {
        sendDrowsinessTelegram();
      }

      drowsyAlertSent = true;
    }
  }

} else {

  stableCount = 0;

  eyeClosed = false;
  drowsyAlertSent = false;

  // 🔇 Stop buzzer
  noTone(BUZZER_PIN);
}

  // =====================================================
  // 🍺 ALCOHOL DETECTION
  // =====================================================
  int alcoholRaw = analogRead(MQ3_PIN);

  alcoholSmooth = (alcoholSmooth * 0.8) + (alcoholRaw * 0.2);
  alcoholDetected = alcoholSmooth > alcoholThreshold;

  Serial.print("Alcohol: ");
  Serial.print(alcoholRaw);
  Serial.print(" | Smoothed: ");
  Serial.print(alcoholSmooth);

  static bool alcoholSent = false;

  if (alcoholDetected) {
    Serial.println(" ⚠️ Alcohol Detected");

    if (!alcoholSent && WiFi.status() == WL_CONNECTED) {
      sendAlcoholTelegram();
      alcoholSent = true;
    }

  } else {
    Serial.println(" ✅ Safe");
    alcoholSent = false;
  }

  // =====================================================
  // 🚨 CRASH DETECTION
  // =====================================================
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float currentAcc = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  ) / 9.81;

  float acc_g = (prevAcc * 0.7) + (currentAcc * 0.3);
  float diff = abs(acc_g - prevAcc);
  prevAcc = acc_g;

  Serial.print(" | Acc: ");
  Serial.println(acc_g);

  if (diff > 1.5 && !impactDetected) {
    impactDetected = true;
    impactTime = millis();
    Serial.println("⚠️ Impact Detected!");
  }

  if (impactDetected && millis() - impactTime > noMotionTime) {

    if (acc_g < 1.2) {
      Serial.println("🚨 CRASH CONFIRMED!");

      crashDetectedFinal = true;  // ✅ set flag

      if (WiFi.status() == WL_CONNECTED) {
        sendAccidentTelegram();
      }

    } else {
      Serial.println("❌ False Alarm");
    }

    impactDetected = false;
  }

  delay(200);


  // ================= SEND TO DASHBOARD =================
static unsigned long lastSend = 0;

if (millis() - lastSend > 2000) {

  // ===== DEBUG TEST =====
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://10.209.119.196:5000/data");
    int code = http.GET();

    Serial.print("TEST GET Code: ");
    Serial.println(code);

    http.end();
  }

  bool drowsyFlag = drowsyAlertSent;
  bool alcoholFlag = alcoholDetected;
  bool crashFlag = crashDetectedFinal;

  sendDataToServer(alcoholFlag, drowsyFlag, crashFlag);

  crashDetectedFinal = false;
  lastSend = millis();
}
}

// =====================================================
// 📩 TELEGRAM FUNCTIONS
// =====================================================

void sendAccidentTelegram() {

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String lat = "0", lng = "0";

  if (gps.location.isValid()) {
    lat = String(gps.location.lat(), 6);
    lng = String(gps.location.lng(), 6);
  }

  String link = "https://www.google.com/maps?q=" + lat + "," + lng;

  String message =
    "🚨%20Accident%20Detected!%0A"
    "📍%20Location:%20" + link + "%0A%0A"

    "⚠️%20दुर्घटना%20झाली%20आहे!%0A"
    "कृपया%20मदत%20करा!%0A%0A"

    "⚠️%20दुर्घटना%20हो%20गई%20है!%0A"
    "कृपया%20मदद%20करें!";

  String url = "https://api.telegram.org/bot" + botToken +
               "/sendMessage?chat_id=" + chatID + "&text=" + message;

  http.begin(client, url);
  http.GET();
  http.end();
}

// ---------------- DROWSINESS ----------------
void sendDrowsinessTelegram() {

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String lat = "0", lng = "0";

  // Check GPS validity
  if (gps.location.isValid()) {
    lat = String(gps.location.lat(), 6);
    lng = String(gps.location.lng(), 6);
  }

  // Google Maps link
  String link = "https://www.google.com/maps?q=" + lat + "," + lng;

  // Telegram message
  String message =
    "😴 Drowsiness Detected!\n"
    "Stop vehicle immediately!\n\n"

    "😴 झोप येत आहे!\n"
    "वाहन थांबवा!\n\n"

    "😴 नींद आ रही है!\n"
    "वाहन रोकें!\n\n"

    "📍 Location:\n" + link;

  // Encode spaces and new lines automatically
  message.replace(" ", "%20");
  message.replace("\n", "%0A");

  String url = "https://api.telegram.org/bot" + botToken +
               "/sendMessage?chat_id=" + chatID +
               "&text=" + message;

  Serial.println(url);

  http.begin(client, url);

  int httpCode = http.GET();

  Serial.print("HTTP Response: ");
  Serial.println(httpCode);

  http.end();
}

// ---------------- ALCOHOL ----------------
void sendAlcoholTelegram() {

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String message =
    "🍺%20Alcohol%20Detected!%0A"
    "Do%20NOT%20drive!%0A%0A"

    "🍺%20मद्य%20आढळले!%0A"
    "वाहन%20चालवू%20नका!%0A%0A"

    "🍺%20शराब%20पाई%20गई!%0A"
    "गाड़ी%20मत%20चलाएं!";

  String url = "https://api.telegram.org/bot" + botToken +
               "/sendMessage?chat_id=" + chatID + "&text=" + message;

  http.begin(client, url);
  http.GET();
  http.end();
}


void sendDataToServer(bool alcohol, bool drowsy, bool crash) {

  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;              // ✅ REQUIRED
  HTTPClient http;

  String url = "http://10.209.119.196:5000/update";

  http.begin(client, url);        // ✅ FIXED
  http.addHeader("Content-Type", "application/json");

  String latitude = "0";
  String longitude = "0";

  if (gps.location.isValid()) {
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
  }

  String json = "{";
  json += "\"alcohol\":" + String(alcohol ? 1 : 0) + ",";
  json += "\"drowsiness\":" + String(drowsy ? 1 : 0) + ",";
  json += "\"crash\":" + String(crash ? 1 : 0) + ",";
  json += "\"latitude\":" + latitude + ",";
  json += "\"longitude\":" + longitude;
  json += "}";

  int code = http.POST(json);

  Serial.print("Server Code: ");
  Serial.println(code);

  Serial.println(http.getString());   // 🔥 DEBUG RESPONSE

  http.end();
}
