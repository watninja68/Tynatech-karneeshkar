#include <WiFi.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void handleWebhook(AsyncWebServerRequest *request) {
  String payload = request->getParam("plain", true)->value();


  Serial.println("Received Webhook:");
  Serial.println(payload);

  request->send(200, "text/plain", "Webhook received");
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("Tynatech Jio", "Tynatech@w62");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/webhook", HTTP_POST, handleWebhook);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
}