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
#include "Countdown.h"
#include "Button.h"
#include "Authentication.h"

enum State {
  CLOCK,
  ALARM_PREVIEW,
  ALARM_EDIT_HOURS,
  ALARM_EDIT_MINUTES,
  TEMPERATURE,
  HUMIDITY,
  CUSTOM,
  COUNTDOWN
};

Preferences pref;
WebServer server(80);

SunriseSiren3000Lighting lights;
SunriseSiren3000Buzzer buzzer;
SunriseSiren3000LDR ldr;
SunriseSiren3000Button button;
SunriseSiren3000NTP ntp;
SunriseSiren3000SHT21 sht21;
SunriseSiren3000Countdown countdown;
SunriseSiren3000Alarm alarms[7];

unsigned long ticks = 0;
unsigned long rebootSignalSentAt = 0;
unsigned long lastStateCycledAt = 0;
enum State currentState = CLOCK;
bool asleep = false;
int sleepLevel = 0; // 0 = not asleep, MAX_BRIGHTNESS = zzz

unsigned int snoozeInterval;
unsigned int clockReturn;
bool leadingZero;
bool enableDST;
int alarmsEnabled;
String alarmTimes;
int alarmEditHour;
int alarmEditMinute;

int customSegments[4];
CRGB customColors[4];
CRGB customColonPoint;

void sendGitHubRedirect() {
  const String targetURL = "https://github.com/ILoveAndLikePizza/SunriseSiren3000/releases";

  String output = "<h1>Welcome to the Sunrise Siren 3000!</h1><h3>Please use <a href=\"";
  output += targetURL;
  output += "\">Sunrise Siren Studio</a> to configure the device.";

  server.sendHeader("Location", targetURL, true);
  server.send(301, "text/html", output);
}

void loadSettings() {
  lights.defaultColor = CRGB(pref.getInt("default-c"));
  lights.highlightColor = CRGB(pref.getInt("highlight-c"));
  clockReturn = pref.getInt("clock-return");
  leadingZero = pref.getBool("leading-zero");
  enableDST = pref.getBool("enable-dst");
  alarmsEnabled = pref.getInt("alarms-enabled");
  alarmTimes = pref.getString("alarm-times");
  snoozeInterval = pref.getInt("snooze-t");
  ldr.minValue = pref.getInt("ldr-min");
  ldr.maxValue = pref.getInt("ldr-max");
}

void updateAlarms() {
  for (int i=0; i<7; i++) {
    alarms[i].enabled = (alarmsEnabled & (int) pow(2, i)) > 0;
    alarms[i].time = alarmTimes.substring(i * 4, i * 4 + 4);
    alarms[i].reset();
  }
}

String generateAlarms() {
  String payload = "";
  for (int i=0; i<7; i++) payload.concat(alarms[i].time);

  return payload;
}

void cycleState() {
  lastStateCycledAt = millis();

  if (currentState == CLOCK) currentState = TEMPERATURE;
  else if (currentState == TEMPERATURE) currentState = HUMIDITY;
  else currentState = CLOCK;
}

int getDigit(float value, int exponent) {
  return (int)(value + 0.5) / (int) pow(10, exponent) % 10;
}

