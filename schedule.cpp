#include "schedule.h"

Task schedule;
unsigned long timerEnd = 0;
bool timerActive = false;

void updateRelay(bool state) {
  if (relayStatus != state) {
    digitalWrite(relayPin, state);
    relayStatus = state;
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