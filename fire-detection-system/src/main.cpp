#include "main.h"

/* -------------------------------------------------------------------------- */
/*                                 VOID SETUP                                 */
/* -------------------------------------------------------------------------- */
void setup() {
    Serial.begin(115200);

    initWiFi();
    timeClient.begin();  // NTP init
    dht.begin();         // DHT Init
    pinMode(fireDigitalPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(blueLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);
    pinMode(greenLedPin, OUTPUT);

    // FIREBASE SETTING
    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("ok");
        signupOK = true;
    } else {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    // EOF FIREBASE SETTING

    digitalWrite(buzzerPin, HIGH);
    delay(250);
    digitalWrite(buzzerPin, LOW);
    delay(250);
    digitalWrite(buzzerPin, HIGH);
    delay(250);
    digitalWrite(buzzerPin, LOW);
    delay(250);
}
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  VOID LOOP                                 */
/* -------------------------------------------------------------------------- */
void loop() {
    timeClient.update();

    float t = readTemperature();
    int gas = analogRead(mq9Pin);
    int fire = readFire();
    // int fireDigital = digitalRead(fireDigitalPin);

    // Timestamp
    timestamp = timeClient.getFormattedTime();
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;
    date = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);

    handleBuzzer();
    makeDecision(gas, t, fire);

    Serial.print("Suhu : ");
    Serial.print(t);
    Serial.print(" Gas : ");
    Serial.print(gas);
    Serial.print(" Fire Sensor: ");
    Serial.print(fire);
    Serial.print(" | isWarning: ");
    Serial.print(isWarning);
    Serial.print(" warningMsg: ");
    Serial.println(warningMsg);

    // Push log to firebase every 60s
    if (Firebase.ready() && signupOK && (millis() - sendLogPrevMillis > 60 * 1000 || sendLogPrevMillis == 0)) {
        sendLogPrevMillis = millis();
        pushHistory(gas, t, fire);
    }

    // Update Real Data to firebase every 5s
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5 * 1000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();
        setRealData(gas, t, fire);
    }

    delay(50);
}
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  FUNCTION                                  */
/* -------------------------------------------------------------------------- */

/* ------------------------------- WiFi Setup ------------------------------- */
void initWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
    Serial.println();
}

/* -------------------------- Read Temperature (C) -------------------------- */
/*

*/
float readTemperature() {
    float t = dht.readTemperature();
    if (isnan(t)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return 0;
    }
    return t;
}

/* -------------------------- Read Flame Gas (PPM) -------------------------- */
// TODO: Hitung gas ke PPM
float readGas() {
    // masih analog
    int gasAnalogValue = analogRead(mq9Pin);
    return gasAnalogValue;
}

/* -------------------------- Read Flame Analog (V) ------------------------- */
float readFire() {
    // masih analog
    int fireAnalogValue = analogRead(fireAnalogPin);
    return fireAnalogValue;
}

/* -------------------------- Handle Buzzer --------------------------------- */
void handleBuzzer() {
    if (isWarning != 0) {
        if (millis() - buzzerPrevMillis > 1000) {
            buzzerPrevMillis = millis();
            buzzerState = !buzzerState;
            digitalWrite(buzzerPin, buzzerState);
        }
    } else {
        digitalWrite(buzzerPin, LOW);
    }
}

/* ------------------- Make Decision based on sensor value ------------------ */
// TODO: Make decision based on sensor value (fuzzy nek iso)
void makeDecision(float gasValue, float temperatureValue, float fireValue) {
    // default value
    isWarningBefore = isWarning;
    isWarning = 0;
    warningMsg = "Aman";

    // warning kecil
    if (gasValue > alertGas) {
        isWarning = 1;
        warningMsg = "Gas atau asap terdeteksi diatas 1000 Analog";
    } else if (temperatureValue > alertTemperature) {
        isWarning = 1;
        warningMsg = "Suhu terdeteksi diatas 35 C";
    } else if (fireValue < alertFire) {
        isWarning = 1;
        warningMsg = "Api terdeteksi";
    }

    // warning sedang
    if (gasValue > alertGas && temperatureValue > alertTemperature) {
        isWarning = 2;
        warningMsg = "Gas dan Suhu terdeteksi";
    } else if (gasValue > alertGas && fireValue < alertFire) {
        isWarning = 2;
        warningMsg = "Gas dan Api terdeteksi";
    } else if (temperatureValue > alertTemperature && fireValue < alertFire) {
        isWarning = 2;
        warningMsg = "Suhu dan Api terdeteksi";
    }

    // warning besar
    if (gasValue > alertGas && temperatureValue > alertTemperature && fireValue < alertFire) {
        isWarning = 3;
        warningMsg = "Gas, Suhu, dan Api terdeteksi";
    }

    // send warning to firebase
    if (isWarning != 0 && isWarning != isWarningBefore) {
        Serial.println("========");
        Serial.println("WARNING!");
        Serial.println("========");
        pushHistory(gasValue, temperatureValue, fireValue);
        setRealData(gasValue, temperatureValue, fireValue);
        return;
    }
}

/* -------------------------- Push log to firebase -------------------------- */
void pushHistory(float gasValue, float temperatureValue, float fireValue) {
    FirebaseJson json;
    json.set("gasValue", gasValue);
    json.set("temperatureValue", temperatureValue);
    json.set("fireValue", fireValue);
    json.set("isWarning", isWarning);
    json.set("warningMsg", warningMsg);
    json.set("timestamp", timestamp);
    json.set("date", date);

    Firebase.pushJSON(fbdo, "log", json);

    Serial.println("Data pushed to log!");
}

/* ------------------------ Set Real Data to firebase ----------------------- */
void setRealData(float gasValue, float temperatureValue, float fireValue) {
    FirebaseJson json;
    json.set("gasValue", gasValue);
    json.set("temperatureValue", temperatureValue);
    json.set("fireValue", fireValue);
    json.set("isWarning", isWarning);
    json.set("warningMsg", warningMsg);
    json.set("timestamp", timestamp);
    json.set("date", date);

    Firebase.setJSON(fbdo, "realData", json);

    Serial.println("Real Data has been updated!");
}