void setup() {
  WiFiManager wm;
  wm.setHostname(HOSTNAME);
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(90);

  if (wm.autoConnect(AP_NAME, OTHER_PASSWORD)) {
    if (!MDNS.begin(HOSTNAME)) lights.error();
    MDNS.addService("http", "tcp", 80);

    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTHER_PASSWORD);
    ArduinoOTA.begin();

    const char *headerKeys[] = {"User-Agent"};
    server.collectHeaders(headerKeys, 1);

    server.on("/", HTTP_GET, sendGitHubRedirect);
    server.on("/connect", HTTP_GET, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();
      else if (!server.header("User-Agent").startsWith("sunrise-siren-studio/v")) return sendGitHubRedirect();

      server.send(200, "text/plain", "Yes, a Sunrise Siren 3000 is here!");
    });
    server.on("/status", HTTP_GET, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();
      else if (!server.header("User-Agent").startsWith("sunrise-siren-studio/v")) return sendGitHubRedirect();

      String output = "{\n  \"colors\": {\n    \"default\": ";
      output.concat(lights.defaultColor.as_uint32_t());
      output.concat(",\n    \"alarm\": ");
      output.concat(lights.highlightColor.as_uint32_t());
      output.concat("\n  },\n  \"alarmsEnabled\": ");
      output.concat(alarmsEnabled);
      output.concat(",\n  \"alarmTimes\": \"");
      output.concat(alarmTimes);
      output.concat("\",\n  \"leadingZero\": ");
      output.concat(leadingZero ? "true" : "false");
      output.concat(",\n  \"enableDST\": ");
      output.concat(enableDST ? "true" : "false");
      output.concat(",\n  \"clockReturn\": ");
      output.concat(clockReturn / 1000);
      output.concat(",\n  \"snoozeInterval\": ");
      output.concat(snoozeInterval / 1000);
      output.concat(",\n  \"ldr\": {\n    \"min\": ");
      output.concat(ldr.minValue);
      output.concat(",\n    \"max\": ");
      output.concat(ldr.maxValue);
      output.concat("\n  }\n}");

      server.send(200, "application/json", output);
    });
    server.on("/sensors", HTTP_GET, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();
      else if (!server.header("User-Agent").startsWith("sunrise-siren-studio/v")) return sendGitHubRedirect();

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
      else if (!server.header("User-Agent").startsWith("sunrise-siren-studio/v")) return sendGitHubRedirect();

      pref.begin("SS3000-Conf", false);

      if (server.hasArg("default-c") && server.arg("default-c").toInt() != pref.getInt("default-c"))
        pref.putInt("default-c", server.arg("default-c").toInt());
      if (server.hasArg("highlight-c") && server.arg("highlight-c").toInt() != pref.getInt("highlight-c"))
        pref.putInt("highlight-c", server.arg("highlight-c").toInt());

      if (server.hasArg("clock-return") && server.arg("clock-return").toInt() != pref.getInt("clock-return"))
        pref.putInt("clock-return", server.arg("clock-return").toInt());
      if (server.hasArg("leading-zero") && server.arg("leading-zero").toInt() != pref.getBool("leading-zero"))
        pref.putBool("leading-zero", server.arg("leading-zero").toInt() == 1);
      if (server.hasArg("enable-dst") && server.arg("enable-dst").toInt() != pref.getBool("enable-dst"))
        pref.putBool("enable-dst", server.arg("enable-dst").toInt() == 1);

      if (server.hasArg("alarms-enabled") && server.arg("alarms-enabled").toInt() != pref.getInt("alarms-enabled"))
        pref.putInt("alarms-enabled", server.arg("alarms-enabled").toInt());
      if (server.hasArg("alarm-times") && !server.arg("alarm-times").equals(pref.getString("alarm-times")))
        pref.putString("alarm-times", server.arg("alarm-times"));

      if (server.hasArg("snooze-t") && server.arg("snooze-t").toInt() != pref.getInt("snooze-t"))
        pref.putInt("snooze-t", server.arg("snooze-t").toInt());
      if (server.hasArg("ldr-min") && server.arg("ldr-min").toInt() != pref.getInt("ldr-min"))
        pref.putInt("ldr-min", server.arg("ldr-min").toInt());
      if (server.hasArg("ldr-max") && server.arg("ldr-max").toInt() != pref.getInt("ldr-max"))
        pref.putInt("ldr-max", server.arg("ldr-max").toInt());

      loadSettings();
      updateAlarms();
      pref.end();

      ntp.setDST(enableDST);

      asleep = false;
      currentState = CLOCK;
      server.send(200, "text/plain", "Done!");
    });
    server.on("/custom", HTTP_POST, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();
      else if (!server.header("User-Agent").startsWith("sunrise-siren-studio/v")) return sendGitHubRedirect();

      for (int i=0; i<4; i++) {
        if (server.hasArg("segment-" + String(i))) customSegments[i] = server.arg("segment-" + String(i)).toInt();
        if (server.hasArg("color-" + String(i))) customColors[i] = CRGB(server.arg("color-" + String(i)).toInt());
      }
      if (server.hasArg("color-colon")) customColonPoint = CRGB(server.arg("color-colon").toInt());

      asleep = false;
      currentState = CUSTOM;
      server.send(200, "text/plain", "Done!");
    });
    server.on("/countdown", HTTP_POST, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();
      else if (!server.header("User-Agent").startsWith("sunrise-siren-studio/v")) return sendGitHubRedirect();

      if (!server.hasArg("t") && !server.hasArg("pauseable")) {
        server.send(400, "text/plain", "Missing arguments.");
        return;
      }
      int totalSeconds = server.arg("t").toInt();

      countdown.pauseable = (server.arg("pauseable").toInt() == 1);
      countdown.start(totalSeconds);

      asleep = false;
      currentState = COUNTDOWN;
      server.send(200, "text/plain", "Done!");
    });
    server.on("/reboot", HTTP_PATCH, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();
      else if (!server.header("User-Agent").startsWith("sunrise-siren-studio/v")) return sendGitHubRedirect();

      server.send(200, "text/plain", "Initiating reboot.");
      rebootSignalSentAt = millis();
    });
    server.on("/sleep", HTTP_PATCH, []() {
      if (!server.authenticate(HTTP_USERNAME, HTTP_PASSWORD)) return server.requestAuthentication();
      else if (!server.header("User-Agent").startsWith("sunrise-siren-studio/v")) return sendGitHubRedirect();

      if (asleep) {
        server.send(418, "text/plain", "zzz... huh? I'm a teapot already doing zzz!");
      } else if (currentState == COUNTDOWN || currentState == ALARM_EDIT_HOURS || currentState == ALARM_EDIT_MINUTES) {
        server.send(400, "text/plain", "Unable in current state.");
      } else {
        asleep = true;
        server.send(200, "text/plain", "zzz");
      }
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

    lights.showTime("----", lights.defaultColor);
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

  if (rebootSignalSentAt > 0 && millis() - rebootSignalSentAt >= 1500) ESP.restart();

  if (asleep) {
    if (++sleepLevel >= MAX_BRIGHTNESS) sleepLevel = MAX_BRIGHTNESS;
  } else {
    if (--sleepLevel <= 0) sleepLevel = 0;
  }

  ldr.update();
  button.update();
  if (ticks % 100 == 0) { // every 2 seconds:
    ntp.update();
    sht21.update();
  }
  countdown.update();

  String t = ntp.getTime();
  int d = ntp.getDay();

  bool alarmJustTripped = alarms[d].update(t, snoozeInterval);
  bool alarmAlreadyTrippedToday = (t >= alarms[d].time);
  const int alarmPreviewIndex = alarmAlreadyTrippedToday ? ntp.getNextDay() : d;

  if (button.released) {
    if (asleep) asleep = false; else {
      if (alarms[d].tripping) alarms[d].snooze();
      else if (currentState == ALARM_EDIT_HOURS) {
        if (++alarmEditHour >= 24) alarmEditHour = 0;
      } else if (currentState == ALARM_EDIT_MINUTES) {
        if (++alarmEditMinute >= 60) alarmEditMinute = 0;
      }
      else if (currentState == COUNTDOWN && !countdown.ended) countdown.togglePause();
      else cycleState();
    }
  } else if (button.held && !asleep) {
    if (alarms[d].tripping || alarms[d].snoozed) alarms[d].stop();
    else if (currentState == ALARM_PREVIEW && alarms[alarmPreviewIndex].enabled) {
      currentState = ALARM_EDIT_HOURS;
      alarmEditHour = alarms[alarmPreviewIndex].time.substring(0, 2).toInt();
      alarmEditMinute = alarms[alarmPreviewIndex].time.substring(2, 4).toInt();
    }
    else if (currentState == ALARM_EDIT_HOURS) currentState = ALARM_EDIT_MINUTES;
    else if (currentState == ALARM_EDIT_MINUTES) {
      char newTime[4];
      sprintf(newTime, "%02d%02d", alarmEditHour, alarmEditMinute);

      alarms[alarmPreviewIndex].time = String(newTime);
      String result = generateAlarms();

      pref.begin("SS3000-Conf", false);
      if (!result.equals(pref.getString("alarm-times")))
        pref.putString("alarm-times", result);

      loadSettings();
      updateAlarms();
      pref.end();
      currentState = CLOCK;
    }
    else if (currentState == COUNTDOWN) currentState = CLOCK;
    else {
      lastStateCycledAt = millis();
      currentState = ALARM_PREVIEW;
    }
  }

  if (
    (millis() - lastStateCycledAt >= clockReturn &&
    !(currentState == CLOCK || currentState == CUSTOM || currentState == COUNTDOWN || currentState == ALARM_EDIT_HOURS || currentState == ALARM_EDIT_MINUTES)) ||
    alarmJustTripped
  ) currentState = CLOCK;
  if (alarmJustTripped) asleep = false;

  if (currentState == CLOCK) {
    if (countdown.started) countdown.stop();

    CRGB clockColor = (alarms[d].activity) ? lights.highlightColor : lights.defaultColor;
    CRGB colonColor = (!alarms[d].snoozed || millis() % 1000 < 750) ? clockColor : CRGB::Black;

    lights.showTime(t, clockColor, leadingZero);
    lights.setColonPoint(colonColor);

    buzzer.enabled = alarms[d].activity;
    buzzer.update();
  } else if (currentState == TEMPERATURE) {
    lights.showSingleDigit(0, getDigit(sht21.temperature, 1), lights.defaultColor);
    lights.showSingleDigit(1, getDigit(sht21.temperature, 0), lights.defaultColor);
    lights.showSingleDigit(2, DIGIT_DEGREE, lights.defaultColor);
    lights.showSingleDigit(3, DIGIT_C, lights.defaultColor);
    lights.setColonPoint(CRGB::Black);
  } else if (currentState == HUMIDITY) {
    lights.showSingleDigit(0, DIGIT_H, lights.defaultColor);
    lights.showSingleDigit(1, DIGIT_u, lights.defaultColor);
    lights.showSingleDigit(2, getDigit(sht21.humidity, 1), lights.defaultColor);
    lights.showSingleDigit(3, getDigit(sht21.humidity, 0), lights.defaultColor);
    lights.setColonPoint(lights.defaultColor);
  } else if (currentState == ALARM_PREVIEW) {
    const int colonFrequency = (alarmPreviewIndex == d) ? 500 : 1200;
    const String preview = alarms[alarmPreviewIndex].enabled ? alarms[alarmPreviewIndex].time : "----";

    lights.showTime(preview, lights.defaultColor, leadingZero);
    lights.setColonPoint((millis() % colonFrequency < colonFrequency / 2) ? lights.defaultColor : CRGB::Black);
  } else if (currentState == ALARM_EDIT_HOURS || currentState == ALARM_EDIT_MINUTES) {
    CRGB hourColor = (currentState == ALARM_EDIT_HOURS && millis() % 1000 < 500) ? lights.highlightColor : lights.defaultColor;
    CRGB minuteColor = (currentState == ALARM_EDIT_MINUTES && millis() % 1000 < 500) ? lights.highlightColor : lights.defaultColor;

    lights.showSingleDigit(0, getDigit(alarmEditHour, 1), hourColor);
    lights.showSingleDigit(1, getDigit(alarmEditHour, 0), hourColor);
    lights.showSingleDigit(2, getDigit(alarmEditMinute, 1), minuteColor);
    lights.showSingleDigit(3, getDigit(alarmEditMinute, 0), minuteColor);
    lights.setColonPoint(lights.defaultColor);
  } else if (currentState == CUSTOM) {
    for (int i=0; i<4; i++) lights.showCustomDigit(i, customSegments[i], customColors[i]);

    lights.setColonPoint(customColonPoint);
  } else if (currentState == COUNTDOWN) {
    CRGB clockColor = (countdown.activity) ? lights.highlightColor : lights.defaultColor;

    for (int i=0; i<4; i++) {
      int digit = countdown.currentTime / (int) pow(10, i) % 10;
      int shownDigit = (i > 0 && digit == 0 && countdown.currentTime < pow(10, i)) ? DIGIT_OFF : digit;

      lights.showSingleDigit(3 - i, shownDigit, clockColor);
    }
    lights.setColonPoint(CRGB::Black);

    buzzer.enabled = countdown.activity;
    buzzer.update();
  }

  int brightness = ldr.averagedValue;
  if (buzzer.enabled) brightness += ALARM_BRIGHTNESS_INCREMENT;
  lights.update(brightness - sleepLevel);

  delay(20);
  ticks++;
}
