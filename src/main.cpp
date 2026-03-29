#include <Arduino.h>
#include <web_page.h>

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Updater.h>
ESP8266WebServer server(80);
#elif defined(ESP32)
#include <ESPmDNS.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
WebServer server(80);
#else
#error Unsupported platform
#endif

#define AP_PASSWORD ""

namespace {

constexpr char AP_SSID[] = "ESP-Setup";
constexpr char MDNS_HOST[] = "esp";
constexpr unsigned long REBOOT_DELAY_MS = 2000;

bool shouldReboot = false;
unsigned long rebootAtMs = 0;
bool uploadHasError = false;
String uploadErrorMessage;

String getUpdateErrorByCode(uint8_t errorCode) {
  switch (errorCode) {
    case UPDATE_ERROR_OK:
      return F("No error");
    case UPDATE_ERROR_WRITE:
      return F("Flash write failed");
    case UPDATE_ERROR_ERASE:
      return F("Flash erase failed");
    case UPDATE_ERROR_READ:
      return F("Flash read failed");
    case UPDATE_ERROR_SPACE:
      return F("Not enough space");
    case UPDATE_ERROR_SIZE:
      return F("Firmware size mismatch");
    case UPDATE_ERROR_STREAM:
      return F("Upload stream timeout");
    case UPDATE_ERROR_MD5:
      return F("MD5 check failed");
    case UPDATE_ERROR_MAGIC_BYTE:
      return F("Invalid firmware format");
#if defined(ESP32)
    case UPDATE_ERROR_ABORT:
      return F("Update aborted");
    case UPDATE_ERROR_ACTIVATE:
      return F("Firmware activate failed");
    case UPDATE_ERROR_NO_PARTITION:
      return F("No suitable partition");
    case UPDATE_ERROR_BAD_ARGUMENT:
      return F("Bad update argument");
#endif
    default:
      return String(F("Update error #")) + errorCode;
  }
}

String getUploadErrorString() {
#if defined(ESP8266)
  return String(Update.getErrorString());
#else
  return getUpdateErrorByCode(Update.getError());
#endif
}

String buildIndexHtml() {
  String html = FPSTR(INDEX_HTML);
  html.replace("{{IP}}", WiFi.softAPIP().toString());
  html.replace("{{HOST}}", String(MDNS_HOST));
  return html;
}

void scheduleReboot() {
  shouldReboot = true;
  rebootAtMs = millis() + REBOOT_DELAY_MS;
}

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  server.send(200, "text/html; charset=utf-8", buildIndexHtml());
}

void handleUpdateUpload() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    uploadHasError = false;
    uploadErrorMessage = "";
    Serial.printf("[OTA] Upload start: %s\n", upload.filename.c_str());

#if defined(ESP8266)
    const uint32_t maxSketchSpace =
        (ESP.getFreeSketchSpace() - 0x1000U) & 0xFFFFF000U;
    if (!Update.begin(maxSketchSpace)) {
#else
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
#endif
      uploadHasError = true;
      uploadErrorMessage = getUploadErrorString();
      Serial.printf("[OTA] Update begin failed: %s\n", uploadErrorMessage.c_str());
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (!uploadHasError) {
      const size_t written = Update.write(upload.buf, upload.currentSize);
      if (written != upload.currentSize) {
        uploadHasError = true;
        uploadErrorMessage = getUploadErrorString();
        Serial.printf("[OTA] Update write failed: %s\n", uploadErrorMessage.c_str());
      }
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (!uploadHasError) {
      if (!Update.end(true)) {
        uploadHasError = true;
        uploadErrorMessage = getUploadErrorString();
        Serial.printf("[OTA] Update finalize failed: %s\n", uploadErrorMessage.c_str());
      } else {
        Serial.printf("[OTA] Upload complete: %u bytes\n", upload.totalSize);
      }
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    uploadHasError = true;
    uploadErrorMessage = "Upload aborted";
#if defined(ESP32)
    Update.abort();
#endif
    Serial.println("[OTA] Upload aborted by client");
  }

  yield();
}

void handleUpdateResult() {
  if (uploadHasError) {
    const String message =
        uploadErrorMessage.isEmpty() ? String("Firmware update failed") : uploadErrorMessage;
    server.send(500, "text/plain; charset=utf-8", message);
    Serial.printf("[OTA] Upload failed: %s\n", message.c_str());
    return;
  }

  server.send(200, "text/plain; charset=utf-8", "OK");
  Serial.println("[OTA] Firmware updated successfully, reboot scheduled");
  scheduleReboot();
}

void handleNotFound() {
  server.send(404, "text/plain; charset=utf-8", "Not found");
}

void initWiFi() {
  WiFi.mode(WIFI_AP);
  delay(100);

  if (strlen(AP_PASSWORD) > 0 && strlen(AP_PASSWORD) < 8) {
    Serial.println("[WiFi] AP password must be at least 8 characters for WPA2");
  }

  bool apStarted = false;
  if (strlen(AP_PASSWORD) == 0) {
    apStarted = WiFi.softAP(AP_SSID);
  } else {
    apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
  }

  if (!apStarted) {
    Serial.println("[WiFi] Failed to start access point");
    return;
  }

  Serial.println("[WiFi] Access Point started");
  Serial.printf("[WiFi] SSID: %s\n", AP_SSID);
  if (strlen(AP_PASSWORD) == 0) {
    Serial.println("[WiFi] Security: Open");
  } else {
    Serial.println("[WiFi] Security: WPA2");
  }
  Serial.printf("[WiFi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void initMDNS() {
  if (MDNS.begin(MDNS_HOST)) {
    Serial.printf("[mDNS] Started: http://%s.local\n", MDNS_HOST);
  } else {
    Serial.println("[mDNS] Failed to start");
  }
}

void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on(
      "/update", HTTP_POST, handleUpdateResult, handleUpdateUpload);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[HTTP] Server started on port 80");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== ESP Firmware Updater ===");

  initWiFi();
  initMDNS();
  setupServer();

  Serial.println("[System] Ready");
}

void loop() {
  server.handleClient();

#if defined(ESP8266)
  MDNS.update();
#endif

  if (shouldReboot && static_cast<long>(millis() - rebootAtMs) >= 0) {
    Serial.println("[System] Rebooting now");
    delay(100);
    ESP.restart();
  }
}
