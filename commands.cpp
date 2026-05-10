#include "commands.h"
#include "schedule.h"

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
        bot.sendMessage(CHAT_ID, "Temporizador activado por " + String(value) + String(unit), "");
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