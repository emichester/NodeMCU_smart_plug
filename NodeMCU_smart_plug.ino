#include "secrets.h"
#include "config.h"
#include "commands.h"
#include "schedule.h"
#include "network.h"

bool relayStatus = LOW;
const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
X509List cert(TELEGRAM_CERTIFICATE_ROOT);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== Setup start ===");
  Serial.print("relayPin = ");
  Serial.println(relayPin);

  pinMode(relayPin, OUTPUT);
  updateRelay(LOW); // Seguridad absoluta al arrancar
  Serial.println("Relay forced LOW at startup");

  Serial.println("Connecting to WiFi...");
  connectToWiFi();
  Serial.print("WiFi connection attempt finished. Status = ");
  Serial.println(WiFi.status());

  Serial.println("Configuring NTP / time sync...");
  configTime(TZ_INFO, "pool.ntp.org");
  client.setTrustAnchors(&cert);

  // Esperar a que la hora se sincronice
  time_t now = time(nullptr);
  Serial.print("Waiting for clock sync. Current time = ");
  Serial.println(now);
  while (now < 24 * 3600) {
    delay(100);
    now = time(nullptr);
    if (millis() % 5000 == 0) {
      Serial.print("Waiting for valid time... current epoch = ");
      Serial.println(now);
    }
  }
  Serial.print("Time synced. Current epoch = ");
  Serial.println(now);

  // Enable watchdog timer
  ESP.wdtEnable(5000);
  Serial.println("Watchdog enabled");
  Serial.println("=== Setup complete ===");
}

void loop() {
  static unsigned long lastDebugPrint = 0;
  static unsigned long lastBotScan;

  // Feed watchdog
  ESP.wdtFeed();

  // Wi-Fi reconnect logic
  maintainWiFiConnection();

  if (millis() - lastBotScan > 1500) {
    Serial.print("Polling Telegram. last_message_received = ");
    Serial.println(bot.last_message_received);
    int num = bot.getUpdates(bot.last_message_received + 1);
    Serial.print("Telegram updates returned: ");
    Serial.println(num);
    handleNewMessages(num);
    lastBotScan = millis();
  }

  if (millis() - lastDebugPrint > 10000) {
    Serial.print("Loop heartbeat. millis = ");
    Serial.print(millis());
    Serial.print(" | WiFi status = ");
    Serial.print(WiFi.status());
    Serial.print(" | relayStatus = ");
    Serial.println(relayStatus ? "HIGH" : "LOW");
    lastDebugPrint = millis();
  }

  checkLogic();
}
