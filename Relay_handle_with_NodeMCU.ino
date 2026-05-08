#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"

const int relayPin = D1;
bool relayStatus = LOW;

// Configuración de Hora (Madrid)
const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3"; 

// Estructura para programaciones
struct Task {
  bool active = false;
  int startH, startM;
  int endH, endM;
  int dayOfWeek; // -1: diario, 1:Lunes... 7:Domingo (tm_wday usa 0 para Domingo, ajustado en lógica)
};
Task schedule;

unsigned long timerEnd = 0;
bool timerActive = false;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
X509List cert(TELEGRAM_CERTIFICATE_ROOT);

void updateRelay(bool state) {
  digitalWrite(relayPin, state);
  relayStatus = state;
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) continue;

    String text = bot.messages[i].text;
    text.toLowerCase();

    // 1. COMANDOS SIMPLES
    if (text == "/start") {
      String welcome = "🤖 *Control de Toma de Fuerza*\n\n";
      welcome += "/encender | /apagar \n";
      welcome += "/timer [valor] [s/m/h] -> Ej: /timer 10 m\n";
      welcome += "/diario [HH:MM] [HH:MM] -> Ej: /diario 08:00 16:00\n";
      welcome += "/[dia] [HH:MM] [HH:MM] -> Ej: /lunes 10:00 14:00\n";
      welcome += "/status | /cancelar";
      bot.sendMessage(CHAT_ID, welcome, "Markdown");
    }

    else if (text == "/encender") {
      updateRelay(HIGH);
      bot.sendMessage(CHAT_ID, "Manual: ENCENDIDO ✅", "");
    }

    else if (text == "/apagar") {
      updateRelay(LOW);
      timerActive = false;
      bot.sendMessage(CHAT_ID, "Manual: APAGADO ❌", "");
    }

    // 2. TEMPORIZADOR DINÁMICO (Ej: /timer 45 m)
    else if (text.startsWith("/timer")) {
      int value = 0;
      char unit = ' ';
      // Escaneamos el texto buscando el número y la unidad
      sscanf(text.c_str(), "/timer %d %c", &value, &unit);

      unsigned long duration = 0;
      if (unit == 's') duration = value * 1000UL;
      else if (unit == 'm') duration = value * 60UL * 1000UL;
      else if (unit == 'h') duration = value * 3600UL * 1000UL;

      if (duration > 0) {
        timerEnd = millis() + duration;
        timerActive = true;
        updateRelay(HIGH);
        bot.sendMessage(CHAT_ID, "Temporizador activado por " + String(value) + unit, "");
      }
    }

    // 3. PROGRAMACIÓN (Ej: /diario 08:00 20:00 o /lunes 09:00 10:00)
    else if (text.startsWith("/diario") || text.startsWith("/lunes") || text.startsWith("/martes") || 
             text.startsWith("/miercoles") || text.startsWith("/jueves") || text.startsWith("/viernes") || 
             text.startsWith("/sabado") || text.startsWith("/domingo")) {
      
      int sh, sm, eh, em;
      if (sscanf(text.c_str(), "%*s %d:%d %d:%d", &sh, &sm, &eh, &em) == 4) {
        schedule.active = true;
        schedule.startH = sh; schedule.startM = sm;
        schedule.endH = eh;   schedule.endM = em;
        
        if (text.startsWith("/diario")) schedule.dayOfWeek = -1;
        else if (text.startsWith("/lunes")) schedule.dayOfWeek = 1;
        else if (text.startsWith("/martes")) schedule.dayOfWeek = 2;
        else if (text.startsWith("/miercoles")) schedule.dayOfWeek = 3;
        else if (text.startsWith("/jueves")) schedule.dayOfWeek = 4;
        else if (text.startsWith("/viernes")) schedule.dayOfWeek = 5;
        else if (text.startsWith("/sabado")) schedule.dayOfWeek = 6;
        else if (text.startsWith("/domingo")) schedule.dayOfWeek = 0;

        bot.sendMessage(CHAT_ID, "Programación guardada con éxito 📅", "");
      }
    }

    else if (text == "/cancelar") {
      schedule.active = false;
      timerActive = false;
      updateRelay(LOW);
      bot.sendMessage(CHAT_ID, "Programaciones anuladas. Relé apagado.", "");
    }

    else if (text == "/status") {
      String st = "Relé: " + String(relayStatus ? "ON" : "OFF") + "\n";
      if (timerActive) st += "Temporizador: " + String((timerEnd - millis())/1000) + "s restantes\n";
      if (schedule.active) st += "Prog: Activa (" + String(schedule.startH) + ":" + String(schedule.startM) + " a " + String(schedule.endH) + ":" + String(schedule.endM) + ")";
      bot.sendMessage(CHAT_ID, st, "");
    }
  }
}

void checkLogic() {
  // Timer check
  if (timerActive && millis() > timerEnd) {
    updateRelay(LOW);
    timerActive = false;
    bot.sendMessage(CHAT_ID, "⏳ Temporizador agotado. Apagado.", "");
  }

  // Schedule check
  if (schedule.active) {
    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);
    int nowMins = ptm->tm_hour * 60 + ptm->tm_min;
    int startMins = schedule.startH * 60 + schedule.startM;
    int endMins = schedule.endH * 60 + schedule.endM;

    bool correctDay = (schedule.dayOfWeek == -1 || schedule.dayOfWeek == ptm->tm_wday);

    if (correctDay && nowMins >= startMins && nowMins < endMins) {
      if (!relayStatus) updateRelay(HIGH);
    } else {
      if (relayStatus && !timerActive) updateRelay(LOW);
    }
  }
}

void setup() {
  pinMode(relayPin, OUTPUT);
  updateRelay(LOW); // Seguridad absoluta al arrancar

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  configTime(TZ_INFO, "pool.ntp.org");
  client.setTrustAnchors(&cert);
  
  // Esperar a que la hora se sincronice
  time_t now = time(nullptr);
  while (now < 24 * 3600) { delay(100); now = time(nullptr); }
}

void loop() {
  static unsigned long lastBotScan;
  if (millis() - lastBotScan > 1500) {
    int num = bot.getUpdates(bot.last_message_received + 1);
    handleNewMessages(num);
    lastBotScan = millis();
  }
  checkLogic();
}
