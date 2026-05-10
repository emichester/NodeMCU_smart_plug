# NodeMCU_smart_plug

Custom programmable smart plug with Telegram bot control. Control a relay remotely via Telegram with support for timers, daily schedules, and day-specific automation.

## Features

- 🤖 Telegram bot control (`/encender`, `/apagar`)
- ⏳ Dynamic timer support (`/timer 10 m`)
- 📅 Daily and day-specific scheduling (`/diario 08:00 16:00`, `/lunes 10:00 14:00`)
- 🔄 Wi-Fi reconnection with watchdog timer
- 🔒 Secure credential handling (secrets not in version control)
- ⏱️ Madrid timezone support (CET-1CEST)

## Requirements

- **Hardware**:
  - NodeMCU v2 (or compatible ESP8266)
  - Relay module
  - Level shifter circuit in my case (to send 5V signal to the relay module instead of 3.3V)
- **Libraries**:
  - ESP8266WiFi
  - WiFiClientSecure
  - UniversalTelegramBot
  - ArduinoJson

- **Tools**: Arduino CLI or Arduino IDE

## Setup Instructions

### 1. Get Your Credentials

**Telegram Bot Token:**
1. Open Telegram and search for `@BotFather`
2. Send `/newbot` and follow the instructions
3. Save the API token (e.g., `1234567890:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghi`)

**Your Chat ID:**
1. Search for `@userinfobot` in Telegram
2. Send any message to get your chat ID

**Wi-Fi Credentials:**
- Your SSID (network name)
- Your Wi-Fi password

### 2. Configure Secrets

1. Copy the template:
   ```bash
   cp secrets.example.h secrets.h
   ```

2. Edit `secrets.h` and fill in your credentials:
   ```cpp
   #ifndef SECRETS_H
   #define SECRETS_H
   
   const char* ssid = "YOUR_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   
   const char* BOTtoken = "YOUR_BOT_TOKEN_HERE";
   const char* CHAT_ID = "YOUR_CHAT_ID_HERE";
   
   #endif
   ```

3. **Never commit `secrets.h`** – it's already in `.gitignore`

### 3. Compile & Upload

**Using Arduino CLI:**
```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 NodeMCU_smart_plug.ino
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2 NodeMCU_smart_plug.ino
```

**Using Arduino IDE:**
1. Open `NodeMCU_smart_plug.ino`
2. Select Board: "NodeMCU 1.0 (ESP8266-12E)"
3. Select Port: (your USB port)
4. Click Upload

## Telegram Commands

| Command | Example | Description |
|---------|---------|-------------|
| `/start` | `/start` | Show help menu |
| `/encender` | `/encender` | Turn relay ON |
| `/apagar` | `/apagar` | Turn relay OFF |
| `/timer` | `/timer 10 m` | Turn ON for 10 minutes/seconds/hours |
| `/diario` | `/diario 08:00 16:00` | Daily schedule |
| `/[dia]` | `/lunes 10:00 14:00` | Specific day schedule (lunes, martes, miércoles, etc.) |
| `/status` | `/status` | Show current relay & schedule status |
| `/cancelar` | `/cancelar` | Cancel active schedules & timers |

## Project Structure

```
NodeMCU_smart_plug.ino      # Main file (setup/loop, watchdog, Wi-Fi)
config.h                     # Global configs & declarations
commands.cpp/h              # Telegram command handlers
schedule.cpp/h              # Timer & schedule logic
secrets.h                  # ⚠️ Local credentials (ignored by git)
secrets.example.h          # Template for credentials
```

## Security Notes

⚠️ **Important:**
- `secrets.h` contains sensitive credentials and is **not** committed to git
- Never share your bot token or chat ID
- If you accidentally commit credentials, regenerate your bot token immediately at BotFather
- Always use the provided `.gitignore` rules

## Hardware Wiring

Connect relay module to pin D1 (GPIO5) on NodeMCU:
- D1 → IN pin on relay module
- GND → GND on relay module
- 5V → VCC on relay module (via appropriate power supply)

## Troubleshooting

**Won't connect to Wi-Fi:**
- Check SSID and password in `secrets.h`
- Verify Wi-Fi is 2.4GHz (ESP8266 doesn't support 5GHz)

**Bot doesn't respond:**
- Verify bot token is correct
- Ensure chat ID is correct
- Check device has internet connection

**Compilation errors:**
- Install dependencies via Arduino IDE Library Manager
- For ESP8266 core, use version 3.1.2 or later

## License

Custom project – use for personal purposes
