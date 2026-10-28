#include <WiFi.h>
#include <WebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <time.h>

const char* SSID = "ok";
const char* PASS = "palindrome@121";

MD_Parola P(MD_MAX72XX::FC16_HW, 5, 4);  // CS=D5, 4 modules
WebServer server(80);

enum Mode { TEXT, COUNTDOWN, CLOCK };
Mode mode = TEXT;
char msg[200] = "WIFI...";
unsigned long cdEnd = 0;     // millis() when countdown hits 0
int blinks = 0;              // flashes left after countdown ends
unsigned long lastBlink = 0;

void setText(const char* s) {
  strncpy(msg, s, sizeof(msg) - 1);
  msg[sizeof(msg) - 1] = 0;
  if (strlen(msg) <= 5)
    P.displayText(msg, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);        // fits: still
  else
    P.displayText(msg, PA_LEFT, 50, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);  // long: scroll
  P.displayReset();
}

void showIfChanged(const char* s) { if (strcmp(s, msg)) setText(s); }

void startMode(Mode m) {
  mode = m;
  blinks = 0;
  P.displayShutdown(false);
  msg[0] = 0;  // force redraw
}

void reply(int code, const char* body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(code, "text/plain", body);
}

// GET /show?text=hello
void handleShow() {
  if (!server.hasArg("text")) return reply(400, "missing text");
  startMode(TEXT);
  setText(server.arg("text").c_str());
  reply(200, "ok");
}

// GET /countdown?s=90
void handleCountdown() {
  long s = server.arg("s").toInt();
  if (s < 1 || s > 5999) return reply(400, "s must be 1-5999");
  startMode(COUNTDOWN);
  cdEnd = millis() + s * 1000UL;
  reply(200, "ok");
}

// GET /clock
void handleClock() {
  startMode(CLOCK);
  reply(200, "ok");
}

// GET /brightness?v=0..15
void handleBrightness() {
  P.setIntensity(constrain(server.arg("v").toInt(), 0, 15));
  reply(200, "ok");
}

void tick() {
  char buf[8];
  if (mode == COUNTDOWN) {
    long left = (long)(cdEnd - millis());
    if (left <= 0) {
      mode = TEXT;
      setText("DONE");
      blinks = 10;
      lastBlink = millis();
    } else {
      long s = (left + 999) / 1000;  // round up so N shows first
      if (s >= 60) snprintf(buf, sizeof(buf), "%ld:%02ld", s / 60, s % 60);
      else snprintf(buf, sizeof(buf), "%ld", s);
      showIfChanged(buf);
    }
  } else if (mode == CLOCK) {
    struct tm t;
    if (getLocalTime(&t, 0)) strftime(buf, sizeof(buf), "%H:%M", &t);
    else strcpy(buf, "--:--");  // not synced yet
    showIfChanged(buf);
  }

  if (blinks > 0 && millis() - lastBlink > 250) {  // flash after countdown
    lastBlink = millis();
    blinks--;
    P.displayShutdown(blinks % 2);
  }
}

void setup() {
  P.begin();
  P.setIntensity(1);
  setText("WIFI...");
  P.displayAnimate();

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) { P.displayAnimate(); delay(10); }
  setText(WiFi.localIP().toString().c_str());

  configTime(19800, 0, "pool.ntp.org", "time.google.com");  // IST = UTC+5:30

  server.on("/show", handleShow);
  server.on("/countdown", handleCountdown);
  server.on("/clock", handleClock);
  server.on("/brightness", handleBrightness);
  server.begin();
}

void loop() {
  server.handleClient();
  tick();
  if (P.displayAnimate()) P.displayReset();
}
