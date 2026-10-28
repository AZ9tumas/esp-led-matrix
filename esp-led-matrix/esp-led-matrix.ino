#include <WiFi.h>
#include <WebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

const char* SSID = "ok";
const char* PASS = "palindrome@121";

MD_Parola P(MD_MAX72XX::FC16_HW, 5, 4);  // CS=D5, 4 modules
WebServer server(80);
char msg[200] = "WIFI...";

void setText(const String& s) {
  s.toCharArray(msg, sizeof(msg));
  if (strlen(msg) <= 5)
    P.displayText(msg, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);        // fits: still
  else
    P.displayText(msg, PA_LEFT, 50, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);  // long: scroll
  P.displayReset();
}

void handleShow() {
  server.sendHeader("Access-Control-Allow-Origin", "*");  // let the web UI read the reply
  if (!server.hasArg("text")) { server.send(400, "text/plain", "missing text"); return; }
  setText(server.arg("text"));
  server.send(200, "text/plain", "ok");
}

void setup() {
  P.begin();
  P.setIntensity(1);
  setText("WIFI...");
  P.displayAnimate();

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) { P.displayAnimate(); delay(10); }
  setText(WiFi.localIP().toString());

  server.on("/show", handleShow);
  server.begin();
}

void loop() {
  server.handleClient();
  if (P.displayAnimate()) P.displayReset();
}
