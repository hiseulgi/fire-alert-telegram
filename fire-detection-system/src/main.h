#pragma once
#ifndef _main_h
#define _main_h

// LIBRARY
#include <Arduino.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "DHT.h"
#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"
// EOF LIBRARY

// FIREBASE SETTING
#define WIFI_SSID "antiwibu31"
#define WIFI_PASSWORD "acumalaka31"

#define API_KEY "AIzaSyCmVWzW1fYdmnmwyb2O_jyUgXrN9Wd9WhQ"
#define DATABASE_URL "https://fire-detection-780e3-default-rtdb.asia-southeast1.firebasedatabase.app"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
// EOF FIREBASE SETTING

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000);
// EOF NTP

// INPUT PINOUT
#define DHTPIN 25
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define mq9Pin 32

#define fireAnalogPin 35
#define fireDigitalPin 0

#define buzzerPin 2
#define blueLedPin 27
#define redLedPin 14
#define greenLedPin 12
// EOF INPUT PINOUT

// CONST SENSOR LIMIT
const int alertTemperature = 32;  // (lebih dari) celcius
const int alertGas = 2000;        // (lebih dari) sementara pake analogRead, belum ppm CO, CH4
const int alertFire = 200;        // (kurang dari) analogRead, test manual dapet yang terdekat itu nilai e 200
// EOF CONST SENSOR LIMIT

// Other Var
unsigned long sendLogPrevMillis = 0;
unsigned long sendDataPrevMillis = 0;
unsigned long buzzerPrevMillis = 0;
String timestamp, warningMsg, date;
byte isWarning = 0, isWarningBefore = 0;
bool buzzerState = 0;
// EOF Other Var

// Prototype Func
void initWiFi();
float readTemperature();
float readGas();
float readFire();
void handleBuzzer();
void makeDecision(float gasValue, float temperatureValue, float fireValue);

void pushHistory(float gasValue, float temperatureValue, float fireValue);
void setRealData(float gasValue, float temperatureValue, float fireValue);
// EOF Prototype Func

#endif