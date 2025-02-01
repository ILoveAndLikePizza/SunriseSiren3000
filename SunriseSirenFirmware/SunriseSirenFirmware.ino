#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <Preferences.h>

#include "Lighting.h"
#include "Buzzer.h"
#include "LDR.h"
#include "NTP.h"
#include "SHT21.h"
#include "Alarm.h"
#include "Button.h"
#include "Authentication.h"

enum State {
  CLOCK,
  ALARM_PREVIEW,
  TEMPERATURE,
  HUMIDITY,
  CUSTOM
};

Preferences pref;
WebServer server(80);

SunriseSiren3000Lighting lights;
SunriseSiren3000Buzzer buzzer;
SunriseSiren3000LDR ldr;
SunriseSiren3000Button button;
SunriseSiren3000NTP ntp;
SunriseSiren3000SHT21 sht21;
SunriseSiren3000Alarm alarms[7];

unsigned long lastStateCycledAt = 0;
enum State currentState = CLOCK;

unsigned int clockReturn;
bool leadingZero;
bool enableDST;
String alarmTimes;

byte customSegments[4][7];
CRGB customColors[4];
CRGB customColonPoint;

void loadSettings() {
  lights.defaultColor = CRGB(pref.getInt("default-c"));
  lights.alarmColor = CRGB(pref.getInt("alarm-c"));
  ldr.minValue = pref.getInt("ldr-min");
  ldr.maxValue = pref.getInt("ldr-max");
  clockReturn = pref.getInt("clock-return");
  leadingZero = pref.getBool("leading-zero");
  enableDST = pref.getBool("enable-dst");
  alarmTimes = pref.getString("alarm-times");
}

void updateAlarms() {
  for (int i=0; i<7; i++) {
    alarms[i].time = alarmTimes.substring(i * 4, i * 4 + 4);
    alarms[i].reset();
  }
}

void cycleState() {
  lastStateCycledAt = millis();

  if (currentState == CLOCK) currentState = TEMPERATURE;
  else if (currentState == TEMPERATURE) currentState = HUMIDITY;
  else currentState = CLOCK;
}

