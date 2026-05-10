#ifndef CONFIG_H
#define CONFIG_H

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <time.h>

extern const char* ssid;
extern const char* password;
extern const char* BOTtoken;
extern const char* CHAT_ID;

const int relayPin = D1;
extern bool relayStatus;

// Configuración de Hora (Madrid)
extern const char* TZ_INFO;

// Estructura para programaciones
struct Task {
  bool active = false;
  int startH, startM;
  int endH, endM;
  int dayOfWeek; // -1: diario, 1:Lunes... 7:Domingo (tm_wday usa 0 para Domingo, ajustado en lógica)
};

extern Task schedule;
extern unsigned long timerEnd;
extern bool timerActive;

extern WiFiClientSecure client;
extern UniversalTelegramBot bot;
extern X509List cert;

#endif