#include "secrets.h"
#include "config.h"
#include "commands.h"
#include "schedule.h"

bool relayStatus = LOW;
const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
X509List cert(TELEGRAM_CERTIFICATE_ROOT);

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

  // Enable watchdog timer
  ESP.wdtEnable(5000);
}

void loop() {
  // Feed watchdog
  ESP.wdtFeed();

  // Wi-Fi reconnect logic
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(5000);
  }

  static unsigned long lastBotScan;
  if (millis() - lastBotScan > 1500) {
    int num = bot.getUpdates(bot.last_message_received + 1);
    handleNewMessages(num);
    lastBotScan = millis();
  }
  checkLogic();
}
