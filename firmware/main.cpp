#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "LiquidCrystal_I2C.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- 1. CONFIGURATION ---

const char* ssid = "hotspot name";
const char* password = "hotspot password";
const char* dashboard_url = "http://websitelink/update";

#define AUTHOR_EMAIL "email@gmail.com"
#define AUTHOR_PASSWORD "api password"
#define RECIPIENT_EMAIL "email@gmail.com"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define I2C_SDA 8
#define I2C_SCL 9
#define TEMP_PIN 4


// --- 2. OBJECTS & VARIABLES ---

SMTPSession smtp;
MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

long lastBeat = 0;
float beatsPerMinute;
int beatAvg = 0;

float currentTemp = 0;
int currentSpo2 = 0;

unsigned long lastEmailTime = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDashboardUpdate = 0;

const unsigned long emailCooldown = 60000;
const int dashboardInterval = 2000;


// --- 3. DASHBOARD FUNCTION ---

void sendDataToDashboard(float t, int b, int s) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        http.begin(dashboard_url);
        http.addHeader("Content-Type", "application/json");

        StaticJsonDocument<200> doc;
        doc["temp"] = String(t, 1);
        doc["bpm"] = String(b);
        doc["spo2"] = String(s);

        String httpRequestData;
        serializeJson(doc, httpRequestData);

        int httpResponseCode = http.POST(httpRequestData);

        Serial.print("Dashboard Sync Code: ");
        Serial.println(httpResponseCode);

        http.end();
    }
}


// --- 4. EMAIL FUNCTION ---

void sendEmailAlert(float temp, int bpm) {
    ESP_Mail_Session session;

    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;

    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;

    SMTP_Message message;

    message.sender.name = "S3 Health Monitor";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "ALERT: Critical Health Reading";

    message.addRecipient("Receiver", RECIPIENT_EMAIL);

    String htmlMsg = "<div style='border:3px solid red; padding:15px; font-family:sans-serif;'>";
    htmlMsg += "<h2 style='color:red;'>Medical Alert</h2>";
    htmlMsg += "<p>Patient Temp: <b>" + String(temp, 1) + " C</b></p>";
    htmlMsg += "<p>Heart Rate: <b>" + String(bpm) + " BPM</b></p></div>";

    message.html.content = htmlMsg.c_str();

    if (smtp.connect(&session)) {
        MailClient.sendMail(&smtp, &message);
        Serial.println(">>> EMAIL SENT!");
        smtp.closeSession();
    }
}


// --- 5. SETUP ---

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    Wire.begin(I2C_SDA, I2C_SCL);

    lcd.init();
    lcd.backlight();
    lcd.print("System Ready...");

    sensors.begin();

    if (!particleSensor.begin(Wire, 100000)) {
        Serial.println("MAX30102 Missing!");
    }

    particleSensor.setup(60, 1, 2, 400, 411, 4096);

    lcd.clear();
}


// --- 6. LOOP ---

void loop() {
    long irValue = particleSensor.getIR();

    // Heart rate detection
    if (checkForBeat(irValue)) {
        long delta = millis() - lastBeat;
        lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 150 && beatsPerMinute > 45) {
            beatAvg = (int)beatsPerMinute;
        }
    }

    if (millis() - lastDisplayUpdate > 800) {

        // Temperature
        sensors.requestTemperatures();
        currentTemp = sensors.getTempCByIndex(0);

        // SpO2 estimation
        if (irValue > 50000) {
            currentSpo2 = map(irValue, 175000, 195000, 95, 99);

            if (currentSpo2 > 100) currentSpo2 = 100;
            if (currentSpo2 < 85) currentSpo2 = 94;
        } else {
            currentSpo2 = 0;
            beatAvg = 0;
        }

        // LCD display
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(currentTemp, 1);
        lcd.print("C  ");

        lcd.setCursor(8, 0);
        lcd.print("O2:");
        lcd.print(currentSpo2);
        lcd.print("%");

        lcd.setCursor(0, 1);
        if (irValue > 50000) {
            lcd.print("BPM: ");
            lcd.print(beatAvg);
            lcd.print(" Pulse OK ");
        } else {
            lcd.print("Place Finger... ");
        }

        // Dashboard update
        if (millis() - lastDashboardUpdate > dashboardInterval) {
            sendDataToDashboard(currentTemp, beatAvg, currentSpo2);
            lastDashboardUpdate = millis();
        }

        // Email alert (NOTE: threshold is low, fix later if needed)
        if (currentTemp >= 32.0 && (millis() - lastEmailTime > emailCooldown)) {
            sendEmailAlert(currentTemp, beatAvg);
            lastEmailTime = millis();
        }

        lastDisplayUpdate = millis();
    }
}
