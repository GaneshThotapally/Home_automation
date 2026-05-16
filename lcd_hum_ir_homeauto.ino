#include "IRremote.hpp"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include <map>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
#define WIFI_SSID "OPPO A53"    
#define WIFI_PASS "@mahi1028"
#define APP_KEY "e253bf1a-b1d0-431f-a33a-341ae432427b"      
#define APP_SECRET "f51ae1af-00fc-4d54-a030-84355a4e2dba-d06a60df-e8df-4e79-a327-d93a2f22ef32"  

// Device IDs
#define device_ID_1 "xxxxxxxxxxxxxxxxxxxxxxxx"
#define device_ID_2 "6665c88d674e208e6fe54add"
#define device_ID_3 "6665c81e6e1af3593502ca53"
#define device_ID_4 "6665c8e76e1af3593502cab5"

// Relay and switch pins
#define RelayPin2 10  //SD3
#define RelayPin3 14 //D5
#define RelayPin4 12 //D6

#define SwitchPin2 0   //D3 
#define SwitchPin3 13  //D7
#define SwitchPin4 3   //RX

#define wifiLed   16   //D0

// IR Receiver and LED control
const int irReceiverPin = D0;
const int ledPin1 = 10;      
const int ledPin2 = D5;
const int ledPin3 = D6;
unsigned long lastSignalTime = 0;
unsigned long signalIgnoreInterval = 100; // 100 milliseconds

#define BAUD_RATE   115200
#define DEBOUNCE_TIME 250

// DHT11 Sensor
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
unsigned long lastDHTReadTime = 0;
const unsigned long DHT_READ_INTERVAL = 2000; // Read DHT every 2 seconds

// LCD Display (Address 0x27, 16x2 Display, SDA → D2, SCL → D1)
LiquidCrystal_I2C lcd(0x27, 16, 2);

typedef struct {
  int relayPIN;
  int flipSwitchPIN;
} deviceConfig_t;

std::map<String, deviceConfig_t> devices = {
    {device_ID_2, {  RelayPin2, SwitchPin2 }},
    {device_ID_3, {  RelayPin3, SwitchPin3 }},
    {device_ID_4, {  RelayPin4, SwitchPin4 }}     
};

typedef struct {
  String deviceId;
  bool lastFlipSwitchState;
  unsigned long lastFlipSwitchChange;
} flipSwitchConfig_t;

std::map<int, flipSwitchConfig_t> flipSwitches;

void setupRelays() { 
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    pinMode(relayPIN, OUTPUT);
    digitalWrite(relayPIN, HIGH);
  }
}

void setupFlipSwitches() {
  for (auto &device : devices) {
    flipSwitchConfig_t flipSwitchConfig;
    flipSwitchConfig.deviceId = device.first;
    flipSwitchConfig.lastFlipSwitchChange = 0;
    flipSwitchConfig.lastFlipSwitchState = true;

    int flipSwitchPIN = device.second.flipSwitchPIN;
    flipSwitches[flipSwitchPIN] = flipSwitchConfig;
    pinMode(flipSwitchPIN, INPUT_PULLUP);
  }
}

bool onPowerState(String deviceId, bool &state) {
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off");
  int relayPIN = devices[deviceId].relayPIN;
  digitalWrite(relayPIN, !state);
  return true;
}

void handleFlipSwitches() {
  unsigned long actualMillis = millis();
  for (auto &flipSwitch : flipSwitches) {
    unsigned long lastFlipSwitchChange = flipSwitch.second.lastFlipSwitchChange;

    if (actualMillis - lastFlipSwitchChange > DEBOUNCE_TIME) {
      int flipSwitchPIN = flipSwitch.first;
      bool lastFlipSwitchState = flipSwitch.second.lastFlipSwitchState;
      bool flipSwitchState = digitalRead(flipSwitchPIN);
      if (flipSwitchState != lastFlipSwitchState) {
        flipSwitch.second.lastFlipSwitchChange = actualMillis;
        String deviceId = flipSwitch.second.deviceId;
        int relayPIN = devices[deviceId].relayPIN;
        bool newRelayState = !digitalRead(relayPIN);
        digitalWrite(relayPIN, newRelayState);

        SinricProSwitch &mySwitch = SinricPro[deviceId];
        mySwitch.sendPowerStateEvent(!newRelayState);
        flipSwitch.second.lastFlipSwitchState = flipSwitchState;
      }
    }
  }
}

void readDHTSensor() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDHTReadTime >= DHT_READ_INTERVAL) {
    lastDHTReadTime = currentMillis;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Print to Serial Monitor
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature);
    lcd.print("C");
    lcd.print(" H:");
    lcd.print(humidity);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("1:");
    lcd.print(digitalRead(RelayPin2) ? "OFF " : "ON ");
    lcd.print("2:");
    lcd.print(digitalRead(RelayPin3) ? "OFF " : "ON ");
    lcd.print("3:");
    lcd.print(digitalRead(RelayPin4) ? "OFF" : "ON ");
  }
}

void setupWiFi() {
  Serial.printf("\r\n[WiFi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  digitalWrite(wifiLed, LOW);
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro() {
  for (auto &device : devices) {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
  }

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, HIGH);

  setupRelays();
  setupFlipSwitches();
  setupWiFi();
  setupSinricPro();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize IR Receiver
  IrReceiver.begin(irReceiverPin, ENABLE_LED_FEEDBACK);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);

  // Start DHT sensor
  dht.begin();
}

void loop() {
  SinricPro.handle();
  handleFlipSwitches();
  readDHTSensor(); // Read temperature & humidity using non-blocking method

  if (IrReceiver.decode()) {
    unsigned long int decCode = IrReceiver.decodedIRData.command;
    Serial.println(decCode, HEX);

    unsigned long currentTime = millis();
    if (currentTime - lastSignalTime > signalIgnoreInterval) {
      if (decCode == 0xA) {
        digitalWrite(ledPin1, !digitalRead(ledPin1));
      } else if (decCode == 0x1B) {
        digitalWrite(ledPin2, !digitalRead(ledPin2));
      } else if (decCode == 0x1F) {
        digitalWrite(ledPin3, !digitalRead(ledPin3));
      }
      lastSignalTime = currentTime;
    }
    IrReceiver.resume();
  }
}