void setup() {
  WiFiManager wm;
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(90);

  if (wm.autoConnect(AP_NAME, OTHER_PASSWORD)) {
    if (!MDNS.begin(HOSTNAME)) lights.error();
    MDNS.addService("http", "tcp", 80);

    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTHER_PASSWORD);
    ArduinoOTA.begin();

    server.on("/", HTTP_GET, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();

      server.send(200, "text/plain", "Welcome to the Sunrise Siren 3000!\nPlease use Sunrise Siren Studio to configure the device.");
    });
    server.on("/connect", HTTP_GET, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();

      server.send(200, "text/plain", "Yes, a Sunrise Siren 3000 is here!");
    });
    server.on("/status", HTTP_GET, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();

      String output = "{\n  \"colors\": {\n    \"default\": ";
      output.concat(lights.defaultColor.as_uint32_t());
      output.concat(",\n    \"alarm\": ");
      output.concat(lights.alarmColor.as_uint32_t());
      output.concat("\n  },\n  \"alarmTimes\": \"");
      output.concat(alarmTimes);
      output.concat("\",\n  \"leadingZero\": ");
      output.concat(leadingZero ? "true" : "false");
      output.concat(",\n  \"enableDST\": ");
      output.concat(enableDST ? "true" : "false");
      output.concat("\n}");
      
      server.send(200, "application/json", output);
    });
    server.on("/sensors", HTTP_GET, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();

      String output = "{\n  \"ldr\": ";
      output.concat(ldr.rawValue);
      output.concat(",\n  \"temperature\": {\n    \"raw\": ");
      output.concat(sht21.rawTemperature);
      output.concat(",\n    \"translated\": ");
      output.concat(sht21.temperature);
      output.concat("\n  },\n  \"humidity\": {\n    \"raw\": ");
      output.concat(sht21.rawHumidity);
      output.concat(",\n    \"translated\": ");
      output.concat(sht21.humidity);
      output.concat("\n  }\n}");

      server.send(200, "application/json", output);
    });
    server.on("/update", HTTP_POST, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();

      pref.begin("SS3000-Conf", false);

      if (server.hasArg("default-c") && server.arg("default-c").toInt() != pref.getInt("default-c"))
        pref.putInt("default-c", server.arg("default-c").toInt());
      if (server.hasArg("alarm-c") && server.arg("alarm-c").toInt() != pref.getInt("alarm-c"))
        pref.putInt("alarm-c", server.arg("alarm-c").toInt());

      if (server.hasArg("ldr-min") && server.arg("ldr-min").toInt() != pref.getInt("ldr-min"))
        pref.putInt("ldr-min", server.arg("ldr-min").toInt());
      if (server.hasArg("ldr-max") && server.arg("ldr-max").toInt() != pref.getInt("ldr-max"))
        pref.putInt("ldr-max", server.arg("ldr-max").toInt());

      if (server.hasArg("clock-return") && server.arg("clock-return").toInt() != pref.getInt("clock-return"))
        pref.putInt("clock-return", server.arg("clock-return").toInt());
      if (server.hasArg("leading-zero") && server.arg("leading-zero").toInt() != pref.getBool("leading-zero"))
        pref.putBool("leading-zero", server.arg("leading-zero").toInt() == 1);
      if (server.hasArg("enable-dst") && server.arg("enable-dst").toInt() != pref.getBool("enable-dst"))
        pref.putBool("enable-dst", server.arg("enable-dst").toInt() == 1);

      if (server.hasArg("alarm-times") && !server.arg("alarm-times").equals(pref.getString("alarm-times")))
        pref.putString("alarm-times", server.arg("alarm-times"));

      loadSettings();
      updateAlarms();
      pref.end();

      ntp.setDST(enableDST);
      server.send(200, "text/plain", "Changes have been saved and applied!");
    });
    server.on("/custom", HTTP_POST, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();

      for (int i=0; i<4; i++) {
        if (server.hasArg("segment-" + String(i))) {
          for (int j=0; j<7; j++) customSegments[i][j] = String(server.arg("segment-" + String(i)).charAt(j)).toInt();
        }
        if (server.hasArg("color-" + String(i))) customColors[i] = CRGB(server.arg("color-" + String(i)).toInt());
      }
      if (server.hasArg("color-colon")) customColonPoint = CRGB(server.arg("color-colon").toInt());

      currentState = CUSTOM;
      server.send(200, "text/plain", "Custom configuration has been saved and is visible now!");
    });
    server.on("/reboot", HTTP_PATCH, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();

      ESP.restart();
    });
    server.onNotFound([]() {
      server.send(404, "text/plain", "404 Not Found");
    });
    server.begin();

    pref.begin("SS3000-Conf", true);
    loadSettings();
    updateAlarms();
    pref.end();

    lights.init();
    buzzer.init();
    ldr.init();
    button.init();
    ntp.init();
    sht21.init();

    lights.showTime("9999", lights.defaultColor);
    lights.update(MAX_BRIGHTNESS);

    while (!ntp.isTimeSet()) {
      ntp.update();
      delay(10);
    }
    ntp.setDST(enableDST);
  }
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  ldr.update();
  button.update();
  ntp.update();
  sht21.update();

  String t = ntp.getTime();
  int d = ntp.getDay();

  if (alarms[d].update(t)) currentState = CLOCK;

  buzzer.enabled = alarms[d].activity;
  buzzer.update();

  if (button.released) {
    if (alarms[d].tripping) alarms[d].stop();
    else cycleState();
  } else if (button.held) {
    lastStateCycledAt = millis();
    currentState = ALARM_PREVIEW;
  }

  if (millis() - lastStateCycledAt >= clockReturn && !(currentState == CLOCK || currentState == CUSTOM))
    currentState = CLOCK;

  if (currentState == CLOCK) {
    CRGB clockColor = (alarms[d].activity) ? lights.alarmColor : lights.defaultColor;
    lights.showTime(t, clockColor, leadingZero);
    lights.setColonPoint(clockColor);
  } else if (currentState == TEMPERATURE) {
    lights.showSingleDigit(0, (int)sht21.temperature / 10 % 10, lights.defaultColor);
    lights.showSingleDigit(1, (int)sht21.temperature % 10, lights.defaultColor);
    lights.showSingleDigit(2, DIGIT_DEGREE, lights.defaultColor);
    lights.showSingleDigit(3, DIGIT_C, lights.defaultColor);
    lights.setColonPoint(CRGB::Black);
  } else if (currentState == HUMIDITY) {
    lights.showSingleDigit(0, DIGIT_H, lights.defaultColor);
    lights.showSingleDigit(1, DIGIT_u, lights.defaultColor);
    lights.showSingleDigit(2, (int)sht21.humidity / 10 % 10, lights.defaultColor);
    lights.showSingleDigit(3, (int)sht21.humidity % 10, lights.defaultColor);
    lights.setColonPoint(lights.defaultColor);
  } else if (currentState == ALARM_PREVIEW) {
    bool alarmAlreadyTrippedToday = (t >= alarms[d].time);
    const int alarmPreviewIndex = alarmAlreadyTrippedToday ? ntp.getNextDay() : d;

    lights.showTime(alarms[alarmPreviewIndex].time, lights.defaultColor, leadingZero);
    lights.setColonPoint((millis() % 600 < 300) ? lights.defaultColor : CRGB::Black);
  } else if (currentState == CUSTOM) {
    for (int i=0; i<4; i++) lights.apply(i, customSegments[i], customColors[i]);

    lights.setColonPoint(customColonPoint);
  }

  int brightness = ldr.averagedValue;
  if (alarms[d].activity) brightness += ALARM_BRIGHTNESS_INCREMENT;
  lights.update(brightness);

  delay(20);
}
